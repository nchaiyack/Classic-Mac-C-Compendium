//ColorWindow.c
//Copyright © 1995 Appalachian Trail Solutions.  All rights reserved.

/*	This file has its roots in and some routines from Color Window Demo by Jim Griffin 
	circa 1990.  This is an excellent program but the version that is posted on AOL is 
	dated.  I spent about two hours getting it to compile under Think C 7.0.  I 
	intended to port it to CodeWarrior but ran into major problems and decided to do a 
	complete rewrite.
	
	I borrowed some functions from ColorTutor and some other Mac Primer programs by 
	Dave Mark.  You may want to check out his excellent books.  They are available 
	from MacTech or in the computer section of your favorite bookstore.  Look for 
	Macintosh Programming Primer and others.
*/

/************************************************************************************
 *
 *	Program name:	ColorWindow
 *	Version:		1.5
 *	Date:			6/26/95
 *	Authors:		Ken Elrod		ElrodK@aol.com
 *					Portions by Dave Mark and Jim Griffin
 *  Address:		147 Kare-Fre Blvd.
 *	City:			West Union
 *	State:			SC
 *  Zip Code:		29696
 *	
 *	No royalties are required to use this program subject to the following 
 *	conditions:
 *	1. You may not sell this program or include it on any media which is sold for
 *	   for profit without permission in writing.
 *	2. It is intended as an example of how to use System 7 window color tables.  You
 *     may use the routines in your programs but must give credit to the authors in 
 *     your final work.  After all, they did all the hard work for you.
 *  3. If you like this please send me a postcard or picture from your town.  I am 
 *     curious to see how far this gets.
 *
 ************************************************************************************
 * 
 * 	Send all bug reports, suggestions, comments, questions, etc via E-mail to me
 *	at elrodk@aol.com.
 *
 ************************************************************************************
 *
 *	Revision History:
 *
 *	Version 1.0:
 *	Initial release - May 29, 1995
 *  Developed on PowerMacintosh 6100/60AV using MetroWerks CodeWarrior 5.  I have not
 *  tested this on other machines as I have access only to my Mac 
 *
 *  Version 1.5:
 *  Released - June 26, 1995
 *  1. Added scroll bars and activated the menu to update the control colors.  I did
 *     this in a nonstandard way because I just wanted the scroll bars so we could change 
 *     the color.  This is not how scroll bars are supposed to be used.  Scroll bars are
 *     supposed to move you around in a document.  The routine I used here just pages
 *     between all the open PICT resources on your machine.  I again borrowed this from 
 *     Show Pict in the Mac Primer. 
 *  2. Also included are some minor changes to make the program work with multiple
 *     windows.  This is mainly a bunch of calls to SetPort and handling activate evts.
 *  3. This version was developed using MetroWerks CodeWarrior 6.
 *
 ************************************************************************************
 *
 *  Known Bugs & Incompatabilities:
 *
 *  Windows and controls are drawn in black & white with some settings.  The less 
 *  colors you have your monitor set to the more often this occurs.  I believe it
 *  has something to do with the way the window manager mixes colors to color some
 *  of the window parts.  As soon as I can track down the problem I will issue an
 *  update.  If anyone finds the answer please let me know. 
 *
 ************************************************************************************
 *
 *  Next version wish list:
 *  1. Save user color choices and restore as the default.
 *  2. Allow color changes in our about box so we can see the results of the 
 *     dialog colors.
 *  3. E-mail yours to me at elrodk@aol.com
 *
 ***********************************************************************************/
 
//miscellaneous constants
#define	kBaseResID			128
#define kErrorALRTid		kBaseResID+1
#define kNullFilterProc		NULL
#define kMoveToFront		(WindowPtr)-1L
#define kSleep				60L
#define	kActivate			TRUE
#define kDeactivate			FALSE

//Apple menu
#define mApple				kBaseResID
#define iAbout				1

//File menu
#define mFile				kBaseResID+1
#define iNew				1
#define iClose				3
#define iQuit				5

//Edit menu
#define mEdit				kBaseResID+2
#define iUndo				1
#define iCut				3
#define iCopy				4
#define iPaste				5
#define iClear				6

//Window menu
#define mWindow				kBaseResID+3
#define iContent			1
#define iFrame				2
#define iText				3
#define iHilite				4
#define iTitle				5
#define iHiliteLight		6
#define iHiliteDark			7
#define iTitleLight			8
#define iTitleDark			9
#define iDialogLight		10
#define iDialogDark			11
#define iTingeLight			12
#define iTingeDark			13

//Control menu
#define mControl			kBaseResID+4
#define iCFrame				1	//Frame
#define iCBody				2	//Body
#define iCText				3	//Text for buttons & checkboxes
#define iCThumb				4	//Reserved
#define iCFillPat			5	//Reserved
#define iCArrowsLight		6	//Arrows & scroll bar bkgrnd
#define iCArrowsDark		7	//Arrows & scroll bar bkgrnd
#define iCThumbLight		8	//Thumb
#define iCThumbDark			9	//Thumb
#define iCHiliteLight		10	//use value of wHiliteLight per InsideMac
#define iCHiliteDark		11	//Use value of wHiliteDark per InsideMac
#define iCTitleBarLight		12	//Use value of wTitleBarLight per InsideMac
#define iCTitleBarDark		13	//Use value of wTitleBarDark per InsideMac
#define iCTingeLight		14	//Use value of wTingeLight per InsideMac
#define iCTingeDark			15	//Use value of wTingeDark per InsideMac

//About Box
#define kAboutDLOGid		kBaseResID

//System 7 colors for 'wctb'
//*** does anybody have any idea why these aren't defined in Apple's headers???
//see Inside Mac, Macintosh Toolbox Essentials p.4-72 for the official definitions.
#define wHiliteColorLight	5
#define wHiliteColorDark	6
#define wTitleBarLight		7
#define wTitleBarDark		8
#define wDialogLight		9
#define wDialogDark			10
#define wTingeLight			11
#define wTingeDark			12

