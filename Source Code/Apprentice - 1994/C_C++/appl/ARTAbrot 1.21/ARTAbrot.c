/************************************************************************/
/*																		*/
/*	FILE:		ARTAbrot.c -- Main windows code for ARTA Mandelbrot		*/
/*					program.											*/
/*																		*/
/*	PURPOSE:	Display Mandelbrot graphics.							*/
/*																		*/
/*	AUTHOR:		George T. Warner										*/
/*																		*/
/*	REVISIONS:															*/
/*	08/22/93	First version.											*/
/*																		*/
/************************************************************************/

#include "ComUtil_ARTAbrot.h"	/* Common */

#include "AB_Alert.h"	/* Alert */
#include "ARTAbrot.h"	/* This file */
#include "brotcode.h"

#pragma segment ARTAbrot

static ControlHandle	ScrollHHandle;		/* Scrollbar for horz scrolling */
static ControlHandle	ScrollVHandle;		/* Scrollbar for vert scrolling */
static RGBColor	WindowBackColor;			/* Background window color */
/* Offscreen pixMap stuff. */
GWorldPtr offscreenGWorld = NIL;	/* Off screen graphics world. */
Rect	GWorldBounds;			/* Boundaries of graphics world. */
extern int xpix, ypix;
extern double distance_per_pixel;


/* Routine: Init_ARTAbrot */
/* Purpose: Initialize our window data to not in use yet */

void Init_ARTAbrot()
{
	WPtr_ARTAbrot = nil;					/* Make sure other routines know we are not valid yet */

	ScrollHHandle = nil;					/* Scrollbar is not valid yet */
	ScrollVHandle = nil;					/* Scrollbar is not valid yet */

	WindowBackColor.red  = 0xFFFF;  WindowBackColor.green  = 0xFFFF;  WindowBackColor.blue  = 0xFFFF;/* Set the color */
}

/* ======================================================= */

/* Routine: Close_ARTAbrot */
/* Purpose: Close out the window */

void Close_ARTAbrot(WindowPtr whichWindow)
{
	/* See if we should close this window */
	if ((WPtr_ARTAbrot != NIL) && ((WPtr_ARTAbrot == whichWindow) || (whichWindow == (WindowPtr)-1))) {
		DisposeWindow(WPtr_ARTAbrot);		/* Clear window and controls */
		WPtr_ARTAbrot = nil;				/* Make sure other routines know we are closed */
		DisposeGWorld(offscreenGWorld);
		offscreenGWorld = NIL;
	}
}

/* ======================================================= */

/* Routine: Resized_ARTAbrot */
/* Purpose: We were resized or zoomed, update the scrolling scrollbars */

void Resized_ARTAbrot(WindowPtr whichWindow)
{
WindowPtr	SavePort;						/* Place to save the last port */
Rect	temp2Rect;							/* temp rectangle */
QDErr result;
long row_bytes;	/* Used for PixMap calculations. */
long bytes2erase;
unsigned char *base_addr;

	if (WPtr_ARTAbrot == whichWindow) {		/* Only do if the window is us */
		GetPort(&SavePort);					/* Save the current port */
		SetPort(WPtr_ARTAbrot);				/* Set the port to my window */
	
		temp2Rect = WPtr_ARTAbrot->portRect;	/* Get the window rectangle */
		EraseRect(&temp2Rect);					/* Erase the new window area */
		InvalRect(&temp2Rect);					/* Set to update the new window area */

		/* Dispose old GWorld. */
		DisposeGWorld(offscreenGWorld);
		offscreenGWorld = NIL;
		/* Allocate memory for offscreen bitmap. */
		/* left, top, right, bottom */
		SetRect(&GWorldBounds, 0, 0, (WPtr_ARTAbrot->portRect.right - WPtr_ARTAbrot->portRect.left)-SCROLLBARWIDTH, (WPtr_ARTAbrot->portRect.bottom - WPtr_ARTAbrot->portRect.top)-SCROLLBARWIDTH);
		result = NewGWorld(&offscreenGWorld, 8, &GWorldBounds, NIL, NIL, NIL);
		if (result) {
			DisposeWindow(WPtr_ARTAbrot); 
			WPtr_ARTAbrot = NIL;
			/* Report an error message here. */
		}
		else {
			/* First clear the offscreen GWorld. */
			/* Lock the offscreen GWorld. */
			LockPixels(offscreenGWorld->portPixMap);
			row_bytes = ((**(offscreenGWorld->portPixMap)).rowBytes) & 0x3fff;
			base_addr = (unsigned char *)GetPixBaseAddr(offscreenGWorld->portPixMap);
			bytes2erase = row_bytes * GWorldBounds.bottom;
			do {
				*base_addr++ = 0;
			} while (--bytes2erase);
			UnlockPixels(offscreenGWorld->portPixMap);
			start_brot();					/* Calculate new Mandelbrot. */
		}
	
		SetPort(SavePort);								/* Restore the old port */
	}
}

