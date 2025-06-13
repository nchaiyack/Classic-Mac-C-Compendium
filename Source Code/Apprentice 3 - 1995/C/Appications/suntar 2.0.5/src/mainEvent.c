/* this file contains code from two TextEdit-based little editors: */

/*********************************************************************

	MiniEdit.c

	The sample application from Inside Macintosh (RoadMap p.
	15-17)
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
#	Copyright © 1989 Apple Computer, Inc.
#	All rights reserved.
#
------------------------------------------------------------------------------*/

/* the integration of the two environments (MiniEdit is autowrap-only, TESample
is horizontal scroll only, both are single-window and their programming style is 
often radically different) is by us: */

/*******************************************************************************\

main events module

suntar, ©1991-95 Sauro & Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/

/* In general, we have kept the general structure of MiniEdit (which is easier to
understand) but inserting many small excerpts of code from TESample (which is more
powerful: for example, it's MultiFinder aware). Many features, however, are
completely new for suntar and come from neither MiniEdit nor TESample (e.g. DoFont
and SilentSuppression, and maintaining the current position, even in autowrap mode,
after a font resize).
*/

#include "antiglue.h"
#include "system7.h"

/* inutili, sono in MacHeaders
#include <QuickDraw.h>
#include <MacTypes.h>
#include <FontMgr.h>
#include <WindowMgr.h>
#include <MenuMgr.h>
#include <TextEdit.h>
#include <DialogMgr.h>
#include <EventMgr.h>
#include <DeskMgr.h>
#include <FileMgr.h>
#include <ToolboxUtil.h>
#include <ControlMgr.h>
*/


#include "windows.h"
#include "suntar.h"


#define ForwardDelChar 127
#define HomeKey 1
#define EndKey 4
#define PageUp 11
#define PageDown 12

void (*my_add_menus)(void) =NULL;	/* aggiunta menù dopo il menù preferences 
									--add items in the preferences menu and further menus
									*/
void (*my_handle_menus)(long)=NULL;		/* gestione nuove entry in qualunque menù 
									-- handles selection of any application-installed menu item
									*/
void (*my_event_filter)(EventRecord*)=NULL;	/* permette di "filtrare" un evento, dopo che
				MainEvent ha chiamato GetNextEvent e prima che lo
				gestisca, basta settare il campo what a nullEvent per 
				sopprimere la gestione standard
				-- event filter !
				*/
short (*my_at_exit)(void);		/* azioni da compiere al Quit 
								--quit handler
								*/
long max_sleep_time=-1;

short quit_selection,n_about_entries;
unsigned char PNS[]="\p";
Point badmount_point={80,112};
short command_modifiers;

window_def		my_windows[n_max_windows];	/* si potrebbe anche renderla una lista
											per non avere un valore massimo... ma mi fa comodo
											così per decidere dove posizionare le finestre */
short			n_currently_open=0;
window_def		*curr_window=NULL;	/* la finestra attualmente corrente 
									-- if the currently active window belongs to this module,
									curr_window must point to it (and the current grafport
									is set to it)
									*/
TEHandle		TEH;			/* della finestra corrente...
								-- copy of curr_window->TEH */

WindowPtr foreign_window[n_max_foreign];
upd_proc update_handler[n_max_foreign],activate_handler[n_max_foreign];

MenuHandle		myMenus[4];
MenuHandle		fontM[3];
#ifdef SUNTAR
MenuHandle		hqxM;
#endif
Cursor			editCursor;
Cursor			waitCursor;
struct acur{
	short n_cursors;
	short delay;
	Ptr cursor_ID_or_Ptr[1000];	/* the true size depends on the size of the resource */
	} *rot_cursor;

extern short current_font,current_size,current_style,curr_font_index;

/* #define	kNoEvents				0		/ * no events mask */
/* GMac is used to hold the result of a SysEnvirons call. This makes
   it convenient for any routine to check the environment. It is
   global information, anyway. */
SysEnvRec	gMac;				/* set up by Initialize */

/* GHasWaitNextEvent is set at startup, and tells whether the WaitNextEvent
   trap is available. If it is false, we know that we must call GetNextEvent. */
Boolean		gHasWaitNextEvent;	/* set up by Initialize */
/* obviously keeping here all these declarations is not portable, since the
list of which traps are needed depend on the rest of the application: I should
perform a call to TrapAvailable every time I need the information, but... */
#ifdef SUNTAR
Boolean		gHasResolveAlias;
Boolean		gHasFindFolder;
Boolean		gHasCustomPutFile;
Boolean		gHasVirtualMemory;
Boolean		gHasSCSIManager;
#endif
Boolean	gHasKeyTrans;

/* gInBackground is maintained by our OSEvent handling routines. Any part of
   the program can check it to find out if it is currently in the background. */
Boolean		gInBackground=false;		/* maintained by Initialize and DoEvent */


void set_marks_size(void);
void set_marks_style(void);
void DoFile(short);
unsigned long GetSleep(void);
void DoFont(long);
void MaintainCursor(Point);
void MaintainMenus(void);


