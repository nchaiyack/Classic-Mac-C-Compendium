/*   PComUtil_ARTAbrot_prj				Common and Utilitys

Name:  PComUtil_ARTAbrot_prj.h  
Function:  Common for the Marksmanª specific code.
History: 8/18/93 Original by George Warner
   */

#include <SANE.h> /* Math definitions */
#include <Values.h> /* Math type sizes */
#include <Types.h> /* Basic Macintosh type definitions */
#include <Memory.h> /* Memory structure types */
#include <Resources.h> /* Resource file routines */
#include <Quickdraw.h> /* QuickDraw types and routines */
#include <Fonts.h> /* Font types and routines */
#include <Events.h> /* Event types and routines */
#include <Controls.h> /* Control routines */
#include <Windows.h> /* Windows */
#include <Menus.h> /* Menus */
#include <TextEdit.h> /* Edit text */
#include <Dialogs.h> /* Dialogs */
#include <Desk.h> /* Desktop routines */
#include <ToolUtils.h> /* Toolbox utilitys */
#include <SegLoad.h> /* Init files selected */
#include <OSUtils.h> /* OS utilitys */
#include <Files.h> /* File routines */
#include <OSEvents.h> /* OS events */
#include <DiskInit.h> /* Floppy disk routines */
#include <Packages.h> /* Extra packages */
#include <Lists.h> /* List manager */
#include <GestaltEqu.h> /* Gestalt definitions */
#include <QDOffscreen.h> /* Color GWorld routines */
#include <Sound.h> /* Sound manager */
#include <Printing.h> /* Print manager */

/* ======================================================= */


/* Basic definitions */
#define TRUE		1								/* Boolean true */ 
#define FALSE		0								/* Boolean false */ 

#define NIL		0L								/* Used for empty pointers and handles */ 

#define SCROLLBARWIDTH 15

/* User event definitions */
#define UserEvent_None 	0						/* No user events available */
#define UserEvent_Open_Window 	1				/* Open Window or modeless dialog */
#define UserEvent_Close_Window 	2				/* Close Window or modeless dialog */
#define UserEvent_Activate_Window 	3			/* Activate Window or modeless dialog */
#define UserEvent_Deactivate_Window 	4			/* Deactivate Window or modeless dialog */
/* IDs 0 to 999 reserved for Marksmanª, all others available for special use */

/* Menu list resource IDs */
#define Res_Menu_Apple 	256						/* Menu resource ID */
#define MItem_About 	1

#define Res_Menu_File 	257						/* Menu resource ID */
#define MItem_Quit2 	1

#define Res_Menu_Commands 	258					/* Menu resource ID */
#define MItem_Enter_Coordinat 	1
#define MItem_Display 	2


/* Window resource IDs, also controls grouped with the appropiate window */

#define Res_MD_About_ARTAbrot 	258				/* Modeless Dialog */
// #define Res_Dlg_Picture 	2					/* ...Picture button */

#define Res_MD_Enter_Coordinat 	257				/* Modeless Dialog */
#define Res_Dlg_OK 	1							/* ...Button */
#define Res_Dlg_Cancel 	2						/* ...Button */
#define Res_Dlg_Static_Text4 	521				/* ...Static text */
#define Res_Dlg_Static_Text3 	520				/* ...Static text */
#define Res_Dlg_Static_Text2 	518				/* ...Static text */
#define Res_Dlg_Static_Text 	517				/* ...Static text */
#define Res_Dlg_Edit_Text4 	3					/* ...Edit text - iterations */
#define Res_Dlg_Edit_Text3 	4					/* ...Edit text - width */
#define Res_Dlg_Edit_Text2 	5					/* ...Edit text - Y coord. */
#define Res_Dlg_Edit_Text 	6					/* ...Edit text - X coord. */

#define Res_W_ARTAbrot 	256						/* Window */

/* Picture resource IDs */
#define Pict_Picture 	30001

/* Sound resource IDs */
#define Snd_Bart__Cooool 	256


/* ======================================================= */


typedef struct UserEventRec{						/* User Event Record definition */
	short	ID;								/* ID for the type of user event */
	short	ID2;								/* Optionally used, 2nd ID, sometimes used */
	long	Data1;								/* Optionally used, Extra data */
	long	Data2;								/* Optionally used, Extra data */
	Handle	theHandle;						/* Optionally used, Handle */
	struct UserEventRec	**Next;				/* Handle of next event in the list */
}UserEventRec,*UserEventPRec,**UserEventHRec;

extern Boolean	Doing_MovableModal;				/* For Movable Modal dialogs */
extern UserEventHRec	UserEventList;				/* User Event record list start */
extern EventRecord	myEvent;						/* Event record for all events */
extern Boolean	WNE;								/* WaitNextEvent trap is available */
extern short	SleepValue;						/* Sleep value for Wait on events */
extern Boolean	doneFlag;						/* Exit program flag */
extern TEHandle	theInput;						/* Used in text edit selections */
extern Rect	tempRect;							/* Temporary rect, not for long term use */
extern Str255	sTemp;							/* Temporary string, not for long term use */
extern Boolean		HasColorQD;					/* Flag for Color QuickDraw being available */
extern Boolean		HasFPU;						/* Flag for Floating Point Math Chip being available */
extern Boolean		HoldOffUserEvents;			/* Flag for holding off UserEvents */
extern Boolean		InTheForeground;				/* Flag for running in MultiFinder foreground */
extern short		tempChar;						/* Temporary use character */
extern long		LTemp;							/* Temporary use long variable */
extern RGBColor		Black_ForeColor,White_BackColor;/* Standard colors */

