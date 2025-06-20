/**********************************************************************\

File:		Diagonal wipe.c

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

#define BoxSize 13
#define CorrectTime 2

/* If you make a long enough region starting at the bottomright corner and
   making a strip up and right of <BoxSize> width, all you have to do is move
   the region left until the entire screen is filled.  Dave thinks ideas like
   this should be taken out and shot, but it works. */
   
void DiagonalWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, int theWindowHeight, int theWindowWidth)
{
	int			counter,maxmax;
	Rect		source;
	RgnHandle	archie;
	
	maxmax=theWindowWidth+BoxSize;
	archie=NewRgn();
	OpenRgn();
		MoveTo(theWindowWidth+BoxSize,theWindowHeight+BoxSize);
		Line(0,BoxSize);
		Line(-maxmax,maxmax);
		Line(-BoxSize,0);
		Line(maxmax,-maxmax);
	CloseRgn(archie);
	source.top=source.left=0;
	source.bottom=theWindowHeight;
	source.right=theWindowWidth;

	/* the only tricky part to this code is the starting param for counter */
	for (counter=2+(theWindowWidth+theWindowHeight)/BoxSize; counter>=0; counter--)
	{
		StartTiming();
		OffsetRgn(archie,0,-BoxSize);
		CopyBits(&(sourceGrafPtr->portBits),&(destGrafPtr->portBits),&source,&source,
			0,archie);
		TimeCorrection(CorrectTime);
	}
	DisposeRgn(archie);
}
