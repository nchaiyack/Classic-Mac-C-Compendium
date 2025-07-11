/**********************************************************************\

File:		Scissors reversed.c

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
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short ScissorsReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* A region in two parts, starting at the middle of the bottom side and working
   toward the bottom corners, then up the left and right sides. */
   
pascal short ScissorsReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	RgnHandle	curregion;
	int			cx,gap,lastx,lasty;
	int			BlockSize;
	
	BlockSize=theWindowHeight/25;
	cx = theWindowWidth / 2;

	curregion=NewRgn();
	lastx=theWindowWidth/2;
	gap=theWindowWidth/2+BlockSize;
	do
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,0);
		OpenRgn();
			LineTo(lastx,theWindowHeight);		/* get the right half on bottom side */
			LineTo(gap,theWindowHeight);
			LineTo(cx,0);
			LineTo(theWindowWidth-lastx,theWindowHeight);	/* left 1/2 on bottom */
			LineTo(theWindowWidth-gap,theWindowHeight);
			LineTo(cx,0);
		CloseRgn(curregion);
		OffsetRgn(curregion, boundsRect.left, boundsRect.top);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&boundsRect, &boundsRect, 0, curregion);
		lastx=gap;
		gap+=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (gap<=theWindowWidth+BlockSize);
	
	gap=theWindowHeight-BlockSize;
	lasty=theWindowHeight;
	do
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,0);
		OpenRgn();
			LineTo(theWindowWidth,lasty);   /* get the right half */
			LineTo(theWindowWidth,gap);
			LineTo(cx,0);
			LineTo(0,lasty);                   /* get the left half */
			LineTo(0,gap);
			LineTo(cx,0);
		CloseRgn(curregion);
		OffsetRgn(curregion, boundsRect.left, boundsRect.top);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&boundsRect, &boundsRect, 0, curregion);
		lasty=gap;
		gap-=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (gap>=0);
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);		/* if we missed any bits */
	
	DisposeRgn(curregion);
	
	return 0;
}
