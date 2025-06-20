/* this file contains code from two TextEdit-based little editors: */

/*********************************************************************

	mini.windows.c

	window functions for MiniEdit
	The sample application from Inside Macintosh (RoadMap p.15-17)
	beefed up a bit by Stephen Z. Stein, Symantec Corp.

*********************************************************************/
/*------------------------------------------------------------------------------
#
#	Apple Macintosh Developer Technical Support
#
#	MultiFinder-Aware TextEdit Sample Application
#
#	TESample.c
#
#	Copyright � 1989 Apple Computer, Inc.
#	All rights reserved.
#
------------------------------------------------------------------------------*/

/* the integration of the two environments (MiniEdit is autowrap-only, TESample
is horizontal scroll only, both are single-window and their programming style is 
often radically different) is by us: */

/*******************************************************************************\

main events module

suntar, �1991-94 Sauro & Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/

/* In general, we have kept the general structure of MiniEdit (which is easier to
understand) but inserting many small excerpts of code from TESample (which is more
powerful: for example, it supports the zoom box). Many features, however, are
completely new for suntar and come from neither MiniEdit nor TESample.
*/

/*#include "antiglue.h"*/
#include "windows.h"

extern BitMap	screenBits;
#define TITLEBARHEIGHT	20

#define HScrollMax	256			/* in caratteri */
#define	MaxDocWidth	16000	/* in pixel */
/* per essere intelligenti, bisognerebbe che HScrollMax e MaxDocWidth siano legate alla
lunghezza max delle righe nel testo (legate, e non coincidenti, perch� ad es 0 ha un
significato speciale) ma � molto pi� semplice sia un valore costante, anche molti programmi 
seri fanno cos� */

short current_font=4,		/* Monaco */
	current_size=9,
	current_style=0,
	curr_font_index;

long myClickLoop(void);
void new_Hscroll_bar(window_def *,Boolean);
void set_window_flags(window_def *);
void dummyClickLoop(void);
void CClikLoop(void);
void SetVScrollMax(window_def *);
void SetView(WindowPtr);
pascal void ScrollProc(ControlHandle,short);
void resize_controls(WindowPtr);
void resize_text(WindowPtr);
void testo_in_abbondanza(TEHandle,short *);
short find_new_position(window_def*,short);

#if 0
/* used for debugging: print to the second window (Ok, it would be more
powerful to adapt printf to print to any window...) */
#include <string.h>
#include "suntar.h"
void print_string(buf)
char*buf;
{
GrafPtr	savePort;
if(!my_windows[1].used) return;
GetPort( &savePort );
SetPort(&my_windows[1]);
TEInsert(buf,(long)strlen(buf),my_windows[1].TEH);
SetPort( savePort );
}

void print_int(i)
short i;
{
char buffer[20];
if(!my_windows[1].used) return;
my_itoa((long)i,buffer);
print_string(buffer);
MaintainScrollBars(&my_windows[1]);
}

void print_number(desc,i)
char*desc;
short i;
{
if(!my_windows[1].used) return;
TESetSelect((long)32767,(long)32767,my_windows[1].TEH);
print_string(desc);
print_int(i);
print_string("\r");
MaintainScrollBars(&my_windows[1]);
}
#endif


