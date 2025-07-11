#include "main.h"
#include "program globals.h"
#include "key layer.h"
#if USE_DRAG
#include "drag layer.h"
#endif
#include "integrity.h"
#include "menus.h"
#include "prefs.h"
#include "environment.h"
#include "error.h"
#if USE_PRINTING
#include "printing layer.h"
#endif
#include "graphics dispatch.h"
#if USE_SOUNDS
#include "sound layer.h"
#endif
#if USE_MUSIC
#include "music layer.h"
#endif
#if USE_SPEECH
#include "speech layer.h"
#endif
#include "window layer.h"
#include "program init.h"
#if USE_MERCUTIO
#include "Mercutio API.h"
#endif

void main(void)
{
	Boolean			programIntegrityVerified;
	Boolean			programIntegritySet;
	ErrorTypes		resultCode;
	
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
	
	/* do integrity check before anything else; see integrity.c for details */
	programIntegrityVerified=DoIntegrityCheck(&programIntegritySet);
	
	if ((resultCode=InitTheEnvironment())!=allsWell)
		HandleError(resultCode, TRUE, TRUE);
	
	if (!programIntegrityVerified)	/* integrity check failed */
		HandleError(kProgramIntegrityNotVerified, TRUE, FALSE);
	
	if (programIntegritySet)	/* integrity check freshly installed */
		HandleError(kProgramIntegritySet, FALSE, TRUE);
	
#if USE_DRAG
	InitTheDragManager();
#endif
#if USE_SOUNDS
	InitTheSounds();
#endif
#if USE_MUSIC
	InitTheMusic();
#endif
#if USE_SPEECH
	InitTheSpeech();
#endif

	PrefsError(PreferencesInit());	/* get prefs (create if necessary) */
	
	if (!InitTheMenus())		/* get menus from .rsrc and draw menu bar */
		HandleError(kProgramIntegrityNotVerified, TRUE, FALSE);
		
#if USE_PRINTING
	InitThePrinting();
#endif
	
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
	WindowRef		front;
	
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
	WindowRef		theWindow;
	unsigned char	theChar;
	long			dummy;
	
	switch (theEvent.what)
	{
		case nullEvent:	/* ain't nuthin' happenin' */
#if USE_SOUNDS
			if ((gSoundAvailable) && (gSoundIsFinishedPlaying))
				CloseTheSoundChannel();
#endif
			if (gKludgeIter<3)
			{
				gKludgeIter++;
			}
			else
			{
				if (gNeedToOpenWindow)
				{
					OpenTheIndWindow(kMainWindow, kAlwaysOpenNew);
					gNeedToOpenWindow=FALSE;
				}
			}
			
			thisPoint=theEvent.where;
			GlobalToLocal(&thisPoint);
			theWindow=FrontWindow();
			if (WindowIsFloat(theWindow))
			{
				if (IdleWindowDispatch(theWindow, thisPoint)==kPassThrough)
				{
					if ((theWindow=GetFrontDocumentWindow())!=0L)
					{
						SetPort(theWindow);
						thisPoint=theEvent.where;
						GlobalToLocal(&thisPoint);
						IdleWindowDispatch(theWindow, thisPoint);
					}
				}
			}
			else
			{
				IdleWindowDispatch(theWindow, thisPoint);
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
				if ((theChar=='�') || (theChar=='�'))
				{	/* hack to map cmd-(option-w) to cmd-w + option key */
					theEvent.modifiers|=optionKey;
					SaveEventModifiers(&theEvent);
					theChar='w';
				}
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
					if (KeyDownDispatch(theWindow, theChar)==kPassThrough)
					{
						if ((theWindow=GetFrontDocumentWindow())!=0L)
						{
							KeyDownDispatch(theWindow, theChar);
						}
					}
				}
				else
				{
					KeyDownDispatch(theWindow, theChar);
				}
			}
#if USE_DRAG
			ResetHiliteRgn(theWindow);
#endif
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
			theWindow=(WindowRef)theEvent.message;	/* which window? */
			BeginUpdate(theWindow);		/* means: "OK, we're dealing with this now" */
			UpdateTheWindow(theWindow);
			EndUpdate(theWindow);		/* means: "OK, we're done updating now" */
			break;
		case activateEvt:	/* window activate or deactivate */
			theWindow=(WindowRef)theEvent.message;
			if ((theEvent.modifiers & activeFlag)!=0)
				ActivateWindowDispatch(theWindow);
			else
				DeactivateWindowDispatch(theWindow);
			break;
		case osEvt:			/* suspend or resume program execution (switch in/out) */
			if (((theEvent.message>>24)&0x0FF)==suspendResumeMessage)
			{
				/* keep track of whether we're in the background or foreground */
				gIsInBackground=((theEvent.message&resumeFlag)==0);
				
				if (gIsInBackground)
				{
					SuspendFloatingWindows();
#if USE_MUSIC
					CloseTheMusicChannel();
#endif
				}
				else
				{
					ResumeFloatingWindows();
#if USE_MUSIC
					if ((gMusicStatus==kMusicAlways) ||
						((gMusicStatus==kMusicGeneratingOnly) && (gInProgress)))
					{
						StartTheMusic();
					}
#endif
				}
				
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
	WindowRef		theWindow;
	short			windowCode;
	Point			theLocalPoint;
	
	windowCode=FindWindow(theEvent.where, &theWindow);	/* which window? */
	
	switch (windowCode)
	{
		case inMenuBar:		/* in menu bar; let system take over */
			AdjustMenus();
			HandleMenu(MenuSelect(theEvent.where));
			break;
		case inContent:		/* in window content */
#if USE_DRAG
			if (!DragInWindow(theWindow, &theEvent))
			{
#endif
				if (!MySelectWindow(theWindow))		/* didn't need to change front windows */
				{
					theLocalPoint=theEvent.where;
					SetPort(theWindow);
					GlobalToLocal(&theLocalPoint);
					MouseDownDispatch(theWindow, theLocalPoint);
				}
#if USE_DRAG
			}
			ResetHiliteRgn(theWindow);
#endif
			break;
		case inSysWindow:	/* in system window (desk accessory) */
			if (allowContextSwitching)
				SystemClick(&theEvent, theWindow);	/* let the system deal with it */
			break;
		case inDrag:		/* in drag _region_, that is */
			MyDragWindow(theWindow, theEvent.where, &((**GetGrayRgn()).rgnBBox));
			break;
		case inGoAway:		/* close box */
			/* the accepted way to track a close box attempt */
			if (TrackGoAway(theWindow, theEvent.where))
				DoTheCloseThing((WindowPeek)theWindow);		/* see menus.c */
			break;
		case inGrow:		/* grow box */
			MyGrowWindow(theWindow, theEvent.where);
			break;
		case inZoomIn:		/* zoom box */
		case inZoomOut:
			/* the accepted way to track a zoom attempt */
			if (TrackBox(theWindow, theEvent.where, windowCode))
			{
				MyZoomWindow(theWindow, windowCode);
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
#if USE_SOUNDS
	CloseTheSoundChannel();
#endif
#if USE_MUSIC
	CloseTheMusicChannel();
#endif
}
