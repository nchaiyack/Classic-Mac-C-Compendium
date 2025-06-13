/**********************************************************************\

File:		Quadrant fade reversed.c

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

pascal short QuadrantFadeReversed(Rect boundsRect, Pattern *thePattern);

/* 4 regions, splitting the screen into 4 quadrants.  Wipe the screen right in
   the top-left quadrant, down in the top-right, left in the bottom-right,
   up in the bottom-left. */
   
pascal short QuadrantFadeReversed(Rect boundsRect, Pattern *thePattern)
{
	int			x;
	Rect		tldest, trdest, bldest, brdest;
	Rect		tlbounds, trbounds, blbounds, brbounds;
	int			cx,cy;
	int			BoxSize, HBoxSize;
	
	BoxSize=theWindowHeight/(NUM_ITERATIONS*2);
	HBoxSize=theWindowWidth/(NUM_ITERATIONS*2);
	
	cx = boundsRect.left + theWindowWidth / 2;
	cy = boundsRect.top + theWindowHeight / 2;
	
	tlbounds=trbounds=blbounds=brbounds=tldest=trdest=bldest=brdest=boundsRect;
	tlbounds.right=trbounds.left=blbounds.right=brbounds.left=
		tldest.right=trdest.left=bldest.right=brdest.left=cx;
	tlbounds.bottom=trbounds.bottom=blbounds.top=brbounds.top=
		tldest.bottom=trdest.bottom=bldest.top=brdest.top=cy;
	brdest.right=brdest.left+HBoxSize;
	bldest.bottom=bldest.top+BoxSize;
	tldest.left=tldest.right-HBoxSize;
	trdest.top=trdest.bottom-BoxSize;
	
	for (x=0; x<NUM_ITERATIONS; x++)
	{
		StartTiming();
		FillRect(&brdest, *thePattern);
		brdest.left+=HBoxSize;
		brdest.right+=HBoxSize;
		
		FillRect(&bldest, *thePattern);
		bldest.top+=BoxSize;
		bldest.bottom+=BoxSize;
		
		FillRect(&tldest, *thePattern);
		tldest.left-=HBoxSize;
		tldest.right-=HBoxSize;
		
		FillRect(&trdest, *thePattern);
		trdest.top-=BoxSize;
		trdest.bottom-=BoxSize;
		
		TimeCorrection(CorrectTime);
	}
	
	FillRect(&boundsRect, *thePattern);		/* in case we miss a few pixels */
	
	return 0;
}