void new_window()
{
	Rect	destRect, viewRect;
	short	i,height;

	if((i=get_window_index())<0)return; /* non dovrebbe capitare, open � disabilitato */
	set_window_flags(curr_window);
	
	{Rect bounds;
	bounds.top=MBARHEIGHT +TITLEBARHEIGHT+4 + (TITLEBARHEIGHT-6)*i;
	bounds.left=4+8*i;
	bounds.bottom=min(480,screenHeight) -16+3*i;
	#ifdef SUNTAR
	bounds.right= min(640,screenWidth) - (screenWidth<640 ? 32 : 68) +3*i;
	#else
	bounds.right= min(640,screenWidth) - (screenWidth<640 ? 32 : 72) +3*i;
	#endif

/*	se gli si fornisce l'area allocata, NewWindow non fa altro che ritornare il 
	primo parametro, quindi � inutile leggerlo a meno di test errori... */

	if(NULL==NewWindow( curr_window, &bounds, window_title, 1, 
		vecchiaROM ? 0 : 8, (WindowPtr)-1L, (curr_window->flags&NOCLOSEBOX)==0, (long)i));
	}

	SetPort(curr_window);
	TextFont(current_font);
	TextSize(current_size);
	TextFace(current_style);

	{Rect	vScrollRect;
	vScrollRect = (*myWindow).portRect;
	vScrollRect.left = vScrollRect.right-SBarWidth;
	vScrollRect.right += 1;
	vScrollRect.bottom -= SBarWidth-1;
	vScrollRect.top -= 1;
#ifndef SUNTAR
	/* creo la scrollbar invisibile, tanto poi viene resa attiva comunque, in
	modo che la sua mancanza contribuisce a rafforzare l'impressione di attesa 
	del cursore a forma di orologio durante il caricamento del file */
	curr_window->vScroll = NewControl( myWindow, &vScrollRect, PNS, 0, 0, 0, 
		0, scrollBarProc, 0L);
#else
	curr_window->vScroll = NewControl( myWindow, &vScrollRect, PNS, 1, 0, 0, 
		0, scrollBarProc, 0L);
#endif
	}

#ifdef HSCROLL
#ifdef SUNTAR
	if(curr_window->flags & HSCROLL) new_Hscroll_bar(curr_window,1);
#else
	if(curr_window->flags & HSCROLL) new_Hscroll_bar(curr_window,0);
#endif
#endif
	{Rect destRect;
	viewRect = thePort->portRect;
	viewRect.right -= SBarWidth;
		viewRect.bottom -= SBarWidth;
	InsetRect(&viewRect, 4, 4);
	destRect=viewRect;
#ifdef HSCROLL
	/*if(curr_window->flags & HSCROLL)
	destRect.right = destRect.left + MaxDocWidth;inutile perch� SetView lo riassegna*/
#endif
	TEH=curr_window->TEH = TENew( &destRect, &viewRect );
	}


#ifdef HSCROLL
	if(curr_window->flags & HSCROLL) (**TEH).crOnly=-1;
#endif
	/*set_window_font();*/
	SetView(thePort);
	curr_window->dirty = 0;
	if(curr_window->flags&READONLY) curr_window->lastPrompt=32767;
	if(!vecchiaROM){
		TEAutoView (true,TEH); /* auto-scroll on...*/
		curr_window->itsClickLoop=(**TEH).clikLoop;
		(**TEH).clikLoop=(ProcPtr)myClickLoop;
		}
	/* il chiamante dovr� occuparsi dei valori corrente e max delle scrollbar 
	(non posso farlo ora perch� il buffer � ancora vuoto)
	-- the caller must set the current and max values of the scrollbar (after
	filling the window with the initial text, if any)
	 */
}

static void new_Hscroll_bar(window,visible)
register window_def *window;
Boolean visible;
{
	Rect	hScrollRect;
	hScrollRect = ((WindowPtr)window)->portRect;
	hScrollRect.top = hScrollRect.bottom-SBarWidth;
	hScrollRect.bottom += 1;
	hScrollRect.right -= SBarWidth-1;
	hScrollRect.left += ROOM_BEFORE_HSCROLL-1;
	if(window != curr_window) visible=false;
	window->hScroll = NewControl( window, &hScrollRect, PNS, visible, 0, 0, 
		HScrollMax, scrollBarProc, 0L);
	SetCtlValue(window->hScroll,0);
	/*if(window != curr_window) HideControl( window->hScroll );*/

}

short get_window_index()
{
short current_index=0;

if(n_currently_open>=n_max_windows)return -1;	/* non dovrebbe succedere, il comando � disabilitato...
		-- it should never happen, if the New command is disabled when...*/
while(my_windows[current_index].used) current_index++;
curr_window=&my_windows[current_index];
curr_window->used=true;
n_currently_open++;

return current_index;
}

static void set_window_flags(window)
register window_def *window;
{
short markChar;
extern MenuHandle	myMenus[4];

window->flags=default_flags;
window->lastPrompt=32767;
GetItemMark (myMenus[prefM], pmAutowrap, &markChar);
if(!markChar) window->flags |= HSCROLL;
}


/* questa routine modifica gli attributi della finestra specificata per 
adeguarli alle preferenze correnti */

