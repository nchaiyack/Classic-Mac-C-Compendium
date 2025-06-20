/************************************************************************/
/*																		*/
/*	FILE:		Enter_Coordinates.c -- Code for entering coordinates	*/
/*					to compute Mandelbrots.								*/
/*																		*/
/*	PURPOSE:	Code to get user input.									*/
/*																		*/
/*	AUTHOR:		George T. Warner										*/
/*																		*/
/*	REVISIONS:															*/
/*	08/22/93	First version.											*/
/*																		*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ComUtil_ARTAbrot.h"	/* Common */

#include "AB_Alert.h"	/* Alert */
#include "Enter_Coordinates.h"	/* This modeless dialog */

#pragma segment Enter_Coordinat

static Boolean		ExitDialog;					/* Flag to exit dialog */

/* ======================================================= */

/* Routine: Init_Enter_Coordinat */
/* Purpose: This procedures purpose is to set the window pointer to nil, */
/* this is used to tell the other routines */

void Init_Enter_Coordinat()
{
	WPtr_Enter_Coordinat = NIL;						/* Initialize to say that the dialog is not yet active */
}

/* ======================================================= */

/* Routine: Moved_Enter_Coordinat */
/* Purpose: We were moved, possibly to another screen and screen depth */

void Moved_Enter_Coordinat(WindowPtr theWindow)/* Moved this window */
{
WindowPtr	SavePort;					/* Place to save the last port */

	if (WPtr_Enter_Coordinat == theWindow) {	/* Only do if the window is us */
		GetPort(&SavePort);					/* Save the current port */
		SetPort(theWindow);					/* Set the port to my window */

		SetPort(SavePort);					/* Restore the old port */
	}
}

/* ======================================================= */

/* Routine: Update_Enter_Coordinat */
/* Purpose: This procedures purpose is to refresh this window, update it, */
/* when we are uncovered by another window.  */