extern SFTypeList	typeList;						/* For use in opening files */
extern OSErr		ErrorCode;						/* For use in reading and writing files */
extern SFReply	Reply;							/* For use in getting file names */
extern StandardFileReply	theStandardFileReply;	/* For use in getting file names */
extern short	inputRefNum;						/* For the input file */
extern short	outputRefNum;						/* For the output file */
extern Str255 inputFileName;						/* For the input file */
extern Str255 outputFileName;						/* For the output file */

extern RgnHandle	cursorRgn;						/* Cursor region for WaitNextEvent */
extern Boolean		HasAppleEvents;				/* Whether AppleEvents are available */
extern Boolean		HasAliasMgr;					/* Whether AliasMgr is available */
extern Boolean		HasEditionMgr;				/* Whether EditionMgr is available */
extern short		ReplyMode;						/* Reply mode for AppleEvents */
extern Boolean		HasNewStdFile;				/* Whether HasNewStdFile is available */
extern Boolean		HasPPCToolbox;				/* Whether PPCToolbox is available */
extern Boolean		Has32BitQuickDraw;			/* Whether 32Bit QuickDraw is available */
extern Boolean		HasGestalt;					/* Whether Gestalt is available */

extern THPrint	hPrint;							/* Printer record */
extern Boolean	PrinterIsOpen;					/* Whether printer is open or not */
extern short	PageCount;							/* For adding in page number */
extern short	LineCount;							/* For printing */
extern short	PrinterVRes,PrinterHRes;			/* Printer resolution */
extern short	MaxPrintCharacters;				/* Max characters on one line */
extern short	MaxLines;							/* Maximum lines in one page */
extern TPPrPort		pPrPort;					/* Printer port */
extern TPrStatus		prStatus;					/* Printing status */
extern short	CopyCount;							/* Number of copies to do */

extern MenuHandle	Menu_Apple;					/* Menu handle */
extern MenuHandle	Menu_File;					/* Menu handle */
extern MenuHandle	Menu_Commands;				/* Menu handle */

/* Window variables for the window titled  "ARTAbrot" */
extern WindowPtr	WPtr_ARTAbrot;					/* Window pointer */

/* Variables for the modeless dialog titled  "About ARTAbrot" */
extern WindowPtr	WPtr_About_ARTAbrot;			/* Modeless Dialog window pointer */

/* Variables for the alert titled  "Alert" */

/* Variables for the modeless dialog titled  "Enter Coordinates" */
extern WindowPtr	WPtr_Enter_Coordinat;			/* Modeless Dialog window pointer */
extern Rect	DTE_Rect_Edit_Text4;				/* ...Edit text - iterations */
extern Rect	DTE_Rect_Edit_Text3;				/* ...Edit text - width */
extern Rect	DTE_Rect_Edit_Text2;				/* ...Edit text - Y coord. */
extern Rect	DTE_Rect_Edit_Text;					/* ...Edit text - X coord. */


/* Mandelbrot coordinates. */
extern float fxcenter, fycenter, fwidth; /* X center, Y center, and width of fractal. */
extern int fiters;				/* Maximum number of iterations. */
extern int new_coordinates;


/* Prototypes */

/* Handle an activate of the window */
void DoActivate(void);

/* See if WaitNextEvent is available */
Boolean IsWNEIsImplemented(void);

/* Check for user events */
void Handle_User_Event(void);

/* Handle a hit in the window */
void DoInContent(WindowPtr whichWindow,EventRecord *myEvent);

/* ======================================================= */

/* Copy Pascal strings */
void PStrCopy(Str255 *SourceString, Str255 *DestString);

/* Concat Pascal strings */
void PStrCat(Str255 *SourceString, Str255 *DestString);

/* Compare Pascal strings for exact match, case sensitive */
Boolean PStrCmp(StringPtr Source1String,StringPtr Source2String);

Boolean CheckTrapAvailable (short trapNumber, short tType);/* See if a trap is available */

void GetUserEvent(UserEventPRec TheUserEvent);		/* See if any user events are available */

void Add_UserEvent(short ID, short ID2, long  Data1, long Data2, Handle  theHandle);/* Add a user event */

void Play_The_Sound(short ResID);					/* Play my sound */

/* This is a routine used to get a string from a TE area, limited to 250 characters */
void Get_TE_String(TEHandle theTEArea, Str255 *theString);/* Get the TE String */

/* This is a routine used to create a TE area */
void Make_TE_Area(TEHandle *theTEArea, Rect *Position, short theFontSize, short theFont, short DefaultStringID);/* Make the TE area */

void SetupTheItem (DialogPtr theDialog, short ItemID, Boolean SizeIt, Boolean ShowIt, Boolean EnableIt,/* Setup a dialog or alert item */
Boolean SetTheMax,Rect * thePosition , long ExtraData , short StringID );/* Setup a dialog or alert item */

void CenterOnColorScreen ( WindowPtr theWindow );	/* Center a window onto the deepest color screen */

void GetDeepestColorScreenRect (Rect *DeepRect);	/* Get the deepest color screen rect */

