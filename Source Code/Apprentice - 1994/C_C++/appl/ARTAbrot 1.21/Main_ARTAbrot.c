/* Main_ARTAbrot */

/* Program name:  Main_ARTAbrot   
 Function:  This is the main module for this program.  
History: 8/18/93 Original by George Warner
   */

#include <stdio.h>
#include <stdlib.h>

#include "ComUtil_ARTAbrot.h"	/* Common */

#include "AB_Alert.h"	/* Alert */
#include "About_ARTAbrot.h"	/* Modeless Dialog */
#include "BrotCode.h"
#include "Enter_Coordinates.h"	/* Modeless Dialog */
#include "ARTAbrot.h"	/* Window */
#include "Menu_ARTAbrot.h"	/* Menus */

#define gestaltRealtimeMgrAttr 'rtmr'
enum  {
    gestaltRealtimeMgrPresent   = 0
};

Boolean		DoIt;								/* Flag saying an event is ready */
short		code;								/* Determine event type */
WindowPtr	whichWindow;						/* See which window for event */
long		mResult;							/* Menu list and item selected values */
short		theMenu,theItem;					/* Menu list and item selected */
Boolean		Is_A_Dialog;						/* Flag for modless dialogs */
short		charCode, itemHit;					/* For modeless dialogs*/
char		ch;									/* Key pressed in Ascii */
Boolean		DoTheModelessEvent, CmdDown;		/* For modeless dialogs*/
WindowPeek	thePeeked;							/* For modeless dialogs*/

extern int processing_brot;

/* Handle key strokes */
static void DoKeyEvent(void);

/* Handle a diskette inserted */
static void DoDiskEvent(void);

/* Handle a window being resized */
static void DoGrow(WindowPtr whichWindow);

/* Handle a window being dragged */
static void DoDrag(WindowPtr whichWindow);

/* Handle a window goaway box */
static void DoGoAway(WindowPtr whichWindow);

/* Handle an update to the window */
static void DoUpdate(void);


/* MAIN entry point */
void main(void);

/* Handle the OS event */
void DoOSEvent(EventRecord *myEvent);


Boolean HasARTA(void)
{
OSErr error;
long response;

	error = Gestalt(gestaltRealtimeMgrAttr, &response);
	if (error)
		return(0);
	if (response & (1 << gestaltRealtimeMgrPresent))
		return(1);
	else
		return(0);
}

/* Routine: DoOSEvent */
/* Purpose: Handle DoOSEvents */

void DoOSEvent(EventRecord *myEvent)
{

if (((myEvent->message & osEvtMessageMask) >> 24) == suspendResumeMessage)/*  See which  */
	{
	if ((myEvent->message & resumeFlag) == 0)		/* Suspend */
		{
		InTheForeground = FALSE;
		}
	else
		{
		InTheForeground = TRUE;
		}										/* End of IF */
	}											/* End of IF */
}

/* ======================================================= */

/* ======================================================= */

/* Routine: WNEIsImplemented */
/* Purpose: See if the MultiFinder trap, WaitNextEvent, is available */

