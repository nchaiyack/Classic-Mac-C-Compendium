/**********************************************************************\

File:		Rescue Raiders.c

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

#define NUMREGIONS 25
#define BlockSize 10
#define CorrectTime 5

/* One region in 8 parts.  Each part of the region either starts at a corner
   or in the middle of a side and moves progressively clockwise until the
   entire screen is filled. Named after a similar effect in the game Rescue
   Raiders on the Apple ][e (now called Armor Alleyª on the Mac, but it doesn't
   have this effect in it anymore). */
   
void RescueRaiders(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, int theWindowHeight, int theWindowWidth)
{
	RgnHandle	curregion;
	Rect		source;
	int			cx,cy,lastx,lasty,VBlockSize;
	
	cx = (destGrafPtr->portRect.right + destGrafPtr->portRect.left) / 2;
	cy = (destGrafPtr->portRect.bottom + destGrafPtr->portRect.top) / 2;
	VBlockSize=BlockSize*theWindowHeight/theWindowWidth;

	source.top=source.left=0;
	source.bottom=theWindowHeight;
	source.right=theWindowWidth;
	
	lasty=lastx=0;
	curregion=NewRgn();
	do
	{
		StartTiming();

		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(lastx,0);
			Line(BlockSize,0);
			LineTo(theWindowWidth-lastx-BlockSize,theWindowHeight);
			Line(BlockSize,0);
			LineTo(cx,cy);
			
			LineTo(cx+lastx,0);
			Line(BlockSize,0);
			LineTo(cx-lastx-BlockSize,theWindowHeight);
			Line(BlockSize,0);
			LineTo(cx,cy);
			
			LineTo(theWindowWidth,lasty);
			Line(0,VBlockSize);
			LineTo(0,theWindowHeight-lasty-VBlockSize);
			Line(0,VBlockSize);
			LineTo(cx,cy);
			
			LineTo(theWindowWidth,cy+lasty);
			Line(0,VBlockSize);
			LineTo(0,cy-lasty-VBlockSize);
			Line(0,VBlockSize);
			LineTo(cx,cy);
		CloseRgn(curregion);

		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&source, &source, 0, curregion);

		lastx+=BlockSize;
		lasty+=VBlockSize;

		TimeCorrection(CorrectTime);
	}
	while (lastx<cx);
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&source, &source, 0, 0L);   /* in case we missed any bits */
	
	DisposeRgn(curregion);
}