//System 7 colors for 'cctb'
//*** does anybody have any idea why these aren't defined in Apple's headers???
//see Inside Mac, Macintosh Toolbox Essentials p.5-122 for the official definitions.
#define cFillPatColor		4	//Reserved
#define cArrowsColorLight	5	//Arrows & scroll bar bkgrnd
#define cArrowsColorDark	6	//Arrows & scroll bar bkgrnd
#define cThumbLight			7	//Thumb
#define cThumbDark			8	//Thumb
#define cHiliteLight		9	//use value of wHiliteLight
#define cHiliteDark			10	//Use value of wHiliteDark
#define cTitleBarLight		11	//Use value of wTitleBarLight
#define cTitleBarDark		12	//Use value of wTitleBarDark
#define cTingeLight			13	//Use value of wTingeLight
#define cTingeDark			14	//Use value of wTingeDark

//Scroll Bars
#define kScrollBarWidth		16
#define kStartValue			1
#define kMinValue			1
#define kNilActionProc		0
#define kVisible			true
#define kNilRefCon			0L
#define kEmptyTitle			"\p"
#define kEmptyString		"\p"


//Globals  (yes Mac programs can be done without a boat load of globals, the less the better)
Boolean				gDone;
ControlActionUPP	gActionUPP;


//  Function Prototypes 
void		ToolboxInit( void );
void		MenuBarInit( void );
void		CreateWindow( void );
void		SetUpScrollBar( WindowPtr window );
pascal void ScrollProc( ControlHandle theControl, short partCode );
void		SetUpGlobals( void );
void		EventLoop( void );
void		DoEvent( EventRecord *eventPtr );
void		HandleMouseDown( EventRecord *eventPtr );
void		HandleMenuChoice( long menuChoice );
void		HandleAppleChoice( short item );
void		HandleFileChoice( short item );
void		HandleWindowChoice( short item );
void		HandleControlChoice( short item );
void 		AdjustMenus(void);
void		AdjustControls( Boolean activate, WindowPtr window );
void		DoUpdate( WindowPtr window );
void		DrawContents( WindowPtr window );
void		DoContent( WindowPtr window, Point globalPoint );
Boolean		HasColorQD( void );
Boolean		HasSystem7( void );
void		CenterPict( PicHandle picture, Rect *destRectPtr );
void		DoError( Str255 errorString );
Boolean		CloseAWindow( WindowPtr window );
void		InitMyWinColor( WindowPtr window );
void		InitMyControlColor( WindowPtr window );


/*******************************  main  ********************************/

void	main( void )
{
	ToolboxInit();
	MenuBarInit();
	
	//Make sure we have access to color QuickDraw…
	if( ! HasColorQD() )
		DoError( "\pThis machine does not support Color QuickDraw!" );
		
	//…and check for system 7.0 or later here.  If not you WILL CRASH the Mac! 
	if( ! HasSystem7() )
		DoError( "\pThis machine does not have System 7 or higher!" );
	
	//My function to put up a window
	CreateWindow();
	//Initialize any global variables  or structures with this function
	SetUpGlobals();
	//Then drop into the main event loop
	EventLoop();
}


/****************** ToolboxInit *********************/

void	ToolboxInit( void ) 
{
	InitGraf( &qd.thePort );
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
	
	//Read our menu from the resource file
	menuBar = GetNewMBar( kBaseResID );
	
	//Check to see if we got it.  If not then call our error routine.
	if ( menuBar == NULL )
		DoError( "\pCouldn't load the MBAR resource..." );
	
	//Set the menu to our menuBar…
	SetMenuBar( menuBar );
	
	//Get a handle to the Apple menu and add the DAs and/or Apple Menu Items folder
	menu = GetMHandle( mApple );
	AddResMenu( menu, 'DRVR' );
	
	//Finally draw our menuBar…
	DrawMenuBar();
}


/****************************  CreateWindow  ****************************/

void	CreateWindow( void )
{
	WindowPtr	window;
	
	//Standard way of reading in a window from a resourse
	window = GetNewCWindow( kBaseResID, NULL, kMoveToFront );
	
	//Check to see if we got it.  If not then call our error routine.
	if( window == NULL )
		DoError( "\pCouldn't load the WIND resource..." );
	
	//Create scroll bars for our new window.
	SetUpScrollBar( window );
	
	//Call our routine to set the initial colors to the defaults…
	InitMyWinColor( window );
	//…scroll bars too.
	InitMyControlColor( window );
	
	//Show our window and set the current port to it.
	ShowWindow( window );
	SetPort( window );
}


/**********************************	SetUpScrollBar	*******/
//Borrowed from the Macintosh Programming Primer by Dave Mark & C. Reed.
void	SetUpScrollBar( WindowPtr window )
{
	Rect			vScrollRect;
	short			numPictures;
	ControlHandle	scrollBarH;
	
	if ( ( numPictures = CountResources( 'PICT' ) ) <= 0 )
		DoError( "\pNo PICT resources were found!" );
		
	vScrollRect = window->portRect;
	vScrollRect.top -= 1;
	vScrollRect.bottom += 1;
	vScrollRect.left = vScrollRect.right - kScrollBarWidth + 1;
	vScrollRect.right += 1;
	
	scrollBarH = NewControl( window, &vScrollRect,
			kEmptyTitle, kVisible, kStartValue, kMinValue,
			numPictures, scrollBarProc, kNilRefCon );
}