void apply_preferences(w)
/* apply the current setting of the preferences menu to the window.
I wrote this routine for a multiwindow editor where changing the
preferences does not automatically change the configuration of
the current window, and there was an explicit "Apply" command.
In suntar there is only one window, and this routine is called
every time one changes the font, style, size or the autowrap/Hscroll
mode.
In a different environment, one could be obliged to store different
settings of the preferences menu and its submenus and restore them
into the menu every time a window becomes the current window. Obviously a new
window should inherit the current settings in the preferences menu
*/

register window_def *w;
{
FontInfo	fInfo;
short oldbottomline;
short oldflags;
GrafPtr	savePort;
register TEHandle myTEH=w->TEH;

GetPort( &savePort );
SetPort(w);

oldflags=w->flags;

/*
print_number("old val=",GetCtlValue(w->vScroll));
print_number("old max=",GetCtlMax(w->vScroll)); */
set_window_flags(w);
(**myTEH).crOnly = (w->flags & HSCROLL) ? -1 : 0;

oldbottomline= ((**myTEH).viewRect.top - (**myTEH).destRect.top) / (**myTEH).lineHeight;
		/* that's not the same as GetCtlValue, it could have been clipped to the max...*/
(**myTEH).viewRect.bottom = ((WindowPtr)w)->portRect.bottom- SBarWidth;	/* perch� � mantenuta
			multiplo della lunghezza della linea, cambiando cambia tutto...*/

EraseRect(&(*myTEH)->viewRect);
InvalRect(&((WindowPtr)w)->portRect);
HidePen();

if( (oldflags&HSCROLL) != ( w->flags&HSCROLL) ){
	if(!(oldflags&HSCROLL))
		new_Hscroll_bar(w,1);
	else
		DisposeControl(w->hScroll);
	}

TextSize ( (**myTEH).txSize=current_size );
TextFont ( (**myTEH).txFont=current_font );
TextFace ( (**myTEH).txFace=current_style );

GetFontInfo(&fInfo);
(**myTEH).lineHeight=fInfo.ascent+fInfo.descent+fInfo.leading;
/*print_string("new line H=");
print_int(fInfo.ascent+fInfo.descent+fInfo.leading);
print_string("\r");*/
(**myTEH).fontAscent=fInfo.ascent;
/* w->char_width=fInfo.widMax;  inutile, lo fa poi SetView */

/*
se l'altezza del font � cambiata bisogna adeguare la posizione verticale, se la larghezza
� cambiata (e non posso guardare carattere per carattere nel caso di font proporzionali...)
bisogna adeguare lo shift orizzontale; in caso di autowrap, una qualunque delle due cose 
e bisogna decidere dove portarsi */

if( (w->flags&HSCROLL) && (oldflags&HSCROLL)){
	SetView(w);
	SetVScrollMax(w);	/* lines_in_window may have changed... */
	SetCtlValue(w->vScroll,oldbottomline);	/* that should still be the current value,
			but it could have been clipped to the max, and the max is changed */
	}
else{
	short oldtopchar=(**myTEH).lineStarts[oldbottomline];
	/*short i;
	print_number("oldtop=",oldtopchar);
	print_number("old = ",((**myTEH).viewRect.top-(**myTEH).destRect.top));
	print_number("bottom line=",oldbottomline); */
	SetView(w);	/* che in autowrap ricalcola gli inizi di riga...*/
	if( (w->flags&HSCROLL) &&  ! (oldflags&HSCROLL)){
		SetCursor(&waitCursor);
		TECalText(myTEH);	/*in questo caso, SetView non la chiama..
							-- only in this case, SetView does not call it */
		SetCursor(&arrow);
		}
	if(!(w->flags&HSCROLL) )
		testo_in_abbondanza(myTEH,&oldtopchar);
	SetVScrollMax(w);
	SetCtlValue(w->vScroll,find_new_position(w,oldtopchar) );
	}

AdjustText(w);	/* che scrolla per adeguare al settaggio delle scrollbar
				-- which scrolls the text according to the scrollbar settings */
ShowPen();

SetPort(savePort);
/*print_number("new = ",((**myTEH).viewRect.top-(**myTEH).destRect.top));*/
}

