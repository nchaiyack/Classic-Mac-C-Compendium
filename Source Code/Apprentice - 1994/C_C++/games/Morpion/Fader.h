#ifndef __FADER__
#define __FADER__
/*
	DarkSide 4.0 - a 7.0 dependant, system clean expandable screen saver.
	
	copyright ©Ê1990, 1991, 1992, 1993 by Tom Dowdy
	All rights reserved.
	
	This header file defines the interface between DarkSide and one of
	its faders.

*/

#ifndef __TYPES__
	#include <Types.h>
#endif
#ifndef __OSUTILS__
	#include <OSUtils.h>
#endif
#ifndef __QUICKDRAW__
	#include <QuickDraw.h>
#endif
#ifndef __SOUND__
	#include <Sound.h>
#endif

#define	preflightFader		0
#define initializeFader		1
#define idleFader			2
#define disposeFader		3
#define updateFader			4
#define hitFader			5

// info about a screen
typedef struct
	{
	Rect		bounds;					// bounds of that screen within our window
	Boolean		isColor;				// color or bw/gray
	short		depth;					// depth in pixels
	GDHandle	theDevice;				// device that goes with that screen
	
	short		originalDepth;			// depth before any changes
	short		originalFlags;			// original device flags
	short		whichFlagsChanged;		// which flags we changed
	
	Handle		screenGamma;			// copy of the screen's original gamma table
	short		brightness;				// current monitor brightness (0-255)
	short		newBrightness;			// new monitor brightness to set (0-255)
	} ScreenInfo, *ScreenInfoPtr;
		
// storage for the user's settings
#define numShorts	20
#define numStrings	4
typedef struct
	{
	short				theShorts[numShorts];
	Str255				theStrings[numStrings];
	} SettingsBlock, *SettingsPtr;

// currently supported options
#define keyAndMouseUnfade		0x0001	// 0 if fader is allowing keystrokes/mouse motion
#define faderRequestsUnfade		0x0002	// 1 if fader wishes to cause an unfade
#define faderRequestsNoWindow 	0x0004	// 1 if fader wishes no fade window
#define ignoreModifierKeys		0x0008	// 1 if fader wishes us to ignore modifiers

// info about the machine
typedef struct
	{
	SysEnvRec		theEnvirons;		// info about the machine
	WindowPtr		fadeWindow;			// where the fade is taking place
	Ptr				applicationQD;		// app QuickDraw globals
	long			applicationA5;		// A5 for the application
	long			faderOptions;		// options for the fader to give to DarkSide
	SettingsPtr		faderSettings;		// user's settings for the fader
	DialogPtr		settingsDialog;		// dialog with the settings
	short			soundVolume;		// volume set by the user
	SndChannelPtr	faderChannel;		// sound channel allocated for fader's use
	Ptr				callbackLoader;		// segment loader for the callbacks
	
	short				numScreens;			// how many screens there are
	ScreenInfo			theScreens[20];		// info about each screen
	} MachineInfoRec, *MachineInfoPtr;
	

// typedefs for the standard fader entry points	
OSErr	PreflightFader(MachineInfoPtr machineInfo, long *minTicks, long *maxTicks);
OSErr	InitializeFader(MachineInfoPtr machineInfo);
OSErr	IdleFader(MachineInfoPtr machineInfo);
OSErr	DisposeFader(MachineInfoPtr machineInfo);
OSErr	UpdateFader(MachineInfoPtr machineInfo);
OSErr	HitFader(MachineInfoPtr machineInfo, DialogPtr dPtr, short itemHit, short itemOffset);


// Utilities
Handle	BestNewHandle(Size theSize);
RgnHandle	BestNewRgn();
short Rnd(long max);
void PlaceRectOnScreen(
	MachineInfoPtr machineInfo,	// give info about the machine here
	short width,				// width of rect, can be 0
	short height,				// height of rect, can be 0
	Rect * placedRect,			// Placed rect is returned here
	Rect * margins,				// margins around screen, can be nil
	short * whichScreen);		// screen index returned here, can be nil

// Utilities that utilize callbacks into DarkSide

// Calls to read and write additional data to/from the preferences file.  
// WritePreferencesHandle disposes of the handle passed in
// The handle returned by ReadPreferencesHandle should be disposed of by the caller
OSErr	WritePreferencesHandle(MachineInfoPtr machineInfo, Handle h, ResType theType);
OSErr	ReadPreferencesHandle(MachineInfoPtr machineInfo, Handle *h, ResType theType);

// play a 'snd ' with the specified resource ID through the sound channel created
// by the 'Chnl' resource (if present).  Does nothing if a 'Chnl' is not present, or
// if the user set the volume to zero
OSErr	PlayResourceSnd(MachineInfoPtr machineInfo, short theID, Boolean async);


/* ------------------------------------------------------------------------------------	*/
/*	Useful macros for your fader							 							*/
/* ------------------------------------------------------------------------------------	*/
#define TopLeft(aRect)	(* (Point *) &(aRect).top)
#define BotRight(aRect)	(* (Point *) &(aRect).bottom)
#define RectWidth(r) (short)((r)->right - (r)->left)
#define RectHeight(r) (short)((r)->bottom - (r)->top)

#endif __FADER__
