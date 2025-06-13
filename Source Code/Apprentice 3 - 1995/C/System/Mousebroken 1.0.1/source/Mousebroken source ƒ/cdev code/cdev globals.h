/**********************************************************************\

File:		cdev globals.h

Purpose:	This is the header file for all the structs and enums in
			the cdev.


Mousebroken -=- your computer isn't truly broken until it's mousebroken
Copyright (C) 1993 Mark Pilgrim

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
	unsigned char	showIcon;				/* Mousebroken shows icon on startup? */
	unsigned char	isOn;					/* Mousebroken is on? */
	unsigned long	mouseCodePtr;			/* pointer to mouse module in memory */
	unsigned long	mouseVBLPtr;			/* pointer to VBL structure in memory */
	FSSpec			moduleFS;				/* file specification for module */
	int				moduleIndex;			/* dir index in "Mouse Modules" folder */
	int				moduleRefNum;			/* resource file refnum; used internally */
	int				oldRefNum;				/* used internally */
	unsigned long	moduleIconHandle;		/* used internally */
	unsigned char	moduleAlreadyOpen;		/* used internally */
	unsigned char	unused;
} PrefStruct, *PrefPtr, **PrefHandle;

enum
{
	kModuleIcon=1,
	kModuleName,
	kModuleCopyright,
	kModuleInfo,
	kButtonLeft,
	kButtonRight,
	kButtonOn,
	kTitleOn,
	kButtonOff,
	kTitleOff,
	kButtonShowIcon,
	kTitleShowIcon
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
	kCantFindSystemFolder,
	kCantOpenModuleFolder,
	kNoModules,
	kCantOpenModule
};