void install_handlers(window,upd_handler,act_handler)
WindowPtr window;
upd_proc upd_handler;
upd_proc act_handler;
{
short i=0;
while(i<n_max_foreign){
	 if(foreign_window[i]==NULL){
	 	foreign_window[i]=window;
	 	update_handler[i]=upd_handler;
	 	activate_handler[i]=act_handler;
	 	return;
	 	}
	 i++;
	 }
}

void remove_handlers(window)
WindowPtr window;
{
short i=0;
while(i<n_max_foreign){
	 if(foreign_window[i]==window){
	 	foreign_window[i]=NULL;
	 	return;
	 	}
	 i++;
	 }
}

void UpdateFilter (theEvent)
register EventRecord *theEvent;
/* to be called exclusively for update & activate events */
{
/* It's annoying to be obliged to handle update and activate events; furthermore,
according to TN 304 update events should never be ignored, even during a ModalDialog
(which ignores them if they are not for the dialog itself, unless you specify a 
filter which handles them).
 Hence, when suntar began to have a number of windows and dialogs, I preferred
to do things in a way that is complex but surely works in any situation,
centralizing the handling of updates and activates: when calling ModalDialog one
must use this filter, when calling MainEvent or get_event updates and activates
are NOT returned to the caller, but are handled internally (just as happens to
updates when one installs a picture in windowPic, but probably nobody has ever
used that method for a dialog).

It's always a good programming rule to rewrite something when the program grows
and the old, simple method begins to be inadequate for the job.
*/
if(ourTE(theEvent->message)){
	if(theEvent->what==updateEvt)
		UpdateWindow((WindowPtr)theEvent->message);
	else	/* activate */
		DoActivate((WindowPtr)theEvent->message,theEvent->modifiers & activeFlag );
	theEvent->what=nullEvent;
	}
else if(theEvent->message){
	short entry_n;
	for(entry_n=0;entry_n<n_max_foreign;entry_n++){
		if((WindowPtr)theEvent->message==foreign_window[entry_n]){
			if(theEvent->what==updateEvt){
				if(update_handler[entry_n])
					(*update_handler[entry_n])(theEvent);
				}
			else{
				if(activate_handler[entry_n])
					(*activate_handler[entry_n])(theEvent);
				}
			theEvent->what=nullEvent;
			break;
			}
		}
	}
return;
}


void MainEvent()
/* see the comment at the beginning of suntar.c for understanding this routine 
and the two following ones: suntar has NOT a main event loop... */
{
	EventRecord		myEvent;
	if(get_event(&myEvent)){
		if(my_event_filter!=NULL) (*my_event_filter)(&myEvent);
		handle_event(&myEvent);
		}
}


Boolean get_event(myEvent)
EventRecord		*myEvent;
{
	Boolean retcode;
	if(curr_window)
		TEIdle(curr_window->TEH);


	#ifdef SUNTAR
	#define MY_MASK (everyEvent& ~diskMask)
	if( ! (retcode=GetOSEvent (diskMask,myEvent)) ){	/* otherwise the disk is mounted... */
	#else
	#define MY_MASK everyEvent
	/* window 0 is the console window, and old text must be deleted if the
	TextEdit limit of 32 kbytes is approached...; in suntar it's not useful since
	this instruction is located in printf, and only printf may increase the size
	of the textedit buffer */
	if( ((**my_windows[0].TEH).teLength >maxTElength-4000 ||
		/* according to TN 237, there is another limit to the size of a TextEdit
		record: the destination rect must not be taller than 32767 pixels: 
		that limit is more stringent only if the line height in pixels is bigger
		than the average line length in characters, a very rare situation,
		unless you set the font size to 36 or 48, and use a small autowrap window  */
	    (**my_windows[0].TEH).nLines > 32000/(**my_windows[0].TEH).lineHeight) )
			SilentSuppression(50);
	#endif
		/* use WNE if it is available */
		if ( gHasWaitNextEvent ) {
			long delay=GetSleep();
			/*AdjustCursor(mouse, cursorRgn);*/
			if(delay==-1){
				OSEventAvail(kNoEvents, myEvent);	/* we aren't interested in any events */
													/* just the mouse position */
				MaintainCursor(myEvent->where);
				}
			retcode= WaitNextEvent(MY_MASK, myEvent,
				delay, NULL/*cursorRgn*/);
				/* in suntar disk insertions are masked, but
				WaitNextEvent returns, so that the event may be caught by the
				following GetOSEvent call above, hence the disk is mounted only if 
				it's a Mac disk */
			}
		else {
			SystemTask();
			retcode= GetNextEvent(MY_MASK, myEvent);
			}
		#ifdef SUNTAR
		}
		#endif
	MaintainCursor(myEvent->where);
	if(retcode && (myEvent->what==updateEvt || myEvent->what==activateEvt) ){
		UpdateFilter(myEvent);
		return myEvent->what!=nullEvent;
		}
	if(retcode && myEvent->what==keyDown && (char)myEvent->message==16){
		/* F1-F15 keys in the extended keyboard */
		char keycode=((short)myEvent->message>>8)&0x7F;
		if(keycode==0x7A){	/* F1 is Undo */
			lowbyte(loword(myEvent->message))='Z';
			myEvent->modifiers|=cmdKey;
			}
		else if(keycode==0x78){	/* F2 is Cut */
			lowbyte(loword(myEvent->message))='X';
			myEvent->modifiers|=cmdKey;
			}
		else if(keycode==0x63){	/* F3 is Copy */
			lowbyte(loword(myEvent->message))='C';
			myEvent->modifiers|=cmdKey;
			}
		else if(keycode==0x76){	/* F4 is Paste */
			lowbyte(loword(myEvent->message))='V';
			myEvent->modifiers|=cmdKey;
			}
		}

	return retcode;
}

