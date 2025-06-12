/**********************************************************************\

File:		cdev globals.h

Purpose:	This is the header file for all the structs and enums in
			the cdev.


Go Sit In The Corner -=- not you, just the cursor
Copyright ©1994, Mark Pilgrim

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

#pragma once

struct QDGlobals
{
	char privates[76];
	long randSeed;
	BitMap screenBits;
	Cursor arrow;
	Pattern dkGray;
	Pattern ltGray;
	Pattern gray;
	Pattern black;
	Pattern white;
	GrafPtr thePort;
	long	end;
};

typedef struct QDGlobals QDGlobals;

typedef struct PrefStruct
{
	unsigned long	numTicks;				/* number of ticks 'til cornering */
	unsigned char	whichCorner;			/* 0=TL, 1=TR, 2=BR, 3=BL */
	unsigned char	always;					/* always go to corner? */
	unsigned char	showIcon;				/* Show icon on startup? */
	unsigned char	isOn;					/* Is on? */
	unsigned long	ourCodePtr;				/* pointer to VBL code in memory */
	unsigned long	ourVBLPtr;				/* pointer to VBL structure in memory */
} PrefStruct, *PrefPtr, **PrefHandle;

enum
{
	kButtonOn=1,
	kTitleOn,
	kButtonOff,
	kTitleOff,
	kButtonShowIcon,
	kTitleShowIcon,
	kButtonFirstTime=11,
	kButtonLastTime=15,
	kTitleFirstTime,
	kTitleLastTime=20,
	kButtonAlways,
	kButtonOnlyIf,
	kTitleAlways,
	kTitleOnlyIf,
	kButtonTopLeft,
	kButtonTopRight,
	kButtonBottomRight,
	kButtonBottomLeft
};

enum
{
	allsWell=0,
	prefs_allsWell=0,
	prefs_diskReadErr,
	prefs_diskWriteErr,
	prefs_cantOpenPrefsErr,
	prefs_cantCreatePrefsErr,
	prefs_virginErr,
	kNoMemory,
	kCantGetResource,
	kCantFindSystemFolder
};
