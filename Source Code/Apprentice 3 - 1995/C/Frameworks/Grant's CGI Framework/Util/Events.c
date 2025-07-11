/*****
 *
 *	Events.c
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright �1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

#include <Threads.h>

#include "constants.h"
#include "globals.h"

#include "AboutBox.h"
#include "DebugUtil.h"
#include "ErrorUtil.h"
#include "MenuFunc.h"
#include "WindowInt.h"

#include "Events.h"


/***  LOCAL PROTOTYPES ***/

pascal	void *	doAEThread	( void * );
		void	doKeyPress	( EventRecord * );


/***  FUNCTIONS  ***/

/* 
	->message	long	the high-level event class to which the event belongs
	->when		long	time when event was posted (in ticks since system startup)
	->where		Point	the high-level event ID (cast as OSType)
	->modifiers	int		state of modifier keys and mouse button at event time */
void
doHighLevelEvent ( EventRecord *theEvent )
{
	OSErr		theErr;
	ThreadID	theThread;
	
	switch ( theEvent->message )
	{
		/* insert checks for high level events other than apple events */
		
		/* default type is apple event */
		default :
			if ( gHasThreadMgr )
			{
				theErr = NewThread ( kCooperativeThread, doAEThread, theEvent, nil,
					kFPUNotNeeded + kCreateIfNeeded,/*��� + kExactMatchThread + kUsePremadeThread,*/
					(void**)nil, &theThread );
			}
			
			if ( !gHasThreadMgr || (theErr != noErr) )
			{
				/* if threading isn't available, or the attempt to thread failed,
					process the Apple Event without threading */ 
				theErr = AEProcessAppleEvent ( theEvent );
			}
			
			if ( theErr != noErr )
			{
				/* you might want to add error handling here.
					However, I advise against putting user interaction handling here,
					it can confuse the user. You might try using an assertion. */
			}
			break;
	}
} /* doHighLevelEvent */


/* this is the function used to call the cgi processing as a thread */
pascal void *
doAEThread ( void *refCon )
{
	EventRecord *	theEvent;
	OSErr			theErr;
	
	theEvent	= (EventRecord *)refCon;
	
	theErr		= AEProcessAppleEvent ( theEvent );
	
	return (void *)theErr;
} /* doAEThread */


/* 
	->message	long	undefined
	->when		long	time when event was posted (in ticks since system startup)
	->where		Point	cursor location at event time
	->modifiers	int		state of modifier keys and mouse button at event time
						and wether the mouse-down caused the application to come to the foreground
	IM-MTE: 2-34 */
void
doMouseDown ( EventRecord *theEvent )
{
	WindowPtr 		theWindow;
	short  			where;
	
	where = FindWindow ( theEvent->where, &theWindow );
	
	switch ( where ) /* where the event occurred */
	{
		case inDesk :
			break;
		
		case inMenuBar :
			adjustMenus	();
			doMenu		( MenuSelect(theEvent->where), theEvent->modifiers );
			break;
		
		case inSysWindow :
			/* click in a desk accessory window */
			SystemClick ( theEvent, theWindow );
			break;
	
		case inContent :
			/* content region of a window */
			if ( (theWindow != FrontWindow()) || !(((WindowPeek)theWindow)->hilited) )
			{
				switch ( WindowType(FrontWindow()) )
				{
					case Window_dlgModal :
					case Window_dlgMoveableModal :
						/* beep because can't switch from modal dialog */
						SysBeep ( 30 );
						break;
					
					case Window_UNKNOWN :
						my_assert ( kForceAssert, "\pdoMouseDown: Unknown window type" );
					
					case Window_about :
					case Window_DA :
					default :
						SelectWindow ( theWindow );
						break;
				}
			}
			else
			{
				WindowContentClick ( theWindow, theEvent );
			}
			break;

		case inDrag :
			/* title bar region of a window */
			switch ( WindowType(theWindow) )
			{
				case Window_dlgMoveableModal :
					if ( (theWindow == FrontWindow()) || (theEvent->modifiers & cmdKey) )
					{
						DragWindow (theWindow, theEvent->where, &gGrayRgnRect);
					}
					else
					{
						/* beep because can't switch from moveable modal dialog */
						SysBeep ( 30 );
					}
					break;
				
				default :
					/* if the command key is pressed, don't activate the window, just drag it */
					if ( !(theEvent->modifiers & cmdKey) )
					{
						SelectWindow ( theWindow );
					}
					
					DragWindow ( theWindow, theEvent->where, &gGrayRgnRect );
					break;
			}
			break;
	
		case inGrow :
			/* grow box region of a window (lower right hand corner) */
			WindowGrow ( theWindow, theEvent->where );
			break;
			
		case inZoomIn:
		case inZoomOut:
			/* zoom box of a window (upper right hand corner) */
			if ( TrackBox(theWindow, theEvent->where, where) )
			{
				WindowZoomBox ( theWindow, where );
			}
			break;
	
		case inGoAway :
			/* close box of a window (upper left hand corner) */
			if ( TrackGoAway ( theWindow, theEvent->where ) )
			{
				WindowClose ( theWindow, theEvent->modifiers, true );
			}
			break;
	}
} /* doMouseDown */


