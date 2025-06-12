#include <Windows.h>
#include <Quickdraw.h>
#include <Menus.h>
#include <Resources.h>
#include <Memory.h>
#include <Fonts.h>
#include <OSEvents.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Desk.h>
#include <ToolUtils.h>
#include "trigtab.h"

trigtab *trig;
int xmax,ymax;
WindowPtr	 myWindow;
Rect		dragRect = { 0, 0, 1024, 1024 };
GrafPtr 	offscreen;/* our off-screen bitmap */
MenuHandle		myMenus[3];

/* resource IDs of menus */
#define appleID			128
#define fileID			129

/* Menu indices */
#define appleM			0
#define fileM			1

#define fmNew		1
#define fmClose		2
#define fmPageSetUp	4
#define fmPrint		5
#define fmQuit		7

static int UpdateWindow (void)
{
	GrafPtr	savePort;
	
	GetPort(&savePort);
	SetPort(myWindow);

	BeginUpdate(myWindow);
	CopyBits(&offscreen->portBits, &(*myWindow).portBits,
			 &offscreen->portRect, &(*myWindow).portRect, srcCopy, 0L);
/*
	DrawControls(myWindow);
	DrawGrowIcon(myWindow);
*/
	EndUpdate(myWindow);

	SetPort(savePort);
}

static int SetUpMenus(void)
{
	int		i;
	
	myMenus[appleM] = NewMenu(appleID, "\p\024");
	AddResMenu(myMenus[appleM], 'DRVR');
	myMenus[fileM] = GetMenu(fileID);
	for ((i=appleM); (i<=fileM); i++)
		InsertMenu(myMenus[i], 0) ;
	DrawMenuBar();
}

#define	ours(w)		((myWindow != NULL) && (w == myWindow))

static int DoCommand(long mResult)
{
	int		theItem;
	Str255	name;
	
	theItem = LoWord(mResult);
	switch (HiWord(mResult)) {
		case appleID:
			GetItem(myMenus[appleM], theItem, name);
			OpenDeskAcc(name);
			SetPort(myWindow);
			break;

		case fileID: 
			return(0);
			break;
	}
	HiliteMenu(0);
	return(1);
}

#define SBarWidth	15

static int MyGrowWindow(WindowPtr w, Point p)

{
	GrafPtr	savePort;
	long	theResult;
	Rect	oldHorizBar;
	Rect 	r;
	
	GetPort(&savePort);
	SetPort(w);
	
	oldHorizBar = w->portRect;
	oldHorizBar.top = oldHorizBar.bottom - (SBarWidth+1);

	SetRect(&r, 80, 80, qd.screenBits.bounds.right, qd.screenBits.bounds.bottom);
	theResult = GrowWindow(w, p, &r);
	if (theResult == 0)
	  return;
	SizeWindow( w, LoWord(theResult), HiWord(theResult), false);

	InvalRect(&w->portRect);
	
	EraseRect(&oldHorizBar);
	
	SetPort(savePort);
}

static int DoMouseDown (int windowPart, WindowPtr whichWindow, EventRecord *myEvent)
{
	switch (windowPart) {
		case inGoAway:
			if (ours(whichWindow))
				if (TrackGoAway(myWindow, myEvent->where))
					ExitToShell();
			break;

		case inMenuBar:
			return(DoCommand(MenuSelect(myEvent->where)));

		case inSysWindow:
			SystemClick(myEvent, whichWindow);
			break;

		case inDrag:
			if (ours(whichWindow))
				DragWindow(whichWindow, myEvent->where, &dragRect);
			break;

		case inGrow:
			if (ours(whichWindow))
				MyGrowWindow(whichWindow, myEvent->where);
			break;

		case inContent:
			if (whichWindow != FrontWindow())
				SelectWindow(whichWindow);
			else if (ours(whichWindow))
				{
				/* DoContent(whichWindow, myEvent); */
				}
			break;
	}
}

static int MainEvent(void) 
{
	EventRecord		myEvent;
	WindowPtr		whichWindow;
	short			windowPart;
	Rect			r;
	
	SystemTask();
	if (GetNextEvent(everyEvent, &myEvent)) {
		switch (myEvent.what) {
		case mouseDown:
			windowPart = FindWindow(myEvent.where, &whichWindow);
			return DoMouseDown(windowPart, whichWindow, &myEvent);
			break;

		case activateEvt:
			if (ours((WindowPtr)myEvent.message)) {
				if (myEvent.modifiers & activeFlag) {
				}
				else {
				}
			}
			break;

		case updateEvt: 
			if (ours((WindowPtr) myEvent.message))
					UpdateWindow();
			break;
		} /* end of case myEvent.what */
	} /* if */
	return(1);
}

main(void)
{
	double ref_lat = 0.0;
	double ref_long = 0.0;
	double plot_size = 90.0;

	Rect rBounds = {50,50,450,550};
	
	struct {
		short col;
		long a,b,c,d;
		} *rsrc;
	int i;
	trigtab *tr;
	Handle themap = GetResource('TRIG', 128);
	int siz = MaxSizeRsrc(themap);
	xmax = rBounds.right-rBounds.left-4;
	ymax = rBounds.bottom-rBounds.top-4;
	
	trig = (trigtab *)NewPtr(siz);
	HLock(themap);
	rsrc = (void *)*themap;
	tr = trig;
	do
		{
		tr->color = rsrc->col;
		tr->cosnewlng = rsrc->a/1000000.0;
		tr->sinnewlng = rsrc->b/1000000.0;
		tr->cosnewlat = rsrc->c/1000000.0;
		tr->sinnewlat = rsrc->d/1000000.0;
		rsrc++;
		tr++;
		}
	while (tr-trig < siz/sizeof(trigtab));

	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	MaxApplZone();

	SetUpMenus();

	myWindow = NewWindow(0,&rBounds,"\pMap",1,noGrowDocProc,(WindowPtr)-1,0,0);
	
	if(!CreateOffscreenBitMap(&offscreen, &myWindow->portRect))
		{
		SysBeep(1);
		ExitToShell();
		}
	
	while (MainEvent())
		{
		GrafPtr	savePort;
	
		GetPort(&savePort);
		SetPort(offscreen);
		EraseRect(&(offscreen->portRect));
		plot(ref_lat,ref_long,plot_size);
		SetPort(myWindow);
		InvalRect(&(myWindow->portRect));
		SetPort(savePort);
		UpdateWindow();
		ref_long += 5;
		}
	DestroyOffscreenBitMap(offscreen);	/* dump the off-screen bitmap */
	}
