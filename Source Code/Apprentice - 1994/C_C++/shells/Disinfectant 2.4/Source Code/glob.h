/*______________________________________________________________________

	glob.h - Global Declarations for Sample.
	
	Copyright � 1988, 1989, 1990, Northwestern University.

	This header file defines the constants, data types, and global
	variables shared by the private modules in Sample.

	Note that the reusable modules are not permitted to include the
	header file, but the virus detection and repair modules are permitted
	to include it.	
_____________________________________________________________________*/


#ifndef __glob__
#define __glob__

#ifndef __rpp__
#include "rpp.h"
#endif

#ifndef __rez__
#include "rez.h"
#endif


/*______________________________________________________________________

	Constant Definitions. 
_____________________________________________________________________*/



/* Other constants. */

#define  invertInterval   	15			/* tick interval between inverts of
													small floppy icon during scan of
													floppy sequence */
#define	zoomSlop				3			/* pixels to leave at top and bottom
													of zoomed windows */
#define	staggerInitialOffset	3		/* pixel offset for first staggered window */
#define	staggerOffset		20			/* pixel offset for subsequent staggered
													windows */
#define	dragSlop				4			/* slop for DragWindow bounds rect */
#define	minMainSize			295		/* min height of main window in pixels */
#define	minHelpSize			100		/* min height of Help window in pixels */
#define	beepDuration		10			/* sysbeep duration in ticks */

/* Tags for help system */

#define	tagFirstButton	300		/* first button - the other buttons
												are assigned tags 301 through 309 */
#define	tagUpperRight	320		/* upper right corner of main window */
#define	tagReport		321		/* report rectangle */
#define	tagMainWind		500		/* main window */
#define	tagHelpWind		501		/* help window */
#define	tagHelpTcon		603		/* help window table of contents */
#define	tagPrefBeep		330		/* prefs beep option */
#define	tagPrefStation	331		/* prefs scanning station option */
#define	tagPrefSave		332		/* prefs saved text file options */
#define	tagPrefNotif	333		/* prefs notification options */
#define	tagAbouWind		503		/* about window */
#define	tagCmdBase		600		/* base for menu commands */
#define	tagCmdMult		20			/* multiplier for menu commands */

/*______________________________________________________________________

	Type Declarations.
_____________________________________________________________________*/


/* Kinds of scans. */

typedef enum ScanKind {
	allScan,					/* all volumes */
	volScan,					/* selected volume */
	foldScan,				/* folder */
	fileScan,				/* file */
	autoScan,				/* sequence of floppies */
	fileFoldScan,			/* file or folder */
	sysFileScan,			/* system file */
	sysFoldScan,			/* system folder */
	volSetScan,				/* set of volumes */
	desktopScan,			/* Finder desktop files */
} ScanKind;

/* Scanning operations. */

typedef enum ScanOp {
	checkOp,					/* scan */
} ScanOp;

/* Notification options. */

typedef enum NotifOption {
	notifDiamond,			/* only diamond in apple menu */
	notifIcon,				/* also rotating icon in menu bar */
	notifAlert,				/* also alert */
} NotifOption;

/* Window kinds. */

typedef enum WindKind {
	mainWind,				/* main window */
	helpWind,				/* help window */
	prefWind,				/* prefs window */
	abouWind,				/* about window */
	daWind,					/* da window */
} WindKind;

/* The standard Edit menu commands. */

typedef enum EditCmd {
	undoCmd,
	editDummyCmd,	/* the separator line in the Edit menu */
	cutCmd,
	copyCmd,
	pasteCmd,
	clearCmd 
} EditCmd;

/* Window objects.  This struct records additional information about each
	window, plus pointers to functions to handle the common window events
	and commands.  The refCon field of each window record points to one
	of these structs. */

typedef void (*FUpdate)(void);
typedef void (*FActivate)(void);
typedef void (*FDeactivate)(void);
typedef void (*FResume)(void);
typedef void (*FSuspend)(void);
typedef void (*FClick)(Point where, short modifiers);
typedef void (*FHelp)(Point where);
typedef void (*FGrow)(short height, short width);
typedef void (*FZoom)(void);
typedef void (*FKey)(short key, short modifiers);
typedef void (*FClose)(void);
typedef void (*FDisk)(long message);
typedef Boolean (*FSave)(void);
typedef OSErr (*FPageSetup)(void);
typedef OSErr (*FPrint)(Boolean printOne);
typedef void (*FEdit)(EditCmd cmd);
typedef void (*FAdjust)(void);
typedef void (*FPeriodic)(void);
typedef Boolean (*FDialogPre)(short key);
typedef void (*FDialogPost)(short item);

