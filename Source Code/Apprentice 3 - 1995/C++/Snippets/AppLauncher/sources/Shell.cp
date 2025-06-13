/*
*	jShell.cp
*
*	AppLauncher
*	^^^^^^^^^^^
*
*	Main shell for application
*	© Andrew Nemeth (where applicable), Warrimoo Australia 1994, 1995
*
*	File created:		8 Mar 95.
*	Modified:			8, 9, 12–14 Mar 95.
*/

#include	"gConstDefines.h"								// 	Global #defines & const's
#include	"gVariables.h"									// 	Global variable defn & 'externs'
#include	"stdHighLevelApple.h"							//	Application High-Level AE

#include	"AZN_Debug.h"									//	debugging utilites
#include	"AZN_TAELaunch.h"								//	File launching AE utilities

#include 	<AppleEvents.h>								//	Mac high level AE
#include	<AEObjects.h>									//	mac OSL constants
#include	<LowMem.h>
#include	<Gestalt.h>
#include	<Balloons.h>									//	Access to System 7 help menu
#include 	<limits.h>									//	For LONG_MAX


//	FILE DEFINES…
//

//														'stopMask' parameter in '::FlushEvents()'
const short			kshZapEvents = 0;


//	PROGRAM GLOBALS
//
struct myGlobalStruct 	*gptrGlobalsRec = NULL;


//	FILE GLOBALS
//
static 	MenuHandle	ghMenus[ kMenusInBar ];
static 	Boolean		gboolUpdateMenus;
static 	RgnHandle		ghMouseRgn;

//	Globals used to “tune” the performance of MainEventLoop
//	(assume we’ll be starting in the foreground)

static	unsigned long	gulgRunQuantum;
static	unsigned long	gulgSleepQuantum;



// 	FILE FUNCTIONS…
//
static void 	toolBoxInit		( void );
static void	assignProgGlobals	( void );
static void	menuBarInit		( void );
static void	eventLoop			( void );
static void	doEvent			( EventRecord * );
static void	doMouseDown		( EventRecord * );
static void 	doMenuChoice		( long );
static void 	doAppleChoice		( short );
static void	doFileChoice		( short );
static void	doOptionsChoice	( short );
static void	doKeyPress		( EventRecord * );
static void	maintainMenus		( Boolean );

static void	quitNow			( void );



void		main( void )
//
{
	OSErr	myErr;
	long		lgFeature;


	toolBoxInit();

	INIT_DEBUG();
//														MUST be system 7!
	myErr = ::Gestalt( gestaltSystemVersion, &lgFeature );
	if ( myErr == noErr && ( lgFeature < 0x0700 ) ) 
		quitNow();
//														use Gestalt to check for OSL compliance
	myErr = ::Gestalt( gestaltFinderAttr, &lgFeature );
	if ( ( myErr == noErr ) && ( lgFeature & ( 1L << gestaltOSLCompliantFinder ) ) )
		NULL;
	else
		{
		::StopAlert( kresidALRT_Death, nil );
		quitNow();
		}

	assignProgGlobals();

	menuBarInit();

	eventLoop();	
}



void 		toolBoxInit( void )
//
//	Initialise toolbox and memory 
//
{
	::InitGraf( &qd.thePort );
	::InitFonts( );
	::FlushEvents( everyEvent, kshZapEvents );
	::InitWindows( );
	::TEInit( );
	::InitDialogs( 0L );
	::InitCursor( );
//														cut down on heap fragmentation
	::MaxApplZone( );
	::MoreMasters( );
	::MoreMasters( );
//														fire up for OSL
	if ( ::AEObjectInit() != noErr )
		quitNow();
//														fire up high leve AE
	highLevelEventInit();
}



void		assignProgGlobals( void )
//
//	A single place to assign all
//	program–wide globals
//
{
//														create global struct on heap
	gptrGlobalsRec = (myGlobalStruct *)::NewPtrClear( sizeof( myGlobalStruct ) );

	//	Note on the above
	//	Don't use 'gptrGlobalsRec = new struct myGlobalStruct;' !
	//	Operator new in ANSI library allocates blocks of 
	//	memory in 65532 byte slabs.  As this global will be 
	//	hanging around for the entire program, it is
	//	much more efficient to only allocate a ptr of
	//	size 352 bytes (or so).
	//
//														initialise global values
	if ( gptrGlobalsRec )
		{
		gptrGlobalsRec->boolDone = FALSE;
//														event 'tuning' globals
		gptrGlobalsRec->foregroundRunQuantum = 0L;	
		gptrGlobalsRec->foregroundSleepQuantum = GetCaretTime( );
		gptrGlobalsRec->backgroundRunQuantum = 0L;
		gptrGlobalsRec->backgroundSleepQuantum = LONG_MAX;
		}
}


