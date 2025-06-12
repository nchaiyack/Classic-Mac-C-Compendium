/*==============================================================================
Project:	POV-Ray

Version:	2.2

File:	Pov.c

Description:
	This file contains the main program and most other Macintosh-specific
	routines for the Persistence Of Vision raytracer (POV-Ray.)
------------------------------------------------------------------------------
Authors:
	Thomas Okken, David Lichtman, Glenn Sugden
	Jim Nitchals, David Harr, Eduard [esp] Schwan
------------------------------------------------------------------------------
	from Persistence of Vision Raytracer
	Copyright 1993 Persistence of Vision Team
------------------------------------------------------------------------------
	NOTICE: This source code file is provided so that users may experiment
	with enhancements to POV-Ray and to port the software to platforms other 
	than those supported by the POV-Ray Team.  There are strict rules under
	which you are permitted to use this file.  The rules are in the file
	named POVLEGAL.DOC which should be distributed with this file. If 
	POVLEGAL.DOC is not available or for more info please contact the POV-Ray
	Team Coordinator by leaving a message in CompuServe's Graphics Developer's
	Forum.  The latest version of POV-Ray may be found there as well.

	This program is based on the popular DKB raytracer version 2.12.
	DKBTrace was originally written by David K. Buck.
	DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
------------------------------------------------------------------------------
More Info:
	This Macintosh version of POV-Ray was created and compiled by Jim Nitchals
	(Think 5.0) and Eduard Schwan (MPW 3.2), based (loosely) on the original
	port by Thomas Okken and David Lichtman, with some help from Glenn Sugden.

	For bug reports regarding the Macintosh version, you should contact:
	Eduard [esp] Schwan
		CompuServe: 71513,2161
		Internet: jl.tech@applelink.apple.com
		AppleLink: JL.Tech
	Jim Nitchals
		Compuserve: 73117,3020
		America Online: JIMN8
		Internet: jimn8@aol.com -or- jimn8@applelink.apple.com
		AppleLink: JIMN8
------------------------------------------------------------------------------
Change History:
	920815	[jln]	version 1.0 Mac released.
	920908	[esp]	version 1.1 alpha Mac
	921228	[esp]	version 1.1 beta Mac
					¥ Refuse to open 2nd file if 1st is rendering
					¥ Fix grow bounds bug on large screens
					¥ Single file dropped during batch render displayed error.
					¥ Fixed logic for various drag-n-drop cases.
					¥ Added dialog & code to catch non-text file drops.
					¥ Made GarbageCollection dialog not appear if nothing to do.
					¥ Fixed "auto-jump to errant line #" being off by one line.
					¥ Fixed bug in Open: if open failed, left New & Open items dimmed
					¥ Removed annoying "Finished Rendering" dialog box
					¥ Display true start/end time in status window for multi-renders
					¥ Status and Source windows have zoom boxes
					¥ Fixed Auto-shutdown (it wasn't)
					¥ Fixed Custom Palette to restore properly
	930104	[esp]	1.13 sync, POVMac 1.1b19
					¥ Changed Splash Screen and about box to reflect the
						#defines DISTRIBUTION_MESSAGE & POV_RAY_VERSION
					¥ Added code in event loop to handle Disk Insert events!
					¥ Changed GetBestSFPos fn name to more generic GetBestDialogPos
					¥ Added one-liner comments above each function
	930113	[esp]	1.1b20 release, looks pretty stable to me
	930115  [jln]   ¥ Fixed the good 'ol highlight bug in goto_line finally
					¥ Fixed bug where user entered a line # too high for goto_line
					resulted in cursor being put at the TOP of the file.
					¥ Fixed bug that caused Undo Typing to undo more than it
					should have (typing, cursor movement, typing counted as
					a single gCanUndo event.  Now it's two events for the typing.)
					¥ Fixed bug in template insertion (didn't support Undo)
					¥ Changed MultiFinder friendliness code (Setting 1 is now
					even friendlier; Settings 7-15 are nastier.)
	930513	[esp]	1.64b20 release
					Moved some code to other C files (Animate,ImageWindow)
	930614	[esp]	version 1.7b6 released
	930620	[esp]	version 1.7b7 released
	930709	[esp]	version 1.75b1 released
	930725	[esp]	version 1.75b3 released (cancel parse bug fixed)
	930903	[esp]	version 1.91b2 released (VIB bug fixed)
	930911	[esp]	version 1.91b4 released (include path uses SFGetFile now)
	930918	[esp]	version 1.92b1 released (AppFriendly works better, ImageWindPos restored OK)
	930929	[esp]	version 1.94b2 released (removed QuickTime "No Image Comp" warning)
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
	931005	[esp]	version 2.01b fixed cmd-O bug (every other file open fails)
	931005	[esp]	version 2.01b undo & redo scroll window to where the action is
	931018	[esp]	version 2.01b Changed RenderOpts AutoBound from checkbox to popup,
					display credits at startup, not 1st render, fixed include prefs logic
	931119	[djh]	2.0.1 source conditionally compiles for PPC machines, keyword __powerc
	940204	[esp]	2.2 prepare for release - bound slabs default OFF
==============================================================================*/


/*==== POV-Ray std headers ====*/
#include "PovMac.h"
#include "povproto.h"


/*==== Standard C headers ====*/
#include <stdlib.h>
#include <setjmp.h>


/*==== Macintosh-specific headers ====*/
// NOTE: _H_MacHeaders_ is defined by Think C if it is using
// precompiled headers.  This is only for compilation speed improvement.
#if !defined(_H_MacHeaders_)
#include <Types.h>
#include <Controls.h>
#include <Desk.h>
#include <Dialogs.h>
#include <Files.h>
#include <Memory.h>
#include <Menus.h>
#include <OSEvents.h>
#include <OSUtils.h>
#include <Packages.h>
#include <pictutil.h>
#include <QuickDraw.h>
#include <Resources.h>
#include <Windows.h>
#include <scrap.h>
#include <shutdown.h>
#include <sound.h>
#include <AppleEvents.h>
#include <DiskInit.h>
#include <GestaltEqu.h>
#include <Folders.h>
#include <errors.h>			/* dupFNErr, etc */
#include <fonts.h>
#include <segload.h>		/* UnloadSeg */
#include <eppc.h>			/* kHighLevelEvent */
#include <traps.h>			/* _Unimplemented */
#include <string.h>			/* strcpy/cat */
#include <toolutils.h>		/* BitTst, etc */
#endif // _H_MacHeaders_

#if !defined (THINK_C)
#include <strings.h>		/* p2cstr */
#endif // THINK_C

// Debug with profiler
#if defined(THINK_C) && defined(NEEDS_PROF)
#include <profile.h>
#endif


/*==== POV-Ray text editor header ====*/
#include "TextEditor.h"


/*==== POV Mac Library routines =====*/
#include "POVLib.h"


/*==== Animation prompt ====*/
#include "Animate.h"


/*==== Compress PICT routines ====*/
#include "SaveCmpPict.h"


/*==== File queueing routines ====*/
#include "FileQueue.h"


/*==== Image Window routines ====*/
#include "ImageWindow.h"


/*==== Progress Bar dialog routines ====*/
#include "ProgressDialog.h"


/*==== Template submenu routines ====*/
#include "TemplateMenu.h"


/*==== General definitions ====*/

/* The Finder Events Suite - defined in the not-yet-existing <AERegistry.h> */
#define kAEFinderEvents			'FNDR'

// preview picture rsrc id
#define	kDemoPICTrsrcID			131

#if defined(applec)
/* entry point to %a5init segment, for disposal after init */
extern void _DataInit();
#endif // applec


#if defined(__powerc)
// PowerPC needs its own QuickDraw global rec
QDGlobals qd;

// forward declarations for routine descriptors
extern pascal void DrawPopupMenu(WindowPtr dlgWindPtr, short dinum);
pascal void dim_dlg_edit_text(WindowPtr dlgWindPtr, short dinum);
pascal void showProgress_UProc(WindowPtr dlgWindPtr, short dinum);
pascal OSErr HandleAEOapp(AEDescList *aevt, AEDescList *reply, long refCon);
pascal OSErr HandleAEOdoc(AEDescList *aevt, AEDescList *reply, long refCon);
pascal OSErr HandleAEQuit(AEDescList *aevt, AEDescList *reply, long refCon);
pascal void MyPutPicProc( char *dataPtr, short byteCount);
pascal void p2wi_VScrollActionProc(ControlHandle control, short part);
pascal void p2wi_HScrollActionProc(ControlHandle control, short part);
pascal void ScrollProc(ControlHandle theControl, short theCode);
#endif // __powerc


/*==== Global variables (external scope) ====*/

p2w_WindowPtr_t	gp2wWindow = NULL;			// the Status (text) window

Boolean				gHasSys70 = false;			// Can we do System 7 stuff?
Boolean				gHasFSSpecs = false; 		// Can use FSSpec calls?
Boolean				gHasAppleEvents = false;	// Apple Events available for doing shutdown?
Boolean				gHas32BitQD = false; 		// is 32 bit Quickdraw available for depth & dithering
Boolean				gHasPictUtils = false;		// is Picture Utils (extract best palette) avail?
Boolean				gHasQuickTime = false;		 // Is QuickTime available?
Boolean				gHasImageCompressionMgr = false; // Is image compression available?
Boolean				gHasPopups = false;			// are popup menus avail?
long				gQTVersion;					// QuicktTime version from Gestalt

Boolean				gInBackground = false;		// is the program currently switched out
Boolean				gDoingRender;				// for determining the menu states
Cursor				gEditCursor;
Cursor				gWaitCursor;
Rect				gDragBounds;				// window dragging boundary

app_prefs_hdl_t		gAppPrefs_h = NULL;			// App prefs from prefs file
file_prefs_hdl_t	gFilePrefs_h = NULL;		// each file has its own prefs
file_prefs_hdl_t	gDefltFilePrefs_h = NULL;	// default file prefs from prefs file
file_prefs_hdl_t	gPrefs2Use_h = NULL;		// points to file or default prefs

Boolean				gCanUndo = false;			// can we undo this operation?

/*==== Global variables (local scope) ====*/

static Boolean		gAutoShutdown = false;		// flag for auto-shutdown on completion
static Boolean		gAutoSave = false;			// flag for auto-save image on completion
static Boolean		gBeginRendering = false;	// True when user pick Render from menu
static Boolean		gUseAppDefaultPrefs = false; // True if overriding file prefs

// Animation-related
static Boolean		gInAnimationLoop = false;	// True when rendering an animation file
static AnimRec_t	gAnimRec;					// current animation frame/clock info

// argv/argc variables for passing to POV-Ray engine
#define	ARGV_Size	64
#define	ARGSTR_Size	1024

static int			ARGC;
static char			**ARGV, *argstr, *argptr;


static StringPtr	gDistMessage;				// concatenated from FRAME.H strings
static jmp_buf		gSetJmpEnv;					// exit state

// resource file reference numbers
static short		AppRefNum = kRsrcFileClosed,
					AppPrefsRefNum = kRsrcFileClosed,
					SrcResRefNum = kRsrcFileClosed;

static char			gTargaFname[64];			// name of Targa output file

static Boolean		gDoingBatchODOCs = false; 	// in the middle of ODOC, postpone other odocs

static long			gPrevTickCount;				// Time of last WNE time slice
static long			gCpuHogginess;				// how long to stay here before relinquishing
static long			gWNEReleaseTicks;			// how long to yield to other apps

static MenuHandle	myMenus[num_of_menus];
static MenuHandle	mySubMenus[num_of_submenus];
static EventRecord	gTheEvent;

static Boolean		gQuit = false;
static short		gPaused = 0;

static DialogPtr	gSplashScreen = NULL;
static long			gSplashStartTicks = 0;

static short		gRenderedOK = false;
static long			TargetTicks;


// This array mirrors the "Preset Image Sizes" popup menu (#302)
// I could GetItem & parse the numbers out of the menu, more versatile...
#define SCRN_SIZE_MENU_MAX	19
#define kDfltScrnSize		5		// default screen size
static short	scrnSizeMenu[SCRN_SIZE_MENU_MAX][2] =
{
//	x/w		y/h
	8,		8,		// [0]
	16,		16,		// [1]
	32,		32,		// [2]
	64,		64,		// [3]
	80,		80,		// [4]
	100,	100,	// [5]
	128,	128,	// :::
	160,	120,
	320,	200,
	320,	240,
	512,	342,
	512,	384,
	640,	400,
	640,	480,
	640,	870,
	832,	624,
	1152,	870,
	1024,	1024,
	2048,	2048	// [SCRN_SIZE_MENU_MAX-1]
};

#if defined(__powerc)
// routine descriptor globals
RoutineDescriptor gModalFilterRD = BUILD_ROUTINE_DESCRIPTOR(uppModalFilterProcInfo, ModalFilter);
RoutineDescriptor gOutlineButtonRD = BUILD_ROUTINE_DESCRIPTOR(uppUserItemProcInfo, outlineDefaultButton);
RoutineDescriptor gPopupMenuRD = BUILD_ROUTINE_DESCRIPTOR(uppUserItemProcInfo, DrawPopupMenu);
RoutineDescriptor gDimDlogTextRD = BUILD_ROUTINE_DESCRIPTOR(uppUserItemProcInfo, dim_dlg_edit_text);
RoutineDescriptor gShowProgressRD = BUILD_ROUTINE_DESCRIPTOR(uppUserItemProcInfo, showProgress_UProc);
RoutineDescriptor gAEOAppRD = BUILD_ROUTINE_DESCRIPTOR(uppEventHandlerProcInfo, HandleAEOapp);
RoutineDescriptor gAEODocRD = BUILD_ROUTINE_DESCRIPTOR(uppEventHandlerProcInfo, HandleAEOdoc);
RoutineDescriptor gAEQuitRD = BUILD_ROUTINE_DESCRIPTOR(uppEventHandlerProcInfo, HandleAEQuit);
RoutineDescriptor gVScrollRD = BUILD_ROUTINE_DESCRIPTOR(uppControlActionProcInfo, p2wi_VScrollActionProc);
RoutineDescriptor gHScrollRD = BUILD_ROUTINE_DESCRIPTOR(uppControlActionProcInfo, p2wi_HScrollActionProc);
RoutineDescriptor gScrollRD = BUILD_ROUTINE_DESCRIPTOR(uppControlActionProcInfo, ScrollProc);
RoutineDescriptor gPutPICTRD = BUILD_ROUTINE_DESCRIPTOR(uppQDPutPicProcInfo, MyPutPicProc);
#endif // __powerc


/*==== QuickTime globals ====*/

static ComponentInstance	gtheSCComponent = NULL;	


/*==== text editor undo globals ====*/

#define		max_undo		 16

typedef struct
{
	char			*buffer;
	char			**buf_handle;
	short			isDirty;
	long			byteCount;
	long			selStart;
	long			selEnd;
	char			reason[16];
} undoStruct;

static int				undoCount;
static char				undo_menu_name[32];
static char				redo_menu_name[32];
static int				undo_key_copied;
static undoStruct		undo_record[max_undo+1];
static undoStruct		redo_record[max_undo+1];



// ---------------------------------------------------------------------
// This is a user item proc for drawing default dialog button outlines
pascal void outlineDefaultButton(DialogPtr theDialog, short theItem)
{
#pragma unused (theItem)
	PenState	SavePen;
	short		itemType;
	Handle		itemHandle;
	Rect		dispRect;

	GetPenState(&SavePen);

	/* use 'ok' (#1) item's rectangle */
	GetDItem(theDialog, ok, &itemType, &itemHandle, &dispRect);
	SetPort(theDialog);

	PenSize(3, 3);
	InsetRect(&dispRect, -4, -4);
	FrameRoundRect(&dispRect, 16, 16);

	SetPenState(&SavePen);
} // outlineDefaultButton


// ---------------------------------------------------------------------
// Sets dialog #3 item's display proc to draw outline around item #1
void SetupDefaultButton(DialogPtr theDialog)
{
	short	itemtype;
	Rect	itemrect;
	Handle	tempHandle;

	/* Set up User item (always #3) to display a border around OK button (#1) */
	GetDItem(theDialog, kDefaultItem, &itemtype, &tempHandle, &itemrect);
#if defined(__powerc)
	SetDItem(theDialog, kDefaultItem, itemtype, (Handle)&gOutlineButtonRD, &itemrect);
#else
	SetDItem(theDialog, kDefaultItem, itemtype, (Handle)&outlineDefaultButton, &itemrect);
#endif // __powerc
} // SetupDefaultButton


// ---------------------------------------------------------------------
// returns the ControlHandle for the item in the dialog box
ControlHandle GrabDItemHandle(DialogPtr theDialog, short theGetItem)
{
    short	itemtype;
    Rect	itemrect;
    Handle	tempHandle;
    
    GetDItem(theDialog, theGetItem, &itemtype, &tempHandle, &itemrect);
    return((ControlHandle)tempHandle);
} // GrabDItemHandle


// ---------------------------------------------------------------------
// ModalFilterProc - handles mapping the Return/Enter/Esc keys, and the
// special case of accepting Return if in certain EditText dialog fields
pascal Boolean ModalFilter(DialogPtr theDialog, EventRecord *theDialogEvent, short *theDialogItem)
{
    WindowPtr	tempGP;
    char		theKey;
    long		tilticks;
    Boolean		returnVal = false; // not yet handled

	/* remember where we parked */
	GetPort(&tempGP);
	SetPort(theDialog);

	/* Deal with KeyDowns */
    if ((theDialogEvent->what == keyDown) || (theDialogEvent->what == autoKey))
	{
		theKey = theDialogEvent->message & charCodeMask;

		/* Do filtering for <ESC> and <CR> as OK and Cancel, per Human Interface guidelines. */
		switch (theKey)
		{
			/* Make the Enter key identical to the Return key! */
			case kEnterKey:
				/* Mask out the Enter key, and OR in the Return key */
				theDialogEvent->message = (theDialogEvent->message & ~charCodeMask) | kReturnKey;
				/* now drop through and handle what the Return key would do */
			
			/* This filters for Return or Enter as item 1, and Esc as item 2 */
			case kReturnKey:
				*theDialogItem = ok;			/* change whatever the current item is to the OK item */
				/* now we need to invert the button */
				HiliteControl(GrabDItemHandle(theDialog, ok), inButton);
				Delay(8, &tilticks);			/* wait about 8 ticks so they can see it */
				HiliteControl(GrabDItemHandle(theDialog, ok), false);
				returnVal = true;
				break;
			
			/* This filters the escape key the same as item 2 (the canx button, usually ) */
			case kEscKey:
				*theDialogItem = cancel;		/* pretend the cancel button was hit */
				HiliteControl(GrabDItemHandle(theDialog, cancel), inButton);
				Delay(8, &tilticks);			/* wait about 8 ticks so they can see it */
				HiliteControl(GrabDItemHandle(theDialog, cancel), false);
				returnVal = true;
				break;
		} // switch
    } // if keydown
    else
    {	// handle popup menus if some installed in dialog
    	if ((theDialogEvent->what == mouseDown) && PopupsExist())
    		PopupMouseDnDlgFilterProc(theDialog, theDialogEvent, theDialogItem);
    }

	SetPort(tempGP);

	return(returnVal);

} // ModalFilter


// ---------------------------------------------------------------------
// Displays a dialog. param ^0 is the error #, ^1 is the string
short displayDialog(short dlogID,
					char * s, short err,
					WindCentering_t doCentering,
					WindPositioning_t whereToShow)
{
	short		itemHit = 0;
	DialogPtr	myDialog;
	char		numstr[10];

	// make sure we're looking down the right resource chain
	UseResFile(AppRefNum);

	(void)CompactMem(FreeMem());	// force a purge, get some memory

	SetCursor(&qd.arrow); // it could still be an I-beam or watch!

	// get the dialog
	myDialog = GetNewDialog(dlogID, (Ptr)0L, (WindowPtr)-1L);

	if (!myDialog)
		SysBeep(2); // can't get dialog, at least beep
	else
	{
		// convert error string to P-style if not null
		if (s != NULL)
			c2pstr(s);

		// see if there is an error number
		numstr[0] = '\0';
		if (err != 0)
			NumToString(err, (StringPtr)numstr);

		// stick parms into dialog
		ParamText((StringPtr)numstr, (StringPtr)s, "\p", "\p");

		// put it where it should be
		PositionWindow(myDialog, doCentering, whereToShow, (WindowPtr)gp2wWindow);
		// "default" the OK button
		SetupDefaultButton(myDialog);
		// show it
		ShowWindow(myDialog);
		// prompt the user
#if defined(__powerc)
		ModalDialog((ModalFilterUPP)&gModalFilterRD, &itemHit);
#else
		ModalDialog(ModalFilter, &itemHit);
#endif // __powerc
		// all done with it
		DisposeDialog(myDialog);
	}

	return itemHit;

} // displayDialog


// ---------------------------------------------------------------------
// Find the best x,y point for a dialog, by finding which screen
// the status window is on and offsetting from there.  Used by
// dialog routines like SFGetFile and DIBadMount.
void GetBestDialogPos(Point * wherep)
{
	Rect		sfRect;
	GDHandle	thGDev;

	// find rect of screen that status window is on..
	GetGlobalWindowRect((WindowPtr)gp2wWindow, &sfRect);
	thGDev = GetClosestGDevice(&sfRect);
	sfRect = (**thGDev).gdRect;

	// ..and offset SF Dialog on that screen
	wherep->h = sfRect.left + 40;
	wherep->v = sfRect.top + 80;
} // GetBestDialogPos


// ---------------------------------------------------------------------
// Dispose and recreate an empty file list queue
static void HaltFileQ()
{
	// halt any multi-doc processes
	gDoingBatchODOCs = false;

	FileQ_d();	// Tear down ODOC File Queue
	FileQ_c();  // re-create it, empty
} // HaltFileQ


