/**********************************************************************\

File:		Quadrant fade 2.c

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

#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define NUM_ITERATIONS	25

pascal short QuadrantFade2(Rect boundsRect, Pattern *thePattern);

/* 4 regions, splitting the screen into 4 quadrants.  Wipe the screen right in
   the top-left quadrant, down in the top-right, left in the bottom-right,
   up in the bottom-left. */
   
pascal short QuadrantFade2(Rect boundsRect, Pattern *thePattern)
{
	int			x;
	Rect		tldest, trdest, bldest, brdest;
	int			cx,cy;
	int			BoxSize, HBoxSize;
	
	BoxSize=theWindowHeight/(NUM_ITERATIONS*2);
	HBoxSize=theWindowWidth/(NUM_ITERATIONS*2);
	
	cx = boundsRect.left + theWindowWidth / 2;
	cy = boundsRect.top + theWindowHeight / 2;
	
	tldest=trdest=bldest=brdest=boundsRect;
	tldest.right=trdest.left=bldest.right=brdest.left=cx;
	tldest.bottom=trdest.bottom=bldest.top=brdest.top=cy;
	bldest.right=bldest.left+HBoxSize;
	tldest.bottom=tldest.top+BoxSize;
	trdest.left=trdest.right-HBoxSize;
	brdest.top=brdest.bottom-BoxSize;
	
	for (x=0; x<NUM_ITERATIONS; x++)
	{
		StartTiming();
		FillRect(&bldest, *thePattern);
		bldest.left+=HBoxSize;
		bldest.right+=HBoxSize;
		
		FillRect(&tldest, *thePattern);
		tldest.top+=BoxSize;
		tldest.bottom+=BoxSize;
		
		FillRect(&trdest, *thePattern);
		trdest.left-=HBoxSize;
		trdest.right-=HBoxSize;
		
		FillRect(&brdest, *thePattern);
		brdest.top-=BoxSize;
		brdest.bottom-=BoxSize;
		
		TimeCorrection(CorrectTime);
	}
	
	FillRect(&boundsRect, *thePattern);			/* fill the whole screen to end it */
	
	return 0;
}
