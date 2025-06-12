/**********************************************************************\

File:		Four corner wipe.c

Purpose:	Graphic effect from offscreen bitmap to main window (on
			screen).  See comments below for more description.


Jotto ][ -=- a simple word game, revisited
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

#include "msg timing.h"
#include "wipe dispatch.h"

#define VBarGap 5
#define HBarGap (VBarGap*theWindowHeight/theWindowWidth)
#define CorrectTime 1

/* Take 4 bars, two on each axis, and move them towards different corners.
   This means lots of overlap copying, but the timing masks it and Quickdraw
   may even take care of some of it. (?) */

void FourCorner(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, int theWindowHeight, int theWindowWidth)
{
	Rect		vsource1,hsource1,vsource2,hsource2;
	int			vbar,hbar,cx,cy;
	
	vbar=VBarGap;
	hbar=HBarGap;
	cx = theWindowWidth/2;
	cy = theWindowHeight/2;
	vsource1.top=vsource2.top=hsource2.left=hsource1.left=0;    /* these */
	vsource1.bottom=vsource2.bottom=theWindowHeight;         /* never */
	hsource1.right=hsource2.right=theWindowWidth;            /* change */
	while (vbar<cx+VBarGap)
	{
		StartTiming();
		vsource1.left=cx-vbar;
		vsource1.right=vsource1.left+VBarGap;
		vsource2.right=cx+vbar;
		vsource2.left=vsource2.right-VBarGap;
		hsource1.top=cy-hbar;
		hsource1.bottom=hsource1.top+HBarGap;
		hsource2.bottom=cy+hbar;
		hsource2.top=hsource2.bottom-HBarGap;
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&vsource1, &vsource1, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&hsource1, &hsource1, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&vsource2, &vsource2, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&hsource2, &hsource2, 0, 0L);
		vbar+=VBarGap;
		hbar+=HBarGap;
		TimeCorrection(CorrectTime);
	}
}