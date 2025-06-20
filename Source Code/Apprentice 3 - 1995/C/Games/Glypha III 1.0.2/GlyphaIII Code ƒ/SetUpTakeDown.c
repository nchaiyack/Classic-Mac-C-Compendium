
//============================================================================
//----------------------------------------------------------------------------
//								SetUpTakeDown.c
//----------------------------------------------------------------------------
//============================================================================

// The below functions are grouped here becuase they are only called once when�
// Glypha either starts up or quits.

#include "Externs.h"
#include <Palettes.h>					// Need "Palettes.h" for the depth calls.


#define kHandPictID				128		// These are all the resource ID's for�
#define kHandMaskID				129		// the various PICT's were going to�
#define kBackgroundPictID		130		// load up into offscreen pixmaps and�
#define kHelpPictID				131		// offscreen bitmaps.
#define kObeliskPictID			134
#define kPlayerPictID			135
#define kPlayerMaskID			136
#define kNumberPictID			137
#define kIdlePictID				138
#define kEnemyWalkPictID		139
#define kEnemyFlyPictID			140
#define kEnemyWalkMaskID		141
#define kEnemyFlyMaskID			142
#define kFlamePictID			143
#define kEggPictID				144
#define kEggMaskID				145
#define kPlatformPictID			146
#define kEyePictID				147
#define kEyeMaskID				148


Boolean DoWeHaveColor (void);			// Prototypes to below functions that are�
Boolean DoWeHaveSystem605 (void);		// called locally.  These aren't really�
short WhatsOurDepth (void);				// necessary, but I don't enjoy relying on�
Boolean CanWeDisplay8Bit (GDHandle);	// the compiler to second guess me.
void SwitchToDepth (short, Boolean);


short		wasDepth;					// Only global variable defined here.

										// Other global variables defined elsewhere.
extern	Rect		mainWindowRect, backSrcRect, workSrcRect, obSrcRect, playerSrcRect;
extern	Rect		numberSrcRect, idleSrcRect, enemyWalkSrcRect, enemyFlySrcRect;
extern	Rect		obeliskRects[4], playerRects[11], numbersSrc[11], numbersDest[11];
extern	Rect		platformRects[6], touchDownRects[6], enemyRects[24], handSrcRect;
extern	Rect		flameSrcRect, flameDestRects[2], flameRects[4], platformCopyRects[9];
extern	Rect		enemyInitRects[5], eggSrcRect, platformSrcRect, helpSrcRect;
extern	Rect		handRects[2], grabZone, eyeSrcRect, eyeRects[4];
extern	GDHandle	thisGDevice;
extern	CGrafPtr	backSrcMap, workSrcMap, obeliskSrcMap, playerSrcMap;
extern	CGrafPtr	numberSrcMap, idleSrcMap, enemyWalkSrcMap, enemyFlySrcMap;
extern	CGrafPtr	flameSrcMap, eggSrcMap, platformSrcMap, helpSrcMap, handSrcMap;
extern	CGrafPtr	eyeSrcMap;
extern	GrafPtr		playerMaskMap, enemyWalkMaskMap, enemyFlyMaskMap, eggMaskMap;
extern	GrafPtr		handMaskMap, eyeMaskMap;
extern	WindowPtr	mainWindow;
extern	RgnHandle	playRgn;
extern	MenuHandle	appleMenu, gameMenu, optionsMenu;
extern	long		theScore, wasTensOfThousands;
extern	short		numLedges, beginOnLevel, levelOn, livesLeft;
extern	Boolean		quitting, playing, pausing, switchedOut, canPlay, whichList;
extern	Boolean		helpOpen, scoresOpen, openTheScores;


//==============================================================  Functions
//--------------------------------------------------------------  ToolBoxInit

// Here's the first function you ever call in Glypha (or any Mac program for�
// that matter).  The calls herein MUST be called before you do anything else.
// Otherwise, you'll get all sorts of System Errors.

