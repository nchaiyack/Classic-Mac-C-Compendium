/**********************************************************************\

File:		crooked mouse VBL.c

Purpose:	This module handles setting low-level globals that make
			the mouse's movement go at a 45-degree angle (rotated
			counterclockwise) from the way the user moved it.
			

Crooked Mouse -=- a mouse rotated 45 degrees
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

extern Boolean CrsrNew : 0x8CE;
extern Point mTemp : 0x828;
extern Point RawMouse : 0x82C;

unsigned long	me;
int				oldX, oldY;

void header(void);
void main(void);

void header(void)
{
	asm {
		dc.l	0
		move.l a0, d0
		lea header, a0
		jmp main
	}
}

#include "SetUpA4.h"

void main(void)
{
	VBLTask*		myVBL;
	int				vx,vy;
	
	RememberA0();
	SetUpA4();
	
	asm
	{
		move.l d0, myVBL
	}
	
	if (me != '�MSG')
	{
		me = '�MSG';
		oldX=RawMouse.h;
		oldY=RawMouse.v;
	}
	
	vx=RawMouse.h-oldX;
	vy=RawMouse.v-oldY;
	
	RawMouse.h+=vy;
	mTemp.h+=vy;
	RawMouse.v-=vx;
	mTemp.v-=vx;

	oldX=RawMouse.h;
	oldY=RawMouse.v;
	
	CrsrNew = TRUE;
	myVBL->vblCount = 1;
	RestoreA4();
}