/* 
	->message	long	undefined
	->when		long	time when event was posted (in ticks since system startup)
	->where		Point	cursor location at event time
	->modifiers	int		state of modifier keys and mouse button at event time */
void
doMouseUp ( EventRecord *theEvent )
{
	
} /* doMouseUp */


/* 
	->message	long	char and virtual key codes in low-order word.
						ADB address in low byte of high-order word
	->when		long	time when event was posted (in ticks since system startup)
	->where		Point	cursor location at event time
	->modifiers	int		state of modifier keys and mouse button at event time
	IM-MTE 2-44 */
void
doKeyDown ( EventRecord *theEvent )
{
	char	theChar;
	
	/* the character being pressed is in the message of the event record */
	theChar = theEvent->message & charCodeMask;
	
	if ( theEvent->modifiers & cmdKey )
	{
		/* if the command key is being held down, treat as menu request */
		adjustMenus	();
		doMenu		( MenuKey (theChar), theEvent->modifiers );
	}
	else
	{
		/* handle regular keypress */
		doKeyPress ( theEvent );
	}
} /* doKeyDown */


/* 
	->message	long	char and virtual key codes in low-order word.
						ADB address in low byte of high-order word
	->when		long	time when event was posted (in ticks since system startup)
	->where		Point	cursor location at event time
	->modifiers	int		state of modifier keys and mouse button at event time */
void
doAutoKey ( EventRecord *theEvent )
{
	if ( theEvent->modifiers & cmdKey )
	{
		/* don't do anything. Command keys should not auto-repeat. */
	}
	else
	{
		/* handle regular keypress */
		doKeyPress ( theEvent );
	}
} /* doAutoKey */


/* 
	->message	long	char and virtual key codes in low-order word.
						ADB address in low byte of high-order word
	->when		long	time when event was posted (in ticks since system startup)
	->where		Point	cursor location at event time
	->modifiers	int		state of modifier keys and mouse button at event time */
void
doKeyUp ( EventRecord *theEvent )
{
	
} /* doKeyUp */


/* handle regular keypress
	->message	long	char and virtual key codes in low-order word.
						ADB address in low byte of high-order word
	->when		long	time when event was posted (in ticks since system startup)
	->where		Point	cursor location at event time
	->modifiers	int		state of modifier keys and mouse button at event time
	IM-MTE: 2-44,45 */
void
doKeyPress ( EventRecord *theEvent )
{
	/* If you add windows that allow for text entry or you accept non-command-key
		keyboard input, you need to add code here. Otherwise, leave it blank. */
} /* doKeyPress */


/* Handle activate event - a window has been brought to front, or a window
	that was in front is no longer.
	->message	long	window ptr for window to de/activate
	->when		long	time when event was posted (in ticks since system startup)
	->where		Point	cursor location at event time
	->modifiers	int		state of modifier keys and mouse button at event time
						and whether window should be activated or deactivated
	IM-MTE: 2-50-55 */
void
doActivateEvent ( EventRecord *theEvent )
{
	WindowPtr	theWindow;
	Boolean		becomingActive;
	
	theWindow		= (WindowPtr) theEvent->message;
	becomingActive	= (theEvent->modifiers & activeFlag) != nil;
	
	switch ( WindowType(theWindow) )
	{
		case Window_none :
			break;
		
		case Window_UNKNOWN :
			my_assert ( kForceAssert, "\pdoActivateEvent: Unknown window type" );
		
		default :
			WindowActivate ( theWindow, becomingActive, theEvent );
			break;
	}
} /* doActivateEvent */


/* A window needs to be updated.
	->message	long	window ptr for window to update
	->when		long	time when event was posted (in ticks since system startup)
	->where		Point	cursor location at event time
	->modifiers	int		state of modifier keys and mouse button at event time
	IM-MTE: 2-47-50 */
