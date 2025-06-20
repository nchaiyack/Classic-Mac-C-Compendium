
//============================================================================
//----------------------------------------------------------------------------
//									Interface.c
//----------------------------------------------------------------------------
//============================================================================

// I put all interface related code in here.  Interface would include event�
// handling, menus, dialog boxes, etc.  All the user interaction that takes�
// place before and after an actual game is in play.

#include "Externs.h"
#include <Sound.h>


#define kAppleMenuID		128
#define iAbout				1
#define kGameMenuID			129
#define iNewGame			1
#define iPauseGame			2
#define iEndGame			3
#define iQuit				5
#define kOptionsMenuID		130
#define iSettings			1
#define iHelp				2
#define iHighScores			3
#define kAboutPictID		132


void DoAppleMenu (short);
void DoGameMenu (short);
void DoOptionsMenu (short);
void UpdateMainWindow (void);
void HandleMouseEvent (EventRecord *);
void HandleKeyEvent (EventRecord *);
void HandleUpdateEvent (EventRecord *);
void HandleOSEvent (EventRecord *);
void HandleHighLevelEvent (EventRecord *);
void DoAbout (void);
void DoGameSettings (void);


Rect		mainWindowRect;
WindowPtr	mainWindow;
MenuHandle	appleMenu, gameMenu, optionsMenu;
Boolean		switchedOut, quitting, canPlay, openTheScores;

extern	prefsInfo	thePrefs;
extern	Rect		backSrcRect, workSrcRect;
extern	CGrafPtr	backSrcMap, workSrcMap;
extern	Boolean		pausing, playing, helpOpen, scoresOpen;


//==============================================================  Functions
//--------------------------------------------------------------  MenusReflectMode

// Depending on whether a game is in progress (paused) or not, I want�
// menu items grayed out in one case and not grayed out in the other.
// This function, when called, displays the menus correctly depending�
// on the mode we're in (playing or not playing, pausing or not).

void MenusReflectMode (void)
{
	if (playing)								// If a game is in progress�
	{
		DisableItem(gameMenu, iNewGame);		// Cannot begin another New Game.
		EnableItem(gameMenu, iPauseGame);		// Can Pause Game.
		if (pausing)							// If we are paused�
			SetItem(gameMenu, iPauseGame, 
					"\pResume Game");			// Rename item "Resume Game".
		else									// If we are not paused�
			SetItem(gameMenu, iPauseGame, 
					"\pPause Game");			// Rename item "Pause Game".
		EnableItem(gameMenu, iEndGame);			// Can End Game.
		DisableItem(optionsMenu, 0);			// Cannot change game settings.
	}
	else										// Else, if Glypha is idle�
	{
		EnableItem(gameMenu, iNewGame);			// Can begin a New Game.
		DisableItem(gameMenu, iPauseGame);		// Cannot Pause Game.
		SetItem(gameMenu, iPauseGame, 
				"\pPause Game");				// Rename item "Pause Game".
		DisableItem(gameMenu, iEndGame);		// Cannot End Game.
		EnableItem(optionsMenu, 0);				// Can change game settings.
	}
}

//--------------------------------------------------------------  DoAppleMenu

// This function takes care of handling the Apple menu (Desk Assecories and the�
// About box).

void DoAppleMenu (short theItem)
{
	Str255		daName;
	GrafPtr		wasPort;
	short		daNumber;
	
	switch (theItem)							// Depending on the item selected�
	{
		case iAbout:							// If the About item was selected�
		if ((scoresOpen) || (helpOpen))			// If high scores or help screens up�
		{
			CloseWall();						// hide them.
			scoresOpen = FALSE;					// High scores no longer open.
			helpOpen = FALSE;					// Help screen is no longer open.
												// Uncheck help & high scores menu items.
			CheckItem(optionsMenu, iHelp, helpOpen);
			CheckItem(optionsMenu, iHighScores, scoresOpen);
		}
		DoAbout();								// Bring up the About dialog.
		break;
		
		default:								// If any other item was selected (DA)�
		GetItem(appleMenu, theItem, daName);	// Get the name of the item selected.
		GetPort(&wasPort);						// Remember our port.
		daNumber = OpenDeskAcc(daName);			// Launch the Desk Accesory.
		SetPort((GrafPtr)wasPort);				// When we return, restore port.
		break;
	}
}

