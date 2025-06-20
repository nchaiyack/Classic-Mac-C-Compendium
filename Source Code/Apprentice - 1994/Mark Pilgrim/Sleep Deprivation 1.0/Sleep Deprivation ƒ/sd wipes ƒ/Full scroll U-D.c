/**********************************************************************\

File:		Full scroll U-D.c

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

#define		BoxSize		10
#define CorrectTime 3

void FullScrollUD(GrafPtr, Pattern*);

/* Take the full screen minus the bottom strip, move it into the whole screen
   shifted down by BoxSize, then take the bottommost source strip and put it
   in the top strip of the dest. window. */
   
void FullScrollUD(GrafPtr thePtr, Pattern *thePattern)
{
	int			x, y;
	Rect		theRect, dest;
	Rect		scrollsource, scrolldest;
	int			width,height;
	
	width=thePtr->portRect.right-thePtr->portRect.left;
	height=thePtr->portRect.bottom-thePtr->portRect.top;
		
	scrollsource=thePtr->portRect;
	scrollsource.bottom-=BoxSize;              /* whole screen minus bottom strip */
	scrolldest = scrollsource;
	OffsetRect(&scrolldest, 0, BoxSize);       /* whole screen shifted down BoxSize */
	
	dest = thePtr->portRect;
	dest.bottom=BoxSize;                       /* top strip for new data */
	
	for(x = height - BoxSize; x >= 0; x -= BoxSize)
	{
		StartTiming();
		CopyBits(&(thePtr->portBits), &(thePtr->portBits),
				&scrollsource, &scrolldest, 0, 0L);
		FillRect(&dest, *thePattern);
		
		TimeCorrection(CorrectTime);
	}
}