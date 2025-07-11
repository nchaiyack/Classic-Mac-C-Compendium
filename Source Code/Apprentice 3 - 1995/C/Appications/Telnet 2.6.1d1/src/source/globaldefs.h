/*
*	globaldefs.h
*	Defines we only want to define once...
*
*****************************************************************************
*       NCSA Telnet for the Macintosh										*
*																			*
*		National Center for Supercomputing Applications						*
*		152 Computing Applications Building									*
*		605 E. Springfield Ave.												*
*		Champaign, IL  61820												*
*																			*
*		Copyright (c) 1992, Board of Trustees of the University of Illinois	*
*																			*
*****************************************************************************
*  Revisions:
*  7/92		Telnet 2.6:	Initial version.  Jim Browne
*/

/*	Operation Governing Defines: */
#define	MaxSess		20		/* The maximum number of sessions */
#define	MaxGraph	30		/* Maximum number of graphics */
#define NPORTS		30
#define MAXVG		20		/* maximum number of VG windows */
#define NEVENTS		50

/*	Possible values for windowKind */
#define	WIN_MODELESS	userKind+1		// Modeless windows
#define	WIN_CONSOLE		userKind+2		// Console window
#define	WIN_LOG			userKind+3		// FTP log window
#define	WIN_CNXN		userKind+4		// Connections
#define	WIN_TEK			userKind+5		// TEK windows
#define	WIN_ICRG		userKind+6		// Interactive Color Raster Graphics windows

/*	Emulation type defines */
#define VTEKTYPE	1
#define TEKTYPE		2
#define DUMBTYPE	3
#define RASTYPE		4

/*	Font Height and width constants */
#define DefFONT		4
#define DefSIZE		9

/*	ASCII character set defines */
#define ESC			0x1b	/* the escape character */
#define	BS			0x08	/* the backspace character */
#define DEL			0x7f	/* the delete character */
#define CR			0x0d	/* the carriage return character */
#define LF			0x0a	/* the line feed character */


/* Now for some event stuff */
#define	BScode		51		/* This is the Keycode for Backspace */
#define KPlowest	65		/* This is the last code before the KP */
#define switchEvt	 1		/* Switching event (suspend/resume )  for app4evt */

/* These next three keep track of the screen windows */
#define NO_WINDOW	0
#define DEC_WINDOW	1
#define TEK_WINDOW	2

#define NUMCURS		7
#define gincurs		0
#define xfercurs	1
#define normcurs	2
#define textcurs	3
#define poscurs		4
#define watchcurs	5
#define graphcurs	6

#define kInFront	(WindowPtr) -1

/* ===================================================================================*/
struct TelInfoRec {
short 
	setdir,
	SettingsFile,	/* The Resource Reference Number of the Settings file */
	ApplicationFile,	// The Resource Reference Number of the Application file
	MacBinary,		// Alternate instance that can allow us to change MB state w/o
					//	changing the actual preference value
	ScrlLock,		/* Are we in network suspend mode */
	ftplogon,		/* Is ftp log visible (default no) */
	done,			/* Are we done yet 0=not */
	numwindows,		/* Number of active window structures (note: not windows ) */
	ginon,			/* True if we are in GIN mode */
	xferon,			/* Is a transfer in progress */
	graphs,			/* How many detached graphs do we have */
	debug,			/* Flag for debugging 1=active */
	myfronttype,	/* what kinda window is the front one */
	suspended;		/* Are we in the air??? */
	
short
	CONFstate,		/* ...for parsing config files */
	CONFactive,
	position,		/* the next three are state variables for config.tel parsing */
	inquote,
	lineno;   		/* state vars */	

long
	oldgraph[MaxGraph],	/* Storage for old Graph numbers */
	myfrontvs,		/* what is the VS # of the window */
	blinktime;
			
Rect
	screenRect,		/* The whole screen */
	dragRect;		/* Where it is legal to drag ourselves */

WindowPeek
	myfrontwindow;	/* window pointer for changing the pointer */

RgnHandle
	myfrontRgn;		/* Handle to the region of the window to scope for stuff */

Cursor
	*lastCursor;

FSSpec
	ApFolder, SysFolder, PrefFolder;

short	
	DefaultDirVRefNum,
	FTPClientVRefNum; 

long
	DefaultDirDirID,
	FTPClientDirID;
};

typedef struct TelInfoRec TelInfoRec;

#ifndef GLOBALMASTER
extern TelInfoRec *TelInfo; 		/* all the globals related to Telnet internals */
#endif