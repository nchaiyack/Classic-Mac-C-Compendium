/********************************************************/
/*														*/
/*  EventTracker Code from Chapter Four of 				*/
/*														*/
/*	** The Macintosh C Programming Primer, 2nd Ed. **	*/
/*  													*/
/*	Copyright 1992, Dave Mark and Cartwright Reed   	*/
/*														*/
/*  This program demonstrates specific Mac programming	*/
/*	techniques.											*/
/*														*/	
/********************************************************/

#include <AppleEvents.h>
#include <GestaltEqu.h>
#include <limits.h>

#define kBaseResID			128
#define	kMoveToFront		(WindowPtr)-1L
#define kSleep				LONG_MAX

#define	kLeftMargin			4
#define kRowStart			285
#define	kFontSize			9
#define	kRowHeight			(kFontSize + 2)
#define	kHorizontalOffset	0

#define kGestaltMask		1L

/*************/
/*  Globals  */
/*************/

Boolean				gDone;
AEEventHandlerUPP	gDoOpenAppUPP,
					gDoOpenDocUPP,
					gDoPrintDocUPP,
					gDoQuitAppUPP;

/***************/
/*  Functions  */
/***************/

void			ToolBoxInit( void );
void			WindowInit( void );
void			EventInit( void );
void			EventLoop( void );
void			DoEvent( EventRecord *eventPtr );
pascal OSErr	DoOpenApp( AppleEvent theAppleEvent, AppleEvent reply, long refCon  );
pascal OSErr	DoOpenDoc( AppleEvent theAppleEvent, AppleEvent reply, long refCon  );
pascal OSErr	DoPrintDoc( AppleEvent theAppleEvent, AppleEvent reply, long refCon );
pascal OSErr	DoQuitApp( AppleEvent theAppleEvent, AppleEvent reply, long refCon  );
void			DrawEventString( Str255 eventString );
void			HandleMouseDown( EventRecord *eventPtr );


/******************************** main *********/

void	main( void )
{
	ToolBoxInit();
	WindowInit();
	EventInit();
	
	EventLoop();
}


/*********************************** ToolBoxInit */

void	ToolBoxInit( void )
{
	InitGraf( &qd.thePort );
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( 0L );
	InitCursor();
}


/******************************** WindowInit *********/

void	WindowInit( void )
{
	WindowPtr	window;
	
	window = GetNewWindow( kBaseResID, nil, kMoveToFront );
	
	if ( window == nil )
	{
		SysBeep( 10 );	/*  Couldn't load the WIND resource!!!  */
		ExitToShell();
	}
	
	SetPort( window );
	TextSize( kFontSize );
	
	ShowWindow( window );
}


/******************************** EventInit *********/

void	EventInit( void )
{
	OSErr	err;
	long	feature;
	
	err = Gestalt( gestaltAppleEventsAttr, &feature );
	
	if ( err != noErr )
	{
		DrawEventString( "\pproblem in calling Gestalt!" );
		return;
	}
	else
	{
		if ( !( feature & ( kGestaltMask << gestaltAppleEventsPresent ) ) )
		{
			DrawEventString( "\pApple events not available!" );
			return;
		}
	}
	
	gDoOpenAppUPP = NewAEEventHandlerProc( DoOpenApp );
	err = AEInstallEventHandler( kCoreEventClass, kAEOpenApplication,
				gDoOpenAppUPP, 0L, false );
	if ( err != noErr ) DrawEventString( "\pkAEOpenApplication Apple event not available!" );
	
	gDoOpenDocUPP = NewAEEventHandlerProc( DoOpenDoc );
	err = AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments,
				gDoOpenDocUPP, 0L, false );
	if ( err != noErr ) DrawEventString( "\pkAEOpenDocuments Apple event not available!" );
				
	gDoPrintDocUPP = NewAEEventHandlerProc( DoPrintDoc );
	err = AEInstallEventHandler( kCoreEventClass, kAEPrintDocuments,
				gDoPrintDocUPP, 0L, false );
	if ( err != noErr ) DrawEventString( "\pkAEPrintDocuments Apple event not available!" );
	
	gDoQuitAppUPP = NewAEEventHandlerProc( DoQuitApp );
	err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication,
				gDoQuitAppUPP, 0L, false );
	if ( err != noErr ) DrawEventString( "\pkAEQuitApplication Apple event not available!" );
}