/*int print_event=false;*/
/*int lastEvent,lastCode;*/


void handle_event(myEvent)
	register EventRecord	*myEvent;
{
	WindowPtr		whichWindow;
/*	RgnHandle		cursorRgn;*/
	short			code;

/*if(print_event) printf("event=%x %d %d\n",myEvent->what,myEvent->where.v,myEvent->where.h);*/
/*printf("event=%d\n",myEvent->what);*/
/*print_number("event=",(short)myEvent->what);*/
/*lastEvent=myEvent->what;*/

	switch (myEvent->what) {
	case mouseDown:
		code=FindWindow( myEvent->where, &whichWindow );
/*lastCode=code;*/
/*if(code!=1)printf("place=%d\n",code);*/
		switch (code) {
		case inDesk:
			SysBeep(5);
			break;
		case inMenuBar:
			{long choice;
			MaintainMenus();
			command_modifiers=myEvent->modifiers;
			choice=MenuSelect(myEvent->where);
			if(hiword(choice) && !(command_modifiers&optionKey) ){
				/* accept "option" even if it's pressed only at the end
				of the menu selection, I always hated to be obliged to
				press option before pulling down the menu */
				OSEventAvail(kNoEvents, myEvent);	/* we aren't interested in any events */
													/* just the modifier keys */
				command_modifiers |= myEvent->modifiers;
				}
			DoCommand( choice);
			}
			break;
		case inSysWindow:
			SystemClick( myEvent, whichWindow );
			break;
		case inDrag:
			{
			Rect	dragRect;
			dragRect.left=0;
			dragRect.top=MBARHEIGHT;
			dragRect.right=screenBits.bounds.right;
			dragRect.bottom=screenBits.bounds.bottom;
			InsetRect(&dragRect,4,4);
			DragWindow( whichWindow, myEvent->where, &dragRect );
			}
			break;
		case inGrow:
			if (curr_window)
				MyGrowWindow( whichWindow, myEvent->where );
			break;
		case inZoomOut:
		case inZoomIn:
			if(curr_window)
				myZoomWindow(whichWindow,myEvent->where,code);
			break;
		case inGoAway:
			if (curr_window)
				if (TrackGoAway( whichWindow, myEvent->where) )
					DoFile(opClose);
			break;
		case inContent:
			if (whichWindow != FrontWindow()){
				SelectWindow(whichWindow);
				}
			else
				if (curr_window)
					DoContent(whichWindow, myEvent);
			break;
		default: ;
		} /* end switch FindWindow */
		break;
	case autoKey:
		if((myEvent->modifiers & cmdKey) != 0)	/*niente autorepeat sui comandi!
					--no autorepeat on commands ! */
			break;	/* else prosegui in sequenza...
					-- else behave as a keyDown */
	case keyDown:
		/*if((myEvent->modifiers & cmdKey) && (char)myEvent->message==',')
			printf(something I need to know); */
		DoKeyDown( myEvent->message, myEvent->modifiers );
		break;
	/* moved to UpdateFilter
	case activateEvt:
		if (ourTE((WindowPtr)myEvent->message)){
			SetPort((WindowPtr)myEvent->message);
			DoActivate((WindowPtr)myEvent->message,myEvent->modifiers & activeFlag );
			}
		else{
			if (myEvent->modifiers & activeFlag )
				SetPort((WindowPtr)myEvent->message);
			}
		break;
		*/
	case kOSEvent:	/* eventi del MultiFinder */
		{
		WindowPtr w;
		switch ((myEvent->message >> 24) & 0x0FF) {		/* high byte of message */
			case kMouseMovedMessage:
				/*MaintainCursor(myevent->where);*/			/* mouse-moved is an idle event */
				break;
			case kSuspendResumeMessage:
				gInBackground = (myEvent->message & kResumeMask) == 0;
				w=FrontWindow();

				myEvent->message=(long)w;
				myEvent->what=activateEvt;		/* suspend/resume is also an activate/deactivate */
				myEvent->modifiers = !gInBackground ? activeFlag : 0;
				UpdateFilter(myEvent);		/* my handler of activate events */
#ifdef SUNTAR
				if(gInBackground){
#if 0 /* abolito nella 2.0, che controlla se il disco è cambiato */
					if( going_to_background() && ourTE(w))
						HiliteWindow (w,false);		/* going_to_background does some
								dirty things which may leave the window incorrectly 
								highlighted */
#else
					void touch_if_needed(void);
					flush_all();	/* if the next foreground application is not
						MultiFinder aware, a lot of time (even hours) may pass before
						I have another chance to do that */
					touch_if_needed();
#endif
					SetCursor(&arrow);	/* some applications do not set the cursor
							when they go to foreground... */
					MainEvent();	/* per andarci subito in background... non 
						posso fare aspettare la prossima chiamata 
						--immediately go to background: in suntar going_to_background
						may do a delay, I can't return to a routine which is going to do
						further work before calling MainEvent again
						when MultiFinder is waiting to switch me to background
						*/
					return;
					}
#endif
				break;
		}}
		break;
	/* moved to UpdateFilter
	case updateEvt: 
		if (ourTE((WindowPtr)myEvent->message)) 
			UpdateWindow((WindowPtr)myEvent->message);
		break;
	*/
	case diskEvt:
	/*	It is not a bad idea to at least call DIBadMount in response
		to a diskEvt, so that the user can format a floppy. */
		#ifndef SUNTAR
		if ( hiword(myEvent->message) != noErr )
			DIBadMount(badmount_point, myEvent->message);
		#else
		unexpected_disk_insertion(myEvent->message);
		#endif

		break;
	default: ;
	} /* end of case myEvent->what */
}

