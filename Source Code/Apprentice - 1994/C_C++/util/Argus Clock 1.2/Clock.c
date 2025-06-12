/*
	CLOCK
	
	by
	Argus Software Engineering
	21 Oct 93
	
	Revision 1.1 (4 Mar 94)
	- Changed minimum sleep variable from 15L to 60L
	  to help speed up background tasks (if any).
	- Changed UPDATE_PARAM from 60 to 90 so you can
	  read date a little easier.
	- Added military time option.
	
	Revision 1.2 (3 May 94)
	- Cleaned up code and added more error checks.
	
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CLOCK_TOP			10
#define CLOCK_SIZE			9
#define CLOCK_FONT			1
#define CLOCK_TITLE			"\pClock"
#define WIN_HEIGHT			12
#define WIN_LENGTH			55
#define FROM_RIGHT			13
#define FROM_TOP			23
#define REF_CON				0
#define PROC_ID				2
#define UPDATE_PARAM		90
#define DELTA				1
#define MAX_LINE_LENGTH		80

#define MAX_DELIMITERS   10
#define LINE_FEED        10
#define RETURN           13
#define SPACE            32
#define TAB              9

#define DEFAULTX			-100
#define DEFAULTY			-100
#define DEFAULTDATE			1
#define DEFAULTMILITARY		0

#define BASE_RES_ID			400
#define NIL_POINTER			0L
#define MOVE_TO_FRONT		-1L
#define REMOVE_ALL_EVENTS	0

#define APPLE_MENU_ID		400
#define FILE_MENU_ID		401
#define EDIT_MENU_ID		402
#define SETUP_MENU_ID		403

#define ABOUT_ITEM			1
#define ABOUT_ALERT			400
#define ERROR_ALERT_ID		401

#define NO_MBAR				BASE_RES_ID
#define NO_MENU				BASE_RES_ID+1
#define WRITE_ERROR			BASE_RES_ID+2
#define PREF_ERROR			BASE_RES_ID+3
#define PREFERENCES			500

#define QUIT_ITEM			1

#define UNDO_ITEM			1
#define CUT_ITEM			3
#define COPY_ITEM			4
#define PASTE_ITEM			5
#define CLEAR_ITEM			6

#define SET_ITEM			1
#define DATE_ITEM			2
#define MILITARY_ITEM		3
#define DEFAULT_ITEM		5

#define DRAG_THRESHOLD		5

#define MIN_SLEEP			60L
#define NIL_MOUSE_REGION	0L

#define LEAVE_WHERE_IT_IS	FALSE

#define WNE_TRAP_NUM		0x60
#define UNIMPL_TRAP_NUM		0x9F

#define NIL_STRING			"\p"
#define FATAL_ERROR			"\pFatal Error."

Boolean			gDone, gWNEImplemented;
EventRecord		gTheEvent;
MenuHandle		gAppleMenu, gFileMenu, gEditMenu, gSetupMenu;
Rect			gDragRect;

time_t			now;
long int		ticksNow, ticksOld;
struct 			tm	*date;
WindowPtr		clockWindow;
int				winX, winY, dateItem, militaryItem;
char			delimiters[MAX_DELIMITERS];
StringHandle	preferencesStringH;
StringHandle	tempH;
char			*preferencesString;
char			oldstring[80] = "Old time";

/* Prototypes - minimum required */
int		IsDAWindow		( WindowPtr whichWindow );

/******************************** main *********/

main()
{
	ToolBoxInit();
	MenuBarInit();
	SetUpDragRect();
	ReadPreferences();
	CreateWindow();
	MainLoop();
}


/*********************************** ToolBoxInit */

ToolBoxInit()
{
	InitGraf( &thePort );
	InitFonts();
	FlushEvents( everyEvent, REMOVE_ALL_EVENTS );
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( NIL_POINTER );
	InitCursor();
}


/***********************************	MenuBarInit	*/

