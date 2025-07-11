/* pinup.c
 * by Darrell Anderson
 * shows PICT ID 128 in the lower right.
 */

/*#include <GestaltEqu.h>*/

#include <values.h>

#define kLoosePixels		5

#define kBaseResID			128
#define kAboutID			999
#define kMoveToFront		(WindowPtr)-1L
#define kUseDefaultProc		(void *)-1L

#define mApple				kBaseResID
#define iAbout				1

#define mFile				kBaseResID+1
#define iQuit				1

/*************************************************//* GLOBALS */

Boolean		gDone;

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

/*************************************************//* main */
void main( void )
{
	ToolBoxInit();
	WindowInit();
	MenuBarInit();
	
	EventLoop();
}

/*************************************************//* ToolBoxInit */
void ToolBoxInit( void )
{
	InitGraf( &thePort );
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
	WindowPtr		window;
	PicHandle		picture;
	Rect			windRect;
	
	picture = GetPicture( kBaseResID );
	if( picture == nil )
	{
		SysBeep( 0 );
		ExitToShell();
	}
	
	windRect = (**(picture)).picFrame;
							
	window = NewCWindow( nil, &windRect, "\pPinUp", false, rDocProc,  
							kMoveToFront, true, (long) nil );
									
	if( window == nil )
	{
		SysBeep( 0 );
		ExitToShell();
	}
	
	MoveWindow( window, 
		(screenBits.bounds.right-kLoosePixels-(windRect.right-windRect.left)),
		(screenBits.bounds.bottom-kLoosePixels-(windRect.bottom-windRect.top)),
						false );
	
	ShowWindow( window );
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
			DragWindow( whichWindow, eventPtr->where, &screenBits.bounds );
			break;
		case inGoAway:
			if( TrackGoAway( whichWindow, eventPtr->where ))
				gDone = true;
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
	
	picture = GetPicture( kBaseResID );

	if( picture == nil )
	{
		SysBeep( 0 );
		ExitToShell();
	}
	
	DoPicture( window, picture );
	EndUpdate( window );
}

/*************************************************//* DoPicture */
void DoPicture( WindowPtr window, PicHandle picture )
{
	Rect			windowRect;
	GrafPtr			oldPort;
	RgnHandle		tempRgn;
	
	GetPort( &oldPort );
	SetPort( window );
	
	tempRgn = NewRgn();
	GetClip( tempRgn );
	EraseRect( &window->portRect );
	DrawGrowIcon( window );
	
	windowRect = window->portRect;
	ClipRect( &window->portRect );
	DrawPicture( picture, &window->portRect );
	
	SetClip( tempRgn );
	DisposeRgn( tempRgn );
	SetPort( oldPort );
}

/*************************************************//* DoAboutSelected */
void DoAboutSelected( void )
{
	PicHandle		picture;
	WindowPtr		window;
	GrafPtr			oldPort;
	
	GetPort( &oldPort );
	
	window = FrontWindow();
	SetPort( window );
	
	picture = GetPicture( kAboutID );

	if( picture == nil )
		SysBeep( 0 );
	
	DoPicture( window, picture );
	
	while( ! Button() )
		;
	
	InvalRect( &window->portRect );
	
	SetPort( oldPort );
}


	