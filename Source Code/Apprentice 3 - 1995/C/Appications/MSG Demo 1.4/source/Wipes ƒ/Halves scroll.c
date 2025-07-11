/**********************************************************************\

File:		Halves scroll.c

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

pascal short HalvesScroll(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* 2 regions, split down the middle of the screen.  Scroll the screen up in one
   region and down in the other. */
   
pascal short HalvesScroll(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	int			x;
	Rect		theTopRect, topdest, theBottomRect, bottomdest;
	Rect		topscrollsource, bottomscrollsource;
	int			cx;
	int			BoxSize;
	
	BoxSize=theWindowHeight/25;
	cx = boundsRect.left + theWindowWidth / 2;

	topscrollsource=topdest=bottomscrollsource=bottomdest=boundsRect;
	topscrollsource.right=topdest.right=bottomscrollsource.left=bottomdest.left=cx;
	topdest.bottom=topdest.top+BoxSize;
	bottomdest.top=bottomdest.bottom-BoxSize;
	SetRect(&theTopRect, boundsRect.left, boundsRect.bottom-BoxSize, cx, boundsRect.bottom);
	SetRect(&theBottomRect, cx, boundsRect.top, boundsRect.right, boundsRect.top+BoxSize);
	
	for(x = theWindowHeight - BoxSize; x >= 0; x -= BoxSize)
	{
		StartTiming();
		ScrollTheRect(&topscrollsource, 0, BoxSize, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theTopRect, &topdest, 0, 0L);
		theTopRect.bottom-=BoxSize;
		theTopRect.top-=BoxSize;
		
		ScrollTheRect(&bottomscrollsource, 0, -BoxSize, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theBottomRect, &bottomdest, 0, 0L);
		theBottomRect.top+=BoxSize;
		theBottomRect.bottom+=BoxSize;
		
		TimeCorrection(CorrectTime);
	}
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);
	
	return 0;
}
