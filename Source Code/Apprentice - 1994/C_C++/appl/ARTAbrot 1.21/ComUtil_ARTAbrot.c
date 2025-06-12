/*   ComUtil_ARTAbrot							Common */

/* Name:  ComUtil_ARTAbrot.c */
/* History: 8/18/93 Original by George Warner */


#include "ComUtil_ARTAbrot.h"	/* Common */
// #include "SysEqu.h"	/* System equates */
/* ======================================================= */

Boolean	Doing_MovableModal;						/* For Movable Modal dialogs */
UserEventHRec	UserEventList;					/* User Event record list start */
EventRecord	myEvent;							/* Event record for all events */
Boolean	WNE;									/* WaitNextEvent trap is available */
short	SleepValue;								/* Sleep value for Wait on events */
Boolean	doneFlag;								/* Exit program flag */
TEHandle	theInput;								/* Used in text edit selections */
Rect	tempRect;									/* Temporary rect, not for long term use */
Str255	sTemp;									/* Temporary string, not for long term use */
Boolean		HasColorQD;							/* Flag for Color QuickDraw being available */
Boolean		HasFPU;								/* Flag for Floating Point Math Chip being available */
Boolean		HoldOffUserEvents;					/* Flag for holding off UserEvents */
Boolean		InTheForeground;					/* Flag for running in MultiFinder foreground */
short		tempChar;								/* Temporary use character */
long		LTemp;									/* Temporary use long variable */
RGBColor		Black_ForeColor,White_BackColor;	/* Standard colors */

SFTypeList	typeList;							/* For use in opening files */
OSErr		ErrorCode;							/* For use in reading and writing files */
SFReply	Reply;									/* For use in getting file names */
StandardFileReply	theStandardFileReply;			/* For use in getting file names */
short	inputRefNum;								/* For the input file */
short	outputRefNum;								/* For the output file */
Str255 inputFileName;								/* For the input file */
Str255 outputFileName;								/* For the output file */

RgnHandle	cursorRgn;							/* Cursor region for WaitNextEvent */
Boolean		HasAppleEvents;						/* Whether AppleEvents are available */
Boolean		HasAliasMgr;						/* Whether AliasMgr is available */
Boolean		HasEditionMgr;						/* Whether EditionMgr is available */
short		ReplyMode;							/* Reply mode for AppleEvents */
Boolean		HasNewStdFile;						/* Whether HasNewStdFile is available */
Boolean		HasPPCToolbox;						/* Whether PPCToolbox is available */
Boolean		Has32BitQuickDraw;					/* Whether 32Bit QuickDraw is available */
Boolean		HasGestalt;							/* Whether Gestalt is available */

THPrint	hPrint;									/* Printer record */
Boolean	PrinterIsOpen;							/* Whether printer is open or not */
short	PageCount;								/* For adding in page number */
short	LineCount;								/* For printing */
short	PrinterVRes,PrinterHRes;					/* Printer resolution */
short	MaxPrintCharacters;						/* Max characters on one line */
short	MaxLines;									/* Maximum lines in one page */
TPPrPort		pPrPort;							/* Printer port */
TPrStatus		prStatus;							/* Printing status */
short	CopyCount;								/* Number of copies to do */

MenuHandle	Menu_Apple;							/* Menu handle */
MenuHandle	Menu_File;							/* Menu handle */
MenuHandle	Menu_Commands;						/* Menu handle */

/* Window variables for the window titled  "ARTAbrot" */
WindowPtr	WPtr_ARTAbrot;						/* Window pointer */

/* Variables for the modeless dialog titled  "About ARTAbrot" */
WindowPtr	WPtr_About_ARTAbrot;					/* Modeless Dialog window pointer */

/* Variables for the alert titled  "Alert" */

/* Variables for the modeless dialog titled  "Enter Coordinates" */
WindowPtr	WPtr_Enter_Coordinat;					/* Modeless Dialog window pointer */
Rect	DTE_Rect_Edit_Text4;						/* ...Edit text rect */
Rect	DTE_Rect_Edit_Text3;						/* ...Edit text rect */
Rect	DTE_Rect_Edit_Text2;						/* ...Edit text rect */
Rect	DTE_Rect_Edit_Text;						/* ...Edit text rect */


/* Mandelbrot coordinates. */
float	fxcenter = 0.0;	/* X center of fractal. */
float	fycenter = 0.0;	/* Y center of fractal. */
float	fwidth = 4.0;		/* Width of fractal. */
int		fiters = 128;	/* Maximum number of iterations. */
int		new_coordinates = TRUE;


