/**********************************************************************\

File:		Split scroll U-D.c

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

#define CorrectTime 4
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short SplitScrollUD(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Draw a line from the topleft to the bottomright of the screen and split the
   screen into two regions.  The top region scrolls down and the bottom region
   scrolls up. */
   
pascal short SplitScrollUD(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	int			x;
	Rect		theTopRect, topdest, theBottomRect, bottomdest;
	Rect		topscrollsource, topscrolldest, bottomscrollsource, bottomscrolldest;
	RgnHandle	toprgn,bottomrgn;
	int			BoxSize;
	
	BoxSize=theWindowHeight/25;
	toprgn=NewRgn();
	SetEmptyRgn(toprgn);
	MoveTo(0,0);
	OpenRgn();
		Line(theWindowWidth,0);
		Line(0,theWindowHeight);
		LineTo(0,0);
	CloseRgn(toprgn);
	OffsetRgn(toprgn, boundsRect.left, boundsRect.top);
	
	bottomrgn=NewRgn();
	SetEmptyRgn(bottomrgn);
	MoveTo(0,0);
	OpenRgn();
		Line(0,theWindowHeight);
		Line(theWindowWidth,0);
		LineTo(0,0);
	CloseRgn(bottomrgn);
	OffsetRgn(bottomrgn, boundsRect.left, boundsRect.top);
	
	topscrollsource=boundsRect;
	topscrollsource.bottom-=BoxSize;
	topscrolldest = topscrollsource;
	OffsetRect(&topscrolldest, 0, BoxSize);
	topdest = boundsRect;
	topdest.bottom=topdest.top+BoxSize;
	SetRect(&theTopRect, boundsRect.left, boundsRect.bottom-BoxSize, boundsRect.right, boundsRect.bottom);
	
	bottomscrollsource=boundsRect;
	bottomscrollsource.top+=BoxSize;
	bottomscrolldest=bottomscrollsource;
	OffsetRect(&bottomscrolldest, 0, -BoxSize);
	bottomdest=boundsRect;
	bottomdest.top=bottomdest.bottom-BoxSize;
	SetRect(&theBottomRect, boundsRect.left, boundsRect.top, boundsRect.right, boundsRect.top+BoxSize);
	
	for(x = theWindowHeight - BoxSize; x >= 0; x -= BoxSize)
	{
		StartTiming();
		CopyBits(&(destGrafPtr->portBits), &(destGrafPtr->portBits),
				&topscrollsource, &topscrolldest, 0, toprgn);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theTopRect, &topdest, 0, toprgn);
		theTopRect.bottom-=BoxSize;
		theTopRect.top-=BoxSize;
		
		CopyBits(&(destGrafPtr->portBits), &(destGrafPtr->portBits),
				&bottomscrollsource, &bottomscrolldest, 0, bottomrgn);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theBottomRect, &bottomdest, 0, bottomrgn);
		theBottomRect.top+=BoxSize;
		theBottomRect.bottom+=BoxSize;
		
		TimeCorrection(CorrectTime);
	}
	
	return 0;
}
