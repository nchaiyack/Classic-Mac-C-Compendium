/*==============================================================================
Project:	POV

File:	Animate.h

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
	930728	[esp]	Added frameValS/frameValE, removed curr fields
	930816	[esp]	Added pAnimPtr to SetCurrFrameVal
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
==============================================================================*/

#if !defined(ANIMATE_H)
#define ANIMATE_H


// Note that the working variables (current frame/clock) are not stored
// in this structure (since they don't need to be saved), and are declared
// as static globals in animate.c instead, with accessor functions.
typedef struct
{
	short		frameVal0;		/* Initial animation frame, always implied one */
	short		frameValS;		/* Current Frame to start session at */
	short		frameValE;		/* Current Frame to end session at */
	short		frameValN;		/* Final animation frame */
	float		clockVal0;		/* initial "clock" value (1st frame) */
	float		clockValN;		/* final "clock" value (last frame) */
} AnimRec_t, *AnimPtr_t, **AnimHdl_t;


// Set local current frame value (and calculate current clock value too)
void SetCurrFrameVal(AnimPtr_t pAnimPtr, short newFrameVal);

// Return local current frame value
short GetCurrFrameVal(void);

// Return local current clock value
float GetCurrClockVal(void);

// Increment Frame counter if not already finished, and recalculate
// new clock value.  Return false if past last frame
Boolean IncToNextFrame(AnimPtr_t pAnimPtr);

/* Returns TRUE if user didn't cancel, and parms would be filled in */
Boolean GetAnimateOptions(AnimPtr_t pAnimPtr);


#endif // ANIMATE_H