void 		menuBarInit( void )
//
//	Install about menu and then
//	zap all items in menubar
//
{
	MenuHandle	hMenu = NULL;
	Handle		hMenuBar = NULL;


	hMenuBar = ::GetNewMBar( kMenuBar_ID );
	ASSERT( hMenuBar != nil );
	
	::SetMenuBar( hMenuBar );
//														assign Global MenuHandle Array
	for ( short i = kAppleNdx, j = kMApple_ID; i < kMenusInBar ; i++, j++ )
		{
		ghMenus[ i ] = (MenuHandle)::GetMenu( j );
		ASSERT( ghMenus[ i ] != nil );
		}
//														insert 'About Application' into apple Menu
	hMenu = ::GetMHandle( kMApple_ID );
	ASSERT( hMenu != nil );

	::AddResMenu( hMenu, 'DRVR' );

	::DrawMenuBar( );
	gboolUpdateMenus = FALSE;
}



void 		eventLoop( void )
//
//	Main event handling loop
//
{
	EventRecord		erRec;
	unsigned long		ulgCheckEvents = 0L;


	gulgRunQuantum = gptrGlobalsRec->foregroundRunQuantum;
	gulgSleepQuantum = gptrGlobalsRec->foregroundSleepQuantum;
	ghMouseRgn = nil;
	gboolUpdateMenus = TRUE;

	gptrGlobalsRec->boolDone = FALSE;

	while ( ! gptrGlobalsRec->boolDone )
		{
		if ( gulgRunQuantum == 0  || ( ::TickCount( ) > ulgCheckEvents ) )
			{
			ulgCheckEvents = ::TickCount() + gulgRunQuantum;
			::WaitNextEvent( everyEvent, &erRec, gulgSleepQuantum, ghMouseRgn );
			doEvent( &erRec );
			}
		}
//														final tidy-up prior to exit
	quitNow();
}




void 		doEvent( EventRecord *ptrEventRec )
//
//	Events are first dealt with here
//	Note: most of these events are processed
//	in the individual dialogFilter procedures!
//
{
	switch ( ptrEventRec->what )
		{
//														null events
		case nullEvent:
			break;
//														see 'stdHighLevelApple.cp'
		case kHighLevelEvent:
			::AEProcessAppleEvent( ptrEventRec );
			break;
//														key press
		case autoKey:
		case keyDown:
			doKeyPress( ptrEventRec );
			break;
//														rat press
		case mouseDown:
			doMouseDown( ptrEventRec );
			break;
//														not handled
		case activateEvt:
		case updateEvt:
			break;
//														disc insertions
		case diskEvt:
			if ( ptrEventRec->message >> 16 )
				{
				static	Point	where = { 50, 50 };
				::DIBadMount( where, ptrEventRec->message );
				}
			break;
//														multi-finder foreground/ background
		case	osEvt:
			switch ( ( ptrEventRec->message & osEvtMessageMask ) >> 24 )
				{
				case	mouseMovedMessage:
					break;
					
				case	suspendResumeMessage:					
					if ( ptrEventRec->message & resumeFlag )
						{
						::SetCursor( &qd.arrow );

						gulgRunQuantum = gptrGlobalsRec->foregroundRunQuantum;
						gulgSleepQuantum = gptrGlobalsRec->foregroundSleepQuantum;
						}
					else
						{
						gulgRunQuantum = gptrGlobalsRec->backgroundRunQuantum;
						gulgSleepQuantum = gptrGlobalsRec->backgroundSleepQuantum;
						}
					break;
				}
			break;
		}
//														update menus when there's an interesting event
	if ( ptrEventRec->what != nullEvent && gboolUpdateMenus )
		maintainMenus( TRUE );
}



void 		doMouseDown( EventRecord  *ptrEventRec )
//
//	Someone squeezed the rat…
//
{
	WindowPtr		ptrWindow;
	short		thePart;
	long			lgMenuChoice;
	

	thePart = ::FindWindow( ptrEventRec->where, &ptrWindow );
	switch( thePart )
		{
//														menu selected
		case inMenuBar:
			lgMenuChoice = ::MenuSelect( ptrEventRec->where );
			doMenuChoice( lgMenuChoice );
			break;
//														window dragged about - not handled
		case inDrag:
		case inContent:
			break;
//														DA's
		case inSysWindow:
			::SystemClick( ptrEventRec, ptrWindow );
			gboolUpdateMenus = TRUE;
			break;
		}
}



