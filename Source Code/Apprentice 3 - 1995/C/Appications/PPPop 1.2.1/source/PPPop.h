/*******************************************************************************

	PPPop.h
	Version 1.2, 15 May 1995

*******************************************************************************/
#pragma once
#include <AppleEvents.h>

void	SetUpHeap(void);
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
void	DoContentClick(EventRecord *event, WindowPtr window);
void	AdjustMenus( void );
void	HandleMenuCommand( long menuResult );
void 	DoKeyPress(EventRecord *event);
void	DoQuit(void);

void 	AEInit(void);
void 	AEInstallHandlers(void);
pascal 	OSErr DoOpenApp(AppleEvent *event, AppleEvent *reply, long refcon);
pascal 	OSErr DoOpenDoc(AppleEvent *event, AppleEvent *reply, long refcon);
pascal 	OSErr DoPrintDoc(AppleEvent *event, AppleEvent *reply, long refcon);
pascal 	OSErr DoQuitApp(AppleEvent *event, AppleEvent *reply, long refcon);


/* Utility routines */

void	CloseAnyWindow( WindowPtr window );
void	DeathAlert( short errorNumber );
Boolean	IsAppWindow( WindowPtr window );
Boolean	IsDAWindow( WindowPtr window );
Boolean	IsDialogWindow( WindowPtr window );
Boolean	TrapExists(short theTrap);
TrapType	GetTrapType(short theTrap);
short	NumToolboxTraps(void);

/* My routines */

void 	InitProgram( void );
void	CleanUp(void);
void 	OperatePPP(short value);
void	OpenPPP(void);
void	ClosePPP(void);
void 	SetState(WindowPtr window);
Point 	GetWindowPosition(WindowPtr window);
void 	AutoPositionTimer(void);
void 	IdleJobs(void);
void 	GetWindowSize(WindowPtr window, long *windWidth, long *windHeight);
void 	CollectGarbage(void);
void	DoAboutBox(void);
Boolean IsDoubleClick(EventRecord *event);
short 	abs(short num);


/* Global variables */

extern	Boolean			gHasColorQD;
extern	Boolean			gQuit;
extern	Boolean			gInBackground;
extern	Boolean			gWillHardClose;
extern	Boolean			gSoundOn;
extern	Boolean			gReturnToFinder;
extern	Boolean			gLaunchOK[3];
extern	WindowPtr		gAppWindow;
extern	WindowPtr		gTimerWindow;
extern	WindowPtr		gAboutWindow;
extern	short			gPPPState;
extern	ControlHandle	gControl;
extern	long			gTimerResolution;
extern	Boolean			gAutoPositionTimer;



#define NIL				0L
#define MAXLONG			0x7FFFFFFF
#define kGestaltMask	1L

#define	TopLeft(myRect)		(*(Point *) &(myRect.top))
#define	BotRight(myRect)	(* (Point *) &(myRect.bottom))

#define	kBytesWanted	30720L	// 30k for garbage collection


/* ========================================================================= */
/* ==================== R E S O U R C E   N U M B E R S ==================== */
/* ========================================================================= */

#define	rErrorAlert				128			/* error user alert */

#define rErrorStrings			128
#define	errWimpyROMs			1
#define	errWimpySystem			2
#define	errWeirdSystem			3
#define	errNoMenuBar			4
#define errNoPPP				5
#define errPPPOpen				6
#define errPPPClose				7
#define errNoResource			8
#define errNoMemory				9

#define rWillClose				129
#define sHard					1
#define sSoft					2

#define rAboutBoxID				200
#define rAboutBoxID_BW			201

#define	rClick1					500			/* snd resources for click sound */
#define	rClick2					501



/* The following constants are used to identify menus and their items. The menu IDs
   have an "m" prefix and the item numbers within each menu have an "i" prefix. */

#define	rMenuBar				128			/* application's menu bar */

#define	mApple					128			/* Apple menu */
#define	iAbout					1

#define	mFile					129			/* File menu */
#define iOpenPPP				1
#define iClosePPP				2
#define iOpenConfigPPP			4
#define iOpenMacTCP				5
#define iQuit					7

#define	mEdit					130			/* Edit menu */
#define	iUndo					1
#define iLine21					2
#define	iCut					3
#define	iCopy					4
#define	iPaste					5
#define	iClear					6

#define	mPrefs					131			/* Prefs menu */
#define	iWillClose				1
#define	iSoundOn				2
#define iReturnToFinder			3
#define iTimerOn				5
#define iTotalTime				6
#define iAutoPosition			7
#define iResetTime				8


#define	rControlID				1000		//  cicn button def

#define rUpID					400			//  PPP Up PICTs (set of 4)
#define rDownID					500			//  PPP Down PICTs (set of 4)
#define rWindowID				128			//  Main window
#define kMainKind				1L			//  Refcon

#define rTimerWindID			129
#define kTimerKind				2L