/* purtroppo alla routine di ClikLoop � stata imposta un'interfaccia che, pur
assomigliando alle regole usate dal compilatore C, non coincide con esse per 
cui una routinetta di "glue" in assembly ci vuole
-- unfortunately, the interface to the ClikLoop routine is not compatible
with the C calling conventions; also Apple's TESample used a glue in assembly,
and this routine is inspired to that one */

static void dummyClickLoop()
{
asm{
extern myClickLoop: /* questo � il vero entry point, non voglio eseguire le operazioni
					di ingresso inserite dal compilatore...
					-- that's the true entry point, after the operations introduced
					by the compiler (probably a LINK instruction); anyway it does
					not work to pass the C function name and doing return rather than rts */
	/* see New Inside Macintosh Memory pag 4-15: TextEdit click-loop routine
	belong to the group which might be called with an invalid A5: really, suntar
	has worked for three years without causing any problem, but... */
	MOVE.L		A5,-(SP)
	MOVE.L		CurrentA5,A5
	move.l		curr_window,A0
	move.l		OFFSET(window_def,itsClickLoop)(A0),A0
	jsr			(A0)		; chiama prima la sua routine...
							; -- call the click loop routine which was installed by
							; TEAutoView
	movem.l		D1-D2/A1,-(SP) ; salva i registri sporcati dalle funzioni C ma da non sporcare
							;save the registers which a C fuction uses but this routine
							;must not alter
	}

CClikLoop();		/* which adjusts scroll bars */
asm{
	movem.l		(SP)+,D1-D2/A1	;restore registers
	MOVE.L		(SP)+,A5
	moveq		#1,D0			;assign return value
	rts							;return to the caller
	}
}

void CClikLoop()
/* presa da TEsample.c, negli esempi dell'MPW, ma con forti modifiche
-- from TEsample.c, heavily modified */
{
	RgnHandle	region;
	short newscroll;
	
	region = NewRgn();
	GetClip(region);					/* save clip */
	ClipRect(&myWindow->portRect);
/* le formule qui usate ovviamente sono le stesse di MaintainScrollBars
-- same formulas as in MaintainScrollBars: */
	newscroll=((**TEH).viewRect.top - (**TEH).destRect.top) / (**TEH).lineHeight;
	SetCtlValue(curr_window->vScroll,newscroll);
#ifdef HSCROLL
	if(curr_window->flags & HSCROLL){
		newscroll=((**TEH).viewRect.left - (**TEH).destRect.left)/curr_window->char_width;
		SetCtlValue(curr_window->hScroll,newscroll);
	}
#endif
	SetClip(region);					/* restore clip */
	DisposeRgn(region);
}


/* i casi sono due: o la posizione � cambiata per azioni nel contenuto della finestra
(tasti return o freccia o delete o paste o autoscroll...) e allora chiamo MaintaiScrollBars  
per adeguare la posizione dei bottoni delle scrollbar, o si � agito sulle scrollbar
e allora si chiama AdjustText per scrollare il testo in modo da adeguarsi
-- if the position has changed for operations in the contents in the window
one must call MaintaiScrollBars to adjust scroll bars, if the situation was
changed due to a click/drag in the scroll bars, one must call AdjustText
*/