void Update_Enter_Coordinat(WindowPtr theWindow)
{
GrafPtr	SavedPort;								/* Save the current port so we can restore to it */
Rect	tempRect;									/* Temporary rectangle variable */
Rect	rTempRect;									/* Temporary rectangle variable */
short	DType;									/* Type of dialog item */
Handle	DItem;									/* Handle to the dialog item */
RGBColor	Saved_ForeColor;						/* Place to save colors */
RGBColor	Saved_BackColor;						/* Place to save colors */
RGBColor	DrawingColor;							/* Place to make colors */

	/* Only do if we are the window to update */
	if ((WPtr_Enter_Coordinat != nil) && (theWindow == WPtr_Enter_Coordinat)) {
		GetPort(&SavedPort);							/* Get the current port */
		SetPort(theWindow);							/* Point to our port for drawing in our window */
		GetForeColor(&Saved_ForeColor);			/* Save the fore color */
		GetBackColor(&Saved_BackColor);			/* Save the back color */

		RGBForeColor(&Black_ForeColor);			/* Set the fore color to Black */
		RGBBackColor(&White_BackColor);			/* Set the back color to White */

		GetDItem(theWindow,Res_Dlg_OK,&DType,&DItem,&tempRect);/* Get the item handle */
		PenSize(3, 3);									/* Change pen to draw thick default outline */
		InsetRect(&tempRect, -4, -4);					/* Draw outside the button by 1 pixel */
		FrameRoundRect(&tempRect, 16, 16);				/* Draw the outline */
		PenSize(1, 1); 								/* Restore the pen size to the default value */

		DrawingColor.red = 0xFFFF;  DrawingColor.green = 0x0000;  DrawingColor.blue = 0x0000;/* Set the color */
		RGBForeColor(&DrawingColor);				/* Set the fore color */

		/* Draw a string of text, Static Text */
		SetRect(&rTempRect, 44,108,157,123);
		GetIndString(sTemp,Res_Dlg_Static_Text4,1);	/* Get the string */
		TextBox(&sTemp[1], sTemp[0], &rTempRect,teJustLeft);

		RGBForeColor(&Black_ForeColor);			/* Set the fore color to Black */
		DrawingColor.red = 0xFFFF;  DrawingColor.green = 0x0000;  DrawingColor.blue = 0x0000;/* Set the color */
		RGBForeColor(&DrawingColor);				/* Set the fore color */
		/* Draw a string of text, Static Text */
		SetRect(&rTempRect, 109,78,159,93);
		GetIndString(sTemp,Res_Dlg_Static_Text3,1);	/* Get the string */
		TextBox(&sTemp[1], sTemp[0], &rTempRect,teJustLeft);

		RGBForeColor(&Black_ForeColor);			/* Set the fore color to Black */
		DrawingColor.red = 0xFFFF;  DrawingColor.green = 0x0000;  DrawingColor.blue = 0x0000;/* Set the color */
		RGBForeColor(&DrawingColor);				/* Set the fore color */
		/* Draw a string of text, Static Text */
		SetRect(&rTempRect, 15,48,155,63);
		GetIndString(sTemp,Res_Dlg_Static_Text2,1);	/* Get the string */
		TextBox(&sTemp[1], sTemp[0], &rTempRect,teJustLeft);

		RGBForeColor(&Black_ForeColor);			/* Set the fore color to Black */
		DrawingColor.red = 0xFFFF;  DrawingColor.green = 0x0000;  DrawingColor.blue = 0x0000;/* Set the color */
		RGBForeColor(&DrawingColor);				/* Set the fore color */
		/* Draw a string of text, Static Text */
		SetRect(&rTempRect, 15,18,155,33);
		GetIndString(sTemp,Res_Dlg_Static_Text,1);		/* Get the string */
		TextBox(&sTemp[1], sTemp[0], &rTempRect,teJustLeft);

		RGBForeColor(&Black_ForeColor);	/* Set the fore color to Black */
		TextSize(12);
		TextFont(systemFont);			/* Select the Font that we want */
		TextFace(0);					/* Select the style that we want */

		RGBForeColor(&Saved_ForeColor);	/* Restore the fore color */
		RGBBackColor(&Saved_BackColor);	/* Restore the back color */

		DrawDialog(theWindow);			/* Draw the rest of the controls */
		SetPort(SavedPort);				/* Restore the port that we saved at the start */
	}
}


/* Routine: Open_Enter_Coordinat */
/* Purpose: This procedures purpose is to open this window and set all */
/* of the initial conditions, such as default edit text. */

