/*
==============================================================================
Project:	POV-Ray

Version:	2.2

File Name:	ScreenUtils.c

Description:
	General-purpose screen/device routines.

	This is the main source file, containing the private definitions and
	code to implement all the needed external and internal support functions.

Related Files:
	ScreenUtils.h	- Header for these routines
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
	920826	[esp]	Created
	920903	[esp]	Major cleanup to PositionWindow
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
	931006	[esp]	Fixed ForceRectOnScreen to not let window dangle off lower left
==============================================================================
*/

/* Macintosh-specific headers */
#include <Types.h>
#include <QuickDraw.h>
#include <script.h>			// GetMBarHeight() old?
#include <menus.h>			// GetMBarHeight()
#include <Windows.h>		// MoveWindow()

#include "ScreenUtils.h"	// our defs

// ==============================================
void GetGlobalWindowRect(WindowPtr theWindow, Rect *theRect)
{
	Rect	windRect;

	windRect = theWindow->portRect;

	// yah, but where is it really?
	SetPort(theWindow);
	LocalToGlobal((Point*)&windRect.top);
	LocalToGlobal((Point*)&windRect.bottom);

	*theRect = windRect;
} // GetGlobalWindowRect


// ==============================================
void ForceRectOnScreen(Rect *aGlobalRect)
{
	long		area;
	long		foundArea;
	GDHandle	aDevice;
	GDHandle	foundDevice;
	Rect		aDeviceRect;
	Rect		foundDeviceRect;
	Rect		intersectRect;

	for (aDevice = GetDeviceList(); aDevice != NULL; aDevice = GetNextDevice(aDevice))
	{
		/* If we found an active screen device that intersects some of the rect.. */
		aDeviceRect = (**aDevice).gdRect;

		/* See if its the main screen.  If so, take menu bar into account! */
		if ( (**aDevice).gdFlags & (1<<mainScreen) )
		{
			aDeviceRect.top += GetMBarHeight();
			/* also initialize the found device to main device */
			/* in case none found, we default to main device */
			foundDevice = aDevice;
			foundDeviceRect = aDeviceRect;
			foundArea = 0;
		}

		/* if it is an active screen... */
		if (TestDeviceAttribute(aDevice, screenDevice)
			&& TestDeviceAttribute(aDevice, screenActive)
			&& SectRect(aGlobalRect, &aDeviceRect, &intersectRect))
		{
			/* find out how much it intersects */
			area = ((long)intersectRect.right  - (long)intersectRect.left) *
				   ((long)intersectRect.bottom - (long)intersectRect.top);
			/* if intersects more than the last device of intersection, use this instead */
			/* this therefore finds the screen of greatest intersection. */
			if (area > foundArea)
			{
				foundDevice = aDevice;
				foundDeviceRect = aDeviceRect;
				foundArea = area;
			}
		}
	}

	/* OK, we have now travelled through all the devices, and 'foundDevice' */
	/* will have the best device to use, or the main device if there were	*/
	/* no intersecting screens at all.. Make sure it fits on the screen		*/

	/* If there is not enough area showing, then move the window to better	*/
	/* position on 'foundDevice' (1600 is width<40> X height<40> pixels).	*/
	/* Also do this if the top of the window (minus 18 for the title bar)	*/
	/* extends above the top of the window (user can't get ahold of it.)	*/
	if ( (foundArea < 1600) || ((aGlobalRect->top-18) < foundDeviceRect.top) )
	{
		/* offset its upper left corner a little from top of screen */
		aDeviceRect = foundDeviceRect;
		OffsetRect(&aDeviceRect, 4, 18+4);

		/* figure out what its lower left corner would be */
		aDeviceRect.right = aDeviceRect.left + (aGlobalRect->right - aGlobalRect->left);
		aDeviceRect.bottom = aDeviceRect.top + (aGlobalRect->bottom - aGlobalRect->top);

		/* Return this rect as the one to use */
		*aGlobalRect = aDeviceRect;
	}

	/* If rect bottom/right sides are still off the screen, inset them to fit */
	if	(aGlobalRect->right > foundDeviceRect.right)
		aGlobalRect->right = foundDeviceRect.right-4;
	if	(aGlobalRect->bottom > foundDeviceRect.bottom)
		aGlobalRect->bottom = foundDeviceRect.bottom-4;

} // ForceRectOnScreen