// ---------------------------------------------------------------------
/*------- coming soon.. when we get an FSSpec-friendly Editor
// System 6 AND System 7 compatible routine to create an FSSpec
static OSErr MyMakeFSSpec(short pVRefNum, long pDirID, StringPtr pFname, FSSpecPtr pTheFSFilePtr)
{
	OSErr	anError = noErr;
	if (gSystem7OrBetter)
		anError = FSMakeFSSpec(pVRefNum, pDirID, pFname, pTheFSFilePtr);
	else
	{	// fake it!
		pTheFSFilePtr->vRefNum = pVRefNum;
		pTheFSFilePtr->parID = pDirID;
		BlockMove(pFname, pTheFSFilePtr->name, 1+pFname[0]);
	}
	return anError;
} // MyMakeFSSpec
-----------*/


// ---------------------------------------------------------------------
// Send a System Seven Shutdown AppleEvent to the Finder to shut us all down
static void SendAEShutdown(void)
{
	OSErr		myErr;
	OSType		theSignature;
	AppleEvent	myAEvt, myReturnEvt;
	AEAddressDesc	myServerAddress;

	theSignature = 'MACS'; // Finder signature.. of course!?
	myErr = AECreateDesc(typeApplSignature, (Ptr)&theSignature, 4, &myServerAddress);
	if (!myErr)
		myErr = AECreateAppleEvent(kAEFinderEvents,			// Event Class
									kAEShutDown,			// Event ID
									&myServerAddress,		// Target Adr
									kAutoGenerateReturnID,	// Return aevt ID
									kAnyTransactionID,		// Trans ID
									&myAEvt);				// fill me in
	if (!myErr)
		myErr = AESend(&myAEvt,
						&myReturnEvt,
						kAENoReply,				// send mode
						kAENormalPriority,		// send priority
						kNoTimeOut,				// N timeout ticks
						NULL,					// idleProcPtr
						NULL);					// EventFilterProcPtr
	/* may as well clean up before we die :-) */
	if (!myErr)
		myErr = AEDisposeDesc(&myServerAddress);
} // SendAEShutdown



// ---------------------------------------------------------------------
// Returns the short version string in the application's version resource
static void GetAppVersionPString(short versID, Str31 versionString)
{
    VersRecHndl	versHandle;		// VersRecHndl declared in MPW's <files.h>
    
	/* Get the resource string from app, 'vers' versID (1 or 2) resource! */
	versHandle = (VersRecHndl)GetResource('vers',versID);
	if (versHandle)
	{
		HLock((Handle)versHandle);
		BlockMove((**versHandle).shortVersion, versionString, (**versHandle).shortVersion[0]+1);
		ReleaseResource((Handle)versHandle);
	}
	else
		versionString[0] = 0;
} // GetAppVersionPString



// ---------------------------------------------------------------------
// Calculate MultiFinder yield time as a function of hogginess.
// If the user is making POV-Ray extra friendly, yield more background
// ticks.  If user's making us extra hoggy, yield no background ticks.
//  gCPUHogginess  gWNEReleaseTicks
//	 1				32
//	 2				24
//	 3				24
//	 4				16
//	 5				16
//	 6				 8
//	 7				 8
//	 8				 4
//	 9				 4
//	10				 0
//	11				 0
static void CalcCpuReleaseTicks(Boolean AllowVerbose)
{
	// # of ticks to release control to other apps
	gWNEReleaseTicks = ((kMaxMultiFriendly - gCpuHogginess)>>1)<<2;
	if ( AllowVerbose && ((**gPrefs2Use_h).progress >= kProgDebug) )
		printf("-d AppPriority=%ld, ReleaseTimes=%ld\n", gCpuHogginess, gWNEReleaseTicks);
}
		

// ---------------------------------------------------------------------
// Convert current date/time into an ASCII C string for display
static char * GetDateTimeString(char *pbuff)
{
	unsigned long	dtsecs;
	char			tbuff[64];
	
	// Does anybody really know what time it is?
	GetDateTime(&dtsecs);

	// convert date to string
	IUDateString(dtsecs, shortDate, (StringPtr)tbuff);
	p2cstr((StringPtr)tbuff);
	strcpy(pbuff, tbuff);

	// put date/time separator in buffer
	strcat(pbuff, "  ");

	// convert time to string
	IUTimeString(dtsecs, true, (StringPtr)tbuff);
	p2cstr((StringPtr)tbuff);
	strcat(pbuff, tbuff);

	// "give user his filled buffer back"
	return (pbuff);
} // GetDateTimeString


// ---------------------------------------------------------------------
// Force all menus to be disabled
static void DisableMenus(void)
{
	int i;

	for (i = 1; i < num_of_menus; i++)
		DisableItem(myMenus[i], 0);

	// if System 6, disable about too.  Don't do this under 7.0,
	// otherwise OS turns OFF ALL MENUS, including help, app, & script menus!
	if (gHasSys70)
	{
		EnableItem(myMenus[apmn_ID - menu_offset], apmn_about);
	}
	else
		DisableItem(myMenus[apmn_ID - menu_offset], apmn_about);

	HiliteMenu(0);
	DrawMenuBar();
} // DisableMenus


// ---------------------------------------------------------------------
// Force all menus to be enabled
static void EnableMenus(void)
{
	int i;
	for (i = 1; i < num_of_menus; i++)
		EnableItem(myMenus[i], 0);

	// If under System 6, re-enable the about item, see above DisableMenus
	if (!gHasSys70)
	{
		EnableItem(myMenus[apmn_ID - menu_offset], apmn_about);
	}

	HiliteMenu(0);
	DrawMenuBar();
} // EnableMenus


// ---------------------------------------------------------------------
// Shut down the Mac power (System 6 or 7)
static void DoShutdownMac()
{
	if (gHasAppleEvents)
		SendAEShutdown();
	else
		ShutDwnPower(); // System 6 call
} // DoShutdownMac


// ---------------------------------------------------------------------
// Play Jim's nifty sound
static void PlayNotifySound(void)
{
	Handle the_snd_handle;
	OSErr myErr;

	the_snd_handle = GetNamedResource('snd ',"\pFileDone");
	if (the_snd_handle)
		myErr = SndPlay(NULL, the_snd_handle, TRUE);
} // PlayNotifySound


// ---------------------------------------------------------------------
// Show an ALL DONE dialog
static void ShowNotifyDialog(void)
{
	if (!gRenderedOK) // ray tracing error
		(void)displayDialog(143, NULL, 0, ewcDoCentering, eSameAsPassedWindow);
	else	// completed OK
		(void)displayDialog(142, NULL, 0, ewcDoCentering, eSameAsPassedWindow);
} // ShowNotifyDialog


// ---------------------------------------------------------------------
// Clean up malloc residue, show progress bar too
static void Collect_Garbage(void)
{
	DialogPtr	progressDialogPtr = NULL;

	// Do we need to do anything?
	if (POV_need_to_reclaim())
		{
		DisableMenus();
		SetCursor(&gWaitCursor);

		// create the please wait dialog
		progressDialogPtr = GetNewProgressDialog(151, 3); // dlog ID, progress item ID
		if (progressDialogPtr)
		{
			PositionWindow(progressDialogPtr, ewcDoCentering, eSameAsPassedWindow, (WindowPtr)gp2wWindow);
			ShowWindow(progressDialogPtr);
			SelectWindow(progressDialogPtr);
			DrawDialog(progressDialogPtr);
			TargetTicks = TICKS + 1*60; // stay up at least 1 second
			// make sure dialog is drawn
			Cooperate(true);
		}

		// do the reclamation
		POV_reclaim((**gPrefs2Use_h).progress >= kProgMinimal, progressDialogPtr);

		// tear down the dialog box when done
		if (progressDialogPtr)
		{
			// Avoid any temporary flashes, make sure it's up for a
			// little bit of time, no matter what..
			while (TICKS < TargetTicks)
				{
				};
			disposeProgressDialog(progressDialogPtr);
		}

		// restore menus and cursor
		EnableMenus();
		SetCursor(&qd.arrow);
	}
} // Collect_Garbage


// ---------------------------------------------------------------------
// Change the type/creator of the Targa output file
static void ChangeTargaType(void)
{
	OSErr i;
	FInfo myFileInfo;

	c2pstr(gTargaFname);

	// Get file info, so we can change it
	i = GetFInfo ((StringPtr)gTargaFname, gSrcWind_VRefNum, &myFileInfo);
	if (i==0)
	{
		myFileInfo.fdType = 'TPIC';
		myFileInfo.fdCreator = (**gPrefs2Use_h).targaFileCreator;
		i = SetFInfo ((StringPtr)gTargaFname, gSrcWind_VRefNum, &myFileInfo);
	}
	p2cstr((StringPtr)gTargaFname);
} // ChangeTargaType



// ---------------------------------------------------------------------
// Set up to call the renderer, call it, and clean up after it returns
static void call_main(int argc, char **argv)
{
	char	timeBuffer[128];

	dispose_virtual(); /* Disposes all old Pixmaps & deletes virtual file */

	gPrevTickCount = TICKS;
	gRenderedOK = true;	/* presume that rendering will finish OK */

	// clean up any old parse memory from previous run
	Collect_Garbage();	

	Max_Trace_Level = 5.0; // render.c
	token_count = 0; // tokenize.c
	line_count = 10; // tokenize.c

	p2cstr(gSrcWind_FileName);
	printf("-- [Start]  File=%s,  Time=%s\n", gSrcWind_FileName, GetDateTimeString(timeBuffer));
	c2pstr((char*)gSrcWind_FileName);

	// Track our memory inside POV core code
	POV_enable_memtracking(true);

	// Return here if 'exit()' called
	if (setjmp(gSetJmpEnv) == 0)
	{
#if defined(THINK_C) && defined(NEEDS_PROF)
		// Turn on Think C's profiler
		// InitProfile(maxSyms,maxCallDepth)
		InitProfile(300,300);
		freopen("POV.ProfileRpt","w",stdout);
		_profile = 1;
//		_trace = 1;
#endif
		alt_main(argc, argv);

#if defined(THINK_C) && defined(NEEDS_PROF)
		_profile = 0;
		_trace = 0;
		// Dump Think C's profiler report
		DumpProfile();
		fclose(stdout);
		gQuit = true; // no reason to stick around.
#endif
	}

	// all done tracking, turn it off
	POV_enable_memtracking(false);

	if (gRenderedOK && (**gPrefs2Use_h).createTargaOutfile)
		ChangeTargaType();	/* change type and creator of the Targa output file */
	InvalRect_ImageWindow(true);

	p2cstr(gSrcWind_FileName);
	printf("-- [Finish]  File=%s,  Time=%s\n", gSrcWind_FileName, GetDateTimeString(timeBuffer));
	c2pstr((char*)gSrcWind_FileName);

	gDoingRender = false;
} // call_main


// ---------------------------------------------------------------------
// Allocate memory for the fake argv parameter buffer
static void InitArgs(void)
{
	ARGV = (char**)malloc(ARGV_Size*sizeof(char*));
	argstr = (char*)malloc(ARGSTR_Size);
	argptr = NULL;
	ARGC = 0;
} // InitArgs


// ---------------------------------------------------------------------
// Reset argv/argc to begin adding args
static void ResetArgs(void)
{
	ARGC = 0;
	argptr = argstr;
}


// ---------------------------------------------------------------------
// Add "s" as the next parameter in the argv/argc list
static void AddArg(const char *s)
{
	if ((**gPrefs2Use_h).progress >= kProgDebug)
		printf("-d argv='%s', argc=%d, argcMax=%d\n",
				s, (int)(argptr-argstr), ARGSTR_Size);

	if ((argptr-argstr)+strlen(s) < ARGSTR_Size)
	{
		ARGV[ARGC++] = argptr;
		while (*argptr++ = *s++)
			;
	}
	else
	{
		printf("## [Error!] Argument overflow adding '%s'!  index=%d, max=%d\n",
				s, (int)(argptr-argstr), ARGSTR_Size);
	}
} // AddArg


// ---------------------------------------------------------------------
// Pin the value x between a and b, i.e.  a <= x <= b
static void PinValue(short *x, short a, short b)
{
	if (*x < a)
		*x = a;
	else
		if (*x > b)
			*x = b;
} // PinValue


// ---------------------------------------------------------------------
// Move the source window's selection to the line # passed
static void goto_line(short line_to_go)
{
	short		i;
	short		line_counter = 1,
				mySelStart = -1,
				mySelEnd = -1;
	Boolean		foundStart = false;
	short		theMaxLength = (**gSrcWind_TEH).teLength-1;
	char		*myptr = *(**gSrcWind_TEH).hText;

	/* (Line 1 is a special case, of course!) */
	if (line_to_go <= 1)
	{
		mySelStart = 0;
		foundStart = true;
	}

	/* Find start and end of line */
	for (i = 0; i < theMaxLength; i++)
		if (myptr[i] == 0x0d)
			if (!foundStart)
			{	/* still looking for start of the line */
				line_counter += 1;
				if (line_counter == line_to_go)
				{
					mySelStart = i + 1;
					foundStart = true;
				}
			}
			else
			{	/* now we found the end of the line */
				mySelEnd = i + 1;
				break;
			}

	/* Beep if we found no start of line */
	if ((mySelStart < 0) && (line_to_go > 1))
	{	/* Hmm, didn't find it! */
		mySelStart = theMaxLength;
		mySelEnd = theMaxLength;
		SysBeep(4);
	}

	/* if no end of line found, ASSUME it is last line of file  */
	if (mySelEnd < mySelStart)
		mySelEnd = theMaxLength;

	/* Set up TE with selection */
	(**gSrcWind_TEH).selStart = mySelStart;
	(**gSrcWind_TEH).selEnd = mySelEnd;

	/* show them that we're not in Kansas anymore! */
	SelectWindow(gSrcWind_Window);
	SetPort(gSrcWind_Window);
	InvalRect(&(gSrcWind_Window->portRect));
	ShowSelect();
} // goto_line



#define	kItem_LineNum	4

// ---------------------------------------------------------------------
// prompt for which line # to go to, and go to it
static void choose_goto_line(void)
{
	short		i, dummyInt, line_to_go;
	short		itemHit;
	DialogPtr	myDialog;
	Rect		displayRect;
	ControlHandle cntl[kItem_LineNum+1];
	char		s1[128];

	DisableMenus();
	
	line_to_go = 1; /* for now */
	myDialog = GetNewDialog(150, NULL, (WindowPtr) -1);
	for (i = 1; i <= kItem_LineNum; i++)
		GetDItem(myDialog, i, &dummyInt, (Handle *) &cntl[i], &displayRect);
	sprintf(s1, "%d", line_to_go); 
	SetIText((Handle) cntl[kItem_LineNum], c2pstr(s1) );
	SelIText(myDialog, kItem_LineNum, 0, 32767);
	PositionWindow(myDialog, ewcDoCentering, eSameAsPassedWindow, (WindowPtr)gp2wWindow);
	/* "default" the OK button */
	SetupDefaultButton(myDialog);
	ShowWindow(myDialog);
	do
	{
#if defined(__powerc)
		ModalDialog((ModalFilterUPP)&gModalFilterRD, &itemHit);
#else
		ModalDialog((ModalFilterProcPtr)ModalFilter, &itemHit);
#endif // __powerc
	}
	while (itemHit != ok && itemHit != cancel);
	
	if (itemHit == ok)
	{
		GetIText((Handle) cntl[kItem_LineNum], (StringPtr)s1);
		sscanf(p2cstr((StringPtr)s1) , "%hd", &line_to_go);
		PinValue(&line_to_go, 0, 32767);
	}
	DisposeDialog(myDialog);
	if (itemHit == ok)
		goto_line(line_to_go);

	EnableMenus();
} // choose_goto_line


// ---------------------------------------------------------------------
static void DoLookup()
{
	if (FrontWindow() == gSrcWind_Window)
	{
		// First, copy the selection to the clipboard
		ZeroScrap(); // clear out the clipboard
		TECopy(gSrcWind_TEH);
		TEToScrap(); // export

		// Now, switch to POV-Reference if it is there

	}
} // DoLookup


// ---------------------------------------------------------------------
static OSErr VRef2VolName(short theVRef, StringPtr theVolName)
{
	OSErr			anError;
	HVolumeParam	pbVInfo;

	pbVInfo.ioCompletion = NULL;
	theVolName[0] = '\0';
	pbVInfo.ioNamePtr = theVolName;
	pbVInfo.ioVRefNum = theVRef;
	pbVInfo.ioVolIndex = 0; // use VRefNum only
	anError = PBHGetVInfo((HParmBlkPtr)&pbVInfo, false);

	return anError;
} // VRef2VolName


// ---------------------------------------------------------------------
static OSErr VolName2VRef(StringPtr myPVolName, short * theVRefPtr)
{
	OSErr			anError = noErr;
	Boolean			foundIt = false;
	short			theDriveNum;
	short			theVRefNum;
	HParamBlockRec	aParamBlk;
	char			myCVolName[31];
	Str31			theVolName;

	// convert passed vol name from P 2 C string for compare
	BlockMove(myPVolName, myCVolName, myPVolName[0]+1);
	p2cstr((StringPtr)myCVolName);

	// loop through all mounted volumes, see if myPVolName matches one
	for (theDriveNum = 1; !foundIt && (anError==noErr); theDriveNum++)
	{
		theVolName[0] = '\0';
		aParamBlk.volumeParam.ioNamePtr = (StringPtr)&theVolName;
		aParamBlk.volumeParam.ioVolIndex = theDriveNum;
		
		anError = PBHGetVInfoSync((HParmBlkPtr)&aParamBlk);
		if (anError == noErr)
		{
			p2cstr(theVolName);
			if (strcmp((char*)theVolName, myCVolName)==0)
			{
				foundIt = true;
				theVRefNum = aParamBlk.volumeParam.ioVRefNum;
			}
		}
	}

	if (foundIt)
		*theVRefPtr = theVRefNum;
	else
		*theVRefPtr = 0;

	if (anError == nsvErr) // no such vol, just bumped end of volume list
		anError = noErr;

	return anError;
} // VolName2VRef


// ---------------------------------------------------------------------
static Boolean PromptForSearchPath(FSSpecPtr theDirFSSpecPtr)
{
	OSErr		anError;
	short		trueVRef;
	long		trueDirID;
	OSType		wdProcID;
	Point		sfWhere;
	short		numTypes;
	SFTypeList	myTypes;
	SFReply		theReply;

	// prompt for Include files
	numTypes = 1;
	myTypes[0] = 'TEXT';
	GetBestDialogPos(&sfWhere);
	SFGetFile(sfWhere, "\pFind INCLUDE Files?",
				0L, numTypes, myTypes, 0L, &theReply);

	if (!theReply.good)
		return false;
	else
	{
		// Convert dorky old Working directory into more useful Vref/DirID
		anError = GetWDInfo(theReply.vRefNum, &trueVRef, &trueDirID, (long*)&wdProcID);
		// remember dir id & vref
		theDirFSSpecPtr->vRefNum = trueVRef;
		theDirFSSpecPtr->parID = trueDirID;
		// convert transient vref to more permanent volume name
		VRef2VolName(trueVRef, theDirFSSpecPtr->name);
		return true;
	}
} // PromptForSearchPath


// ---------------------------------------------------------------------
/*
Function:	PathNameFromDirID

Parameters:
	aDirID		the directory ID to start at
	avRefNum	the volume ref # to start at
	aPathStyle	The style of pathname to format (based on OS)
	aPath		the final path string is placed in this buffer

Return Value:
	zero is returned if OK, else an error is returned

Purpose/Description:
	PathNameFromDirID takes a real vRefNum and a DirID and returns
	the full pathname that corresponds to it.  It does this by calling
	_PBGetCatInfo for the given directory and finding out its name and
	the DirID of its parent. It then performs the same operation on the
	parent, sticking its name onto the beginning of the first directory.
	This whole process is continued until we have processed everything up
	to the root directory (identified with a DirID of 2). 
*/
OSErr PathNameFromDirID(const long			aDirID,
						const short			avRefNum,
						char				* aPath)
{
	Handle		strHandle = NULL;
	OSErr		anError = noErr;
	long		currDirID = 0,
				nextDirID = 0;
	Size		theLen = 0;
	char		tempName[64];	// directory or volume name
	CInfoPBRec	aPBblock;

	/* create an empty string handle for Munger */
	strHandle = NewHandle(0);

	if (strHandle == NULL)
	{
		anError = MemError();
	}
	else
	{
		*aPath = '\0';
		aPBblock.dirInfo.ioCompletion	= NULL;
		aPBblock.dirInfo.ioNamePtr		= (StringPtr)tempName;	// temp destination buffer
		nextDirID						= aDirID;	// start at DirID passed in..

		/* walk up the directories, collecting dir names as we go, until we hit the top (volname) */
		do {
			
			aPBblock.dirInfo.ioVRefNum		= avRefNum;
			aPBblock.dirInfo.ioFDirIndex	= -1; // ignore ioNamePtr input, use ioDirID
			aPBblock.dirInfo.ioDrDirID		= nextDirID;

			anError = PBGetCatInfo(&aPBblock,false);
			currDirID	= aPBblock.dirInfo.ioDrDirID;	// remember current
			nextDirID	= aPBblock.dirInfo.ioDrParID;	// find parent for next loop

			if (anError != noErr)
				break;

			/* Insert a separator in front of the Directory Entry */
			Munger(strHandle, 0L, NULL, 0L, ":", 1L);

			/* Insert the next directory/volume name into the front of the string */
			Munger(strHandle, 0L, NULL, 0L, &(tempName[1]), (long)tempName[0]);

			/* NOTE: fsRtDirID is defined in Files.h */
		} while (currDirID != fsRtDirID);

		/* now move the string into the parameter */
		theLen = GetHandleSize(strHandle);
		if (!anError && theLen)
		{
			BlockMove(&(**strHandle), aPath, theLen);
			anError = MemError();
		}
		aPath[theLen] = '\0';	/* set C string length */
	
		/* dispose of temp storage */
		DisposeHandle(strHandle);

	} /*else NULL*/

	return(anError);

} /* PathNameFromDirID */


