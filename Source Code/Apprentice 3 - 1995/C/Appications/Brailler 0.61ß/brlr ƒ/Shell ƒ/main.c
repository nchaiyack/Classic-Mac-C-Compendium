#define USE_MERCUTIO		0

#include "main.h"
#include "key layer.h"
#include "drag utilities.h"
#include "apple events.h"
#include "integrity.h"
#include "menus.h"
#include "prefs.h"
#include "environment.h"
#include "error.h"
#include "print meat.h"
#include "graphics.h"
#include "graphics dispatch.h"
#include "window layer.h"
#include "program globals.h"
#include "program init.h"
#include <CursorCtl.h>
#if USE_MERCUTIO
#include "Mercutio API.h"
#endif

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
	InitCursorCtl(0L);
	GetDateTime((unsigned long*)&qd.randSeed);
	
	InitTheDragManager();
	
	if (!InitTheEnvironment())			/* gestalt checks and variable initialization */
		HandleError(kSystemTooOld, TRUE, TRUE);		/* less than system 7.0 */
	
	if (!programIntegrityVerified)	/* integrity check failed */
		HandleError(kProgramIntegrityNotVerified, TRUE, FALSE);
	
	if (programIntegritySet)	/* integrity check freshly installed */
		HandleError(kProgramIntegritySet, FALSE, TRUE);
	
	PrefsError(PreferencesInit());	/* get prefs (create if necessary) */
	
	if (!InitTheMenus())		/* get menus from .rsrc and draw menu bar */
		HandleError(kProgramIntegrityNotVerified, TRUE, FALSE);
		
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
	
	gFrontWindowIndex=-1;
	front=FrontWindow();
	if (front!=0L)	/* if there's a front window, see if it's one of ours */
	{
		SetPort(front);
		gFrontWindowIndex=GetWindowIndex(front);
	}
		
	/* get an event from the queue */
	WaitNextEvent(everyEvent, &theEvent, gIsInBackground ? gBackgroundWaitTime : gForegroundWaitTime, 0L);
	SaveEventModifiers(&theEvent);
	
	DispatchEvents(theEvent, allowContextSwitching);	/* handle the event we just got */
	
	return (theEvent.what!=nullEvent);
}

