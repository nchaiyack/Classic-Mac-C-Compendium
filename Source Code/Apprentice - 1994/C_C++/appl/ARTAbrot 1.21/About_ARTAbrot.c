/* About_ARTAbrot */

/* File name: About_ARTAbrot */
/* Function: Handle a modeless dialog */
/*            This dialog operates like a window, it is not modal. */
/*           Opened by:    */
/*           Closed by:     */
/*           Purpose:       
History: 8/18/93 Original by George Warner */


#include "ComUtil_ARTAbrot.h"	/* Common */
#include "AB_Alert.h"	/* Alert */
#include "About_ARTAbrot.h"	/* This modeless dialog */

#pragma segment About_ARTAbrot

/* ======================================================= */


/* ======================================================= */

/* Routine: Init_About_ARTAbrot */
/* Purpose: This procedures purpose is to set the window pointer to nil, */
/* this is used to tell the other routines */

void Init_About_ARTAbrot()
{
	WPtr_About_ARTAbrot = NIL;	/* Initialize to say that the dialog is not yet active */
}

/* ======================================================= */

/* Routine: Moved_About_ARTAbrot */
/* Purpose: We were moved, possibly to another screen and screen depth */

void Moved_About_ARTAbrot(WindowPtr theWindow)/* Moved this window */
{
WindowPtr	SavePort;						/* Place to save the last port */

	if (WPtr_About_ARTAbrot == theWindow) {	/* Only do if the window is us */
		GetPort(&SavePort);					/* Save the current port */
		SetPort(theWindow);					/* Set the port to my window */

		SetPort(SavePort);					/* Restore the old port */
	}
}

/* ======================================================= */

/* Routine: Update_About_ARTAbrot */
/* Purpose: This procedures purpose is to refresh this window, update it, */
/* when we are uncovered by another window.  */

void Update_About_ARTAbrot(WindowPtr theWindow)
{
GrafPtr	SavedPort;							/* Save the current port so we can restore to it */
Rect	rTempRect, tempRect;				/* Temporary rectangle variable */
RGBColor	Saved_ForeColor;				/* Place to save colors */
RGBColor	Saved_BackColor;				/* Place to save colors */
RGBColor	DrawingColor;					/* Place to make colors */
PicHandle	Pic_Handle;							/* Handle used in displaying pictures */

	/* Only do if we are the window to update */
	if ((WPtr_About_ARTAbrot != nil) && (theWindow == WPtr_About_ARTAbrot)) {
		GetPort(&SavedPort);				/* Get the current port */
		SetPort(theWindow);					/* Point to our port for drawing in our window */

		GetForeColor(&Saved_ForeColor);		/* Save the fore color */
		GetBackColor(&Saved_BackColor);		/* Save the back color */

		DrawingColor.red = 0xFFFF;  DrawingColor.green = 0xFFFF;  DrawingColor.blue = 0x0;/* Set the color */
		RGBForeColor(&DrawingColor);		/* Set the fore color */

		Pic_Handle = GetPicture(Pict_Picture);	/* Get Picture into memory */
		SetRect(&tempRect, 0,0,300,180);		/* left,top,right,bottom */
		if (Pic_Handle != nil) { 				/* Only use handle if it is valid */
			DrawPicture(Pic_Handle, &tempRect);	/* Draw this picture */
		}

		/* Display the status lines. */
		TextFont(geneva);						/* Select the Font that we want */
		TextSize(9);
		/* Draw a string of text. */
		MoveTo(95, 80);
		DrawString("\pARTAbrot Version 1.21");
		MoveTo(95, 90);
		DrawString("\pGeorge Warner");
		MoveTo(95, 100);
		DrawString("\p03/26/94");
		MoveTo(95, 110);
		DrawString("\pwarnergt@aloft.att.com");

		TextSize(12);
		TextFont(systemFont);				/* Select the Font that we want */
		TextFace(0);						/* Select the style that we want */

		RGBForeColor(&Saved_ForeColor);		/* Restore the fore color */
		RGBBackColor(&Saved_BackColor);		/* Restore the back color */

		DrawDialog(theWindow);				/* Draw the rest of the controls */
		SetPort(SavedPort);					/* Restore the port that we saved at the start */
	}
}

