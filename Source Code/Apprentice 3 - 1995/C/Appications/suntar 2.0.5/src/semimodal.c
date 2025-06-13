/*******************************************************************************\

movable modal dialog module
© Gabriele Speranza  1-5 May 1991
 modified on 26-27 Dec 1991 in order to use a resource
 and some other changes later

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/
#include "antiglue.h"
#include "windows.h"
#include "suntar.h"

/* I know that Apple has specified its rules about movable modal dialogs,
but this module was written before I've seen System 7.0 or read about
movable modal dialogs, hence I did it according to my personal taste and 
gave it the name I felt right for it. Maybe later I'll remove those little
incompatibilities with Apple guidelines.
Anyway, it works with dialogs having WDEF 5 rather than 3 (it does NOT work
with WDEF 1, since MultiFinder does not send an application to background
when the front window has WDEF 1), with the only anomaly that the dialog
may be dragged also from any point in its content, not only from the title
bar
*/


static WindowRecord	*sm_win_ptr;
static Rect my_text_rect;
static unsigned char** my_text;
static short my_text_just;
static Boolean my_has_default;
Boolean finestra_sm_aperta=false;
Boolean sm_checkbox_status;

#define noItem 100

short new_sm_evf(EventRecord *);
static short new_sm_evf(theEvent)
register EventRecord *theEvent;
{
/* internal event filter: it stops all events which are trying to bring
to front a window which is not the semimodal dialog, and handles events
regarding the window
returns:
0: the event was not handled
noItem: the event was handled
anything else: semimodalDialog must close the window and return this code to the caller
*/
short				code;
WindowPtr		whichWindow;
Point pt=theEvent->where;

switch(theEvent->what){
case mouseDown:
	code=FindWindow( theEvent->where, &whichWindow );
	if(code==inMenuBar||code==inDesk||code==inSysWindow)
		;
	else if(code==inDrag && whichWindow!=(WindowPtr)sm_win_ptr && 
		(theEvent->modifiers&cmdKey))
		;		/* il tasto command comporta il non rendere corrente la finestra
		-- accept a command-drag of another window
		 */
	else if(whichWindow==(WindowPtr)sm_win_ptr){
	/* inutile testare code, non pu˜ essere che inContent, non ha altre aree */
		GrafPtr	savePort;
		ControlHandle 	theControl;
		short item;
		GetPort( &savePort );
		SetPort( sm_win_ptr );
		GlobalToLocal( &pt );
		if ((FindControl(pt, whichWindow, &theControl)) != 0) {
			/*code=TrackControl(theControl, theEvent->where, 0L);
			if(code) controlHit=theControl;	 a button was pushed */
			if(DialogSelect(theEvent,&whichWindow,&item)){
				/* if you want to add an hook to the parameters of semimodalDialog,
				so that it becomes as much customizable as ModalDialog, it should
				be called here */
				short	kind;
				Handle	h;
				Rect	r;
				GetDItem(sm_win_ptr,item,&kind,&h,&r);
				if( (kind&0x7F)==ctrlItem+chkCtrl ){
					SetCtlValue((ControlHandle)h, (sm_checkbox_status=!sm_checkbox_status) );
					return noItem;
					}
				else
					return item;
				}
			}
		else{	/* nel resto della finestra => la sposto
			-- the semimodal dialog is moved by clicking on any point inside it
			but outside the buttons, and dragging
			*/
					Rect	dragRect;
					dragRect.left=0;
					dragRect.top=MBARHEIGHT;
					dragRect.right=screenBits.bounds.right;
					dragRect.bottom=screenBits.bounds.bottom;
					InsetRect(&dragRect,20,10);
					DragWindow( sm_win_ptr, theEvent->where, &dragRect );
				}
		SetPort( savePort );
		return noItem;
		}
	else{
		WindowPtr wp;
		wp=FrontWindow();
		if(wp!=NULL && ((WindowPeek)wp)->windowKind<userKind){
		/* allora il click sulla console va reinterpretato come un click sulla finestrina
		di dialogo e non va affatto ignorato
		-- I've got a click on the console when the current window belonged to a desk
		accessory or another task: bring to front the semimodal dialog
		*/
			SelectWindow(sm_win_ptr);
			}
		return noItem;
		}
	break;
case keyDown:
	if(my_has_default && ((unsigned char)theEvent->message==CR || 
						(unsigned char)theEvent->message==enter_key) ){
		short	kind;
		Handle	h;
		Rect	r;
		GetDItem(sm_win_ptr,1,&kind,&h,&r);
		SelectButton(h);
		return 1;
		}

}	/* fine switch */
return 0;
}