typedef struct WindowObject {
	WindKind				windKind;			/* which kind of window */
	Boolean				moved;				/* true if moved or grown */
	Rect					sizeRect;			/* size rectangle */
	FUpdate				update;				/* ptr to update function */
	FActivate			activate;			/* ptr to activate function */
	FDeactivate			deactivate;			/* ptr to deactivate function */
	FResume				resume;				/* ptr to resume function */
	FSuspend				suspend;				/* ptr to suspend function */
	FClick				click;				/* ptr to click function */
	FHelp					help;					/* ptr to help function */
	FGrow					grow;					/* ptr to grow function */
	FZoom					zoom;					/* ptr to zoom function */
	FKey					key;					/* ptr to key function */
	FClose				close;				/* ptr to close function */
	FDisk					disk;					/* ptr to disk inserted function */
	FSave					save;					/* ptr to save function */
	FPageSetup			pageSetup;			/* ptr to page setup function */
	FPrint				print;				/* ptr to print function */
	FEdit					edit;					/* ptr to edit function */
	FAdjust				adjust;				/* ptr to menu adjust function */
	FPeriodic			periodic;			/* ptr to periodic function */
	FDialogPre			dialogPre;			/* ptr to dialog event preprocessor */
	FDialogPost			dialogPost;			/* ptr to dialog event postprocessor */
} WindowObject;

/* Saved window state.  See HIN 6. */

typedef struct WindState {
	Rect					userState;				/* user state rectangle */
	Boolean				zoomed;					/* true if in zoomed (standard) state */
	Boolean				moved;					/* true if moved or grown */
} WindState;

/* Preferences. */

typedef struct PrefsType {
	char					version[30];			/* version number */
	WindState			mainState;				/* main window state */
	WindState			helpState;				/* help window state */
	WindState			prefState;				/* prefs window state */
	WindState			abouState;				/* about window state */
	rpp_PrtBlock		mainPrint;				/* main window print block */
	rpp_PrtBlock		helpPrint;				/* help window print block */
	TPrint				mainPrintRec;			/* main window print record */
	TPrint				helpPrintRec;			/* help window print record */
	short					numOpenWind;			/* number of open windows */
	WindKind				openWind[5];			/* list of open windows, in front to
															back order */
	short					helpScrollPos;			/* help window scroll position */
	short					beepCount;				/* beep count pref */
	Boolean				scanningStation;		/* scanning station pref */
	ScanOp				scanningStationOp;	/* scanning station op - 
															scan or disinfect */
	OSType				repCreator;				/* report creator type pref */
	OSType				docCreator;				/* document creator type pref */
	OSType				repOtherCre;			/* report "other" creator type */
	OSType				docOtherCre;			/* doc "other" creator type */
	NotifOption			notifOption;			/* notification option pref */
} PrefsType;

/*______________________________________________________________________

	Global Variables.
_____________________________________________________________________*/


/* The following variables are set during initialization, and never
	changed thereafter. */
	
extern ControlHandle	Controls[numControls];	/* main window control handles */
extern Handle			Report;					/* handle to main window report record */
extern Rect				DragRect;				/* drag rectangle */
extern Boolean			SysHasShutDown;		/* true if shutdown trap exists */
extern long				LongSleep;				/* long sleep time */
extern Rect				RectList[numRects];	/* rectangle list */
extern CursHandle		Watch;					/* handle to watch cursor */
extern CursHandle		HelpCurs;				/* handle to help cursor */
extern CursHandle		IBeamCurs;				/* handle to ibeam cursor */
extern short			SysRefNum;				/* system file ref num */
extern short			SysVol;					/* system vol ref num */
extern long				SysDirID;				/* blessed folder dir id */
extern short			DfectRefNum;			/* Sample's file ref num */
extern short			DfectVol;				/* Sample's vol ref num */
extern Boolean			OldRom;					/* true if 64K rom */
extern Boolean			Initialized;			/* true when initialization complete */

/*	The following variables are changed after intialization. */
													
extern PrefsType		Prefs;					/* preferences */
extern Boolean			Done;						/* true when time to quit */
extern Boolean			MenuPick;				/* true if command was via menu pick */
extern Boolean			Scanning;				/* true while scan in progress */
extern Boolean			FloppyWait;				/* true while waiting for floppy insert */
extern Boolean			Canceled;				/* true if scan canceled */
extern Boolean			HelpMode;				/* true if help mode */
extern Boolean			InForeground;			/* true if running in foreground */																
extern Boolean			NoMemFullAlert;		/* true to tell grow zone proc to 
															return 0 on mem full instead of 
															posting mem full alert */
extern ScanKind		CurScanKind;			/* kind of scan */
extern long				NumScanned;				/* number of files scanned counter */
extern long				NumInfected;			/* number of files infected counter */
extern long				NumErrors;				/* number of errors counter */
extern Boolean			Notified;				/* true if notification posted */
extern NMRec			NotifRec;				/* Notification Manager record */
extern Str255			NotifString;			/* notification string */
extern Boolean			VolIsLocal;				/* true if vol being scanned is
															local vol (for Zig) */

extern long				TotFiles;			/* total number of files scanned */
extern long				TotErrors;			/* total number of errors */
extern long				TotInfected;		/* total number of infected files */
extern long				TotStillInf;		/* total number of infected files
														not repaired */
extern long				TotNoAccess;		/* total number of folders with 
														insufficient access rights */											

#endif