// ---------------------------------------------------------------------
/*
Function:	PathNameFromWD

Parameters:
	avRefNum	the volume ref # to start at (Assumes WD is set)
	aPathStyle	The style of pathname to format (based on OS)
	aPath		the final path string is placed in this buffer

Return Value:
	Describe the value returned, or void

Purpose/Description:
	PathNameFromWD takes an HFS Working Directory ID and returns
	the full pathname that corresponds to it.  It does this by calling
	_PBGetWDInfo to get the vRefNum and DirID of the real directory.
	It then calls PathNameFromDirID and returns its result.  
*/
OSErr PathNameFromWD(	const short			avRefNum,
						char				* aPath)
{
    WDPBRec	aPBblock;
	OSErr	anError	= noErr;

    /*
    /* PBGetWDInfo has a bug under A/UX 1.1.  If vRefNum is a real vRefNum
    /* and not a wdRefNum, then it returns garbage.  Since A/UX has only 1
    /* volume (in the Macintosh sense) and only 1 root directory, this can
    /* occur only when a file has been selected in the root directory (/).
    /* So we look for this and hard code the DirID and vRefNum. */
#if defined(AUX)
    if ((aPathStyle == ePathTypeAUX) && (avRefNum == -1))
	{
		anError = PathNameFromDirID(fsRtDirID, -1, aPath);
	}
	else
#endif
	{
		aPBblock.ioCompletion = NULL;
		aPBblock.ioNamePtr = NULL;
		aPBblock.ioVRefNum = avRefNum;
		aPBblock.ioWDIndex = 0;
		aPBblock.ioWDProcID = 0;
	
		/* Change the Working Directory number in vRefnum into a real vRefnum */
		/* and DirID. The real vRefnum is returned in ioVRefnum, and the real */
		/* DirID is returned in ioWDDirID. */
	
		anError = PBGetWDInfo(&aPBblock,false);
		if (anError == noErr)
			anError = PathNameFromDirID(aPBblock.ioWDDirID,
										aPBblock.ioWDVRefNum,
										aPath);
	}

	return(anError);

} /* PathNameFromWD */



// ---------------------------------------------------------------------
// Find and open the Application Prefs file
static OSErr OpenAppPrefs(void)
{
	long		theResponse;		/* For call to Gestalt */
	OSErr		errCode;
	long		prefs_dirID;
	short		prefs_volNum;
 	SysEnvRec	theWorld;

	/* default in case neither FindFolder nor SysEnvirons can help locate the Preference Folder */
	prefs_dirID = 0;
	prefs_volNum = 0;
	
	errCode = -1;
	/* Find the preferences folder */
	errCode = Gestalt(gestaltFindFolderAttr, &theResponse);
	
	if (errCode == noErr && (BitTst((Ptr)&theResponse, 31 - gestaltFindFolderPresent)))
	  errCode = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
		 &prefs_volNum, &prefs_dirID);

	if (errCode)
	{
		/* Error locating the preferences folder, so use the blessed folder */
		if ((errCode = SysEnvirons(1, &theWorld)) == noErr)
			prefs_volNum = theWorld.sysVRefNum;
		else
			/* Had a problem with SysEnvirons, try the default volume */
			prefs_volNum = 0;
		prefs_dirID = 0;
	}

	/* Create the prefs file.  If it's already created, no need to create the resource fork. */
	if ((errCode = HCreate(prefs_volNum, prefs_dirID, POV_RAY_PREFS_FNAME,
	 kAppSignature, kPrefsFileType)) == noErr)
	{
		HCreateResFile(prefs_volNum, prefs_dirID, POV_RAY_PREFS_FNAME);
		errCode = ResError();
	}

	/* Return an error code */
	if (errCode == dupFNErr) /* file's already created?  Not an error. */
		errCode = noErr;
	if (errCode == noErr)
	{
		AppPrefsRefNum = HOpenResFile(prefs_volNum, prefs_dirID, POV_RAY_PREFS_FNAME, fsRdWrPerm);
   		if (AppPrefsRefNum == kRsrcFileClosed)
   			errCode = kRsrcFileClosed;
   		if (ResError())
   			errCode = ResError();
   	}
	return errCode;
} // OpenAppPrefs



// ---------------------------------------------------------------------
// Close the Application Prefs file
static void CloseAppPrefs(void)
{
	CloseResFile (AppPrefsRefNum);
	AppPrefsRefNum = kRsrcFileClosed;
} // CloseAppPrefs



// ---------------------------------------------------------------------
// Write the prefs resource back to the app prefs file
static void WriteAppPrefs(void)
{
	OSErr				theResError = noErr;
	file_prefs_hdl_t	tmpFileHandle = NULL;
	app_prefs_hdl_t		tmpAppHandle = NULL;

	theResError = OpenAppPrefs();

	if (theResError == noErr)
	{
		UseResFile(AppPrefsRefNum);

		// as long as there are old prefs resources, delete them..
		// note that I'm getting by index, so as to get any rsrc id #s..
		do {
			// App prefs
			tmpAppHandle = (app_prefs_hdl_t)Get1IndResource(kAppPrefsRsrc, 1);
			if (tmpAppHandle != NULL)
			{
				RmveResource((Handle)tmpAppHandle);
				UpdateResFile(AppPrefsRefNum);
				DisposeHandle((Handle)tmpAppHandle);
			}
		} while (tmpAppHandle != NULL);
		do {
			// Default File prefs
			tmpFileHandle = (file_prefs_hdl_t)Get1IndResource(kFilePrefsRsrc, 1);
			if (tmpFileHandle != NULL)
			{
				RmveResource((Handle)tmpFileHandle);
				UpdateResFile(AppPrefsRefNum);
				DisposeHandle((Handle)tmpFileHandle);
			}
		} while (tmpFileHandle != NULL);
		
		/* Create a new app prefs resource to add */
		tmpAppHandle = (app_prefs_hdl_t)NewHandle(sizeof(app_prefs_rec_t));
	
		/* Fill it up and add it to file */
		if (tmpAppHandle != NULL)
		{
			**tmpAppHandle = **gAppPrefs_h;
			AddResource((Handle)tmpAppHandle, kAppPrefsRsrc, kPrefs_rsrcID, "\pPOV-Ray App Prefs");
			WriteResource((Handle)tmpAppHandle);
			theResError = ResError();
		}
	
		/* Create a new default file prefs resource to add */
		if (theResError == noErr)
			tmpFileHandle = (file_prefs_hdl_t)NewHandle(sizeof(file_prefs_rec_t));
	
		/* Fill it up and add it to file */
		if ((tmpFileHandle != NULL) && (theResError == noErr))
		{
			**tmpFileHandle = **gDefltFilePrefs_h;
			AddResource((Handle)tmpFileHandle, kFilePrefsRsrc, kPrefs_rsrcID, "\pPOV-Ray Default File Prefs");
			WriteResource((Handle)tmpFileHandle);
			theResError = ResError();
		}
		UseResFile(AppRefNum);
	
		if ((theResError) || (tmpAppHandle == NULL) || (tmpFileHandle == NULL))
		{
			// error updating prefs file
			displayDialog(144, NULL, theResError, ewcDoCentering, eMainDevice);
			gAppPrefs_h = NULL;
			exit_handler();
		}

		// All done, clean up shop
		CloseAppPrefs();

	} // if !error
} // WriteAppPrefs



// # of ticks "Updating Prefs" dialog is up
#define	kUpdPrefsTime	((long)4*60)

// ---------------------------------------------------------------------
// Open the App prefs file & get the prefs out of the file.  If the file
// is out of date or doesn't exist, create a new one and fill it with defaults.
static void GetAppPrefs(void)
{
	OSErr				theResError = noErr;
	DialogPtr			updPrefsDialogPtr;
	file_prefs_hdl_t	tmpFileHandle = NULL;
	app_prefs_hdl_t		tmpAppHandle = NULL;

	theResError = OpenAppPrefs();

	// Get saved prefs for POV_Ray prefs dialog from the app prefs file
	if (theResError == noErr)
	{
		UseResFile(AppPrefsRefNum);
		theResError = ResError();

		// read app prefs
		if (theResError == noErr)
		{
			tmpAppHandle = (app_prefs_hdl_t) Get1Resource(kAppPrefsRsrc, kPrefs_rsrcID);
			theResError = ResError();
			if (theResError != noErr)
				tmpAppHandle = NULL;
		}
	
		// read default file prefs
		if (theResError == noErr)
		{
			tmpFileHandle = (file_prefs_hdl_t) Get1Resource(kFilePrefsRsrc, kPrefs_rsrcID);
			theResError = ResError();
			if (theResError != noErr)
				tmpFileHandle = NULL;
		}
	
		if (theResError != noErr)
		{
			// error creating prefs file
			(void)displayDialog(145, NULL, theResError, ewcDoCentering, eMainDevice);
			exit_handler();
		}
	
		// did we get the App prefs?
		if (tmpAppHandle != NULL)
		{
			// Is it the wrong version? Delete it if so..
			if ((**tmpAppHandle).prefsVersion != kPrefs_rsrcVers)
			{
				RmveResource((Handle)tmpAppHandle);
				UpdateResFile(AppPrefsRefNum);
				DisposeHandle((Handle)tmpAppHandle);
				tmpAppHandle = NULL; // flag code below to add new
			}
		}
	
		// did we get the Default File prefs?
		if (tmpFileHandle != NULL)
		{
			// Is it the wrong version or did we fail to get App prefs? Delete file prefs if so..
			if ( ((**tmpFileHandle).prefsVersion != kPrefs_rsrcVers)
			|| (tmpAppHandle == NULL) )
			{
				RmveResource((Handle)tmpFileHandle);
				UpdateResFile(AppPrefsRefNum);
				DisposeHandle((Handle)tmpFileHandle);
				tmpFileHandle = NULL; // flag code below to add new
			}
		}
	
		// Still got App Prefs?  Use it!
		if (tmpAppHandle != NULL)
		{
			// copy contents from rsrc to our memory buffer
			**gAppPrefs_h = **tmpAppHandle;
			ReleaseResource((Handle)tmpAppHandle);
		}

		// Still got File Prefs?  Use it!
		if (tmpFileHandle != NULL)
		{
			// copy contents from rsrc to our memory buffer
			**gDefltFilePrefs_h = **tmpFileHandle;

			// make sure our windows are visible on THIS MACHINE'S screens
			ForceRectOnScreen(&(**gDefltFilePrefs_h).srcWind_pos);
			ForceRectOnScreen(&(**gDefltFilePrefs_h).statWind_pos);
			ForceRectOnScreen(&(**gDefltFilePrefs_h).imageWind_pos);
			ReleaseResource((Handle)tmpFileHandle);
		}
	
		// return to app rsrc for dialogs
		UseResFile(AppRefNum);
	
		// All done, clean up shop
		CloseAppPrefs();

		// fill & add new ones if we couldn't read or use the old ones
		if ((tmpAppHandle == NULL) || (tmpFileHandle == NULL))
		{
			// didn't get it, tell user what's happening with a little dialog..
			TargetTicks = TICKS + kUpdPrefsTime;
			updPrefsDialogPtr = GetNewDialog(147, NULL, (WindowPtr)-1); // updating prefs..
			if (updPrefsDialogPtr)
			{
				PositionWindow(updPrefsDialogPtr, ewcDoCentering, eMainDevice, (WindowPtr)NULL);
				ShowWindow(updPrefsDialogPtr);
				SelectWindow(updPrefsDialogPtr);
				DrawDialog(updPrefsDialogPtr);
			}

			//
			// --- set global app record to defaults
			//

			if (tmpAppHandle == NULL)
			{
				(**gAppPrefs_h).prefsVersion		= kPrefs_rsrcVers;
				(**gAppPrefs_h).howMultiFriendly	= kDefMultiFriendly;
				(**gAppPrefs_h).whenToNotify		= eWhenNtf_BgFg;
				(**gAppPrefs_h).howToNotify			= eHowNtf_Noise;

				// Just use current volume name as include directory for now
				GetVol((**gAppPrefs_h).includeDirFSSpec.name,
						&(**gAppPrefs_h).includeDirFSSpec.vRefNum);
				(**gAppPrefs_h).includeDirFSSpec.parID = fsRtDirID;

			} // if (tmpAppHandle == NULL)

			//
			// --- set global default file record to defaults
			//

			if (tmpFileHandle == NULL)
			{

				(**gDefltFilePrefs_h).prefsVersion = kPrefs_rsrcVers;

				// SOURCE window
				SetRect(&(**gDefltFilePrefs_h).srcWind_pos,	// left, top, right, bottom
								qd.screenBits.bounds.left + 2,
								qd.screenBits.bounds.top + GetMBarHeight()+40,
								min(qd.screenBits.bounds.left + 480, qd.screenBits.bounds.right - 40),
								400);

				// STATUS window
				SetRect(&(**gDefltFilePrefs_h).statWind_pos,	// left, top, right, bottom
								qd.screenBits.bounds.left + 3,
								qd.screenBits.bounds.bottom - 260,
								min(qd.screenBits.bounds.left + 480, qd.screenBits.bounds.right - 40),
								qd.screenBits.bounds.bottom - 20);
	
				// IMAGE window
				SetRect(&(**gDefltFilePrefs_h).imageWind_pos,	// left, top, right, bottom
								qd.screenBits.bounds.left + 1,
								qd.screenBits.bounds.top + GetMBarHeight()+20,
								0, 0); // not used
	
				(**gDefltFilePrefs_h).imageWidth = scrnSizeMenu[kDfltScrnSize][0]; // x
				(**gDefltFilePrefs_h).imageHeight = scrnSizeMenu[kDfltScrnSize][1]; // y
				(**gDefltFilePrefs_h).imageUpperLeft.h = 0;
				(**gDefltFilePrefs_h).imageUpperLeft.v = 0;

				SetRect(&(**gDefltFilePrefs_h).selectionArea, 1, 1,
						(**gDefltFilePrefs_h).imageWidth,
						(**gDefltFilePrefs_h).imageHeight);

				(**gDefltFilePrefs_h).pictFileCreator	= 'ttxt';
				(**gDefltFilePrefs_h).targaFileCreator	= '8BIM';

				(**gDefltFilePrefs_h).renderQuality		= 9;
				(**gDefltFilePrefs_h).custom_palette	= palette_median;
				(**gDefltFilePrefs_h).imageMagFactor	= viewmn_normal;

				(**gDefltFilePrefs_h).createTargaOutfile = false;
				(**gDefltFilePrefs_h).continueTarga		= false;
				(**gDefltFilePrefs_h).doDither			= true;
				(**gDefltFilePrefs_h).doBoundSlabs		= false;
				(**gDefltFilePrefs_h).boundSlabThreshold= 0;
				(**gDefltFilePrefs_h).progress			= kProgNone;
				(**gDefltFilePrefs_h).languageVersion	= kMaxLangVersion;
				(**gDefltFilePrefs_h).maxSymbolsVal		= kDefaultMaxSym;
				
				(**gDefltFilePrefs_h).doAnimation		= false;
				(**gDefltFilePrefs_h).doCompression		= false;
				(**gDefltFilePrefs_h).do24BitPict		= true;

				(**gDefltFilePrefs_h).doAntialias		= false;
				(**gDefltFilePrefs_h).antialiasThreshold = 0.30;
				(**gDefltFilePrefs_h).antialiasDepth	= 3;
				(**gDefltFilePrefs_h).antiJitterScale	= 0.0;

				// CLOCK - animation	
				(**gDefltFilePrefs_h).animRec.frameVal0	= 1;
				(**gDefltFilePrefs_h).animRec.frameValN	= 50;
				(**gDefltFilePrefs_h).animRec.frameValS	= (**gDefltFilePrefs_h).animRec.frameVal0;
				(**gDefltFilePrefs_h).animRec.frameValE	= (**gDefltFilePrefs_h).animRec.frameValN;
				(**gDefltFilePrefs_h).animRec.clockVal0	= 0.0;
				(**gDefltFilePrefs_h).animRec.clockValN	= 10.0;

				// Initial defaults for SC compression dialog	
				if (gtheSCComponent)
				{
					(void)SCGetInfo(gtheSCComponent, scSpatialSettingsType, &(**gDefltFilePrefs_h).sc_DialogParams);
				}
			} // if (tmpFileHandle == NULL)

			WriteAppPrefs();
	
			// if update dialog is up, wait for user to read it, then tear it down
			if (updPrefsDialogPtr)
			{
				SysBeep(4);
				while (TICKS < TargetTicks)
					{
					}
				DisposeDialog(updPrefsDialogPtr);
			}
	
		} // didn't get it, create new
	
		// make sure file prefs is initialized too
		**gFilePrefs_h = **gDefltFilePrefs_h; // initialize this to defaults
	
		// set up our current prefs to be those of the file.
		// This can be changed with the App Prefs dialog
		gPrefs2Use_h = gFilePrefs_h;

	}
} // GetAppPrefs



// ---------------------------------------------------------------------
// Open a source file's resource fork for getting/putting settings info
static void OpenFilePrefs(void)
{
	if (gSrcWind_VRefNum != 0)	// if there is a file open..
	{
		SetVol(NULL, gSrcWind_VRefNum);
		if (SrcResRefNum != kRsrcFileClosed)
			printf("## ERROR! opening already-opened rsrc file: OpenFilePrefs()\n");
		SrcResRefNum = OpenResFile(gSrcWind_FileName);
		if (SrcResRefNum == kRsrcFileClosed)
		{
			CreateResFile(gSrcWind_FileName);
			SrcResRefNum = OpenResFile(gSrcWind_FileName);
		}
	}
} // OpenFilePrefs



// ---------------------------------------------------------------------
// Close the source file's resource fork
static void CloseFilePrefs(void)
{
	if (SrcResRefNum != kRsrcFileClosed)
		CloseResFile(SrcResRefNum);
	SrcResRefNum = kRsrcFileClosed;
} // CloseFilePrefs



// ---------------------------------------------------------------------
// Update the source file's settings info
void WriteFilePrefs(void)
{
	file_prefs_hdl_t	tmpFilePrefsHandle;

	if (SrcResRefNum == kRsrcFileClosed)
		OpenFilePrefs();

	if (SrcResRefNum != kRsrcFileClosed)
	{
		/* switch over to our open file */
		UseResFile(SrcResRefNum);

		/* as long as there are old file prefs resources, delete them */
		do {
			tmpFilePrefsHandle = (file_prefs_hdl_t)Get1IndResource(kFilePrefsRsrc, 1);
			if (tmpFilePrefsHandle != NULL)
			{
				RmveResource((Handle)tmpFilePrefsHandle);
				UpdateResFile(SrcResRefNum);
				DisposeHandle((Handle)tmpFilePrefsHandle);
			}
		} while (tmpFilePrefsHandle != NULL);

		/* Create a new resource to add */
		tmpFilePrefsHandle = (file_prefs_hdl_t)NewHandle(sizeof(file_prefs_rec_t));

		/* Fill it up and add it to file */
		if (tmpFilePrefsHandle != NULL)
		{
			**tmpFilePrefsHandle = **gFilePrefs_h;
			AddResource((Handle)tmpFilePrefsHandle, kFilePrefsRsrc, kPrefs_rsrcID, "\pPOV-Ray File Prefs");
			WriteResource((Handle)tmpFilePrefsHandle);
			ReleaseResource((Handle)tmpFilePrefsHandle);
		}
		CloseFilePrefs();
		UseResFile(AppRefNum);
	} // if opened
} // WriteFilePrefs



// ---------------------------------------------------------------------
// Get the setting info from the source file
static void GetFilePrefs(void)
{
	file_prefs_hdl_t	tmpFilePrefsHandle = NULL;

	if (SrcResRefNum == kRsrcFileClosed)
		OpenFilePrefs();

	if (SrcResRefNum != kRsrcFileClosed)
		tmpFilePrefsHandle = (file_prefs_hdl_t)Get1Resource(kFilePrefsRsrc, kPrefs_rsrcID);

	if (tmpFilePrefsHandle != NULL)
	{
		/* Is it the wrong version? */
		if ((**tmpFilePrefsHandle).prefsVersion != kPrefs_rsrcVers)
		{	/* old version, delete & create new one below! */
			RmveResource((Handle)tmpFilePrefsHandle);
			UpdateResFile(SrcResRefNum);
			DisposeHandle((Handle)tmpFilePrefsHandle);
			tmpFilePrefsHandle = NULL; /* flag code below to add new */
		}
		/* Still got it?  Use it! */
		if (tmpFilePrefsHandle != NULL)
		{
			**gFilePrefs_h = **tmpFilePrefsHandle;
			/* make sure our windows are on THIS MACHINE'S screens */
			ForceRectOnScreen(&(**gFilePrefs_h).srcWind_pos);
			ForceRectOnScreen(&(**gFilePrefs_h).statWind_pos);
			ForceRectOnScreen(&(**gFilePrefs_h).imageWind_pos);

			if ((**gDefltFilePrefs_h).progress >= kProgDebug)
			{
				printf("-d GetFilePrefs():\n");
				printf("-d   ImageWind = <%d, %d, %d, %d>\n",
					(**gFilePrefs_h).imageWind_pos.left,
					(**gFilePrefs_h).imageWind_pos.top,
					(**gFilePrefs_h).imageWind_pos.right,
					(**gFilePrefs_h).imageWind_pos.bottom);
				printf("-d   srcWind = <%d, %d, %d, %d>\n",
					(**gFilePrefs_h).srcWind_pos.left,
					(**gFilePrefs_h).srcWind_pos.top,
					(**gFilePrefs_h).srcWind_pos.right,
					(**gFilePrefs_h).srcWind_pos.bottom);
			}

		}
	}

	/* create a new one if can't read or use the old one */
	if (tmpFilePrefsHandle == NULL)
	{
		**gFilePrefs_h = **gDefltFilePrefs_h;	// get default settings
		WriteFilePrefs();
	}
	else
		ReleaseResource((Handle)tmpFilePrefsHandle);
} // GetFilePrefs


//
// POV-Ray Prefs Dialog items
//
#define	kPU_HowMFFriendlyPopup			4
#define	kRB_UseDefaultAppPrefs			5
#define	kRB_UseDefaultFilePrefs			6
#define	kPU_NotifyPopup					7
#define	kPU_NotifyStylePopup			8
#define	kBT_SetSearchPath				9
#define	kST_SearchPath					11
#define	k_LastItem					kST_SearchPath
#define	kRsrc_HowMultiFriendlyMenuID	300
#define	kRsrc_NotifyMenuID				303
#define	kRsrc_NotifyStyleMenuID			304