void MaintainScrollBars(w)
register window_def *w;
{
short newscroll;
register TERec *TEp=*(w->TEH);
/* se andassi a leggere i valori direttamente nel ControlRecord farei degli if
per non chiamare queste routines se il nuovo valore � uguale al vecchio, ma se devo
chiamare delle GetCtl... per avere il vecchio, tanto vale chiamare comunque SetCtl.. */

#if 1
/* a few instructions copied from setVscrollMax: */
short n = TEp->nLines - w->linesInWindow;

if (TEp->teLength > 0 && (*(TEp->hText))[TEp->teLength-1]==CR)
	n++;
if(n<0) n=0;

newscroll=(TEp->viewRect.top - TEp->destRect.top) / TEp->lineHeight;
if((**w->vScroll).contrlMax && (**w->vScroll).contrlMax==(**w->vScroll).contrlValue &&
	n==newscroll){	/* la scrollbar era al massimo e resta al massimo, la posizione
			del quadratino non cambia, quindi non succede niente di male
			se aggiorno i valori direttamente nel record SENZA passare
			per le routines apposite
			-- during a series of printf it's typical that the scrollbar must
			stay at the ending position, but using SetCtlValue and SetCtlMax
			one can't avoid to draw the scrollbar in an temporary position,
			after updating Max but not Value. In that case, just assign the
			struct fields, no drawing need be performed
			*/
	(**w->vScroll).contrlValue=newscroll;
	(**w->vScroll).contrlMax=n;
	}
else{
	/* A similar optimization is possible: but beware, if only one
	of the the numbers changes, the other one must be set anyway. If both
	change, one may "poke" the Max and then set the value (the opposite does
	not limit the value to Max)
	Attento, non sono del tutto sicuro che funzioni*/
	if((**w->vScroll).contrlValue==newscroll)
		SetCtlMax(w->vScroll, n);
	else{
		(**w->vScroll).contrlMax=n;
		SetCtlValue(w->vScroll,newscroll);
		}
	}
#else
/* no strange optimizations */
newscroll=(TEp->viewRect.top - TEp->destRect.top) / TEp->lineHeight;
SetVScrollMax(w);
SetCtlValue(w->vScroll,newscroll);
#endif

#ifdef HSCROLL
if(w->flags & HSCROLL){
	TEp=*(w->TEH);	/* SetCtlValue may move memory */
	newscroll= (TEp->viewRect.left - TEp->destRect.left)/w->char_width;
	SetCtlValue(w->hScroll,newscroll);
	}
#endif
}

void AdjustText (w)
register window_def *w;
{
	short oldScroll, newScroll, delta1,delta2;
	register TERec *TEp=*(w->TEH);

	oldScroll = TEp->viewRect.top - TEp->destRect.top;
	newScroll = GetCtlValue(w->vScroll) * TEp->lineHeight; /* does not move memory */
	/*
	print_number("scrollbar=",GetCtlValue(w->vScroll));
	print_number("line height=",(**(w->TEH)).lineHeight);
	print_number("old scroll=",oldScroll);
	print_number("new scroll=",newScroll);
	TEp=*(w->TEH);
	*/
	delta1 = oldScroll - newScroll;
#ifdef HSCROLL
	if(w->flags & HSCROLL){
		oldScroll = TEp->viewRect.left - TEp->destRect.left;
		newScroll = GetCtlValue(w->hScroll)  * w->char_width;
		delta2 = oldScroll - newScroll;
		}
	else{
		delta2=0;
		if(  ! (w->flags & HSCROLL) )	/* succede solo passando da scroll a autowrap */
			delta2=TEp->viewRect.left-TEp->destRect.left;
		}
#else
delta2=0;
#endif
if (delta1 || delta2)
  TEScroll(delta2, delta1, w->TEH);
}


static void SetVScrollMax(w)
register window_def *w;
{
	register short	n;
	register TERec*TEp = *(w->TEH);
/* It is a kluge necessitated by a minor bug in TE. The bug is that if the last 
character in the TE text buffer is a Carriage Return, the field "nLines" may be off by one,
and cause slightly incorrect updating of the window. These instructions always returns the
correct number of lines in the buffer.
*/
	n = TEp->nLines - w->linesInWindow;

	if (TEp->teLength > 0 && (*(TEp->hText))[TEp->teLength-1]==CR)
		n++;

	SetCtlMax(w->vScroll, n > 0 ? n : 0);

}

#ifdef may_run_on_64K_ROM
ShowSelect()

