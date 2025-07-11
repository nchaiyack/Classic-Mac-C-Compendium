/**********************************************************************\

File:		Four corner fade reversed.c

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

pascal short FourCornerFadeReversed(Rect boundsRect, Pattern *thePattern);

/* Take four rects, one at each corner, and make them bigger, growing on each
   side of the screen.  This means lots of overlap copying, but the timing masks
   it and Quickdraw may even take care of some of it. (?) */

pascal short FourCornerFadeReversed(Rect boundsRect, Pattern *thePattern)
{
	Rect		tl,tr,bl,br;
	int			cx,cy;
	int			VBarGap, HBarGap;
	
	VBarGap=theWindowWidth/100;
	HBarGap=theWindowHeight/100;

	cx = boundsRect.left + theWindowWidth/2;
	cy = boundsRect.top + theWindowHeight/2;
	
	tl.top=tr.top=boundsRect.top;
	tl.left=bl.left=boundsRect.left;
	tr.right=br.right=boundsRect.right;
	bl.bottom=br.bottom=boundsRect.bottom;
	
	tl.right=tr.left=bl.right=br.left=cx;
	tl.bottom=tr.bottom=bl.top=br.top=cy;
	
	while (tl.right>0)
	{
		tl.bottom-=HBarGap;
		tr.bottom-=HBarGap;
		bl.top+=HBarGap;
		br.top+=HBarGap;
		tl.right-=VBarGap;
		tr.left+=VBarGap;
		bl.right-=VBarGap;
		br.left+=VBarGap;
	}
	
	while (tl.right<cx)
	{
		StartTiming();
		tl.bottom+=HBarGap;
		tr.bottom+=HBarGap;
		bl.top-=HBarGap;
		br.top-=HBarGap;
		tl.right+=VBarGap;
		bl.right+=VBarGap;
		tr.left-=VBarGap;
		br.left-=VBarGap;
		FillRect(&tl, *thePattern);
		FillRect(&tr, *thePattern);
		FillRect(&bl, *thePattern);
		FillRect(&br, *thePattern);
		TimeCorrection(CorrectTime);
	}
	
	return 0;
}