// ---------------------------------------------------------------------
// Prompt the user to change the Application Preferences
static void ChangeAppPrefs(void)
{
	short			i, dummyInt;
	short			itemHit;
	DialogPtr		myDialog;
	Rect			displayRect;
	ControlHandle	cntl[k_LastItem+1];
	app_prefs_rec_t	thePrefs;
	popupRec_t		aPopupRec;
	Str255			thePath;

	myDialog = GetNewDialog(131, NULL, (WindowPtr) -1);
	if (!myDialog)
	{
		SysBeep(4);
		return;
	}

	DisableMenus();
	
	// update temporary prefs record until user clicks OK!
	thePrefs = **gAppPrefs_h;

	for (i = 1; i <= k_LastItem; i++)
		GetDItem(myDialog, i, &dummyInt, (Handle *) &cntl[i], &displayRect);

	// init the popup menus
	InitPopups();

	// Add the Multifriendliness popup
	aPopupRec.fParentDialog	= myDialog;
	aPopupRec.fMenuID		= kRsrc_HowMultiFriendlyMenuID;
	aPopupRec.fPopupItemID	= kPU_HowMFFriendlyPopup;
	aPopupRec.fLastChoice	= thePrefs.howMultiFriendly;
	AddPopupToList(&aPopupRec);

	// Add the Notify & Notify Style popups
	aPopupRec.fParentDialog	= myDialog;
	aPopupRec.fMenuID		= kRsrc_NotifyMenuID;
	aPopupRec.fPopupItemID	= kPU_NotifyPopup;
	aPopupRec.fLastChoice	= (short)thePrefs.whenToNotify+1;
	AddPopupToList(&aPopupRec);

	aPopupRec.fParentDialog	= myDialog;
	aPopupRec.fMenuID		= kRsrc_NotifyStyleMenuID;
	aPopupRec.fPopupItemID	= kPU_NotifyStylePopup;
	aPopupRec.fLastChoice	= (short)thePrefs.howToNotify+1;
	AddPopupToList(&aPopupRec);

	// convert volume name to vref for making full path
	VolName2VRef(thePrefs.includeDirFSSpec.name,
				&thePrefs.includeDirFSSpec.vRefNum);
	// make full path
	PathNameFromDirID(thePrefs.includeDirFSSpec.parID, thePrefs.includeDirFSSpec.vRefNum, (char*)thePath);
	c2pstr((char*)thePath);
	// stuff path into display field
	SetIText((Handle) cntl[kST_SearchPath], thePath);

	PositionWindow(myDialog, ewcDoCentering, eSameAsPassedWindow, (WindowPtr)gp2wWindow);

	/* "default" the OK button */
	SetupDefaultButton(myDialog);
	SetCursor(&qd.arrow);
	ShowWindow(myDialog);

	do
	{
		// Set the Use App/File prefs RBs
		SetCtlValue(cntl[kRB_UseDefaultAppPrefs],	gUseAppDefaultPrefs);
		SetCtlValue(cntl[kRB_UseDefaultFilePrefs],	!gUseAppDefaultPrefs);

#if defined(__powerc)
		ModalDialog((ModalFilterUPP)&gModalFilterRD, &itemHit);
#else
		ModalDialog((ModalFilterProcPtr)ModalFilter, &itemHit);
#endif // __powerc
		switch (itemHit)
		{
			case kRB_UseDefaultAppPrefs:
				gUseAppDefaultPrefs = true;
				break;

			case kRB_UseDefaultFilePrefs:
				gUseAppDefaultPrefs = false;
				break;

			case kBT_SetSearchPath:
				// Call up Standard File to let user find the path
				if (PromptForSearchPath(&thePrefs.includeDirFSSpec))
				{
					PathNameFromDirID(thePrefs.includeDirFSSpec.parID, thePrefs.includeDirFSSpec.vRefNum, (char*)thePath);
					c2pstr((char*)thePath);
					SetIText((Handle) cntl[kST_SearchPath], thePath);
				}
				break;
		}
	}
	while ((itemHit != ok) && (itemHit != cancel));

	if (itemHit == ok)
	{
		thePrefs.howMultiFriendly = GetPopupValue(kPU_HowMFFriendlyPopup);
		gCpuHogginess = thePrefs.howMultiFriendly;
		CalcCpuReleaseTicks(true);

		// find When notification
		thePrefs.whenToNotify = (WhenNotify_t)GetPopupValue(kPU_NotifyPopup)-1;

		// find How notification
		thePrefs.howToNotify = (HowNotify_t)GetPopupValue(kPU_NotifyStylePopup)-1;

		// Change which rendering prefs to use
		if (gUseAppDefaultPrefs)
			gPrefs2Use_h = gDefltFilePrefs_h;
		else
			gPrefs2Use_h = gFilePrefs_h;

		// fill in the prefs with new values
		**gAppPrefs_h = thePrefs;

		// save prefs in file if OK
		WriteAppPrefs();
	}

	// all done with the popup menus
	KillPopups();

	SetCursor(&qd.arrow);

	// get rid of the dialog
	DisposeDialog(myDialog);

	EnableMenus();
} // ChangeAppPrefs



//
// Rendering Options Dialog items
//
#define	kST_Filename			4
#define	kET_fImageWidth			5
#define	kET_fImageHeight		6
#define	kET_fRowFrom			7
#define	kET_fRowTo				8
#define	kPU_fImageQuality		9
#define	kCB_fAntiAliasCheck		10
#define	kPU_fTAntiAliasThresh	11
#define	kET_fAntiAliasThresh	12
#define	kST_fTAntialiasDepth	13
#define	kST_fAntialiasDepth		14
#define	kST_fTAntiJitterScale	15
#define	kET_fAntiJitterScale	16
#define	kUU_fAntiAliasDummyItem	17
#define kPU_fProgress			18
#define	kPU_fMaxBounds			19
#define	kCB_fAnimate			20
#define	kBT_fAnimSettings		21
#define kPU_fLangVersion		22
#define	kCB_fDoPICTCmp			23
#define	kBT_fPICTCmpSettings	24
#define	kPU_fMaxSyms			25
#define	kCB_fCreateTarga		26
#define	kCB_fContinueFromTarga	27
#define	kPU_fImageSize			28
#define	kCB_SaveAsDefault		29
#define	kItem_fLastItem			kCB_SaveAsDefault

#define	kMenuID_fImageQuality	301
#define	kMenuID_fImageSize		302
#define	kMenuID_fMaxSym			305
#define	kMenuID_fLangVersion	306
#define	kMenuID_fProgress		307
#define	kMenuID_fMaxBounds		308


// ---------------------------------------------------------------------
// dim a rectangular area of a window
static void erase_TE_rect(WindowPtr dlgWindPtr, Rect *pRect)
{
#pragma unused (dlgWindPtr)
	PenState	thePen;
	Rect		outerRect;

	// Save and restore the pen state so we don't mess things up
	// for other drawing routines
	GetPenState(&thePen);

	outerRect = *pRect;
	InsetRect(&outerRect, -3, -3); // border outside TE item
	ForeColor(whiteColor);
	FrameRect(&outerRect);

	SetPenState(&thePen);
	ForeColor(blackColor);
}


// ---------------------------------------------------------------------
// dim a rectangular area of a window
static void dim_rect(WindowPtr dlgWindPtr, Rect *pRect)
{
#pragma unused (dlgWindPtr)
	PenState	thePen;

	// Save and restore the pen state so we don't mess things up
	// for other drawing routines
	GetPenState(&thePen);
	PenMode(notPatBic);

	PenPat((ConstPatternParam)&qd.gray);
	PaintRect(pRect);

	SetPenState(&thePen);
}


// ---------------------------------------------------------------------
// dim/enable the aa edit text fields
static void update_edit_text(WindowPtr dlgWindPtr)
{
#define	MAX_TE_CONTROLS			3	// size of theTEControlH array

	Boolean			aaCheckBox;
	int				k;
	short int		itemType;
	GrafPtr			saveGP;
	Rect			aRect;
	ControlHandle	theAAControlH;
	ControlHandle	theTEControlH[MAX_TE_CONTROLS];
	short int		theTEitem[MAX_TE_CONTROLS];

	GetPort(&saveGP);
	SetPort(dlgWindPtr);

	// What is the state of the aa checkbox now?
	GetDItem(dlgWindPtr, kCB_fAntiAliasCheck, &itemType, (Handle*)&theAAControlH, &aRect);
	aaCheckBox = GetCtlValue(theAAControlH);

	// Get the actual TE items
	theTEitem[0] = kET_fAntiAliasThresh;
	theTEitem[1] = kST_fAntialiasDepth;
	theTEitem[2] = kET_fAntiJitterScale;
	GetDItem(dlgWindPtr, theTEitem[0], &itemType, (Handle*)&(theTEControlH[0]), &aRect);
	GetDItem(dlgWindPtr, theTEitem[1], &itemType, (Handle*)&(theTEControlH[1]), &aRect);
	GetDItem(dlgWindPtr, theTEitem[2], &itemType, (Handle*)&(theTEControlH[2]), &aRect);

	// dim/enable each TE item
	for (k=0; k<MAX_TE_CONTROLS; k++)
		if (aaCheckBox)
		{	// aa is ON, set to enabled EditText item
			// enable TE
			GetDItem(dlgWindPtr, theTEitem[k], &itemType, (Handle*)&(theTEControlH[k]), &aRect);
	 		SetDItem(dlgWindPtr, theTEitem[k], editText, (Handle)theTEControlH[k], &aRect);
			// invalidate TEs so they are redrawn
			InsetRect(&aRect, -5, -5); // just outside TE item
			InvalRect(&aRect);
		}
		else
		{	// aa is OFF, disable & dim TE item

	        // If current text edit cursor inside dimmed TE...
	        if	( ((DialogPeek)dlgWindPtr)->editField == theTEitem[k]-1 )
	        {
				// bump it back to beginning edit text field
				SelIText(dlgWindPtr, kET_fImageWidth, 0, 0);
			}

			// make it think it is a static text item, not EditText
			GetDItem(dlgWindPtr, theTEitem[k], &itemType, (Handle*)&(theTEControlH[k]), &aRect);
	 		erase_TE_rect(dlgWindPtr, &aRect);
			SetDItem(dlgWindPtr, theTEitem[k], statText+itemDisable, (Handle)(theTEControlH[k]), &aRect);

			InsetRect(&aRect, -5, -5); // just outside TE item
			InvalRect(&aRect);
		} // else off

	SetPort(saveGP);

} // update_edit_text



// ---------------------------------------------------------------------
// Determine closest index in the scrnSizeMenu array to the h/v passed
static short GetClosestImageSizeItem(short pWidth, short pHeight)
{
	int		k;

	// go down the list & find a match
	for (k=0; k<SCRN_SIZE_MENU_MAX; k++)
	{
		if ((scrnSizeMenu[k][0] >= pWidth) && (scrnSizeMenu[k][1] >= pHeight))
			return k+1;	// return item #
	}

	// off the scale, return max
	return SCRN_SIZE_MENU_MAX;

} // GetClosestImageSizeItem



// ---------------------------------------------------------------------
// Prompt the user to change the file settings
static void ChangeFilePrefs(void)
{
	short				itemHit;
	short				i, dummyInt;
	DialogPtr			myDialog;
	Rect				displayRect;
	file_prefs_rec_t	thePrefs;
	ControlHandle		cntl[kItem_fLastItem+1];
	char				s1[256];
	popupRec_t			aPopupRec;

	myDialog = GetNewDialog(128, NULL, (WindowPtr) -1);
	if (!myDialog)
	{
		SysBeep(4);
		return;
	}

	DisableMenus();
	
	// update temporary prefs record until user clicks OK!
	thePrefs = **gFilePrefs_h;

	for (i = 1; i <= kItem_fLastItem; i++)
		GetDItem(myDialog, i, &dummyInt, (Handle *) &cntl[i], &displayRect);

	SetIText((Handle) cntl[kST_Filename], gSrcWind_FileName);

	// init the popup menus
	InitPopups();

	// Add the Image Quality popup
	aPopupRec.fParentDialog	= myDialog;
	aPopupRec.fMenuID		= kMenuID_fImageQuality;
	aPopupRec.fPopupItemID	= kPU_fImageQuality;
	aPopupRec.fLastChoice	= thePrefs.renderQuality+1;
	AddPopupToList(&aPopupRec);

	// Add the Image Size Presets popup
	aPopupRec.fParentDialog	= myDialog;
	aPopupRec.fMenuID		= kMenuID_fImageSize;
	aPopupRec.fPopupItemID	= kPU_fImageSize;
	aPopupRec.fLastChoice	= GetClosestImageSizeItem(thePrefs.imageWidth, thePrefs.imageHeight);
	AddPopupToList(&aPopupRec);

	// Add the Auto Bounds popup
	aPopupRec.fParentDialog	= myDialog;
	aPopupRec.fMenuID		= kMenuID_fMaxBounds;
	aPopupRec.fPopupItemID	= kPU_fMaxBounds;
	if (thePrefs.doBoundSlabs)
		// off, ---, 1, 5, 10, ...
		aPopupRec.fLastChoice	= 3 + (thePrefs.boundSlabThreshold / 5);
	else
		aPopupRec.fLastChoice	= 1; // off
	AddPopupToList(&aPopupRec);

	// Add the Max Symbols popup
	aPopupRec.fParentDialog	= myDialog;
	aPopupRec.fMenuID		= kMenuID_fMaxSym;
	aPopupRec.fPopupItemID	= kPU_fMaxSyms;
	if (thePrefs.maxSymbolsVal < kMaxSymMinVal)
		thePrefs.maxSymbolsVal = kMaxSymMinVal;
	else if (thePrefs.maxSymbolsVal > kMaxSymMaxVal)
		thePrefs.maxSymbolsVal = kMaxSymMaxVal;
	// set fLastChoice to 1, 2, 3, etc.
	aPopupRec.fLastChoice	= (thePrefs.maxSymbolsVal / kMaxSymFactor);
	AddPopupToList(&aPopupRec);

	// Add the Language Version popup
	aPopupRec.fParentDialog	= myDialog;
	aPopupRec.fMenuID		= kMenuID_fLangVersion;
	aPopupRec.fPopupItemID	= kPU_fLangVersion;
	if (thePrefs.languageVersion < kMinLangVersion)
		thePrefs.languageVersion = kMinLangVersion;
	else if (thePrefs.languageVersion > kMaxLangVersion)
		thePrefs.languageVersion = kMaxLangVersion;
	aPopupRec.fLastChoice	= thePrefs.languageVersion;
	AddPopupToList(&aPopupRec);

	// Add the Progress popup
	aPopupRec.fParentDialog	= myDialog;
	aPopupRec.fMenuID		= kMenuID_fProgress;
	aPopupRec.fPopupItemID	= kPU_fProgress;
	if (thePrefs.progress < kProgNone)
		thePrefs.progress = kProgNone;
	else if (thePrefs.progress > kProgMax)
		thePrefs.progress = kProgMax;

	// set fLastChoice to 1, 2, 3, etc.
	aPopupRec.fLastChoice	= thePrefs.progress;
	AddPopupToList(&aPopupRec);

	sprintf(s1, "%d", thePrefs.imageWidth);
	SetIText((Handle) cntl[kET_fImageWidth], c2pstr(s1) );

	sprintf(s1, "%d", thePrefs.imageHeight);
	SetIText((Handle) cntl[kET_fImageHeight], c2pstr(s1) );

	sprintf(s1, "%d", thePrefs.selectionArea.top);
	SetIText((Handle) cntl[kET_fRowFrom], c2pstr(s1) );

	sprintf(s1, "%d", thePrefs.selectionArea.bottom);
	SetIText((Handle) cntl[kET_fRowTo], c2pstr(s1) );

	// Currently, selection L/R is not settable, so force its value
	thePrefs.selectionArea.left = 1;
	thePrefs.selectionArea.right = thePrefs.imageWidth;

	SetCtlValue(cntl[kCB_fAntiAliasCheck], thePrefs.doAntialias);

	sprintf(s1, "%.2f", thePrefs.antialiasThreshold);
	SetIText((Handle) cntl[kET_fAntiAliasThresh], c2pstr(s1) );

	sprintf(s1, "%d", thePrefs.antialiasDepth);
	SetIText((Handle) cntl[kST_fAntialiasDepth], c2pstr(s1) );

	sprintf(s1, "%.2f", thePrefs.antiJitterScale);
	SetIText((Handle) cntl[kET_fAntiJitterScale], c2pstr(s1) );

	SetCtlValue(cntl[kCB_fAnimate], thePrefs.doAnimation);
	HiliteControl(cntl[kBT_fAnimSettings], thePrefs.doAnimation?0:255);  // dimmed

	SetCtlValue(cntl[kCB_fDoPICTCmp], (thePrefs.doCompression && gHasImageCompressionMgr));
	HiliteControl(cntl[kCB_fDoPICTCmp], gHasImageCompressionMgr?0:255);  // dimmed
	HiliteControl(cntl[kBT_fPICTCmpSettings], GetCtlValue(cntl[kCB_fDoPICTCmp])?0:255);	// enabled

	SetCtlValue(cntl[kCB_fCreateTarga], thePrefs.createTargaOutfile);
	if(thePrefs.createTargaOutfile)
		SetCtlValue(cntl[kCB_fContinueFromTarga], thePrefs.continueTarga);
	else
		HiliteControl(cntl[kCB_fContinueFromTarga], 255);

	// Save as default always starts out as OFF
	SetCtlValue(cntl[kCB_SaveAsDefault], false);

	// select something...
	SelIText(myDialog, kET_fImageWidth, 0, -1);

	PositionWindow(myDialog, ewcDoCentering, eSameAsPassedWindow, (WindowPtr)gp2wWindow);

	// "default" the OK button
	SetupDefaultButton(myDialog);
	SetCursor(&qd.arrow);
	ShowWindow(myDialog);
	update_edit_text(myDialog);

	do
	{
#if defined(__powerc)
		ModalDialog((ModalFilterUPP)&gModalFilterRD, &itemHit);
#else
		ModalDialog((ModalFilterProcPtr)ModalFilter, &itemHit);
#endif
		switch (itemHit)
		{
			case kCB_fAntiAliasCheck:		/* anti-alias */
				SetCtlValue(cntl[itemHit], !GetCtlValue(cntl[itemHit]));
                update_edit_text(myDialog);
      			break;

			case kCB_fCreateTarga:  /* Create Targa file */
				SetCtlValue(cntl[itemHit], !GetCtlValue(cntl[itemHit]));
				if (GetCtlValue(cntl[itemHit]) == 0)
				{
					// if no Targa output, then certainly no contine
					SetCtlValue(cntl[kCB_fContinueFromTarga],0);
					HiliteControl(cntl[kCB_fContinueFromTarga], 255); // dim it
				}
				else
					HiliteControl(cntl[kCB_fContinueFromTarga], 0);
				break;

			case kCB_fContinueFromTarga:
			case kCB_SaveAsDefault:
				SetCtlValue(cntl[itemHit], !GetCtlValue(cntl[itemHit]));
				break;

			case kCB_fDoPICTCmp:
				SetCtlValue(cntl[itemHit], !GetCtlValue(cntl[itemHit]));
				HiliteControl(cntl[kBT_fPICTCmpSettings],
							GetCtlValue(cntl[kCB_fDoPICTCmp])?0:255);
				break;

			case kBT_fPICTCmpSettings:
				{
				(void)SCRequestImageSettings(gtheSCComponent);
				break;
				}

			case kCB_fAnimate:
				SetCtlValue(cntl[itemHit], !GetCtlValue(cntl[itemHit]));
				HiliteControl(cntl[kBT_fAnimSettings],
							GetCtlValue(cntl[itemHit])?0:255);
				break;

			case kBT_fAnimSettings:
				GetAnimateOptions(&thePrefs.animRec);
				break;

			case kPU_fImageSize:
				// get index into imageSize array
				i = GetPopupValue(kPU_fImageSize) - 1;

				// width
				sprintf(s1, "%d", scrnSizeMenu[i][0]);	// X (width)
				c2pstr(s1);
				SetIText((Handle) cntl[kET_fImageWidth], (StringPtr)s1);

				// height
				sprintf(s1, "%d", scrnSizeMenu[i][1]); // Y (height)
				c2pstr(s1);
				SetIText((Handle) cntl[kET_fImageHeight], (StringPtr)s1);

				// from top (always 1)
				SetIText((Handle) cntl[kET_fRowFrom], "\p1");
				// to bottom
				SetIText((Handle) cntl[kET_fRowTo], (StringPtr)s1);

				// Currently, selection L/R is not settable, so force its value
				thePrefs.selectionArea.left = 1;
				thePrefs.selectionArea.right = scrnSizeMenu[i][0];	// X (width)

				// re-select first item (size may have changed)
				SelIText(myDialog, kET_fImageWidth, 0, -1);
				break;
		}
	}
	while (itemHit != ok && itemHit != cancel);
	
	if (itemHit == ok)
	{
		GetIText((Handle) cntl[kET_fImageWidth], (StringPtr)s1);
		sscanf(p2cstr((StringPtr)s1) , "%hd", &thePrefs.imageWidth);
		PinValue(&thePrefs.imageWidth, 1, 9999);
		thePrefs.selectionArea.right = thePrefs.imageWidth;

		GetIText((Handle) cntl[kET_fImageHeight], (StringPtr)s1);
		sscanf(p2cstr((StringPtr)s1) , "%hd", &thePrefs.imageHeight);
		PinValue(&thePrefs.imageHeight, 1, 9999);

		GetIText((Handle) cntl[kET_fRowFrom], (StringPtr)s1);
		sscanf(p2cstr((StringPtr)s1) , "%hd", &thePrefs.selectionArea.top);
		PinValue(&thePrefs.selectionArea.top, 1, thePrefs.imageHeight);

		GetIText((Handle) cntl[kET_fRowTo], (StringPtr)s1);
		sscanf(p2cstr((StringPtr)s1) , "%hd", &thePrefs.selectionArea.bottom);
		PinValue(&thePrefs.selectionArea.bottom, thePrefs.selectionArea.top, thePrefs.imageHeight);

		thePrefs.renderQuality = GetPopupValue(kPU_fImageQuality)-1;

		thePrefs.doAntialias = GetCtlValue(cntl[kCB_fAntiAliasCheck]);

		GetIText((Handle) cntl[kET_fAntiAliasThresh], (StringPtr)s1);
		sscanf(p2cstr((StringPtr)s1) , "%f", &thePrefs.antialiasThreshold);
		if (thePrefs.antialiasThreshold > 2.0)
			thePrefs.antialiasThreshold = 2.0;
		else if (thePrefs.antialiasThreshold < 0.0)
			thePrefs.antialiasThreshold = 0.0;

		GetIText((Handle) cntl[kST_fAntialiasDepth], (StringPtr)s1);
		thePrefs.antialiasDepth = atoi(p2cstr((StringPtr)s1));
		PinValue(&thePrefs.antialiasDepth, 1, 9);

		GetIText((Handle) cntl[kET_fAntiJitterScale], (StringPtr)s1);
		thePrefs.antiJitterScale = atof(p2cstr((StringPtr)s1));
		if (thePrefs.antiJitterScale > 2.0)
			thePrefs.antiJitterScale = 2.0;
		else if (thePrefs.antiJitterScale < 0.0)
			thePrefs.antiJitterScale = 0.0;

		thePrefs.progress = GetPopupValue(kPU_fProgress);

		// off, ---, 1, 5, 10, 15, ...
		thePrefs.boundSlabThreshold = (GetPopupValue(kPU_fMaxBounds)-3)*5;
		// if popup was at position 1,2, set to 0...
		if (thePrefs.boundSlabThreshold < 0)
			thePrefs.boundSlabThreshold = 0;
		else // if at 3, set the value to 1
		if (thePrefs.boundSlabThreshold == 0)
			thePrefs.boundSlabThreshold = 1;

		// if popup is not at position 1 (off) then it must be on! :-)
		thePrefs.doBoundSlabs = (GetPopupValue(kPU_fMaxBounds) > 1);

		thePrefs.doAnimation = GetCtlValue(cntl[kCB_fAnimate]);

		thePrefs.languageVersion =  GetPopupValue(kPU_fLangVersion);

		thePrefs.doCompression = GetCtlValue(cntl[kCB_fDoPICTCmp]);
		if (thePrefs.doCompression)
			(void)SCGetInfo(gtheSCComponent, scSpatialSettingsType, &(**gDefltFilePrefs_h).sc_DialogParams);

		thePrefs.maxSymbolsVal = kMaxSymMinVal +
				((GetPopupValue(kPU_fMaxSyms)-1) * kMaxSymFactor);

		thePrefs.createTargaOutfile = GetCtlValue(cntl[kCB_fCreateTarga]);
		thePrefs.continueTarga = GetCtlValue(cntl[kCB_fContinueFromTarga]);

		// make sure from/to width is in range
		if	(
				(thePrefs.selectionArea.left > 1)
			||	(thePrefs.selectionArea.top > 1)
			||	(thePrefs.selectionArea.right < thePrefs.imageWidth)
			||	(thePrefs.selectionArea.bottom < thePrefs.imageHeight)
			)
		{
			// you picked sub-area to render, OK?
			itemHit = displayDialog(146, NULL, 0, ewcDoCentering, eSameAsPassedWindow);
			if (itemHit == cancel)
			{
				// they changed their mind, restore sanity
				thePrefs.selectionArea.left = 1;
				thePrefs.selectionArea.top = 1;
				thePrefs.selectionArea.right = thePrefs.imageWidth;
				thePrefs.selectionArea.bottom = thePrefs.imageHeight;
			}
		}

		// Set default file prefs to mirror current file prefs settings?
		if (GetCtlValue(cntl[kCB_SaveAsDefault]))
			**gDefltFilePrefs_h = thePrefs;

		// fill in the prefs with new values
		**gFilePrefs_h = thePrefs;

		// save prefs in file if OK
		WriteFilePrefs();
	}

	// all done with the popup menus
	KillPopups();

	SetCursor(&qd.arrow);

	// get rid of the dialog
	DisposeDialog(myDialog);

	EnableMenus();
} // ChangeFilePrefs