/* Prototypes */

/* Handle an activate of the window */
void DoActivate(void);

/* See if WaitNextEvent is available */
Boolean IsWNEIsImplemented(void);

/* Check for user events */
void Handle_User_Event(void);

/* Handle a hit in the window */
void DoInContent(WindowPtr whichWindow,EventRecord *myEvent);

#define	fnfErr		-43

/* ======================================================= */


/* Routine: PStrCopy */
/* Purpose: Copy Pascal strings */

void PStrCopy(Str255 *SourceString, Str255 *DestString)/* Copy Pascal strings */
{

*DestString[0] = *SourceString[0];					/* Get the string size */
if (*DestString[0] != 0)							/* Do if there is anything in the string */
	BlockMove(SourceString,DestString,*DestString[0]+1);
}

/* ======================================================= */

/* Routine: PStrCat */
/* Purpose: Concat Pascal strings */

void PStrCat(Str255 *SourceString, Str255 *DestString)/* Concat Pascal strings */
{
short	OldDestLength,NewDestLength,SizeFromSource;

OldDestLength = *DestString[0];					/* Get the string old length */
SizeFromSource = *SourceString[0];					/* Get the source length */
NewDestLength = OldDestLength + SizeFromSource;	/* Get the string new length */
if (NewDestLength > 255)							/* Compare to length of the final string */
	{
	NewDestLength = 255;							/* The string final length */
	SizeFromSource = 255 - OldDestLength;			/* The string final length */
	}											/* End of IF */
*DestString[0] = NewDestLength;					/* Set the string length */
if (SizeFromSource > 0)							/* Do if there is anything in the string */
	BlockMove((Ptr)((long)SourceString + 1L),(Ptr)((long)DestString+OldDestLength+1L),SizeFromSource);
}

/* ======================================================= */

/* Routine: PStrCmp */
/* Purpose: Compare Pascal strings for exact match, case sensitive */

Boolean PStrCmp(StringPtr Source1String,StringPtr Source2String)/* Compare Pascal strings */
{
short		theLength,Index;
Boolean		theSame;

	theSame = false;									/* Init to different */
	theLength = *Source1String++;						/* Get length of one string */
	if (theLength == *Source2String++)					/* Compare to length of other string, easy and quick check */
	{
		theSame = true;								/* Init to same, now we will check each character */
		for (Index=1; Index <= theLength; Index++)		/* Compare to length of other string, easy and quick check */
		{
			if (*Source1String++ != *Source2String++)	/* See if this character is the same */
			{
				theSame = false;						/* Different */
			}									/* End of IF */
		}										/* End of IF */
	}											/* End of IF */
	return(theSame);
}

/* ======================================================= */

/* Routine: CheckTrapAvailable */
/* Purpose: See if trap is available, non-available traps all have a unique address */

Boolean CheckTrapAvailable (short trapNumber,short tType)/* See if a trap is available */
{
#define UnimplementedTrapNumber 	0xA89F			/* Unimplemented trap number */
Boolean	theResult;

theResult = (NGetTrapAddress(trapNumber, tType) != GetTrapAddress(UnimplementedTrapNumber));/* Check the two traps */
return(theResult);
}

/* ======================================================= */

/* Routine: GetUserEvent */
/* Purpose: See if any user events are available */

void GetUserEvent(UserEventPRec TheUserEvent)
{ 
UserEventHRec	NextUserEvent;					/* The next user event */

TheUserEvent->ID = UserEvent_None;					/* Set ID to no events are available */
if (UserEventList != NIL)							/* Get first entry in the list */
	{
	HLock((Handle)UserEventList);					/* Lock for safety */
	TheUserEvent->ID = (*UserEventList)->ID;		/* The event ID */
	TheUserEvent->ID2 = (*UserEventList)->ID2;		/* The optional ID */
	TheUserEvent->Data1 = (*UserEventList)->Data1;/* The optional data */
	TheUserEvent->Data2 = (*UserEventList)->Data2;/* The optional data */
	TheUserEvent->theHandle = (*UserEventList)->theHandle;/* The optional handle */
	NextUserEvent = (*UserEventList)->Next;		/* The next list */

	DisposHandle((Handle)UserEventList);			/* Remove this list item */
	UserEventList = NextUserEvent;					/* Make the next item the new first item */
	}											/* End of IF */
}

/* ======================================================= */

/* Routine: Add_UserEvent */
/* Purpose: Add a user event */

