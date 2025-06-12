/**********************************************************************\

File:		Four corner fade.c

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

#define CorrectTime 1
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short FourCornerFade(Rect boundsRect, Pattern *thePattern);

/* Take 4 bars, two on each axis, and move them towards different corners.
   This means lots of overlap copying, but the timing masks it and Quickdraw
   may even take care of some of it. (?) */

pascal short FourCornerFade(Rect boundsRect, Pattern *thePattern)
{
	Rect		vsource1,hsource1,vsource2,hsource2;
	int			vbar,hbar,cx,cy;
	int			VBarGap, HBarGap;
	
	VBarGap=theWindowWidth/100;
	HBarGap=theWindowHeight/100;

	vbar=VBarGap;
	hbar=HBarGap;
	cx = boundsRect.left + theWindowWidth/2;
	cy = boundsRect.top + theWindowHeight/2;
	vsource1.top=vsource2.top=boundsRect.top;
	hsource2.left=hsource1.left=boundsRect.left;
	vsource1.bottom=vsource2.bottom=boundsRect.bottom;
	hsource1.right=hsource2.right=boundsRect.right;
	while (vbar+boundsRect.left<cx+VBarGap)
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
		SectRect(&vsource1, &boundsRect, &vsource1);
		SectRect(&hsource1, &boundsRect, &hsource1);
		SectRect(&vsource2, &boundsRect, &vsource2);
		SectRect(&hsource2, &boundsRect, &hsource2);
		FillRect(&vsource1, *thePattern);
		FillRect(&hsource1, *thePattern);
		FillRect(&vsource2, *thePattern);
		FillRect(&hsource2, *thePattern);
		vbar+=VBarGap;
		hbar+=HBarGap;
		TimeCorrection(CorrectTime);
	}
	
	return 0;
}
