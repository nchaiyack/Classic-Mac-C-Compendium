/*==============================================================================
Project:	POV-Ray

Version:	2.2

File:	PovMac.h

Description:
	This file contains typedefs, function prototypes and extern var declarations
	that need to be exported from pov.c.
------------------------------------------------------------------------------
Authors:
	Thomas Okken, David Lichtman, Glenn Sugden
	Jim Nitchals, Eduard [esp] Schwan
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
		AppleLink: jl.tech
	Jim Nitchals
		Compuserve: 73117,3020
		America Online: JIMN8
		Internet: jimn8@aol.com -or- jimn8@applelink.apple.com
		AppleLink: JIMN8
------------------------------------------------------------------------------
Change History:
	920815	[jln]	version 1.0 Mac released.
	920908	[esp]	version 1.1 alpha Mac
	921221	[esp]	Changed to include QuickTime 1.5 Headers
	930826	[esp]	Added 1.x/2.x ifdefs in
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
	931119	[djh]	2.0.1 source conditionally compiles for PPC machines, keyword __powerc
==============================================================================*/

#if !defined(POVMAC_H)
#define POVMAC_H


/*==== POV definitions =====*/
#include "FRAME.H"

/*==== Mac definitions =====*/


/*==== QuickTime compression headers ====*/

#if defined (THINK_C)
// THINK C has problems linking to StdCompressionGlue.o, fix it!
#define OpenStdCompression OPENSTDCOMPRESSION
#endif // THINK_C
#include "ImageCompression.h"
#include "QuickTimeComponents.h"

#if defined(__powerc)
#include <MixedMode.h>
#endif


/*==== Animate header ====*/
#include "Animate.h"	// for AnimRec_t definition in prefs



/*==== Think to non-THINK compatibility ====*/

#if !defined(THINK_C)
#include <Strings.h>	// p<->cstr
#endif // THINK_C


/*==== definitions ====*/

// Application prefs file
#define	POV_RAY_PREFS_FNAME		"\pPOV-Ray 2.0 Prefs"

#define	kPrefsFileType			'pref'

// default stack size
#define	DEFAULT_STACK_SIZE		80000

#define _QD32Trap 0xAB03 /* 32 bit Quickdraw trap */

#define kRsrcFileClosed			(-1)

/* keyboard values used by ModalFilter() */
#define kReturnKey				0x0D
#define kEnterKey				0x03
#define kEscKey					0x1B
#define kTabKey					0x09

/* All dialogs have a user item #3 for outlining buttons */
#define kDefaultItem			3

/* Windows */
#define	kWindID_p2w				9600	// POV-Ray status window

/* Dialog resource IDs */
#define	kdlog_P2W_INIT_ERROR	9600

#define	kdlog_ConfigFatalErr	129	// bad hardware/software config
#define	kdlog_GenericFatalErr	152 // general purpose fatal error dialog
#define	kdlog_CantOpenOverDirty	154	// cant open over existing busy file
#define	kdlog_CantOpenNonText	155	// cant open non-text files
#define	kdlog_UseFPUVersion		158	// Note: should use FPU version of POV-Ray

/* our application's creator signature */
#define	kAppSignature	'PvRy'

/* POV Menu equates */
#define num_of_menus	7
#define num_of_submenus 3
#define menu_offset		128		// ID of the first menu
#define submenu_offset	200		// ID of the first submenu

/* Apple Menu */
#define apmn_ID			128
#define apmn_about		1

/* File Menu */
#define fmn_ID			129
#define fmn_new			1
#define fmn_open		2
#define fmn_close		4
#define fmn_save		5
#define fmn_saveas		6
#define fmn_quit		8

/* Edit Menu */
#define edmn_ID			130
#define edmn_undo		1
#define edmn_cut		3
#define edmn_copy		4
#define edmn_paste		5
#define edmn_clear		6
#define edmn_selectAll	7
#define edmn_redo		9
#define edmn_goto		11
#define edmn_prefs		13
#define edmn_macros		15
#define edmn_lookup		17

/* Image Menu */
#define immn_ID			131
#define immn_view		1
#define immn_dither		2
#define immn_custom 	3

/* Render Menu */
#define rnmn_ID			132
#define rnmn_options	1
#define rnmn_render		3
#define rnmn_pause		4
#define rnmn_stop		5
#define rnmn_autosave	7
#define rnmn_shutdown	8
#define render_file		40

