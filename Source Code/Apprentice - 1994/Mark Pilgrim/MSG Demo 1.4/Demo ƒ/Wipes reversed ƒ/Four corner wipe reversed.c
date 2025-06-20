/**********************************************************************\

File:		Four corner wipe reversed.c

Purpose:	Graphic effect from offscreen bitmap to main window (on
			screen).  See comments below for more description.

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

#include "timing.h"

#define CorrectTime 1
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short FourCornerReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Take four rects, one at each corner, and make them bigger, growing on each
   side of the screen.  This means lots of overlap copying, but the timing masks
   it and Quickdraw may even take care of some of it. (?) */

pascal short FourCornerReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect		tl,tr,bl,br;
	int			cx,cy;
	int			VBarGap, HBarGap;
	
	VBarGap=theWindowWidth/100;
	HBarGap=theWindowHeight/100;

	cx = boundsRect.left + theWindowWidth/2;
	cy = boundsRect.top + theWindowHeight/2;
	
	tl.top=tr.top=tl.bottom=tr.bottom=boundsRect.top;
	tl.left=bl.left=tl.right=bl.right=boundsRect.left;
	tr.right=br.right=tr.left=br.left=boundsRect.right;
	bl.bottom=br.bottom=bl.top=br.top=boundsRect.bottom;
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
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&tl, &tl, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&tr, &tr, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&bl, &bl, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&br, &br, 0, 0L);
		TimeCorrection(CorrectTime);
	}
	
	return 0;
}