Boolean IsWNEIsImplemented()				/* See if WaitNextEvent is available */
{
#define	WNETrapNumber	0xA860				/* The expected trap number */
#define	kGestaltTrapID	0xA1AD				/* The expected trap number */

SysEnvRec	theWorld;						/* Environment record */
OSErr	discardError;						/* Error code returned */
Boolean	theWNEIsImplemented;				/* Value to return */
long	result;								/* Value returned */

	Black_ForeColor.red = 0x0000;  Black_ForeColor.green = 0x0000;	Black_ForeColor.blue = 0x0000;  /* Get black color */
	White_BackColor.red = 0xFFFF;  White_BackColor.green = 0xFFFF;  White_BackColor.blue = 0xFFFF;  /* Get white color */

	HasColorQD = FALSE;						/* Init to no color QuickDraw */
	HasFPU = FALSE;							/* Init to no floating point chip */
	HasAppleEvents = FALSE;					/* Whether AppleEvents are available */
	HasAliasMgr = FALSE;					/* Whether AliasMgr is available */
	HasEditionMgr = FALSE;					/* Whether EditionMgr is available */
	HasGestalt = FALSE;						/* Whether Gestalt is available */
	HasNewStdFile = FALSE;					/* Whether HasNewStdFile is available */
	HasPPCToolbox = FALSE;					/* Whether PPCToolbox is available */
	Has32BitQuickDraw = FALSE;				/* Whether 32Bit QuickDraw is available */
	InTheForeground = TRUE;					/* Init to a foreground app */
	discardError = SysEnvirons(1, &theWorld);	/* Check how old this system is */
	if (theWorld.machineType < 0) {			/* Negative means really old */
		theWNEIsImplemented = FALSE;		/* Really old ROMs, no WNE possible */
	}
	else {
		theWNEIsImplemented = CheckTrapAvailable(WNETrapNumber, ToolTrap);/* See if trap is there */
		HasColorQD = theWorld.hasColorQD;	/* Flag for Color QuickDraw being available */
		HasFPU = theWorld.hasFPU;			/* Flag for Floating Point Math Chip being available */
		HasGestalt = CheckTrapAvailable(kGestaltTrapID, ToolTrap);	/* Whether Gestalt is available */
		if (HasGestalt) {					/* Do if Gestalt is available */
			discardError = Gestalt(gestaltAliasMgrAttr,&result);
			if ((discardError == 0) && ((result & (0x00000001 << gestaltAliasMgrPresent)) != 0))
				HasAliasMgr = TRUE;

			discardError = Gestalt(gestaltEditionMgrAttr,&result);
			if ((discardError == 0) && ((result & (0x00000001 << gestaltEditionMgrPresent)) != 0))
				HasEditionMgr = TRUE;

			discardError = Gestalt(gestaltAppleEventsAttr,&result);
			if ((discardError == 0) && ((result & (0x00000001 << gestaltAppleEventsPresent)) != 0))
				HasAppleEvents = TRUE;

			discardError = Gestalt(gestaltPPCToolboxAttr,&result);
			if ((discardError == 0) && ((result & (0x00000001 << gestaltPPCToolboxPresent)) != 0))
				HasPPCToolbox = TRUE;

			discardError = Gestalt(gestaltQuickdrawVersion,&result);
			if ((discardError == 0) && ((result & gestalt32BitQD) != 0))
				Has32BitQuickDraw = TRUE;

			discardError = Gestalt(gestaltStandardFileAttr,&result);
			if ((discardError == 0) && ((result & (0x00000001 << gestaltStandardFile58)) != 0))
				HasNewStdFile = TRUE;
		}
	}
	return(theWNEIsImplemented);
}

/* ======================================================= */

/* Routine: Handle_User_Event */
/* Purpose: Check for user events */

void Handle_User_Event()							/* Check for user events */
{
UserEventRec	TheUserEvent;						/* The user event */

GetUserEvent(&TheUserEvent);						/* Check for any user events */
if (TheUserEvent.ID != UserEvent_None)				/* Only do if we have any */
	{

	switch (TheUserEvent.ID)						/* Key off the Event ID */
		{
		case UserEvent_Open_Window: 			/* Open a Window or Modeless dialog */
			switch (TheUserEvent.ID2)				/* Do the appropiate window */
				{
				case Res_W_ARTAbrot: 
					Open_ARTAbrot();		/* Open this window */
					break;
				case Res_MD_About_ARTAbrot:
					Open_About_ARTAbrot();/* Open this modeless dialog */
					break;
				case Res_MD_Enter_Coordinat:
					Open_Enter_Coordinat();/* Open this modeless dialog */
					break;
				default:						/* Handle others */
					break;
				}								/* End of the switch */
			break;

		case UserEvent_Close_Window: 			/* Close a Window or Modeless dialog */
			switch (TheUserEvent.ID2) {			/* Do the appropiate window */
				case Res_W_ARTAbrot:
					Close_ARTAbrot(WPtr_ARTAbrot);/* Close this window */
					break;
				case Res_MD_About_ARTAbrot:
					Close_About_ARTAbrot(WPtr_About_ARTAbrot);/* Close this modeless dialog */
					break;
				case Res_MD_Enter_Coordinat:
					Close_Enter_Coordinat(WPtr_Enter_Coordinat);/* Close this modeless dialog */
					break;
				default:						/* Handle others */
					break;
				}								/* End of the switch */

			break;
		default:								/* Not standard, must be program specific */
			break;
		}										/* End of switch */
	}											/* End of IF */
}

