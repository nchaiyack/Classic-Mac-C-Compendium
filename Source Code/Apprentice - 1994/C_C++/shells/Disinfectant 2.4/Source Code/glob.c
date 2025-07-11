/*______________________________________________________________________

	glob.c - Sample Globals.
	
	Copyright � 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.

	This module declares the global variables used by the private
	modules in Sample.
_____________________________________________________________________*/


#pragma load "precompile"
#include "rez.h"
#include "glob.h"

/*______________________________________________________________________

	Global Variables.
_____________________________________________________________________*/


/* The following variables are set during initialization, and never
	changed thereafter. */

ControlHandle	Controls[numControls];	/* main window control handles */
Handle			Report;					/* handle to main window report record */
Rect				DragRect;				/* drag rectangle */
Boolean			SysHasShutDown;		/* true if shutdown trap exists */
long				LongSleep;				/* long sleep time */
Rect				RectList[numRects];	/* rectangle list */
CursHandle		Watch;					/* handle to watch cursor */
CursHandle		HelpCurs;				/* handle to help cursor */
CursHandle		IBeamCurs;				/* handle to ibeam cursor */
short				SysRefNum;				/* system file ref num */
short				SysVol;					/* system vol ref num */
long				SysDirID;				/* blessed folder dir id */
short				DfectRefNum;			/* Sample's file ref num */
short				DfectVol;				/* Sample's vol ref num */
Boolean			OldRom;					/* true if 64K rom */
Boolean			Initialized=false;	/* true when initialization complete */

/*	The following variables are changed after intialization. */

PrefsType		Prefs;					/* preferences */														
Boolean			Done = false;			/* true when time to quit */
Boolean			MenuPick;				/* true if command was via menu pick */
Boolean			Scanning = false;		/* true while scan in progress */
Boolean			FloppyWait = false;	/* true while waiting for floppy insert */
Boolean			Canceled;				/* true if scan canceled */
Boolean			HelpMode = false;		/* true if help mode */
Boolean			InForeground = true;	/* true if running in foreground */																
Boolean			NoMemFullAlert = false;	/* true to tell grow zone proc to return 0
														on mem full instead of posting mem full
														alert */
ScanKind			CurScanKind;			/* kind of scan */
long				NumScanned = 0;		/* number of files scanned counter */
long				NumInfected = 0;		/* number of files infected counter */
long				NumErrors = 0;			/* number of errors counter */
Boolean			Notified = false;		/* true if notification posted */
NMRec				NotifRec;				/* Notification Manager record */
Str255			NotifString;			/* notification string */
Boolean			VolIsLocal;				/* true if vol being scanned is
													local vol (for Zig) */
long				TotFiles;				/* total number of files scanned */
long				TotErrors;				/* total number of errors */
long				TotInfected;			/* total number of infected files */
long				TotNoAccess;			/* total number of folders with 
													insufficient access rights */											
