/**********************************************************************\

File:		Quadrant scroll 2 reversed.c

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

pascal short QuadrantScroll2Reversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* 4 regions, splitting the screen into 4 quadrants.  Scroll the screen down in
   the top-left quadrant, right in the top-right, up in the bottom-right,
   left in the bottom-left. */
   
pascal short QuadrantScroll2Reversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	int			x;
	Rect		tlsource, trsource, blsource, brsource;
	Rect		tldest, trdest, bldest, brdest;
	Rect		tlscroll, trscroll, blscroll, brscroll;
	int			cx,cy;
	int			BoxSize, HBoxSize;
	
	BoxSize=theWindowHeight/(NUM_ITERATIONS*2);
	HBoxSize=theWindowWidth/(NUM_ITERATIONS*2);
	
	cx = boundsRect.left + theWindowWidth / 2;
	cy = boundsRect.top + theWindowHeight / 2;
	
	tlscroll=trscroll=blscroll=brscroll=tldest=trdest=bldest=brdest=boundsRect;
	tlscroll.right=trscroll.left=blscroll.right=brscroll.left=
		tldest.right=trdest.left=bldest.right=brdest.left=cx;
	tlscroll.bottom=trscroll.bottom=blscroll.top=brscroll.top=
		tldest.bottom=trdest.bottom=bldest.top=brdest.top=cy;
	brdest.bottom=brdest.top+BoxSize;
	trdest.right=trdest.left+HBoxSize;
	tldest.top=tldest.bottom-BoxSize;
	bldest.left=bldest.right-HBoxSize;
	SetRect(&tlsource, boundsRect.left, boundsRect.top, cx, boundsRect.top+BoxSize);
	SetRect(&blsource, boundsRect.left, cy, boundsRect.left+HBoxSize, boundsRect.bottom);
	SetRect(&brsource, cx, boundsRect.bottom-BoxSize, boundsRect.right, boundsRect.bottom);
	SetRect(&trsource, boundsRect.right-HBoxSize, boundsRect.top, boundsRect.right, cy);
	
	for (x=0; x<NUM_ITERATIONS; x++)
	{
		StartTiming();
		ScrollTheRect(&trscroll, HBoxSize, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&trsource, &trdest, 0, 0L);
		trsource.left-=HBoxSize;
		trsource.right-=HBoxSize;
		
		ScrollTheRect(&brscroll, 0, BoxSize, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&brsource, &brdest, 0, 0L);
		brsource.top-=BoxSize;
		brsource.bottom-=BoxSize;
		
		ScrollTheRect(&blscroll, -HBoxSize, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&blsource, &bldest, 0, 0L);
		blsource.left+=HBoxSize;
		blsource.right+=HBoxSize;
		
		ScrollTheRect(&tlscroll, 0, -BoxSize, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&tlsource, &tldest, 0, 0L);
		tlsource.top+=BoxSize;
		tlsource.bottom+=BoxSize;
		
		TimeCorrection(CorrectTime);
	}
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);
	
	return 0;
}