/* ======================================================= */

/* Routine: Open_About_ARTAbrot */
/* Purpose: This procedures purpose is to open this window and set all */
/* of the initial conditions, such as default edit text. */

void Open_About_ARTAbrot()
{
Rect	tempRect;    							/* Temporary rectangle */
int top, left;

	if (WPtr_About_ARTAbrot == NIL) {
		WPtr_About_ARTAbrot = GetNewDialog(Res_MD_About_ARTAbrot, NIL,  (WindowPtr)-1 );/* Bring in the dialog resource */

		/* Get window size, we will now center it */
		tempRect.top = WPtr_About_ARTAbrot->portRect.top;
		tempRect.left = WPtr_About_ARTAbrot->portRect.left;
		tempRect.bottom = WPtr_About_ARTAbrot->portRect.bottom;
		tempRect.right = WPtr_About_ARTAbrot->portRect.right;
		top = ((qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) - (tempRect.bottom - tempRect.top)) / 2;/* Center vert */
		left = ((qd.screenBits.bounds.right - qd.screenBits.bounds.left) - (tempRect.right - tempRect.left)) / 2;/* Center Horz */
		/* Now move the window to the proper position */
		MoveWindow(WPtr_About_ARTAbrot, left, top, true);

		SetPort(WPtr_About_ARTAbrot);		/* Prepare to add conditional text */
		Doing_MovableModal = true;			/* We are now in the movable modal mode */

		ShowWindow(WPtr_About_ARTAbrot);	/* Open a dialog box */
		SelectWindow(WPtr_About_ARTAbrot);	/* Lets see it */
	}
	else
		SelectWindow(WPtr_About_ARTAbrot);	/* Lets see it */
}

/* ======================================================= */

/* Routine: Close_About_ARTAbrot */
/* Purpose: This procedures purpose is to close this window and clear */
/* the window pointer variable */

void Close_About_ARTAbrot(WindowPtr theWindow)
{
	/* Only close if it is us and we were open */
	if ((WPtr_About_ARTAbrot != NIL) && (theWindow == WPtr_About_ARTAbrot)) {
		Doing_MovableModal = false;		/* We are now out of the movable modal mode */
		DisposDialog(theWindow);		/* Close on the screen and Flush the dialog out of memory */
		WPtr_About_ARTAbrot = nil;		/* Make sure our other routines know that we are closed */
	}
}

/* ======================================================= */


/* Routine: Do_About_ARTAbrot */
/* Purpose: This procedures purpose is to handle all actions, such as buttons being pressed. */
/* This is the real meat of this unit and is where the code is for acting upon the users actions. */

void Do_About_ARTAbrot(EventRecord *theEvent,WindowPtr theWindow,short itemHit)
{
Point	myPt;								/* For the local mouse position */
short	DType;								/* Type of dialog item */
Handle	DItem;								/* Handle to the dialog item */
Rect	tempRect;							/* Temporary rectangle */
ControlHandle	CItem;						/* Control handle */

	if ((theEvent->what == mouseDown) && (WPtr_About_ARTAbrot != nil)) {
		SetPort(WPtr_About_ARTAbrot);		/* Set the port to our dialog */
		myPt = theEvent->where;				/* Get the position where the mouse was pressed */
		GlobalToLocal(&myPt);				/* Change from global to local location */
	}

	if ((WPtr_About_ARTAbrot != nil)  && (WPtr_About_ARTAbrot  == theWindow)) {
		if ((theEvent->what == keyDown) || (theEvent->what == mouseDown)) {
			/* Close the window when a key is hit or mouse button is pressed. */
			Add_UserEvent(UserEvent_Close_Window,Res_MD_About_ARTAbrot,0,0,nil);
			Play_The_Sound(Snd_Bart__Cooool);	/* Play my sound */
		}
	}
}