/* ======================================================= */

/* Routine: Moved_ARTAbrot */
/* Purpose: We were moved, possibly to another screen and screen depth */

void Moved_ARTAbrot(WindowPtr whichWindow)
{
WindowPtr	SavePort;						/* Place to save the last port */
	
	if (WPtr_ARTAbrot == whichWindow) {		/* Only do if the window is us */
		GetPort(&SavePort);					/* Save the current port */
		SetPort(WPtr_ARTAbrot);				/*  Set the port to my window  */
	
		SetPort(SavePort);								/* Restore the old port */
	}
}

/* ======================================================= */

/* Routine: Update_ARTAbrot */
/* Purpose: Update our window */

void Update_ARTAbrot(WindowPtr whichWindow)
{
WindowPtr	SavePort;								/* Place to save the last port */

	/* Handle the update to our window */
	if ((WPtr_ARTAbrot != NIL) && (WPtr_ARTAbrot == whichWindow)) {
		GetPort(&SavePort);					/* Save the current port */
		SetPort(WPtr_ARTAbrot );			/* Set the port to my window */

		DrawControls(WPtr_ARTAbrot);		/* Draw all the controls */
		DrawGrowIcon(WPtr_ARTAbrot );		/* Draw the Grow box */
		DrawControls(WPtr_ARTAbrot);		/* Draw all the controls */
		/* Restore image. */
		if (LockPixels(offscreenGWorld->portPixMap)) {
			CopyBits((BitMap *)*offscreenGWorld->portPixMap, &(WPtr_ARTAbrot->portBits), &(GWorldBounds), &(GWorldBounds), srcCopy, 0);
			UnlockPixels(offscreenGWorld->portPixMap);
		}
		SetPort(SavePort);					/* Restore the old port */
	}
}

/* ======================================================= */

/* Routine: Open_ARTAbrot */
/* Purpose: Open our window */

void Open_ARTAbrot()
{
QDErr result;
long row_bytes;	/* Used for PixMap calculations. */
long bytes2erase;
unsigned char *base_addr;

	if (WPtr_ARTAbrot == NIL) {					/* See if already opened */
		WPtr_ARTAbrot = GetNewCWindow(Res_W_ARTAbrot,nil, (WindowPtr)-1);/* Get the COLOR window from the resource file */
		SetPort(WPtr_ARTAbrot);					/* Prepare to write into our window */

		ShowWindow(WPtr_ARTAbrot);				/* Show the window now */
		/* Allocate memory for offscreen bitmap. */
		/* left, top, right, bottom */
		SetRect(&GWorldBounds, 0, 0, (WPtr_ARTAbrot->portRect.right - WPtr_ARTAbrot->portRect.left)-SCROLLBARWIDTH, (WPtr_ARTAbrot->portRect.bottom - WPtr_ARTAbrot->portRect.top)-SCROLLBARWIDTH);
		result = NewGWorld(&offscreenGWorld, 8, &GWorldBounds, NIL, NIL, NIL);
		if (result) {
			DisposeWindow(WPtr_ARTAbrot); 
			WPtr_ARTAbrot = NIL;
			/* Report an error message here. */
		}
		else {
			/* First clear the offscreen GWorld. */
			/* Lock the offscreen GWorld. */
			LockPixels(offscreenGWorld->portPixMap);
			row_bytes = ((**(offscreenGWorld->portPixMap)).rowBytes) & 0x3fff;
			base_addr = (unsigned char *)GetPixBaseAddr(offscreenGWorld->portPixMap);
			bytes2erase = row_bytes * GWorldBounds.bottom;
			do {
				*base_addr++ = 0;
			} while (--bytes2erase);
			UnlockPixels(offscreenGWorld->portPixMap);

			start_brot();					/* Calculate new Mandelbrot. */
		}
		SelectWindow(WPtr_ARTAbrot);		/* Already open, so show it */
	}
	else {
		SelectWindow(WPtr_ARTAbrot); 		/* Already open, so show it */
		if (new_coordinates)
			start_brot();					/* Calculate new Mandelbrot. */
		else {
			/* Restore image. */
			if (LockPixels(offscreenGWorld->portPixMap)) {
				CopyBits((BitMap *)*offscreenGWorld->portPixMap, &(WPtr_ARTAbrot->portBits), &(GWorldBounds), &(GWorldBounds), srcCopy, 0);
				UnlockPixels(offscreenGWorld->portPixMap);
			}
		}
	}
}