void 		doMenuChoice( long lgMenuChoice )
//
//	Menu chosen
//
{
	short		menu;
	short		shMenuItem;
	

	if ( lgMenuChoice != 0 )
		{
//														Macros to break 4-byte value into 2 shorts
		menu = HiWord( lgMenuChoice ); 
		shMenuItem = LoWord( lgMenuChoice );
			
		switch( menu )
			{
			case kMApple_ID:
				doAppleChoice( shMenuItem );
				break;

			case kMFile_ID:
				doFileChoice( shMenuItem );
				break;

			case kMOptions_ID:
				doOptionsChoice( shMenuItem );
				break;
//														system 7 Help menu - not handled
			case kHMHelpMenuID:
				break;
//														catch-all escape to quit application
			default:
				quitNow( );
				break;
			}
		::HiliteMenu( 0 );
		}
}



void 		doAppleChoice( short shMenuItem )
//
//	'About Application' chosen or DA
//
{
	MenuHandle	appleMenu = NULL;
	Str255		accName = { kEmptyString };
	short		accNumber;
	

	switch ( shMenuItem )
		{
		case kM_About:
//														knockout highlight on apple menu
			::FlashMenuBar( kMApple_ID );
			::NoteAlert( kresidALRT_About, kNilFilterProc );
			gboolUpdateMenus = TRUE;
			break;
//														existing Apple Menus
		default:
			appleMenu = ::GetMHandle( kMApple_ID );
			::GetItem( appleMenu, shMenuItem, accName );
			accNumber = ::OpenDeskAcc( accName );
			gboolUpdateMenus = TRUE;
			break;
		}
}



void		doFileChoice( short shMenuItem )
//
//	Parse 'File' menu choice
//
{
	switch( shMenuItem )
		{
//														signal ready for quit
		case kM_Quit:
		default:
			gptrGlobalsRec->boolDone = TRUE;
			break;
		}
}



void		doOptionsChoice( short shMenuItem )
//
//	Parse 'Options' menu choice
//	Note:  for the following code to work
//	you need to have a copy of 'TeachText'
//	or 'SimpleText' somewhere on one of
//	your drives
//
{
//														the creator for 'SimpleText' application
	const OSType		kostypeAPP = 'ttxt',
//														the creator for 'Date/Time' control panel
					kostypeCPNL = 'time';

	Boolean			boolToFront = FALSE;
	FSSpec			fsspecAPP, 
					fsspecDOC;
	OSErr			myErr = noErr;
	Boolean			boolResult = FALSE;


	switch( shMenuItem )
		{
//	BACKGROUND!
//														launch a remote application
		case kM_LaunchApp_Back:
			boolToFront = FALSE;
			myErr = TAELaunch::findApplication( kostypeAPP, &fsspecAPP );
			if ( myErr == noErr )
				{
				boolResult = TAELaunch::openItemInFinder( fsspecAPP, boolToFront );
				ASSERT( boolResult );
				}
			break;
//														launch app with doc
		case kM_LaunchDoc_Back:
			boolToFront = FALSE;
//														assemble FSSpec for readme file in default dir
			myErr = ::FSMakeFSSpec( -LMGetSFSaveDisk(),
								LMGetCurDirStore(),
								"\pREAD_ME",
								&fsspecDOC );
			if (myErr == noErr )
				{
				boolResult = TAELaunch::openItemInFinder( fsspecDOC, boolToFront );
				ASSERT( boolResult );
				}
			break;
//														open a doc in already running remote app.
		case kM_OpenDocInRemoteApp_Back:
			boolToFront = FALSE;
//														assemble FSSpec for readme file in default dir
			myErr = ::FSMakeFSSpec( -LMGetSFSaveDisk(),
								LMGetCurDirStore(),
								"\pREAD_ME",
								&fsspecDOC );
			if (myErr == noErr )
				{
				boolResult = TAELaunch::openDocInApp( kostypeAPP, fsspecDOC, boolToFront );
				ASSERT( boolResult );
				}
			break;

//	FOREGROUND!
//														launch a remote application
		case kM_LaunchApp_Fore:
			boolToFront = TRUE;
			myErr = TAELaunch::findApplication( kostypeAPP, &fsspecAPP );
			if ( myErr == noErr )
				{
				boolResult = TAELaunch::openItemInFinder( fsspecAPP, boolToFront );
				ASSERT( boolResult );
				}
			break;
//														launch app with doc
		case kM_LaunchDoc_Fore:
			boolToFront = TRUE;
//														assemble FSSpec for readme file in default dir
			myErr = ::FSMakeFSSpec( -LMGetSFSaveDisk(),
								LMGetCurDirStore(),
								"\pREAD_ME",
								&fsspecDOC );
			if (myErr == noErr )
				{
				boolResult = TAELaunch::openItemInFinder( fsspecDOC, boolToFront );
				ASSERT( boolResult );
				}
			break;
//														open a doc in already running remote app.
		case kM_OpenDocInRemoteApp_Fore:
			boolToFront = TRUE;
//														assemble FSSpec for readme file in default dir
			myErr = ::FSMakeFSSpec( -LMGetSFSaveDisk(),
								LMGetCurDirStore(),
								"\pREAD_ME",
								&fsspecDOC );
			if (myErr == noErr )
				{
				boolResult = TAELaunch::openDocInApp( kostypeAPP, fsspecDOC, boolToFront );
				ASSERT( boolResult );
				}
			break;

//	OTHER STUFF…
//														send quit AE to remote application
		case kM_QuitOtherApp:
			TAELaunch::quitRemoteApp( kostypeAPP );
			break;
//														hide remote application
		case kM_HideOtherApp:
			boolResult = TAELaunch::hideItemInFinder( kostypeAPP );
			if ( ! boolResult )
				::SysBeep( 1 );
			break;
//														open a control panel
		case kM_OpenControlPnl:
			boolResult = TAELaunch::openControlPanel( kostypeCPNL );
			ASSERT( boolResult );
			break;
//														catch-all in case of stupidity!
		default:
			quitNow();
			break;
		}

	gboolUpdateMenus = TRUE;
}