/* Processing Menu */
#define psmn_ID			133
#define psmn_border		1
#define psmn_darken		2
#define psmn_lighten	3
#define psmn_reduceC	4
#define psmn_increaseC	5
#define psmn_invert		6
#define psmn_divider1	7
#define psmn_revert		8

/* Windows Menu */
#define wndmn_ID		134
#define wndmn_status	1
#define wndmn_source	2
#define wndmn_image		3

/* View submenu */
#define viewmn_ID		200
#define viewmn_hidden		1
#define viewmn_Size2Window	2
#define viewmn_normal		3
#define viewmn_x2			4
#define viewmn_x3			5
#define viewmn_x4			6

/* palette submenu */
#define plmn_ID			201
#define palette_none	1
#define palette_default	3
#define palette_median	4
#define palette_popular 5
#define palette_var_min	6
#define palette_octree	7

/* don't want to collide with system equates, so... */
#define varianceMethod	1000
#define octreeMethod	1001


#if !defined (min)
#define min(x,y) ((x>y)?y:x)
#endif // min


// Application internal configuration settings resource (stack size, etc.)
#define	kAppConfigRsrc			'CNFG'
#define	kAppConfigRsrcID		1000

typedef struct
{
	long		stackSize;			// # of bytes to grow stack to
	short		memTrackingSize;	// % of heap to use for malloc-tracking arrays
} app_config_rec_t, *app_config_ptr_t, **app_config_hdl_t;


/* the resource ID of the preferences resources (1.0 was 128, 2.0 is 200) */
#define	kPrefs_rsrcID	200


/* the version # of the prefs resources - increment this when prefs_rec_t's change */
/* This currently follows the app version #, so 0x0201 is version 2.01 */
#define	kPrefs_rsrcVers		0x0201


/*==== preferences resource - Application ====*/

/* values for background CPU usage in App Prefs */
#define	kMinMultiFriendly		1
#define	kDefMultiFriendly		5
#define	kMaxMultiFriendly		11

#define	kAppPrefsRsrc	'aPrf'

typedef enum
{
	eWhenNtf_Quiet=0,		// don't notify at all when done
	eWhenNtf_BgOnly,		// notify when done, only if in background
	eWhenNtf_FgOnly,		// notify when done, only if in foreground
	eWhenNtf_BgFg			// notify when done, in background or foreground
} WhenNotify_t;

typedef enum
{
	eHowNtf_Noise=0,		// beep when done
	eHowNtf_Dlg,			// show dialog when done
	eHowNtf_NoiseDlg		// beep & show dialog when done
} HowNotify_t;


typedef struct
{
	short			prefsVersion;
	short			howMultiFriendly;		// how tight are YOUR loops?
	WhenNotify_t	whenToNotify;			// circumstances to notify
	HowNotify_t		howToNotify;			// style of notification
	FSSpec			includeDirFSSpec;		// search path directory ref (name=volname!)
} app_prefs_rec_t, *app_prefs_ptr_t, **app_prefs_hdl_t;


/*==== preferences resource - File ====*/

// Max Symbols value (+ms)
// Goes up from 500, in increments of 500
#define	kMaxSymMinVal			500					// min value allowed
#define	kMaxSymFactor			500					// increment value
#define	kMaxSymMaxVal			(kMaxSymMinVal+8*kMaxSymFactor)	// max value allowed (8 menu items)
#define	kDefaultMaxSym			MAX_CONSTANTS	// default (from config.h)

// Progress item values
#define kProgNone		1	// quiet
#define kProgMinimal	2	// Mac related info
#define kProgVerbose	3	// Mac & Core info
#define kProgDebug		4	// Core debug messages to external file
#define kProgMax		kProgDebug

// Min language version values (+mv)
#define kMinLangVersion		1	// POV-Ray 1.0
#define kMaxLangVersion		2	// POV-Ray 2.0

#define	kFilePrefsRsrc	'fPrf'