{

/* se � attivato l'autoscroll, il TextEdit provvede a portare in vista il punto in cui
sta il cursore, quindi non devo fare altro che sistemare le scroll bar, e lo faccio senza
chiamare questa routine; sulle vecchie ROM l'autoscroll non � disponibile, e allora non 
posso fare a meno di cercare il punto di selezione e portarlo in vista */

SetVScrollMax(curr_window);
{
	register	TERec* TEp;
	register	short	theLine;
	
/* la ricerca lineare nell'array lineStarts fa schifo, bisognerebbe usare ricerca 
logaritmica. Non l'ho cambiato perch� viene usato solo con le ROM pre-Mac plus...
Nota che l'ultimo lineStarts � sempre uguale alla lunghezza del testo, per cui il 
ciclo si ferma comunque...
 */
 {
 	register short topLine, bottomLine;
	topLine = GetCtlValue(curr_window->vScroll);
	bottomLine = topLine + curr_window->linesInWindow;
	TEp=*TEH;
#ifdef HSCROLL
 /* in questo caso theLine mi serve per il calcolo in orizzontale per cui non
 posso evitare di fare la ricerca */
	theLine = TEp->selStart < TEp->lineStarts[bottomLine] ? 0 : bottomLine;
	for (; TEp->selStart >= TEp->lineStarts[theLine+1]; theLine++)
		;
	if (TEp->selStart < TEp->lineStarts[topLine] ||
		TEp->selStart >= TEp->lineStarts[bottomLine]) {
			SetCtlValue(curr_window->vScroll,theLine - curr_window->linesInWindow / 2);
			TEp=*TEH;	/* SetCtlValue pu� chiamare il Quickdraw e quindi anche il MemMgr */
		}
}

{
	short strwidth;
	strwidth=TextWidth( *(TEp->hText), TEp->lineStarts[theLine], 
		TEp->selStart - TEp->lineStarts[theLine]);
	TEp=*TEH;	/*non credo ci siano problemi, ma a scanso di guai...*/
	if(strwidth< (TEp->viewRect.left-TEp->destRect.left) || 
	   strwidth> (TEp->viewRect.right-TEp->destRect.left) )
		SetCtlValue(curr_window->hScroll,
				(strwidth+ (TEp->viewRect.left-TEp->viewRect.right)/2)/curr_window->char_width);
#else
	if (TEp->selStart < TEp->lineStarts[topLine] ||
			TEp->selStart >= TEp->lineStarts[bottomLine]) {
		theLine = TEp->selStart < TEp->lineStarts[bottomLine] ? 0 : bottomLine;
		for (; TEp->selStart >= TEp->lineStarts[theLine+1]; theLine++)
			;
		SetCtlValue(curr_window->vScroll,
			theLine - curr_window->linesInWindow / 2);
		}
#endif
}
	AdjustText(curr_window);
}}
#endif


static void SetView(w)
/* SetView, da chiamare quando si crea una finestra o si modifica font, dimensione, 
o attributo di autowrap; NON si occupa delle scrollbar
-- to be called when a window is created or the font-size-autowrap state is
changed */
/* calcola viewrect e informazioni associate; per autowrap (e solo autowrap)
chiama anche TECaText */
register WindowPtr w;
{
	FontInfo fInfo;
	register TERec* TEp;
	GetFontInfo(&fInfo);
	TEp=*(((window_def*)w)->TEH);
	TEp->viewRect = w->portRect;
	TEp->viewRect.right -= SBarWidth;
		TEp->viewRect.bottom -= SBarWidth;
	InsetRect(&TEp->viewRect, 4, 4); /*certo non chiama il memory manager...*/
	((window_def*)w)->linesInWindow =
		(TEp->viewRect.bottom-TEp->viewRect.top)/TEp->lineHeight;
	((window_def*)w)->char_width=fInfo.widMax;
	TEp->viewRect.bottom = TEp->viewRect.top + TEp->lineHeight*((window_def*)w)->linesInWindow;
#ifdef HSCROLL
	if(((window_def*)w)->flags & HSCROLL)
		TEp->destRect.right = TEp->destRect.left + MaxDocWidth;
	else{
		TEp->destRect.right = TEp->viewRect.right;
		SetCursor(&waitCursor);
		TECalText(((window_def*)w)->TEH); /* anche in caso di creazione, perch� lo 
				faccio PRIMA di caricare il file, il buffer � vuoto... */
		SetCursor(&arrow);
		}
#else
	TEp->destRect.right = TEp->viewRect.right;
	TECalText(((window_def*)w)->TEH);
#endif
}

void UpdateWindow(theWindow)
register WindowPtr	theWindow;
{
	GrafPtr	savePort;

	GetPort( &savePort );
	SetPort( theWindow );
	BeginUpdate( theWindow );
	EraseRect(&theWindow->portRect);
	DrawControls( theWindow );
	DrawGrowIcon( theWindow );
	TEUpdate( &theWindow->portRect, ((window_def*)theWindow)->TEH );
	#ifdef SUNTAR
	if(theWindow==(WindowPtr)&my_windows[0])
		aggiorna_nome_aperto();
	#endif
	EndUpdate( theWindow );
	SetPort( savePort );
}


