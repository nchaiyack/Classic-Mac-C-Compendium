/**********************************************************************\

File:		stoned mouse VBL.c

Purpose:	This module handles setting low-level globals that make
			the mouse's movement more jerky.
			

Stoned Mouse -=- a slightly distracted mouse
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

#include "Retrace.h"

extern Boolean	CrsrNew : 0x8CE;
extern Point	mTemp : 0x828;
extern Point	RawMouse : 0x82C;

void main(void);

void main(void)
{
	VBLTask*		myVBL;
	
	asm
	{
		move.l d0, myVBL
	}
	mTemp.h=RawMouse.h;
	mTemp.v=RawMouse.v;
	CrsrNew = TRUE;
	myVBL->vblCount = 1;
}
