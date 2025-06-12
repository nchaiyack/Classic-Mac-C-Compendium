/**********************************************************************\

File:		main.c

Purpose:	This module handles the event loop and event dispatching.

\**********************************************************************/

#include "graphics.h"
#include "main.h"
#include "apple events.h"
#include "integrity.h"
#include "about.h"
#include "help.h"
#include "menus.h"
#include "prefs.h"
#include "environment.h"
#include "progress.h"
#include "error.h"
#include "dialectic.h"
#include "program globals.h"

void main(void)
{
	Boolean			programIntegrityVerified;
	Boolean			programIntegritySet;
	
	/* do integrity check before anything else; see integrity.c for details */
	programIntegrityVerified=DoIntegrityCheck(&programIntegritySet);
	
	/* standard program initialization stuff */
	MaxApplZone();	
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	GetDateTime(&randSeed);
	
	if (!InitTheEnvironment())			/* gestalt checks and variable initialization */
		HandleError(kSystemTooOld, TRUE);		/* less than system 4.1 */
	
	if (!programIntegrityVerified)	/* integrity check failed */
		HandleError(kProgramIntegrityNotVerified, TRUE);
	
	if (programIntegritySet)	/* integrity check freshly installed */
		HandleError(kProgramIntegritySet, FALSE);
	
	if (!InitTheMenus())		/* get menus from .rsrc and draw menu bar */
		HandleError(kProgramIntegrityNotVerified, TRUE);
		
	if (!InitTheGraphics())		/* initialize offscreen gworlds/bitmaps, etc */
		HandleError(kNoMemoryAndQuitting, TRUE);
	
	PrefsError(PreferencesInit());	/* get prefs (create if necessary) */
	
	InitTheProgram();				/* program-specific initialization */
	
	EventLoop();					/* where it all happens (see below) */
	
	ShutDownEnvironment();			/* where it all ends (see below) */
	
	ExitToShell();
}

void EventLoop(void)
{
	EventRecord		theEvent;
	int				i;
	
	while (!gDone)	/* gDone set by choosing "Quit" menu item or by "quit" apple event */
	{
		SetCursor(&arrow);		/* should set once every time through event loop */
		HiliteMenu(0);			/* normalize menubar */
		
		gFrontWindowIsOurs=FALSE;
		if (FrontWindow()!=0L)	/* if there's a front window, see if it's one of ours */
		{
			for (i=0; ((i<NUM_WINDOWS) && (!gFrontWindowIsOurs)); i++)
			{
				if (FrontWindow()==gTheWindow[i])	/* found one of ours, so SetPort */
				{
					SetPort(gTheWindow[i]);
					gFrontWindowIsOurs=TRUE;
				}
			}
		}
			
		/* get an event from the queue */
		GetTheEvent(&theEvent, gIsInBackground ? gBackgroundWaitTime : gForegroundWaitTime);
		
		DispatchEvents(theEvent);	/* handle the event we just got */
	}
}

void GetTheEvent(EventRecord *theEvent, int waitTime)
{
	if (gWaitNextEventAvailable)
		WaitNextEvent(everyEvent, theEvent, waitTime, 0L);
	else
	{
		GetNextEvent(everyEvent, theEvent);
		SystemTask();
	}
}