/******************************** EventLoop *********/

void	EventLoop( void )
{		
	EventRecord		event;
	
	gDone = false;
	while ( gDone == false )
	{
		if ( WaitNextEvent( everyEvent, &event, kSleep, nil ) )
			DoEvent( &event );
		/*else DrawEventString( "\pnullEvent" );*/
		/*	Uncomment the previous line for a burst of flavor!  */
	}
}


/************************************* DoEvent	 */

void	DoEvent( EventRecord *eventPtr )
{
	switch ( eventPtr->what )
	{
		case kHighLevelEvent:
			DrawEventString( "\pHigh level event: " );
			AEProcessAppleEvent( eventPtr );
			break;
		case mouseDown: 
			DrawEventString( "\pmouseDown" );
			HandleMouseDown( eventPtr );
			break;
		case mouseUp: 
			DrawEventString( "\pmouseUp" );
			break;
		case keyDown:
			DrawEventString( "\pkeyDown" );
			break;
		case keyUp:
			DrawEventString( "\pkeyUp" );
			break;
		case autoKey:
			DrawEventString( "\pautoKey" );
			break;
		case updateEvt:
			DrawEventString( "\pupdateEvt" );
			BeginUpdate( (WindowPtr)eventPtr->message );
			EndUpdate( (WindowPtr)eventPtr->message );
			break;
		case diskEvt:
			DrawEventString( "\pdiskEvt" );
			break;
		case activateEvt:
			DrawEventString( "\pactivateEvt" );
			break;
		case networkEvt:
			DrawEventString( "\pnetworkEvt" );
			break;
		case driverEvt:
			DrawEventString( "\pdriverEvt" );
			break;
		case app1Evt:
			DrawEventString( "\papp1Evt" );
			break;
		case app2Evt:
			DrawEventString( "\papp2Evt" );
			break;
		case app3Evt:
			DrawEventString( "\papp3Evt" );
			break;
		case osEvt:
			DrawEventString( "\posEvt: " );
			if ( ( eventPtr->message & suspendResumeMessage ) == resumeFlag )
				DrawString( "\pResume event" );
			else
				DrawString( "\pSuspend event" );
			break;
	}
}

/************************************* DoOpenApp	 */

pascal OSErr	DoOpenApp( AppleEvent theAppleEvent, AppleEvent reply,
			long refCon )
{
	DrawString( "\pApple event: kAEOpenApplication" );
}


/************************************* DoOpenDoc	 */

pascal OSErr	DoOpenDoc( AppleEvent theAppleEvent, AppleEvent reply,
			long refCon )
{
	DrawString( "\pApple event: kAEOpenDocuments" );
}


/************************************* DoPrintDoc	 */

pascal OSErr	DoPrintDoc( AppleEvent theAppleEvent, AppleEvent reply,
			long refCon )
{
	DrawString( "\pApple event: kAEPrintDocuments" );
}


/************************************* DoQuitApp	 */

pascal OSErr	DoQuitApp( AppleEvent theAppleEvent, AppleEvent reply,
			long refCon )
{
	DrawString( "\pApple event: kAEQuitApplication" );
}

/************************************* DrawEventString *******/

void	DrawEventString( Str255 eventString )
{
	RgnHandle	tempRgn;
	WindowPtr	window;
	
	window = FrontWindow();
	tempRgn = NewRgn();
	ScrollRect( &window->portRect, kHorizontalOffset, -kRowHeight, tempRgn );
	DisposeRgn( tempRgn );

	MoveTo( kLeftMargin, kRowStart );
	DrawString( eventString );
}


/************************************* HandleMouseDown */

void	HandleMouseDown( EventRecord *eventPtr )
{
	WindowPtr	window;
	long		thePart;
	
	thePart = FindWindow( eventPtr->where, &window );
	
	switch ( thePart )
	{
		case inSysWindow : 
			SystemClick( eventPtr, window );
			break;
		case inDrag : 
			DragWindow( window, eventPtr->where, &qd.screenBits.bounds );
			break;
		case inGoAway : 
			gDone = true;
			break;
	}
}