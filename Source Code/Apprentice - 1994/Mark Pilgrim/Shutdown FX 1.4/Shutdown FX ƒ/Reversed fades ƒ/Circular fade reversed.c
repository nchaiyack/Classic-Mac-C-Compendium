/**********************************************************************\

File:		Circular fade reversed.c

Purpose:	Graphic effect to fade main screen to solid pattern.
			See comments below for more description.


Shutdown FX -=- graphic effects on shutdown
Copyright (C) 1993-4, Mark Pilgrim & Dave Blumenthal

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

#define CorrectTime 2
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CircularFadeReversed(Rect boundsRect, Pattern *thePattern);

/* Trace a region from the center to the topleft corner, down <BlockSize> pixels,
   and back to the center.  Fill that in and move the region parameters +BlockSize.
   Repeat for all sides (counterclockwise). */

pascal short CircularFadeReversed(Rect boundsRect, Pattern *thePattern)
{
	RgnHandle	curregion, boundsRgn, sectrgn;
	int			cx,cy,lastx,lasty;
	int			BlockSize;
	
	BlockSize=theWindowWidth/40;
	cx = boundsRect.left + theWindowWidth / 2;
	cy = boundsRect.top + theWindowHeight / 2;

	boundsRgn=NewRgn();
	SetRectRgn(boundsRgn, boundsRect.left, boundsRect.top, boundsRect.right, boundsRect.bottom);
	
	sectrgn=NewRgn();
	curregion=NewRgn();
	lasty=boundsRect.top;
	do                                            /* left quadrant */
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(boundsRect.left,lasty);
			Line(0, BlockSize);
			LineTo(cx,cy);
		CloseRgn(curregion);
		SectRgn(curregion, boundsRgn, sectrgn);
		FillRgn(sectrgn, *thePattern);
		lasty+=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (lasty<boundsRect.bottom);
	
	lastx=boundsRect.left;
	do                                            /* bottom quadrant */
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(lastx,boundsRect.bottom);
			Line(BlockSize, 0);
			LineTo(cx,cy);
		CloseRgn(curregion);
		SectRgn(curregion, boundsRgn, sectrgn);
		FillRgn(sectrgn, *thePattern);
		lastx+=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (lastx<boundsRect.right);
	
	lasty=boundsRect.bottom;
	do                                            /* right quadrant */
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(boundsRect.right,lasty);
			Line(0,-BlockSize);
			LineTo(cx,cy);
		CloseRgn(curregion);
		SectRgn(curregion, boundsRgn, sectrgn);
		FillRgn(sectrgn, *thePattern);
		lasty-=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (lasty>boundsRect.top-BlockSize);
	
	lastx=boundsRect.right;
	do                                            /* top quadrant */
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(lastx,boundsRect.top);
			Line(-BlockSize,0);
			LineTo(cx,cy);
		CloseRgn(curregion);
		SectRgn(curregion, boundsRgn, sectrgn);
		FillRgn(sectrgn, *thePattern);
		lastx-=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (lastx>boundsRect.left-BlockSize);
	
	DisposeRgn(curregion);
	DisposeRgn(boundsRgn);
	DisposeRgn(sectrgn);
	
	return 0;
}
	