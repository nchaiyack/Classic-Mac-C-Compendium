/*  AB_Alert										 Handle this alert */

/* File name:  AB_Alert.c  */
/* Function:  Handle this alert. */
/* This is a NOTE alert, it is used to inform the user of some general information. */
/* This alert is not used if there is a possibility of losing any data. */
/* This alert is called when:   */
/*     */
/* The choices in this alert allow for:   */
/*    
History: 8/18/93 Original by George Warner
   */
#include <StdIO.h>
#include <String.h>
#include <Quickdraw.h>
#include "ComUtil_ARTAbrot.h"	/* Common */
#include "AB_Alert.h"	/* This file */


#pragma segment AB_Alert

/********************************************************************
*																	*
*	Function :	check_stop											*
*																	*
*	Purpose: 	Function to see if user typed Command-period.		*
*																	*
*	Returns: 	None.												*
*																	*
*********************************************************************/
int check_stop(void)
{
EventRecord  myEvent;
char inchar;

	if (GetNextEvent(keyDownMask, &myEvent)) {
		inchar = (char)(myEvent.message & charCodeMask);
		if (((myEvent.modifiers & cmdKey) != 0) && (inchar == '.'))
            return(true);
		else return(false);
	}
	else return(false);
}

/* ======================================================= */
void I_A_Alert()
{


}

/* ======================================================= */
void AB_Alert(char *err_description)
{
#define MAX_STRING 256
char	tmp_string[MAX_STRING];
GrafPtr	oldPort;
WindowPtr        errorWindow;
Rect             errorWBounds = { 40, 10, 140, 210 }; /* t,l,b,r */
Rect	buttonRect;
EventRecord  myEvent;
int		hcenter, swidth;
int rectwidth, rectheight;
ControlHandle	Ctrl_OK;
Point myPt;
int GoodClick;

	/* Set the font. */
	TextSize(12);
	TextFont(systemFont);
	
	/* Get length of string. */
	strncpy(tmp_string+1, err_description, MAX_STRING-1);
	/* Make sure string ends in a NUL. */
	tmp_string[MAX_STRING-1] = 0;
	tmp_string[0] = strlen(err_description);
	swidth = StringWidth((ConstStr255Param)tmp_string);
	
	/* Get window width and make sure it is enough. */
	rectheight = errorWBounds.bottom - errorWBounds.top;
	rectwidth = errorWBounds.right - errorWBounds.left;
	if (rectwidth < swidth) {
		rectwidth = swidth + 20;
		errorWBounds.right = errorWBounds.left + rectwidth;
	}
	
    GetPort(&oldPort); /* Get the current grafPort. */
    errorWindow = NewWindow(0L, &errorWBounds, (ConstStr255Param)"\pError", true, noGrowDocProc, (WindowPtr)(-1L), true, 0L);
    SetPort(errorWindow);
    
//    PenPat((ConstPatternParam)qd.black);
//    BackPat((ConstPatternParam)qd.white);
    
	hcenter = rectwidth/2;

	MoveTo(hcenter - (swidth/2), 20);
	DrawString((ConstStr255Param)tmp_string);

	buttonRect.right = rectwidth - 15;
	buttonRect.bottom = rectheight - 15;
	buttonRect.top = buttonRect.bottom - 22;
	buttonRect.left = buttonRect.right - 64;
	
	Ctrl_OK = NewControl(errorWindow, &buttonRect, (ConstStr255Param)"\pOK", true, 1, 0, 0, 0, 1);
	PenSize(3,3);
	InsetRect(&buttonRect, -4, -4);   	/* Draw outside the button by 1 pixel */
	FrameRoundRect(&buttonRect, 16, 16); /* Draw the outline */

	GoodClick = false;
	/* Wait for a mouse click in the window. */
	
	/* change cursor back to arrow */
	InitCursor();
	while (!GoodClick) {
		/* Wait for a mouse click. */
		while (!(GetNextEvent(mDownMask, &myEvent)));
		myPt = myEvent.where;     							/* Get mouse position */
		GlobalToLocal(&myPt);								/* Make it relative */
		if (PtInRect(myPt,&buttonRect)) {
			GoodClick = true;
			HiliteControl(Ctrl_OK, 10);
		}
		else
			SysBeep(1);
	}
	DisposeWindow(errorWindow);
	SetPort(oldPort);    			/* Restore old window. */
}

/* ======================================================= */