static pascal void ScrollProc(theControl, theCode)
ControlHandle	theControl;
short				theCode;
{
	short	scrollAmt;

	switch (theCode) {
		case 0:
			return;
		case inUpButton: 
			scrollAmt = -1;
			break;
		case inDownButton: 
			scrollAmt = 1;
			break;
		case inPageDown:
		case inPageUp: 
			if(theControl==curr_window->vScroll){
				scrollAmt =  ((**TEH).viewRect.bottom-(**TEH).viewRect.top) / 
					(**TEH).lineHeight - 1;
				if(!scrollAmt) scrollAmt=1;	/* with the minimum size window and the
						maximum size font it may happen */
				}
			else	/* hScroll */
				scrollAmt = ((**TEH).viewRect.right-(**TEH).viewRect.left) /
					curr_window->char_width -1;
			if(theCode==inPageUp) scrollAmt = -scrollAmt;
			break;
		}

	SetCtlValue( theControl, GetCtlValue(theControl)+scrollAmt );
	AdjustText(curr_window);

}


void DoContent(theWindow, theEvent)
WindowPtr	theWindow;
EventRecord	*theEvent;
{
	short				cntlCode;
	ControlHandle 	theControl;
	short				pageSize;
	GrafPtr			savePort;
	
	GetPort(&savePort);
	SetPort(theWindow);
	GlobalToLocal( &theEvent->where );
	if (PtInRect( theEvent->where, &(**TEH).viewRect )){
		TEClick( theEvent->where, (theEvent->modifiers & shiftKey )!=0, TEH);
		/* senza autoscroll il TEclick non pu� avere provocato scroll e quindi 
		il punto di selezione � ancora in vista, con autoscroll ci pensa quello
		a portarlo in vista comunque, per� la posizione pu� 
		essere cambiata per cui su qualunque ROM: */

		MaintainScrollBars(curr_window);
		}
	else if ((cntlCode = FindControl(theEvent->where, theWindow, &theControl)) != 0) {
		if(cntlCode == inThumb) {
			TrackControl(theControl, theEvent->where, 0L);
			AdjustText(curr_window);
			}
		else
			TrackControl(theControl, theEvent->where, &ScrollProc);
		}
	SetPort(savePort);
}

void MyGrowWindow( w, p )
WindowPtr w;
Point p;
{
	GrafPtr	savePort;
	long	theResult;
	Rect 	r;
	/* in modalit� autowrap e per file lunghi questa routine e MyZoomWindow sono lente, 
	ma visto che cos� non � in modalit� scroll, suppongo che la colpa sia del ricalcolo 
	degli inizi di riga per cui non c'� niente da fare */

	GetPort( &savePort );
	SetPort( w );

	SetRect(&r, 210, 90, screenBits.bounds.right, screenBits.bounds.bottom);
	theResult = GrowWindow( w, p, &r );
	if (theResult != 0){
		EraseRect(&w->portRect);
		SizeWindow( w, loword(theResult), hiword(theResult), 1);
		HidePen();
		resize_controls(w);
		resize_text(w);
		AdjustText(curr_window);
		ShowPen();
		}
	SetPort( savePort );
}

void myZoomWindow(w,where,code)
register WindowPtr w;
Point where;
short code;
{
	GrafPtr saveport;
	GetPort (&saveport);
	SetPort (w);
	if ( TrackBox(w, where, code)) {
		EraseRect(&w->portRect);
		ZoomWindow (w, code, 0);
		HidePen();
		resize_controls(w);
		resize_text(w);
		AdjustText(w);
		ShowPen();
	}
	SetPort (saveport);
}


static void resize_controls(w)
register WindowPtr w;
{
	MoveControl(((window_def*)w)->vScroll, w->portRect.right - SBarWidth, 
			w->portRect.top-1);
	SizeControl(((window_def*)w)->vScroll, SBarWidth+1, 
			w->portRect.bottom - w->portRect.top-(SBarWidth-2));
#ifdef HSCROLL
	if(((window_def*)w)->flags & HSCROLL){
		short hsize=w->portRect.right - w->portRect.left-(SBarWidth-2) - ROOM_BEFORE_HSCROLL;
		MoveControl(((window_def*)w)->hScroll, w->portRect.left + ROOM_BEFORE_HSCROLL - 1, 
			w->portRect.bottom-SBarWidth);
		SizeControl(((window_def*)w)->hScroll, hsize, SBarWidth+1);
		}
#endif
}


