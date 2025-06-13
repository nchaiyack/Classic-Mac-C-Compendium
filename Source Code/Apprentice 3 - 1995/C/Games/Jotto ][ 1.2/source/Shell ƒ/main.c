#include "main.h"
#include "apple events.h"
#include "integrity.h"
#include "menus.h"
#include "prefs.h"
#include "environment.h"
#include "error.h"
#include "print meat.h"
#include "sounds.h"
#include "jotto.h"
#include "graphics.h"
#include "graphics dispatch.h"
#include "window layer.h"
#include "program globals.h"
#include <AppleEvents.h>
#include <EPPC.h>

static	short			gTheCurrentModifiers;

void main(void)
{
	Boolean			programIntegrityVerified;
	Boolean			programIntegritySet;
	
	/* do integrity check before anything else; see integrity.c for details */
	programIntegrityVerified=DoIntegrityCheck(&programIntegritySet);
	
	/* standard program initialization stuff */
	MaxApplZone();	
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	GetDateTime((unsigned long*)&qd.randSeed);
	
	InitTheWindowLayer();
	
	if (!InitTheEnvironment())			/* gestalt checks and variable initialization */
		HandleError(kSystemTooOld, TRUE);		/* less than system 4.1 */
	
	if (!programIntegrityVerified)	/* integrity check failed */
		HandleError(kProgramIntegrityNotVerified, TRUE);
	
	if (programIntegritySet)	/* integrity check freshly installed */
		HandleError(kProgramIntegritySet, FALSE);
	
	InitTheSounds();				/* see if sound is available, etc. */
	
	PrefsError(PreferencesInit());	/* get prefs (create if necessary) */
	
	if (!InitTheMenus())		/* get menus from .rsrc and draw menu bar */
		HandleError(kProgramIntegrityNotVerified, TRUE);
		
	InitThePrinting();
	
	InitTheProgram();
	
	EventLoop();					/* where it all happens (see below) */
	
	ShutDownEnvironment(TRUE);		/* where it all ends (see below) */
	
	ExitToShell();
}

void EventLoop(void)
{
	while (!gDone)	/* gDone set by choosing "Quit" menu item or by "quit" apple event */
		HandleSingleEvent(TRUE);
}

Boolean HandleSingleEvent(Boolean allowContextSwitching)
{
	EventRecord		theEvent;
	WindowPtr		front;
	
	if (!gCustomCursor)
		SetCursor(&qd.arrow);
	HiliteMenu(0);			/* normalize menubar */
	
	gFrontWindowIndex=0;
	gFrontWindowIsOurs=FALSE;
	front=FrontWindow();
	if (front!=0L)	/* if there's a front window, see if it's one of ours */
	{
		if (WindowHasLayer(front))
		{
			SetPort(front);
			gFrontWindowIsOurs=TRUE;
			gFrontWindowIndex=GetWindowIndex(front);
		}
	}
		
	/* get an event from the queue */
	WaitNextEvent(everyEvent, &theEvent, gIsInBackground ? gBackgroundWaitTime : gForegroundWaitTime, 0L);
	gTheCurrentModifiers=theEvent.modifiers;
	
	DispatchEvents(theEvent, allowContextSwitching);	/* handle the event we just got */
	
	return (theEvent.what!=nullEvent);
}

short GetTheModifiers(void)
{
	return gTheCurrentModifiers;
}

void DispatchEvents(EventRecord theEvent, Boolean allowContextSwitching)
{
	Point			thisPoint;
	short			index;
	WindowPtr		theWindow;
	Boolean			thisWindowIsOurs;
	unsigned char	theChar;
	
	thisWindowIsOurs=FALSE;
	/* for update/activate events, see if the window in question is one of ours */
	if ((theEvent.what==activateEvt) || (theEvent.what==updateEvt))
	{
		thisWindowIsOurs=WindowHasLayer((WindowPtr)theEvent.message);
		if (thisWindowIsOurs)
			index=GetWindowIndex((WindowPtr)theEvent.message);
	}
	else if (gFrontWindowIsOurs)	/* if front window is ours, get its window index */
		index=gFrontWindowIndex;
	else index=-1;
	
	switch (theEvent.what)
	{
		case nullEvent:	/* ain't nuthin' happenin' */
			if ((gSoundAvailable) && (gSoundIsFinishedPlaying))
				CloseTheSoundChannel();
			
			if (gKludgeIter<3)
			{
				gKludgeIter++;
			}
			else
			{
				if (gNeedToOpenWindow)
				{
					NewGame();
					gNeedToOpenWindow=FALSE;
				}
			}
			
			if (gFrontWindowIsOurs)		/* give control to window dispatch to handle null */
			{
				thisPoint=theEvent.where;
				GlobalToLocal(&thisPoint);
				theWindow=FrontWindow();
				if ((WindowHasLayer(theWindow)) && (WindowIsFloat(theWindow)))
				{
					if (IdleWindowDispatch(index, thisPoint)==kPassThrough)
					{
						if ((theWindow=GetFrontDocumentWindow())!=0L)
						{
							index=GetWindowIndex(theWindow);
							SetPort(theWindow);
							thisPoint=theEvent.where;
							GlobalToLocal(&thisPoint);
							IdleWindowDispatch(index, thisPoint);
						}
					}
				}
				else
				{
					IdleWindowDispatch(index, thisPoint);
				}
			}
			break;
		case mouseDown:	/* mouse button pressed */
			HandleMouseDown(theEvent, allowContextSwitching);	/* see below for mousedown handling */
			break;
		case keyDown:	/* key pressed */
		case autoKey:	/* key help down */
			theChar=(unsigned char)(theEvent.message & charCodeMask);
			
			if (theEvent.modifiers & cmdKey)	/* handle as command-key equivalent */
			{
				AdjustMenus();	/* just to be safe */
				HandleMenu(MenuKey(theChar));
			}
			else if (gFrontWindowIsOurs)	/* --> window's dispatch for keydown */
			{
				theWindow=FrontWindow();
				if ((WindowHasLayer(theWindow)) && (WindowIsFloat(theWindow)))
				{
					if (KeyDownDispatch(index, theChar)==kPassThrough)
					{
						if ((theWindow=GetFrontDocumentWindow())!=0L)
						{
							index=GetWindowIndex(theWindow);
							KeyDownDispatch(index, theChar);
						}
					}
				}
				else
				{
					KeyDownDispatch(index, theChar);
				}
			}
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
				UpdateTheWindow(theWindow);
			
			EndUpdate(theWindow);		/* means: "OK, we're done updating now" */
			break;
		case activateEvt:	/* window activate or deactivate */
			if (thisWindowIsOurs)		/* one of ours?  send message to window dispatch */
			{
				if (gIgnoreNextActivateEvent)
					gIgnoreNextActivateEvent=FALSE;
				else
				{
					if ((theEvent.modifiers & activeFlag)!=0)
						ActivateWindowDispatch(index);
					else
						DeactivateWindowDispatch(index);
				}
			}
			break;
		case osEvt:			/* suspend or resume program execution (switch in/out) */
			if (((theEvent.message>>24)&0x0FF)==suspendResumeMessage)
			{
				/* keep track of whether we're in the background or foreground */
				gIsInBackground=((theEvent.message&resumeFlag)==0);
				
				if (gFrontWindowIsOurs)		/* send activate/deactivate to front window */
				{
					if (gIsInBackground)
						DeactivateWindowDispatch(index);
					else
						ActivateWindowDispatch(index);
				}
				
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
			AEProcessAppleEvent(&theEvent);		/* see apple events.c */
			break;
	}
}

void HandleMouseDown(EventRecord theEvent, Boolean allowContextSwitching)
{
	WindowPtr		theWindow;
	short			windowCode;
	long			windSize;
	GrafPtr			oldPort;
	Rect			sizeRect;
	short			index;
	Point			theLocalPoint;
	Boolean			thisWindowIsOurs;
	long			mSelect;
	
	windowCode=FindWindow(theEvent.where, &theWindow);	/* which window? */

	thisWindowIsOurs=FALSE;
	thisWindowIsOurs=WindowHasLayer(theWindow);
	if ((thisWindowIsOurs=WindowHasLayer(theWindow))==TRUE)
		index=GetWindowIndex(theWindow);
	else
		index=-1;
	
	switch (windowCode)
	{
		case inMenuBar:		/* in menu bar; let system take over */
			AdjustMenus();
			mSelect=MenuSelect(theEvent.where);
//			mSelect=ManualMenuSelect(2);
			HandleMenu(mSelect);
			break;
		case inContent:		/* in window content */
			if (!MySelectWindow(theWindow))		/* didn't need to change front windows */
			{
				if (gFrontWindowIsOurs)	/* inform window dispatch of mousedown */
				{
					theLocalPoint=theEvent.where;
					SetPort(theWindow);
					GlobalToLocal(&theLocalPoint);
					MouseDownDispatch(index, theLocalPoint);
				}
			}
			break;
		case inSysWindow:	/* in system window (desk accessory) */
			if (allowContextSwitching)
				SystemClick(&theEvent, theWindow);	/* let the system deal with it */
			break;
		case inDrag:		/* in drag _region_, that is */
			/* the accepted way to draw a window */
			DragWindow(theWindow, theEvent.where, &((**GetGrayRgn()).rgnBBox));
			if (thisWindowIsOurs)	/* update window bounds in window data struct */
			{
				SetWindowBounds(theWindow,
					(*(((WindowPeek)theWindow)->contRgn))->rgnBBox);
				theLocalPoint.v=GetWindowBounds(theWindow).top;
				theLocalPoint.h=GetWindowBounds(theWindow).left;
				SetWindowTopLeft(theWindow, theLocalPoint);
				MySelectWindow(theWindow);
			}
			break;
		case inGoAway:		/* close box */
			/* the accepted way to track a close box attempt */
			if (TrackGoAway(theWindow, theEvent.where))
				DoTheCloseThing((WindowPeek)theWindow);		/* see menus.c */
			break;
		case inGrow:		/* grow box */
			/* the accepted way to grow a window */
			if (thisWindowIsOurs)
			{
				if (GetGrowSizeDispatch(index, &sizeRect)==kFailure)
					sizeRect=qd.screenBits.bounds;
			}
			else sizeRect=qd.screenBits.bounds;
			
			windSize=GrowWindow(theWindow, theEvent.where, &sizeRect);
			if (windSize!=0)
			{
				GetPort(&oldPort);
				SetPort(theWindow);
				EraseRect(&theWindow->portRect);
				SizeWindow(theWindow, LoWord(windSize), HiWord(windSize), TRUE);
				InvalRect(&theWindow->portRect);
				SetPort(oldPort);
				
				if (thisWindowIsOurs)	/* update window bounds in window data struct */
				{
					GrowWindowDispatch(index);
					SetWindowBounds(theWindow,
						(*(((WindowPeek)theWindow)->contRgn))->rgnBBox);
					theLocalPoint.v=GetWindowBounds(theWindow).top;
					theLocalPoint.h=GetWindowBounds(theWindow).left;
					SetWindowTopLeft(theWindow, theLocalPoint);
				}
			}
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
			{
				ZoomWindowDispatch(index);
				SetWindowBounds(theWindow,
					(*(((WindowPeek)theWindow)->contRgn))->rgnBBox);
				theLocalPoint.v=GetWindowBounds(theWindow).top;
				theLocalPoint.h=GetWindowBounds(theWindow).left;
				SetWindowTopLeft(theWindow, theLocalPoint);
			}
			break;
	}
}

void ShutDownEnvironment(Boolean fullShutdown)
{
	SaveThePrefs();
	if (fullShutdown)
	{
		ShutDownTheProgram();		/* program-specific cleanup */
		ShutDownTheWindowLayer();	/* do shutdown dispatch for all windows we've used */
		ShutDownTheMenus();			/* release menu resources */
	}
}