// ---------------------------------------------------------------------
// set up the POV-Ray engine's argc/argv parameters from file settings
static void SetupRenderArgs(void)
{
	int		i;
	char	s1[256], s2[256];

	ResetArgs();

	AddArg("POV-Ray"); // argv[0] is always the program name

	// display format (just used to turn on the display calls)
	AddArg("+d0");

	// Min Language Syntax Version
	AddArg((**gPrefs2Use_h).languageVersion==1 ? "+mv1" : "+mv2");

	// renderQuality value
	sprintf(s1, "-q%d", (**gPrefs2Use_h).renderQuality);
	AddArg(s1);

	// file buffer size in Kbytes
	AddArg("+b8");

	// prompt for keypress on exit (In the Mac world?  NOT!)
	AddArg("-p");

	// set clock variable value
	if ((**gPrefs2Use_h).doAnimation)
		sprintf(s2, "%g", GetCurrClockVal());
	else
		sprintf(s2, "%g", 0.0);
	sprintf(s1, "+k%s",s2);
	AddArg(s1);

	// set max symbols value
	sprintf(s1, "+ms%d", (**gPrefs2Use_h).maxSymbolsVal);
	AddArg(s1);

	// show render progress messages
	switch ((**gPrefs2Use_h).progress)
	{
		case kProgDebug:
			AddArg("+z");
			// fall through & get the +v1 too

		case kProgVerbose:
			AddArg("+v1");
			break;

		case kProgNone:
		case kProgMinimal:
			// Minimal is Mac debugs, not core debugs
			AddArg("-v");
			break;
	}

	// exit enable (Not needed on Mac, exit via COOPERATE)
	AddArg("-x");

	// render output size
	sprintf(s1, "-w%d", (**gPrefs2Use_h).imageWidth); AddArg(s1);
	sprintf(s1, "-h%d", (**gPrefs2Use_h).imageHeight); AddArg(s1);

	// actual render rectangle
	sprintf(s1, "-sr%d", (**gPrefs2Use_h).selectionArea.top); AddArg(s1);
	sprintf(s1, "-er%d", (**gPrefs2Use_h).selectionArea.bottom); AddArg(s1);
	sprintf(s1, "-sc%d", (**gPrefs2Use_h).selectionArea.left); AddArg(s1);
	sprintf(s1, "-ec%d", (**gPrefs2Use_h).selectionArea.right); AddArg(s1);

	// enable/disable bounding slabs
	if ((**gPrefs2Use_h).doBoundSlabs)
	{
		// set min objects to start auto-bounding
		sprintf(s2, "%d", (**gPrefs2Use_h).boundSlabThreshold);
		sprintf(s1, "+mb%s",s2);
		AddArg(s1);
	}
	else
		AddArg("-mb");

	// anti-aliasing
	if ((**gPrefs2Use_h).doAntialias)
	{
		sprintf(s1, "+a%g", (**gPrefs2Use_h).antialiasThreshold);
		AddArg(s1);

		// Anti-aliasing Depth
		sprintf(s1, "+r%d", (**gPrefs2Use_h).antialiasDepth);
		AddArg(s1);

		// Anti-aliasing Jitter Scale
		if ((**gPrefs2Use_h).antiJitterScale > 0.0)
		{
			sprintf(s1, "+j%g", (**gPrefs2Use_h).antiJitterScale);
			AddArg(s1);
		}
		else
			AddArg("-j");
	}
	else
		AddArg("-a");

	// output targa file
	AddArg((**gPrefs2Use_h).createTargaOutfile ? "+ft" : "-f");

	// continue with previous targa file
	AddArg((**gPrefs2Use_h).continueTarga ? "+c" : "-c");

	// input file
	pStrCopy(gSrcWind_FileName, (StringPtr)s2);
	p2cstr((StringPtr)s2);
	sprintf(s1, "-i%s", s2); AddArg(s1);
	
	// set up targa output file name
	if ((**gPrefs2Use_h).createTargaOutfile)
	{
		// whack .POV suffix off input file name
		if ( strstr(s2, ".POV") || strstr(s2, ".pov") )
		{
			i = strlen(s2);
			s2[i - 4] = 0;
		}

		// add .tga suffix
		strcat(s2, ".tga");
		strcpy(gTargaFname, s2);
		sprintf(s1, "-o%s", s2);
		AddArg(s1);
	}

	// Add -l options for library paths
	// convert volume name to vref for making full path
	VolName2VRef((**gAppPrefs_h).includeDirFSSpec.name,
				&(**gAppPrefs_h).includeDirFSSpec.vRefNum);
	PathNameFromDirID((**gAppPrefs_h).includeDirFSSpec.parID, (**gAppPrefs_h).includeDirFSSpec.vRefNum, s2);
	// whack trailing colon, POV-Ray wants to add it!
	if (s2[strlen(s2)-1] == ':')
		s2[strlen(s2)-1] = '\0';
	sprintf(s1, "-l%s", s2);
	AddArg(s1);
	
	// set current directory to that of scene file
	SetVol(NULL, gSrcWind_VRefNum);

	// flush any pending writes prior to rendering
	FlushVol(NULL, gSrcWind_VRefNum);

	Stop_Flag = 0;
	gDoingRender = true;

	// calculate and display current CPU timeslice setting
	CalcCpuReleaseTicks(true);

} // SetupRenderArgs



// ---------------------------------------------------------------------
// Display Application About Box
static void AboutPOV(void)
{
	short		itemHit;
	Boolean		specialAbout = false;
	DialogPtr	myDialog;
	char		povVers[16],
				compilerName[16],
				compDate[32];

	// grab the dialog into memory
	myDialog = GetNewDialog(130, NULL, (WindowPtr) -1);
	if (myDialog)
	{
		// check for special info-key for about box
		if (gTheEvent.modifiers & optionKey)
		{
			specialAbout = false;
			PlayNotifySound();
		}

		strcpy(compDate, "Compiled: ");
		strcat(compDate, __DATE__);
		c2pstr(compDate);

		strcpy(povVers, POV_RAY_VERSION);
		c2pstr(povVers);

		strcpy(compilerName, COMPILER_VER);
		c2pstr(compilerName);

		GetAppVersionPString(1, (StringPtr)povVers);

		ParamText(	(StringPtr)povVers,
					(StringPtr)compilerName,
					(StringPtr)compDate,
					(StringPtr)gDistMessage);

		/* Get into dialog's port & fiddle with fonts.. */
		SetPort((GrafPtr)myDialog);
		TextFont(geneva);
		TextSize(9);

		PositionWindow(myDialog, ewcDoCentering, eDeepestDevice, (WindowPtr)gp2wWindow);

		/* "default" the OK button */
		SetupDefaultButton(myDialog);
		ShowWindow(myDialog);
		do {
			ModalDialog(NULL, &itemHit);
			// make noise if other stuff clicked.. just for fun
			if (itemHit != 1)
				PlayNotifySound();
		} while (itemHit != 1);
		TextFont(geneva);
		TextSize(9);
		DisposeDialog(myDialog);
	}

} // AboutPOV



// ---------------------------------------------------------------------
// Set all menu items availability depending on application states
static void SetItemEnable(MenuHandle theMenu, short theItem, Boolean isEnabled)
{
	if (isEnabled)
		EnableItem(theMenu, theItem);
	else
		DisableItem(theMenu, theItem);
} // SetItemEnable



// ---------------------------------------------------------------------
// Set all menu items availability depending on application states
static void AdjustMenus(void)
{
	short		i;
	WindowPtr	theFrontWindow;

	// Find out which window is in front, many menus depend on which is frontmost.
	theFrontWindow = FrontWindow();


	//======== APPLE

	SetItemEnable(myMenus[apmn_ID - menu_offset], apmn_about, true); /* about box */


	//======== FILE

	// New
	SetItemEnable(myMenus[fmn_ID - menu_offset], fmn_new, !gDoingRender && !gSrcWind_dirty);

	// Open
	SetItemEnable(myMenus[fmn_ID - menu_offset], fmn_open, !gDoingRender && !gSrcWind_dirty);

	// Close
	SetItemEnable(myMenus[fmn_ID - menu_offset], fmn_close, false);
	if (theFrontWindow == gSrcWind_Window)
	{
		if (!gDoingRender)
			SetItemEnable(myMenus[fmn_ID - menu_offset], fmn_close, true);
		}
/* -- not yet, need to prompt to save first
	else if (theFrontWindow == gImageWindowPtr)
	{
		SetItemEnable(myMenus[fmn_ID - menu_offset], fmn_close, true);
	}
*/

	// Save
	if (theFrontWindow == gSrcWind_Window)
	{
		SetItemEnable(myMenus[fmn_ID - menu_offset], fmn_save, gSrcWind_dirty || (gSrcWind_VRefNum == 0));
	}
	else if (theFrontWindow == (WindowPtr)gp2wWindow)
	{
		SetItemEnable(myMenus[fmn_ID - menu_offset], fmn_save, true);
	}
	else if (theFrontWindow == gImageWindowPtr)
	{
		SetItemEnable(myMenus[fmn_ID - menu_offset], fmn_save, !gDoingRender);
	}

	// Save As
	if (theFrontWindow == gSrcWind_Window)
	{
		SetItemEnable(myMenus[fmn_ID - menu_offset], fmn_saveas, true);
	}
	else if (theFrontWindow == gImageWindowPtr)
	{
		SetItemEnable(myMenus[fmn_ID - menu_offset], fmn_saveas, !gDoingRender);
	}
	else
	{
		SetItemEnable(myMenus[fmn_ID - menu_offset], fmn_saveas, false);
	}


	//======== EDIT

	// undo
	SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_undo,
			( (theFrontWindow == gSrcWind_Window) && (undo_record[max_undo].reason[0]) )
			|| ( (theFrontWindow == gImageWindowPtr) && gCanUndo) );

	// cut
	if (theFrontWindow == gSrcWind_Window)
	{
		SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_cut,
				(**gSrcWind_TEH).selStart != (**gSrcWind_TEH).selEnd);
	}
	else
		SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_cut, false);

	// copy
	if (theFrontWindow == gSrcWind_Window)
	{
		SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_copy,
				(**gSrcWind_TEH).selStart != (**gSrcWind_TEH).selEnd);
	}
	else
	if (theFrontWindow == gImageWindowPtr)
	{
		SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_copy, true);
	}
	else
	if (theFrontWindow == (WindowPtr)gp2wWindow)
	{
		SetItemEnable(myMenus[edmn_ID - menu_offset],
			edmn_copy,
			(**(gp2wWindow->p2wTEHandle)).selStart !=
			(**(gp2wWindow->p2wTEHandle)).selEnd );
	}

	// paste
	if (theFrontWindow == gSrcWind_Window)
	{ // can only paste into src window
		SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_paste, true);
	}
	else
		SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_paste, false);

	// clear
	if (theFrontWindow == gSrcWind_Window)
	{
		SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_clear,
				(**gSrcWind_TEH).selStart != (**gSrcWind_TEH).selEnd);
	}
	else
		SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_clear, false);


	// select all
	if ((theFrontWindow == gSrcWind_Window) || (theFrontWindow == (WindowPtr)gp2wWindow))
	{
		SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_selectAll, true);
	}
	else
		SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_selectAll, false);

	// redo
	if (theFrontWindow == gSrcWind_Window)
	{
		SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_redo, redo_record[max_undo].reason[0]);
	}
	else
		SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_redo, false);

	// goto
	SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_goto, gSrcWind_visible);

	// lookup
	if (theFrontWindow == gSrcWind_Window)
	{
		SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_lookup,
				(**gSrcWind_TEH).selStart != (**gSrcWind_TEH).selEnd);
	}
	else
		SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_lookup, false);

	// insert template
	SetItemEnable(myMenus[edmn_ID - menu_offset], edmn_macros, gSrcWind_visible);


	//======== IMAGE

	// view
	SetItemEnable(myMenus[immn_ID - menu_offset], immn_view, true);
		// should disable view submenu if !gSrcWind_visible...

	// dither
	SetItemEnable(myMenus[immn_ID - menu_offset], immn_dither, gHas32BitQD);
	CheckItem(myMenus[immn_ID - menu_offset], immn_dither, ((**gPrefs2Use_h).doDither && gHas32BitQD));

	// custom palette
	SetItemEnable(myMenus[immn_ID - menu_offset], immn_custom, true);


	//======== RENDER

	// Render options
	SetItemEnable(myMenus[rnmn_ID - menu_offset], rnmn_options,
				!gDoingRender && (gSrcWind_VRefNum != 0));

	// render
	SetItemEnable(myMenus[rnmn_ID - menu_offset], rnmn_render,
				!gDoingRender && (gSrcWind_VRefNum != 0));

	// pause rendering
	SetItemEnable(myMenus[rnmn_ID - menu_offset], rnmn_pause, gDoingRender);

	// stop rendering
	SetItemEnable(myMenus[rnmn_ID - menu_offset], rnmn_stop, gDoingRender);

	// auto-save & auto-shutdown
	CheckItem(myMenus[rnmn_ID - menu_offset], rnmn_autosave, gAutoSave);
	CheckItem(myMenus[rnmn_ID - menu_offset], rnmn_shutdown, gAutoShutdown);


	//======== PROCESSING

	for (i=psmn_border; i <= psmn_revert; i++)
	{
		Boolean b;
		b =	!gDoingRender
			&& (theFrontWindow == gImageWindowPtr)
			&& ((WindowPeek)gImageWindowPtr)->visible
			&& !gDoingVirtualFile;
		if (i != psmn_divider1)
			SetItemEnable(myMenus[psmn_ID - menu_offset], i, b);
	}


	//======== WINDOWS

	// [1] status window
	CheckItem(myMenus[wndmn_ID - menu_offset], wndmn_status, theFrontWindow == (WindowPtr)gp2wWindow);
	SetItemEnable(myMenus[wndmn_ID - menu_offset], wndmn_status, gp2wWindow != NULL);

	// [2] source window
	CheckItem(myMenus[wndmn_ID - menu_offset], wndmn_source, theFrontWindow == gSrcWind_Window);
	SetItemEnable(myMenus[wndmn_ID - menu_offset], wndmn_source, gSrcWind_visible);

	// [3] image window
	CheckItem(myMenus[wndmn_ID - menu_offset], wndmn_image, theFrontWindow == gImageWindowPtr);
	SetItemEnable(myMenus[wndmn_ID - menu_offset], wndmn_image, gImageWindowPtr && ((WindowPeek)gImageWindowPtr)->visible);


	//======== VIEW SUBMENU

	SetItemEnable(mySubMenus[viewmn_ID - submenu_offset], 0, true);
	for (i=viewmn_hidden; i <= viewmn_x4; i++)
		CheckItem(mySubMenus[viewmn_ID - submenu_offset], i, i == (**gPrefs2Use_h).imageMagFactor);


	//======== CUSTOM PALETTE SUBMENU

	SetItemEnable(mySubMenus[plmn_ID - submenu_offset], 0, gHasPictUtils);
	CheckItem(mySubMenus[plmn_ID - submenu_offset], palette_none, !use_custom_palette);
	CheckItem(mySubMenus[plmn_ID - submenu_offset], palette_default, gColorQuantMethod == systemMethod);
	CheckItem(mySubMenus[plmn_ID - submenu_offset], palette_median, gColorQuantMethod == medianMethod);
	CheckItem(mySubMenus[plmn_ID - submenu_offset], palette_popular, gColorQuantMethod == popularMethod);

} // AdjustMenus



// ---------------------------------------------------------------------
// Prompt user "OK to Quit?"
static void ask_about_quit(void)
{
	short		itemHit;

	if	(gDoingRender && ((**gPrefs2Use_h).createTargaOutfile == 0))
	{
		// ok to quit?
		itemHit = displayDialog(137, NULL, 0, ewcDoCentering, eSameAsPassedWindow);
		if (itemHit == ok)
			Stop_Flag = gQuit = 1;
	}
	else
		Stop_Flag = gQuit = 1;
} // ask_about_quit



// ---------------------------------------------------------------------
// Prompt user, "OK to Stop Rendering?"
static void ask_about_stop(void)
{
	short		itemHit;

	if	((**gPrefs2Use_h).createTargaOutfile)
		// stopped, to continue, use targa..
		itemHit = displayDialog(135, NULL, 0, ewcDoCentering, eSameAsPassedWindow);
	else
		// no targa, OK to stop?
		itemHit = displayDialog(134, NULL, 0, ewcDoCentering, eSameAsPassedWindow);

	if (itemHit == ok)
	{
		Stop_Flag = 1;
		HaltFileQ(); // halt any queued files too!
	}
} // ask_about_stop



// ---------------------------------------------------------------------
// Pause rendering until user continues
static void pause_it(void)
{

	if (gPaused == 1)
		gPaused = 2;	// already recursively paused, so now un-pause
	else
	{	// prepare to pause
		gPaused = 1;
		DisableMenus();

		// re-enable just the pause menu item
		EnableItem(myMenus[rnmn_ID - menu_offset], 0);
		EnableItem(myMenus[rnmn_ID - menu_offset], rnmn_pause);
		CheckItem(myMenus[rnmn_ID - menu_offset], rnmn_pause, 1);
		DrawMenuBar();

		// pause loop..
		do
		{
			// allow user to do stuff..
			Cooperate(true);
			// until they pick pause again (2)
		} while ((Stop_Flag == 0) && (gPaused != 2));
	
		// done pausing, restore & return
		EnableMenus();
		CheckItem(myMenus[rnmn_ID - menu_offset], rnmn_pause, 0);
		gPaused = 0;
	}
} // pause_it



// ---------------------------------------------------------------------
// Handle any window's Activate event
static void HandleActivate(WindowPtr theWindow, Boolean becomingActive)
{
/* Show/hide the window controls */
	if (becomingActive)
	{
		if ((theWindow == gSrcWind_Window) && (gSrcWind_Window != NULL))
		{
			TEActivate(gSrcWind_TEH);
			ShowControl(gSrcWind_VScroll);
		}
		else
		{
			if ((theWindow == (WindowPtr)gp2wWindow) && (gp2wWindow))
				p2w_DoActivate(gp2wWindow, becomingActive);
		}
	}
	else
	{
		if ((theWindow == gSrcWind_Window) && (gSrcWind_Window != NULL))
		{
			TEDeactivate(gSrcWind_TEH);
			HideControl(gSrcWind_VScroll);
		}
		else
		{
			if ((theWindow == (WindowPtr)gp2wWindow) && (gp2wWindow))
				p2w_DoActivate(gp2wWindow, becomingActive);
		}
	}
} // HandleActivate



