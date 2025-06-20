/**********************************************************************\

File:		Circular wipe.c

Purpose:	This module handles clearing the screen in a funky
			manner.  See the comments below for more details.
			

Sleep Deprivation -- graphic effects on sleep
Copyright (C) 1993 Mark Pilgrim & Dave Blumenthal

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

#define BlockSize 17
#define CorrectTime 2

void CircularWipe(GrafPtr, Pattern*);

/* Trace a region from the center to the topleft corner, over <BlockSize> pixels,
   and back to the center.  Fill that in and move the region parameters +BlockSize.
   Repeat for all sides. */

void CircularWipe(GrafPtr thePtr, Pattern *thePattern)
{
	RgnHandle	curregion;
	Rect		source;
	int			cx,cy,gap,lastx,lasty;
	int			width,height;
	
	width=thePtr->portRect.right-thePtr->portRect.left;
	height=thePtr->portRect.bottom-thePtr->portRect.top;
	
	cx = (thePtr->portRect.right + thePtr->portRect.left) / 2;
	cy = (thePtr->portRect.bottom + thePtr->portRect.top) / 2;
	
	curregion=NewRgn();
	source.top=source.left=0;
	source.bottom=height;
	source.right=width;
	
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

		FillRgn(curregion, *thePattern);
		
		lastx=gap;
		gap+=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (gap<width+BlockSize);
	
	gap=BlockSize;
	lasty=0;
	do                                            /* right quadrant */
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(width,lasty);
			LineTo(width,gap);
			LineTo(cx,cy);
		CloseRgn(curregion);

		FillRgn(curregion, *thePattern);
		
		lasty=gap;
		gap+=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (gap<height+BlockSize);
	
	lastx=width;
	gap=width-BlockSize;
	do                                            /* bottom quadrant */
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(lastx,height);
			LineTo(gap,height);
			LineTo(cx,cy);
		CloseRgn(curregion);

		FillRgn(curregion, *thePattern);
		
		lastx=gap;
		gap-=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (gap>-BlockSize);
	
	lasty=height;
	gap=height-BlockSize;
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

		FillRgn(curregion, *thePattern);
		
		lastx=gap;
		gap-=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (gap>-BlockSize);
	
	DisposeRgn(curregion);
}
	