/* ======================================================= */

/* Routine: Activate_ARTAbrot */
/* Purpose: We activated or deactivated */

void Activate_ARTAbrot(WindowPtr whichWindow,Boolean Do_An_Activate)
{
WindowPtr	SavePort;								/* Place to save the last port */

if (WPtr_ARTAbrot == whichWindow)					/*  Only do if the window is us  */
	{
	GetPort(&SavePort);							/* Save the current port */
	SetPort(whichWindow);							/* Set the port to my window */

	DrawGrowIcon(whichWindow);						/* Draw the grow Icon */

	if (Do_An_Activate)							/* Handle the activate */
		{
		}
	else
		{
		if (theInput != nil)						/* See if there is already a TE area */
			{
			TEDeactivate(theInput);				/* Yes, so turn it off */
			}									/* End of IF */
		theInput = nil;							/* Deactivate the TE area */
		}										/* End of IF */

	SetPort(SavePort);								/* Restore the old port */
	}											/* End of IF */
}

/* ======================================================= */

/* Routine: Do_ARTAbrot */
/* Purpose: Handle action to our window, like controls */

void Do_ARTAbrot(EventRecord *myEvent)
{
short	code;							/* Location of event in window or controls */
WindowPtr	whichWindow;				/* Window pointer where event happened */
Point	myPt;							/* Point where event happened */
ControlHandle	theControl;				/* Handle for a control */
Rect		rDrag;
PenState	xpenSave;
float		ratio;						/* Aspect ratio of window. */
Point		newPt;
short		oldBottom, oldRight;

	/* Get where in window and which window */
	code = FindWindow(myEvent->where, &whichWindow);

	if (WPtr_ARTAbrot == whichWindow) {			/* Handle only when the window is valid */
		code = FindWindow(myEvent->where, &whichWindow);/* Get where in window and which window */
	
		if ((myEvent->what == mouseDown) && (WPtr_ARTAbrot == whichWindow)) {
			myPt = myEvent->where;					/* Get mouse position */
			GlobalToLocal(&myPt);					/* Make it relative */

			rDrag.top = myPt.v;
			rDrag.left = myPt.h;
			rDrag.bottom = myPt.v;
			rDrag.right = myPt.h;
			ratio = (float) xpix / (float) ypix;
			
			GetPenState(&xpenSave);
			PenMode(patXor);
			FrameRect(&rDrag);
			while (StillDown()) {
				GetMouse(&newPt);
				if (newPt.h != myPt.h || newPt.v != myPt.v) {
					
					myPt = newPt;
					FrameRect(&rDrag);
					oldBottom = rDrag.bottom;
					oldRight = rDrag.right;
					
					rDrag.bottom = newPt.v;
					if (rDrag.bottom > ypix)	/* Don't let it go off the bottom of the window. */
						rDrag.bottom = ypix;
						
					/* Force the rectangle to have the same aspect ratio as the window. */
					rDrag.right = rDrag.left + ratio * (rDrag.bottom - rDrag.top);
					
					if (rDrag.right > xpix) {	/* Don't let it go off the right of the window. */
						rDrag.bottom = oldBottom;
						rDrag.right = oldRight;
					}
					FrameRect(&rDrag);
				}
			}
			FrameRect(&rDrag);
			SetPenState(&xpenSave);
			
			/* Don't do anything if the rectangle is empty. */
			if (rDrag.top < rDrag.bottom && rDrag.left < rDrag.right) {
				fxcenter += distance_per_pixel * (rDrag.left + rDrag.right - xpix) / 2.;
				fycenter += distance_per_pixel * (rDrag.top + rDrag.bottom - ypix) / 2.;
				fwidth = fwidth * (rDrag.right - rDrag.left);
				fwidth = fwidth / xpix;
				new_coordinates=TRUE;
				/* Open a Window */
				Add_UserEvent(UserEvent_Open_Window,Res_W_ARTAbrot,0,0,nil);
			}
		}
	}

	if ((WPtr_ARTAbrot == whichWindow) && (code == inContent)) {	/* for our window */
		code = FindControl(myPt, whichWindow, &theControl);/* Get type of control */

		if (code != 0)								/* Check type of control */
			code = TrackControl(theControl,myPt, (ProcPtr)-1);/* Track the control */

	}
}

/* ======================================================= */