void DispatchEvents(EventRecord theEvent)
{
	int				i;
	Point			thisPoint;
	int				index;
	unsigned long	dummy;
	WindowPtr		theWindow;
	Boolean			thisWindowIsOurs;
	ExtendedWindowDataHandle
					theData;
	
	thisWindowIsOurs=FALSE;
	/* for update/activate events, see if the window in question is one of ours */
	if ((theEvent.what==activateEvt) || (theEvent.what==updateEvt))
	{
		for (i=0; ((i<NUM_WINDOWS) && (!thisWindowIsOurs)); i++)
			thisWindowIsOurs=((WindowPtr)theEvent.message==gTheWindow[i]);
	}

	if (thisWindowIsOurs)	/* for activate/update events, get window data structure */
	{
		theData=(WindowDataHandle)GetWRefCon((WindowPtr)theEvent.message);
		index=(**theData).windowIndex;
	}
	else if (gFrontWindowIsOurs)	/* if front window is ours, get window data struct */
	{
		theData=(WindowDataHandle)GetWRefCon(FrontWindow());
		index=(**theData).windowIndex;
	}
	else index=-1;
	
	switch (theEvent.what)
	{
		case nullEvent:	/* ain't nuthin' happenin' */
			if (gFrontWindowIsOurs)		/* give control to window dispatch to handle null */
				((**theData).dispatchProc)(theData, kNull, 0L);
			break;
		case mouseDown:	/* mouse button pressed */
			HandleMouseDown(theEvent);	/* see below for mousedown handling */
			break;
		case keyDown:	/* key pressed */
		case autoKey:	/* key help down */
			if (theEvent.modifiers & cmdKey)	/* handle as command-key equivalent */
			{
				AdjustMenus();	/* just to be safe */
				/* get the menu ID + item and handle it as a menu choice */
				HandleMenu(MenuKey((char)(theEvent.message & charCodeMask)));
			}
			else if (gFrontWindowIsOurs)	/* --> window's dispatch for keydown */
				((**theData).dispatchProc)(theData, kKeydown, theEvent.message);
			break;
		case diskEvt:	/* disk insert */
			if (HiWord(theEvent.message)!=noErr)	/* bad disk inserted */
			{
				DILoad();	/* load disk initialization package */
				SetPt(&thisPoint, 120, 120);
				DIBadMount(thisPoint, theEvent.message);	/* give format? dialog */
				DIUnload();	/* unload 'cuz we certainly don't need it */
			}
			break;
		case updateEvt:	/* window update */
			theWindow=(WindowPtr)theEvent.message;	/* which window? */
			
			BeginUpdate(theWindow);		/* means: "OK, we're dealing with this now" */
			
			if (thisWindowIsOurs)		/* one of ours?  see graphics.c */
				UpdateTheWindow(theData);
			/* if not, could be our progress bar */
			else if ((theWindow!=0L) && (theWindow==gProgressDlog))
				UpdateDialog(theWindow, theWindow->visRgn);
			
			EndUpdate(theWindow);		/* means: "OK, we're done updating now" */
			break;
		case activateEvt:	/* window activate or deactivate */
			if (thisWindowIsOurs)		/* one of ours?  send message to window dispatch */
				((**theData).dispatchProc)(theData,
					((theEvent.modifiers&activeFlag)!=0) ? kActivate : kDeactivate, 0L);
			break;
		case osEvt:			/* suspend or resume program execution (switch in/out) */
			if (((theEvent.message>>24)&0x0FF)==suspendResumeMessage)
			{
				/* keep track of whether we're in the background or foreground */
				gIsInBackground=((theEvent.message&resumeFlag)==0);
				
				if (gFrontWindowIsOurs)		/* send activate/deactivate to front window */
					((**theData).dispatchProc)(theData, gIsInBackground ?
						kDeactivate : kActivate, 0L);
				
				for (i=0; i<NUM_WINDOWS; i++)	/* send suspend/resume to all our */
					if (gTheWindow[i]!=0L)		/* windows that currently exist */
						((**(gTheWindowData[i])).dispatchProc)(gTheWindowData[i],
							gIsInBackground ? kSuspend : kResume, 0L);
				
				/* if we just came into the foreground and we have a pending error,
				   now's the time to display it */
				if ((!gIsInBackground) && (gPendingResultCode!=allsWell))
				{
					if (gHasNotificationManager)
						NMRemove(&gMyNotification);		/* remove notification request */
					HandleError(gPendingResultCode, FALSE);	/* display alert, see error.c */
					gPendingResultCode=allsWell;		/* ...now it is */
				}
			}
			break;
		case kHighLevelEvent:	/* apple event */
			if (gHasAppleEvents)
				AEProcessAppleEvent(&theEvent);		/* see apple events.c */
			break;
	}
}