/**********************************	ScrollProc	*******/
//Borrowed from the Macintosh Programming Primer by Dave Mark & C. Reed.
pascal void ScrollProc( ControlHandle theControl, short partCode )
{
	short		curCtlValue, maxCtlValue, minCtlValue;
	WindowPtr	window;
	
	maxCtlValue = GetControlMaximum( theControl );
	curCtlValue = GetControlValue( theControl );
	minCtlValue = GetControlMinimum( theControl );
	
	window = (**theControl).contrlOwner;
	
	switch ( partCode )
	{
		case inPageDown:
		case inDownButton:
			if ( curCtlValue < maxCtlValue )
			{
				curCtlValue += 1;
				SetCtlValue( theControl, curCtlValue );
				DrawContents( window );
			}
			break;
		case inPageUp:
		case inUpButton:
			if ( curCtlValue > minCtlValue )
			{
				curCtlValue -= 1;
				SetCtlValue( theControl, curCtlValue );
				DrawContents( window );
			}
	}
}


/****************************  SetUpGlobals  ***************************/

void	SetUpGlobals( void )
{
	//If you have any.
}


/******************************** EventLoop *********/
//Standard system 7 way of doing things.  Since we only run under system 7 or later
//we don't call GetNextEvent().
void	EventLoop( void )
{		
	EventRecord		event;
	
	gDone = false;
	while ( gDone == false )
	{
		if ( WaitNextEvent( everyEvent, &event, kSleep, NULL ) )
			DoEvent( &event );
	}
}


/************************************* DoEvent	 */

void	DoEvent( EventRecord *eventPtr )
{
	char		theChar;
	
	//Check for event types and handle or pass off to appropriate function.
	switch( eventPtr->what )
	{
		case mouseDown: 
			HandleMouseDown( eventPtr );
			break;
			
		case keyDown:
		case autoKey:
			theChar = eventPtr->message & charCodeMask;
			//If user is typing check for command key and handle menu item.
			if ( (eventPtr->modifiers & cmdKey) != 0 ) 
			{	
				//Make sure the menu's are updated…
				AdjustMenus();
				//…then call our menu function.
				HandleMenuChoice( MenuKey( theChar ) );
			}
			break;
			
		case updateEvt:
			//We get updates whenever our window needs redrawing etc.
			DoUpdate( (WindowPtr)eventPtr->message );
			break;
			
		case activateEvt:
			//You receive two activate events when a user switches windows.  One to 
			//deactivate the current window and one to activate the clicked on window.
			if ( (eventPtr->modifiers & activeFlag) != 0 )
			{
				//If activate
				AdjustControls( kActivate, (WindowPtr) eventPtr->message );
				AdjustMenus();
			}
			else
			{
				//If deactivate
				AdjustControls( kDeactivate, (WindowPtr) eventPtr->message );
				AdjustMenus();
			}
			break;
			
		case osEvt:
			if ( eventPtr->message & 0x01000000 ) 
			//All we are intrested in is suspend or resume.
			{	
				if ( eventPtr->message & 0x00000001 )
				//If resume evt (activate our application)…
				{
					InitCursor(); //…first make sure we have the arrow cursor…
					AdjustMenus();//…then make sure our menus are properly set…
					AdjustControls( kActivate, FrontWindow() );//…and finally take care of the controls.
				}
				else
				// suspend = deactivate; call our suspend routine, if any.
				{
					//I am calling HiliteMenu() because switching can leave a menu hilited
					//when you return and it looks real ugly.
					HiliteMenu( 0 );
					//Hide scroll bars and dim controls.
					AdjustControls( kDeactivate, FrontWindow() );
				} 	
			}
			break;
	}
}


/************************************* HandleMouseDown */

void	HandleMouseDown( EventRecord *eventPtr )
{
	WindowPtr		window;
	short			thePart;
	long			menuChoice;
	GrafPtr			oldPort;
	long			windSize;
	Point			mouse;
	
	//First find the window that was clicked on if any…
	thePart = FindWindow( eventPtr->where, &window );
	
	switch ( thePart )
	{
		case inMenuBar:
		//If in the menuBar 
			AdjustMenus();
			menuChoice = MenuSelect( eventPtr->where );
			HandleMenuChoice( menuChoice );
			break;
		case inSysWindow : 
		//If in a system window (desk accessory, remember those?)
			SystemClick( eventPtr, window );
			break;
		case inContent:
		//OK, Now we have a click in our app's window…
			if ( window != FrontWindow() )
			//…if it's not in front then make it so…
				SelectWindow( window );
			else
			//…if it was in front handle the click as necessary.
			{
				SetPort( window );
				DoContent( window, eventPtr->where );
			}
			break;
		case inDrag : 
		//If the user is dragging us around then follow the yellow brick road (mouse).
			DragWindow( window, eventPtr->where, &qd.screenBits.bounds );
			break;
		case inGoAway:
		//If the click is in the close box then close the window.
			if ( TrackGoAway(window, eventPtr->where) )
				CloseAWindow( window );
			break;
		case inGrow:
		//If in the grow box handle it here, we don't have one in ColorWindow.
			break;
		case inZoomIn:
		case inZoomOut:
		//If in the zoom box, just do it.  We aren't using it either but I left it in.
			if ( TrackBox(window, eventPtr->where, thePart) )
			//Commented out the call to DoZoomWindow() since it is not in our code.
			//	DoZoomWindow(window, thePart);
			break;
	}
}


/****************** HandleMenuChoice ***********************/

void	HandleMenuChoice( long menuChoice )
{
	short	menu;
	short	item;
	
	//First make sure the user selected something…
	if ( menuChoice != 0 )
	{
		menu = HiWord( menuChoice );
		item = LoWord( menuChoice );
		
		switch ( menu )
		//…then handle the choice as appropriate.
		{
			case mApple:
				HandleAppleChoice( item );
				break;
			case mFile:
				HandleFileChoice( item );
				break;
			case mWindow:
				HandleWindowChoice( item );
				break;
			case mControl:
				HandleControlChoice( item );
				break;
		}
		//The system hilites the menu title and we have to remove the hilite.
		//You should leave it hilited until you are finished in case of a lengthy operation. 
		HiliteMenu( 0 );
	}
}