static void semimodal_updater(EventRecord*);
static void semimodal_updater(theEvent)
EventRecord*theEvent;
{
WindowPtr w=NULL;
short item;
DialogSelect(theEvent,&w,&item);

if(my_has_default)
	{short	kind;
	Handle	h;
	Rect	r;
	GetDItem(sm_win_ptr,1,&kind,&h,&r);
	OutlineControl(h);
	}
}

pascal void redrawText(WindowPtr,short);
static pascal void redrawText(theWindow,itemNo)
WindowPtr theWindow;
short itemNo;
{
HLock(my_text);
TextBox ((*my_text)+1,(long)(**my_text),&my_text_rect,my_text_just);
HUnlock(my_text);
}

short semimodalDialog(dialog_ID,where,filter,titoli_italiani,n_titoli,p1,p2,p3,
	just,has_default,initializer)
/* the only external entry point: create and handle a semimodal dialog: */
short dialog_ID;
Point *where;	/* NULL for a centered window, a pointer to a point (usually
	initialized to -1,-1) for a window which remembers its old position */
short (*filter)(EventRecord*);	/* an optional event filter to use during the dialog,
		executed before the internal event filter. It must return on of these codes:
		0: that event was not filtered, or was filtered but the standard actions 
			still need be performed
		1: filtered event, don't handle it
		<0: filtered event, you must return immediately with this value as return code,
			the caller knows what to do with that code
		*/
char **titoli_italiani;
short n_titoli;
Str255 p1,p2,p3;	/* unfortunately, if I use static text items I have two problems:
	1) suntar 1.0 and 1.1 used two different ways to center the messages,
	  with static text items that can't be done: that's bad, but it might
	  be tolerated
	2) if a modal dialog appears (and there may be a modal dialog above a
	  semimodal...) the ParamText for the modal dialog overwrites the ParamText
	  for the semimodal, so that when it's redrawn later its messages are
	  changed: horrible !
	Hence, I was obliged to avoid using ParamText. And since I did not want
	to handle updates in a complex way, I had to use a userItem rather than a
	staticText. But this is a bad choice, one day I'll change that
	*/
short just;		/* justification, usually centered, but it may be left too */
Boolean has_default;
void (*initializer)(WindowRecord*);		/* The biggest problem with this routine it
	that it's monolithic: unlike with ModalDialog, it's called before
	creating the window and returns after disposing it, so I can't do
	any preprocessing (setting the checkbox value) nor postprocessing
	(reading its final value), and everything must be handled here.
	 Probably one day I'll rewrite it so that such things may be done, maybe
	breaking it into three pieces. By now, the most necessary thing is an
	initializer, and here it is */