// ---------------------------------------------------------------------
// Set up the undo buffers
static void init_undo_system()
{
	int i;
	for (i = 0; i <= max_undo; i++)
	{
		undo_record[i].buf_handle = NULL;
		undo_record[i].reason[0] = 0;
	}
	undo_key_copied = 0;
} // init_undo_system



// ---------------------------------------------------------------------
// Set up the redo buffers
static void init_redo_system()
{
	int i;
	for (i = 0; i <= max_undo; i++)
	{
		redo_record[i].buf_handle = NULL;
		redo_record[i].reason[0] = 0;
	}
} // init_redo_system



// ---------------------------------------------------------------------
// Dispose of all the undo buffers
static void flush_undo_system(void)
{
	int i;
	for (i = 0; i <= max_undo; i++)
	{
		if (undo_record[i].buf_handle)
		{
			DisposeHandle(undo_record[i].buf_handle);
			undo_record[i].buf_handle = NULL;
		}
		undo_record[i].reason[0] = 0;
	}
	undo_key_copied = 0;
} // flush_undo_system



// ---------------------------------------------------------------------
// Dispose of all the redo buffers
static void flush_redo_system(void)
{
	int i;
	for (i = 0; i <= max_undo; i++)
	{
		if (redo_record[i].buf_handle)
		{
			DisposeHandle(redo_record[i].buf_handle);
			redo_record[i].buf_handle = NULL;
		}
		redo_record[i].reason[0] = 0;
	}
} // flush_redo_system



// ---------------------------------------------------------------------
// get rid of all undo buffers
static short free_undo_memory(void)
{
	short	i;
	/* from the oldest to newest (except the last), dispose the Undo & redo buffers */
	for (i = 0; i < max_undo; i++)
	{
		if (redo_record[i].buf_handle)
		{
			DisposeHandle(redo_record[i].buf_handle);
			redo_record[i].buf_handle = 0;
			redo_record[i].reason[0] = 0;
			return (1);
		}			
		if (undo_record[i].buf_handle)
		{
			DisposeHandle(undo_record[i].buf_handle);
			undo_record[i].buf_handle = 0;
			undo_record[i].reason[0] = 0;
			return (1);
		}
	}
	return (0);
} // free_undo_memory



// ---------------------------------------------------------------------
// Support undo by duplicating the current file's text as necessary
void support_undo(char *the_reason, short flush_redo)
{
	int i;
	char *buffer;

	/* new changes are being made to the file.  Redo no longer makes sense. */
	if (flush_redo)
		flush_redo_system();

	/* Dispose oldest undo record storage, because the record will be destroyed. */
	if (undo_record[0].buf_handle)
		DisposeHandle(undo_record[0].buf_handle);

	/* block-move the records down to make room for a new undo entry */
	for (i = 0; i < max_undo; i++)
		undo_record[i] = undo_record[i+1];

	/* Try repeatedly to allocate memory for the undo buffer, disposing old */
	/* undo buffers as necessary. */

TRY_AGAIN:
	undo_record[max_undo].buf_handle = (char **) NewHandle((**gSrcWind_TEH).teLength);
	if (undo_record[max_undo].buf_handle == 0)
		if (free_undo_memory()) goto TRY_AGAIN;

	if (undo_record[max_undo].buf_handle == 0)
	{
		undo_record[max_undo].reason[0] = 0;
		return;
	}
	HLock(undo_record[max_undo].buf_handle);
	buffer = *(undo_record[max_undo].buf_handle);
	undo_record[max_undo].selStart = (**gSrcWind_TEH).selStart;
	undo_record[max_undo].selEnd = (**gSrcWind_TEH).selEnd;
	strcpy(undo_record[max_undo].reason, the_reason);
	strcpy(undo_menu_name, "Undo ");
	strcat(undo_menu_name, the_reason);
	c2pstr(undo_menu_name);
	SetItem(myMenus[edmn_ID - menu_offset], edmn_undo, (StringPtr)undo_menu_name);
	undo_record[max_undo].byteCount = (**gSrcWind_TEH).teLength;
	undo_record[max_undo].isDirty = gSrcWind_dirty;
	memcpy(buffer, *(**gSrcWind_TEH).hText, (**gSrcWind_TEH).teLength);
	HUnlock(undo_record[max_undo].buf_handle);
	undo_key_copied = 0;
} // support_undo



// ---------------------------------------------------------------------
// Support redo by duplicating the current file's text as necessary
static void support_redo(char *the_reason)
{
	int i;
	char *buffer;

	/* Dispose oldest redo record storage, because the record will be destroyed. */
	if (redo_record[0].buf_handle)
		DisposeHandle(redo_record[0].buf_handle);

	/* block-move the records down to make room for a new redo entry */
	for (i = 0; i < max_undo; i++)
		redo_record[i] = redo_record[i+1];

	/* Try repeatedly to allocate memory for the redo buffer, disposing old */
	/* redo buffers as necessary. */
TRY_AGAIN:
	redo_record[max_undo].buf_handle = (char **) NewHandle((**gSrcWind_TEH).teLength);
	if (redo_record[max_undo].buf_handle == 0)
		if (free_undo_memory()) goto TRY_AGAIN;

	if (redo_record[max_undo].buf_handle == 0)
	{
		redo_record[max_undo].reason[0] = 0;
		return;
	}
	HLock(redo_record[max_undo].buf_handle);
	buffer = *(redo_record[max_undo].buf_handle);
	redo_record[max_undo].selStart = (**gSrcWind_TEH).selStart;
	redo_record[max_undo].selEnd = (**gSrcWind_TEH).selEnd;
	strcpy(redo_record[max_undo].reason, the_reason);
	strcpy(redo_menu_name, "Redo ");
	strcat(redo_menu_name, the_reason);
	c2pstr(redo_menu_name);
	SetItem(myMenus[edmn_ID - menu_offset], edmn_redo, (StringPtr)redo_menu_name);
	redo_record[max_undo].byteCount = (**gSrcWind_TEH).teLength;
	redo_record[max_undo].isDirty = gSrcWind_dirty;
	memcpy(buffer, *(**gSrcWind_TEH).hText, (**gSrcWind_TEH).teLength);
	HUnlock(redo_record[max_undo].buf_handle);
} // support_redo



// ---------------------------------------------------------------------
// Show that we can undo previous typing
static void support_undo_key(void)
{
	if (undo_key_copied == 0)
	{
		support_undo("Typing", TRUE);
		undo_key_copied = 1;
	}
} // support_undo_key



// ---------------------------------------------------------------------
// Actually do the undo operation on the source file
static void undo_text()
{
	int i;
	char *buffer;
	if (undo_record[max_undo].reason[0])
	{
		support_redo(undo_record[max_undo].reason);
		(**gSrcWind_TEH).selStart = 0;
		(**gSrcWind_TEH).selEnd = (**gSrcWind_TEH).teLength;
		TEDelete(gSrcWind_TEH);
		HLock(undo_record[max_undo].buf_handle);
		buffer = *(undo_record[max_undo].buf_handle);
		TEInsert(buffer, undo_record[max_undo].byteCount, gSrcWind_TEH);
		undo_key_copied = 0;
		TESetSelect(undo_record[max_undo].selStart, undo_record[max_undo].selEnd, gSrcWind_TEH);
		gSrcWind_dirty = undo_record[max_undo].isDirty;
		InvalRect(&gSrcWind_Window->portRect);
		ShowSelect();

		/* dispose of the current undo record's data */
		DisposeHandle(undo_record[max_undo].buf_handle);

		/* move other undo records up the chain. */
		for (i = max_undo; i > 0; i--)
			undo_record[i] = undo_record[i-1];

		/* Delete the contents of the (duplicated) last record */
		undo_record[0].buf_handle = 0;
		undo_record[0].reason[0] = 0;

		/* show everyone what the next undoable item will be */
		strcpy(undo_menu_name, "Undo ");
		strcat(undo_menu_name, undo_record[max_undo].reason);
		c2pstr(undo_menu_name);
		SetItem(myMenus[edmn_ID - menu_offset], edmn_undo, (StringPtr)undo_menu_name);
	}
} // undo_text



// ---------------------------------------------------------------------
// Actually do the redo operation on the text file
static void redo_text()
{
	int i;
	char *buffer;
	if (redo_record[max_undo].reason[0])
	{
		support_undo(redo_record[max_undo].reason, false);
		(**gSrcWind_TEH).selStart = 0;
		(**gSrcWind_TEH).selEnd = (**gSrcWind_TEH).teLength;
		TEDelete(gSrcWind_TEH);
		HLock(redo_record[max_undo].buf_handle);
		buffer = *(redo_record[max_undo].buf_handle);
		TEInsert(buffer, redo_record[max_undo].byteCount, gSrcWind_TEH);
		undo_key_copied = 0;
		TESetSelect(redo_record[max_undo].selStart, redo_record[max_undo].selEnd, gSrcWind_TEH);
		gSrcWind_dirty = redo_record[max_undo].isDirty;
		ShowSelect();

		/* dispose of the current undo record's data */
		DisposeHandle(redo_record[max_undo].buf_handle);

		/* move other undo records up the chain. */
		for (i = max_undo; i > 0; i--)
			redo_record[i] = redo_record[i-1];

		/* Delete the contents of the (duplicated) last record */
		redo_record[0].buf_handle = 0;
		redo_record[0].reason[0] = 0;

		/* show everyone what the next redoable item will be */
		strcpy(redo_menu_name, "Redo ");
		strcat(redo_menu_name, redo_record[max_undo].reason);
		c2pstr(redo_menu_name);
		SetItem(myMenus[edmn_ID - menu_offset], edmn_redo, (StringPtr)redo_menu_name);
	}
} // redo_text



// ---------------------------------------------------------------------
// Display additional credits on POV-Ray startup
void PrintMacCredits(void)
{
	char	appVers[32];

	fprintf (stderr,"  This Macintosh implementation of POV-Ray is brought to you by:\n");
	fprintf (stderr,"       David Harr         Jim Nitchals       Eduard [esp] Schwan\n");

	fprintf (stderr,"  Macintosh Beta Testing by:\n");
	fprintf (stderr,"       Mark de Jong       Anton Raves\n");

	GetAppVersionPString(1, (StringPtr)appVers);
	p2cstr((StringPtr)appVers);
	fprintf (stderr,"\n");
	fprintf (stderr,"  POV-Ray version %s [%s]\n", appVers,COMPILER_VER);

	fprintf (stderr,"  ----------------------------------------------------------------------\n");
} // PrintMacCredits



// ---------------------------------------------------------------------
// This is what is really called if exit() is called
void catch_exit(int n)
{
	gRenderedOK = 0;
	// if error & in main file, then go to line #
	if ((n == 1) && (Include_File_Index == 0))
	{
		// Show source window, and hilite line #
		SelectWindow(gSrcWind_Window);
		goto_line(Token.Token_Line_No+1);
	}

	HaltFileQ();

	longjmp(gSetJmpEnv, 1);
} // catch_exit



// ---------------------------------------------------------------------
// Moves and sizes the window passed to fit the Rect passed.
// No checking is done to see that it fits on a screen.
void MoveSizeWindow(WindowPtr pWindow, Rect * pWindRect)
{
	// position & size the window
	MoveWindow(pWindow, pWindRect->left, pWindRect->top, false);
	// oh the grodiest hack for now... someday, all windows will be equal!
	// until then, we gotta put up with the skanky editor window.
	if (pWindow == gSrcWind_Window)
		MyResizeWindow(pWindow, pWindRect->right - pWindRect->left, pWindRect->bottom - pWindRect->top);
	else
		SizeWindow(pWindow, pWindRect->right - pWindRect->left, pWindRect->bottom - pWindRect->top, false);
} // MoveSizeWindow


// ---------------------------------------------------------------------
// positions and displays a new source window.
void OpenNewSourceWindow(void)
{
	// set up window title to be file name
	SetWTitle(gSrcWind_Window, gSrcWind_FileName);

	// Set Image window to saved position, shown later when render starts
	MoveSizeWindow(gImageWindowPtr, &(**gFilePrefs_h).imageWind_pos);

	// Set Source window position to saved position & show it
	MoveSizeWindow(gSrcWind_Window, &(**gFilePrefs_h).srcWind_pos);
	ShowWindow(gSrcWind_Window);

	// bring to front if splash screen not up
	if (!gSplashScreen)
		SelectWindow(gSrcWind_Window);

	gSrcWind_visible = TRUE;
	gSrcWind_dirty = 0;
	TESetSelect(0, 0, gSrcWind_TEH);
	ShowSelect();
} // OpenNewSourceWindow


// ---------------------------------------------------------------------
// Convert a vref/dirID into a magic vref (working dir)
static OSErr MyDirID2VrefNum(short vRefNum, long dirID, short *theWDVrefNumPtr)
{
	OSErr			anError;

	// Open a Working directory for ourselves.  Note that we leave
	// it open forever (just like SFGetFile does.)  This is supposed
	// to be kosher, the Finder is to clean up after us when we quit!?
	anError = OpenWD(vRefNum, dirID, kAppSignature, theWDVrefNumPtr);
	return anError;
} // MyDirID2VrefNum



// ---------------------------------------------------------------------
// General entry to open a source file for editing
static void OpenTextFile(Str255 fn, short vRef, long dirID, Boolean UseDirID)
{
	short	fRefNum, vRefNum;
	int		errcode = noErr;

	// close any existing file first
	CloseMyWindow();

	// If ODOC, find working dir from vref/dirID so we can use FSOpen etc.
	if (UseDirID)
	{
		// convert vref/dirID into WD vrefnum for later FSOpen and SetVol calls
		errcode = MyDirID2VrefNum(vRef, dirID, &vRefNum);
	}
	else
	{
		vRefNum = vRef;
	}

	if (errcode==noErr)
		errcode = FSOpen(fn, vRefNum, &fRefNum);
	if (errcode==noErr)
	{
		flush_undo_system();
		errcode = ReadFile(fRefNum, gSrcWind_TEH);
		FSClose(fRefNum);	/* and ignore close errors */
		if (errcode == 999)
			FileError("\pFile too large to be edited: ", fn);
		if (errcode == 1) 
		{
			pStrCopy(fn, gSrcWind_FileName);
			gSrcWind_VRefNum = vRefNum;
			GetFilePrefs();
			OpenNewSourceWindow();
		}
	}
	else
		FileError("\pError opening ", fn);

} // OpenTextFile



// ---------------------------------------------------------------------
// Save the status window text into a new text file
static int Save_StatusWindow(void)
{
	OSErr	ioError;
	char	hstate;
	short	vRef = 0;
	short	refNum;
	char	fn[] = "\pPOV-Ray Status.Window";

	SetCursor(&gWaitCursor); // could take a little while..

	ioError = FSDelete((StringPtr)fn, vRef); 
	ioError = Create((StringPtr)fn, vRef, 'ttxt', 'TEXT');
	if ((ioError == noErr) || (ioError == dupFNErr))
		ioError = FSOpen((StringPtr)fn, vRef, &refNum);
	if (ioError)
	{
		SetCursor(&qd.arrow);
		FileError("\pError creating Status file ", (StringPtr)fn);
		return (0);
	}
	else
	{
		hstate = HGetState((**gp2wWindow->p2wTEHandle).hText);
		HLock((**gp2wWindow->p2wTEHandle).hText);

		if (WriteFile(refNum, (*(**gp2wWindow->p2wTEHandle).hText),
						(long)(**gp2wWindow->p2wTEHandle).teLength))
			FileError("\pError writing file ", (StringPtr)fn);

		HSetState((**gp2wWindow->p2wTEHandle).hText, hstate);

		FSClose(refNum);

		SetCursor(&qd.arrow); // all done
		return(1);
	}
} // Save_StatusWindow



// ---------------------------------------------------------------------
// Done rendering, beep if in background, or auto-shutdown if requested
static void notify_user(void)
{
	Boolean		doFeedback = false;
	Boolean		doBeep = false;
	Boolean		doDialog = false;

	if (gRenderedOK)
		SetCustomPalette(!gInBackground);

	/* If the image rendered OK and shutdown was requested, then shut down. */
	if (gAutoShutdown  &&  gRenderedOK)
	{
		Save_StatusWindow();	/* save status window too */
		DoShutdownMac();
	}
	else
	{ // check for feedback

		// are we going to give user feedback?
		switch ((**gAppPrefs_h).whenToNotify)
		{
			case eWhenNtf_Quiet:	// don't notify at all when done
				break;
			case eWhenNtf_BgOnly:	// notify when done, only if in background
				doFeedback = gInBackground;
				break;
			case eWhenNtf_FgOnly:	// notify when done, only if in foreground
				doFeedback = !gInBackground;
				break;
			case eWhenNtf_BgFg:		// notify when done, in background or foreground
				doFeedback = true;
				break;
		} // switch

		// do the beep, dialog, or both?
		if (doFeedback)
		{
			switch ((**gAppPrefs_h).howToNotify)
			{
				case eHowNtf_Noise:	// do noise
					doBeep = true;
					break;
				case eHowNtf_Dlg:	// do dialog
					doDialog = true;
					break;
				case eHowNtf_NoiseDlg:	// do noise & dialog
					doBeep = true;
					doDialog = true;
					break;
			} // switch
		} // if doFeedback
	} // else check for feedback

	// get everyone's attention...
	if (doBeep)
		PlayNotifySound();
	if (doDialog)
		ShowNotifyDialog();

} // notify_user



// ---------------------------------------------------------------------
// Get and display the intro splash screen
static void CreateSplashScreen()
{
	char	povVers[16];
	gSplashStartTicks = 0;
	gSplashScreen = GetNewDialog(138, NULL, (WindowPtr) -1);
	if (gSplashScreen != NULL)
	{
		gSplashStartTicks = TICKS;
		strcpy(povVers, POV_RAY_VERSION);
		c2pstr(povVers);

		ParamText((StringPtr)povVers, gDistMessage, "\p", "\p");
		SetPort((GrafPtr)gSplashScreen);

		TextFont(times);
		TextSize(12);

		PositionWindow(gSplashScreen, ewcDoCentering, eDeepestDevice, (WindowPtr)gp2wWindow);
		ShowWindow(gSplashScreen);
		SelectWindow(gSplashScreen);
		DrawDialog(gSplashScreen);
	}
}



// ---------------------------------------------------------------------
// close and dispose of the intro splash screen
static void KillSplashScreen(void)
{
	if (gSplashScreen)
		DisposeDialog(gSplashScreen);
	gSplashScreen = NULL;
	gSplashStartTicks = 0;

	// bring source window to front now
	if (gSrcWind_Window)
		SelectWindow(gSrcWind_Window);

} // KillSplashScreen



// ---------------------------------------------------------------------
// create and display a new empty source window
static void DoFile_New(void)
{
	// close any existing file
	CloseMyWindow();

	// set up untitled
	SetUpFiles();

	// new file gets default prefs
	**gFilePrefs_h = **gDefltFilePrefs_h;

	OpenNewSourceWindow();

} // DoFile_New



// ---------------------------------------------------------------------
// Prompt user for a source file, and open it into source window
static void DoFile_Open(void)
{
	short	vRef = 0;
	Str255 	fn;

	if (OldFile(fn, &vRef))
	{
		OpenTextFile(fn, vRef, 0L, false/*!UseDirID*/);	
	}
} // DoFile_Open



// ---------------------------------------------------------------------
// Set up, call the renderer, and notify user when done
static void DoRendering(void)
{
	Boolean	stillDoingAnimation;
	long	start_ticks;
	short	animFrameNumber;

	if (gSrcWind_dirty)
		if (DoFile(fmn_save))
			;

	gInAnimationLoop = (**gPrefs2Use_h).doAnimation;
	gAnimRec = (**gPrefs2Use_h).animRec;
	SetCurrFrameVal(&gAnimRec, gAnimRec.frameValS);
	stillDoingAnimation = gInAnimationLoop;

	do	{
		SetupRenderArgs();

		if ( stillDoingAnimation && ((**gPrefs2Use_h).progress >= kProgMinimal) )
		{
			printf("-- [Animation] StartFrame=%d,  EndFrame=%d, CurrFrame=%d, CurrClock=%g)\n",
				gAnimRec.frameValS, gAnimRec.frameValE, GetCurrFrameVal(), GetCurrClockVal());
		}

		start_ticks = TICKS;

		// pull status window to front (not user-friendly, but user-requested!)
		SelectWindow((WindowPtr)gp2wWindow);

		// Do the real POV-Ray work now
		call_main(ARGC, ARGV);

		// Give a quick breath after each render
		Cooperate(true);

		// write PICT file with current name.PICT
		if	(
			(gAutoSave || gAutoShutdown || gInAnimationLoop || gDoingBatchODOCs)
			&&  gRenderedOK && !Stop_Flag && !gQuit
			)
		{
			// set up frame # suffix (if any)
			if (gInAnimationLoop)
				animFrameNumber = GetCurrFrameVal();
			else
				animFrameNumber = kNoAnimSuffix;
			// save the image
			SaveOutputFile(false, animFrameNumber, gtheSCComponent);
		}

		// next frame
		if (gInAnimationLoop)
			stillDoingAnimation = IncToNextFrame(&gAnimRec);
	} while (stillDoingAnimation && !Stop_Flag && !gQuit);

	gInAnimationLoop = false;

	/*
	If POV wasn't prematurely stopped, and if it is not in the middle of multiple renders,
	then call the notification procedure.
	*/
	if	(!Stop_Flag && (FileQ_NumItems() == 0))
		notify_user();

	gBeginRendering = false; // all done for now
} // DoRendering



// ---------------------------------------------------------------------
// Open a source file, render it, save the image PICT, and close the source file
static void BatchProcessOneFile(FSSpec	*pFile)
{
	gDoingBatchODOCs = true; // postpone other ODOCs until done with this one!

	OpenTextFile(pFile->name, pFile->vRefNum, pFile->parID, true/*UseDirID*/);
	DoRendering();
	DoFile(fmn_close);

	gDoingBatchODOCs = false;
} // BatchProcessOneFile



