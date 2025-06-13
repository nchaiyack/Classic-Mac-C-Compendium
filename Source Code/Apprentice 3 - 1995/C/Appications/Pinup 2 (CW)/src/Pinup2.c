/* pinup_II.c */

/* by Darrell Anderson
 * shows PICT ID 128 in a windoid
 * remembers last position.
 */

#include "errorDLOG.h"

#define kBaseResID			128
#define kMoveToFront		(WindowPtr)-1L

#define mApple				kBaseResID
#define iAbout				1

#define mFile				kBaseResID+1
#define iQuit				1

#define k_PICT_ID			128
#define k_WIND_ID			128

#define MAXLONG				32768

/*************************************************//* GLOBALS */

Boolean		gDone;
CWindowPtr	gWindow;
PicHandle	gPicture;
Handle		gCoordsRectH;

/*************************************************//* FUNCTION HEADERS */

void		ToolBoxInit( void );
void		WindowInit( void );
void		MenuBarInit( void );
void		EventLoop( void );
void		DoEvent( EventRecord *eventPtr );
void		HandleMouseDown( EventRecord *eventPtr );
void		HandleMenuChoice( long menuChoice );
void		HandleAppleChoice( short item );
void		HandleFileChoice( short item );
void		DoUpdate( EventRecord *eventPtr );
void 		DoPicture( WindowPtr window, PicHandle picture );
void 		DoUpdate( EventRecord *eventPtr );
void		DoAboutSelected( void );
void		AtQuit( void );

/*************************************************//* main */
void main( void )
{
	ToolBoxInit();
	WindowInit();
	MenuBarInit();
	
	EventLoop();
	
	AtQuit();
}

/*************************************************//* ToolBoxInit */
void ToolBoxInit( void )
{
	InitGraf( &qd.thePort );
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( nil );
	InitCursor();
}

/*************************************************//* WindowInit */
void WindowInit( void )
{
	Rect			windRect;
	short			*shortPtr;
	
	// get the picture!
	gPicture = GetPicture( k_PICT_ID );
	if( gPicture == nil )
		ErrorDLOG( "Can't load PICT 128! Make sure it's there and/or increase my memory allocation", true );

	HNoPurge( (Handle)gPicture );

	// get the window, size for pict!
	gWindow = (CWindowPtr)GetNewCWindow( k_WIND_ID, nil, kMoveToFront );
	
	if( gWindow == nil )
		ErrorDLOG( "Couldn't open window, try increasing my memory allocation.", true );

	windRect = (**(gPicture)).picFrame;		
	SizeWindow( (WindowPtr)gWindow, (windRect.right - windRect.left), (windRect.bottom - windRect.top),
			false );
			
	// move to last known location!
	gCoordsRectH = GetResource( 'data', 128 );
	HLock( gCoordsRectH );
	if( gCoordsRectH == nil )
		ErrorDLOG( "Can't load coordinates!  Sorry!", true );
		
	shortPtr = (short *) (*gCoordsRectH);
	MoveWindow( (WindowPtr)gWindow, *shortPtr, *(shortPtr+1), false );
	
	HUnlock( gCoordsRectH );
		
	// set the picture to the window so the system will deal with redrawing it!
//	SetWindowPic( gWindow, gPicture );
									
	// at this point, gWindow and gPicture are valid.		
	ShowWindow( (WindowPtr)gWindow );
}

/*************************************************//* MenuBarInit */
void MenuBarInit( void )
{
	Handle			menuBar;
	MenuHandle		menu;
	ControlHandle	control;
	OSErr			myErr;
	long			feature;
	
	menuBar = GetNewMBar( kBaseResID );
	SetMenuBar( menuBar );
	
	menu = GetMHandle( mApple );
	AddResMenu( menu, 'DRVR' );
	
	DrawMenuBar();
}

/*************************************************//* EventLoop */
void EventLoop( void )
{
	EventRecord		event;
	
	gDone = false;
	
	while( gDone == false )
	{
		if( WaitNextEvent( everyEvent, &event, MAXLONG, nil ))
			DoEvent( &event );
	}
}

/*************************************************//* DoEvent */
void DoEvent( EventRecord *eventPtr )
{
	char		theChar;
	
	switch( eventPtr->what )
	{
		case mouseDown:
			HandleMouseDown( eventPtr );
			break;
		case keyDown:
		case autoKey:
			theChar = eventPtr->message & charCodeMask;
			if( (eventPtr->modifiers & cmdKey) != 0 )
				HandleMenuChoice( MenuKey( theChar ) );
			break;
		case updateEvt:
			DoUpdate( eventPtr );
			break;
	}
}

