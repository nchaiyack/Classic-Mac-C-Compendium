/**********************************************************************\

File:		Four centered fade reversed.c

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

#define CorrectTime 6
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short FourCenteredFadeReversed(Rect boundsRect, Pattern *thePattern);

/* Really, this only uses one region and one CopyBits per round.  Regions don't
   have to be continuous.  On each axis, on each side of the center of the screen,
   there are two parts of the region which move from the corners to the center. */
   
pascal short FourCenteredFadeReversed(Rect boundsRect, Pattern *thePattern)
{
	RgnHandle	curregion, boundsRgn, sectrgn;
	int			cx,cy,lastx,lasty;
	int			BlockSize, VBlockSize;
	
	cx = boundsRect.left + theWindowWidth / 2;
	cy = boundsRect.top + theWindowHeight / 2;
	BlockSize=theWindowWidth/50;
	VBlockSize=theWindowHeight/50;

	boundsRgn=NewRgn();
	SetRectRgn(boundsRgn, boundsRect.left, boundsRect.top, boundsRect.right, boundsRect.bottom);
	sectrgn=NewRgn();
	
	curregion=NewRgn();
	
	lasty=theWindowHeight/2;
	lastx=theWindowWidth/2;
	do
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();              /* much much faster than 8 regions/CopyBits */
			LineTo(cx-lastx,boundsRect.top);
			Line(-BlockSize,0);
			LineTo(cx+lastx+BlockSize,boundsRect.bottom);
			Line(-BlockSize,0);
			LineTo(cx,cy);
			
			LineTo(cx+lastx,boundsRect.top);
			Line(BlockSize,0);
			LineTo(cx-lastx-BlockSize,boundsRect.bottom);
			Line(BlockSize,0);
			LineTo(cx,cy);
			
			LineTo(boundsRect.right,cy-lasty);
			Line(0,-VBlockSize);
			LineTo(boundsRect.left,cy+lasty+VBlockSize);
			Line(0,-VBlockSize);
			LineTo(cx,cy);
			
			LineTo(boundsRect.right,cy+lasty);
			Line(0,VBlockSize);
			LineTo(boundsRect.left,cy-lasty-VBlockSize);
			Line(0,VBlockSize);
			LineTo(cx,cy);
		CloseRgn(curregion);

		SectRgn(curregion, boundsRgn, sectrgn);
		FillRgn(sectrgn, *thePattern);
		lastx-=BlockSize;
		lasty-=VBlockSize;
		TimeCorrection(CorrectTime);
	}
	while (lastx>boundsRect.left);

	FillRect(&boundsRect, *thePattern);		/* in case we miss a few pixels */
	
	DisposeRgn(curregion);
	DisposeRgn(boundsRgn);
	DisposeRgn(sectrgn);
	
	return 0;
}
