/*----------------------------------------------------------------------------

	NewsWatcher - Macintosh Network News Reader.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

------------------------------------------------------------------------------

	newswatcher.c

	This module contains the main entry point of the program
	and the main event loop.
----------------------------------------------------------------------------*/

#include <string.h>
#include <appleevents.h>

#include "glob.h"
#include "activate.h"
#include "close.h"
#include "collapse.h"
#include "draw.h"
#include "init.h"
#include "key.h"
#include "mark.h"
#include "menus.h"
#include "mouse.h"
#include "newart.h"
#include "nntp.h"
#include "prefs.h"
#include "print.h"
#include "util.h"
#include "log.h"
#include "killfile.h"


/*	FixCursor sets up the type of cursor needed, depending on where the mouse
	pointer is located.  This routine is called in conjunction with WaitNextEvent.
*/

static void FixCursor (Point mouse, RgnHandle region)
{
	WindowPtr wind;
	RgnHandle arrowRgn,iBeamRgn;
	Rect iBeamRect;
	Point topLeftPt,botRightPt;
	TWindow **info;
	EWindowKind kind;
	TEHandle theTE;
	
	wind = FrontWindow();
	if (!gInBackground && !IsDAWindow(wind)) {
		arrowRgn = NewRgn();
		iBeamRgn = NewRgn();
		SetRectRgn(arrowRgn,-32768,-32768,32767,32767);
		if (IsAppWindow(wind)) {
			info = (TWindow**)GetWRefCon(wind);
			kind = (**info).kind;
			if (kind == kArticle || kind == kMiscArticle ||
				kind == kPostMessage || kind == kMailMessage) 
			{
				theTE = (**info).theTE;
				iBeamRect = (**theTE).viewRect;
				SetPort(wind);
				SetPt(&topLeftPt,iBeamRect.left,iBeamRect.top);
				SetPt(&botRightPt,iBeamRect.right,iBeamRect.bottom);
				LocalToGlobal(&topLeftPt);
				LocalToGlobal(&botRightPt);
				iBeamRect.left = topLeftPt.h;
				iBeamRect.top = topLeftPt.v;
				iBeamRect.right = botRightPt.h;
				iBeamRect.bottom = botRightPt.v;
				RectRgn(iBeamRgn,&iBeamRect);
			}
		}
		DiffRgn(arrowRgn,iBeamRgn,arrowRgn);
		if (PtInRgn(mouse,iBeamRgn)) {
			SetCursor(&gIBeamCurs);
			CopyRgn(iBeamRgn,region);
		} else {
			SetCursor(&qd.arrow);
			CopyRgn(arrowRgn,region);
		}
		DisposeRgn(arrowRgn);
		DisposeRgn(iBeamRgn);
	}
}


/*	DoQuit closes windows prior to the quitting of the application.
*/

static Boolean DoQuit (void)
{
	WindowPtr wind;
	TWindow **info;
	
	while (true) {
		wind = FrontWindow();
		while (wind && (wind == gFullGroupWindow || 
			!IsAppWindow(wind) || IsStatusWindow(wind)))
			wind = (WindowPtr)((WindowPeek)wind)->nextWindow;
		if (wind == nil) break;
		if (!DoCloseWindow(wind)) return false;
	}
	if (gFullGroupWindow != nil) DoCloseWindow(gFullGroupWindow);
	return true;
}



/* MainEvent - main event loop. */

static void MainEvent (void)
{
	EventRecord ev;
	RgnHandle cursorRgn;
	Boolean	gotEvt;
	WindowPtr wind;
	TWindow **info;
	EWindowKind kind;

	cursorRgn = NewRgn();

	while (!gDone) {
	
		gCancel = false;
		
		wind = FrontWindow();
		if (IsStatusWindow(wind)) {
			CloseStatusWindow();
		} else if (IsAppWindow(wind)) {
			info = (TWindow**)GetWRefCon(wind);
			kind = (**info).kind;
			if (kind == kArticle || kind == kMiscArticle  ||
				kind == kPostMessage || kind == kMailMessage)
				TEIdle((**info).theTE);
		}
		
		AdjustMenus();
		
		CloseIdleNewsConnection(false);
			
		gotEvt = WaitNextEvent(everyEvent,&ev,GetCaretTime(),cursorRgn);
		
		FixCursor(ev.where,cursorRgn);
		
		if (gotEvt) {
			switch (ev.what) {
				case mouseDown:
					HandleMouseDown(&ev);
					break;
				case keyDown:
				case autoKey:
					HandleKeyDown(wind, &ev);
					break;
				case activateEvt:
					HandleActivate((WindowPtr)ev.message,
						((ev.modifiers & activeFlag) != 0)); 
					break;
				case updateEvt:
					HandleUpdate((WindowPtr)(ev.message));
					break;
				case app4Evt:
					HandleSuspendResume(ev.message);
					break;
				case kHighLevelEvent:
					AEProcessAppleEvent(&ev);
					break;
			}
			FixCursor(ev.where,cursorRgn);
		}
		
		if (gDone) gDone = DoQuit();
	}
	
	DisposeRgn(cursorRgn);
		
	CloseIdleNewsConnection(true);
	
	EndNNTP();
	WritePrefs();
	
	if (gPrefs.logActionsToFile) CloseLogFile();
	
	#ifdef FILTERS
		WriteKillFile();
	#endif

	memset(gPrefs.remotePassword,'%', sizeof(gPrefs.remotePassword));
	memset(gAutoFetchPass,'%', sizeof(gAutoFetchPass));
}


/* main - main entry point. */

void main (void)
{
	SetApplLimit(GetApplLimit() - 20000);
	MaxApplZone();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	Init();
	UnloadSeg(Init);
	MainEvent();
	ExitToShell();	/* Yes, you really do need to do this! */
}