// ---------------------------------------------------------------------
// Handle choosing items from the File menu
static void DoFileMenu(short theItem)
{
	WindowPtr	theFrontWindow;

	theFrontWindow = FrontWindow();
	switch (theItem)
	{
		case fmn_new:								/* New File */
				DoFile_New();
				break;

		case fmn_open:						 		/* Open a file */
				DoFile_Open();
				break;

		case fmn_close:								/* Close */
				if (theFrontWindow == gImageWindowPtr)
					CloseImageWindow();
				else if (theFrontWindow == gSrcWind_Window)
					DoFile(fmn_close);
				break;

		case fmn_save:							/* Save */
				if (theFrontWindow == gSrcWind_Window)
				{	
					if (DoFile(fmn_save))
						WriteFilePrefs();							
				}						
				else if (theFrontWindow == gImageWindowPtr)
						SaveOutputFile(false, kNoAnimSuffix, gtheSCComponent); // don't prompt for name
				else if (theFrontWindow == (WindowPtr)gp2wWindow)
					Save_StatusWindow();
				break;

		case fmn_saveas:							/* Save asÉ */
				if (theFrontWindow == gSrcWind_Window)
				{	
					if (DoFile(fmn_saveas))
					{
						// this will have no settings, so write existing file prefs to it
						WriteFilePrefs();							
					}
				}
				else if (theFrontWindow == gImageWindowPtr)
					SaveOutputFile(true, kNoAnimSuffix, gtheSCComponent); // prompt for name
				else if (theFrontWindow == (WindowPtr)gp2wWindow)
					Save_StatusWindow();
				break;

		case fmn_quit:
				ask_about_quit();
				if (gQuit)
					if (DoFile(fmn_close))
						;
				break;	/* Quit the program */
	}
} // DoFileMenu



// ---------------------------------------------------------------------
// Handle choosing items from the Edit menu
static void DoEditMenu(short theItem)
{
	WindowPtr	theFrontWindow;

	theFrontWindow = FrontWindow();

	if (SystemEdit(theItem-1) != 0)
		return;

	/* handle the menu items for the front window */
		switch (theItem)
		{
			case edmn_undo:					/*undo*/
				if (theFrontWindow == gSrcWind_Window)
				{
					undo_text();
					AdjustText();
				}
				else if (theFrontWindow == gImageWindowPtr)
				{
					if (gCanUndo)
						undo_image();
				}
				break;

			case edmn_cut:
				support_undo("Cut", TRUE);
				ZeroScrap(); // clear out the clipboard
				TECut(gSrcWind_TEH);
				TEToScrap(); // export
				AdjustText();
				gSrcWind_dirty = 1;
				break;

			case edmn_copy:					/*copy*/
				if (theFrontWindow == gSrcWind_Window)
				{
					ZeroScrap(); // clear out the clipboard
					TECopy(gSrcWind_TEH);
					TEToScrap(); // export
				}
				else if (theFrontWindow == gImageWindowPtr)
				{
					ZeroScrap(); // clear out the clipboard
					paint_to_picture(false);
					TEToScrap(); // export
				}
				else if (theFrontWindow == (WindowPtr)gp2wWindow)
				{
					ZeroScrap(); // clear out the clipboard
					TECopy(gp2wWindow->p2wTEHandle);
					TEToScrap(); // export
				}
				break;
	
			case edmn_paste:
				support_undo("Paste", TRUE);
				TEFromScrap(); // import
				TEPaste(gSrcWind_TEH);
				AdjustText();
				gSrcWind_dirty = 1;
				break;

			case edmn_clear:
				support_undo("Clear", TRUE);
				TEDelete(gSrcWind_TEH);
				AdjustText();
				gSrcWind_dirty = 1;
				break;

			case edmn_selectAll:
				if (theFrontWindow == gSrcWind_Window)
				{
					SelectAllText();
				}
				else if (theFrontWindow == (WindowPtr)gp2wWindow)
				{
					p2w_SelectAll(gp2wWindow);
				}
				break;

			case edmn_redo:
				redo_text();
				AdjustText();
				break;

			case edmn_goto:
				choose_goto_line();
				break;

			case edmn_lookup:
				DoLookup();
				break;

			case edmn_prefs:
				ChangeAppPrefs();
				break;
		} /*switch*/
} // DoEditMenu



// ---------------------------------------------------------------------
// Handle choosing items from various menus
static void DoCommand(long m)
{
	short		theMenu, theItem;
	Str255		name;

	theMenu = (m >> 16);
	theItem = m;
	switch (theMenu)
	{
		case apmn_ID:				/* Apple Menu */
			if (theItem == apmn_about)
				AboutPOV();
			else
			{
				GetItem(myMenus[apmn_ID - menu_offset], theItem, name);
				OpenDeskAcc(name);
			}
			break;
			
		case fmn_ID:				/* File Menu */
			DoFileMenu(theItem);
			break;

		case edmn_ID:				/* Edit Menu */
			DoEditMenu(theItem);
			break;

		case immn_ID:				/* Image Menu */
			if (theItem == immn_dither)
			{
				// toggle it
				(**gPrefs2Use_h).doDither = !(**gFilePrefs_h).doDither;
				// make sure file prefs are updated too (in case using app prefs)
				(**gFilePrefs_h).doDither = (**gPrefs2Use_h).doDither;
				InvalRect_ImageWindow(false);
				break;
			}
			break;

		case viewmn_ID:				/* View submenu */
			(**gFilePrefs_h).imageMagFactor = theItem;
			// make sure current is same as file (in case using app prefs)
			(**gPrefs2Use_h).imageMagFactor = (**gFilePrefs_h).imageMagFactor;
			SetImageWindowMag(theItem);
			break;

		case plmn_ID:				/* Custom Palette submenu */
			switch(theItem)
			{
				case palette_none:
					use_custom_palette = false;
					gColorQuantMethod = -1;
					break;

				case palette_default:
					use_custom_palette = true;
					gColorQuantMethod = systemMethod;
					break;

				case palette_median:
					use_custom_palette = true;
					gColorQuantMethod = medianMethod;
					break;

				case palette_popular:
					use_custom_palette = true;
					gColorQuantMethod = popularMethod;
					break;

				case palette_var_min:
					use_custom_palette = true;
					gColorQuantMethod = varianceMethod;
					break;

				case palette_octree:
					use_custom_palette = true;
					gColorQuantMethod = octreeMethod;
					break;
			}

			if (((WindowPeek)gImageWindowPtr)->visible)
				SetCustomPalette(!gInBackground);
			break;

		case rnmn_ID:				/* Render Menu */
			switch(theItem)
			{
				case rnmn_options:				 			/* Change options */
						if (gSrcWind_dirty)
						{
							if (DoFile(fmn_save))
								ChangeFilePrefs();
						}
						else
							ChangeFilePrefs();
						break;
				case rnmn_render:							/* Render */
						if (gSrcWind_dirty)
							if (DoFile(fmn_save))
								;
						gBeginRendering = true;
						break;

				case rnmn_pause:							/* pause trace */
						pause_it();
						break;	

				case rnmn_stop:								/* Abort trace in progress? */
						ask_about_stop();
						break;		

				case rnmn_autosave:
						gAutoSave = !gAutoSave;
						break;

				case rnmn_shutdown:
						gAutoShutdown = !gAutoShutdown;
						// display auto shutdown warning dialog
						if (gDoingRender && gAutoShutdown)
							(void)displayDialog(148, NULL, 0, ewcDoCentering, eSameAsPassedWindow); // power off warning
						break;
			}
			break;

		case psmn_ID:				/* Processing Menu */
			switch (theItem)
			{
				case psmn_border:
					draw_border();
					break;

				case psmn_darken:
					darken_image();
					break;

				case psmn_lighten:
					lighten_image();
					break;

				case psmn_reduceC:
					reduce_contrast();
					break;

				case psmn_increaseC:
					increase_contrast();
					break;

				case psmn_invert:
					invert_image();
					break;

				case psmn_revert:
					revert_image();
					break;
			}
			break;

		case wndmn_ID: 				/* Windows menu */
			switch (theItem)
			{
				case 1: if (gp2wWindow)
						{
							SelectWindow((WindowPtr)gp2wWindow);
						}
						break;

				case 2: if ((gSrcWind_Window) && (gSrcWind_visible))
						{
							SelectWindow(gSrcWind_Window);
						}
						break;

				case 3: if (((WindowPeek)gImageWindowPtr)->visible)
						{
							SelectWindow(gImageWindowPtr);
						}
						break;
			}
			break;

		default:
			if ((theMenu >= macmn_sub_ID) && (theMenu <= macmn_sub_ID+NUM_MACRO_MENUS-1))
			{
				HandleTemplateMenu(theMenu, theItem);
				AdjustText();
			}
			break;
	}

	HiliteMenu(0);

} // DoCommand



// ---------------------------------------------------------------------
// Handle mouse down in windows or menus
static void DoMouseDown(void)
{
	short			code;
	WindowPtr		whichWindow;
	Rect			theWindowPos;	

	code = FindWindow(gTheEvent.where, &whichWindow);
	if (whichWindow == gSrcWind_Window)
	{
		DoEditMouseDown (code, whichWindow, &gTheEvent);
	}
	else
	{
		switch (code)
		{
			case inMenuBar:
				SetCursor(&qd.arrow);
				AdjustMenus();
				DoCommand(MenuSelect(gTheEvent.where));
				break;

			case inSysWindow:
				SystemClick(&gTheEvent, whichWindow);
				break;

			case inGoAway:
				// neither Image nor Status have a close box, and the
				// Source window's close is handled in DoEditMouseDown()
				if (TrackGoAway(whichWindow, gTheEvent.where))
					;
				break;

			case inGrow:
				if ((whichWindow == (WindowPtr)gp2wWindow) && (gp2wWindow))
				{ // Status Window
					p2w_DoGrow(gp2wWindow, &gTheEvent);
					GetGlobalWindowRect((WindowPtr)gp2wWindow, &theWindowPos);
					(**gFilePrefs_h).statWind_pos = theWindowPos;
					// also remember this as default for next time
					(**gDefltFilePrefs_h).statWind_pos = theWindowPos;
				}
				else if ((whichWindow == gImageWindowPtr) && (gImageWindowPtr))
				{ // Image Window
					DoGrowImageWindow(gImageWindowPtr, gTheEvent.where);
					GetGlobalWindowRect(gImageWindowPtr, &theWindowPos);
					(**gFilePrefs_h).imageWind_pos = theWindowPos;
// Uncomment the next line to keep updating the default
// image wind position, not a good idea, IMHO.
/*
					// also remember this as default for next time
					(**gDefltFilePrefs_h).imageWind_pos = theWindowPos;
*/
				}
				break;

			case inZoomIn:
			case inZoomOut:
				SelectWindow(whichWindow);
				if (TrackBox(whichWindow, gTheEvent.where, code))
				{
					if ((whichWindow == (WindowPtr)gp2wWindow) && (gp2wWindow))
						p2w_DoZoom(gp2wWindow, code);
					else if ((whichWindow == gImageWindowPtr) && (gImageWindowPtr))
						; // none
				}
				break;

			case inDrag:
				SelectWindow(whichWindow);
				DragWindow(whichWindow, gTheEvent.where, &gDragBounds);
				GetGlobalWindowRect(whichWindow, &theWindowPos);
				if ((whichWindow == (WindowPtr)gp2wWindow) && (gp2wWindow))
				{ // Status window
					(**gFilePrefs_h).statWind_pos = theWindowPos;
					// also remember this as default for next time
					(**gDefltFilePrefs_h).statWind_pos = theWindowPos;
				}
				else if ((whichWindow == gImageWindowPtr) && (gImageWindowPtr))
				{ // Image Window
					(**gFilePrefs_h).imageWind_pos = theWindowPos;
// Uncomment the next line to keep updating the default
// image wind position, not a good idea, IMHO.
/*
					// also remember this as default for next time
					(**gDefltFilePrefs_h).imageWind_pos = theWindowPos;
*/
				}
				break;

			case inContent:
				if (whichWindow != FrontWindow())
					SelectWindow(whichWindow);
				else
				{
					if ((whichWindow == (WindowPtr)gp2wWindow) && (gp2wWindow))
						p2w_DoContentClick(gp2wWindow, &gTheEvent);
					else
					if (whichWindow == gImageWindowPtr)
					{
						SetPort(whichWindow);
						GlobalToLocal(&gTheEvent.where);
						// perform marquee dragging in here!
					}
				}
				break;
		}
	}
} // DoMouseDown



// ---------------------------------------------------------------------
// Handle key downs
static void DoKeyDown(void)
{
	char	theChar, theVirtualCode;

	theChar = gTheEvent.message & charCodeMask;
	theVirtualCode = (gTheEvent.message & keyCodeMask) >> 8;

	if (gTheEvent.modifiers & cmdKey)
	{
		AdjustMenus();
		DoCommand(MenuKey(theChar));
	}
	else
	{
		if (FrontWindow() == gSrcWind_Window)
		{
			switch (theVirtualCode)
			{
				case 0x33:				/* delete */
					if ((**gSrcWind_TEH).selStart != (**gSrcWind_TEH).selEnd)
					{
						support_undo("Delete", TRUE);
						TEDelete(gSrcWind_TEH);	/* delete a range */
					}
					else
					{
						support_undo_key();
						TEKey(theChar, gSrcWind_TEH); /* delete char */
					}
					gSrcWind_dirty = 1;
					AdjustText();
					break;

				case 0x73:				/* HOME */
					undo_key_copied = 0;
					SetCtlValue(gSrcWind_VScroll, 0);
					AdjustText();
					break;

				case 0x77:				/* END */
					undo_key_copied = 0;
					SetCtlValue(gSrcWind_VScroll, (**gSrcWind_TEH).teLength);
					AdjustText();
					break;

				case 0x74:				/* PAGE UP */
					undo_key_copied = 0;
					ScrollProc(gSrcWind_VScroll, inPageUp);
					AdjustText();
					break;

				case 0x79:				/* PAGE DOWN */
					undo_key_copied = 0;
					ScrollProc(gSrcWind_VScroll, inPageDown);
					AdjustText();
					break;

				case 0x7A:				/* UNDO (F1) */
					undo_text();
					break;

				case 0x78:				/* CUT (F2) */
					support_undo("Cut", TRUE);
					TECut(gSrcWind_TEH);
					AdjustText();
					gSrcWind_dirty = 1;
					break;

				case 0x63:				/* COPY (F3) */
					TECopy(gSrcWind_TEH);
					break;

				case 0x76:				/* PASTE (F4) */
					support_undo("Paste", TRUE);
					TEPaste(gSrcWind_TEH);
					AdjustText();
					gSrcWind_dirty = 1;
					break;
				
				case 0x75:				/* del >x> */
					if ((**gSrcWind_TEH).selStart != (**gSrcWind_TEH).selEnd)
					{
						support_undo("Delete", TRUE);
						TEDelete(gSrcWind_TEH);
						AdjustText();
					}
					else
					{   /* cursor-right then delete */
						support_undo_key();
						TEKey(0x1D, gSrcWind_TEH); 
						TEKey(0x08, gSrcWind_TEH);
					}
					gSrcWind_dirty = 1;
					break;

				case 0x1C:				/* Cursor keys, don't dirty the buffer or require undo */
				case 0x1D:
				case 0x1E:
				case 0x1F:
				case 0x7B:	/* arrow keys? */
				case 0x7C:
				case 0x7D:
				case 0x7E:
					undo_key_copied = 0;
					TEKey(theChar, gSrcWind_TEH);
					ShowSelect();
					break;

				default:				/* all other keys */
					support_undo_key();
					TEKey(theChar, gSrcWind_TEH);
					gSrcWind_dirty = 1;
					ShowSelect();
			}
		}
	}			
} // DoKeyDown



// ---------------------------------------------------------------------
// Handle disk inserted events (format uninit. disks if needed)
static void DoDiskEvt(EventRecord * pTheEventPtr)
{
	OSErr	anError;
	Point	aPoint;

	if (HiWord(pTheEventPtr->message) != noErr)
	{	// prompt to format it
		GetBestDialogPos(&aPoint);
		anError = DIBadMount(aPoint, pTheEventPtr->message);
	}
} // DoDiskEvt



// ---------------------------------------------------------------------
// Handle window update events
static void DoUpdateEvt(void)
{
	WindowPtr	wnd;
	
	wnd = (WindowPtr) gTheEvent.message;
	if ((wnd == gSrcWind_Window) && gSrcWind_Window)
		UpdateWindow (wnd);
	else
	if ((wnd == (WindowPtr) gp2wWindow) && gp2wWindow)
	{
		p2w_DoUpdate(gp2wWindow);
	}
	else
	{	/* update the image window */
		if ((wnd == gImageWindowPtr) && gImageWindowPtr)
		{
			UpdateImageWindow();
		}
	}
} // DoUpdateEvt



// ---------------------------------------------------------------------
// Handle window activate events
static void DoActivateEvt(void)
{
	HandleActivate((WindowPtr) gTheEvent.message, gTheEvent.modifiers & 1);
} // DoActivateEvt



// ---------------------------------------------------------------------
// Handle OS Events (Suspend/resume)
static void DoOSEvt(void)
{
	if (gTheEvent.message >> 24 == 1)
	{
		if (gTheEvent.message & 1)
		{
			gInBackground = false;
			HandleActivate((WindowPtr) gTheEvent.message, 1);
		}
		else
		{
			gInBackground = true;
			HandleActivate((WindowPtr) gTheEvent.message, 0);
		}
		SetCursor(&qd.arrow);
	}
} // DoOSEvt



// ---------------------------------------------------------------------
// Handle any high level events (AppleEvents currently)
static void DoHighLevelEvt(void)
{
	short i;

	if ((**gPrefs2Use_h).progress >= kProgDebug)
		printf("-d HighLvlEvtClass='%4s'\n", &gTheEvent.message);
	if (gTheEvent.message == kCoreEventClass)
		i = AEProcessAppleEvent(&gTheEvent);
	/* AppleEvents are currently the only supported high level events */
} // DoHighLevelEvt



// ---------------------------------------------------------------------
// Do everything you ever need to do in the guts of a Mac event loop
static void CheckForEvents(void)
{
	FSSpec	aFile;

	// Stuff that only gets done if we're in front.
	if (!gInBackground)
	{
		if (gSrcWind_Window)
		{
			SetPort(gSrcWind_Window);
			MaintainCursor();
			if (gSrcWind_TEH)
				TEIdle(gSrcWind_TEH);
		}
	}

	// Check for timeout on startup message.  Remove the dialog after timeout.
	if (gSplashScreen) // if screen still up..
		if	((gSplashStartTicks + 8*60L) < TICKS)	// timeout after 8 seconds
		{
			KillSplashScreen();
		}

	// take care of next drag-n-dropped file
	if (!gDoingBatchODOCs)
		if (FileQ_NumItems() > 0)
			if (FileQ_Get(&aFile))
				BatchProcessOneFile(&aFile);

	// get next event
	if (gDoingRender)
	{
		// Rendering, don't go away for long
		WaitNextEvent(everyEvent, &gTheEvent, gWNEReleaseTicks, NULL);
	}
	else
	{
		// idle, let other apps breathe more
		WaitNextEvent(everyEvent, &gTheEvent, 30, NULL);
	}

	// handle the event
	switch (gTheEvent.what)
	{
		case mouseDown:		
					// If the startup screen is still up, kill it
					if (gSplashScreen)
						KillSplashScreen();
					DoMouseDown();
					break;

		case keyDown:
		case autoKey: 
					// If the startup screen is still up, kill it
					if (gSplashScreen)
						KillSplashScreen();
					DoKeyDown(); 
					break;

		case diskEvt:
					DoDiskEvt(&gTheEvent);
					break;

		case updateEvt:
					DoUpdateEvt();
					break;

		case activateEvt:
					DoActivateEvt();
					break;

		case osEvt:	// suspend/resume, mousemoved..
					DoOSEvt();
					break;

		case kHighLevelEvent:	// AppleEvents
					DoHighLevelEvt();
					break;

		case nullEvent:
		default:
					// DoNullEvt();
					break;
	}
} // CheckForEvents



// ---------------------------------------------------------------------
// Called from rendering engine, allows Mac code to breathe while the
// renderer is parsing and rendering.  This keeps menus working..
// doImmediate if true will always breathe, otherwise it breathes every N calls
void Cooperate(int doImmediate)
{
	if ( doImmediate || (TICKS > (gPrevTickCount+((gCpuHogginess-1)<<2))) )
	{
		// Update the clock before other applications
		// have breathed, so our time slices are more equally spaced
		gPrevTickCount = TICKS;

		// Now let other apps breathe
		CheckForEvents();
	}
} // Cooperate


// Silly Wabbit, programming trix are for twids!
// MPW C and Symantec C define lomem globals ever-so-slightly
// differently, so we have to special case them here.

#if defined(THINK_C)
#define CURDIRSTORE_LOC		(CurDirStore)
#define SFSAVEDISK_LOC		(SFSaveDisk)
#else
#define CURDIRSTORE_LOC		(*(long int*)CurDirStore)
#define SFSAVEDISK_LOC		(*(short int*)SFSaveDisk)
#endif

// ---------------------------------------------------------------------
// Set default SF Getfile dialog initial directory
static void SetSFCurrent(FSSpecPtr pTheFSFilePtr)
{
	OSErr		anError = noErr;

	// Standard File wants the dirID and the negated vRefnum..
	// so give it to 'em that way
#if defined(__powerc)
	/* do it the new PowerPC way */
	LMSetCurDirStore(pTheFSFilePtr->parID);
	LMSetSFSaveDisk(-(pTheFSFilePtr->vRefNum));
#else
	CURDIRSTORE_LOC		= pTheFSFilePtr->parID;
	SFSAVEDISK_LOC		= -(pTheFSFilePtr->vRefNum);
#endif // __powerc

} // SetSFCurrent