/* gestione tasti */

void DoKeyDown( message, modifiers )
long message;
short modifiers;
{
register unsigned char theChar=(unsigned char) message;


#if 0
/* per scoprire il mappaggio dei tasti tastiera estesa */
if(theChar==3){
		Handle h=IUGetIntl (0);
		if(h!=NULL){
			short i=((Intl0Rec*)*h)->intl0Vers>>8;
			ReleaseResource(h);
			h=GetResource('KCHR',i);	/* get the keyboard translation map for
					the language of the currently open System File */
			if(h!=NULL){
				long l=0; int i;
				unsigned char codici[]={0x72,0x75,0x73,0x77,0x74,0x79,0x7A,0x78,0x63,0x76};
				SignedByte oldflags=HGetState(h);
				HLock(h);
				for(i=0;i<sizeof(codici);i++){
					l=0;
					theChar=(unsigned char)KeyTrans(*h,
						codici[i],&l);
					printf("da %x a %d\n",codici[i],theChar);
					}
				HSetState(h,oldflags);
				}
			}
		}
#endif

if (modifiers & cmdKey) {
	long mnk;
	MaintainMenus();

	if((modifiers & optionKey) && gHasKeyTrans){
		/* sometimes the option key is considered as a modifier of the corresponding
		menu command, however the ASCII char in the Event message uses option
		to alter the keymapping: e.g. on my keyboard command-option-A
		does not yield 'A' +the modifiers bits command&option, but 'Å', which is not
		recognized by MenuKey. Hence I must discover which ASCII key would have
		been returned if the option key had not been pressed, and that's not easy ! */
#ifdef THINK_C_5
#define Intl0Vers intl0Vers
#endif
		Handle h=IUGetIntl (0);
		if(h!=NULL){
			short i=((Intl0Rec*)*h)->Intl0Vers>>8;
			ReleaseResource(h);
			h=GetResource('KCHR',i);	/* get the keyboard translation map for
					the language of the currently open System File */
			if(h!=NULL){
				long l=0;
				SignedByte oldflags=HGetState(h);
				HLock(h);
				theChar=(unsigned char)KeyTrans(*h,
					(((short)message>>8)&0x7F) | (modifiers&(0xFF00&~optionKey)),&l);
				HSetState(h,oldflags);
				}
			}
		}

	mnk=MenuKey( theChar );
	if(hiword(mnk) == 0){ /* non esiste un tale comando, o non è abilitato...
					--command not existing or not enabled
					*/
		SysBeep(5);
		FlushEvents( keyDownMask|autoKeyMask, 0 );
		}
	else{
		if(curr_window) SetPort(curr_window);
		command_modifiers=modifiers;
		DoCommand( mnk );
		}
	return;
	}
else if (curr_window){

	SetPort(curr_window);
	if (theChar== enter_key) /* il TextEdit non lo gestisce
				--TextEdit does not know that enter is a "return" char
				 */
		theChar=CR;
	if( theChar >= 28 &&theChar <= 31){	/* è un tasto freccia
							--a arrow key
							*/
		TEKey( theChar, TEH );
		}
	else if	( theChar == DelChar) /* questo è gestito correttamente, ma per poter 
								poi aggiungere al programma l'undo bisogna gestirlo 
								direttamente, e la gestione per console è diversa... */
		if( curr_window->flags&READONLY && ((**TEH).selStart<curr_window->lastPrompt 
			|| (**TEH).selStart==curr_window->lastPrompt &&(**TEH).selStart==(**TEH).selEnd))
			SysBeep(5);
		else{
			TEKey( theChar, TEH );
			curr_window->dirty = 1;
			}
	else if	( theChar == ForwardDelChar)
		if( curr_window->flags&READONLY && (**TEH).selStart<curr_window->lastPrompt) 
			SysBeep(5);
		else{
			if( (**TEH).selStart==(**TEH).selEnd){
				if((**TEH).selEnd==(**TEH).teLength)
					; /* cursor at end of text */
				else{
					(**TEH).selEnd++;
					TEDelete(TEH);
					curr_window->dirty = 1;
					}
				}
			else{
				TEDelete(TEH);
				curr_window->dirty = 1;
				}
			}
	else if(theChar==PageUp || theChar==PageDown){
		/* see ScrollProc in windows.c */
		short scrollAmt =  ((**TEH).viewRect.bottom-(**TEH).viewRect.top) / 
					(**TEH).lineHeight - 1;
		if(!scrollAmt) scrollAmt=1;
		if(theChar==PageUp) scrollAmt = -scrollAmt;
		SetCtlValue( curr_window->vScroll, GetCtlValue(curr_window->vScroll)+scrollAmt );
		AdjustText(curr_window);
		}
	else if(theChar==HomeKey || theChar==EndKey){
		/* see ScrollProc in windows.c */
		short scrollPos=0;
		if(theChar==EndKey) scrollPos = GetCtlMax(curr_window->vScroll);
		SetCtlValue( curr_window->vScroll, scrollPos );
		AdjustText(curr_window);
		}
#ifndef AUTOSELECT
	else if( curr_window->flags&READONLY && (**TEH).selStart<curr_window->lastPrompt){
		SysBeep(5);
		FlushEvents( keyDownMask|autoKeyMask, 0 );	/* è facile battere più caratteri 
									insieme, e il beep dà tempo per accumularli...*/
		}
#endif
	else if ( curr_window->flags&READONLY && curr_window->lastPrompt == 32767){
	/* a console window completely read-only */
		SysBeep(5);
		FlushEvents( keyDownMask|autoKeyMask, 0 );
		}
	else if((*TEH)->teLength - ((*TEH)->selEnd - (*TEH)->selStart) + 1 < maxTElength ){
		/* a console window with a read-only part on top and a read/write part
		on bottom */
		if(theChar==CR&&(curr_window->flags&CONSOLE)){
			TESetSelect (32767L ,32767L, TEH);

			TEKey( CR, TEH );
			ShowCursor(); /* According to Apple, the mouse cursor should disappear when
				one types, and reappear when one moves the mouse: hence, TEKey
				calls ObscureCursor. Now I'm changing mode, and the cursor must
				reappear: note that ShowCursor does nothing if the cursor is already
				visible */
			curr_window->lastPrompt=32767;	/* questo è l'unico segno visibile dall'esterno 
											che il return c'è stato
											-- it's the only state change remembering
											that there was a carriage return */
			}
		else{
		/* non so se sia il caso di gestire a parte anche altri caratteri, tipo l'escape */
#ifdef AUTOSELECT
			if( curr_window->flags&READONLY && (**TEH).selStart<curr_window->lastPrompt){
				if((**TEH).selStart>=curr_window->lastPrompt-2)	/* it it's out for only
							a couple chars, simply move the selection so that it's in */
					TESetSelect((long)curr_window->lastPrompt,(**TEH).selEnd,TEH);
				else
					TESetSelect(32767L,32767L,TEH);
				}
#endif
			TEKey( theChar, TEH );
			curr_window->dirty = 1;
			}
		}
	else{	/* buffer pieno ! */
		SysBeep(5);
		ParamText("\pNo more room in window",PNS,PNS,PNS);
		my_alert();
		FlushEvents( keyDownMask|autoKeyMask, 0 );
		}
	SCR_BAR_AND_TEXT_IN_SYNC
	}
}