/* one more parameter, sm_checkbox_status, is a global variable: after calling
semimodalDialog, it contains the final value of a checkbox in the dialog (if it
was correctly initialized): that allows me to avoid the "finalizer" and the dialog hook.
*/
{
	AlertTHndl	alertHandle;
	short item;

	flush_all();
	check_foreground();
	SetCursor(&arrow);

	alertHandle = (AlertTHndl)GetResource('DLOG',dialog_ID);
	if (alertHandle) {
		short	kind;
		Handle	h;
		Rect	r;
		short items_in_dialog;
		HLock((Handle)alertHandle);
		if(where==NULL||where->v<0)
			PositionDialog( &((**alertHandle).boundsRect));
		else{
			register Rect *r=&((**alertHandle).boundsRect);
			r->bottom= where->v+(r->bottom-r->top);
			r->right= where->h+(r->right-r->left);
			r->top=where->v;
			r->left=where->h;
			}
		/* (**alertHandle).windowDefProc=(Handle)altDBoxProc; */
		sm_win_ptr=GetNewDialog(dialog_ID,NULL,(WindowPtr)-1L);
		sm_win_ptr->refCon=-1;		/* il -1 
							serve per non essere considerata 
							una "mia" finestra dal modulo windows 
							-- the windows module uses the refCon field to
							recognize its own windows, hence it can't be 0 */ 
		HUnlock((Handle)alertHandle);
		finestra_sm_aperta=true;
		items_in_dialog= *(short*)*(((DialogRecord*)sm_win_ptr)->items) + 1;
		if(in_Italia && titoli_italiani){
			short fatti=0;
			for(item=1;fatti<n_titoli&&item<=items_in_dialog;item++){
				GetDItem(sm_win_ptr,item,&kind,&h,&r);
				if((kind&0x7F)==ctrlItem+btnCtrl || (kind&0x7F)==ctrlItem+chkCtrl){
					SetCTitle(h,*titoli_italiani++);
					fatti++;
					}
				}
			}
		my_has_default=has_default;
		if(has_default){
			GetDItem(sm_win_ptr,1,&kind,&h,&r);	/* item 1 must be the default button */
			OutlineControl(h);
			}
/*if(p1==NULL) printf("p1=NULL "); else printf("p1=%P",p1);
if(p2==NULL) printf("p2=NULL "); else printf("p2=%P",p2);
if(p3==NULL) printf("p3=NULL "); else printf("p3=%P",p3);*/
		if(p1!=NULL){
			short i;
			item=1;
			do{	/* find the Rect of the useritem (p1,p2,p3): an userItem MUST exist */
				GetDItem(sm_win_ptr,++item,&kind,&h,&my_text_rect);
				}
			while((kind&0x7F)!=userItem && item<items_in_dialog);
			if((kind&0x7F)==userItem) SetDItem (sm_win_ptr,item, kind,
				(Handle)redrawText, &my_text_rect);
				/* probably I could have used a static text item and SetIText,
				but then I would have got left justification only */
			i=p1[0]+1;
			if(p2) i+=p2[0];
			if(p3) i+=p3[0];
			my_text=NewHandle(i);
			my_text_just=just;
			pStrcpy(*my_text,p1);
			if(p2)pStrcat(*my_text,p2);
			if(p3)pStrcat(*my_text,p3);
			}
		else
			my_text=NULL;
		if(initializer!=NULL)
			(*initializer)(sm_win_ptr);
		else
			sm_checkbox_status=false;
		ShowWindow(sm_win_ptr);

		item=0;
		install_handlers(sm_win_ptr,semimodal_updater,NULL);
		do{
			{
			WindowPeek wp;
			wp= gInBackground ? NULL : (WindowPeek) FrontWindow();
			if(wp!=NULL &&  wp->windowKind>=userKind && wp !=sm_win_ptr)
				SelectWindow(sm_win_ptr);	/* non dovrebbe mai capitare,ma non fidarsi
											pu˜ salvare dai guai
						-- the internal event filter should prevent the console window 
						from going to front but there are a few utilities which add
						a "windows" menu to applications and can bring to front any
						window by cheating. If that happens, bring to front the semimodal
						dialog again
						*/
			}{
			EventRecord		myEvent;
			if(get_event(&myEvent)){
				if(filter){
					if ( (item=(*filter)(&myEvent))==0 )
						item=new_sm_evf(&myEvent);
					else if(item>0){
						GetDItem(sm_win_ptr,item,&kind,&h,&r);
						if(kind==ctrlItem+btnCtrl)
							SelectButton(h);
						}
					}
				else
					item=new_sm_evf(&myEvent);
				if(item==0){
					if(my_event_filter!=NULL) (*my_event_filter)(&myEvent);
					handle_event(&myEvent);
					if(is_pause_command()){
						HideWindow(sm_win_ptr);
						handle_pause();
						ShowWindow(sm_win_ptr);
						SelectWindow(sm_win_ptr);
						}
					}
				}
			else{
				/* the user filter must be called for null events too... */
				if(filter){
					myEvent.what=nullEvent;
					item=(*filter)(&myEvent);
					}
				}
			if(item==noItem) item=0;
			}
			accept_abort_command();
			}
		while(item==0);

		my_event_filter=NULL;

		if(where){
			GrafPtr	savePort;
			GetPort( &savePort );
			SetPort( sm_win_ptr );
			*where=*(Point*)&(sm_win_ptr->port.portRect);	/* top e left */
			LocalToGlobal(where);		/* remember where the window was before closing it*/
			SetPort(savePort);
			}
	
		DisposDialog(sm_win_ptr);
		remove_handlers(sm_win_ptr);
		if(my_text)DisposHandle(my_text);
		finestra_sm_aperta=false;
		return item;
		}
}


void close_semimodal()
{	/* viene chiamata in caso di un longjmp che potrebbe abortire la routine precedente
-- in suntar, all files or windows which may be open when the abort command is enabled
or when an error may occur, must provide a "cleanup" routine to close everything (except 
when their creator has a setjmp and performs directly the cleanup).
Don't use for any other purpose
*/
if(finestra_sm_aperta){
	DisposDialog(sm_win_ptr);
	remove_handlers(sm_win_ptr);
	if(my_text)DisposHandle(my_text);
	finestra_sm_aperta=false;
	}
}


