/**********************************************************************\

File:		globals.h

Purpose:	This header file is used by show init.c and sd main.c.
			It gives the structure of a mirror of QuickDraw globals.

Note:		This file was not written by the authors of Sleep Deprivation
			and is not subject to the terms of the GNU General Public
			License.
			

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

struct QDGlobals {
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

Point	RawMouse : 0x82C;