void DoActivate(w,active_flag)
register window_def* w;
short active_flag;
{
GrafPtr	savePort;
if (!active_flag ) GetPort( &savePort );
SetPort(w);		/* pare che non guasti, per proteggersi da DA e (sotto
				MultiFinder) altri programmi che si comportino male; e poi,
				tutto il modulo presume che se curr_window è != NULL allora
				la porta corrente è lei
				-- in this module, all routines think that if curr_window is
				not 0, then the current port is curr_window */
if (active_flag ) {
	Rect r;
	curr_window=w;
	TEH=w->TEH;
	TEActivate( w->TEH );
	if(curr_window==&my_windows[0]){
		/* posso averci fatto delle print, col che posizione e dimensioni
		complessive sono cambiate
		-- if printf has added text while the window was not in foreground,
		the scrollbar was not updated to reflect the current state
		*/
		SCR_BAR_AND_TEXT_IN_SYNC
		}
	ShowControl( w->vScroll );
#ifdef HSCROLL
	if( w->flags&HSCROLL) ShowControl( w->hScroll);
#endif
	TEFromScrap();
	/* the growbox needs to be redrawn on activation: */
	r=((WindowPtr)w)->portRect;
	r.top = r.bottom - (SBarWidth+1);
	r.left = r.right - (SBarWidth+1);
	InvalRect(&r);		
	}
else {
	ZeroScrap();
	TEToScrap();
	TEDeactivate(w->TEH);
	HideControl( w->vScroll );
#ifdef HSCROLL
	if( w->flags&HSCROLL) HideControl( w->hScroll );
#endif
	DrawGrowIcon(w);
	curr_window=NULL;
	SetPort( savePort );
	}
}



void set_marks_size()
{
short i,size;
char sizestr[16];
for(i=CountMItems(fontM[1]);i>0;i--){
	GetItem(fontM[1],i,sizestr);
	if(RealFont(current_font, (size=pstrtoi(sizestr))) )
		SetItemStyle(fontM[1],i,outline);
	else
		SetItemStyle(fontM[1],i,0);
	CheckItem (fontM[1], i, size==current_size );
	}
}

void set_marks_style()
{
short i,mask;
CheckItem (fontM[2], 1, current_style==0 );
mask=1;
for(i=2;i<=6;i++){
	CheckItem (fontM[2], i, (current_style&mask) !=0 );
	mask <<=1;
	}
}