//--------------------------------------------------------------  DoGameMenu

// This function handles a users interaction with the Game menu.  Quitting�
// Glypha, starting a new game, resuming a paused game are handled here.

void DoGameMenu (short theItem)
{
	switch (theItem)						// Depending on menu item selected�
	{
		case iNewGame:						// If user selected New Game item�
		if ((scoresOpen) || (helpOpen))		// If high scores or help screen is up,�
		{									// close them first.
			CloseWall();
			scoresOpen = FALSE;
			helpOpen = FALSE;
			CheckItem(optionsMenu, iHelp, helpOpen);
			CheckItem(optionsMenu, iHighScores, scoresOpen);
		}
		InitNewGame();						// Initialize variables for a new game.
		MenusReflectMode();					// Properly gray out the right menu items.
		break;
		
		case iPauseGame:					// If user selected Pause Game item�
		if (pausing)						// If we are paused, resume playing.
		{
			pausing = FALSE;				// Turn off pausing flag.
			DumpBackToWorkMap();			// Restore off screen just in case.
		}									// Actually pausing a game (not resuming)�
		break;								// is not really handled here.  It's handled�
											// directly within the main game loop.
											
		case iEndGame:						// Ending a game in progress isn't really�
		break;								// handled here - this is a dummy item.
											// Ending a game is handled within the main�
											// game loop by looking for the 'command'�
											// and 'E' key explicitly.
		
		case iQuit:							// If user selected Quit item�
		quitting = TRUE;					// Set quitting flag to TRUE.
		break;
	}
}

//--------------------------------------------------------------  DoOptionsMenu

// This function handles the Options menu.  Options include game settings,�
// displaying the high scores, and bringing up the Help screen.

void DoOptionsMenu (short theItem)
{
	switch (theItem)					// Depending on which item the user selected�
	{
		case iSettings:					// If user selected Game Settings item�
		if ((scoresOpen) || (helpOpen))	// Close high scores or help screen.
		{
			CloseWall();
			scoresOpen = FALSE;
			helpOpen = FALSE;
			CheckItem(optionsMenu, iHelp, helpOpen);
			CheckItem(optionsMenu, iHighScores, scoresOpen);
		}
		DoGameSettings();				// Bring up game settings dialog.
		break;
		
		case iHelp:						// If user selected Help item�
		if (helpOpen)					// If Help open, close it.
		{
			CloseWall();
			helpOpen = FALSE;
		}
		else							// Else, if Help is not open - open it.
		{
			if (scoresOpen)				// If the High Scores are up though,�
			{
				CloseWall();			// Close them first.
				scoresOpen = FALSE;
				CheckItem(optionsMenu, iHighScores, scoresOpen);
			}
			OpenHelp();					// Now open the Help screen.
		}
		CheckItem(optionsMenu, iHelp, helpOpen);
		break;
		
		case iHighScores:				// If user selected High Scores�
		if (scoresOpen)					// If the High Scores are up, close them.
		{
			CloseWall();
			scoresOpen = FALSE;
		}
		else							// If the High Scores are not up�
		{
			if (helpOpen)				// First see if Help is open.
			{
				CloseWall();			// And close the Help screen.
				helpOpen = FALSE;
				CheckItem(optionsMenu, iHelp, helpOpen);
			}
			OpenHighScores();			// Now open the High Scores.
		}
		CheckItem(optionsMenu, iHighScores, scoresOpen);
		break;
	}
}

//--------------------------------------------------------------  DoMenuChoice

// This is the main menu-handling function.  It examines which menu was selected�
// by the user and passes on to the appropriate function, the item within that�
// menu that was selected.

