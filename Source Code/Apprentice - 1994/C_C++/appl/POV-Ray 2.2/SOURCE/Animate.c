/*==============================================================================
Project:	POV

File:	Animate.c

Description:
Routines to handle the clock animation variable prompting for POV-Ray.
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
	930226	[esp]	Created
	930605	[esp]	Changed initial frame # from zero to one
	930707	[esp]	Checked for frame 1 to 1, avoid div by zero
	930708	[esp]	Changed clock value display from 3 to 5 digits
	930728	[esp]	Added frameValS/frameValE
	930816	[esp]	Added pAnimPtr to SetCurrFrameVal & made it set curr clock val too
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
	931119	[djh]	2.0.1 source conditionally compiles for PPC machines, keyword __powerc
==============================================================================*/

#define ANIMATE_C

/*==== POV-Ray std headers ====*/
#include "POVMac.h"	// setupdefaultbutton


/*==== Standard C headers ====*/
#include <stdlib.h>		// malloc


/*==== Macintosh-specific headers ====*/
// NOTE: _H_MacHeaders_ is defined by Think C if it is using
// precompiled headers.  This is only for compilation speed improvement.
#if !defined(_H_MacHeaders_)
#include <types.h>
#include <memory.h>		// NewPtr
#endif // _H_MacHeaders_

#if !defined (THINK_C)
#include <strings.h>
#endif // THINK_C


/*==== Animate header ====*/
#include "Animate.h"


/*==== defs ====*/

// dialog constants
#define Dlog_ID					156
#define	Ditem_ST_FrameVal0		4
#define	Ditem_ET_FrameValS		5
#define	Ditem_ET_FrameValE		6
#define	Ditem_ET_FrameValN		7
#define	Ditem_ET_Clock0Val		8
#define	Ditem_ST_ClockValS		9
#define	Ditem_ST_ClockValE		10
#define	Ditem_ET_ClockNVal		11
#define	Ditem_MAX				Ditem_ET_ClockNVal


/*==== vars ====*/

static short	gCurrFrameVal;
static float	gCurrentClockVal;


//-----------------------------------------------------------
// Calculate a clock value based on currFrameVal, etc.
static float CalcClockVal(AnimPtr_t pAnimPtr, short currFrameVal)
{
	DBL		tCurrClockVal;

	// THE MOST USEFUL CALCULATION IN THE WORLD! :-)
	// if frames are related to clocks, and frame is this, what's clock?
	if (currFrameVal == 1)
	{
		// If Frame(1), just return clock(0), so we don't divide by zero!
		tCurrClockVal = pAnimPtr->clockVal0;
	}
	else
	{
		// do the real calculation
		tCurrClockVal = pAnimPtr->clockVal0
			+ (	(DBL)currFrameVal        - (DBL)pAnimPtr->frameVal0	)
			* (	(DBL)pAnimPtr->clockValN - pAnimPtr->clockVal0		)
			/ (	(DBL)pAnimPtr->frameValN - (DBL)pAnimPtr->frameVal0	);
	}

	return tCurrClockVal;

} // CalcClockVal


//-----------------------------------------------------------
// Set local current frame value (and calculate current clock value too)
void SetCurrFrameVal(AnimPtr_t pAnimPtr, short newFrameVal)
{
	gCurrFrameVal		= newFrameVal;
	gCurrentClockVal	= CalcClockVal(pAnimPtr, newFrameVal); // calc. curr clock
} // SetCurrFrameVal


//-----------------------------------------------------------
// Return local current frame value
short GetCurrFrameVal(void)
{
	return gCurrFrameVal;
} // GetCurrFrameVal


//-----------------------------------------------------------
// Return local current clock value
float GetCurrClockVal(void)
{
	return gCurrentClockVal;
} // GetCurrClockVal


//-----------------------------------------------------------
// Increment Frame counter if not already finished, and recalculate
// new clock value.  Return true if already at last frame
Boolean IncToNextFrame(AnimPtr_t pAnimPtr)
{
	Boolean	nextFrameOK = false;
	short	frameVal;

	frameVal = GetCurrFrameVal();

	// before last frame?
	if (frameVal < pAnimPtr->frameValE)
	{	// go to next frame
		frameVal++; // go to next
		SetCurrFrameVal(pAnimPtr, frameVal); // save it & calc curr clock val
		nextFrameOK = true;
	}

	return nextFrameOK;
} // IncToNextFrame