void ToolBoxInit (void)
{
	InitGraf(&qd.thePort);		// Initialize QuickDraw variables for our program.
	InitFonts();				// Initialize fonts.
	FlushEvents(everyEvent, 0);	// Clear event queue of any pending events.
	InitWindows();				// Initialize the Window Manager.
	InitMenus();				// Ditto for the Menu Manager.
	TEInit();					// blah, blah Text Edit.
	InitDialogs(0L);			// blah, blah Dialog Manager.
	InitCursor();				// Set the cursor to the arrow cursor and init.
	
	MaxApplZone();				// Grab application memory.
	
	MoreMasters();				// Allocate a block of master pointers.
	MoreMasters();				// And allocate more.
	MoreMasters();				// And more.
	MoreMasters();				// Hey, lets do it again too.
	
	switchedOut = FALSE;		// We "should" be the foreground app at this time.
	GetDateTime((unsigned long *)&qd.randSeed);		// Randomize random seed.
}

//--------------------------------------------------------------  DoWeHaveColor  

// Simple function that returns TRUE if we're running on a Mac that�
// is running Color Quickdraw.

Boolean DoWeHaveColor (void)
{
	SysEnvRec		thisWorld;
	
	SysEnvirons(2, &thisWorld);		// Call the old SysEnvirons() function.
	return (thisWorld.hasColorQD);	// And return whether it has Color QuickDraw.
}

//--------------------------------------------------------------  DoWeHaveSystem605  

// Another simple "environment" function.  Returns TRUE if the Mac we're running�
// on has System 6.0.5 or more recent.  (6.0.5 introduced the ability to switch�
// color depths.)

Boolean DoWeHaveSystem605 (void)
{
	SysEnvRec		thisWorld;
	Boolean			haveIt;
	
	SysEnvirons(2, &thisWorld);		// Call the old SysEnvirons() function.
	if (thisWorld.systemVersion >= 0x0605)
		haveIt = TRUE;				// Check the System version for 6.0.5�
	else							// or more recent
		haveIt = FALSE;
	return (haveIt);
}

//--------------------------------------------------------------  WhatsOurDepth  

// Function returns the current bit depth.  For example, 4 = 16 colors, 8 = 256�
// colors.  Note, this function assumes System 6.0.5 or more recent and Color�
// Quickdraw capable.  You should haved called the previous two functions before�
// attempting this call.

short WhatsOurDepth (void)
{
	short			thisDepth;
	char			wasState;
	
	if (thisGDevice != 0L)							// Make sure we have device handle.
	{
		wasState = HGetState((Handle)thisGDevice);	// Remember the handle's state.
		HLock((Handle)thisGDevice);					// Lock the device handle down.
													// Get it's depth (pixelSize).
		thisDepth = (**(**thisGDevice).gdPMap).pixelSize;
		HSetState((Handle)thisGDevice, wasState);	// Restore handle's state.
	}
	else
		RedAlert("\pUnknown Error.");				// Post generic error message.
	
	return (thisDepth);								// Return screen depth.
}

//--------------------------------------------------------------  CanWeDisplay8Bit  

// Simple function that returns TRUE if the current device (monitor) is�
// capable of displaying 256 colors (or grays).  This function, the one above,�
// and many following functions assume we have System 6.0.5 or more recent and�
// are capable of Color QuickDraw.

Boolean CanWeDisplay8Bit (GDHandle theDevice)
{
	short		canDepth;
	Boolean		canDo;
	
	canDo = FALSE;		// Assume intially that we can't display 8 bit.
						// Call HasDepth() to see if monitor supports 8 bit.
	canDepth = HasDepth(theDevice, 8, 1, 0);
	if (canDepth != 0)	// If HasDepth() returned anything other than 0�
		canDo = TRUE;	// we can indeed switch it to 8 bit.
	
	return (canDo);		// Return the result.
}

//--------------------------------------------------------------  SwitchToDepth