// ---------------------------------------------------------------------
// Returns TRUE if all required AE parms were extracted OK
static OSErr GotRequiredAEParams(AppleEvent	*theAppleEvent)
{
	DescType	typeCode;
	Size		actualSize;
	OSErr		err;

	err = AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr, typeWildCard,
							&typeCode, NULL, 0, &actualSize);	/* NULL ok here; need only function result */
	if (err == errAEDescNotFound)
		// we got all the required params: all is ok
		return noErr;
	else if (err == noErr)
		// found one left behind: error!
		return errAEEventNotHandled;
	else
		// some other oddity
		return err;
} // GotRequiredAEParams



// ---------------------------------------------------------------------
// AE Handler - Oapp - If app run with no documents
static pascal OSErr HandleAEOapp(AEDescList *aevt, AEDescList *reply, long refCon)
{
#pragma unused (reply,refCon)
	OSErr	anError;

	/* We don't normally expect any parms, but check in case the client requires any */
	anError = GotRequiredAEParams(aevt);

	// if user hasn't already opened a window, open a new untitled window
	if (!anError && (gSrcWind_VRefNum == 0))
		DoFile_New();

	return anError;
} // HandleAEOApp



// ---------------------------------------------------------------------
// AE Handler - Odoc - If app run with some documents, or if docs are
// dropped onto app after it is running
static pascal OSErr HandleAEOdoc(AEDescList *aevt, AEDescList *reply, long refCon)
{
#pragma unused (reply,refCon)
	OSErr		anError;
	long		numFiles;
	long		index;
	long		actualSize;
	DescType	actualType;
	FInfo		theFileInfo;
	AEDesc		fileListDesc;
	AEKeyword	actualKeyword;
	FSSpec		theFSSpec;
	char		theFname[64];
						
	/* The "odoc" and "pdoc" messages contain a list of aliases as the direct paramater.	*/
	/* This means that we'll need to extract the list, count the list's elements, and		*/
	/* then process each file in turn.	Return any errors to the client.					*/

	/* Extract the list of aliases into fileListDesc */
	anError = AEGetParamDesc(aevt, keyDirectObject, typeAEList, &fileListDesc);

	/* Make sure that's all we're supposed to do */
	if (!anError)
		anError = GotRequiredAEParams(aevt);
		
	/* Count the list elements */
	if (!anError)
		anError = AECountItems(&fileListDesc, &numFiles);

	/* now get each file from the list and process it. */
	/* Even though the event contains a list of aliases, the Apple Event Manager */
	/* will convert each alias to an FSSpec if we ask it to. */
	for (index = 1; (index <= numFiles) && !anError; index++)
	{
		/* Pull the Nth file out of the aevt list */
		anError = AEGetNthPtr( &fileListDesc, index, typeFSS, &actualKeyword,
							&actualType, (Ptr)&theFSSpec, sizeof(theFSSpec), &actualSize);

		// C version of filename
		BlockMove(theFSSpec.name, theFname, theFSSpec.name[0]+1);
		p2cstr((StringPtr)theFname);

		if ((**gDefltFilePrefs_h).progress >= kProgDebug)
		{
			printf("-d ODOC='%s'\n", theFname);
		}

		// better be a text file..
		// Note: We're in an AE Handler, so guaranteed to be System 7, so ok to call:
		FSpGetFInfo(&theFSSpec, &theFileInfo);
		if (theFileInfo.fdType != 'TEXT')
		{
			(void)displayDialog(kdlog_CantOpenNonText,
							theFname, 0, ewcDoCentering, eSameAsPassedWindow);
			anError = 1;
			break; // drop out of for loop..
		}

		/* Open the File here */
		if (!anError)
		{
			// Use this file to set anchor for any future SFGetFile opens...
			// in other words, if you drop one or more files onto POV-Ray, then
			// the next time you do a File-Open, the standard file dialog will
			// open at the directory of your dropped file, not the app directory.
			// (You're welcome Anton! [esp] :-)
			if (index == 1)
				SetSFCurrent(&theFSSpec);

			if ((**gDefltFilePrefs_h).progress >= kProgDebug)
			{
				printf("-d  NumFiles=%d,  DoingBatch=%d,  DoingRender=%d\n",
						(int)numFiles, gDoingBatchODOCs, gDoingRender);
			}
	
			if (gDoingBatchODOCs)
			{	// already queued some, queue these too
				FileQ_Put(&theFSSpec);
			}
			else
			{
				// Don't open it if a file is already open and dirty (unsaved)
				if (gSrcWind_dirty)
				{
					(void)displayDialog(kdlog_CantOpenOverDirty,
									NULL, 0, ewcDoCentering, eSameAsPassedWindow);
					break; // no more files..
				}
				else
				{	// ok to add
					if ((numFiles == 1) && !gDoingRender)
					{
						// Simple case, just one file to open, just open it and wait
						OpenTextFile(theFSSpec.name, theFSSpec.vRefNum, theFSSpec.parID, true/*UseDirID*/);
					}
					else
					{
						// mult. files dropped, or already rendering one.
						// put each file in a queue for later batch processing
						FileQ_Put(&theFSSpec);

						// This is a sly way to tell DoRendering() to save the current
						// file being rendered automatically, before opening the next
						// file in the queue.
						if (gDoingRender)
							gDoingBatchODOCs = true;
					}
				}	// else ok to add
			}
		}	// if !error
	}	// for

	/* All done with the AE list, throw it away */
	if (!anError)
		anError = AEDisposeDesc(&fileListDesc);

	return anError;
} // HandleAEOdoc



// ---------------------------------------------------------------------
// AE Handler - Quit - Ask app to quit cleanly
static pascal OSErr HandleAEQuit(AEDescList *aevt, AEDescList *reply, long refCon)
{
#pragma unused (reply,refCon)

	OSErr	anError;

	/* We don't normally expect any parms, but check in case the client requires any */
	anError = GotRequiredAEParams(aevt);

	Stop_Flag = gQuit = TRUE;
	return anError;
} // HandleAEQuit



// ---------------------------------------------------------------------
// Install our AppleEvent handlers (above) so they can be called automatically
// by AEProcessAppleEvent()
static void InstallAppleEvents(void)
{

#if defined(__powerc)
	(void)AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, 
						(EventHandlerUPP)&gAEOAppRD, 0, false);
	(void)AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,   
						(EventHandlerUPP)&gAEODocRD, 0, false);
	/* handle "Print" as Open */
	(void)AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,  
						(EventHandlerUPP)&gAEODocRD, 0, false);
	(void)AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, 
						(EventHandlerUPP)&gAEQuitRD, 0, false);
#else
	(void)AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, 
						(EventHandlerProcPtr)HandleAEOapp, 0, false);
	(void)AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,   
						(EventHandlerProcPtr)HandleAEOdoc, 0, false);
	/* handle "Print" as Open */
	(void)AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,  
						(EventHandlerProcPtr)HandleAEOdoc, 0, false);
	(void)AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, 
						(EventHandlerProcPtr)HandleAEQuit, 0, false);
#endif //__powerc
} // InstallAppleEvents



// ---------------------------------------------------------------------
// Do any necessary cleanup prior to exiting
void exit_handler(void)
{
	dispose_virtual();		/* close virtual file */

	KillImageWindow();

	KillOffscreen();

	FileQ_d();	// Tear down ODOC File Queue

	KillTemplateMenu();

	if (gSrcWind_VRefNum != 0)	// if there is a file open..
	{
		WriteFilePrefs();
	}

	if (gAppPrefs_h)
		WriteAppPrefs();

	if (gtheSCComponent != NULL)
	{
		CloseComponent(gtheSCComponent);
		gtheSCComponent = NULL;
	}

	if (gp2wWindow)
	{
		p2w_DisposeWindow(gp2wWindow);
		gp2wWindow = NULL;
	}
	p2w_Terminate();

	gQuit = 1;	// must do this, Think C calls unexpected_exit (again) INSIDE ExitToShell!
} // exit_handler



// ---------------------------------------------------------------------
// Called if unexpectedly aborting
static void unexpected_exit(void)
{
	// we are exiting within an error condition at this point...
	if (gQuit == 0)
	{
		if (gAppPrefs_h)
		{	
			DisposeHandle((Handle) gAppPrefs_h);
			gAppPrefs_h = NULL;	
		}	
		// fatal error, exiting
		(void)displayDialog(132, NULL, 0, ewcDoCentering, eMainDevice);
		exit_handler();
		ExitToShell();
	}
} // unexpected_exit



// ---------------------------------------------------------------------
// Set initial configuration flags
static Boolean CheckGestaltBit(unsigned long gestaltResponse, short gestaltBit)
{
	return( (gestaltResponse & (1<<gestaltBit) ) != 0);
} // CheckGestaltBit



// ---------------------------------------------------------------------
// Set initial configuration flags
static void CheckConfiguration(void)
{
	long		gestaltResponse;
	char		*errStr;	

	errStr = NULL;	

	// Check system version stuff
	gHasSys70 = false;
	if (Gestalt(gestaltSystemVersion, &gestaltResponse) == noErr)
	{
		gHasSys70 = (gestaltResponse >= 0x0700);
		if ((gestaltResponse < 0x0604) && (errStr == NULL))
			errStr = "You do not have System 6.0.4 or better.";	
	}

	// Check CPU stuff
#if defined (NEEDS_68020)
	if (Gestalt(gestaltProcessorType, &gestaltResponse) == noErr)
	{
		if ((gestaltResponse < gestalt68020)	 && (errStr == NULL))
			errStr = "You do not have a 68020 or better CPU.";	
	}
#endif // NEEDS_68020

	// Check FPU stuff
	if (Gestalt(gestaltFPUType, &gestaltResponse) == noErr)
	{
#if defined (NEEDS_FPU)
		// If this was compiled to REQUIRE FPU, get error if no FPU installed
		if ((gestaltResponse == gestaltNoFPU) && (errStr == NULL))
			errStr = "You do not have a Floating Point Unit.";
#else
		// if compiled to not need FPU, give warning if running on FPU machine
		if (gestaltResponse != gestaltNoFPU)
			displayDialog(kdlog_UseFPUVersion, NULL, 0, ewcDoCentering, eMainDevice);
#endif // NEEDS_FPU
	}

	// Check Quickdraw stuff
	gHas32BitQD = false;
	gHasPictUtils = false;
	if (Gestalt(gestaltQuickdrawVersion, &gestaltResponse) == noErr)
	{
		gHas32BitQD = (gestaltResponse >= gestalt32BitQD);
		gHasPictUtils = (gestaltResponse >= gestalt32BitQD) && gHasSys70;
#if defined (NEEDS_32BITQD)
		if ((!gHas32BitQD)  && (errStr == NULL))
			errStr = "You do not have 32 Bit Quickdraw.";
#endif // NEEDS_32BITQD
	}

	// Check QuickTime version
	gHasQuickTime = false;
	gHasImageCompressionMgr = false;
	if (Gestalt(gestaltQuickTime, &gestaltResponse) == noErr)
	{
		gQTVersion = gestaltResponse;
		if (gQTVersion >= 0x01508000) // at least version 1.5 final
		{
			gHasQuickTime = true;
			// Check QuickTime Compression stuff
			if (Gestalt(gestaltCompressionMgr, &gestaltResponse) == noErr)
				gHasImageCompressionMgr = CheckGestaltBit(gestaltResponse, 4);
		}
	}

	// Check AppleEvent stuff
	gHasAppleEvents = false;
	if (Gestalt(gestaltAppleEventsAttr, &gestaltResponse) == noErr)
		gHasAppleEvents = CheckGestaltBit(gestaltResponse, gestaltAppleEventsPresent);

	// Check popup mgr stuff
	gHasPopups = false;
	if (Gestalt(gestaltPopupAttr, &gestaltResponse) == noErr)
		gHasPopups = CheckGestaltBit(gestaltResponse, gestaltPopupPresent);

	// Show any initialization errors now
	if (errStr)	
	{	// insufficient hardware/software config..
		displayDialog(kdlog_ConfigFatalErr, errStr, 0, ewcDoCentering, eMainDevice);
		exit_handler();
		ExitToShell();
	}
} // CheckConfiguration



// ---------------------------------------------------------------------
// Read any special cursors to be used later
static void SetupCursors(void)
{
	CursHandle	hCurs;
	
	// Note: iBeamCursor & watchCursor are defined in ToolUtils.h
	hCurs = GetCursor(iBeamCursor);
	gEditCursor = **hCurs;
	hCurs = GetCursor(watchCursor);
	gWaitCursor = **hCurs;
} // SetupCursors



// ---------------------------------------------------------------------
// load all needed menus
static void SetupMenus()
{
	int	i;

	for (i = 0; i < num_of_menus; i++)
	{
		myMenus[i] = GetMenu(i + menu_offset);
		if (myMenus[i] == NULL)
		{
			(void)displayDialog(kdlog_GenericFatalErr, "Cannot get main menu resource", i, ewcDoCentering, eMainDevice);
			exit_handler();
		}
		else
		{
			if (i==0) // add DA list to Apple menu
				AddResMenu(myMenus[apmn_ID - menu_offset], 'DRVR');
			InsertMenu(myMenus[i], 0);
		}
	}
	for (i = 0; i < num_of_submenus; i++)
	{
		mySubMenus[i] = GetMenu(i + submenu_offset);
		if (mySubMenus[i] == NULL)
		{
			(void)displayDialog(kdlog_GenericFatalErr, "Cannot get sub-menu resource", i, ewcDoCentering, eMainDevice);
			exit_handler();
		}
		else
			InsertMenu(mySubMenus[i], -1);
	}
	DrawMenuBar();
	
	// disable menus until we are ready...
	DisableMenus();
} // SetupMenus



// ---------------------------------------------------------------------
// hello Mac world...
int main(void)
{
	long				stk_size;
	short				memTrax_Size;
	long				maxMallocListSize;
	app_config_hdl_t	app_config_h;
	int					i,j;
	OSErr				anError;
	Rect				p2wRect;	
	PicHandle			scPicH = NULL;

#if defined(applec)
	UnloadSeg((Ptr) _DataInit);
#endif // applec

	AppRefNum = CurResFile();

	/* Allocate more memory for stack before doing much heap stuff! */
	app_config_h = (app_config_hdl_t)Get1Resource(kAppConfigRsrc, kAppConfigRsrcID);
	if (app_config_h)
	{
		// stackSize - # of bytes to grow the stack to (for recursion)
		// [10k < stk_size < 150k]
		stk_size = (**app_config_h).stackSize;
		if (stk_size <= 10000L)
			stk_size = 10000L;
		else if (stk_size > 150000L)
			stk_size = 150000L;

		// memTrackingSize - percentage of free mem to use for malloc tracking
		// [0 < memTrax_Size < 100]
		memTrax_Size = (**app_config_h).memTrackingSize;
		if (memTrax_Size < 0)
			memTrax_Size = 0;
		else if (memTrax_Size > 99)
			memTrax_Size = 99;
		// all done with this
		ReleaseResource((Handle)app_config_h);
	}
	else
	{ // set to default values
		stk_size = DEFAULT_STACK_SIZE;
		memTrax_Size = 10; // 10% avail mem
	}
	SetApplLimit(GetApplLimit() - stk_size);
	MaxApplZone();

	/* give us some master pointer blocks ahead of time, to reduce later memfrag */
	// base the # needed on the % mallocs they track
	j = 20*memTrax_Size;
	for (i=0; i<j; i++)
		MoreMasters();

	/* Initialize toolbox managers. */
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	InitCursor();
	SetupCursors();
	SetCursor(&gWaitCursor);

	/* Register an exit handler */
	atexit(unexpected_exit);

	/* Bring us to the front under MultiFinder */
	i=EventAvail(0, &gTheEvent);
	i=EventAvail(0, &gTheEvent);
	i=EventAvail(0, &gTheEvent);

	/* Check if the necessary hard- & software is present. */
	CheckConfiguration();

	// set up a full distribution message for display in splash screen and about box.
	gDistMessage = (StringPtr)NewPtr(256);
	strcpy((char*)gDistMessage, DISTRIBUTION_MESSAGE_1);
	strcat((char*)gDistMessage, " ");
	strcat((char*)gDistMessage, DISTRIBUTION_MESSAGE_2);
	strcat((char*)gDistMessage, " ");
	strcat((char*)gDistMessage, DISTRIBUTION_MESSAGE_3);
	c2pstr((char*)gDistMessage);

	/* create application prefs records */
	gAppPrefs_h = (app_prefs_hdl_t) NewHandle(sizeof(app_prefs_rec_t)); 
	MoveHHi((Handle) gAppPrefs_h);
	HLock((Handle) gAppPrefs_h);

	/* create default file settings record */
	gDefltFilePrefs_h = (file_prefs_hdl_t) NewHandle(sizeof(file_prefs_rec_t)); 
	MoveHHi((Handle) gDefltFilePrefs_h);
	HLock((Handle) gDefltFilePrefs_h);

	/* create file settings record */
	gFilePrefs_h = (file_prefs_hdl_t) NewHandle(sizeof(file_prefs_rec_t)); 
	MoveHHi((Handle) gFilePrefs_h);
	HLock((Handle) gFilePrefs_h);

	/* set up bounds for window dragging (multiple/big monitors) */
	SetRect(&gDragBounds, -32000, -32000, 32000, 32000);

	/* display splash screen early on, while app gets set up */
	CreateSplashScreen();

	/* See if the image compression manager (QuickTime to the Rest Of Us) is installed. */
	if (gHasImageCompressionMgr)
	{
		/* Install and open the standard compression dialog component. */
// the old way..
//		gtheSCComponent = OpenStdCompression();
// the better way
		gtheSCComponent = OpenDefaultComponent(StandardCompressionType, StandardCompressionSubType);
		if (gtheSCComponent)
		{
			/* Initial defaults for SC compression dialog. */	
			(void)SCGetInfo(gtheSCComponent, scSpatialSettingsType, &(**gDefltFilePrefs_h).sc_DialogParams);
			scPicH = GetPicture(kDemoPICTrsrcID);
			// pick a preview pict for initializing the SC defaults
			if (scPicH)
			{
				// set default pict
				(void)SCDefaultPictHandleSettings(gtheSCComponent, scPicH, false);
				// get defaults the component set up
				(void)SCSetTestImagePictHandle(gtheSCComponent, scPicH,
												NULL, scPreferScalingAndCropping);
			}
		}
	}

	/* Set up application menus. */
	SetupMenus();

	/* Read global settings from prefs file */
	GetAppPrefs();

	// refresh splash screen in case previous dialogs walked on it
/*---
This creates an annoying refresh partway through startup EVERY TIME.  Instead,
we will live with the ONE TIME that the "updating prefs" dialog steps on the
splash screen, and leave it with an un-updated hole in it. [esp]
-bug reported 7/16/93 [ar]
	if (gSplashScreen)
	{
		SelectWindow(gSplashScreen);
		DrawDialog(gSplashScreen);
	}
---*/


	*gSrcWind_FileName = '\0';

	// Set up the status output window
	anError = p2w_Init();
	if (!anError)
	{
		p2wRect = (**gDefltFilePrefs_h).statWind_pos;
		gp2wWindow = p2w_NewWindow(kWindID_p2w, &p2wRect, "\pPOV-Ray Status", true, monaco, 9, &anError);
	}
	if (anError)
	{
		(void)displayDialog(kdlog_P2W_INIT_ERROR, NULL, 0, ewcDoCentering, eMainDevice);
		exit_handler();
	}

	// has QuickTime, but not Image compression mgr (probably old QT version)
	// if debug mode, display some info to user
	if (((**gDefltFilePrefs_h).progress >= kProgDebug) && gHasQuickTime)
	{
		printf("-d QuickTimeVersion=0x%08lx, ImageCompressionExists=%d\n", gQTVersion, gHasImageCompressionMgr);
	}

	// set up template menu
	anError = InitTemplateMenu();
	if (anError)
	{
		// fatal error
		displayDialog(kdlog_GenericFatalErr, "Cannot create template submenus",
					anError, ewcDoCentering, eMainDevice);
		exit_handler();
	}

	InitImageWindow();

	SetupPalettes();

	// Set up offscreen pixmap
	SetupOffscreen();

	// display stack size
	if ((**gDefltFilePrefs_h).progress >= kProgDebug)
	{
		printf("-d StackSize=%ldK\n", stk_size/1024);
	}

	// allocate malloc() garbage collection buffers
	// max entries = memTrax_Size % of avail memory (div 2 since there are 2 lists),
	// and 500 entries minimum.
	maxMallocListSize = (FreeMem() * (long)memTrax_Size) / (2L*100L) / (long)sizeof(Ptr);
	if ((**gDefltFilePrefs_h).progress >= kProgDebug)
	{
		printf("-d MemTrackBuffEntries=%ld, MemTrackBuffPercent=%d%%\n", maxMallocListSize, memTrax_Size);
	}
	anError = POV_init_memtracking(maxMallocListSize);
	if (anError)
	{
		// fatal error
		displayDialog(kdlog_GenericFatalErr,
				"Cannot allocate memory for garbage collection",
				maxMallocListSize, ewcDoCentering, eMainDevice);
		exit_handler();
	}
	POV_enable_memtracking(false);

	// init the fake argc/argv buffers
	InitArgs();

	// initialize the ODOC File Queue
	FileQ_c();

	init_undo_system();
	init_redo_system();

	if (gHasAppleEvents)
		InstallAppleEvents();

	gRenderedOK = true;

	/* Figure out how much (or little) time to give other processes (oink) */
	gCpuHogginess = (**gAppPrefs_h).howMultiFriendly;
	CalcCpuReleaseTicks(false);
	main_init();		/* for text editor */

	/* switch to regular cursor after initialization is done */
	SetCursor(&qd.arrow);

	/* if under System 6, gotta do an initial empty document ourselves */
	if (!gHasSys70)
		DoFile_New();

	// give the user some control!
	EnableMenus();

	/* Display initial credit screens now */
	print_credits();
	PrintMacCredits();

	/* Main event loop, sort of. */
	while (!gQuit)
	{
		Cooperate(true);

		if (gBeginRendering)
			DoRendering();

	} // while
	
	/* Clean up & exit. */
	exit_handler();

	return 0; // reduce MPW compiler warnings

} // main

// oops, too far...