void DispatchEvents(EventRecord theEvent, Boolean allowContextSwitching)
{
	Point			thisPoint;
	short			index;
	WindowPtr		theWindow;
	unsigned char	theChar;
	long			dummy;
	
	index=gFrontWindowIndex;
	
	switch (theEvent.what)
	{
		case nullEvent:	/* ain't nuthin' happenin' */
			if (gKludgeIter<3)
			{
				gKludgeIter++;
			}
			else
			{
				if (gNeedToOpenWindow)
				{
					OpenTheIndWindow(kMainWindow);
					gNeedToOpenWindow=FALSE;
				}
			}
			
			thisPoint=theEvent.where;
			GlobalToLocal(&thisPoint);
			theWindow=FrontWindow();
			if (WindowIsFloat(theWindow))
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
			break;
		case mouseDown:	/* mouse button pressed */
			HandleMouseDown(theEvent, allowContextSwitching);	/* see below for mousedown handling */
			break;
		case keyDown:	/* key pressed */
		case autoKey:	/* key help down */
			theChar=(char)(theEvent.message & charCodeMask);
			if (theEvent.modifiers & cmdKey)
			{
				AdjustMenus();
#if USE_MERCUTIO
				dummy=PowerMenuKey(theEvent.message, theEvent.modifiers, gBuildMenu);
				if (dummy==0L)
#endif
					dummy=MenuKey(theChar);
				HandleMenu(dummy);
			}
			else
			{
				theWindow=FrontWindow();
				if (WindowIsFloat(theWindow))
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
				ResetHiliteRgn(theWindow);
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
			UpdateTheWindow(theWindow);
			EndUpdate(theWindow);		/* means: "OK, we're done updating now" */
			break;
		case activateEvt:	/* window activate or deactivate */
			index=GetWindowIndex((WindowPtr)theEvent.message);
			if ((theEvent.modifiers & activeFlag)!=0)
				ActivateWindowDispatch(index);
			else
				DeactivateWindowDispatch(index);
			break;
		case osEvt:			/* suspend or resume program execution (switch in/out) */
			if (((theEvent.message>>24)&0x0FF)==suspendResumeMessage)
			{
				/* keep track of whether we're in the background or foreground */
				gIsInBackground=((theEvent.message&resumeFlag)==0);
				
				if (gIsInBackground)
					SuspendFloatingWindows();
				else
					ResumeFloatingWindows();
				
				/* if we just came into the foreground and we have a pending error,
				   now's the time to display it */
				if ((!gIsInBackground) && (gPendingErrorRec.resultCode!=allsWell))
				{
					if (gHasNotificationManager)
						NMRemove(&gPendingErrorRec.notificationRec);		/* remove notification request */
					HandleError(gPendingErrorRec.resultCode, gPendingErrorRec.isFatal,
						gPendingErrorRec.isSmall);	/* display alert, see error.c */
					gPendingErrorRec.resultCode=allsWell;		/* ...now it is */
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
	Rect			sizeRect;
	short			index;
	Point			theLocalPoint;
	
	windowCode=FindWindow(theEvent.where, &theWindow);	/* which window? */
	index=GetWindowIndex(theWindow);
	
	switch (windowCode)
	{
		case inMenuBar:		/* in menu bar; let system take over */
			AdjustMenus();
			HandleMenu(MenuSelect(theEvent.where));
			break;
		case inContent:		/* in window content */
			if (!DragInWindow(theWindow, &theEvent))
			{
				if (!MySelectWindow(theWindow))		/* didn't need to change front windows */
				{
					theLocalPoint=theEvent.where;
					SetPort(theWindow);
					GlobalToLocal(&theLocalPoint);
					MouseDownDispatch(index, theLocalPoint);
				}
			}
			ResetHiliteRgn(theWindow);
			break;
		case inSysWindow:	/* in system window (desk accessory) */
			if (allowContextSwitching)
				SystemClick(&theEvent, theWindow);	/* let the system deal with it */
			break;
		case inDrag:		/* in drag _region_, that is */
			MyDragWindow(theWindow, theEvent.where, &((**GetGrayRgn()).rgnBBox));
			/* update window bounds in window data struct */
			SetWindowBounds(theWindow,
				(*(((WindowPeek)theWindow)->contRgn))->rgnBBox);
			theLocalPoint.v=GetWindowBounds(theWindow).top;
			theLocalPoint.h=GetWindowBounds(theWindow).left;
			SetWindowTopLeft(theWindow, theLocalPoint);
			MySelectWindow(theWindow);
			break;
		case inGoAway:		/* close box */
			/* the accepted way to track a close box attempt */
			if (TrackGoAway(theWindow, theEvent.where))
				DoTheCloseThing((WindowPeek)theWindow);		/* see menus.c */
			break;
		case inGrow:		/* grow box */
			/* the accepted way to grow a window */
			if (GetGrowSizeDispatch(index, &sizeRect)==kFailure)
				sizeRect=qd.screenBits.bounds;
			
			windSize=GrowWindow(theWindow, theEvent.where, &sizeRect);
			if (windSize!=0)
			{
				SizeWindow(theWindow, LoWord(windSize), HiWord(windSize), TRUE);
				SetPort(theWindow);
				EraseRect(&(theWindow->portRect));
				InvalRect(&(theWindow->portRect));
				
				SetWindowBounds(theWindow,
					(*(((WindowPeek)theWindow)->contRgn))->rgnBBox);
				SetWindowWidth(theWindow, theWindow->portRect.right-theWindow->portRect.left);
				SetWindowHeight(theWindow, theWindow->portRect.bottom-theWindow->portRect.top);
				
				theLocalPoint.v=GetWindowBounds(theWindow).top;
				theLocalPoint.h=GetWindowBounds(theWindow).left;
				SetWindowTopLeft(theWindow, theLocalPoint);
				GrowWindowDispatch(index);
				
				ResetHiliteRgn(theWindow);
			}
			break;
		case inZoomIn:		/* zoom box */
		case inZoomOut:
			/* the accepted way to track a zoom attempt */
			if (TrackBox(theWindow, theEvent.where, windowCode))
			{
				ZoomWindow(theWindow, windowCode, FALSE);
				SetPort(theWindow);
				EraseRect(&(theWindow->portRect));
				InvalRect(&(theWindow->portRect));
				
				SetWindowWidth(theWindow, theWindow->portRect.right-theWindow->portRect.left);
				SetWindowHeight(theWindow, theWindow->portRect.bottom-theWindow->portRect.top);
				SetWindowBounds(theWindow,
					(*(((WindowPeek)theWindow)->contRgn))->rgnBBox);
				theLocalPoint.v=GetWindowBounds(theWindow).top;
				theLocalPoint.h=GetWindowBounds(theWindow).left;
				SetWindowTopLeft(theWindow, theLocalPoint);
				ZoomWindowDispatch(index);
				ResetHiliteRgn(theWindow);
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
		ShutDownTheMenus();			/* release menu resources */
	}
}