void DoMenuChoice (long menuChoice)
{
	short		theMenu, theItem;
	
	if (menuChoice == 0)			// A little error checking.
		return;
	
	theMenu = HiWord(menuChoice);	// Extract which menu was selected.
	theItem = LoWord(menuChoice);	// Extract which item it was that was selected.
	
	switch (theMenu)				// Now, depending upon which menu was selected�
	{
		case kAppleMenuID:			// If the Apple menu selected�
		DoAppleMenu(theItem);		// Call the function that handles the Apple menu.
		break;
		
		case kGameMenuID:			// If the Game menu selected�
		DoGameMenu(theItem);		// Call the function that handles the Game menu.
		break;
		
		case kOptionsMenuID:		// If the Options menu selected�
		DoOptionsMenu(theItem);		// Call the function that handles the Options menu.
		break;
	}
	
	HiliteMenu(0);					// "De-invert" menu.
}

//--------------------------------------------------------------  UpdateMainWindow

// This is a simple function that simply copies the contents from the�
// background offscreen pixmap to the main screen.  It is primarily�
// called in response to an update event, but could be called any time�
// when I want to force the screen to be redrawn.

void UpdateMainWindow (void)
{
	CopyBits(&((GrafPtr)backSrcMap)->portBits, 
			&(((GrafPtr)mainWindow)->portBits), 
			&mainWindowRect, &mainWindowRect, 
			srcCopy, 0L);
}

//--------------------------------------------------------------  HandleMouseEvent

// Mouse clicks come here.  This is standard event-handling drivel.  No different 
// from any other standard Mac program (game or otherwise).

void HandleMouseEvent (EventRecord *theEvent)
{
	WindowPtr	whichWindow;
	long		menuChoice;
	short		thePart;
												// Determine window and where in window.
	thePart = FindWindow(theEvent->where, &whichWindow);
	
	switch (thePart)							// Depending on where mouse was clicked�
	{
		case inSysWindow:						// In a Desk Accesory.
		SystemClick(theEvent, whichWindow);		// (Is this stuff obsolete yet?)
		break;
		
		case inMenuBar:							// Selected a menu item.
		menuChoice = MenuSelect(theEvent->where);
		if (canPlay)							// Call menu handling routine.
			DoMenuChoice(menuChoice);
		break;
		
		case inDrag:							// Like the lazy bastard I am�
		case inGoAway:							// I'll just ignore these.
		case inGrow:							// But, hey, the window isn't�
		case inZoomIn:							// movable or growable!
		case inZoomOut:
		break;
		
		case inContent:							// Click in the window itself.
		FlashObelisks(TRUE);					// Do lightning animation.
		LogNextTick(3);							// Lightning will hit cursor location.
		GenerateLightning(theEvent->where.h, theEvent->where.v - 20);
		StrikeLightning();
		WaitForNextTick();
		StrikeLightning();
		LogNextTick(2);
		WaitForNextTick();
		PlayExternalSound(kLightningSound, kLightningPriority);
		LogNextTick(3);
		GenerateLightning(theEvent->where.h, theEvent->where.v - 20);
		StrikeLightning();
		WaitForNextTick();
		StrikeLightning();
		LogNextTick(2);
		WaitForNextTick();
		LogNextTick(3);
		GenerateLightning(theEvent->where.h, theEvent->where.v - 20);
		StrikeLightning();
		WaitForNextTick();
		StrikeLightning();
		LogNextTick(2);
		WaitForNextTick();
		PlayExternalSound(kLightningSound, kLightningPriority);
		LogNextTick(3);
		GenerateLightning(theEvent->where.h, theEvent->where.v - 20);
		StrikeLightning();
		WaitForNextTick();
		StrikeLightning();
		LogNextTick(2);
		WaitForNextTick();
		FlashObelisks(FALSE);
		break;
	}
}

//--------------------------------------------------------------  HandleKeyEvent

// More standard issue.  This function handles any keystrokes when no game is
// in session.  Command-key strokes handled here too.