typedef struct
{
	short		prefsVersion;		// magic version # of this record (kPrefs_rsrcVers)
	Rect		srcWind_pos;		// source window placement on screen
	Rect		statWind_pos;		// status window placement
	Rect		imageWind_pos;		// image window placement
	short		imageWidth;			// width of image in pixels
	short		imageHeight;		// height of image in pixels
	Point		imageUpperLeft;		// image scroll position within window
	Rect		selectionArea;		// image selection area within window (+SC/+EC/+SR/+ER)
	OSType		pictFileCreator;	// PICT file creator
	OSType		targaFileCreator;	// Targa file creator
	short		renderQuality;		// quality index for image (+Q 0-9)
	short		custom_palette;		// style of palette to use
	short		imageMagFactor;		// magnification factor of image window (0,1,2,3,4)
	short		createTargaOutfile;	// True if generating Targa output file
	short		continueTarga;		// True to use previous Targa file
	short		doDither;			// True to dither image
	short		doBoundSlabs;		// True to use bounding slabs (+MB)
	short		boundSlabThreshold;	// Min. # of objects before enabling slabs
	short		progress;			// levels of verbose progress during render (1-4) (+V1)
	short		languageVersion;	// for POV-Ray +MV command
	short		maxSymbolsVal;		// for POV-Ray +MS command
	short		doAnimation;		// True if doing animation
	short		doCompression;		// True if compressing image
	short		do24BitPict;		// True if doing 24 bit PICTs, false = 8 bit indexed
	short		doAntialias;		// True if antialiasing
	float		antialiasThreshold;	// value of anti-alias threshold
	short		antialiasDepth;		// for POV-Ray +R command
	float		antiJitterScale;	// for POV-Ray +J command
	AnimRec_t	animRec;			// animation info record
	SCSpatialSettings	sc_DialogParams;
} file_prefs_rec_t, *file_prefs_ptr_t, **file_prefs_hdl_t;


/*==== POV-Ray engine globals ====*/

// need to reference these from POV engine
extern volatile int			Stop_Flag;
extern DBL					Max_Trace_Level;			// render.c (5)
extern int					token_count, line_count;	// tokenize.c (0,5)
extern int					Include_File_Index;			// tokenize.c (0)
extern struct Token_Struct	Token;					// tokenize.c

extern void alt_main(short argc, char **argv);


/*==== pov.c globals ====*/

extern p2w_WindowPtr_t	gp2wWindow;			// the Status (text) window

extern app_prefs_hdl_t	gAppPrefs_h;		// App prefs from prefs file
extern file_prefs_hdl_t	gFilePrefs_h;		// for remembering source window resizing
extern file_prefs_hdl_t	gPrefs2Use_h;		// points to file or default prefs

extern Boolean			gHasSys70;			// Can we do System 7 stuff?
extern Boolean			gHasWNEvent; 		// WaitNextEvent trap around?
extern Boolean			gHasFSSpecs; 		// Can use FSSpec calls?
extern Boolean			gHasAppleEvents;	// Apple Events available for doing shutdown?
extern Boolean			gHas32BitQD; 		// is 32 bit Quickdraw available for depth & dithering
extern Boolean			gHasPictUtils;			// is Picture Utils (extract best palette) avail?
extern Boolean			gHasImageCompressionMgr; // Is QuickTime Image compression available?
extern Boolean			gHasPopups;			// are popup menus avail?

extern Boolean			gInBackground;		// is the program currently switched out
extern Boolean			gDoingRender;		// for determining the menu states
extern Cursor			gEditCursor;
extern Cursor			gWaitCursor;
extern Rect				gDragBounds;		// window dragging boundary
extern Boolean			gCanUndo;			// can we undo this operation?


/*==== PowerPC-specific globals ====*/

#if defined(__powerc)
extern QDGlobals qd;
extern RoutineDescriptor gModalFilterRD;
extern RoutineDescriptor gOutlineButtonRD;
extern RoutineDescriptor gPopupMenuRD;
extern RoutineDescriptor gDimDlogTextRD;
extern RoutineDescriptor gShowProgressRD;
extern RoutineDescriptor gAEOAppRD;
extern RoutineDescriptor gAEODocRD;
extern RoutineDescriptor gAEQuitRD;
extern RoutineDescriptor gPutPICTRD;
extern RoutineDescriptor gVScrollRD;
extern RoutineDescriptor gHScrollRD;
#endif // __powerc


/*==== pov.c Prototypes ====*/

pascal void	outlineDefaultButton(DialogPtr theDialog, short theItem);
void	SetupDefaultButton(DialogPtr theDialog);
ControlHandle	GrabDItemHandle(DialogPtr theDialog, short theGetItem);
pascal	Boolean	ModalFilter(DialogPtr theDialog, EventRecord *theDialogEvent, short *theDialogItem);
short	displayDialog(short dlogID, char *s, short err, WindCentering_t doCentering, WindPositioning_t whereToShow);
void	GetBestDialogPos(Point *wherep);
void	restore_state(int n);
void	WriteFilePrefs(void);
void	support_undo(char *the_reason, short flush_redo);
void	Cooperate(int doImmediate);
void	exit_handler(void);


#endif // POVMAC_H