void DoFont( mResult)
long mResult;
{
	Str255	fontName;

	switch (hiword(mResult)) {
	case fontID:
		CheckItem (fontM[0], curr_font_index, false );
		SetItemMark (fontM[0], curr_font_index=loword( mResult ), '•');
		GetItem(fontM[0],curr_font_index,&fontName);
		GetFNum(fontName,&current_font);
		set_marks_size();	/* è cambiato l'essere real font o no
							-- "real font" sizes are represented differently,
							hence I must recompute their state */
		break;
	case sizeID:
		{char sizestr[16];
		GetItem(fontM[1],loword( mResult ),sizestr);
		current_size= pstrtoi(sizestr);
		}
		set_marks_size();
		break;
	case styleID:
		if(loword( mResult )==1)
			current_style=0;
		else
			current_style ^= (1<< (loword( mResult )-2)) ;
		set_marks_style();
		break;
	}
}

void DoCommand( mResult )
long mResult;
{
#ifdef SUNTAR
void toggleLog(void);
#endif
	short		theItem, temp;

	theItem = loword( mResult );
	switch (hiword(mResult)) {
	case appleID:
		if(theItem<=n_about_entries)
			my_handle_menus(mResult);
		else
			{Str255	name;
			GrafPtr savePort;

			GetPort(&savePort);
			GetItem(myMenus[appleM], theItem, &name);
			OpenDeskAcc( &name );
			SetPort( savePort );
			}
		break;
	case fileID: 
		if(theItem==quit_selection){
			if(my_at_exit!=NULL)
				if( !(*my_at_exit)()) break;	/* quit cancelled */
			DoFile(opQuit);
			HiliteMenu(0);
			FlushEvents( everyEvent&~diskMask, 0 );

			ExitToShell();
			}
		else
			(*my_handle_menus)(mResult);
		break;
	case editID:
/* la funzione MaintainMenus dovrebbe disabilitare i comandi di editing se non esiste 
una finestra corrente, ma per prudenza è meglio controllare sempre...
-- The commands are disabled when they are meaningless, but it's better to check
anyway
*/
		if (SystemEdit(theItem-1)==0) {
#ifndef AUTOSELECT
			if( curr_window==NULL || theItem!=copyCommand && (curr_window->flags&READONLY) &&
				(**TEH).selStart<curr_window->lastPrompt ){
				SysBeep(5);
				break;
				}
#else
			if( curr_window==NULL ||  theItem!=copyCommand && theItem!=pasteCommand && 
				(curr_window->flags&READONLY) && (**TEH).selStart<curr_window->lastPrompt ) {
				SysBeep(5);
				break;
				}
			if( theItem==pasteCommand && (curr_window->flags&READONLY) ){
			/* un paste in una console si comporta in modo complicato: a parte che 
			comunque non fa il paste di caratteri CR, accetta di essere eseguito 
			anche quando non si è dopo il prompt, portandocisi automaticamente, e se 
			è selezionato qualcosa fa un copy di esso, prché sia nella parte read only
			-- a paste to a console is handled in a complex way: it performs autocopy
			(if something was selected, copy it to the clipboard), then autoselect
			(if the selection point is in the read-only portion of the window, move
			it to the end), and it truncates long clipboards and refuses to paste
			carriage returns */
				if( curr_window->lastPrompt == 32767 ){
					/* è del tutto readonly, non console, quindi ho sbagliato a non dare errore */
					SysBeep(5);
					break;
					}
				if( (**TEH).selStart < curr_window->lastPrompt ){
					/* autocopy */
					if( (**TEH).selStart != (**TEH).selEnd && 
						(**TEH).selEnd < curr_window->lastPrompt)
							TECopy (TEH);
					TESetSelect(32767L,32767L,TEH);
					}
				}
#endif
			switch (theItem) {
			case cutCommand:
				TECut( TEH );
				curr_window->dirty = 1;
				break;
			case copyCommand:
				TECopy( TEH );
				break;
			case pasteCommand:
				{
				register short i=TEGetScrapLen();
				if(curr_window->flags&CONSOLE && i>300 ) i=300; /* troppo per una riga sola... */
				if ( i + ((*TEH)->teLength -
							((*TEH)->selEnd - (*TEH)->selStart)) > maxTElength ){
					ParamText("\pInsufficient space for pasting",PNS,PNS,PNS);
					my_alert();
					}
				else{
					if( curr_window->flags&CONSOLE){
				/* in modalità console, non ha senso il paste di un carriage return! */
						char **scrap= TEScrapHandle();
						register char *p,*q;
						TEDelete(TEH);
						HLock(scrap);	/*TEInsert vuole un Ptr...*/
						q= *scrap;
						while(i>0){
							p=q;
							while(i>0&&*q!=CR) q++,--i;
							if(p!=q)TEInsert(p,(long)(q-p),TEH);
							if(i>0){	/* e quindi *q==CR */
								TEKey(' ',TEH);
								q++;i--;
								}
							}
						HUnlock(scrap);

						/*for(n=0;n<i;n++)
							TEKey ( ((*scrap)[n]==CR) ? ' ' : (*scrap)[n] ,TEH); */
						}
					else
						TEPaste( TEH );
					curr_window->dirty = 1;
					}
				}
				break;
			case clearCommand:
				TEDelete( TEH );
				curr_window->dirty = 1;
				break;
			default: ;/* non esistono altri comandi abilitati*/
			}
			SCR_BAR_AND_TEXT_IN_SYNC
		}
		break;
	case prefID:
		switch(theItem){
		case pmAutowrap:
			{short markChar;
			GetItemMark (myMenus[prefM], pmAutowrap, &markChar);
			CheckItem (myMenus[prefM], pmAutowrap, !markChar);
			if(my_windows[0].used) apply_preferences(&my_windows[0]);
			}
			break;
		default:
			(*my_handle_menus)(mResult);
		}
		break;
	case fontID:
	case sizeID:
	case styleID:
		DoFont( mResult);
		if(my_windows[0].used) apply_preferences(&my_windows[0]);
		break;
#ifdef SUNTAR
	case hackID:
		if(theItem==hmCreateLog)
			toggleLog();
		else
			(*my_handle_menus)(mResult);
		break;
#endif
	default:
		(*my_handle_menus)(mResult);
	}
	HiliteMenu(0);
}

