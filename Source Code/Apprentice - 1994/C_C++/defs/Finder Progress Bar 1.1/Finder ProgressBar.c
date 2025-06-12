/*****************************************************************************************************
*                                                                                                    *
* Finder ProgressBar.c - Copyright 1993 - 1994 Chris Larson, All rights reserved                     *
*                                              (cklarson@engr.ucdavis.edu)                           *
*                                                                                                    *
* Source file of a CDEF which mimics the progress bar used in the Finder. This source file and its   *
* compiled derivatives may be freely used within any freeware/shareware/postcardware/beerware/É as   *
* long as you mention my name in your credits. Neither this source nor its compiled derivatives are  *
* in the public domain and may not be use in any form in public domain software. Neither this source *
* nor its compiled derivatives may be used in any form in a commercial product without the expressed,*
* written consent of the author (me).                                                                *
*                                                                                                    *
* Version 1.1 -- April 28, 1994.                                                                     *
*                                                                                                    *
*****************************************************************************************************/

#include "Finder ProgressBar.h"

//----------------------------------------------------------------------------------------------------
//
// main -- Handle everything but draw messages.
//
//----------------------------------------------------------------------------------------------------

pascal long main (short varCode, ControlHandle theControlHandle, short message, long param)
{
	SignedByte	controlRecState;	// Holds the state of the control recordÕs handle.
	long		returnValue;		// Holds the value returned by the CDEF.
	ControlPtr	theControl;			// Holds a pointer to the control record.
	
	// ----------
	// Since I use some static local variables I need to set up A4 so they can be accessed
	// properly.
	// ----------
	
	MySetUpA4();
	
	// ----------
	// Lock down the control record.
	// ----------
	
	controlRecState = HGetState((Handle)theControlHandle);
	HLock((Handle)theControlHandle);
	
	// ----------
	// Initialize the return value and the control pointer (since the record is locked it is safe
	// to use a pointer). The return value is initialized to 0 since most CDEF messages want 0
	// returned.
	// ----------
	
	returnValue = 0;
	theControl = *theControlHandle;
	
	switch (message)
		{
		
		// ----------
		// Ignore draw messages if the control is not visible.
		// ----------
		
		case drawCntl:
			if (theControl->contrlVis)
				DrawProgressBar(theControl);
			break;
		
		// ----------
		// Test the control: return 1 if the given point is within the control's rectangle
		// and the control is not dimmed, 0 otherwise (the whole bar is part #1).
		// ----------
		
		case testCntl:
			if (theControl->contrlHilite != 0xFF)
				returnValue = PtInRect(*(Point*)(&param),&(theControl->contrlRect));
			break;
		
		// ----------
		// For the 24-bit mode region calculation message, the high bit of the given region handle
		// must be cleared. Note that I donÕt test this bit to determine which region to calculate.
		// Since this control has no indicator (part with a part code >= 129) I will never receive
		// a message asking for indicator region calculation.
		// ----------
		
		case calcCRgns:
			param &= 0x7FFFFFFF;
		
		// ----------
		// Calculate the controlÕs region.
		// ----------
		
		case calcCntlRgn:
			RectRgn((RgnHandle)param,&(theControl->contrlRect));
			break;
		
		// ----------
		// All other messages do nothing and return 0.
		// ----------
		
		default:
			break;
		}
	
	// ----------
	// Restore the state of the control recordÕs handle.
	// ----------
	
	HSetState ((Handle)theControlHandle, controlRecState);
	
	// ----------
	// Restore A4
	// ----------
	
	MyRestoreA4();
	
	// ----------
	// WeÕre outa here.
	// ----------
	
	return (returnValue);
}

//----------------------------------------------------------------------------------------------------
//
// DrawProgressBar -- Draw the control according to the values within the control record.
//
//----------------------------------------------------------------------------------------------------

void DrawProgressBar (ControlPtr theControl)
{
	PenState		oldPen;
	RGBColor		oldFore, oldBack;
	static RGBColor	blackColor = cBlack, whiteColor = cWhite, blueColor = cBlue, grayColor = cGray;
	RgnHandle		oldClip,currentClip;
	Rect			theBox = theControl->contrlRect;
	GrafPtr			savePort, controlPort = theControl->contrlOwner;
		
	// ----------
	// Set the port to the controlÕs port. Very probably not needed (since AppleÕs CDEFs do not
	// do this) but just to be extra safeÉ
	// ----------
	
	GetPort (&savePort);
	SetPort (controlPort);
	
	// ----------
	// Save the current portÕs clip region and set it to the intersection of the clip region with
	// the controlÕs rectangle.
	// ----------
	
	oldClip = NewRgn();
	GetClip(oldClip);
	
	ClipRect(&theBox);
	currentClip = theControl->contrlOwner->clipRgn;
	SectRgn(currentClip,oldClip,currentClip);
	
	// ----------
	// Save and reset the pen state
	// ----------
	
	GetPenState(&oldPen);
	PenNormal();
	
	// ----------
	// If we have color QD, save the foreground and background colors and set the foreground color
	// to black, background color to white.
	// ----------
	
	if ( (ROM85 & kNoColorQD) == 0 )
		{
		GetForeColor(&oldFore);
		GetBackColor(&oldBack);
		
		RGBForeColor(&blackColor);
		RGBBackColor(&whiteColor);
		}
	
	// ----------
	// Draw the progress bar frame and set up the rectangle for drawing the bar.
	// ----------
	
	FrameRect(&theBox);
	InsetRect(&theBox,1,1);
	
	// ----------
	// Calculate the location of the right end of the progress bar. (See header file.)
	// ----------
	
	theBox.right = CalculateBarBoundry(theBox.left,theBox.right,theControl);
				
	// ----------
	// If we have color, set the foreground color to gray, leave the background white so that
	// the gray will map to black on a 1-bit monitor. Note that for non-color QD machines, we want
	// the bar to be black. Since the PenNormal() call set the pen pattern to black, we need do
	// nothing for this case. Then draw the bar.
	// ----------
	
	if ( (ROM85 & kNoColorQD) == 0 )
		RGBForeColor(&grayColor);

	PaintRect(&theBox);
	
	 // ----------
	 // Set up the rectangle to describe the space from the end of the bar to the edge of the
	 // inset rectangle.
	 // ----------
	 
	theBox.left = theBox.right;
	theBox.right = (theControl->contrlRect.right) - 1;

	// ----------
	// If we have color, set the foreground to blue and the background to black (so that the blue
	// will map to white on a 1-bit monitor). If no color, we want to erase the empty space, so
	// set the pen to erase. Then paint the rectangle.
	// ----------

	if ( (ROM85 & kNoColorQD) == 0 )
		{
		RGBForeColor(&blueColor);
		RGBBackColor(&blackColor);
		}
	else
		PenMode(patBic);
	
	PaintRect(&theBox);
	
	// ----------
	// Restore all the stuff we munged: colors (if needed), pen state, clip region, and current port.
	// ----------
	
	if ( (ROM85 & kNoColorQD) == 0 )
		{
		RGBForeColor(&oldFore);
		RGBBackColor(&oldBack);
		}
	
	SetPenState(&oldPen);
	
	SetClip(oldClip);
	DisposeRgn(oldClip);
	
	SetPort(savePort);
}