void Add_UserEvent(short ID1,short  ID2,long Data1,long Data2,Handle  theHandle)
{ 
UserEventHRec	NewUserEvent;						/* The new user event */
UserEventHRec	theUserEvent;						/* The user event */

NewUserEvent = (UserEventHRec)NewHandle(sizeof(UserEventRec));/* Allocate a record */
if (NewUserEvent != NIL)							/* Only do if we got the new record */
	{
	HLock((Handle)NewUserEvent);					/* Lock for safety */
	(*NewUserEvent)->ID = ID1;						/* The event ID */
	(*NewUserEvent)->ID2 = ID2;					/* The optional ID */
	(*NewUserEvent)->Data1 = Data1;				/* The optional data */
	(*NewUserEvent)->Data2 = Data2;				/* The optional data */
	(*NewUserEvent)->theHandle = theHandle;		/* The optional handle */
	(*NewUserEvent)->Next = NIL;					/* No next item after this one */

	if (UserEventList == NIL)						/* See if anyone is in the list yet */
		{
		UserEventList = NewUserEvent;				/* Make this one the first in the list */
		}
	else
		{
		theUserEvent = UserEventList;				/* Get the first one */
		while ((*theUserEvent)->Next != NIL)		/* Get the next one */
			{
			theUserEvent = (*theUserEvent)->Next;
			}
		(*theUserEvent)->Next = NewUserEvent;		/* Tack on to the end */
		}										/* End of IF */
	}											/* End of IF */
}

/* ======================================================= */

/* Routine: Play_The_Sound */
/* Purpose: Play my sound */

void Play_The_Sound(short ResID)
{ 
Handle	ResHandle;								/* The sound resource handle */
short	SndResourceID;							/* Sound resource ID */
Str255	SndName;									/* Sound name*/
ResType	theType;								/* Resource type */
OSErr	MyErr;									/* Error flag */

ResHandle = Get1Resource('snd ', ResID);			/* See if the resource is out there */
if (ResHandle != NIL)								/* Only do if we got the sound */
	{
	GetResInfo(ResHandle, &SndResourceID, &theType, SndName);
	MyErr = SndPlay(NIL, ResHandle, FALSE);
	ReleaseResource(ResHandle);
	}											/* End of IF */
}

/* ======================================================= */

/* Routine: Get_TE_String */
/* Purpose: This is a routine used to get a string from a TE area, limited to 250 characters */

void Get_TE_String(TEHandle theTEArea,Str255 *theString)
{ 
short Index;										/* Use to loop thru the characters */
short TitleLength;									/* Number of characters to do */
CharsHandle theCharsHandle;						/* Used to get global edit text */
Ptr theStringPtr;									/* Pointer to the string, byte level */
Ptr theTECharPtr;									/* Pointer to the string, byte level */

theCharsHandle = TEGetText(theTEArea);				/* Get the character handle */
HLock ( ( Handle ) theCharsHandle );				/* Lock it for safety */
theTECharPtr = (Ptr)*theCharsHandle;				/* Get string Ptr */
TitleLength = (*theTEArea)->teLength;				/* Get the number of characters */
*theString[0] = 0;									/* Start with an empty string */
if (TitleLength > 0) 
	{
	theStringPtr = (Ptr)((long)theString + (long)1);/* Start of the string data */
	if (TitleLength > 250) 
		TitleLength = 250;
	for (Index = 0; Index < TitleLength; Index++)
		*theStringPtr++ = (char)(*theTECharPtr++);
	*theString[0] = TitleLength;
	}											/* End of IF */
}

/* ======================================================= */

/* Routine: Make_TE_Area */
/* Purpose: This is a routine used to create a TE area */

void Make_TE_Area(TEHandle *theTEArea,Rect *Position,short theFontSize,short theFont,short DefaultStringID)
{
FontInfo	ThisFontInfo;							/* Use to get the font data */

TextSize(theFontSize);								/* Set the size */
TextFont(theFont);									/* Set the font */
GetFontInfo(&ThisFontInfo);						/* Get Ascent height for positioning */
TextSize(12);										/* Restore the size */
TextFont(applFont);								/* Restore the font */

tempRect = *Position;								/* Get the rect */
FrameRect(&tempRect);								/* Frame this TE area */
InsetRect(&tempRect, 3, 3);						/* Indent for TE inside of box */
*theTEArea = TENew(&tempRect, &tempRect);			/* Create the TE area */
if (theInput != NIL)								/* See if there is already a TE area */
	TEDeactivate(theInput);					/* Yes, so turn it off */

theInput = *theTEArea;								/* Activate the TE area */
HLock((Handle)*theTEArea);							/* Lock the handle before using it */
(**theTEArea)->txFont = theFont;					/* Font to use for the TE area */
(**theTEArea)->fontAscent = ThisFontInfo.ascent;	/* Font ascent */
(**theTEArea)->lineHeight = ThisFontInfo.ascent + ThisFontInfo.descent + ThisFontInfo.leading;/* Font ascent + descent + leading */
HUnlock((Handle)*theTEArea);						/* UnLock the handle when done */
GetIndString(sTemp, DefaultStringID, 1);			/* Get the default string */
TESetText(&sTemp[1], sTemp[0], theInput);			/* Place default text in the TE area */
TEActivate(theInput);								/* Make the TE area active */
}