MenuBarInit()
{
	Handle		myMenuBar;
	WindowPtr	temp;

	if ( ( myMenuBar = GetNewMBar( BASE_RES_ID ) ) == NIL_POINTER )
		ErrorHandler( NO_MBAR );
	SetMenuBar( myMenuBar );
	if ( ( gAppleMenu = GetMHandle( APPLE_MENU_ID ) ) == NIL_POINTER )
		ErrorHandler( NO_MENU );
	if ( ( gFileMenu = GetMHandle( FILE_MENU_ID ) ) == NIL_POINTER )
		ErrorHandler( NO_MENU );
	if ( ( gEditMenu = GetMHandle( EDIT_MENU_ID ) ) == NIL_POINTER )
		ErrorHandler( NO_MENU );
	if ( ( gSetupMenu = GetMHandle( SETUP_MENU_ID ) ) == NIL_POINTER )
		ErrorHandler( NO_MENU );

	AddResMenu( gAppleMenu, 'DRVR' );
	DrawMenuBar();
}


/******************************** SetUpDragRect *********/

SetUpDragRect()
{
	gDragRect = screenBits.bounds;
	gDragRect.left += DRAG_THRESHOLD;
	gDragRect.right -= DRAG_THRESHOLD;
	gDragRect.bottom -= DRAG_THRESHOLD;
}


/******************************** ReadPreferences *********/

ReadPreferences()
{
  char         *str_ptr;
  char         buffer[MAX_LINE_LENGTH];
  char         string[MAX_LINE_LENGTH];
  int			i;
	
	sprintf(delimiters,"%c%c%c%c",LINE_FEED,SPACE,RETURN,TAB);
	if( ( preferencesStringH = GetString(PREFERENCES) ) == NULL )
	{	/* Use default settings if can't read resource */
		winX = DEFAULTX;
		winY = DEFAULTY;
		dateItem = DEFAULTDATE;
		militaryItem = DEFAULTMILITARY;
	}
	else
	{	/* Use settings read from resource file */
		HLock( preferencesStringH);
		preferencesString = *preferencesStringH;
		for( i = 0; i < MAX_LINE_LENGTH - 2; i++ )
		{
			string[i] = *(preferencesString + i);
		}
		str_ptr = PtoCstr( (char *)string );
		
		if( strlen( str_ptr ) < 20 )
			ErrorHandler( PREF_ERROR );		
		
		strcpy(buffer, str_ptr);
		str_ptr = strtok(buffer,delimiters);
	   	strcpy(string,str_ptr);      
	   	winX = atoi(string);
	   	str_ptr = strtok(NULL,delimiters);
	   	strcpy(string,str_ptr);      
	   	winY = atoi(string);
	   	str_ptr = strtok(NULL,delimiters);
	   	strcpy(string,str_ptr);      
	   	dateItem = atoi(string);
	   	str_ptr = strtok(NULL,delimiters);
	   	strcpy(string,str_ptr);      
	   	militaryItem = atoi(string);
	}

  if( dateItem )
	CheckItem( gSetupMenu, DATE_ITEM, TRUE );
  else
	CheckItem( gSetupMenu, DATE_ITEM, FALSE );
	
  if( militaryItem )
	CheckItem( gSetupMenu, MILITARY_ITEM, TRUE );
  else
	CheckItem( gSetupMenu, MILITARY_ITEM, FALSE );
	
  ticksNow = TickCount();
  ticksOld = ticksNow;
}


/******************************** MainLoop *********/

MainLoop()
{
	gDone = FALSE;
	gWNEImplemented = ( NGetTrapAddress( WNE_TRAP_NUM, ToolTrap ) !=
						NGetTrapAddress( UNIMPL_TRAP_NUM, ToolTrap ) );
	while ( gDone == FALSE )
	{
		HandleEvent();
	}
}


/************************************* HandleEvent	 */

HandleEvent()
{
	char	theChar;
	GrafPtr	oldPort;
			
	if ( gWNEImplemented )
		WaitNextEvent( everyEvent, &gTheEvent, MIN_SLEEP, NIL_MOUSE_REGION );
	else
	{
		SystemTask();
		GetNextEvent( everyEvent, &gTheEvent );
	}

	switch ( gTheEvent.what )
	{
		case nullEvent:
			HandleNull();
			break;
		case mouseDown: 
			HandleMouseDown();
			break;
		case keyDown:
		case autoKey:
			theChar = gTheEvent.message & charCodeMask;
			if (( gTheEvent.modifiers & cmdKey ) != 0)
			{
				AdjustMenus(); 
				HandleMenuChoice( MenuKey( theChar ) );
			}
			break;
		case updateEvt:
			if (!IsDAWindow( (WindowPtr)gTheEvent.message ) )
			{
				strcpy( oldstring, "Update me" );
				GetPort( &oldPort );
				SetPort( (WindowPtr)gTheEvent.message );
				BeginUpdate( (WindowPtr)gTheEvent.message );
				DrawClock( clockWindow );
				EndUpdate( (WindowPtr)gTheEvent.message );
				SetPort( oldPort );
			}
			break;
	}
}


