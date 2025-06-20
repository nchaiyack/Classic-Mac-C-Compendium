/*
==============================================================================
Project:	POV-Ray

Version:	2.2

File Name:	ProgressDialog.c

Description:
	General-purpose progress bar handling for dialogs.

	This is the main source file, containing the private definitions and
	code to implement all the needed external and internal support functions.

Related Files:
	ProgressDialog.h	- Header for these routines
------------------------------------------------------------------------------
Author:
	Eduard [esp] Schwan
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
	920820	[esp]	Created
	920920	[esp]	changed highestVal from int to long
	930619	[esp]	added dispose fn
	930903	[esp]	Worked on display bug in showProgress_UProc
	930911	[esp]	Finally fixed the display bug in showProgress_UProc
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
	931119	[djh]	2.0.1 conditionally compiles for PPC machine, keyword __powerc
==============================================================================
*/

/* Macintosh-specific headers */
#include <Types.h>
#include <Dialogs.h>
#include <QuickDraw.h>

#include "POVMac.h"			// RoutineDescriptors & MixedMode.h
#include "ProgressDialog.h"	// our defs


/*  gProgressItemValue will be set to 0 to 100 by CalculateProgressValue() */

static unsigned long	gProgressItemValue;
static short			gProgressItemNum;
static Rect				gProgressItemRect;
static RgnHandle		gProgressItemRgn = NULL;


// ==============================================
// Set up a user item proc for drawing progress bar
pascal void showProgress_UProc(DialogPtr theDialog, short theItem)
{
#pragma unused (theItem)
	PenState	SavePen;
	short		progressPos;
	Rect		dispRect,
				outerRect,
				progressRect;

	// remember original penstate
	SetPort(theDialog);
	GetPenState(&SavePen);

	// find progress bar rectangle
	dispRect = gProgressItemRect;

	// outer frame
	outerRect = dispRect;
	InsetRect(&outerRect, 1, 1);
	PenSize(1, 1);
	ForeColor(blackColor);
	FrameRect(&outerRect);

	// set up progress rect
	progressRect = dispRect;
	InsetRect(&progressRect, 3, 3);

	// calculate inner bar progress position
	if (gProgressItemValue > 100)
		gProgressItemValue = 100;
	progressPos = ((unsigned long)(progressRect.right - progressRect.left) * gProgressItemValue) / 100L;

	// draw inner bar (left filled side)
	if (progressPos > 0)
	{
		progressRect.right = dispRect.left + progressPos;
		ForeColor(blackColor);
		PaintRect(&progressRect);

		// draw inner bar (right open side)
		if (progressPos < 100)
		{
			progressRect.left = dispRect.left + progressPos;
			ForeColor(whiteColor);
			PaintRect(&progressRect);
		}
	}

	// restore state
	SetPenState(&SavePen);

} // showProgress_UProc


// ==============================================
// Sets dialog item's display proc to draw progress bar
static void SetupProgressItem(DialogPtr theDialog, short theItemNum)
{
    short	itemtype;
    Handle	tempHandle;

	gProgressItemValue = 0;

	// Set up User item to display a progress bar
	GetDItem(theDialog, theItemNum, &itemtype, &tempHandle, &gProgressItemRect);
#if defined(__powerc)
	SetDItem(theDialog, theItemNum, itemtype, (Handle)&gShowProgressRD, &gProgressItemRect);
#else
	SetDItem(theDialog, theItemNum, itemtype, (Handle)&showProgress_UProc, &gProgressItemRect);
#endif

	// remember.. for later updates
	gProgressItemRgn = NewRgn();
	RectRgn(gProgressItemRgn, &gProgressItemRect);
	gProgressItemNum = theItemNum;
} // SetupProgressItem


// ==============================================
// Loads Dialog resource and sets up progress bar user item proc
DialogPtr GetNewProgressDialog(short theDialogID, short theProgressItemNum)
{
	DialogPtr	theDialog = NULL;

	theDialog = GetNewDialog(theDialogID, NULL, (WindowPtr)-1);

	if (theDialog)
		SetupProgressItem(theDialog, theProgressItemNum);

	return (theDialog);

} // GetNewProgressDialog


// ==============================================
// Calculates current value for progress bar
static void CalculateProgressValue(long lowestVal, long highestVal, long currentVal)
{
	// Clip against upper & lower bounds
	if (currentVal < lowestVal)
		currentVal = lowestVal;
	else
		if (currentVal > highestVal)
			currentVal = highestVal;

	// Now calculate current value
	gProgressItemValue = (currentVal-lowestVal) * 100L / (highestVal-lowestVal);

} // CalculateProgressValue


// ==============================================
// Recalculate progress bar and redisplay dialog
void updateProgressDialog(DialogPtr pDialogPtr, long lowestVal, long highestVal, long currentVal)
{
	CalculateProgressValue(lowestVal, highestVal, currentVal);
	SetPort(pDialogPtr);
	InvalRect(&gProgressItemRect);

	BeginUpdate(pDialogPtr);
	UpdateDialog(pDialogPtr, gProgressItemRgn);
	EndUpdate(pDialogPtr);

} // updateProgressDialog


// ==============================================
// dispose the progress bar dialog
void disposeProgressDialog(DialogPtr pDialogPtr)
{
	if (pDialogPtr)
	{
		DisposeDialog(pDialogPtr);
		pDialogPtr = NULL;
	}

	if (gProgressItemRgn)
	{
		DisposeRgn(gProgressItemRgn);
		gProgressItemRgn = NULL;
	}
} // disposeProgressDialog