/****************** HandleAppleChoice ***********************/

void	HandleAppleChoice( short item )
{
	MenuHandle	appleMenu;
	Str255		accName;
	short		accNumber;
	WindowPtr	window;
	
	switch ( item )
	//See what they chose…
	{
		case iAbout:
		//…if our about box…
			window = FrontWindow();
	
			//…make sure we have a window open and if so…
			if( window != nil )
				AdjustControls( kDeactivate, window );	//…deactivate controls…
			Alert( kBaseResID, nil );					//…then show it to them,…
			if( window != nil )
				AdjustControls( kActivate, window );	//…and activate controls.
			break;
		default:
		//…if not our about box then open the app, DA, or folder.
			appleMenu = GetMHandle( mApple );
			GetItem( appleMenu, item, accName );
			accNumber = OpenDeskAcc( accName );
			break;
	}
}


/****************** HandleFileChoice ***********************/

void	HandleFileChoice( short item )
{
	switch ( item )
	//See what they chose…
	{
		case iNew:
		//…if new, make them a new window.
			CreateWindow();
			break;
		case iClose:
		//…if close, then get rid on one.
			CloseAWindow( FrontWindow() );
			break;
		case iQuit:
		//…if quit, we are all done. Set our global flag and scram!
			gDone = true;
			break;
	}
}


/****************** HandleWindowChoice ***********************/

void	HandleWindowChoice( short item )
{
	RGBColor		theColor;
	AuxWinHandle	myAuxWinH;
	Boolean			auxFlag;
	WindowPtr 		window;
	GrafPtr			savePort;
	Point			where = {0, 0};
	
	//Save our grafPort before starting
	GetPort( &savePort );
	
	//Get a pointer to the window, our menu works on the active (front) window
	window = FrontWindow();
	
	//Get a handle to our color table
	auxFlag = GetAuxWin( window, &myAuxWinH );
	
	if( !auxFlag || (window == nil) )
	/* Test to see if we got a window pointer and a color table. GetAuxWin() 
	returns true if the handle is found or a handle to the system colors if 
	the window is nil.  We don't want the system colors so I test for nil. */
	{
		DoError( "\pWindow color record not available..." );
	}
	
	// Lock the handle before calling any functions that may move memory...
	HLock( (Handle)(**myAuxWinH).awCTable);
	
	switch ( item )
	//See what they chose…
	{
		case iContent:
		//…if content, use the color picker (GetColor) to get the color…
			 if( GetColor( where, "\pContent Color", 
				&(**(**myAuxWinH).awCTable).ctTable[wContentColor].rgb, &theColor))
				//…and if the user didn't cancel, set it.
				(**(**myAuxWinH).awCTable).ctTable[wContentColor].rgb = theColor;
			break;
			
		case iFrame:
		//…if frame, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pFrame Color", 
				&(**(**myAuxWinH).awCTable).ctTable[wFrameColor].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxWinH).awCTable).ctTable[wFrameColor].rgb = theColor;
			break;
			
		case iText:
		//…if text, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pText Color", 
				&(**(**myAuxWinH).awCTable).ctTable[wTextColor].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxWinH).awCTable).ctTable[wTextColor].rgb = theColor;
			break;
			
		case iHilite:
		//…if hilite, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pHilite Color", 
				&(**(**myAuxWinH).awCTable).ctTable[wHiliteColor].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxWinH).awCTable).ctTable[wHiliteColor].rgb = theColor;
			break;
			
		case iTitle:
		//…if titleBar, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pTitle Bar Color", 
				&(**(**myAuxWinH).awCTable).ctTable[wTitleBarColor].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxWinH).awCTable).ctTable[wTitleBarColor].rgb = theColor;
			break;
			
		case iHiliteLight:
		//…if hiliteLight, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pHilite Light Color", 
				&(**(**myAuxWinH).awCTable).ctTable[wHiliteColorLight].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxWinH).awCTable).ctTable[wHiliteColorLight].rgb = theColor;
			break;
			
		case iHiliteDark:
		//…if hiliteDark, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pHilite Dark Color", 
				&(**(**myAuxWinH).awCTable).ctTable[wHiliteColorDark].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxWinH).awCTable).ctTable[wHiliteColorDark].rgb = theColor;
			break;
			
		case iTitleLight:
		//…if titleLight, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pTitle Bar Light Color", 
				&(**(**myAuxWinH).awCTable).ctTable[wTitleBarLight].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxWinH).awCTable).ctTable[wTitleBarLight].rgb = theColor;
			break;
			
		case iTitleDark:
		//…if titleDark, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pTitle Bar Dark Color", 
				&(**(**myAuxWinH).awCTable).ctTable[wTitleBarDark].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxWinH).awCTable).ctTable[wTitleBarDark].rgb = theColor;
			break;
			
		case iDialogLight:
		//…if dialogLight, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pDialog Light Color", 
				&(**(**myAuxWinH).awCTable).ctTable[wDialogLight].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxWinH).awCTable).ctTable[wDialogLight].rgb = theColor;
			break;
			
		case iDialogDark:
		//…if dialogDark, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pDialog Dark Color", 
				&(**(**myAuxWinH).awCTable).ctTable[wDialogDark].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxWinH).awCTable).ctTable[wDialogDark].rgb = theColor;
			break;
			
		case iTingeLight:
		//…if tingeLight, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pTinge Light Color", 
				&(**(**myAuxWinH).awCTable).ctTable[wTingeLight].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxWinH).awCTable).ctTable[wTingeLight].rgb = theColor;
			break;
			
		case iTingeDark:
		//…if tingeDark, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pTinge Dark Color", 
				&(**(**myAuxWinH).awCTable).ctTable[wTingeDark].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxWinH).awCTable).ctTable[wTingeDark].rgb = theColor;
			break;
	}
	
	//Now call SetWinColor() to set the window to our color table.
	SetWinColor( window, (WCTabHandle)(**myAuxWinH).awCTable);
	
	// Unlock the handle now that we are done…
	HUnlock( (Handle)(**myAuxWinH).awCTable);
	//…and restore the port.
	SetPort( savePort );
}


