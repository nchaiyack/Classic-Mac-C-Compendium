//-- EVENT.C

// Handling events that come up.
#include <Events.h>

#include <stdio.h>
#include "res.h"
#include "struct.h"


extern MenuHandle applMenu;
extern MenuHandle fileMenu;
extern MenuHandle editMenu;
extern unsigned char ColorQD;
extern unsigned char MultiWidget;
static unsigned char QuitFlag;

//------------------------------------------------------------------------//																							*/
//----EVENTS-------How to handle the interface to the event stuff---------//								*/
//------------------------------------------------------------------------//																							*/

//-- GetEvent

// Based upon the flag settings in ModeFlags, this calls either GetNextEvent or WaitNextEvent
// for the next event in the event queue.

int GetEvent(event)
EventRecord *event;
{
	if (MultiWidget) {
		return WaitNextEvent(everyEvent,event,30L,NULL);		/* Allow timeout for 1/2 second */
	} else {
		SystemTask();
		return GetNextEvent(everyEvent,event);
	}
}

//-- DoEvent

// This fetches the next event from the event queue, dispatching the event as needed.

int DoEvent()
{
	EventRecord event;
	char c;
	short wlow,whigh;
	Point p;
	char buffer[64];
	long l;
	short s;

	QuitFlag = 0;
	GetEvent(&event);
	switch (event.what) {
		case nullEvent:
			SearchDisks();
			break;
		case mouseDown:
			DoMouse(&event);
			break;
		case keyDown:
		case autoKey:
			c = event.message & charCodeMask;
			if (event.modifiers & cmdKey) {
				if (event.what == keyDown) {
					DoMenu(MenuKey(c));
				}
			} else DoKey(c,FrontWindow());
			break;
		case updateEvt:
			DoUpdate(event.message);
			break;
		case activateEvt:
			DoActivate(event.message,(event.modifiers & activeFlag) ? 1 : 0);
			break;
		case diskEvt:
			wlow = (event.message) & 0x0FFFF;
			whigh = (event.message) >> 16;
			if (whigh != 0) {
				DILoad();						/* Prepare to init disk */
				p.h = p.v = 75;
				whigh = DIBadMount(p, event.message);
				DIUnload();
				if (whigh != 0) break;
			}
			GetVInfo(wlow,buffer,&s,&l);
			NewPlan(s);
			break;
	}

	return QuitFlag;
}

//-- DoMouse

// What to do when the mouse goes down somewhere in the application.

DoMouse(event)
EventRecord *event;
{
	int i;
	WindowPtr w;
	GrafPtr p;
	Rect r;
	long l;

	i = FindWindow(event->where,&w);
	if (w != NULL) SetPort(w);

	switch (i) {
		case inDesk:
			break;
		case inMenuBar:
			DoMenu(MenuSelect(event->where));
			break;
		case inSysWindow:
			SystemClick(event,w);
			break;
		case inGrow:
			r.top = 100;
			r.left = 150;
			r.bottom = 32767;
			r.right = 32767;			/* Absolutely rediculous size */
			l = GrowWindow(w,event->where,&r);
			if (l != 0) {
				DoResizeInit(w);
				EraseRect(&(w->portRect));
				SizeWindow(w,(short)(l & 0x0FFFF),(short)(l >> 16),false);
				DoResizeWindow(w);
				InvalRect(&(w->portRect));
			}
			break;
		case inContent:
			if (w != FrontWindow()) SelectWindow(w);
			else DoWindowClick(event,w);
			break;
		case inDrag:
			GetWMgrPort(&p);
			r = p->portRect;
			r.top += 38;
			InsetRect(&r,4,4);
			DragWindow(w,event->where,&r);
			break;
		case inGoAway:
			if (TrackGoAway(w,event->where)) CloseWin(w);
			break;
		case inZoomIn:
		case inZoomOut:
			if (TrackBox(w,event->where,i)) {
				DoResizeInit(w);
				EraseRect(&(w->portRect));
				ZoomWindow(w,i,false);
				DoResizeWindow(w);
				InvalRect(&(w->portRect));
			}
			break;
	}
}

//------------------------------------------------------------------------//																							*/
//-----------MENUS-----How to handle the interface to the menu bar--------//
//------------------------------------------------------------------------//																							*/

//-- DoMenu

// What to do when something happens in the menu bar.

