/*
GetFileIconExample.c
3/10/95
ver 1.1
-------


GetFileIconExample.c is based on one of the 'Getting Started' articles
in MacTech magazine.  It is used solely to provide a backbone for the
GetFileIcon routine.

The purpose of this program is to demonstrate how to get a files' icon
and display it.

For a more thorough description, please see the file, GetFileIcon.c
*/


#include <stdio.h>
#include <GestaltEqu.h>
#include "GetFileIcon.h"





#define kBaseResID			128
#define kErrorALRTid		128
#define kAboutALRTid		129

#define kWINDResID			128

#define kVisible			true
#define	kMoveToFront		(WindowPtr)-1L
#define kSleep				60L
#define kNilFilterProc		0L

#define kWindowStartX		20
#define kWindowStartY		50

#define mApple				kBaseResID
#define iAbout				1

#define mFile				kBaseResID+1
#define iGetFile			1
#define iQuit				3






/*************/
/*  Globals  */
/*************/

Boolean				gDone;
Handle				gTheSuite;
StandardFileReply	gTheReply;


/***************/
/*  Functions  */
/***************/

void		ToolboxInit( void );
void		MenuBarInit( void );
WindowPtr	CreateWindow( Str255 name );
void		DoCloseWindow( WindowPtr window );
void		EventLoop( void );
void		DoEvent( EventRecord *eventPtr );
void		HandleMouseDown( EventRecord *eventPtr );
void		HandleMenuChoice( long menuChoice );
void		HandleAppleChoice( short item );
void		HandleFileChoice( short item );
void		DoUpdate( EventRecord *eventPtr );
void		DoError( Str255 errorString );


/******************************** main *********/

void	main( void )
{
	ToolboxInit();
	MenuBarInit();

	CreateWindow( "\p<Untitled>" );

	EventLoop();
}


/*********************************** ToolboxInit */

void	ToolboxInit( void )
{
#ifdef THINK_C
	InitGraf( &thePort );
#else
	InitGraf( &qd.thePort );
#endif
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( 0L );
	InitCursor();
}


/****************** MenuBarInit ***********************/

void	MenuBarInit( void )
{
	Handle			menuBar;
	MenuHandle		menu;
	
	menuBar = GetNewMBar( kBaseResID );
	
	if(menuBar == NULL)
		DoError( "\pCouldn't load the MBAR resource..." );

	SetMenuBar( menuBar );

	menu = GetMHandle( mApple );
	AddResMenu( menu, 'DRVR' );

	DrawMenuBar();
}



/****************** CreateWindow ***********************/
WindowPtr CreateWindow( Str255 name )
{
	WindowPtr	window;
	short		windowWidth, windowHeight;
	
	window = GetNewCWindow( kWINDResID, nil, kMoveToFront );
	SetWTitle( window, name );

	
	ShowWindow( window );
	SetPort( window );
	
	return window;
}


/****************** DoCloseWindow ***********************/

void	DoCloseWindow( WindowPtr window )
{
	if(window != nil)
		DisposeWindow( window );
}


/******************************** EventLoop *********/

void	EventLoop( void )
{		
	EventRecord		event;
	
	gDone = false;
	while ( gDone == false )
	{
		if( WaitNextEvent(everyEvent, &event, kSleep, nil) )
			DoEvent( &event );
	}
}


/************************************* DoEvent	 */

void	DoEvent( EventRecord *eventPtr )
{
	char		theChar;
	
	switch ( eventPtr->what )
	{
		case mouseDown: 
			HandleMouseDown( eventPtr );
			break;

		case keyDown:
		case autoKey:
			theChar = eventPtr->message & charCodeMask;
			if ( (eventPtr->modifiers & cmdKey) != 0 ) 
				HandleMenuChoice( MenuKey( theChar ) );
			break;

		case updateEvt:
			DoUpdate( eventPtr );
			break;
	}
}


/************************************* HandleMouseDown */

void	HandleMouseDown( EventRecord *eventPtr )
{
	WindowPtr		window;
	short			thePart;
	long			menuChoice;
	
	thePart = FindWindow( eventPtr->where, &window );
	
	switch ( thePart )
	{
		case inMenuBar:
			menuChoice = MenuSelect( eventPtr->where );
			HandleMenuChoice( menuChoice );
			break;
		case inSysWindow : 
			SystemClick( eventPtr, window );
			break;
		case inGoAway:
			if ( TrackGoAway( window, eventPtr->where ) )
				gDone = TRUE;
			break;
		case inContent:
			SelectWindow( window );
			break;
		case inDrag : 
			DragWindow( window, eventPtr->where, &qd.screenBits.bounds );
			break;
	}
}


/****************** HandleMenuChoice ***********************/

void	HandleMenuChoice( long menuChoice )
{
	short	menu;
	short	item;
	
	if ( menuChoice != 0 )
	{
		menu = HiWord( menuChoice );
		item = LoWord( menuChoice );
		
		switch ( menu )
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


/****************** HandleAppleChoice ***********************/

void	HandleAppleChoice( short item )
{
	MenuHandle	appleMenu;
	Str255		accName;
	short		accNumber;
	
	switch ( item )
	{
		case iAbout:
			Alert(kAboutALRTid, nil);
			break;

		default:
			appleMenu = GetMHandle( mApple );
			GetItem( appleMenu, item, accName );
			accNumber = OpenDeskAcc( accName );
			break;
	}
}


/****************** HandleFileChoice ***********************/
void	HandleFileChoice( short item )
{
	WindowPtr			window;


	switch ( item )
	{
		case iGetFile:
			StandardGetFile(0L, -1, 0L, &gTheReply);
			if( gTheReply.sfGood )
			{
				GetFileIcon(&gTheReply.sfFile, svAllAvailableData, &gTheSuite);
				/* Note: if you are just using large icons (32x32), just use
					svLarge1Bit + svLarge4Bit + svLarge8Bit instead of svAllAvailableData */
				window = FrontWindow();
				EraseRgn( window->visRgn );
				InvalRgn( window->visRgn );
			}
			break;

		case iQuit:
			gDone = true;
			DoCloseWindow( FrontWindow() );
			break;
	}
}



/************************************* DoUpdate	 */
void	DoUpdate( EventRecord *eventPtr )
{
	WindowPtr	window;
	Rect		r;
	char		temp[40];



	window = (WindowPtr )eventPtr->message;
	BeginUpdate( window );

	if(gTheSuite == nil)
	{
		MoveTo(20, 50);
		DrawString("\pPlease press CMD-O to display a file's icon");
	}
	else
	{
		SetRect(&r, 24, 30, 24 + 32, 30 + 32);
		PlotIconSuite(&r, 0, 0, gTheSuite);
		MoveTo(70, 50);
		DrawString( gTheReply.sfFile.name );

		sprintf(temp, "vRefNum = %d, parID = %ld",
			gTheReply.sfFile.vRefNum, gTheReply.sfFile.parID);
		MoveTo(70, 64);
		DrawString( CtoPstr(temp) );
	}
	EndUpdate(window);
}


/***************** DoError ********************/

void	DoError( Str255 errorString )
{
	ParamText(errorString, "\p", "\p", "\p");
	StopAlert(kErrorALRTid, kNilFilterProc);
	ExitToShell();
}