/* ======================================================= */

/* Routine: DoKeyEvent */
/* Purpose: Handle a key pressed */

static void DoKeyEvent()							/* Handle key presses */
{
short	charCode;									/* Key code */
char	ch;										/* Key pressed in Ascii */
long	mResult;									/* Menu list and item, if a command key */
short	theMenu,theItem;							/* Menu list and item, if command key */

	charCode = myEvent.message & charCodeMask;		/* Get the character */
	ch = (char)charCode;							/* Change it to ASCII */
	
	if ((myEvent.modifiers & cmdKey) != 0) {		/* See if Command key is down */
		mResult = MenuKey(ch);						/* See if a menu selection */
		theMenu = HiWord(mResult);					/* Get the menu list number */
		theItem = LoWord(mResult);					/* Get the menu item number */
		if (theMenu != 0)							/* See if a list was selected */
			Handle_My_Menu(theMenu, theItem);		/* Do the menu selection */
	
		if (((ch == 'x') || (ch == 'X')) && (theInput != NIL))/* See if a standard Cut */
			TECut(theInput);						/* Handle a Cut in a TE area */
		if (((ch == 'c') || (ch == 'C')) && (theInput != NIL))/* See if a standard Copy */
			TECopy(theInput);						/* Handle a Copy in a TE area */
		if (((ch == 'v')  ||  (ch == 'V')) && (theInput != NIL))/* See if a standard Paste */
			TEPaste(theInput);						/* Handle a Paste in a TE area */
		if ((ch = '.') && (processing_brot)) {
			finish_brot();
		}
	}
	else if (theInput != NIL)
		TEKey(ch,theInput);						/* Place the normal key stroke */

}

/* ======================================================= */

/* Routine: DoDiskEvent */
/* Purpose: Handle a diskette inserted */

static void DoDiskEvent()							/* Handle disk inserted */
{
short	theError;									/* Error returned from mount */

if (HiWord(myEvent.message) != noErr)				/* See if a diskette mount error */
	{
	myEvent.where.h = ((qd.screenBits.bounds.right - qd.screenBits.bounds.left) / 2) - (304 / 2);/* Center horz */
	myEvent.where.v = ((qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) / 3) - (104 / 2);/* Top 3ed vertically */
	InitCursor();									/* Make sure it has an arrow cursor */
	theError = DIBadMount(myEvent.where, myEvent.message);/* Let the OS handle the diskette */
	}											/* End of IF */

}

/* ======================================================= */

/* Routine: DoGrow */
/* Purpose: Handle a window resize */

