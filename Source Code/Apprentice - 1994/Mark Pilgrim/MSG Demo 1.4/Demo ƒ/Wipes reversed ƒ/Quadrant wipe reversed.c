/**********************************************************************\

File:		Quadrant wipe reversed.c

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

pascal short QuadrantWipeReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* 4 regions, splitting the screen into 4 quadrants.  Wipe the screen right in
   the top-left quadrant, down in the top-right, left in the bottom-right,
   up in the bottom-left. */
   
pascal short QuadrantWipeReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	int			x;
	Rect		tldest, trdest, bldest, brdest;
	Rect		tlbounds, trbounds, blbounds, brbounds;
	RgnHandle	tlrgn, trrgn, blrgn, brrgn;
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
	tlrgn=NewRgn();
	trrgn=NewRgn();
	blrgn=NewRgn();
	brrgn=NewRgn();
	RectRgn(tlrgn, &tlbounds);
	RectRgn(trrgn, &trbounds);
	RectRgn(blrgn, &blbounds);
	RectRgn(brrgn, &brbounds);
	brdest.right=brdest.left+HBoxSize;
	bldest.bottom=bldest.top+BoxSize;
	tldest.left=tldest.right-HBoxSize;
	trdest.top=trdest.bottom-BoxSize;
	
	for (x=0; x<NUM_ITERATIONS; x++)
	{
		StartTiming();
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&brdest, &brdest, 0, brrgn);
		brdest.left+=HBoxSize;
		brdest.right+=HBoxSize;
		
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&bldest, &bldest, 0, blrgn);
		bldest.top+=BoxSize;
		bldest.bottom+=BoxSize;
		
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&tldest, &tldest, 0, tlrgn);
		tldest.left-=HBoxSize;
		tldest.right-=HBoxSize;
		
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&trdest, &trdest, 0, trrgn);
		trdest.top-=BoxSize;
		trdest.bottom-=BoxSize;
		
		TimeCorrection(CorrectTime);
	}
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);
	
	DisposeRgn(tlrgn);
	DisposeRgn(trrgn);
	DisposeRgn(blrgn);
	DisposeRgn(brrgn);
	
	return 0;
}