/************************************* HandleMouseDown */

HandleMouseDown()
{
	WindowPtr	whichWindow;
	short int	thePart;
	long int	menuChoice, windSize;
	
	thePart = FindWindow( gTheEvent.where, &whichWindow );
	switch ( thePart )
	{
		case inMenuBar:
			AdjustMenus();
			menuChoice = MenuSelect( gTheEvent.where );
			HandleMenuChoice( menuChoice );
			break;
		case inSysWindow: 
			SystemClick( &gTheEvent, whichWindow );
			break;
		case inDrag: 
			DragWindow( whichWindow, gTheEvent.where, &gDragRect );
			break;
		case inGoAway: 
			DisposeWindow( whichWindow );
			break;
		case inContent:
			/* SelectWindow( whichWindow );  Don't need - only 1 window */
			if( dateItem )
				DrawDate( clockWindow );
			else
				DrawMyString( clockWindow );
			DragWindow( whichWindow, gTheEvent.where, &gDragRect );	
			ticksOld = TickCount();		
			break;
	}
}

/************************************* AdjustMenus */

AdjustMenus()
{
	WindowPtr	currentWindow;
	
	currentWindow = FrontWindow();
	if (IsDAWindow( currentWindow ) )
	{
		EnableItem(gEditMenu, UNDO_ITEM );
		EnableItem(gEditMenu, CUT_ITEM );
		EnableItem(gEditMenu, COPY_ITEM );
		EnableItem(gEditMenu, PASTE_ITEM );
		EnableItem(gEditMenu, CLEAR_ITEM );
	}
	else
	{
		DisableItem(gEditMenu, UNDO_ITEM );
		DisableItem(gEditMenu, CUT_ITEM );
		DisableItem(gEditMenu, COPY_ITEM );
		DisableItem(gEditMenu, PASTE_ITEM );
		DisableItem(gEditMenu, CLEAR_ITEM );
	}
}


/************************************* IsDAWindow */

int	IsDAWindow( whichWindow )
WindowPtr	whichWindow;
{
	if ( whichWindow == NIL_POINTER )
		return( FALSE );
	else /* DA windows have negative windowKinds */
		return( ( (WindowPeek)whichWindow )->windowKind < 0 );
}


/************************************* HandleMenuChoice */

HandleMenuChoice( menuChoice )
long int	menuChoice;
{
	int	theMenu;
	int	theItem;
	
	if ( menuChoice != 0 )
	{
		theMenu = HiWord( menuChoice );
		theItem = LoWord( menuChoice );
		switch ( theMenu )
		{
			case APPLE_MENU_ID :
				HandleAppleChoice( theItem );
				break;
			case FILE_MENU_ID :
				HandleFileChoice( theItem );
				break;
			case EDIT_MENU_ID :
				HandleEditChoice( theItem );
				break;
			case SETUP_MENU_ID :
				HandleSetupChoice( theItem );
				break;
		}
		HiliteMenu( 0 );
	}
}


/********************************	HandleAppleChoice	*******/

HandleAppleChoice( theItem )
int	theItem;
{
	Str255		accName;
	int			accNumber;
	
	switch ( theItem )
	{
		case ABOUT_ITEM : 
			Alert( ABOUT_ALERT, NIL_POINTER );
			break;
		default :
			GetItem( gAppleMenu, theItem, accName );
			accNumber = OpenDeskAcc( accName );
			break;
	}
}


/********************************	HandleFileChoice	*******/

HandleFileChoice( theItem )
int	theItem;
{
	switch ( theItem )
	{
		case QUIT_ITEM :
			gDone = TRUE;
			break;
	}
}


