/**********************************************************************\

File:		gravity VBL.c

Purpose:	This module handles the actual VBL routine which moves
			the mouse towards the bottom of the screen.
			

Gravity Mouse -=- add gravity to your mouse
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
#include "GestaltEQU.h"

extern Boolean CrsrNew : 0x8CE;
extern Point mTemp : 0x828;

Rect			gMainScreenBounds;
unsigned long	me;
Boolean			fallingNow;
int				gap;

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
	long			gestalt_temp;
	OSErr			isHuman;
	Boolean			gHasColorQD;
	
	RememberA0();
	SetUpA4();
	
	asm
	{
		move.l d0, myVBL
	}
	
	if (me != '©MSG')
	{
		isHuman = Gestalt(gestaltQuickdrawVersion, &gestalt_temp);
		gHasColorQD = !(isHuman || (gestalt_temp < gestalt8BitQD));
		gMainScreenBounds = (gHasColorQD) ? (**GetMainDevice()).gdRect : screenBits.bounds;
		me = '©MSG';
		fallingNow=FALSE;
	}
	
	if (!fallingNow)
		gap=1;
	
	fallingNow=TRUE;
	mTemp.v+=gap/5;
	if (mTemp.v>gMainScreenBounds.bottom)
		fallingNow=FALSE;
	gap++;
	CrsrNew = TRUE;

	myVBL->vblCount = (fallingNow) ? 1 : ((TickCount()&0x7fffffff)%4096)+2500;
	RestoreA4();
}