void
doUpdateEvent ( EventRecord *theEvent )
{
	GrafPtr			oldPort;
	WindowPtr		theWindow;
	window_type		windowType;
	
	theWindow	= (WindowPtr) (theEvent->message);
	windowType	= WindowType ( theWindow );
	
	if ( !(windowType == Window_DA) )
	{
		GetPort		( &oldPort );
		SetPort		( (GrafPtr)theWindow );
		BeginUpdate	( theWindow );

		switch ( windowType )
		{
			case Window_about :
				AboutBoxUpdate ();
				break;
			
			case Window_UNKNOWN :
				my_assert ( kForceAssert, "\pdoUpdateEvent: Unknown window type" );
			
			case Window_dlgModal :
			case Window_dlgMoveableModal :
			case Window_none :
			default :
				/* don't do anything */
				break;
		}
		
		EndUpdate	( theWindow );
		SetPort		( oldPort );
	}
} /* doUpdateEvent */


/*
	->message	long	window ptr for window to de/activate
	->when		long	time when event was posted (in ticks since system startup)
	->where		Point	cursor location at event time
	->modifiers	int		state of modifier keys and mouse button at event time
						and whether window should be activated or deactivated
	IM-MTE: 2-58-62 */
void
doOsEvt ( const EventRecord *theEvent )
{
	WindowPtr	theWindow;
	
	switch ( (theEvent->message >> 24) & 0x000000FF )	/* should have a constant here */
	{
		case suspendResumeMessage :
			gFrontProcess = theEvent->message & resumeFlag;
			
			theWindow = FrontWindow ();
			
			if ( gFrontProcess )
			{
				/* resume event */
				
				if ( theEvent->message & convertClipboardFlag )
				{
					/* convert the clipboard */
				}
				
				switch ( WindowType(theWindow) )
				{
					case Window_none :
						break;
					
					case Window_UNKNOWN :
						my_assert ( kForceAssert, "\pdoOsEvt: Unknown window type" );
					
					default :
						WindowActivate ( theWindow, true, theEvent );
						break;
				}
			}
			else
			{
				/* suspend event */
				/* convert the clipboard, if needed, for export */
				
				switch ( WindowType(theWindow) )
				{
					case Window_none :
						break;
					
					case Window_UNKNOWN :
						my_assert ( kForceAssert, "\pdoOsEvt: Unknown window type" );
					
					default :
						WindowActivate ( theWindow, false, theEvent );
						break;
				}
			}
			break;
		
		case mouseMovedMessage : /* IM-MTE: 2-62-67 */
			/* you may want to adjust your cursor and mouse region here */
			break;
	}
} /* doOsEvt */


/* Handle 'bad' disk mounts. Just call the system DIBadMount call.
	->message	long	drive number in low-order word, file manager result code in high-order word
	->when		long	time when event was posted (in ticks since system startup)
	->where		Point	cursor location at event time
	->modifiers	int		state of modifier keys and mouse button at event time
	IM-MTE:2-55,56 */
void
doDiskEvt ( EventRecord *theEvent )
{
	Point	aPoint;
	OSErr	theErr;
	
	/* if disk mount was unsuccessful */
	if ( HiWord(theEvent->message) != noErr )
	{
		/* load disk initialization manager */
		DILoad ();
		
		aPoint.v	= 120;
		aPoint.h	= 120;
		theErr		= DIBadMount ( aPoint, theEvent->message );
		
		/* unload disk initialization manager */
		DIUnload ();
	}
} /* doDiskEvt */


/* 
	->message	long	undefined
	->when		long	time when event was posted (in ticks since system startup)
	->where		Point	cursor location at event time
	->modifiers	int		state of modifier keys and mouse button at event time
	IM-MTE: 2-57,58, 6-79,80 */
void
doIdle ( EventRecord *theEvent )
{
	WindowPtr	theWindow;
	short		itemHit;
	Boolean		theResult;
	
	if ( gHasThreadMgr )
	{
		YieldToAnyThread ();
	}
	
	if ( gFrontProcess )
	{
		theWindow = FrontWindow ();
		
		switch ( WindowType(theWindow) )
		{
			case Window_about :
			case Window_none :
				/* don't do anything */
				break;
			
			case Window_dlgModal :
			case Window_dlgMoveableModal :
				/* IM:MTB 2-29,30 */
				theResult = DialogSelect ( theEvent, (DialogPtr *)&theWindow, &itemHit );
				break;
			
			case Window_UNKNOWN :
				my_assert ( kForceAssert, "\pdoIdle: unknown window type" );
			
			default :
				break;
		}
	}
} /* doIdle */


/*****  EOF  *****/