/*************************************************//* HandleMouseDown */
void HandleMouseDown( EventRecord *eventPtr )
{
	WindowPtr		whichWindow;
	short			thePart;
	long			menuChoice;
	
	thePart = FindWindow( eventPtr->where, &whichWindow );
	switch( thePart )
	{
		case inMenuBar:
			menuChoice = MenuSelect( eventPtr->where );
			HandleMenuChoice( menuChoice );
			break;
		case inSysWindow:
			SystemClick( eventPtr, whichWindow );
			break;
		case inDrag:
			DragWindow( whichWindow, eventPtr->where, &qd.screenBits.bounds );
			break;
		case inGoAway:
			if( TrackGoAway( whichWindow, eventPtr->where ))
				gDone = true;
			break;
		case inContent:
			DragWindow( whichWindow, eventPtr->where, &qd.screenBits.bounds );
			break;
	}
}

/*************************************************//* HandleMenuChoice */
void HandleMenuChoice( long menuChoice )
{
	short 		menu;	
	short		item;
	
	if( menuChoice != 0 )
	{
		menu = HiWord( menuChoice );
		item = LoWord( menuChoice );
		
		switch( menu )
		{
			case mApple:
				HandleAppleChoice( item );
				break;
			case mFile:
				HandleFileChoice( item );
				break;
		}
		HiliteMenu( 0 );
	}
}

/*************************************************//* HandleAppleChoice */
void HandleAppleChoice( short item )
{
	MenuHandle		appleMenu;
	Str255			accName;
	short			accNumber;
	
	switch( item )
	{
		case iAbout:
			DoAboutSelected();
			break;
		default:
			appleMenu = GetMHandle( mApple );
			GetItem( appleMenu, item, accName );
			accNumber = OpenDeskAcc( accName );
			break;
	}
}

/*************************************************//* HandleFileChoice */
void HandleFileChoice( short item )
{
	switch( item )
	{
		case iQuit:
			gDone = true;
			break;
	}
}

/*************************************************//* DoUpdate */
void DoUpdate( EventRecord *eventPtr )
{
	PicHandle	picture;
	WindowPtr	window;
	
	window = (WindowPtr)eventPtr->message;
	
	BeginUpdate( window );
	
	DoPicture( window, gPicture );
	EndUpdate( window );
}

/*************************************************//* DoPicture */
void DoPicture( WindowPtr window, PicHandle picture )
{
	Rect			windowRect;
	GrafPtr			oldPort;
//	RgnHandle		tempRgn;
	
	GetPort( &oldPort );
	SetPort( window );
	
//	tempRgn = NewRgn();
//	GetClip( tempRgn );
	EraseRect( &window->portRect );
//	DrawGrowIcon( window );
	
//	windowRect = window->portRect;
//	ClipRect( &window->portRect );
	HLock( (Handle)picture );
	DrawPicture( picture, &window->portRect );
	HUnlock( (Handle)picture );
	
//	SetClip( tempRgn );
//	DisposeRgn( tempRgn );
	SetPort( oldPort );
}

/*************************************************//* DoAboutSelected */
void DoAboutSelected( void )
{
	GrafPtr			oldPort;
	
	GetPort( &oldPort );

	// about stuff here!
	ErrorDLOG( "(Just kidding!) Ool for Elaine, by DA", false );
		
	SetPort( oldPort );
}

void AtQuit( void )
{
	Point	refPoint;
	short 	*shortPtr;
	GrafPtr			oldPort;
	
	GetPort( &oldPort );
	
	// store the window's current coordinates!
	
	// get global coords!
	SetPort( (GrafPtr)gWindow );
	refPoint.h = gWindow->portRect.left;
	refPoint.v = gWindow->portRect.top;
	LocalToGlobal( &refPoint );
	
	// store them in our string!
	
	HLock( gCoordsRectH );
	shortPtr = (short *)(*gCoordsRectH);
	
	*shortPtr = refPoint.h;
	*(shortPtr+1) = refPoint.v;
	
	ChangedResource( gCoordsRectH );
	WriteResource( gCoordsRectH );
	HUnlock( gCoordsRectH );
	
	SetPort( oldPort );
	DisposeWindow( (WindowPtr)gWindow );
}