//-----------------------------------------------------------
// Put up dialog of options, and return TRUE if user clicks ok, else FALSE if cancel
Boolean GetAnimateOptions(AnimPtr_t pAnimPtr)
{
	short			itemHit, prevItemHit;
	DialogPtr		myDialog;
	short			k, dummy;
	float			clockValS, clockValE;
	AnimRec_t		tAnimRec;
	Rect			displayRect;
	ControlHandle	theDItems[Ditem_MAX+1];
	char			aString[32];

	myDialog = GetNewDialog(Dlog_ID, NULL, (WindowPtr)-1);

	for (k = 1; k<=Ditem_MAX; k++)
		GetDItem(myDialog, k, &dummy, (Handle *) &theDItems[k], &displayRect);

	tAnimRec = *pAnimPtr;	// start off with what they passed us

	// select something..
	SelIText(myDialog, Ditem_ET_FrameValS, 0, -1);

	PositionWindow(myDialog, ewcDoCentering, eSameAsPassedWindow, (WindowPtr)gp2wWindow);

	// "default" the OK button
	SetupDefaultButton(myDialog);

	SetCursor(&qd.arrow);
	ShowWindow(myDialog);

	// prompt until user clicks ok or cancel
	prevItemHit = itemHit = -1;
	do
	{
		// Initial Frame
		sprintf(aString, "%d", tAnimRec.frameVal0); // always starts at frame 1
		c2pstr(aString);
		SetIText((Handle)theDItems[Ditem_ST_FrameVal0], (StringPtr)aString);

		// Start Frame
		sprintf(aString, "%d", tAnimRec.frameValS);
		c2pstr(aString);
		SetIText((Handle) theDItems[Ditem_ET_FrameValS], (StringPtr)aString);

		// End Frame
		sprintf(aString, "%d", tAnimRec.frameValE);
		c2pstr(aString);
		SetIText((Handle) theDItems[Ditem_ET_FrameValE], (StringPtr)aString);

		// Final Frame
		sprintf(aString, "%d", tAnimRec.frameValN);
		c2pstr(aString);
		SetIText((Handle) theDItems[Ditem_ET_FrameValN], (StringPtr)aString);

		// Initial Clock
		sprintf(aString, "%.5f", tAnimRec.clockVal0);
		c2pstr(aString);
		SetIText((Handle) theDItems[Ditem_ET_Clock0Val], (StringPtr)aString);

		// Start Clock
		clockValS = CalcClockVal(&tAnimRec, tAnimRec.frameValS);
		sprintf(aString, "%.5f", clockValS);
		c2pstr(aString);
		SetIText((Handle) theDItems[Ditem_ST_ClockValS], (StringPtr)aString);

		// End Clock
		clockValE = CalcClockVal(&tAnimRec, tAnimRec.frameValE);
		sprintf(aString, "%.5f", clockValE);
		c2pstr(aString);
		SetIText((Handle) theDItems[Ditem_ST_ClockValE], (StringPtr)aString);

		// Final Clock
		sprintf(aString, "%.5f", tAnimRec.clockValN);
		c2pstr(aString);
		SetIText((Handle) theDItems[Ditem_ET_ClockNVal], (StringPtr)aString);

		// don't exit & re-validate until they exit a field
		do {
			prevItemHit = itemHit;
			ModalDialog(NULL, &itemHit);
		} while ((prevItemHit == itemHit) && (itemHit != ok) && (itemHit != cancel));

		if (itemHit != cancel)
		{
			// Initial Frame = 1 (n/a)

			// Final Frame
			GetIText((Handle) theDItems[Ditem_ET_FrameValN], (StringPtr)aString);
			p2cstr((StringPtr)aString);
			if (strlen(aString) == 0)
				tAnimRec.frameValN = tAnimRec.frameVal0;
			else
				tAnimRec.frameValN = atoi(aString);
			if (tAnimRec.frameValN < tAnimRec.frameVal0)
				tAnimRec.frameValN = tAnimRec.frameVal0;

			// Start Frame
			GetIText((Handle) theDItems[Ditem_ET_FrameValS], (StringPtr)aString);
			p2cstr((StringPtr)aString);
			if (strlen(aString) == 0)
				tAnimRec.frameValS = tAnimRec.frameVal0;
			else
				tAnimRec.frameValS = atoi(aString);
			if ((tAnimRec.frameValS < tAnimRec.frameVal0)
			|| (tAnimRec.frameValS > tAnimRec.frameValN))
				tAnimRec.frameValS = tAnimRec.frameVal0;

			// End Frame
			GetIText((Handle) theDItems[Ditem_ET_FrameValE], (StringPtr)aString);
			p2cstr((StringPtr)aString);
			if (strlen(aString) == 0)
				tAnimRec.frameValE = tAnimRec.frameValN;
			else
				tAnimRec.frameValE = atoi(aString);
			if ((tAnimRec.frameValE < tAnimRec.frameVal0)
			|| (tAnimRec.frameValE > tAnimRec.frameValN))
				tAnimRec.frameValE = tAnimRec.frameValN;

			// Do Start & End Frame overlap?
			if (tAnimRec.frameValE < tAnimRec.frameValS)
				tAnimRec.frameValE = tAnimRec.frameValS;
			

			// Initial Clock Val
			GetIText((Handle) theDItems[Ditem_ET_Clock0Val], (StringPtr)aString);
			p2cstr((StringPtr)aString);
			if (strlen(aString) == 0)
				tAnimRec.clockVal0 = 0.0;
			else
				tAnimRec.clockVal0 = atof(aString);

			// Final Clock Val
			GetIText((Handle) theDItems[Ditem_ET_ClockNVal], (StringPtr)aString);
			p2cstr((StringPtr)aString);
			if (strlen(aString) == 0)
				tAnimRec.clockValN = 1.0;
			else
				tAnimRec.clockValN = atof(aString);

			// Final & Initial clock are too close, change Final
			if (fabs(tAnimRec.clockValN-tAnimRec.clockVal0) < 1.0e-10)
			{
				tAnimRec.clockValN = tAnimRec.clockVal0 + 1.0;
				SysBeep(4);
			}
		}
	} while (itemHit != ok && itemHit != cancel);

	if (itemHit == ok)
	{
		// return it
		*pAnimPtr = tAnimRec;
	}

	DisposeDialog(myDialog);

	// return TRUE if user hit OK
	return (itemHit == ok);

} // GetAnimateOptions