DoMenu(l)
long l;
{
	short hi,lo;
	short i;

	hi = (short)(l >> 16);
	lo = (short)(l & 0x0FFFF);

	switch (hi) {
		case APPLMENU:
			if (lo == ABOUTMEMENU) DoAboutMe();
			else {
				char buffer[64];
				GetItem(applMenu,lo,buffer);
				OpenDeskAcc(buffer);
			}
			break;
		case FILEMENU:
			switch (lo) {
				case CLOSEMENU:
					CloseWin(FrontWindow());
					break;
				case QUITMENU:
					QuitFlag = 1;
					return;
				case SAVEMENU:
					SaveText();
					break;
				case PRINTSTL:
					PrintStl();
					break;
				case PRINTJOB:
					PrintJob();
					break;
			}
			break;
		case EDITMENU:
			if (!SystemEdit(lo-1)) switch (lo) {
				case OPTIONS:
					OptionDialog();
					break;
			}
			break;
	}
	HiliteMenu(0);
}


//------------------------------------------------------------------------//																							*/
//-----------------WINDOWS---------How to handle windows------------------//
//------------------------------------------------------------------------//																							*/

//-- CloseWin

// This closes a window.  Return TRUE if the window was closed successfully.

int CloseWin(w)
WindowPtr w;
{
	short i;

	if (w == NULL) return true;			/* No window to close */
	i = ((WindowPeek)w)->windowKind;	/* Figure out what window we have and close it */
	if (i < 0) {
		CloseDeskAcc(i);				/* Shut down the desk accessory */
		return true;
	} else switch (i) {					/*CH*/
		case WK_PLAN:
			return ClosePlan(w);
		default:
			DisposeWindow(w);
			return true;
	}
}


//-- DoWindowClick

// This figures out what to do when the mouse goes down.

DoWindowClick(e,w)
EventRecord *e;
WindowPtr w;
{
	short i;
	Point p;
	ControlHandle c;
	short j,k;
	Rect r;
	Rect s;
	WindowPtr port;

	if (w == NULL) return;
	i = ((WindowPeek)w)->windowKind;
	p = e->where;
	GlobalToLocal(&p);
	switch (i) {						/*CH*/
		case WK_PLAN:
			MousePlan(w,e,&p);
			break;
	}
}


//-- DoResizeInit

// Initialize any variables which need to be initialized before resizing a window.

DoResizeInit(w)
WindowPtr w;
{
	short i;

	if (w == NULL) return;
	i = ((WindowPeek)w)->windowKind;
	switch (i) {						/*CH*/
		case WK_PLAN:
			ResizeInitPlan(w);
			break;
	}
}


//-- DoResizeWindow

// This resizes the data structures for a new size window.

DoResizeWindow(w)
WindowPtr w;
{
	short i;

	if (w == NULL) return;
	i = ((WindowPeek)w)->windowKind;
	switch (i) {						/*CH*/
		case WK_PLAN:
			ResizePlan(w);
			break;
	}
}


//-- DoKey

// What to do when the user hits a key.

DoKey(c,w)
char c;
WindowPtr w;
{
	short i;

	if (w == NULL) return;
	i = ((WindowPeek)w)->windowKind;
	switch (i) {
	}
}


//-- DoUpdate

// This does the window update.

DoUpdate(w)
WindowPtr w;
{
	short i;

	if (w == NULL) return;
	i = ((WindowPeek)w)->windowKind;

	BeginUpdate(w);
	SetPort(w);
	switch (i) {						/*CH*/
		case WK_PLAN:
			UpdatePlan(w);
			break;
	}
	EndUpdate(w);
}


//-- DoActivate

// Handle the activate event.

DoActivate(w,flag)
WindowPtr w;
int flag;
{
	short i;

	if (w == NULL) return;
	i = ((WindowPeek)w)->windowKind;
	switch (i) {						/*CH*/
		case WK_PLAN:
			ActivatePlan(w,flag);
			break;
	}
}



//-- AboutMeDH

// The function to pass to ModalDialog for the 'about me' dialog.

pascal Boolean AboutMeDH(theDialog,theEvent,itemHit)
DialogPtr *theDialog;
EventRecord *theEvent;
short *itemHit;
{
	if (theEvent->what == mouseDown) {
		*itemHit = 1;
		return true;
	} else return false;
}




//-- DoAboutMe

// How to handle the 'about me' box.

DoAboutMe()
{
	DialogPtr dlog;
	short i1;
	Handle i2;
	Rect i3;
	short x,y;
	char **ptr;
	char buffer[32];

	dlog = GetNewDialog(ABOUTMEDLOG,NULL,(char *)-1);
	if (dlog == NULL) return;
	SetPort(dlog);

	for (;;) {
		ModalDialog(AboutMeDH,&i1);
		if (i1 == 1) break;
	}

	DisposDialog(dlog);
}