// ==============================================
// Note that this routine and ForceRectOnScreen() should be sharing code.. later.
GDHandle GetClosestGDevice(const Rect * sourceRectp)
{
	GDHandle	theGDevice;

	// find max device this rect intersects
	theGDevice = GetMaxDevice(sourceRectp);
	if (theGDevice == NULL)
	{	// didn't work for some reason, get the regular screen
		theGDevice = GetMainDevice();
	}

	return theGDevice;

} // GetClosestGDevice


// ==============================================
void GetMaxGrowRect(WindowPtr wind2Grow, Rect * growRectp)
{
	Rect 		rw, rs;
	GDHandle	theGDevice;

	// find true/global window pos.
	GetGlobalWindowRect(wind2Grow, &rw);
	// make a teeny rect at the mouse pos, so we can
	// find the GDevice at this point
	rs = rw;
	rs.left = rs.right-1;
	rs.top = rs.bottom-1;
	theGDevice = GetClosestGDevice(&rs);
	// now find the rect bounds of the screen that the grow box is on
	rs = (**theGDevice).gdRect;
	// set max size allowed (upper corner of window to bottom corner of screen)
	// this allows the window to grow just to the bottom corner of the screen
	rw.right	= rs.right - rw.left - 4;
	rw.bottom	= rs.bottom - rw.top - 4;
	// set minimum size allowed
	rw.left		= 40;
	rw.top		= 40;
	// let caller know what we finally decided on
	*growRectp	= rw;
} // GetMaxGrowRect


// ==============================================
void CenterWindInRect(WindowPtr	wind2Center, Rect outerRect)
{
	short	x, y;
	Rect	windRect;

	// figure center of outer rect
	x = (outerRect.left + outerRect.right)  >> 1;
	y = (outerRect.top  + outerRect.bottom) >> 1;

	// figure out window's rect
	GetGlobalWindowRect(wind2Center, &windRect);

	// now center window around x,y (offset by half window size)
	x = x - ((windRect.right - windRect.left) >> 1);
	y = y - ((windRect.bottom - windRect.top) >> 1);
	MoveWindow(wind2Center, x, y, true);

} // CenterWindInRect


// ==============================================
void PositionWindow(WindowPtr wind2Position, WindCentering_t doCentering, WindPositioning_t whereToShow, WindowPtr passedWindow)
{
	Point		upperCorner;
	Rect		mainRect;
	Rect		windRect;
	Rect		deviceRect;
	Rect		maxDragBounds;
	GDHandle	theGDevice;

	if (wind2Position == NULL)
		return;

	// Set up bounds for all devices
	SetRect(&maxDragBounds, -32000, -32000, 32000, 32000);

	// Find main device bounds
	theGDevice = GetMainDevice();
	mainRect = (**theGDevice).gdRect;

	// find the device rectangle to show window on
	switch (whereToShow)
	{
		case eSameAsPassedWindow:
			if (passedWindow)
			{
				// where's the passed window
				GetGlobalWindowRect(passedWindow, &windRect);
				theGDevice = GetClosestGDevice(&windRect);
				deviceRect = (**theGDevice).gdRect;
				break;
			}
			// otherwise fall through to main device

		case eMainDevice:
			deviceRect = mainRect;
			break;

		case eDeepestDevice:
			theGDevice = GetMaxDevice(&maxDragBounds);
			deviceRect = (**theGDevice).gdRect;
			break;
	} // switch

	// See if its the main screen.  If so, take menu bar into account!
	if ( (**theGDevice).gdFlags & (1<<mainScreen) )
		deviceRect.top += GetMBarHeight();

	// if centering, center on screen using that rect
	// otherwise put in relatively same spot on screen
	if (doCentering == ewcDoCentering)
		CenterWindInRect(wind2Position, deviceRect);
	else
	{
		// where's the window, relative to main screen
		GetGlobalWindowRect(wind2Position, &windRect);
		// find relative spot on new device
		upperCorner.h = windRect.left;
		upperCorner.v = windRect.top;
		// now relative to new screen
		upperCorner.h = upperCorner.h + deviceRect.left;
		upperCorner.v = upperCorner.v + deviceRect.top;
		// move it there
		MoveWindow(wind2Position, upperCorner.h, upperCorner.v, true);
	}
} // PositionWindow