// This handy function forces the device (monitor) in question to switch�
// to a specific depth.  We'll call this function if we need to switch to�
// 8-bit (256 colors).  We should have called the above function first in�
// order to be sure that we CAN in fact switch this monitor to 8-bit.

void SwitchToDepth (short newDepth, Boolean doColor)
{
	OSErr			theErr;
	short			colorFlag;
	char			tagByte;
	
	if (doColor)					// We can switch to either colors or grays.
		colorFlag = 1;
	else
		colorFlag = 0;
	
	if (thisGDevice != 0L)			// Make sure we have a device.
	{								// Remember handle's state (as usual).
		tagByte = HGetState((Handle)thisGDevice);
		HLock((Handle)thisGDevice);	// Lock it.
									// Call SetDepth() (System 6.0.5 or more recent).
		theErr = SetDepth(thisGDevice, newDepth, 1, colorFlag);
									// Restore handle's state.
		HSetState((Handle)thisGDevice, tagByte);
		if (theErr != noErr)		// If call failed, go to error dialog.
			RedAlert("\pUnknown Error.");
	}
	else							// Call error dialog if no device handle.
		RedAlert("\pUnknown Error.");
}

//--------------------------------------------------------------  CheckEnvirons

// This is the "wrapper" function that calls all the above functions.
// After calling ToolBoxInit(), Glypha will call this function to see�
// if the current Mac we're running on is capable of running Glypha.

void CheckEnvirons (void)
{
	if (!DoWeHaveColor())			// We absolutely need Color QuickDraw.
		RedAlert("\pGlypha II only runs in 256 colors.");
	
	if (!DoWeHaveSystem605())		// We absolutely need System 6.0.5. or more recent.
		RedAlert("\pGlypha II requires System 6.0.5 or better to run.");
									// If we passed the above 2 tests, get a�
	FindOurDevice();				// handle to the main device (monitor).
	
	wasDepth = WhatsOurDepth();		// Find out our monitor's depth.
	if (wasDepth != 8)				// If it's not 8-bit we'll need to switch depths.
	{								// Test 1st to see if monitor capable of 8-bit.
		if (CanWeDisplay8Bit(thisGDevice))
			SwitchToDepth(8, TRUE);	// If so, switch to 256 colors.
		else						// If not, we have to quit.
			RedAlert("\pGlypha II only runs in 256 colors.");
	}
}

//--------------------------------------------------------------  OpenMainWindow

// This is a simple function that merely opens up a large black window and�
// centers it in the monitor.  It assumes a 'WIND' (window) resource, which if you�
// look at the resource file in ResEdit, you'll find it also has a 'WCTB'�
// (window color table) resource.  The 'WCTB' resource specifies a content�
// color of black - thus this window comes up black.

void OpenMainWindow (void)
{
	SetRect(&mainWindowRect, 0, 0, 640, 460);			// Our window size.
	mainWindow = GetNewCWindow(128, 0L, kPutInFront);	// Load window from resource.
														// Make it the right size.
	SizeWindow((GrafPtr)mainWindow, mainWindowRect.right - mainWindowRect.left, 
			mainWindowRect.bottom - mainWindowRect.top, FALSE);
														// Center the window.
	MoveWindow((GrafPtr)mainWindow, (qd.screenBits.bounds.right - 640) / 2, 
			((qd.screenBits.bounds.bottom - 480) / 2) + LMGetMBarHeight(), TRUE);
	ShowWindow((GrafPtr)mainWindow);					// Now display it.
	SetPort((GrafPtr)mainWindow);						// Make its port current.
	ClipRect(&mainWindowRect);							// Set its clip region.
	CopyRgn(mainWindow->clipRgn, mainWindow->visRgn);	// Set its visRgn.
	ForeColor(blackColor);								// Set its pen color to black.
	BackColor(whiteColor);								// Set background color white.
}

//--------------------------------------------------------------  InitMenubar

// This function loads up the menus and displays the menu bar.