/********************************	HandleEditChoice	*******/

HandleEditChoice( theItem )
int	theItem;
{
	SystemEdit( theItem - 1 );
}


/********************************	HandleSetupChoice	*******/

HandleSetupChoice( theItem )
int	theItem;
{
	switch ( theItem )
	{
		case SET_ITEM :
			SaveSettings();
			break;
		case DATE_ITEM :
			if( dateItem )
			{
				CheckItem( gSetupMenu, DATE_ITEM, FALSE );
				dateItem = 0;
			}
			else
			{
				CheckItem( gSetupMenu, DATE_ITEM, TRUE );
				dateItem = 1;
			}
			break;
		case MILITARY_ITEM :
			if( militaryItem )
			{
				CheckItem( gSetupMenu, MILITARY_ITEM, FALSE );
				militaryItem = 0;
			}
			else
			{
				CheckItem( gSetupMenu, MILITARY_ITEM, TRUE );
				militaryItem = 1;
			}
			break;					
		case DEFAULT_ITEM :
			winX = DEFAULTX;
			winY = DEFAULTY;
			dateItem = DEFAULTDATE;
			if( dateItem )
				CheckItem( gSetupMenu, DATE_ITEM, TRUE );
			else
				CheckItem( gSetupMenu, DATE_ITEM, FALSE );
			DisposeWindow( clockWindow );
			CreateWindow();
			/* ? SaveSettings(); ? */
			break;
	}
}


/************************************ CreateWindow  */

CreateWindow()
{
	WindowRecord	*myWinRec;
	Rect			myWinRect;
	Str255 			myTitle = CLOCK_TITLE;
	int				myprocID = PROC_ID;
	Ptr				myBehind = (Ptr)MOVE_TO_FRONT;
	long			myrefCon = REF_CON;

	myWinRect = screenBits.bounds;
	myWinRect.left = myWinRect.right - WIN_LENGTH - FROM_RIGHT;
	myWinRect.right = myWinRect.left + WIN_LENGTH;
	myWinRect.top = myWinRect.top + FROM_TOP;
	myWinRect.bottom = myWinRect.top + WIN_HEIGHT;
	
	if( ( (winX > 0) && (winX < screenBits.bounds.right) ) &&
	      ( (winY > 0) && (winY < screenBits.bounds.bottom) ) )
	{
		myWinRect.left = winX;
		myWinRect.right = myWinRect.left + WIN_LENGTH;
		myWinRect.top = winY;
		myWinRect.bottom = myWinRect.top + WIN_HEIGHT;
	}
	
	myWinRec = NIL_POINTER;
	clockWindow = NewWindow(	myWinRec, &myWinRect, myTitle, 
								TRUE, myprocID, myBehind, FALSE, myrefCon		);
	
	ShowWindow( clockWindow );
	SetPort( clockWindow );
	TextFont( CLOCK_FONT );
	TextSize( CLOCK_SIZE );
}


/******************************** ErrorHandler *********/

ErrorHandler( stringNum )
int	stringNum;
{
	StringHandle	errorStringH;
	
	if ( ( errorStringH = GetString( stringNum ) ) == NIL_POINTER )
		ParamText( FATAL_ERROR, NIL_STRING, NIL_STRING, NIL_STRING );
	else
	{
		HLock( errorStringH );
		ParamText( *errorStringH, NIL_STRING, NIL_STRING, NIL_STRING );
		HUnlock( errorStringH );
	}
	StopAlert( ERROR_ALERT_ID, NIL_POINTER );
	ExitToShell();
}


/******************************** DisplayError *********/

DisplayError( stringNum )
int	stringNum;
{
	StringHandle	errorStringH;
	
	if ( ( errorStringH = GetString( stringNum ) ) == NIL_POINTER )
	{
		ParamText( FATAL_ERROR, NIL_STRING, NIL_STRING, NIL_STRING );
		StopAlert( ERROR_ALERT_ID, NIL_POINTER );
		ExitToShell();
	}
	else
	{
		HLock( errorStringH );
		ParamText( *errorStringH, NIL_STRING, NIL_STRING, NIL_STRING );
		HUnlock( errorStringH );
		CautionAlert( ERROR_ALERT_ID, NIL_POINTER );
	}
	
}


