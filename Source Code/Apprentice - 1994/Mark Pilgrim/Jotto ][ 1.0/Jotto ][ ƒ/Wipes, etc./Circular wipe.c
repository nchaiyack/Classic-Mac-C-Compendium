/**********************************************************************\

File:		Circular wipe.c

Purpose:	Graphic effect from offscreen bitmap to main window (on
			screen).  See comments below for more description.


Jotto ][ -=- a simple word game, revisited
Copyright (C) 1993 Mark Pilgrim

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
#include "wipe dispatch.h"

#define BlockSize 17
#define CorrectTime 2

/* Trace a region from the center to the topleft corner, over <BlockSize> pixels,
   and back to the center.  Fill that in and move the region parameters +BlockSize.
   Repeat for all sides. */

void CircularWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, int theWindowHeight, int theWindowWidth)
{
	RgnHandle	curregion;
	Rect		source;
	int			cx,cy,gap,lastx,lasty;
	
	cx = theWindowWidth / 2;
	cy = theWindowHeight / 2;

	curregion=NewRgn();
	source.top=source.left=0;
	source.bottom=theWindowHeight;
	source.right=theWindowWidth;
	
	gap=BlockSize;
	lastx=0;
	do                                            /* top quadrant */
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(lastx,0);
			LineTo(gap,0);
			LineTo(cx,cy);
		CloseRgn(curregion);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&source, &source, 0, curregion);
		lastx=gap;
		gap+=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (gap<theWindowWidth+BlockSize);
	
	gap=BlockSize;
	lasty=0;
	do                                            /* right quadrant */
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(theWindowWidth,lasty);
			LineTo(theWindowWidth,gap);
			LineTo(cx,cy);
		CloseRgn(curregion);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&source, &source, 0, curregion);
		lasty=gap;
		gap+=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (gap<theWindowHeight+BlockSize);
	
	lastx=theWindowWidth;
	gap=theWindowWidth-BlockSize;
	do                                            /* bottom quadrant */
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(lastx,theWindowHeight);
			LineTo(gap,theWindowHeight);
			LineTo(cx,cy);
		CloseRgn(curregion);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&source, &source, 0, curregion);
		lastx=gap;
		gap-=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (gap>-BlockSize);
	
	lasty=theWindowHeight;
	gap=theWindowHeight-BlockSize;
	do                                            /* left quadrant */
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(0,lasty);
			LineTo(0,gap);
			LineTo(cx,cy);
		CloseRgn(curregion);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&source, &source, 0, curregion);
		lastx=gap;
		gap-=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (gap>-BlockSize);
	
	DisposeRgn(curregion);
}
	