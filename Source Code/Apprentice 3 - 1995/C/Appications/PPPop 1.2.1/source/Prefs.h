/* Prefs.h
   PPPop 1.2  15 May 1995
*/

#pragma once

#include "PPPop.h"
#include "Timer.h"
#include <Folders.h>
#include <Script.h>
#include <Aliases.h>

void	DoReadPrefs(void);
void	DoSavePrefs(void);
short 	DoCreatePrefsFile (FSSpec *spec);
OSErr 	DoCopyResource(ResType rType, short rID, short source, short dest);


#define	defWindTop		40
#define	defWindLeft		40
#define defTWindTop		80
#define defTWindLeft	40

#define	rFileNames	130		// file name list
#define	sPrefsFile	3		// index for file name

#define kPrefsType				'PRFN'		//	Preferences resource
#define rPrefsID 				128
#define versNum					0x0120		// Version 1.2.0
#define kCreator				'pppO'
#define kPrefsFile				'pref'
#define	rAliasID				128


typedef struct PrefsRec {
	short	version;		// PPPop version #
	short	top;			// window's top
	short	left;			// window's left
	short	willHardClose;	// 1 = hardclose, 0 = softclose
	short	sound;			// 1 = sound on, 0 = sound off
	short	finder;			// 1 = return to finder, 0 = stay in PPPop
	long	totalTime;		// cumulative time
	short	timer;			// 1 = show timer, 0 = hidden
	short	showCurrent;	// 1 = show session time, 0 = show cumulative time
	short	ttop;			// timer window top
	short	tleft;			// timer window left
	short	autoPosition;	// 1 = autoposition timer window
	long	resetTime;		// GetDateTime of last cum reset
	short	resetWhen;		// When to autoreset
	short	resetDay;		// If reset monthly, which day
	short	spare2;
	long	spare3;
} PrefsRec, *PrefsPtr, **PrefsHnd;


