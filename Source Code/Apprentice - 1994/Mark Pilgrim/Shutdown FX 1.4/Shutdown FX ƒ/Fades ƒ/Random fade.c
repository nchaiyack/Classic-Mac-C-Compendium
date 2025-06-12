/**********************************************************************\

File:		Random fade.c

Purpose:	Graphic effect to fade main screen to solid pattern.
			See comments below for more description.


Shutdown FX -=- graphic effects on shutdown
Copyright (C) 1993-4, Mark Pilgrim & Dave Blumenthal

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

#include "msg timing.h"

#define		SUB_HOR		20
#define		SUB_VER		20
#define		AREA		(SUB_HOR * SUB_VER)
#define CorrectTime 1
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short RandomFade(Rect boundsRect, Pattern *thePattern);

/* Basically, we divide the window into a bunch of blocks, and copy
each to the screen in random order. */

pascal short RandomFade(Rect boundsRect, Pattern *thePattern)
{
	int				order[AREA];
	int				i;
	long			randtemp;
	int				ordertemp;
	Rect			subBox;
	Rect			dest;
	Boolean			everyOther;
	
	everyOther=FALSE;
	for(i = 0; i < AREA; i++)
		order[i] = i;
	
	for(i = (AREA - 1); i >= 0; i--) {
		randtemp = ((((long)Random()) +32767) * (i + 1)) / 65535;
		
		ordertemp = order[randtemp];
		order[randtemp] = order[i];
		order[i] = ordertemp;
	}
	
	for(i = 0; i < AREA; i++) {
		StartTiming();
		subBox.top = ((order[i] / SUB_VER) * theWindowHeight) / SUB_VER;
		subBox.left = ((order[i] % SUB_HOR) * theWindowWidth) / SUB_HOR;
		subBox.bottom = (((order[i] / SUB_VER) + 1) * theWindowHeight) / SUB_VER;
		subBox.right = (((order[i] % SUB_HOR) + 1) * theWindowWidth) / SUB_HOR;
		OffsetRect(&subBox, boundsRect.left, boundsRect.top);
		FillRect(&subBox, *thePattern);
		if (everyOther)
			TimeCorrection(CorrectTime);
		everyOther=!everyOther;
	}
	
	return 0;
}
