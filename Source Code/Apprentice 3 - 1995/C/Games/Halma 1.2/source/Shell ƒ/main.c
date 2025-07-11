/**********************************************************************\

File:		main.c

Purpose:	This module handles the event loop and event dispatching.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

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
#include "sounds.h"
#include "halma.h"
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
	GetDateTime((unsigned long*)&randSeed);
	
	if (!InitTheEnvironment())			/* gestalt checks and variable initialization */
		HandleError(kSystemTooOld, TRUE);		/* less than system 4.1 */
	
	if (!programIntegrityVerified)	/* integrity check failed */
		HandleError(kProgramIntegrityNotVerified, TRUE);
	
	if (programIntegritySet)	/* integrity check freshly installed */
		HandleError(kProgramIntegritySet, FALSE);
	
	if (!InitTheGraphics())		/* initialize offscreen gworlds/bitmaps, etc */
		HandleError(kNoMemoryAndQuitting, TRUE);
	
	if (!InitTheMenus())		/* get menus from .rsrc and draw menu bar */
		HandleError(kProgramIntegrityNotVerified, TRUE);
		
	InitTheSounds();				/* see if sound is available, etc. */
	
	PrefsError(PreferencesInit());	/* get prefs (create if necessary) */
	
	CallIndDispatchProc(kHelp, kStartup, 0L);
	
	InitTheProgram();				/* program-specific initialization */
	
	EventLoop();					/* where it all happens (see below) */
	
	ShutDownEnvironment();			/* where it all ends (see below) */
	
	ExitToShell();
}

void EventLoop(void)
{
	while (!gDone)	/* gDone set by choosing "Quit" menu item or by "quit" apple event */
		HandleSingleEvent();
}

Boolean HandleSingleEvent(void)
{
	EventRecord		theEvent;
	short			i;
	
	SetCursor(&arrow);		/* should set once every time through event loop */
	HiliteMenu(0);			/* normalize menubar */
	
	gFrontWindowIsOurs=FALSE;
	gFrontWindowIndex=0;
	if (FrontWindow()!=0L)	/* if there's a front window, see if it's one of ours */
	{
		for (i=0; ((i<NUM_WINDOWS) && (!gFrontWindowIsOurs)); i++)
		{
			if (FrontWindow()==GetIndWindowGrafPtr(i))	/* found one of ours, so SetPort */
			{
				SetPort(GetIndWindowGrafPtr(i));
				gFrontWindowIsOurs=TRUE;
				gFrontWindowIndex=i;
			}
		}
	}
		
	/* get an event from the queue */
	GetTheEvent(&theEvent, gIsInBackground ? gBackgroundWaitTime : gForegroundWaitTime);
	
	DispatchEvents(theEvent);	/* handle the event we just got */
	
	return (theEvent.what!=nullEvent);
}