static void resize_text(w)
register WindowPtr w;
{
	short oldbottomline = GetCtlValue(((window_def*)w)->vScroll);
	short oldtopchar = (*((window_def*)w)->TEH)->lineStarts [ oldbottomline ];

	InvalRect(&w->portRect);
	SetView(w);
	if(!(((window_def*)w)->flags&HSCROLL) )
		testo_in_abbondanza(((window_def*)w)->TEH,&oldtopchar);
	SetVScrollMax(w);

#ifdef HSCROLL
	if(((window_def*)w)->flags & HSCROLL){
		SetCtlValue(((window_def*)w)->vScroll,oldbottomline);
		return;
		}
#endif
	/* in modalit� autowrap cambiare dimensioni cambia anche la
	suddivisione in righe, bisogna verificare che pi� o meno quello che viene
	mostrato nella finestra sia sempre lo stesso pezzo di file */
	SetCtlValue( ((window_def*)w)->vScroll, find_new_position(w,oldtopchar) );

}

static void testo_in_abbondanza(myTEH,oldtopchar)
/* the max number of chars that the window may hold may change when
resizing it, changing font or going to autowrap, delete excess text
*/
register TEHandle myTEH;
short *oldtopchar;
{
short olds;
if( (olds=(**myTEH).nLines - 32000/(**myTEH).lineHeight )>0 ){
	short olde,charsToDelete;
	charsToDelete= (**myTEH).lineStarts[olds];
	/*print_number("chars=",charsToDelete);
	print_number("lines=",olds); */
	if((*oldtopchar -= charsToDelete)<0) *oldtopchar=0;
	if((olds=(**myTEH).selStart-charsToDelete)<0) olds=0;
	if((olde=(**myTEH).selEnd-charsToDelete)<0) olde=0;
	TESetSelect(0L, (long) charsToDelete, myTEH);
	TEDelete (myTEH);
	TESetSelect((long)olds, (long) olde, myTEH);
	}
}

short find_new_position(w,oldtopchar)
/* in TeachText and in other editors, many operations change the portion
of text which is visible in the window. With this routine, the window
is scrolled so that the first character on top bottom remains the first
character on top bottom, it that's possible, even after a resize or a
change of font or size.
By G. Speranza: it existed since the first 1.1 alfa version of suntar, but
a silly last-minute bug destroyed its work in the shipping 1.1
*/
window_def* w;
register short oldtopchar;
{
	register short i=0;
	register short *p= (**((window_def*)w)->TEH).lineStarts;
	while(*p++ < oldtopchar) i++;
	return i;
}

void CloseMyWindow()
/* to be called for clicks in the close box */
{
	curr_window->used=false;
	TEDispose(TEH);
	CloseWindow(curr_window);
	curr_window=NULL;
	n_currently_open--;
}


/******************************************************************************
 PositionDialog		(from tinyEdit.c, another demo editor from Symantec, 
 					but modified to center with respect the the area below the
 					menu bar, I think that's what Apple meant in its guidelines )
 
		Center the bounding box of a dialog or alert in the upper third
		of the screen.  This is the preferred location according to the
		Human Interface Guidelines.
		Modificato: non ho ben capito come fa quello della Apple
		(utilities.c) ma ho la sensazione che faccia cos�, lascia fuori la
		men� bar dal conto
 ******************************************************************************/

void	PositionDialog(theRect)
Rect *theRect;
{
short left,top;

				/* Center horizontally on screen	*/
	left = (screenBits.bounds.right - (theRect->right - theRect->left)) / 2;

				/* Leave twice as much space below	*/
				/*   as above the rectangle		*/	
	top = MBARHEIGHT +
	 (screenBits.bounds.bottom - MBARHEIGHT - (theRect->bottom - theRect->top)) / 3;

	theRect->right += left - theRect->left;
	theRect->left = left;
	theRect->bottom += top - theRect->top;
	theRect->top = top;
}
