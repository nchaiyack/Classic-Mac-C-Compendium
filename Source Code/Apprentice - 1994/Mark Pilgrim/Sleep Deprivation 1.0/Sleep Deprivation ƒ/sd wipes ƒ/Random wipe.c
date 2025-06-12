/**********************************************************************\

File:		Random wipe.c

Purpose:	This module handles clearing the screen in a funky
			manner.  See the comments below for more details.
			

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

#include "msg timing.h"

#define		SUB_HOR		25
#define		SUB_VER		25
#define		AREA		(SUB_HOR * SUB_VER)
#define CorrectTime 1

void RandomWipe(GrafPtr, Pattern*);

/* Basically, we divide the window into a bunch of blocks, and copy
each to the screen in random order. */
void RandomWipe(GrafPtr thePtr, Pattern *thePattern)
{
	int		order[AREA];
	int		i;
	long	randtemp;
	int		ordertemp;
	Rect	subBox;
	Rect	dest;
	Boolean	everyOther;
	int		width,height;
	
	width=thePtr->portRect.right-thePtr->portRect.left;
	height=thePtr->portRect.bottom-thePtr->portRect.top;

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
		subBox.top = (((order[i] / SUB_VER) *
						height)
						/ SUB_VER) + (thePtr->portRect).top;
		subBox.left = (((order[i] % SUB_HOR) *
						width)
						/ SUB_HOR) + (thePtr->portRect).left;
		subBox.bottom = ((((order[i] / SUB_VER) + 1) *
						height)
						/ SUB_VER) + (thePtr->portRect).top;
		subBox.right = ((((order[i] % SUB_HOR) + 1) *
						width)
						/ SUB_HOR) + (thePtr->portRect).left;
	
		FillRect(&subBox, *thePattern);
			
		if (everyOther)
			TimeCorrection(CorrectTime);
		everyOther=!everyOther;
	}
}
