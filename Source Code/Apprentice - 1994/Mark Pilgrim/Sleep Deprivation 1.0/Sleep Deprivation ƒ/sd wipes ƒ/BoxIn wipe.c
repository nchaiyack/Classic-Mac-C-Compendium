/**********************************************************************\

File:		BoxIn wipe.c

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

#define CorrectTime 3

void BoxInWipe(GrafPtr, Pattern*);

/* Basically, there are four bars -- one starts at the top and moves down;
   one starts at the bottom and moves up; one starts at the left and moves
   right; one starts at the right and moves left.  There's a lot of overlap
   of bitcopying, but it's masked by the timing correction */
   
void BoxInWipe(GrafPtr thePtr, Pattern *thePattern)
{
	Rect		vsource1,vsource2, hsource1, hsource2;
	int			vbar,hbar;
	int			width,height;
	int			VBarGap, HBarGap;
	
	width=thePtr->portRect.right-thePtr->portRect.left;
	height=thePtr->portRect.bottom-thePtr->portRect.top;
	
	VBarGap=10;
	HBarGap=VBarGap*height/width;
	
	vbar=0;
	hbar=0;
	vsource1.top=vsource2.top=hsource2.left=hsource1.left=0;	/* these */
	vsource1.bottom=vsource2.bottom=height;						/* never */
	hsource1.right=hsource2.right=width;						/* change */
	while (vbar<width/2+VBarGap)
	{
		StartTiming();
		vsource1.left=vbar;
		vsource1.right=vsource1.left+VBarGap;
		vsource2.right=width-vbar;
		vsource2.left=vsource2.right-VBarGap;
		hsource1.top=hbar;
		hsource1.bottom=hsource1.top+HBarGap;
		hsource2.bottom=height-hbar;
		hsource2.top=hsource2.bottom-HBarGap;

		FillRect(&vsource1, *thePattern);
		FillRect(&hsource1, *thePattern);
		FillRect(&vsource2, *thePattern);
		FillRect(&hsource2, *thePattern);

		vbar+=VBarGap;
		hbar+=HBarGap;
		TimeCorrection(CorrectTime);
	}
}
