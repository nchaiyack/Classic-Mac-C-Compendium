/*******************************************************************************

	Here are declarations for all of the C routines. These are used for
	compile time checking of our parameters.

*******************************************************************************/

void	InitToolbox( void );
void	MainEventLoop( void );

/* Event handling routines */

void	HandleEvent( EventRecord *event );

void	HandleActivate( EventRecord *event );
void	HandleDiskInsert( EventRecord *event );
void	HandleKeyPress( EventRecord *event );
void	HandleMouseDown( EventRecord *event );
void	HandleOSEvent( EventRecord *event );
void	HandleUpdate( EventRecord *event );

void	AdjustMenus( void );
void	HandleMenuCommand( long menuResult );


/* Utility routines */

void	CloseAnyWindow( WindowPtr window );
void	DeathAlert( short errorNumber );
Boolean	IsAppWindow( WindowPtr window );
Boolean	IsDAWindow( WindowPtr window );
Boolean	IsDialogWindow( WindowPtr window );
void	InvalidateScrollbars(WindowPtr theWindow);
Boolean	TrapExists(short theTrap);
TrapType	GetTrapType(short theTrap);
short	NumToolboxTraps(void);


/* Global variables */

extern	SysEnvRec	gMac;
extern	Boolean		gQuit;
extern	Boolean		gInBackground;

#define NIL			0L
#define MAXLONG		0x7FFFFFFF

/* ========================================================================= */
/* ==================== R E S O U R C E   N U M B E R S ==================== */
/* ========================================================================= */

#define	rAboutAlert				128			/* about alert */
#define	rErrorAlert				129			/* error user alert */

#define rErrorStrings			128
#define	errWimpyROMs			1
#define	errWimpySystem			2
#define	errWeirdSystem			3
#define	errNoMenuBar			4

/* The following constants are used to identify menus and their items. The menu IDs
   have an "m" prefix and the item numbers within each menu have an "i" prefix. */

#define	rMenuBar				128			/* application's menu bar */

#define	mApple					128			/* Apple menu */
#define	iAbout					1

#define	mFile					129			/* File menu */
#define	iNew					1
#define	iOpen					2
#define	iLine11					3
#define	iClose					4
#define	iSave					5
#define	iSaveAs					6
#define	iRevert					7
#define	iLine12					8
#define	iPageSetup				9
#define	iPrint					10
#define	iLine13					11
#define	iQuit					12

#define	mEdit					130			/* Edit menu */
#define	iUndo					1
#define iLine21					2
#define	iCut					3
#define	iCopy					4
#define	iPaste					5
#define	iClear					6