/* ======================================================= */

/* Routine: SetupTheItem */
/* Purpose: Setup a dialog or alert item */

void SetupTheItem(DialogPtr theDialog,short ItemID,Boolean SizeIt,Boolean ShowIt,
Boolean EnableIt,Boolean SetTheMax,Rect *thePosition,long ExtraData,short StringID)
{
Rect		tempRect;							/* Temporary rectangle */
short	DType;									/* Type of dialog item */
Handle	DItem;									/* Handle to the dialog item */
ControlHandle	CItem;							/* Control handle */

	GetDItem(theDialog,ItemID,&DType,&DItem,&tempRect);/* Get the item handle and size */
	CItem = (ControlHandle)DItem;				/* Change to control handle */
	if (SizeIt)									/* Have to resize all CDEF connected controls */
		SizeControl(CItem, tempRect.right-tempRect.left, tempRect.bottom-tempRect.top);/* Size it */
	*thePosition = tempRect;					/* Pass back the zone location and size */
	if (ExtraData != NIL)						/* See if extra data for a CDEF */
		(*CItem)->contrlData = (Handle)ExtraData;	/* Send it */
	if (StringID != 0) {						/* See if a CDEF and needs the title set again*/
		GetIndString(sTemp,StringID,1);			/* Get the string */
		SetCTitle(CItem,sTemp);					/* Set the string */
	}
	if (EnableIt)								/* See if enable or disable the zone */
		HiliteControl (CItem,0);				/* Enable the zone */
	else
		HiliteControl (CItem,255);				/* Dim the zone */
		if (SetTheMax)
	SetCtlMax(CItem,12345);						/* Set the flag to the CDEF */
	if (ShowIt)
		ShowControl(CItem);						/* Show it to activate it */
}

/* ======================================================= */

/* Routine: CenterOnColorScreen */
/* Purpose: Center a window onto the deepest color screen */

void CenterOnColorScreen (WindowPtr theWindow )
{
Rect		tempRect,gdRect,temp2Rect;				/* Temporary rectangle */
GDHandle	theDevice;								/* Graphic device */

if (theWindow != NIL) {	/* Make sure it is safe to go ahead */
	SetRect(&tempRect,-32000,-32000,32000,32000);	/* Look in all directions */
	theDevice = GetMaxDevice(&tempRect);			/* Get the deepest device */

	if (theDevice != NIL)							/* Make sure we got a device */
		{
		gdRect = (*theDevice)->gdRect;				/* Get the device rect */
		temp2Rect = theWindow->portRect;			/* Get the window rect */

		tempRect.top = gdRect.top + ((gdRect.bottom-gdRect.top)/2) - ((temp2Rect.bottom-temp2Rect.top)/2);
		tempRect.left = gdRect.left + ((gdRect.right-gdRect.left)/2) - ((temp2Rect.right-temp2Rect.left)/2);
		MoveWindow(theWindow, tempRect.left, tempRect.top, TRUE);/* Move the window*/
		SetPort(theWindow); 						/* Prepare to write into our window */
		}										/* End of IF */
	}											/* End of IF */
}

/* ======================================================= */

/* Routine: GetDeepestColorScreenRect */
/* Purpose: Get the deepest color screen rect */

void GetDeepestColorScreenRect(Rect *DeepRect )
{
Rect		tempRect;						/* Temporary rectangle */
GDHandle	theDevice;						/* Graphic device */

	*DeepRect = qd.screenBits.bounds;		/* Default to the main screen */
	theDevice = NIL;						/* Default to no device */
	SetRect(&tempRect,-32000,-32000,32000,32000);	/* Look in all directions */
	theDevice = GetMaxDevice(&tempRect);	/* Get the deepest device */

	if (theDevice != NIL) {					/* Make sure we got a device */
		*DeepRect = (*theDevice)->gdRect;	/* Get the device rect */
	}
}
/* ======================================================= */
