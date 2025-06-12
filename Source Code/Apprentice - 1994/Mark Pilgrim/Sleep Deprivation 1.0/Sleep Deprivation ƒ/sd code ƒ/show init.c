/**********************************************************************\

File:		show init.c

Purpose:	This module handles actually putting the startup icon
			on the screen at INIT loading time.
			
Note:		This file was not written by the authors of Sleep Deprivation
			and is not subject to the licensing terms of the GNU General
			Public License.


Sleep Deprivation -- graphic effects on sleep
Copyright (C) 1993 Mark Pilgrim & Dave Blumenthal

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "globals.h"
#include "show init.h"

/* this file was received from Chris Tate - fixer@faxcsl.dcrt.nih.gov */
/* it is modified - some typedefs have been moved to header files */

/*
	ShowIconFamily.c
	
	ShowINIT compatible routine that shows 'ICN#' and 'iclx' flavor icons.
	For use by all INITs in System 7 and beyond.
	
	by Patrick C. Beard.
	
	Instructions for use:
	
		% Create a family of icons with ResEdit 2.1 or later.  This will include
		 'ICN#', 'icl4', & 'icl8' icons.
		% Use SetUpA4 to set up Think C globals.
		% Call ShowIconFamily() with the resource id of the family that you used.
	
	Enhancements:
		
		% Uses 'iclx' & 'ICN#' icons from the Finder's "icon family" in System 7.
		% Generates a position for icons that is guaranteed to be on screen, while
		  remaining compatible with previous releases of ShowInit.
	
	This code is completely public domain.  Let's hope this becomes a new standard.
	
	This code is derived from the original ShowInit by Paul Mercer, Darin Adler,
	Paul Snively, and Steve Capps.
	
	Special thanks to Ben Haller & Rob Vaterlaus for valuable suggestions and help.
 */

#include <OSUtils.h>
#include <QuickDraw.h>
/*#include <Color.h>*/

#ifndef nil
#define nil ((void*)0)
#endif

#ifndef topLeft
#define topLeft(r) ((Point*)&r)[0])
#endif

#ifndef botRight
#define botRight(r) ((Point*)&r)[1])
#endif

static short theDepth;				/* the depth the monitor is in. */
static CGrafPtr port;

static void DrawBWIcon(short iconId);
static void DrawColorIcon(short iconId);

/* this is where it all happens. */

void ShowIconFamily(short iconId)
{
	long oldA5;
	QDGlobals qd;				/* our QD globals. */
	SysEnvRec environment;		/* machine configuration. */
	CGrafPort gp;				/* our grafport. */
	
	/* get a value for A5, a structure that mirrors qd globals. */
	oldA5 = SetA5((long)&qd.end);
	InitGraf(&qd.thePort);
	
	/* find out what kind of machine this is. */
	SysEnvirons(curSysEnvVers, &environment);
	if (environment.hasColorQD) {
		theDepth = (**(**GetMainDevice()).gdPMap).pixelSize;
		if (theDepth < 4)
			theDepth = 1;
	} else {
		theDepth = 1;
	}

	/* see what type of port to open. */
	if (theDepth >= 4) {
		OpenCPort(&gp);
	} else {
		OpenPort((GrafPtr)&gp);
	}
	port = &gp;
	
	if (theDepth == 1)
		DrawBWIcon(iconId);
	else
		DrawColorIcon(iconId);
	
	SetA5(oldA5);
}

/*
	ShowInit's information is nestled at the tail end of CurApName.
	It consists of a short which encodes the next horizontal offset,
	and another short which is that value checksummed with the function below.
 */

#define CurApName_LM	0x910
#define ShowINITTable ((short*)(CurApName_LM + 32 - 4))
#define CheckSumConst 0x1021		/* magic value to check-sum with. */

#define InitialXPosition 8			/* initial horizontal offset. */
#define YOffset			40			/* constant from bottom to place the icon. */
#define XOffset			40			/* amount to change it by. */

/* CheckSum() computes the magic value to determine if ShowInit's have run already. */

static short CheckSum(register short x)
{
	asm {
		rol.w	#1, x
	}
	return (x ^ CheckSumConst);
}

/*
	GetIconRect() generates an appropriate rectangle to display the
	next INIT's icon in.  It is also responsible for updating the horizontal
	position in low memory.  This is a departure from the original ShowInit code,
	which updates low memory AFTER displaying the icon.  This code won't generate
	an icon position until it is certain that the icon can be loaded, so the
	same behaviour occurs.
	
	This routine also generates a rectangle which is guaranteed to be onscreen.  It
	does this by taking the horizontal offset modulo the screen width to generate
	the horizontal position of the icon, and the offset divided by the screen
	width to generate the proper row.
 */