void Open_Enter_Coordinat()
{
TEHandle	ThisEditText; 
DialogPeek		TheDialogPtr;
Rect	tempRect;									/* Temporary rectangle variable */
short	DType;									/* Type of dialog item */
unsigned char temp_string[256];
Handle	DItem;									/* Handle to the dialog item */

	if (WPtr_Enter_Coordinat == NIL) {
		WPtr_Enter_Coordinat = GetNewDialog(Res_MD_Enter_Coordinat, NIL,  (WindowPtr)-1 );/* Bring in the dialog resource */
		SetPort(WPtr_Enter_Coordinat);					/* Prepare to add conditional text */
		Doing_MovableModal = true;						/* We are now in the movable modal mode */
		TheDialogPtr = (DialogPeek)WPtr_Enter_Coordinat;/* Get to the inner record */
		ThisEditText = TheDialogPtr->textH;			/* Get to the TE record */
		HLock((Handle)ThisEditText);					/* Lock it for safety */
		(*ThisEditText)->txSize = 12;					/* TE Point size */
		TextSize(12);									/* Window Point size */
		(*ThisEditText)->txFont = systemFont;			/* TE Font ID */
		TextFont(systemFont);							/* Window Font ID */
		(*ThisEditText)->txFont = 0;					/* TE Font ID */
		(*ThisEditText)->fontAscent = 12;				/* Font ascent */
		(*ThisEditText)->lineHeight = 12 + 3 + 1;		/* Font ascent + descent + leading */
		HUnlock((Handle)ThisEditText);					/* UnLock the handle when done */


		/* Setup initial conditions */
		/* Set initial X coordinate. */
		GetDItem(WPtr_Enter_Coordinat,Res_Dlg_Edit_Text,&DType,&DItem,&tempRect);/* Get the item handle */
		sprintf((char *)&temp_string[1],"%f",fxcenter);	/* Get the string to display */
		temp_string[0] = strlen((char *)&temp_string[1]);
		SetIText(DItem,temp_string);							/* Set the default text string */

		/* Set initial Y coordinate. */
		GetDItem(WPtr_Enter_Coordinat,Res_Dlg_Edit_Text2,&DType,&DItem,&tempRect);/* Get the item handle */
		sprintf((char *)&temp_string[1],"%f",fycenter);	/* Get the string to display */
		temp_string[0] = strlen((char *)&temp_string[1]);
		SetIText(DItem,temp_string);							/* Set the default text string */

		/* Set initial width. */
		GetDItem(WPtr_Enter_Coordinat,Res_Dlg_Edit_Text3,&DType,&DItem,&tempRect);/* Get the item handle */
		sprintf((char *)&temp_string[1],"%f",fwidth);	/* Get the string to display */
		temp_string[0] = strlen((char *)&temp_string[1]);
		SetIText(DItem,temp_string);							/* Set the default text string */

		/* Set initial number of iterations. */
		GetDItem(WPtr_Enter_Coordinat,Res_Dlg_Edit_Text4,&DType,&DItem,&tempRect);/* Get the item handle */
		sprintf((char *)&temp_string[1],"%d",fiters);	/* Get the string to display */
		temp_string[0] = strlen((char *)&temp_string[1]);
		SetIText(DItem,temp_string);							/* Set the default text string */

		SelIText(WPtr_Enter_Coordinat,Res_Dlg_Edit_Text4,0,300);

		ShowWindow(WPtr_Enter_Coordinat);		/* Open a dialog box */
		SelectWindow(WPtr_Enter_Coordinat);		/* Lets see it */
	}
	else
		SelectWindow(WPtr_Enter_Coordinat);		/* Lets see it */
}

/* ======================================================= */

/* Routine: Close_Enter_Coordinat */
/* Purpose: This procedures purpose is to close this window and clear */
/* the window pointer variable */

void Close_Enter_Coordinat(WindowPtr theWindow)
{
	/* Only close if it is us and we were open */
	if ((WPtr_Enter_Coordinat != NIL) && (theWindow == WPtr_Enter_Coordinat)) {
		Doing_MovableModal = false;		/* We are now out of the movable modal mode */
		DisposDialog(theWindow);		/* Close on the screen and Flush the dialog out of memory */
		WPtr_Enter_Coordinat = nil;		/* Make sure our other routines know that we are closed */
	}
}

/* ======================================================= */


/* Routine: Do_Enter_Coordinat */
/* Purpose: This procedures purpose is to handle all actions, such as buttons being pressed. */
/* This is the real meat of this unit and is where the code is for acting upon the users actions. */