void GetTheEvent(EventRecord *theEvent, short waitTime)
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
	short			i;
	Point			thisPoint;
	short			index;
	unsigned long	dummy;
	WindowPtr		theWindow;
	Boolean			thisWindowIsOurs;
	
	thisWindowIsOurs=FALSE;
	/* for update/activate events, see if the window in question is one of ours */
	if ((theEvent.what==activateEvt) || (theEvent.what==updateEvt))
	{
		for (i=0; ((i<NUM_WINDOWS) && (!thisWindowIsOurs)); i++)
			thisWindowIsOurs=((WindowPtr)theEvent.message==GetIndWindowGrafPtr(i));
	}

	if (thisWindowIsOurs)	/* for activate/update events, get window index */
		index=(**((WindowDataHandle)GetWRefCon((WindowPtr)theEvent.message))).windowIndex;
	else if (gFrontWindowIsOurs)	/* if front window is ours, get its window index */
		index=gFrontWindowIndex;
	else index=-1;
	
	switch (theEvent.what)
	{
		case nullEvent:	/* ain't nuthin' happenin' */
			if ((gSoundAvailable) && (gSoundIsFinishedPlaying))
				CloseTheSoundChannel();
			if (gFrontWindowIsOurs)		/* give control to window dispatch to handle null */
				CallIndDispatchProc(index, kNull, 0L);
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
				CallIndDispatchProc(index, kKeydown, theEvent.message);
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
				UpdateTheWindow(GetIndWindowDataHandle(index));
			/* if not, could be our progress bar */
			else if ((theWindow!=0L) && (theWindow==gProgressDlog))
				UpdateDialog(theWindow, theWindow->visRgn);
			
			EndUpdate(theWindow);		/* means: "OK, we're done updating now" */
			break;
		case activateEvt:	/* window activate or deactivate */
			if (thisWindowIsOurs)		/* one of ours?  send message to window dispatch */
				CallIndDispatchProc(index, ((theEvent.modifiers&activeFlag)!=0) ?
					kActivate : kDeactivate, 0L);
			break;
		case osEvt:			/* suspend or resume program execution (switch in/out) */
			if (((theEvent.message>>24)&0x0FF)==suspendResumeMessage)
			{
				/* keep track of whether we're in the background or foreground */
				gIsInBackground=((theEvent.message&resumeFlag)==0);
				
				if (gFrontWindowIsOurs)		/* send activate/deactivate to front window */
					CallIndDispatchProc(index, gIsInBackground ? kDeactivate : kActivate, 0L);
				
				for (i=0; i<NUM_WINDOWS; i++)	/* send suspend/resume to all our */
					if (GetIndWindowGrafPtr(i)!=0L)		/* windows that currently exist */
						CallIndDispatchProc(i, gIsInBackground ? kSuspend : kResume, 0L);
				
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
	short				windowCode;
	long				windSize;
	GrafPtr				oldPort;
	short				i;
	Rect				sizeRect;
	Boolean				gotone;
	short				index;
	unsigned long		dummy;
	Point				theLocalPoint;
	Boolean				thisWindowIsOurs;
	ExtendedWindowDataHandle	theData;
	
	windowCode=FindWindow(theEvent.where, &theWindow);	/* which window? */

	thisWindowIsOurs=FALSE;
	/* find out if the target window was one of ours */
	if (theWindow!=0L)
		for (index=0; ((index<NUM_WINDOWS) && (!thisWindowIsOurs)); index++)
			thisWindowIsOurs=(theWindow==GetIndWindowGrafPtr(index));

	if (thisWindowIsOurs)	/* if target window is one of ours, get window data struct */
	{
		theData=(ExtendedWindowDataHandle)GetWRefCon(theWindow);
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
				theLocalPoint=theEvent.where;
				GlobalToLocal(&theLocalPoint);
//				GetMouse(&theLocalPoint);	/* in window's local coordinates */
				dummy=theLocalPoint.h;		/* all this fiddling is so we can pass */
				dummy=dummy<<16;			/* the point (two integers) to the */
				dummy+=theLocalPoint.v;		/* dispatch procedure in an unsigned long */
				CallIndDispatchProc(index, kMousedown, dummy);	/* go for it */
			}
			break;
		case inSysWindow:	/* in system window (desk accessory) */
			SystemClick(&theEvent, theWindow);	/* let the system deal with it */
			break;
		case inDrag:		/* in drag _region_, that is */
			/* the accepted way to draw a window */
			DragWindow(theWindow, theEvent.where, &((**GetGrayRgn()).rgnBBox));
			if (thisWindowIsOurs)	/* update window bounds in window data struct */
				(**theData).windowBounds=
					(*(((WindowPeek)GetIndWindowGrafPtr(index))->contRgn))->rgnBBox;
			break;
		case inGoAway:		/* close box */
			/* the accepted way to track a close box attempt */
			if (TrackGoAway(theWindow, theEvent.where))
				DoTheCloseThing((WindowPeek)theWindow);		/* see menus.c */
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
				(**theData).windowBounds=
					(*(((WindowPeek)GetIndWindowGrafPtr(index))->contRgn))->rgnBBox;
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
				(**theData).windowBounds=
					(*(((WindowPeek)GetIndWindowGrafPtr(index))->contRgn))->rgnBBox;
			break;
	}
}

void ShutDownEnvironment(void)
{
	SaveThePrefs();
	ShutDownTheProgram();		/* program-specific cleanup */
	ShutDownTheGraphics();		/* shell-specific clean-up */
	CloseTheSoundChannel();		/* dispose of sound channel, if exists */
}
