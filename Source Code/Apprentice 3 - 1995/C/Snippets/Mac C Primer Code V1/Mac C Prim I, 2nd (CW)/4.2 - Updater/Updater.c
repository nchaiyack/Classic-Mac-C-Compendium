/********************************************************/
/*														*/
/*  Updater Code from Chapter Four of 					*/
/*														*/
/*	** The Macintosh C Programming Primer, 2nd Ed. **	*/
/*  													*/
/*	Copyright 1992, Dave Mark and Cartwright Reed   	*/
/*														*/
/*  This program demonstrates specific Mac programming	*/
/*	techniques.											*/
/*														*/	
/********************************************************/

#include <limits.h>

#define kBaseResID			128
#define	kMoveToFront		(WindowPtr)-1L
#define kSleep				LONG_MAX

#define kScrollBarAdjust	(16-1)
#define	kLeaveWhereItIs		false
#define kNormalUpdates		true

#define kMinWindowHeight	50
#define	kMinWindowWidth		80


/*************/
/*  Globals  */
/*************/

Boolean		gDone;


/***************/
/*  Functions  */
/***************/

void	ToolBoxInit( void );
void	WindowInit( void );
void	EventLoop( void );
void	DoEvent( EventRecord *eventPtr );
void	HandleMouseDown( EventRecord *eventPtr );
void	DoUpdate( EventRecord *eventPtr );
void	DoActivate( WindowPtr window, Boolean becomingActive );
void	DoPicture( WindowPtr window, PicHandle picture );
void	CenterPict( PicHandle picture, Rect *destRectPtr );


/******************************** main *********/

void	main( void )
{
	ToolBoxInit();
	WindowInit();
	
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
	
	SetWRefCon ( window, (long)kBaseResID );
	ShowWindow( window );
	
	window = GetNewWindow( kBaseResID+1, nil, kMoveToFront );
	
	if ( window == nil )
	{
		SysBeep( 10 );	/*  Couldn't load the WIND resource!!!  */
		ExitToShell();
	}
	
	SetWRefCon ( window, (long)( kBaseResID+1 ) );
	ShowWindow( window );
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
	}
}


/************************************* DoEvent	 */

void	DoEvent( EventRecord *eventPtr )
{
	Boolean	becomingActive;
	
	switch ( eventPtr->what )
	{
		case mouseDown: 
			HandleMouseDown( eventPtr );
			break;
		case updateEvt:
			DoUpdate( eventPtr );
			break;
		case activateEvt:
			becomingActive = ( (eventPtr->modifiers & activeFlag) == activeFlag );
			
			DoActivate( (WindowPtr)eventPtr->message, becomingActive );
			break;
	}
}


/************************************* HandleMouseDown */

void	HandleMouseDown( EventRecord *eventPtr )
{
	WindowPtr		window;
	short			thePart;
	GrafPtr			oldPort;
	long			windSize;
	Rect			growRect;
	
	thePart = FindWindow( eventPtr->where, &window );
	
	switch ( thePart )
	{
		case inSysWindow : 
			SystemClick( eventPtr, window );
			break;
		case inContent:
			SelectWindow( window );
			break;
		case inDrag : 
			DragWindow( window, eventPtr->where, &qd.screenBits.bounds );
			break;
		case inGoAway :
			if ( TrackGoAway( window, eventPtr->where ) )
				gDone = true;
			break;
		case inGrow:
			growRect.top = kMinWindowHeight;
			growRect.left = kMinWindowWidth;
			growRect.bottom = SHRT_MAX;
			growRect.right = SHRT_MAX;
			windSize = GrowWindow( window, eventPtr->where, &growRect );
			if ( windSize != 0 )
			{
				GetPort( &oldPort );
				SetPort( window );
				EraseRect( &window->portRect );
				SizeWindow( window, LoWord( windSize ),
						HiWord( windSize ), kNormalUpdates );
				InvalRect( &window->portRect );
				SetPort( oldPort );
			}
			break;
		case inZoomIn:
		case inZoomOut:
			if ( TrackBox( window, eventPtr->where, thePart ) )
			{
				GetPort( &oldPort );
				SetPort( window );
				EraseRect( &window->portRect );
				ZoomWindow( window, thePart, kLeaveWhereItIs );
				InvalRect( &window->portRect );
				SetPort( oldPort );
			}
			break;
	}
}


/************************************* DoUpdate	 */

void	DoUpdate( EventRecord *eventPtr )
{
	short		pictureID;
	PicHandle	picture;
	WindowPtr	window;
	
	window = (WindowPtr)eventPtr->message;
	
	BeginUpdate( window );
	pictureID = GetWRefCon ( window );
	picture = GetPicture( pictureID );
	
	if ( picture == nil )
	{
		SysBeep( 10 );	/*  Couldn't load the PICT resource!!!  */
		ExitToShell();
	}
	
	DoPicture( window, picture );
	EndUpdate( window );
}


/************************************* DoActivate	 */

void	DoActivate( WindowPtr window, Boolean becomingActive )
{
	DrawGrowIcon( window );
}


/******************************** DoPicture *********/

void	DoPicture( WindowPtr window, PicHandle picture )
{
	Rect	drawingClipRect, myRect;
	GrafPtr	oldPort;
	RgnHandle	tempRgn;
	
	GetPort( &oldPort );
	SetPort( window );
	
	tempRgn = NewRgn();
	GetClip( tempRgn );
	EraseRect( &window->portRect );
	
	DrawGrowIcon( window );

	drawingClipRect = window->portRect;
	drawingClipRect.right -= kScrollBarAdjust;
	drawingClipRect.bottom -= kScrollBarAdjust;
	
	myRect = window->portRect;
	CenterPict( picture, &myRect );
	ClipRect( &drawingClipRect );
	DrawPicture( picture, &myRect );
	
	SetClip( tempRgn );
	DisposeRgn( tempRgn );
	SetPort( oldPort );
}


/****************** CenterPict ********************/

void	CenterPict( PicHandle picture, Rect *destRectPtr )
{
	Rect	windRect, pictRect;
	
	windRect = *destRectPtr;
	pictRect = (**( picture )).picFrame;
	OffsetRect( &pictRect, windRect.left - pictRect.left,
						   windRect.top	 - pictRect.top);
	OffsetRect( &pictRect,(windRect.right - pictRect.right)/2,
						  (windRect.bottom - pictRect.bottom)/2);
	*destRectPtr = pictRect;
}