void HandleMouseDown(EventRecord theEvent)
{
	WindowPtr			theWindow;
	int					windowCode;
	long				windSize;
	GrafPtr				oldPort;
	int					i;
	Rect				sizeRect;
	Boolean				gotone;
	int					index;
	unsigned long		dummy;
	Point				theLocalPoint;
	Boolean				thisWindowIsOurs;
	ExtendedWindowDataHandle	theData;
	
	windowCode=FindWindow(theEvent.where, &theWindow);	/* which window? */

	thisWindowIsOurs=FALSE;
	/* find out if the target window was one of ours */
	if (theWindow!=0L)
		for (index=0; ((index<NUM_WINDOWS) && (!thisWindowIsOurs)); index++)
			thisWindowIsOurs=(theWindow==gTheWindow[index]);

	if (thisWindowIsOurs)	/* if target window is one of ours, get window data struct */
	{
		theData=(WindowDataHandle)GetWRefCon(theWindow);
		index=(**theData).windowIndex;
	}
	else index=-1;
	
	switch (windowCode)
	{
		case inMenuBar:		/* in menu bar; let system take over */
			AdjustMenus();
			HandleMenu(MenuSelect(theEvent.where));
			break;
		case inContent:		/* in window content */
			if (FrontWindow() != theWindow)		/* maybe switch to different window */
			{
				if (FrontWindow()==gProgressDlog)	/* but not if progress bar is up */
					SysBeep(7);
				else SelectWindow(theWindow);
			}
			else if (gFrontWindowIsOurs)	/* inform window dispatch of mousedown */
			{
				GetMouse(&theLocalPoint);	/* in window's local coordinates */
				dummy=theLocalPoint.h;		/* all this fiddling is so we can pass */
				dummy=dummy<<16;			/* the point (two integers) to the */
				dummy+=theLocalPoint.v;		/* dispatch procedure in an unsigned long */
				((**theData).dispatchProc)(theData, kMousedown, dummy);	/* go for it */
			}
			break;
		case inSysWindow:	/* in system window (desk accessory) */
			SystemClick(&theEvent, theWindow);	/* let the system deal with it */
			break;
		case inDrag:		/* in drag _region_, that is */
			/* the accepted way to draw a window */
			DragWindow(theWindow, theEvent.where, &((**GetGrayRgn()).rgnBBox));
			if (thisWindowIsOurs)	/* update window bounds in window data struct */
				(**theData).windowBounds=(*(((WindowPeek)gTheWindow[index])->contRgn))->rgnBBox;
			break;
		case inGoAway:		/* close box */
			/* the accepted way to track a close box attempt */
			if (TrackGoAway(theWindow, theEvent.where))
				DoTheCloseThing(theWindow);		/* see menus.c */
			break;
		case inGrow:		/* grow box */
			/* the accepted way to grow a window */
			sizeRect = screenBits.bounds;
			OffsetRect(&sizeRect, sizeRect.left, sizeRect.top);
			
			windSize = GrowWindow(theWindow, theEvent.where, &sizeRect);
			if (windSize != 0)
			{
				GetPort(&oldPort);
				SetPort(theWindow);
				EraseRect(&theWindow->portRect);
				SizeWindow(theWindow, LoWord(windSize), HiWord(windSize), TRUE);
				InvalRect(&theWindow->portRect);
				SetPort(oldPort);
			}
			
			if (thisWindowIsOurs)	/* update window bounds in window data struct */
				(**theData).windowBounds=(*(((WindowPeek)gTheWindow[index])->contRgn))->rgnBBox;
			break;
		case inZoomIn:		/* zoom box */
		case inZoomOut:
			/* the accepted way to track a zoom attempt */
			if (TrackBox(theWindow, theEvent.where, windowCode))
			{
				GetPort(&oldPort);
				SetPort(theWindow);
				ZoomWindow(theWindow, windowCode, FALSE);
				InvalRect(&theWindow->portRect);
				SetPort(oldPort);
			}
			
			if (thisWindowIsOurs)	/* update window bounds in window data struct */
				(**theData).windowBounds=(*(((WindowPeek)gTheWindow[index])->contRgn))->rgnBBox;
			break;
	}
}

void ShutDownEnvironment(void)
{
	ShutDownTheProgram();		/* program-specific cleanup */
	ShutDownTheGraphics();		/* shell-specific clean-up */
}