void		doKeyPress( EventRecord *ptrEventRec  )
//
//	Respond to cmd-'' keypresses
//
{
	register char		chCharPressed;


	chCharPressed = ptrEventRec->message & charCodeMask;

	if ( ( ptrEventRec->modifiers & cmdKey ) != 0 )
		{
//														quit now 'cmd-/' key press
		if ( chCharPressed == '/' && ptrEventRec->modifiers & cmdKey )
			quitNow( );
		else
			doMenuChoice( MenuKey( chCharPressed ) );
		}
}



void		maintainMenus( Boolean boolEnableAll )
//
//	Make sure menus reflect 
//	current state of application
//	If 'boolEnableAll' is FALSE, then menubar
//	is deactivated
//
{
	const short	kshDoForAll = 0;
	short	i;


	//	Enable/ disable all menu bar headings
	//
	for ( i = kAppleNdx; i < kMenusInBar ; i++ )
		{
		if ( boolEnableAll )
			::EnableItem( ghMenus[ i ], kshDoForAll );
		else
			::DisableItem( ghMenus[ i ], kshDoForAll );
		}

//														enable/ disable individual items
	if ( boolEnableAll )
		{
//														always activate
		::EnableItem( ghMenus[ kFileNdx ], kM_Quit );

		::DisableItem( ghMenus[ kOptionsNdx ], kBackground_Heading );
		::EnableItem( ghMenus[ kOptionsNdx ], kM_LaunchApp_Back );
		::EnableItem( ghMenus[ kOptionsNdx ], kM_LaunchDoc_Back );
		::EnableItem( ghMenus[ kOptionsNdx ], kM_OpenDocInRemoteApp_Back );

		::DisableItem( ghMenus[ kOptionsNdx ], kForeground_Heading );
		::EnableItem( ghMenus[ kOptionsNdx ], kM_LaunchApp_Fore );
		::EnableItem( ghMenus[ kOptionsNdx ], kM_LaunchDoc_Fore );
		::EnableItem( ghMenus[ kOptionsNdx ], kM_OpenDocInRemoteApp_Fore );

		::EnableItem( ghMenus[ kOptionsNdx ], kM_QuitOtherApp );
		::EnableItem( ghMenus[ kOptionsNdx ], kM_HideOtherApp );
		::EnableItem( ghMenus[ kOptionsNdx ], kM_OpenControlPnl );
//														wait for update!
		::InvalMenuBar( );
		}
//														deactivate mbar NOW!
	else
		::DrawMenuBar( );

	gboolUpdateMenus = FALSE;
}



void		quitNow( void )
//
//	When the cmd—'/' is pressed,
//	things to do before exit
//
{
	::ExitToShell( );
}
