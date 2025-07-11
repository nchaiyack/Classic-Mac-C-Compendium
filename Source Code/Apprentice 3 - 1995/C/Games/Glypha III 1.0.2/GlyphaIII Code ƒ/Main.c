
//============================================================================
//----------------------------------------------------------------------------
//								Glypha III 1.0.2
//								by  Scheherazade
//----------------------------------------------------------------------------
//============================================================================

// Here is the "main" file for Glypha.  Here is where the game begins and ends.
// Also included are the preference calls.

#include "Externs.h"
#include <Sound.h>


#define kPrefsVersion			0x0001


void ReadInPrefs (void);
void WriteOutPrefs (void);
void main (void);


prefsInfo	thePrefs;
short		wasVolume;

extern	Boolean		quitting, playing, pausing, evenFrame;


//==============================================================  Functions
//--------------------------------------------------------------  ReadInPrefs

//	This function loads up the preferences.  If the preferences 
//	aren't found, all settings are set to their defaults.

void ReadInPrefs (void)
{
	short		i;
							// Call LoadPrefs() function - returns TRUE if it worked.
	if (LoadPrefs(&thePrefs, kPrefsVersion))
		SetSoundVol(thePrefs.wasVolume);
	else					// If LoadPrefs() failed, set defaults.
	{
		thePrefs.prefVersion = kPrefsVersion;				// version of prefs
		thePrefs.filler = 0;								// just padding
		PasStringCopy("\pYour Name", thePrefs.highName);	// last highscores name
		for (i = 0; i < 10; i++)							// loop through scores
		{
			PasStringCopy("\pNemo", thePrefs.highNames[i]);	// put "Nemo" in name
			thePrefs.highScores[i] = 0L;					// set highscore to 0
			thePrefs.highLevel[i] = 0;						// level attained = 0
		}
		GetSoundVol(&thePrefs.wasVolume);
	}
							// Get sound volume so we can restore it.
	GetSoundVol(&wasVolume);
}

//--------------------------------------------------------------  WriteOutPrefs

//	This function writes out the preferences to disk and restores 
//	the sound volume to its setting before Glypha was launched.

void WriteOutPrefs (void)
{
	if (!SavePrefs(&thePrefs, kPrefsVersion))
		SysBeep(1);
	SetSoundVol(wasVolume);
}

//--------------------------------------------------------------  main

//	This is the main function.  Every C program has one of these.
//	First it initializes our program and then falls into a loop
//	until the user chooses to quit.  At that point, it cleans up
//	and exits.

void main (void)
{
	long		tickWait;
	
	ToolBoxInit();			// Call function that initializes the ToolBox managers.
	CheckEnvirons();		// Check the Mac we're on to see if we can run.
	OpenMainWindow();		// Open up the main window - it will fill the monitor.
	InitVariables();		// Initialize Glypha's variables.
	InitSound();			// Create sound channels and load up sounds.
	InitMenubar();			// Set up the game's menubar.
	ReadInPrefs();			// Load up the preferences.
	
	do						// Here begins the main loop.
	{
		HandleEvent();		// Check for events.
		if ((playing) && (!pausing))
			PlayGame();		// If user began game, drop in game loop. (play mode)
		else				// If no game, animate the screen. (idle mode)
		{
			tickWait = TickCount() + 2L;
			evenFrame = !evenFrame;
			DrawTorches();	// Flicker torches.
			CopyAllRects();	// Refresh screen.
			do				// Wait for 2 Ticks to pass to keep fast Macs at bay.
			{
			}
			while (TickCount() < tickWait);
		}
	}
	while (!quitting);
	
	KillSound();			// Dispose of sound channels.
	ShutItDown();			// Dispose of other structures.
	WriteOutPrefs();		// Save preferences to disk.
}