static void DoFile( operation )
short		operation;

{
	switch (operation) {
	case opClose:
		if(curr_window!=NULL)
			CloseMyWindow();
		break;
	case opQuit:
		ZeroScrap();
		TEToScrap();
		break;
	}
}


void MaintainCursor(pt)
Point		pt;
{
/* lo chiamo dopo GetNextEvent, col che lo so già dove sta il mouse (al più, l'evento
potrebbe essere un po' vecchio, se il programma ha delle situazioni in cui per lungo
tempo non riceve eventi, la posizione così determinata può essere diversa da quella corrente) */
static unsigned long next_rotation=0;
static short current_cursor=0;

	if(gInBackground) return;
	if(sto_lavorando){
		if(((WindowPeek)FrontWindow())->windowKind<0)
			/*SetCursor(&arrow) no, the desk accessory will take its decision,
				anyway the event which brought it to front caused a SetCursor(&arrow) */
				;
		else{
			unsigned long t=TickCount();
			if(t>next_rotation){
				next_rotation=t+rot_cursor->delay;
				if(++current_cursor>=rot_cursor->n_cursors) 
					current_cursor=0;
				}
			SetCursor( rot_cursor->cursor_ID_or_Ptr[current_cursor] );
			}
		}
	else{
		current_cursor=0;
		if (curr_window && (WindowPtr)FrontWindow()==(WindowPtr)curr_window) {
			
			SetPort(curr_window);
			GlobalToLocal(&pt);
			if ( PtInRect(pt, &(**TEH).viewRect ) && PtInRgn(pt, ((WindowPeek)curr_window)->port.visRgn) ){
				SetCursor( &editCursor);
				return;
				}
			}
		SetCursor( &arrow );
		}
}

void MaintainMenus()
{

/*
In presenza di console, readonly etc. le condizioni per cui i comandi del menù
Edit sono leciti diventano complicate, ho cercato di copiarle da DoCommand, con
l'idea che è meglio lasciarne abilitati di quelli che poi danno errore piuttosto
che disabilitarne di quelli che sono eseguibili
-- the conditions for allowing a command are complex, but since doCommands does
its own checks, I duplicated those tests and in case of difference it's
better to let the item enabled when it should be disabled then vice-versa
*/

WindowPtr w=FrontWindow();
Boolean selected,all_readonly,sel_readonly;

	if ( ! ourTE(w) ) {
		Boolean en=isDAwindow(w);
		en_dis_edit(undoCommand ,en);
		en_dis_edit(pasteCommand ,en);
		en_dis_edit(cutCommand ,en);
		en_dis_edit(copyCommand ,en);
		en_dis_edit(clearCommand ,en);
		}
	else {
		/*
		EnableItem(myMenus[prefM],pmAutowrap);
		EnableItem(myMenus[prefM],pmFont);
		EnableItem(myMenus[prefM],pmSize);
		EnableItem(myMenus[prefM],pmStyle);
		*/
		DisableItem( myMenus[editM], undoCommand );
		selected = (**TEH).selStart!=(**TEH).selEnd;
		en_dis_edit(copyCommand,selected);
		if(curr_window->flags&CONSOLE){
/* questo è pensato per una console con autoselect e autocopy */
			all_readonly= curr_window->lastPrompt==32767;
			sel_readonly= (**TEH).selStart<curr_window->lastPrompt;
			en_dis_edit(cutCommand, selected&&!sel_readonly);
			en_dis_edit(clearCommand, selected&&!sel_readonly);
			en_dis_edit(pasteCommand,!all_readonly&&(TEGetScrapLen()>0||selected));
			}
		else{
			en_dis_edit(cutCommand, selected);
			en_dis_edit(clearCommand, selected);
			en_dis_edit(pasteCommand,TEGetScrapLen()>0);
			}

	}
	#ifdef SUNTAR
	MaintainApplSpecificMenus();
	#endif
}

void en_dis_edit(item,flag)
short item;
Boolean flag;
{
if(flag)
	EnableItem(myMenus[editM], item);
else
	DisableItem(myMenus[editM], item);
}