void InitMenubar (void)
{
	appleMenu = GetMenu(128);		// Get the Apple menu (About�).
	if (appleMenu == 0L)			// See that it loaded okay.
		RedAlert("\pCouldn't Load Menus Error");
	AddResMenu(appleMenu, 'DRVR');	// Add Desk Accesories.
	InsertMenu(appleMenu, 0);		// Add to menu bar.
	
	gameMenu = GetMenu(129);		// Next load the Game menu.
	if (gameMenu == 0L)				// Make sure it loaded as well.
		RedAlert("\pCouldn't Load Menus Error");
	InsertMenu(gameMenu, 0);		// And add it next to the menu bar.
	
	optionsMenu = GetMenu(130);		// Finally load the Options menu.
	if (optionsMenu == 0L)
		RedAlert("\pCouldn't Load Menus Error");
	InsertMenu(optionsMenu, 0);		// And insert it.
	
	DrawMenuBar();					// Now display the menu bar.
}

//--------------------------------------------------------------  InitVariables

// This function is called only once.  It initializes all the global variables�
// used by Glypha.  It may not in fact be necessary to initialize many of these�
// (your compiler should ensure that all globals are set to zero when your app�
// launches), but it's a good idea to NEVER TRUST YOUR COMPILER.

void InitVariables (void)
{
	short		i;
	
	quitting = FALSE;	// I won't describe every one of these, many are�
	playing = FALSE;	// self explanatory.  Suffice it to say that first�
	pausing = FALSE;	// I'm initializing all the Boolean variables.
	canPlay = FALSE;
	whichList = TRUE;
	helpOpen = FALSE;
	scoresOpen = FALSE;
	openTheScores = FALSE;
	
	numLedges = 3;					// Number of ledges in the "arena".
	beginOnLevel = 1;				// First level to begin playing.
	levelOn = 0;					// Level number player is on.
	livesLeft = kInitNumLives;		// Number of player lives.
	theScore = 0L;					// Player's score (notice it's a long!).
	wasTensOfThousands = 0L;		// For tracking when player gets an extra life.
	GenerateLightning(320, 240);	// Initialize a lightning bolt.
	
	backSrcRect = mainWindowRect;	// Create background offscreen pixmap.
	ZeroRectCorner(&backSrcRect);
	backSrcMap = 0L;
	CreateOffScreenPixMap(&backSrcRect, &backSrcMap);
	LoadGraphic(kBackgroundPictID);
	
	workSrcRect = mainWindowRect;	// Create "work" offscreen pixmap.
	ZeroRectCorner(&workSrcRect);
	workSrcMap = 0L;
	CreateOffScreenPixMap(&workSrcRect, &workSrcMap);
	
	SetRect(&obSrcRect, 0, 0, 20, 418);
	obeliskSrcMap = 0L;				// Create pixmap to hold "obelisk" graphics.
	CreateOffScreenPixMap(&obSrcRect, &obeliskSrcMap);
	LoadGraphic(kObeliskPictID);
	SetRect(&obeliskRects[0], 0, 0, 20, 209);
	OffsetRect(&obeliskRects[0], 0, 0);
	SetRect(&obeliskRects[1], 0, 0, 20, 209);
	OffsetRect(&obeliskRects[1], 0, 209);
	SetRect(&obeliskRects[2], 0, 0, 20, 209);
	OffsetRect(&obeliskRects[2], 161, 250);
	SetRect(&obeliskRects[3], 0, 0, 20, 209);
	OffsetRect(&obeliskRects[3], 457, 250);
	
	SetRect(&playerSrcRect, 0, 0, 48, 436);
	playerSrcMap = 0L;				// Create pixmap to hold "player" graphics.
	CreateOffScreenPixMap(&playerSrcRect, &playerSrcMap);
	LoadGraphic(kPlayerPictID);
	playerMaskMap = 0L;
	CreateOffScreenBitMap(&playerSrcRect, &playerMaskMap);
	LoadGraphic(kPlayerMaskID);
	
	SetRect(&enemyWalkSrcRect, 0, 0, 48, 576);
	enemyWalkSrcMap = 0L;			// Create pixmap to hold "enemy" graphics.
	CreateOffScreenPixMap(&enemyWalkSrcRect, &enemyWalkSrcMap);
	LoadGraphic(kEnemyWalkPictID);
	enemyWalkMaskMap = 0L;
	CreateOffScreenBitMap(&enemyWalkSrcRect, &enemyWalkMaskMap);
	LoadGraphic(kEnemyWalkMaskID);
	SetRect(&enemyFlySrcRect, 0, 0, 64, 480);
	enemyFlySrcMap = 0L;
	CreateOffScreenPixMap(&enemyFlySrcRect, &enemyFlySrcMap);
	LoadGraphic(kEnemyFlyPictID);
	enemyFlyMaskMap = 0L;
	CreateOffScreenBitMap(&enemyFlySrcRect, &enemyFlyMaskMap);
	LoadGraphic(kEnemyFlyMaskID);
	for (i = 0; i < 12; i++)
	{								// Set up enemy source rectangles.
		SetRect(&enemyRects[i], 0, 0, 48, 48);
		OffsetRect(&enemyRects[i], 0, 48 * i);
	}
	for (i = 0; i < 12; i++)
	{
		SetRect(&enemyRects[i + 12], 0, 0, 64, 40);
		OffsetRect(&enemyRects[i + 12], 0, 40 * i);
	}
	SetRect(&enemyInitRects[0], 0, 0, 48, 1);
	OffsetRect(&enemyInitRects[0], 72, 284);
	SetRect(&enemyInitRects[1], 0, 0, 48, 1);
	OffsetRect(&enemyInitRects[1], 520, 284);
	SetRect(&enemyInitRects[2], 0, 0, 48, 1);
	OffsetRect(&enemyInitRects[2], 72, 105);
	SetRect(&enemyInitRects[3], 0, 0, 48, 1);
	OffsetRect(&enemyInitRects[3], 520, 105);
	SetRect(&enemyInitRects[4], 0, 0, 48, 1);
	OffsetRect(&enemyInitRects[4], 296, 190);
	
	SetRect(&eggSrcRect, 0, 0, 24, 24);
	eggSrcMap = 0L;					// Create pixmap to hold "egg" graphics.
	CreateOffScreenPixMap(&eggSrcRect, &eggSrcMap);
	LoadGraphic(kEggPictID);
	eggMaskMap = 0L;
	CreateOffScreenBitMap(&eggSrcRect, &eggMaskMap);
	LoadGraphic(kEggMaskID);
	
	SetRect(&eyeSrcRect, 0, 0, 48, 124);
	eyeSrcMap = 0L;					// Create pixmap to hold "eye" graphics.
	CreateOffScreenPixMap(&eyeSrcRect, &eyeSrcMap);
	LoadGraphic(kEyePictID);
	eyeMaskMap = 0L;
	CreateOffScreenBitMap(&eyeSrcRect, &eyeMaskMap);
	LoadGraphic(kEyeMaskID);
	for (i = 0; i < 4; i++)
	{
		SetRect(&eyeRects[i], 0, 0, 48, 31);
		OffsetRect(&eyeRects[i], 0, i * 31);
	}
	
	SetRect(&handSrcRect, 0, 0, 56, 114);
	handSrcMap = 0L;				// Create pixmap to hold "mummy hand" graphics.
	CreateOffScreenPixMap(&handSrcRect, &handSrcMap);
	LoadGraphic(kHandPictID);
	handMaskMap = 0L;
	CreateOffScreenBitMap(&handSrcRect, &handMaskMap);
	LoadGraphic(kHandMaskID);
	SetRect(&handRects[0], 0, 0, 56, 57);
	OffsetRect(&handRects[0], 0, 0);
	SetRect(&handRects[1], 0, 0, 56, 57);
	OffsetRect(&handRects[1], 0, 57);
	SetRect(&grabZone, 0, 0, 96, 108);
	OffsetRect(&grabZone, 48, 352);
	
	SetRect(&idleSrcRect, 0, 0, 48, 48);
	idleSrcMap = 0L;				// Create pixmap to hold "shielded player".
	CreateOffScreenPixMap(&idleSrcRect, &idleSrcMap);
	LoadGraphic(kIdlePictID);
	
	SetRect(&flameSrcRect, 0, 0, 16, 64);
	flameSrcMap = 0L;				// Create pixmap to hold "torch" graphics.
	CreateOffScreenPixMap(&flameSrcRect, &flameSrcMap);
	LoadGraphic(kFlamePictID);
	SetRect(&flameDestRects[0], 0, 0, 16, 16);
	OffsetRect(&flameDestRects[0], 87, 325);
	SetRect(&flameDestRects[1], 0, 0, 16, 16);
	OffsetRect(&flameDestRects[1], 535, 325);
	for (i = 0; i < 4; i++)
	{
		SetRect(&flameRects[i], 0, 0, 16, 16);
		OffsetRect(&flameRects[i], 0, i * 16);
	}
	
	SetRect(&numberSrcRect, 0, 0, 8, 121);
	numberSrcMap = 0L;				// Create pixmap to hold "score numbers".
	CreateOffScreenPixMap(&numberSrcRect, &numberSrcMap);
	LoadGraphic(kNumberPictID);
	for (i = 0; i < 11; i++)
	{
		SetRect(&numbersSrc[i], 0, 0, 8, 11);
		OffsetRect(&numbersSrc[i], 0, 11 * i);
	}
	SetRect(&numbersDest[0], 0, 0, 8, 11);	// # of lives digit 1
	OffsetRect(&numbersDest[0], 229, 443);
	SetRect(&numbersDest[1], 0, 0, 8, 11);	// # of lives digit 2
	OffsetRect(&numbersDest[1], 237, 443);
	SetRect(&numbersDest[2], 0, 0, 8, 11);	// score digit 1
	OffsetRect(&numbersDest[2], 293, 443);
	SetRect(&numbersDest[3], 0, 0, 8, 11);	// score digit 2
	OffsetRect(&numbersDest[3], 301, 443);
	SetRect(&numbersDest[4], 0, 0, 8, 11);	// score digit 3
	OffsetRect(&numbersDest[4], 309, 443);
	SetRect(&numbersDest[5], 0, 0, 8, 11);	// score digit 4
	OffsetRect(&numbersDest[5], 317, 443);
	SetRect(&numbersDest[6], 0, 0, 8, 11);	// score digit 5
	OffsetRect(&numbersDest[6], 325, 443);
	SetRect(&numbersDest[7], 0, 0, 8, 11);	// score digit 6
	OffsetRect(&numbersDest[7], 333, 443);
	SetRect(&numbersDest[8], 0, 0, 8, 11);	// # of level digit 1
	OffsetRect(&numbersDest[8], 381, 443);
	SetRect(&numbersDest[9], 0, 0, 8, 11);	// # of level digit 2
	OffsetRect(&numbersDest[9], 389, 443);
	SetRect(&numbersDest[10], 0, 0, 8, 11);	// # of level digit 3
	OffsetRect(&numbersDest[10], 397, 443);
	
	SetRect(&playerRects[0], 0, 0, 48, 37);
	OffsetRect(&playerRects[0], 0, 0);
	SetRect(&playerRects[1], 0, 0, 48, 37);
	OffsetRect(&playerRects[1], 0, 37);
	SetRect(&playerRects[2], 0, 0, 48, 37);
	OffsetRect(&playerRects[2], 0, 74);
	SetRect(&playerRects[3], 0, 0, 48, 37);
	OffsetRect(&playerRects[3], 0, 111);
	SetRect(&playerRects[4], 0, 0, 48, 48);
	OffsetRect(&playerRects[4], 0, 148);
	SetRect(&playerRects[5], 0, 0, 48, 48);
	OffsetRect(&playerRects[5], 0, 196);
	SetRect(&playerRects[6], 0, 0, 48, 48);
	OffsetRect(&playerRects[6], 0, 244);
	SetRect(&playerRects[7], 0, 0, 48, 48);
	OffsetRect(&playerRects[7], 0, 292);
	SetRect(&playerRects[8], 0, 0, 48, 37);		// falling bones rt.
	OffsetRect(&playerRects[8], 0, 340);
	SetRect(&playerRects[9], 0, 0, 48, 37);		// falling bones lf.
	OffsetRect(&playerRects[9], 0, 377);
	SetRect(&playerRects[10], 0, 0, 48, 22);	// pile of bones
	OffsetRect(&playerRects[10], 0, 414);
	
	MoveTo(0, 0);			// Generate clipping region that excludes the obelisks.
	playRgn = NewRgn();		// Create empty new region.
	OpenRgn();				// Open region for definition.
	LineTo(0, 450);			// Walk around the vertices of our region.
	LineTo(161, 450);
	LineTo(161, 269);
	LineTo(172, 250);
	LineTo(182, 269);
	LineTo(182, 450);
	LineTo(457, 450);
	LineTo(457, 269);
	LineTo(468, 250);
	LineTo(478, 269);
	LineTo(478, 450);
	LineTo(640, 450);
	LineTo(640, 0);
	LineTo(0, 0);
	CloseRgn(playRgn);		// Stop region definition.
	MoveHHi((Handle)playRgn);
	HLock((Handle)playRgn);
	
	SetRect(&platformSrcRect, 0, 0, 191, 192);
	platformSrcMap = 0L;	// Create pixmap to hold "platform" graphic.
	CreateOffScreenPixMap(&platformSrcRect, &platformSrcMap);
	LoadGraphic(kPlatformPictID);
	for (i = 0; i < 6; i++)
	{
		SetRect(&platformCopyRects[i], 0, 0, 191, 32);
		OffsetRect(&platformCopyRects[i], 0, 32 * i);
	}
	SetRect(&platformCopyRects[6], 233, 190, 424, 222);
	SetRect(&platformCopyRects[7], 0, 105, 191, 137);
	SetRect(&platformCopyRects[8], 449, 105, 640, 137);
							// These are the platforms.  See diagram for numbering.
	SetRect(&platformRects[0], 206, 424, 433, 438);		//_______________
	SetRect(&platformRects[1], -256, 284, 149, 298);	//
	SetRect(&platformRects[2], 490, 284, 896, 298);		//--3--     --4--
	SetRect(&platformRects[3], -256, 105, 149, 119);	//     --5--
	SetRect(&platformRects[4], 490, 105, 896, 119);		//--1--     --2--
	SetRect(&platformRects[5], 233, 190, 407, 204);		//_____--0--_____
	
	for (i = 0; i < 6; i++)
	{						// "Hot rects" to sense if player landing on platform.
		touchDownRects[i] = platformRects[i];
		touchDownRects[i].left += 23;
		touchDownRects[i].right -= 23;
		touchDownRects[i].bottom = touchDownRects[i].top;
		touchDownRects[i].top = touchDownRects[i].bottom - 11;
	}
	
	SetRect(&helpSrcRect, 0, 0, 231, 398);
	helpSrcMap = 0L;		// Create pixmap to hold "help screen" graphic.
	CreateOffScreenPixMap(&helpSrcRect, &helpSrcMap);
	LoadGraphic(kHelpPictID);
	
	SetPort((GrafPtr)mainWindow);
}

//--------------------------------------------------------------  ShutItDown

// This function is called when the player has chosen to quit Glypha.
// It "should" probably do more, but in fact all it does is to restore�
// their Mac's monitor back to the depth it was before they launched�
// Glypha (recall that we may have changed it to 8-bit).

void ShutItDown (void)
{
	if (wasDepth != WhatsOurDepth())	// Need only switch if wasn't 8-bit.
		SwitchToDepth(wasDepth, TRUE);	// Switch back to out "old" depth.
}

