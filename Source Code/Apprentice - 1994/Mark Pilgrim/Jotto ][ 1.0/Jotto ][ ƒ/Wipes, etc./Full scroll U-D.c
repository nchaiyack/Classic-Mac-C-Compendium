/**********************************************************************\

File:		Full scroll U-D.c

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

#define		BoxSize		10
#define CorrectTime 3

/* Take the full screen minus the bottom strip, move it into the whole screen
   shifted down by BoxSize, then take the bottommost source strip and put it
   in the top strip of the dest. window. */
   
void FullScrollUD(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, int theWindowHeight, int theWindowWidth)
{
	int			x, y;
	Rect		theRect, dest;
	Rect		scrollsource, scrolldest;
	
	scrollsource=destGrafPtr->portRect;
	scrollsource.bottom-=BoxSize;              /* whole screen minus bottom strip */
	scrolldest = scrollsource;
	OffsetRect(&scrolldest, 0, BoxSize);       /* whole screen shifted down BoxSize */
	
	dest = destGrafPtr->portRect;
	dest.bottom=BoxSize;                       /* top strip for new data */
	
	theRect.top=theWindowHeight-BoxSize;    /* bottom strip of source window */
	theRect.bottom=theWindowHeight;
	theRect.left=0;
	theRect.right=theWindowWidth;
	
	for(x = theWindowHeight - BoxSize; x >= 0; x -= BoxSize)
	{
		StartTiming();
		CopyBits(&(destGrafPtr->portBits), &(destGrafPtr->portBits),
				&scrollsource, &scrolldest, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theRect, &dest, 0, 0L);
		theRect.bottom-=BoxSize;
		theRect.top-=BoxSize;
		TimeCorrection(CorrectTime);
	}
}