static void DoGrow(WindowPtr whichWindow)
{
Rect	OldRect;									/* Window rect before the grow */
Point	myPt;										/* Point for tracking grow box */
Rect	GrowRect;									/* Set the grow bounds */
long	mResult;									/* Result from the grow */
long	theRefCon;									/* Refcon with layer masked off */

if (!Doing_MovableModal)							/* Select proper window */
	{
	if (whichWindow != nil)						/* See if we have a legal window */
		{
		SetPort(whichWindow);						/* Get ready to draw in this window */

		myPt = myEvent.where;						/* Get mouse position */
		GlobalToLocal(&myPt);						/* Make it relative */

		OldRect = whichWindow->portRect;			/* Save the rect before resizing */


		theRefCon = GetWRefCon(whichWindow);		/* Get the refcon */
		switch (theRefCon) {							/* Do the appropriate window */
			case Res_W_ARTAbrot:
				/* Don't allow resize if we're running. */
				if (!processing_brot) {
//					SetRect(&GrowRect, 4, 4, (qd.screenBits.bounds.right - qd.screenBits.bounds.left)-4,  (qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) - 4);/* l,t,r,b */
					SetRect(&GrowRect, 4, 4, (1280)-4,  (qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) - 4);/* l,t,r,b */
					mResult = GrowWindow(whichWindow, myEvent.where, &GrowRect);/* Grow it */
					SizeWindow(whichWindow, LoWord(mResult), HiWord(mResult), true);/* Resize to result */
					Resized_ARTAbrot(whichWindow);	/* Resized this window */
				}
				break;
			default:							/* allow other buttons, trap for debug */
				SetRect(&GrowRect, 4, 4, (qd.screenBits.bounds.right - qd.screenBits.bounds.left)-4,  (qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) - 4);/* l,t,r,b */
				mResult = GrowWindow(whichWindow, myEvent.where, &GrowRect);/* Grow it */
				SizeWindow(whichWindow, LoWord(mResult), HiWord(mResult), true);/* Resize to result */
				break;
		}

		SetPort(whichWindow);						/* Get ready to draw in this window */

		myPt.h = whichWindow->portRect.right - whichWindow->portRect.left;/* Local right edge */
		myPt.v = whichWindow->portRect.bottom - whichWindow->portRect.top;/* Local bottom edge */

		SetRect(&GrowRect, 0, OldRect.bottom - 15, OldRect.right + 15, OldRect.bottom + 15);/* Position for horz scrollbar area */
		EraseRect(&GrowRect);						/* Erase old area */
		InvalRect(&GrowRect);						/* Flag us to update it */

		SetRect(&GrowRect, OldRect.right - 15, 0, OldRect.right + 15, OldRect.bottom + 15);/* Position for vert scrollbar area */
		EraseRect(&GrowRect);						/* Erase old area */
		InvalRect(&GrowRect);						/* Flag us to update it */

		DrawGrowIcon(whichWindow);					/* Draw the grow Icon again */
		}										/* End of IF */
	}											/* End of IF */
}



/* Routine: DoDrag */
/* Purpose: Drag a window around */

static void DoDrag(WindowPtr whichWindow)
{
Rect	OldRect;							/* Window rect before the drag */
Rect	tempRect;							/* temporary rect */
long	theRefCon;							/* Refcon with layer masked off */

	if ((!Doing_MovableModal) || (Doing_MovableModal && (whichWindow == FrontWindow()))) {	/* See if OK to drag */
		OldRect = whichWindow->portRect;	/* Save the rect before resizing */

		tempRect = qd.screenBits.bounds;	/* Get screen area,  l,t,r,b, drag area */
		SetRect(&tempRect,tempRect.left+4,tempRect.top+4,tempRect.right-4,tempRect.bottom - 4);
		DragWindow(whichWindow, myEvent.where, &tempRect);	/* Drag the window */

		theRefCon = GetWRefCon(whichWindow);	/* Get the refcon */
		switch (theRefCon) {				/* Do the appropiate window */
			case Res_W_ARTAbrot:
				Moved_ARTAbrot(whichWindow);	/* Moved this window */
				break;
			case Res_MD_About_ARTAbrot:
				Moved_About_ARTAbrot(whichWindow);	/* Moved this modeless dialog */
				break;
			case Res_MD_Enter_Coordinat:
				Moved_Enter_Coordinat(whichWindow);	/* Moved this modeless dialog */
				break;
			default:						/* allow other buttons, trap for debug */
				break;	
		}	
	}	
}



/* Routine: DoGoAway */
/* Purpose: Close a window */