short SilentSuppression (nlines)
short nlines;
/* questa funzione serve per implementare finestre di console: in quel caso, 
l'utente può tornare indietro a vedere che è successo nel passato ma la storia
passata non gli interesserà poi moltissimo, per cui se il buffer del TextEdit 
si riempie è molto meno dannoso per lui cancellargli a tradimento le cose più 
vecchie che dare errore.
La funzione cerca di cancellare le prime nlines righe, ma riduce il numero se 
le ultime sono visibili sullo schermo o fanno parte della selezione, in ogni
caso ritorna il numero di righe soppresse. Sullo schermo l'unico effetto 
visibile è uno spostamento del bottone della scrollbar verticale (la posizione 
relativa entro il buffer è cambiata).
-- Deletes the first nlines lines of the content of the window, without
letting the user suspect what's happening (the scroll bar button is the only
thing on the screen which may be modified)
*/
{
short olds,olde,deltax,deltay,oktodelete,charsToDelete,linesOutOfScreen;
Rect oldr;
GrafPtr savePort;
register TERec *TEp;

	TEp = *my_windows[0].TEH;
	linesOutOfScreen=(TEp->viewRect.top-TEp->destRect.top)/TEp->lineHeight;
	if(linesOutOfScreen<nlines) nlines=linesOutOfScreen;

	do{
		if(nlines<=0) return 0;
		charsToDelete= TEp->lineStarts[nlines];
		if( ! (oktodelete=TEp->selStart>charsToDelete) ) 
			nlines--;
		}
	while(!oktodelete);

	olds=TEp->selStart-charsToDelete;
	olde=TEp->selEnd-charsToDelete;
	oldr=TEp->destRect;

	GetPort( &savePort );
	SetPort( &my_windows[0]);
	HidePen();
	TESetSelect(0L, (long) charsToDelete, my_windows[0].TEH);
	TEDelete (my_windows[0].TEH);
	TESetSelect((long)olds, (long) olde, my_windows[0].TEH);
	TEp = *my_windows[0].TEH;
	deltax= oldr.left - TEp->destRect.left;
	deltay= oldr.top+nlines*TEp->lineHeight - TEp->destRect.top;
	if(deltax || deltay)
		TEScroll(deltax,deltay,my_windows[0].TEH);
	/* si potrebbe fare un ValidRect, ma allora bisogna sottrarci la vecchia regione
	di update e io questi giochi non li so ancora fare. Comunque, ho provato e il
	main loop non riceve alcun evento di update dopo il fatto, se qualcuno ritraccia
	allora è direttamente il TextEdit */
	ShowPen();
	MaintainScrollBars(&my_windows[0]);

	if(my_windows[0].lastPrompt!=32767)
		if((my_windows[0].lastPrompt -= charsToDelete) <0)
			my_windows[0].lastPrompt=0;
	SetPort(savePort);
	return nlines;
}


/*	Calculate a sleep value for WaitNextEvent. This takes into account the things
	that DoIdle does with idle time. */
#define MAXLONG 0xFFFFFFFF
unsigned long GetSleep()
{
	long		sleep;
	WindowPtr	window;
	TEHandle	te;

	if(sto_lavorando) return 0L;
	if ( gInBackground)
		sleep = MAXLONG;
	else if(!curr_window){
		#ifndef SUNTAR
		sleep = MAXLONG;
		#else
		sleep = 40;	/* under System 6, with the Finder and WaitNextEvent in ROM, I
					do NOT get a null event when an event arrives but it's masked.
					Since to be able to catch disk insertions by GetOSEvent and be
					MultiFinder aware it's essential to perform a GetOSEvent with
					little delay relative to the disk insertion, I must use a short
					sleep time even if nothing need be done (it would be better to
					call GetNextEvent, since the Finder can't exploit the free time,
					and under MultiFinder and System 7 the null event arrives,
					but ther is no way to know whether I'm under MultiFinder or not !)
					And I was told that the problem may happen under System 7 too,
					hence it's better to avoid risking, unless Apple officiallly
					declares that not returning that event is a bug and lists all the
					System versions containing that bug
				    */
		if(max_sleep_time!=-1 && max_sleep_time<sleep) sleep=max_sleep_time;
		#endif
		}
	else{
		if (curr_window && (*TEH)->selStart == (*TEH)->selEnd )
			sleep = GetCaretTime();		/* blink time for the insertion point */
		else
			sleep=60;	/* non uso le region per aggiornare la forma del cursore, 
				quindi ho bisogno di lavorare ogni tanto anche se non ho un punto 
				di inserzione lampeggiante
				-- I need to be periodically called to update the cursor shape,
				it would be better to use the MouseMoved event but this way is easier
			    */
		if(max_sleep_time!=-1 && max_sleep_time<sleep) sleep=max_sleep_time;
		}
	return sleep;
} /*GetSleep*/



/* conversione stringa Pascal -> intero
pascal string to integer
*/
long pstrtoi(str)
Str255 str;
{
register short i=str[0];
register unsigned char *p=&str[1];
register long val=0;

while(i>0 && *p==' ') p++,i--;
while(i>0 && *p>='0' && *p <='9' ){
	/* val = val*10+ *p -'0'; */
	val<<=1;
	val += (val<<2) + (*p -'0');
	p++;	i--;
	}
return val;
}