/****************** HandleControlChoice ***********************/

void	HandleControlChoice( short item )
{
	RGBColor		theColor;
	ControlHandle	myCtlHandle;
	AuxCtlHandle	myAuxCtlH;
	Boolean			auxFlag;
	WindowPtr 		window;
	WindowPeek		myWinPeek;
	GrafPtr			savePort;
	Point			where = {0, 0};
	
	//Save our grafPort before starting
	GetPort( &savePort );
	
	//Get a pointer to the window, our menu works on the active (front) window
	window = FrontWindow();
	myWinPeek = (WindowPeek) window;
	myCtlHandle = myWinPeek->controlList;
	
	//Get a handle to our color table
	auxFlag = GetAuxCtl( myCtlHandle, &myAuxCtlH );
	//auxFlag = GetAuxWin( window, &myAuxWinH );
	
	if( !auxFlag || (myCtlHandle == nil) )
	/* Test to see if we got a window pointer and a color table. GetAuxWin() 
	returns true if the handle is found or a handle to the system colors if 
	the window is nil.  We don't want the system colors so I test for nil. */
	{
		DoError( "\pControl color record not available..." );
	}
	
	// Lock the handle before calling any functions that may move memory...
	HLock( (Handle) (**myAuxCtlH).acCTable );
	
	switch ( item )
	//See what they chose…
	{
		case iCFrame:
		//…if frame, use the color picker (GetColor) to get the color…
			 if( GetColor( where, "\pControl Frame Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cFrameColor].rgb, &theColor))
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cFrameColor].rgb = theColor;
			break;
			
		case iCBody:
		//…if body, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\p Control Body Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cBodyColor].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cBodyColor].rgb = theColor;
			break;
			
		case iCText:
		//…if text, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pControl Text Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cTextColor].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cTextColor].rgb = theColor;
			break;
			
		case iCThumb:
		//…if thumb, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pControl Thumb Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cThumbColor].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cThumbColor].rgb = theColor;
			break;
			
		case iCFillPat:
		//…if fill pattern, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pControl Fill Pat Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cFillPatColor].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cFillPatColor].rgb = theColor;
			break;
			
		case iCArrowsLight:
		//…if arrowsLight, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pControl Arrows Light Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cArrowsColorLight].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cArrowsColorLight].rgb = theColor;
			break;
			
		case iCArrowsDark:
		//…if arrowsDark, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pArrows Dark Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cArrowsColorDark].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cArrowsColorDark].rgb = theColor;
			break;
			
		case iCThumbLight:
		//…if thumbLight, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pThumb Light Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cThumbLight].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cThumbLight].rgb = theColor;
			break;
			
		case iCThumbDark:
		//…if thumbDark, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pThumb Dark Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cThumbDark].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cThumbDark].rgb = theColor;
			break;
			
		case iCHiliteLight:
		//…if hiliteLight, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pHilite Light Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cHiliteLight].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cHiliteLight].rgb = theColor;
			break;
			
		case iCHiliteDark:
		//…if hiliteDark, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pHilite Dark Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cHiliteDark].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cHiliteDark].rgb = theColor;
			break;
			
		case iCTitleBarLight:
		//…if controlTitleBarLight, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pControl TitleBar Light Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cTitleBarLight].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cTitleBarLight].rgb = theColor;
			break;
			
		case iCTitleBarDark:
		//…if controlTitleBarDark, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pControl TitleBar Dark Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cTitleBarDark].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cTitleBarDark].rgb = theColor;
			break;
		case iCTingeLight:
		//…if tingeLight, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pTinge Light Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cTingeLight].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cTingeLight].rgb = theColor;
			break;
			
		case iCTingeDark:
		//…if tingeDark, use the color picker (GetColor) to get the color…
			if( GetColor( where, "\pTinge Dark Color", 
				&(**(**myAuxCtlH).acCTable).ctTable[cTingeDark].rgb, &theColor)) 
				//…and if the user didn't cancel, set it.
				(**(**myAuxCtlH).acCTable).ctTable[cTingeDark].rgb = theColor;
			break;
	}
	
	//Now call SetControlColor() to set the control to our color table.
	SetControlColor( myCtlHandle, (**myAuxCtlH).acCTable );
	
	// Unlock the handle now that we are done…
	HUnlock( (Handle) (**myAuxCtlH).acCTable );
	//…and restore the port.
	SetPort( savePort );
}

/****************** AdjustMenus **********************/

void AdjustMenus(void)
{
	WindowPtr	window;
	MenuHandle	menu;
	short		windowType;
	WindowPeek	myWinPeek;
	
	//Get a pointer to the front window.
	window = FrontWindow();
	
	//Make sure we got one.
	if( window != nil )
	{
		//If so, use windowPeek to find out what kind of window and adjust menus.
		myWinPeek = (WindowPeek) window;
		windowType = myWinPeek->windowKind;
		
		if( windowType < 0 )
		//If it is a DA (DAs are < 0), enable the edit menu…
		{
			menu = GetMenuHandle( mEdit );
			EnableItem( menu, 0 );
			//…and disable the window & control menus so user can't screw up 
			//the system colors.
			menu = GetMenuHandle( mWindow );
			DisableItem( menu, 0 );
			menu = GetMenuHandle( mControl );
			DisableItem( menu, 0 );
		}
		
		else
		{	if( windowType == userKind )
			{
				//UserKind is one of our document windows.
				//Since ColorWindow doesn't use the Edit menu disable it.
				menu = GetMenuHandle( mEdit );
				DisableItem( menu, 0 );
				
				//Adjust the file menu too…
				menu = GetMenuHandle( mFile );
				EnableItem( menu, iClose);
				
				//…and enable the window & control menus so user can change 
				//the colors.
				menu = GetMenuHandle( mWindow );
				EnableItem( menu, 0 );
				menu = GetMenuHandle( mControl );
				EnableItem( menu, 0 );
			}
			else 
			{
				//must be a dialog window so disable the window & control menus so
				//user can't screw up the system colors. We're not doing dialogs yet.
				menu = GetMenuHandle( mWindow );
				DisableItem( menu, 0 );
				menu = GetMenuHandle( mControl );
				DisableItem( menu, 0 );
			}
		}
	}
	else 
	{
		//FrontWindow returned nil, no open windows for our app.
		menu = GetMenuHandle( mFile );
		DisableItem( menu, iClose);

		//Disable the window & control menus so user can't screw up 
		//the system colors.
		menu = GetMenuHandle( mWindow );
		DisableItem( menu, 0 );
		menu = GetMenuHandle( mControl );
		DisableItem( menu, 0 );
	}
	//Call DrawMenuBar to redraw the menuBar with our changes.
	DrawMenuBar();
}

/***************************  AdjustControls()  **************************/
void AdjustControls( Boolean activate, WindowPtr window)
{
	WindowPeek		myWinPeek;
	short			windowType;
	ControlHandle	myCtlHandle;
	
	//Make sure there is an open window.  If not just blow outta here.
	if( window != nil )
	{
		myWinPeek = (WindowPeek) window;
		windowType = myWinPeek->windowKind;
		myCtlHandle = myWinPeek->controlList;
		
		
		if( windowType == userKind )
		{
			//UserKind is one of our document windows.
			if( activate )
			{
				//If activate then show the controls.
				while( myCtlHandle )
				{
					ShowControl( myCtlHandle );
					myCtlHandle = (*myCtlHandle)->nextControl;
				}
			}
			else //Gotta be deactivate.
			{ 
				//If deactivate then hide the controls.
				while( myCtlHandle )
				{
					HideControl( myCtlHandle );
					myCtlHandle = (*myCtlHandle)->nextControl;
				}
			}
		}
	}
}

/****************** DoUpdate ***********************/

void	DoUpdate( WindowPtr window )
{
	//Always call BeginUpdate and EndUpdate.  See Inside Macintosh for why.
	BeginUpdate( window );
	
	DrawControls( window ); //Toolbox call to redraw controls
	DrawContents( window ); //Redraw our window contents,
	
	AdjustMenus(); 			//Make sure menus are current
	EndUpdate( window );
}


/****************** DrawContents ***********************/
//Borrowed from the Macintosh Programming Primer by Dave Mark & C. Reed.
//Comments are mine along with minor revisions.
void	DrawContents( WindowPtr window )
{
	//In a real app you would maintain a handle to your window data and
	//redraw it here.
	PicHandle	currentPicture;
	Rect		windowRect;
	RgnHandle	tempRgn;
	
	SetPort( window ); //New for version 1.5 to handle multiple windows
	
	tempRgn = NewRgn();
	GetClip( tempRgn );
	
	windowRect = window->portRect;
	windowRect.right -= kScrollBarWidth;
	EraseRect( &windowRect );
	
	ClipRect( &windowRect );
	
	currentPicture = (PicHandle)GetIndResource( 'PICT',
			GetControlValue( ((WindowPeek)window)->controlList ) );
						
	if ( currentPicture == nil )
		DoError( "\pCan't Load PICT resource!" );
		
	CenterPict( currentPicture, &windowRect );
	DrawPicture( currentPicture, &windowRect );
	
	SetClip( tempRgn );
	DisposeRgn( tempRgn );
}

	
/*****************************  DoContent  *****************************/
//Borrowed from the Macintosh Programming Primer by Dave Mark & C. Reed.
void	DoContent( WindowPtr window, Point globalPoint )
{
	short			thePart;
	Point			thePoint;
	ControlHandle	theControl;
	
	thePoint = globalPoint;
	GlobalToLocal( &thePoint );
	
	thePart = FindControl( thePoint, window, &theControl );
	
	if ( theControl == ((WindowPeek)window)->controlList )
	{
		if ( thePart == inThumb )
		{
			thePart = TrackControl( theControl, thePoint, kNilActionProc );
			InvalRect( &(window->portRect) );
		}
		else
		{
			gActionUPP = NewControlActionProc( ScrollProc );
			thePart = TrackControl( theControl, thePoint, gActionUPP );
		}
	}
}


/****************** HasColorQD *****************/
//Borrowed from the Macintosh Programming Primer by Dave Mark & C. Reed.
Boolean	HasColorQD( void )
{
	unsigned char	version[ 4 ];
	OSErr			err;
	
	err = Gestalt( gestaltQuickdrawVersion, (long *)version );
	
	if ( version[ 2 ] > 0 ) //32 bit color quickdraw or later.
		return( true );
	else
		return( false );
}


/***************** HasSystem7 *****************/
//Based on above function
Boolean	HasSystem7( void )
{
	unsigned char	version[ 4 ];
	OSErr			err;
	
	err = Gestalt( gestaltSystemVersion, (long *)version );
	
	if ( version[ 2 ] >= 0x07 ) //system 7.0 or higher.
		return( true );
	else
		return( false );
}


/****************** CenterPict ********************/
//Borrowed from the Macintosh Programming Primer by Dave Mark & C. Reed.
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


/***************** DoError ********************/
//Borrowed from the Macintosh Programming Primer by Dave Mark & C. Reed.
//Comments are mine.
void	DoError( Str255 errorString )
{
	//Set the text to our error string…
	ParamText( errorString, "\p", "\p", "\p" );
	//…show the alert to the poor user…
	StopAlert( kErrorALRTid, kNullFilterProc );
	//…then quit gracefully. 
	ExitToShell();
}

/************************************* DoCloseWindow */

Boolean		CloseAWindow( WindowPtr window )
{	
	DisposeWindow( window ); //Erases it from the screen and discards storage.
	window = FrontWindow();  //Find our next window…
	if( window != nil )		 //…if there is one set the port & update it.
	{
		SetPort( window );
		DoUpdate( window );
	}
	AdjustMenus();			 //Make our menus up to date.
	InitCursor();			 //Make sure we have an arrow cursor.
	return( true );
}

/***************************  InitMyWinColor()  **************************/
void InitMyWinColor( WindowPtr window )
{
	GrafPtr			savePort;
	WCTabHandle		colors;
	
	//Save our grafPort before starting.
	GetPort(&savePort);
	
	// Allocate memory for our color table
	colors = (WCTabHandle)NewHandle( sizeof( WinCTab ) + (13 * sizeof( ColorSpec )));
	
	// If we couldn't get the memory then show error and quit.
	if( !colors )
		DoError( "\pCouldn't create color table, Bye..." );
	
	//Lock the handle down in case something moves memory.
	HLock( (Handle) colors );
	
	// Set size to 13. There are 13 parts to window color tables starting with system 7.0
	// Make sure you have checked for System 7 or later and color quickdraw...
	(*colors)->ctSize = 13;
	
	//The next part initializes the colors to some values I though looked nice.
	//We could have used what was out there in memory if we were real adventurous.
	//I intend to  use a 'wctb' resourse and read in the color table with version 2.0.
	
	// Window content color
	(*colors)->ctTable[0].value = wContentColor;
	(*colors)->ctTable[0].rgb.red = 13000;
	(*colors)->ctTable[0].rgb.green = 30000;
	(*colors)->ctTable[0].rgb.blue = 20000;
	
	// Window frame color
	(*colors)->ctTable[1].value = wFrameColor;
	(*colors)->ctTable[1].rgb.red = 0;
	(*colors)->ctTable[1].rgb.green = 0;
	(*colors)->ctTable[1].rgb.blue = 0;
	
	// Window text color
	(*colors)->ctTable[2].value = wTextColor;
	(*colors)->ctTable[2].rgb.red = 0;
	(*colors)->ctTable[2].rgb.green = 0;
	(*colors)->ctTable[2].rgb.blue = 0;
	
	// Window hilite color
	(*colors)->ctTable[3].value = wHiliteColor;
	(*colors)->ctTable[3].rgb.red = 0;
	(*colors)->ctTable[3].rgb.green = 32434;
	(*colors)->ctTable[3].rgb.blue = 0;
	
	// Window title bar color
	(*colors)->ctTable[4].value = wTitleBarColor;
	(*colors)->ctTable[4].rgb.red = 65535;
	(*colors)->ctTable[4].rgb.green = 0;
	(*colors)->ctTable[4].rgb.blue = 20000;
	
	// Window hilite color light
	(*colors)->ctTable[5].value = wHiliteColorLight;
	(*colors)->ctTable[5].rgb.red = 0;
	(*colors)->ctTable[5].rgb.green = 0;
	(*colors)->ctTable[5].rgb.blue = 0;
	
	// Window hilite color dark
	(*colors)->ctTable[6].value = wHiliteColorDark;
	(*colors)->ctTable[6].rgb.red = 13000;
	(*colors)->ctTable[6].rgb.green = 30000;
	(*colors)->ctTable[6].rgb.blue = 20000;
	
	// Window title bar light
	(*colors)->ctTable[7].value = wTitleBarLight;
	(*colors)->ctTable[7].rgb.red = 13000;
	(*colors)->ctTable[7].rgb.green = 30000;
	(*colors)->ctTable[7].rgb.blue = 20000;
	
	// Window title bar dark
	(*colors)->ctTable[8].value = wTitleBarDark;
	(*colors)->ctTable[8].rgb.red = 0;
	(*colors)->ctTable[8].rgb.green = 0;
	(*colors)->ctTable[8].rgb.blue = 0;
	
	// Window dialog light
	(*colors)->ctTable[9].value = wDialogLight;
	(*colors)->ctTable[9].rgb.red = 0;
	(*colors)->ctTable[9].rgb.green = 20000;
	(*colors)->ctTable[9].rgb.blue = 0;
	
	// Window dialog dark
	(*colors)->ctTable[10].value = wDialogDark;
	(*colors)->ctTable[10].rgb.red = 10000;
	(*colors)->ctTable[10].rgb.green = 40000;
	(*colors)->ctTable[10].rgb.blue = 10000;
	
	// Window tinge light
	(*colors)->ctTable[11].value = wTingeLight;
	(*colors)->ctTable[11].rgb.red = 8000;
	(*colors)->ctTable[11].rgb.green = 25000;
	(*colors)->ctTable[11].rgb.blue = 15000;
	
	// Window tinge dark
	(*colors)->ctTable[12].value = wTingeDark;
	(*colors)->ctTable[12].rgb.red = 18000;
	(*colors)->ctTable[12].rgb.green = 40000;
	(*colors)->ctTable[12].rgb.blue = 30000;
	
	//Now call SetWinColor() to set the window to our color table.
	SetWinColor( window, colors );
	// Unlock the handle now that we are done…
	HUnlock( (Handle) colors );
	//…and restore the port.
	SetPort(savePort);
}

/***************************  InitMyControlColor()  **************************/
void InitMyControlColor( WindowPtr window )
{
	GrafPtr			savePort;
	CCTabHandle		colors;
	WindowPeek		myWinPeek;
	ControlHandle	myCtlHandle;
	
	//Save our grafPort before starting.
	GetPort(&savePort);
	
	// Allocate memory for our color table
	colors = (CCTabHandle)NewHandle( sizeof( ColorTable ) + (15 * sizeof( ColorSpec )));
	
	// If we couldn't get the memory then show error and quit.
	if( !colors )
		DoError( "\pCouldn't create control color table, Bye..." );
	
	myWinPeek = (WindowPeek) window;
	myCtlHandle = myWinPeek->controlList;
	
	while( myCtlHandle )
	{
		//Lock the handle down in case something moves memory.
		HLock( (Handle) colors );
		
		// Set size to 15. There are 15 parts to control color tables for scroll bars
		// and 3 for buttons and check boxes.  I am using 15 to assign all fields to 
		// keep the value field consistent with the number of the arroy element.  
		// You can assign the fields in any order if you like and only need to use 12.
		// You can also assign only the ones you want and the rest will be set to the 
		// system default.
		// Make sure you have checked for System 7 or later and color quickdraw...
		(*colors)->ctSize = 15;
		
		//The next part initializes the colors to some values I though looked nice.
		//We could have used what was out there in memory if we were real adventurous.
		//I intend to  use a 'cctb' resourse and read in the color table with version 2.0.
		
		// Control Frame color
		// Frame color for scroll bars, used to produce the foreground color
		// for scroll arrows & gray area.
		(*colors)->ctTable[0].value = cFrameColor;
		(*colors)->ctTable[0].rgb.red = 0;
		(*colors)->ctTable[0].rgb.green = 0;
		(*colors)->ctTable[0].rgb.blue = 0;
		
		// Control Body color
		// Body color for scroll bars, used to produce colors in the scroll box.
		(*colors)->ctTable[1].value = cBodyColor;
		(*colors)->ctTable[1].rgb.red = 65535;
		(*colors)->ctTable[1].rgb.green = 65535;
		(*colors)->ctTable[1].rgb.blue = 65535;
		
		// Control Text color
		(*colors)->ctTable[2].value = cTextColor;
		(*colors)->ctTable[2].rgb.red = 65535;
		(*colors)->ctTable[2].rgb.green = 65535;
		(*colors)->ctTable[2].rgb.blue = 65535;
		
		// Control Thumb color
		// Not used.  I set the color to black.
		(*colors)->ctTable[3].value = cThumbColor;
		(*colors)->ctTable[3].rgb.red = 65535;
		(*colors)->ctTable[3].rgb.green = 65535;
		(*colors)->ctTable[3].rgb.blue = 65535;
		
		// Control FillPat color
		// Not used.  I set the color to white.
		(*colors)->ctTable[4].value = cFillPatColor;
		(*colors)->ctTable[4].rgb.red = 65535;
		(*colors)->ctTable[4].rgb.green = 65535;
		(*colors)->ctTable[4].rgb.blue = 65535;
		
		// Control ArrowColorLight color
		(*colors)->ctTable[5].value = cArrowsColorLight;
		(*colors)->ctTable[5].rgb.red = 16000;
		(*colors)->ctTable[5].rgb.green = 50000;
		(*colors)->ctTable[5].rgb.blue = 30000;
		
		// Control ArrowColorDark color
		(*colors)->ctTable[6].value = cArrowsColorDark;
		(*colors)->ctTable[6].rgb.red = 8000;
		(*colors)->ctTable[6].rgb.green = 25000;
		(*colors)->ctTable[6].rgb.blue = 15000;
		
		// Control ThumbLight color
		(*colors)->ctTable[7].value = cThumbLight;
		(*colors)->ctTable[7].rgb.red = 26000;
		(*colors)->ctTable[7].rgb.green = 60000;
		(*colors)->ctTable[7].rgb.blue = 40000;
		
		// Control ThumbDark color
		(*colors)->ctTable[8].value = cThumbDark;
		(*colors)->ctTable[8].rgb.red = 13000;
		(*colors)->ctTable[8].rgb.green = 30000;
		(*colors)->ctTable[8].rgb.blue = 20000;
		
		// Control HiliteLight color
		(*colors)->ctTable[9].value = cHiliteLight;
		(*colors)->ctTable[9].rgb.red = 20000;
		(*colors)->ctTable[9].rgb.green = 35000;
		(*colors)->ctTable[9].rgb.blue = 20000;
		
		// Control HiliteDark color
		(*colors)->ctTable[10].value = cHiliteDark;
		(*colors)->ctTable[10].rgb.red = 10000;
		(*colors)->ctTable[10].rgb.green = 30000;
		(*colors)->ctTable[10].rgb.blue = 10000;
		
		// Control TitleBarLight color
		(*colors)->ctTable[11].value = cTitleBarLight;
		(*colors)->ctTable[11].rgb.red = 26000;
		(*colors)->ctTable[11].rgb.green = 60000;
		(*colors)->ctTable[11].rgb.blue = 40000;
		
		// Control TitleBarDark color
		(*colors)->ctTable[12].value = cTitleBarDark;
		(*colors)->ctTable[12].rgb.red = 13000;
		(*colors)->ctTable[12].rgb.green = 30000;
		(*colors)->ctTable[12].rgb.blue = 20000;
		
		// Control TingeLight color
		(*colors)->ctTable[13].value = cTingeLight;
		(*colors)->ctTable[13].rgb.red = 16000;
		(*colors)->ctTable[13].rgb.green = 50000;
		(*colors)->ctTable[13].rgb.blue = 30000;
		
		// Control TingeDark color
		(*colors)->ctTable[14].value = cTingeDark;
		(*colors)->ctTable[14].rgb.red = 8000;
		(*colors)->ctTable[14].rgb.green = 25000;
		(*colors)->ctTable[14].rgb.blue = 15000;
		
		//Now call SetControlColor() to set the control to our color table.
		SetControlColor( myCtlHandle, colors );
		// Unlock the handle now that we are done…
		HUnlock( (Handle) colors );
		
		myCtlHandle = (*myCtlHandle)->nextControl;
	}
	//…and restore the port.
	SetPort(savePort);
}