static void DoGoAway(WindowPtr whichWindow)
{
long	theRefCon;									/* Refcon with layer masked off */

	if (!Doing_MovableModal) {							/* Select proper window */
		if (TrackGoAway(whichWindow,myEvent.where)) {	/* See if mouse released in GoAway box */
			theRefCon = GetWRefCon(whichWindow);		/* Get the refcon */
			switch (theRefCon) {							/* Do the appropiate window */
				case Res_W_ARTAbrot:
					Close_ARTAbrot(whichWindow);		/* Close this window */
					break;
				case Res_MD_About_ARTAbrot:
					Close_About_ARTAbrot(whichWindow);/* Close this modeless dialog */
					break;
				case Res_MD_Enter_Coordinat:
					Close_Enter_Coordinat(whichWindow);/* Close this modeless dialog */
					break;
				default:							/* allow other buttons, trap for debug */
					break;	
			}	
		}	
	}	
}



/* Routine: DoInContent */
/* Purpose: Pressed in the content area */

static void DoInContent(WindowPtr whichWindow,EventRecord *myEvent)
{
long	theRefCon;							/* Refcon with layer masked off */

	if (!Doing_MovableModal) {				/* Select proper window */
		if (whichWindow != FrontWindow()) {	/* See if already selected or not, in front if selected */
			SelectWindow(whichWindow);		/* Select this window to make it active */
		}
		else {
			SetPort(whichWindow);			/* Get ready to draw in this window */
			theRefCon = GetWRefCon(whichWindow);	/* Get the refcon */
			switch (theRefCon) {			/* Do the appropiate window */
				case Res_W_ARTAbrot:
					Do_ARTAbrot(myEvent);	/* Handle this window */
					break;
				default:					/* allow other buttons, trap for debug */
					break;
			}
		}
	}
}



/* Routine: DoUpdate */
/* Purpose: Got an update event */

static void DoUpdate()
{
WindowPtr	whichWindow;					/* See which window for event */
long	theRefCon;							/* Refcon with layer masked off */

	whichWindow = (WindowPtr)myEvent.message;	/* Get the window the update is for */

	BeginUpdate(whichWindow);				/* Set the clipping to the update area */
	theRefCon = GetWRefCon(whichWindow);	/* Get the refcon */
	switch (theRefCon) {					/* Do the appropiate window */
		case Res_W_ARTAbrot:
			Update_ARTAbrot(whichWindow);	/* Update this window */
			break;
		default:							/* allow other buttons, trap for debug */
			break;
	}
	EndUpdate(whichWindow);					/* Return to normal clipping area */
}



/* Routine: DoActivate */
/* Purpose: Got an activate or deactivate event */

void DoActivate()
{
WindowPtr	whichWindow;				/* See which window for event */
Boolean	Do_An_Activate;					/* Flag to pass */
long	theRefCon;						/* Refcon with layer masked off */

	whichWindow = (WindowPtr)myEvent.message;	/* Get the window the update is for */

	Do_An_Activate =  ((myEvent.modifiers & 0x0001) != 0);	/* Make sure it is Activate and not DeActivate */
	theRefCon = GetWRefCon(whichWindow);	/* Get the refcon */
	switch (theRefCon) {				/* Do the appropiate window */
		case Res_W_ARTAbrot:
			Activate_ARTAbrot(whichWindow,Do_An_Activate);	/* Activate or deactivate this window */
			break;
		default:						/* allow other buttons, trap for debug */
			break;
	}
}