void HandleKeyEvent (EventRecord *theEvent)
{
	char		theChar;
	Boolean		commandDown;
	
	theChar = theEvent->message & charCodeMask;				// Extract key hit.
	commandDown = ((theEvent->modifiers & cmdKey) != 0);	// See if command key down.
	
	if (commandDown)								// If command key down, call menu�
	{												// handling routine.
		if (canPlay)
			DoMenuChoice(MenuKey(theChar));
	}
	else
	{
		if (helpOpen)								// Handle special keys if the help�
		{											// screen is up.
			if (theChar == kUpArrowKeyASCII)		// Up arrow key scrolls help down.
			{
				if (theEvent->what == autoKey)
					ScrollHelp(-3);
				else
					ScrollHelp(-1);
			}
			else if (theChar == kDownArrowKeyASCII)	// Down arrow key scrolls help up.
			{
				if (theEvent->what == autoKey)
					ScrollHelp(3);
				else
					ScrollHelp(1);
			}
			else if (theChar == kPageDownKeyASCII)	// Handle page down for help screen.
			{
				ScrollHelp(199);
			}
			else if (theChar == kPageUpKeyASCII)	// Handle page up for help.
			{
				ScrollHelp(-199);
			}
			else if ((theChar == kHelpKeyASCII) && (!playing))
			{										// Hitting Help key closes help�
				CloseWall();						// (if it's already open).
				helpOpen = FALSE;
				CheckItem(optionsMenu, iHelp, helpOpen);
			}
		}
		else if ((theChar == kHelpKeyASCII) && (!playing))
		{											// Else, if help not open and Help�
			if (scoresOpen)							// key is hit, open Help.
			{										// Close high scores if open.
				CloseWall();
				scoresOpen = FALSE;
				CheckItem(optionsMenu, iHighScores, scoresOpen);
			}
			OpenHelp();								// Open help.
			CheckItem(optionsMenu, iHelp, helpOpen);
		}
	}
}

//--------------------------------------------------------------  HandleUpdateEvent

// This function handles update events.  Standard event-handling stuff.

void HandleUpdateEvent (EventRecord *theEvent)
{	
	if ((WindowPtr)theEvent->message == mainWindow)
	{
		SetPort((GrafPtr)mainWindow);		// Don't forget this line, BTW.
		BeginUpdate((GrafPtr)mainWindow);	// I did once and it took me�
		UpdateMainWindow();					// ages to track down that bug.
		EndUpdate((GrafPtr)mainWindow);		// Well, it took me a week I think.
		canPlay = TRUE;
	}
}

//--------------------------------------------------------------  HandleOSEvent

// Handle switchin in and out events.  Standard event-handling stuff.

void HandleOSEvent (EventRecord *theEvent)
{	
	if (theEvent->message & 0x01000000)		// If suspend or resume event�
	{
		if (theEvent->message & 0x00000001)	// Specifically, if resume event�
			switchedOut = FALSE;			// I keep thinking I should do more here.
		else								// Or if suspend event�
			switchedOut = TRUE;				// What am I forgetting?
	}
}

//--------------------------------------------------------------  HandleHighLevelEvent

// Again, it's a fact I'm lazy.  AppleEvents are fairly easy to implement but�
// a nightmare to try and explain.  Filling out the below function is left as�
// and exercise to the reader.

void HandleHighLevelEvent (EventRecord *theEvent)
{	
//	theErr = AEProcessAppleEvent(theEvent);
}

//--------------------------------------------------------------  HandleEvent

// Standard event stuff.  This is the culling function that calls all the above�
// functions.  It looks for an event and if it detects one, it calls the appropriate�
// function above to handle it.

void HandleEvent (void)
{
	EventRecord	theEvent;
	long		sleep = 1L;
	Boolean		itHappened;
											// See if an event is queued up.
	itHappened = WaitNextEvent(everyEvent, &theEvent, sleep, 0L);
	
	if (itHappened)							// Ah, an event.  I live for events!
	{
		switch (theEvent.what)				// And what kind of event be ya'?
		{
			case mouseDown:					// Aiy!  Y' be a mouse click do ya'?
			HandleMouseEvent(&theEvent);
			break;
			
			case keyDown:					// Key down, key held down events.
			case autoKey:
			HandleKeyEvent(&theEvent);
			break;
			
			case updateEvt:					// Something needs redrawing!
			HandleUpdateEvent(&theEvent);
			break;
			
			case osEvt:						// Switching in and out events.
			HandleOSEvent(&theEvent);
			break;
			
			case kHighLevelEvent:			// Hmmmm.  A "what" event?
			HandleHighLevelEvent(&theEvent);
			break;
		}
	}
	else if (openTheScores)					// Check for "auto open" flag.
	{										// If TRUE, set the flag back to�
		openTheScores = FALSE;				// FALSE and open the high scores.
		OpenHighScores();
	}
}