/******************************** HandleNull *********/

HandleNull( )
{
	ticksNow = TickCount();
	if( (ticksNow - ticksOld) >= UPDATE_PARAM )
		DrawClock( clockWindow );
}


/******************************** SaveSettings *********/

SaveSettings( )
{
  	Point		temp;
	GrafPtr		portHolder;
	int			myResult = 0;
	long int 	i;
	int			errorCode;	
	char		*myStrPtr;
	char		myArr[MAX_LINE_LENGTH] = "                                        ";
	
	temp.h = 1;
	temp.v = 1;
	GetPort( &portHolder );
	SetPort( clockWindow );
	LocalToGlobal( &temp );
	SetPort( portHolder );
	winX = temp.h;
	winY = temp.v;
	
	/* String length = 22 */
	sprintf( myArr, "%5d %5d %1d %1d       ", winX - 1, winY - 1, dateItem, militaryItem );
	
	HUnlock( preferencesStringH );
	HPurge( preferencesStringH );
	LoadResource( preferencesStringH );
	HLock( preferencesStringH );      
	HNoPurge( preferencesStringH );
	
	myStrPtr = (char *)*preferencesStringH;
	myStrPtr = myStrPtr + 1;
	for( i = 0; i < 30; i++ )
	{
		*myStrPtr = myArr[i];
		myStrPtr = myStrPtr + 1;
	}
	ChangedResource( preferencesStringH );
	errorCode = ResError();
	if( errorCode != 0 )
		myResult = 1;
	WriteResource( preferencesStringH );
	errorCode = ResError();
	if( errorCode != 0 )
		myResult = 1;
	if( myResult )
		DisplayError( WRITE_ERROR );
}


/******************************** DrawClock *********/

DrawClock( theWindow )
WindowPtr	theWindow;
{
	char		s[80];
	GrafPtr		portHolder;
	int			horizontal_position;
	int			string_length;
	
	time( &now );
	date = localtime( &now );
	if( militaryItem )
		strftime( s, 80, "%H:%M", date );
	else
		strftime( s, 80, "%X", date );
		
	if( strcmp( s, oldstring ) == 0 )
	{
		/* Do nothing, same time. */
	}
	else
	{
		strcpy( oldstring, s );
		CtoPstr( s );
		string_length = StringWidth ( s );
		horizontal_position = (WIN_LENGTH / 2) - (string_length / 2);
		GetPort( &portHolder );
		SetPort( theWindow );
		EraseRect( &( theWindow->portRect ) );
		MoveTo( horizontal_position, CLOCK_TOP );
		DrawString( s );
		SetPort( portHolder );
	}
	ticksOld = ticksNow;
	
}


/******************************** DrawDate *********/

DrawDate( theWindow )
WindowPtr	theWindow;
{
	char		s[80];
	GrafPtr		portHolder;
	int			horizontal_position;
	int			string_length;
	
	time( &now );
	date = localtime( &now );
	strftime( s, 80, "%x", date );
	CtoPstr( s );
	string_length = StringWidth ( s );
	horizontal_position = (WIN_LENGTH / 2) - (string_length / 2);

	GetPort( &portHolder );
	SetPort( theWindow );
	EraseRect( &( theWindow->portRect ) );
	MoveTo( horizontal_position, CLOCK_TOP );
	DrawString( s );
	ticksOld = ticksNow;
	SetPort( portHolder );
	strcpy( oldstring, "Update me" );
}


/******************************** DrawMyString *********/

DrawMyString( theWindow )
WindowPtr	theWindow;
{
	Str255		myString = "\pARGUS";
	GrafPtr		portHolder;
	int			horizontal_position;
	int			string_length;
	
	string_length = StringWidth ( myString );
	horizontal_position = (WIN_LENGTH / 2) - (string_length / 2) + DELTA;

	GetPort( &portHolder );
	SetPort( theWindow );
	EraseRect( &( theWindow->portRect ) );
	MoveTo( horizontal_position, CLOCK_TOP );
	DrawString( myString );
	ticksOld = ticksNow;
	SetPort( portHolder );
	strcpy( oldstring, "Update me" );
}