void main()
{
char tmp_string[256];

	MoreMasters();						/* This reserves space for more handles */
	MaxApplZone();						/* Give us room for memory allocation */
	InitGraf((Ptr) &qd.thePort);		/* Quickdraw Init */
	InitFonts();						/* Font manager init */
	InitWindows();						/* Window manager init */
	InitMenus();						/* Menu manager init */
	TEInit();							/* Text edit init */
	InitDialogs(nil);					/* Dialog manager */

	FlushEvents(everyEvent , 0);		/* Clear out all events */
	InitCursor();						/* Make an arrow cursor */

	doneFlag = FALSE;					/* Do not exit program yet */

	Init_My_Menus();					/* Initialize menu bar */

	theInput = nil;						/* Init to no text edit selection active */

	SleepValue = 0;						/* Set sleep value */
	WNE = IsWNEIsImplemented();			/* See if WaitNextEvent is available */

	/* We need certain minimum conditions.  We'll check here. */
	if (!HasGestalt) {
		sprintf(tmp_string, "Sorry, Gestalt required for this program.");
		AB_Alert(tmp_string);
		doneFlag = TRUE;				/* Exit program. */
	}

	if (doneFlag == FALSE) {
		if (!HasColorQD) {
			sprintf(tmp_string, "Sorry, Color Quickdraw required for this program.");
			AB_Alert(tmp_string);
			doneFlag = TRUE;			/* Exit program. */
		}
	}

	if (doneFlag == FALSE) {
		if (!HasARTA()) {
			sprintf(tmp_string, "Sorry, DSP3210 ARTA required for this program.");
			AB_Alert(tmp_string);
			doneFlag = TRUE;			/* Exit program. */
		}
	}

	UserEventList = nil;				/* No user events yet */

	cursorRgn = NewRgn();				/* Cursor region for WaitNextEvent */

	Init_ARTAbrot();					/* Initialize the window routines */
	Init_About_ARTAbrot();				/* Initialize the modeless dialog routines */
	I_A_Alert();						/* Initialize the alert globals */
	Init_Enter_Coordinat();				/* Initialize the modeless dialog routines */
	Doing_MovableModal = false;			/* Not currently doing a movable modal */

	if (doneFlag == FALSE) {
		new_coordinates=TRUE;
	
		/* Open a Window */
		Add_UserEvent(UserEvent_Open_Window,Res_W_ARTAbrot,0,0,nil);
	}

	do {
		if (processing_brot)
			continue_brot();			/* See if we can get a line of Mandelbrot. */
		Handle_User_Event();			/* Check for user events */
		if (theInput != NIL) {			/* See if a TE is active */
			TEIdle(theInput);			/* Blink the cursor if everything is ok */
		}
		if (WNE) {						/* See if do the MultiFinder way */
			DoIt = WaitNextEvent(everyEvent, &myEvent, SleepValue, cursorRgn);/* Wait for an event */
		}
		else {
			SystemTask();				/* For support of desk accessories */
			DoIt = GetNextEvent(everyEvent, &myEvent);/* See if an event is ready */
		}

		if (DoIt) {						/* If event then... */
			Is_A_Dialog = IsDialogEvent(&myEvent);	/* See if a modeless dialog event */
			if (Is_A_Dialog == TRUE) {				/* Handle a dialog event */
				if (myEvent.what == updateEvt) {	/* Handle the update of a Modeless Dialog */
					whichWindow = (WindowPtr)myEvent.message; /* Get the window the update is for */
					BeginUpdate(whichWindow);		/* Set update clipping area */
					Update_About_ARTAbrot(whichWindow);	/* Update the modeless dialog */
					Update_Enter_Coordinat(whichWindow);/* Update the modeless dialog */
					EndUpdate(whichWindow);			/* Return to normal clipping area */
				}
				else {
					DoTheModelessEvent = TRUE;		/* Go ahead and do it so far */

					if (myEvent.what == keyDown) { 	/* Check the key down, for a command key event */
						CmdDown = ((myEvent.modifiers / cmdKey) & 1);/* Get the command key state */
						charCode = myEvent.message & charCodeMask;/* Get the character */
						ch = (char)charCode;		/* Change it to ASCII */

						if ((charCode == 13) || (charCode == 0x03))/* CR or Enter */
							DoTheModelessEvent = TRUE;/* Handle the default selection */

						if (CmdDown != 0) {			/* Handle if the command key was down */
							mResult = MenuKey(ch);	/* See if a menu selection */
							theMenu = HiWord(mResult);	/* Get the menu list number */
							theItem = LoWord(mResult);	/* Get the menu item number */
							if (theMenu != 0)  		/* See if a list was selected */
								Handle_My_Menu(theMenu, theItem); /* Do the menu selection */

							whichWindow = FrontWindow(); /* Get the front window */
							if ((ch == 'x') || (ch == 'X'))	/* Handle a CUT */
								DlgCut(whichWindow);/* Do the dialog cut */
							if ((ch == 'c') || (ch == 'C'))	/* Handle a COPY */
								DlgCopy(whichWindow);/* Do the dialog copy */
							if ((ch == 'v') || (ch == 'V'))	/* Handle a PASTE */
								DlgPaste(whichWindow);	/* Do the dialog paste */

							DoTheModelessEvent = FALSE;/* We handled the command key */
						}
					}

					if (DoTheModelessEvent == TRUE) { 	/* Do we handle it? */
						if ((DialogSelect(&myEvent, &whichWindow, &itemHit)) || (myEvent.what == mouseDown) || (myEvent.what == keyDown)) { /* Ck if do it */
							Do_About_ARTAbrot(&myEvent,whichWindow,itemHit);	/* Handle the Modeless Dialog */
							Do_Enter_Coordinat(&myEvent,whichWindow,itemHit);	/* Handle the Modeless Dialog */
						}
					}
				}
			}
			else {
				switch (myEvent.what) {		/* Decide type of event */
					case mouseDown :		/* Mouse button pressed */
						code = FindWindow(myEvent.where, &whichWindow);/* Get which window the event happened in */

						switch (code) {		/* Decide type of event again */
							case inMenuBar:	/* In the menubar */
								mResult = MenuSelect(myEvent.where);	/* Do menu selection */
								theMenu = HiWord(mResult);	/* Get the menu list number */
								theItem = LoWord(mResult);	/* Get the menu list item number */
								Handle_My_Menu( theMenu, theItem);	/* Handle the menu */
								break;

							case inDrag:	/* In window drag area */
								DoDrag(whichWindow);	/* Go drag the window */
								break;

							case inGrow:	/* In window grow area */
								DoGrow(whichWindow);	/* Handle the growing */
								break;

							case inGoAway:	/* In window goaway area */
								DoGoAway(whichWindow);	/* Handle the goaway button */
								break;

							case inContent:	/* In window  contents */
								DoInContent(whichWindow,&myEvent);	/* Handle the hit inside a window */
								break;

							case inSysWindow:	/* See if a DA selectio */
								SystemClick(&myEvent, whichWindow);	/* Let other programs in */
								break;

							default:		/* Handle others */
								break;
						}
						break;				/* End of MouseDown */

					case keyDown:			/* Handle key inputs */
					case autoKey:			/* and auto repeats */
						DoKeyEvent();		/* Get the key and handle it */
						break;

					case updateEvt:			/* Update event for a window */
						DoUpdate();			/* Handle the update */
						break;

					case diskEvt:			/* Disk inserted event */
						DoDiskEvent();		/* Handle a disk event */
						break;

					case activateEvt:		/* Window activated event */
						DoActivate();		/* Handle the activation */
						break;

					case osEvt:				/* OS event */
						DoOSEvent(&myEvent);	/* Handle the activation */
						break;					/* End of osEvt */

					default:				/* Used for debugging, to see what other events are coming in */
						break;
				}
			}
		}
		else {
			whichWindow = FrontWindow();	/* Get the current front window */
			if (whichWindow != NIL) {		/* See if we have a window */
				thePeeked = (WindowPeek)whichWindow;	/* Peek inside, look for dialog */
				if (thePeeked->windowKind == dialogKind) {	/* DialogSelect will crash if no dialogs */
					if (DialogSelect(&myEvent, &whichWindow, &itemHit)) { /* Blink cursor in modeless TEs */
					}
				}
			}
		}
	}
	while (doneFlag == FALSE);				/* End of the event loop */
}											/* end of main */