void GetIconRect(register Rect* iconRect)
{
	register short screenWidth = port->portRect.right - port->portRect.left;
	/* if we are the first INIT to run we need to initialize the horizontal value. */
	if (CheckSum(ShowINITTable[0]) != ShowINITTable[1])
		ShowINITTable[0] = InitialXPosition;
	
	/* compute top left of icon's rect. */
	iconRect->left = (ShowINITTable[0] % screenWidth);
	iconRect->top = port->portRect.bottom - YOffset * (1 + (ShowINITTable[0] / screenWidth));
	iconRect->right = iconRect->left + 32;
	iconRect->bottom = iconRect->top + 32;
	
	/* advance the position for the next icon. */
	ShowINITTable[0] += XOffset;
	
	/* recompute the checksum. */
	ShowINITTable[1] = CheckSum(ShowINITTable[0]);
}

/* DrawBWIcon() draws the 'ICN#' member of the icon family. */

void DrawBWIcon(short iconId)
{
	Handle icon;
	Rect iconRect;
	BitMap source, destination;
	
	icon = Get1Resource('ICN#', iconId);
	if (!icon)
		return;
	HLock(icon);
	
	GetIconRect(&iconRect);

	/* prepare the source and destination bitmaps. */
	source.baseAddr = *icon + 128;					/* mask address. */
	source.rowBytes = 4;
	SetRect(&source.bounds, 0, 0, 32, 32);
	destination = ((GrafPtr)port)->portBits;
	
	/* transfer the mask. */
	CopyBits(&source, &destination, &source.bounds, &iconRect, srcBic, nil);
	
	/* and the icon. */
	source.baseAddr = *icon;	
	CopyBits(&source, &destination, &source.bounds, &iconRect, srcOr, nil);
	
	ReleaseResource(icon);
}

/*
	ChooseIcon() chooses the optimal icon for the current screen depth.
	
	Priorities for choosing icons:
		1. match the bit depth to the icon.
		2. use alternate bit depth version if available.
		3. draw the black & white version.
 */
	
Handle ChooseIcon(short iconId, short* suggestedDepth)
{
	short depth = *suggestedDepth;
	Handle icon = nil;

	if (depth == 4) {
		icon = Get1Resource('icl4', iconId);
		if (!icon) {
			/* try alternate depth. */
			icon = Get1Resource('icl8', iconId);
			if (icon)
				depth = 8;
		}
	} else {
		depth = 8;
		icon = Get1Resource('icl8', iconId);
		if (!icon) {
			/* try alternate depth. */
			icon = Get1Resource('icl4', iconId);
			if (icon)
				depth = 4;
		}
	}
	
	*suggestedDepth = depth;
	return icon;
}

/* DrawColorIcon() draws the appropriate icon for the current screen depth. */

void DrawColorIcon(short iconId)
{
	short depthToUse;
	Handle mask, icon;
	CTabHandle clut;
	PixMapHandle source;
	BitMap maskBits;
	long rowBytes;
	Rect iconRect, bounds;
	
	/* by default we will be using the actual depth of the screen. */
	depthToUse = theDepth;
	icon = ChooseIcon(iconId, &depthToUse);
	
	/* if no color icon available, draw the black & white icon. */
	if (!icon) {
		DrawBWIcon(iconId);
		return;
	}
	HLock(icon);
	
	/* get the black & white icon to get the mask drawn. */
	mask = Get1Resource('ICN#', iconId);
	if (!mask)
		return;
	HLock(mask);

	/* get the correct color lookup table. */
	clut = GetCTable(depthToUse);
	if (!clut)
		return;
	
	/* create a pixmap to stick the icon bits into for screen blitting. */
	source = NewPixMap();
	if (!source) {
		DisposCTable(clut);
		return;
	}
	
	/* set up the source pixmap with the appropriate bounds, depth, and clut. */
	bounds.top = bounds.left = 0;
	bounds.bottom = bounds.right = 32;
	rowBytes = (((depthToUse * 32) + 15) / 16) * 2;
	(**source).baseAddr = *icon;
	(**source).rowBytes = ((short)rowBytes) | 0x8000;
	(**source).bounds = bounds;
	(**source).pixelType = 0;			/* chunky model. */
	(**source).pixelSize = depthToUse;
	(**source).cmpCount = 1;			/* if in 32 bit mode this will be 3, so must change. */
	(**source).cmpSize = depthToUse;	/* only chunky images used. */
	DisposCTable((**source).pmTable);	/* dispose of default, uninitialized table. */
	(**source).pmTable = clut;

	/* get position to draw icon in. */
	GetIconRect(&iconRect);

	/* prepare the mask bitmap. */
	maskBits.baseAddr = *mask + 128;					/* mask address. */
	maskBits.rowBytes = 4;
	maskBits.bounds = bounds;

	/* punch out the mask. */	
	CopyBits(&maskBits, &port->portPixMap, &bounds, &iconRect, srcBic, nil);
	
	/* draw the actual color icon. */
	HLock((Handle)source);
	CopyBits(*source, &port->portPixMap, &bounds, &iconRect, srcOr, nil);
	
	/* release everything we've allocated. */
	(**source).baseAddr = nil;
	DisposPixMap(source);
	
	/* release the icon and mask. */
	ReleaseResource(icon);
	ReleaseResource(mask);
}