void Do_Enter_Coordinat(EventRecord *theEvent,WindowPtr theWindow,short itemHit)
{
Point	myPt;						/* For the local mouse position */
Boolean	ExitDialog;					/* Flag to close this dialog */
short	DType;						/* Type of dialog item */
Handle	DItem;						/* Handle to the dialog item */
Rect	tempRect;					/* Temporary rectangle */
ControlHandle	CItem;				/* Control handle */
short	temp;						/* temp integer */
unsigned char inputText[256];
int		text_length;

	ExitDialog = false;				/* Do not close the dialog yet */
	if ((theEvent->what == mouseDown) && (WPtr_Enter_Coordinat != nil)) {
		SetPort(WPtr_Enter_Coordinat);	/* Set the port to our dialog */
		myPt = theEvent->where;		/* Get the position where the mouse was pressed */
		GlobalToLocal(&myPt);		/* Change from global to local location */
	}

	if ((WPtr_Enter_Coordinat != nil)  && (WPtr_Enter_Coordinat  == theWindow)) {
		if (theEvent->what == keyDown) {
			itemHit = 0;						/* Default to no item hit */
			temp = (short)theEvent->message & charCodeMask;/* Get the character pressed */
			if ((temp == 13) || (temp == 3)) {	/* See if CR or Enter */
				itemHit = 1;					/* Default item hit */
			}
		}

		if ((WPtr_Enter_Coordinat != NIL)  && (WPtr_Enter_Coordinat  == theWindow)) {
			myPt = theEvent->where;				/* Get the position where the mouse was pressed */
			GlobalToLocal(&myPt);				/* Change from global to local location */

			GetDItem(theWindow, itemHit,&DType,&DItem,&tempRect);/* Get which item was pressed */
			CItem = (ControlHandle)DItem;		/* Change the pointer for getting to the control */

			/* Handle it real time */
			if (itemHit ==Res_Dlg_OK) {			/* Handle the Button being pressed */
				/* Get value entered for X coordinate. */
				GetDItem(WPtr_Enter_Coordinat,Res_Dlg_Edit_Text,&DType,&DItem,&tempRect);/* Get the item handle */
				GetIText(DItem,inputText);
				/* Convert to C string. */
				text_length = inputText[0];
				if (text_length<=254)
					text_length+=1;
				inputText[text_length] = 0;
				fxcenter = atof((char *)&inputText[1]);

				/* Get value entered for Y coordinate. */
				GetDItem(WPtr_Enter_Coordinat,Res_Dlg_Edit_Text2,&DType,&DItem,&tempRect);/* Get the item handle */
				GetIText(DItem,inputText);
				/* Convert to C string. */
				text_length = inputText[0];
				if (text_length<=254)
					text_length+=1;
				inputText[text_length] = 0;
				fycenter = atof((char *)&inputText[1]);

				/* Get value entered for width. */
				GetDItem(WPtr_Enter_Coordinat,Res_Dlg_Edit_Text3,&DType,&DItem,&tempRect);/* Get the item handle */
				GetIText(DItem,inputText);
				/* Convert to C string. */
				text_length = inputText[0];
				if (text_length<=254)
					text_length+=1;
				inputText[text_length] = 0;
				fwidth = atof((char *)&inputText[1]);

				/* Get value entered for number of iterations. */
				GetDItem(WPtr_Enter_Coordinat,Res_Dlg_Edit_Text4,&DType,&DItem,&tempRect);/* Get the item handle */
				GetIText(DItem,inputText);
				/* Convert to C string. */
				text_length = inputText[0];
				if (text_length<=254)
					text_length+=1;
				inputText[text_length] = 0;
				fiters = atoi((char *)&inputText[1]);

				new_coordinates=TRUE;

				/* Close a modeless dialog */
				Add_UserEvent(UserEvent_Close_Window,Res_MD_Enter_Coordinat,0,0,nil);
				/* Open a Window */
				Add_UserEvent(UserEvent_Open_Window,Res_W_ARTAbrot,0,0,nil);
			}

			if (itemHit ==Res_Dlg_Cancel) {		/* Handle the Button being pressed */
				Add_UserEvent(UserEvent_Close_Window,Res_MD_Enter_Coordinat,0,0,nil);/* Open a modeless dialog */
			}
		}
	}

	if (ExitDialog) {							/* Do the close of the dialog */
		Close_Enter_Coordinat(WPtr_Enter_Coordinat);
		WPtr_Enter_Coordinat = nil;				/* Clear it for future checks */
	}
}
