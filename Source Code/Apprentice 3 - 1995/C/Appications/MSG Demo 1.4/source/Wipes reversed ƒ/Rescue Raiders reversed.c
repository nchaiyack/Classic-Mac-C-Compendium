/**********************************************************************\

File:		Rescue Raiders reversed.c

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

#define CorrectTime 5
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short RescueRaidersReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* One region in 8 parts.  Each part of the region either starts at a corner
   or in the middle of a side and moves progressively counterclockwise until the
   entire screen is filled. Named after a similar effect in the game Rescue
   Raiders on the Apple ][e (now called Armor Alley� on the Mac, but it doesn't
   have this effect in it anymore). */
   
pascal short RescueRaidersReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	RgnHandle	curregion;
	int			cx,cy,lastx,lasty;
	int			BlockSize, VBlockSize;
	
	cx = theWindowWidth / 2;
	cy = theWindowHeight / 2;
	BlockSize=theWindowWidth/25;
	VBlockSize=theWindowHeight/25;

	lastx=cx-BlockSize;
	lasty=cy-VBlockSize;
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
		OffsetRgn(curregion, boundsRect.left, boundsRect.top);
		
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&boundsRect, &boundsRect, 0, curregion);

		lastx-=BlockSize;
		lasty-=VBlockSize;

		TimeCorrection(CorrectTime);
	}
	while (lastx>=0);
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);   /* in case we missed any bits */
	
	DisposeRgn(curregion);
	
	return 0;
}
