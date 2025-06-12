/**********************************************************************\

File:		gsitc VBL.c

Purpose:	This module handles actually moving the mouse to the right
			corner.
						

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

#include "Retrace.h"

extern Boolean CrsrNew : 0x8CE;
extern Point mTemp : 0x828;
extern Point RawMouse : 0x82C;

unsigned long	me;
int				gOldX;
int				gOldY;
unsigned long	gStartTicks;
unsigned char	gAlways;
unsigned long	gTargetTicks;
int				gTargetX;
int				gTargetY;
Boolean			gNeedNewCursor;

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
	Boolean			needNewCursor;
	
	RememberA0();
	SetUpA4();
	
	asm
	{
		move.l d0, myVBL
	}
	if (me != '©MSG')
	{
		me = '©MSG';
		gTargetTicks=*((unsigned long*)((long)myVBL-8));
		gTargetX=*((int*)((long)myVBL-4));
		gTargetY=*((int*)((long)myVBL-2));
		gAlways=*((unsigned char*)((long)myVBL-12));
		gOldX=-1;
		gOldY=-1;
		gStartTicks=0L;
		gNeedNewCursor=TRUE;
	}
	
	needNewCursor=((RawMouse.h!=gOldX) || (RawMouse.v!=gOldY));
	
	if ((RawMouse.h!=gOldX) || (RawMouse.v!=gOldY))
	{
		gStartTicks=TickCount();
		gOldX=RawMouse.h;
		gOldY=RawMouse.v;
		gNeedNewCursor=TRUE;
	}
	
	if (TickCount()-gStartTicks>=gTargetTicks)
	{
		RawMouse.h=mTemp.h=gOldX=gTargetX;
		RawMouse.v=mTemp.v=gOldY=gTargetY;
		gStartTicks=TickCount();
		if (gNeedNewCursor)
			CrsrNew=TRUE;
		gNeedNewCursor=FALSE;
	}
	
	myVBL->vblCount = 1;
	RestoreA4();
}