//--------------------------------------------------------------  DoAbout

// This handles the About dialog.  It brings up the About box in a�
// simple centered window with no drag bar, close box or anything.
// Leaving the dialog is handled with a simple mouse click.

void DoAbout (void)
{
	Rect		aboutRect;
	WindowPtr	aboutWindow;
	
	SetRect(&aboutRect, 0, 0, 325, 318);		// Bring up centered window.
	CenterRectInRect(&aboutRect, &qd.screenBits.bounds);
	aboutWindow = GetNewCWindow(129, 0L, kPutInFront);
	MoveWindow((GrafPtr)aboutWindow, aboutRect.left, aboutRect.top, TRUE);
	ShowWindow((GrafPtr)aboutWindow);
	SetPort((GrafPtr)aboutWindow);
	LoadGraphic(kAboutPictID);					// Draw About dialog graphic.
	
	do											// Make sure button not down�
	{											// before proceeding.
	}
	while (Button());							// Proceed.
	do											// And now wait until the mouse�
	{											// is pressed before closing the�
	}											// window (ABout dialog).
	while (!Button());
	
	FlushEvents(everyEvent, 0);					// Flush the queue.
	
	if (aboutWindow != 0L)
		DisposeWindow(aboutWindow);				// Close the About dialog.
}

//--------------------------------------------------------------  DoGameSettings

// This one however is a good and proper dialog box.  It handles the meager�
// preference settings for Glypha.  Nothing fancy here to report.  Just a�
// straight-forward dialog calling routine.

void DoGameSettings (void)
{
	#define		kGameSettingsDialogID	133
	DialogPtr	theDial;
	long		newVolume;
	short		i, item;
	Boolean		leaving;
	
	CenterDialog(kGameSettingsDialogID);	// Center dialog, then call up.
	theDial = GetNewDialog(kGameSettingsDialogID, 0L, kPutInFront);
	SetPort((GrafPtr)theDial);
	ShowWindow((GrafPtr)theDial);			// Make visible (after centering).
	DrawDefaultButton(theDial);				// Draw border around Okay button.
	FlushEvents(everyEvent, 0);
											// Put in a default sound volume.
	SetDialogNumToStr(theDial, 3, (long)thePrefs.wasVolume);
	SelIText(theDial, 3, 0, 1024);			// Select it.
	
	leaving = FALSE;
	
	while (!leaving)
	{
		ModalDialog(0L, &item);				// Simple modal dialog filtering.
		
		if (item == 1)						// Did user hit the Okay button?
		{									// Well see if volume entered is legal.
			GetDialogNumFromStr(theDial, 3, &newVolume);
			if ((newVolume >= 0) && (newVolume <= 7))
			{								// If it is legal, we'll note it and quit.
				thePrefs.wasVolume = (short)newVolume;
				SetSoundVol((short)newVolume);
				leaving = TRUE;				// Bye.
			}
			else							// Otherwise, the volume entered is wrong.
			{								// So we'll Beep, enter the last legal�
				SysBeep(1);					// value and select the text again.
				SetDialogNumToStr(theDial, 3, (long)thePrefs.wasVolume);
				SelIText(theDial, 3, 0, 1024);
			}
		}
		else if (item == 2)					// Did the user hit the "Clear Scores"�
		{									// button?
			for (i = 0; i < 10; i++)		// Walk through and zero scores.
			{
				PasStringCopy("\pNemo", thePrefs.highNames[i]);
				thePrefs.highScores[i] = 0L;
				thePrefs.highLevel[i] = 0;
				openTheScores = TRUE;		// Bring up scores when dialog quits.
			}
			DisableControl(theDial, 2);		// Gray out Clear Scores button.
		}
	}
	
	DisposDialog(theDial);					// Clean up before going.
}

