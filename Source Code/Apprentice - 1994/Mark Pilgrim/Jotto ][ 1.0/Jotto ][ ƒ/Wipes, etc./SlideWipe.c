/**********************************************************************\

File:		SlideWipe.c

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

#define		BoxSize		20
#define		StripSize	40
#define CorrectTime 1

void SlideWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, int theWindowHeight, int theWindowWidth)
{
	int			x, y;
	Rect		theRect, dest;
	Rect		scrollsource, scrolldest;
	int			direction;
	
	direction = 0;
	
	for(y = 0; y < theWindowHeight; y += StripSize)
	{
		scrollsource = destGrafPtr->portRect;
		scrollsource.top = scrollsource.top + y;
		scrollsource.bottom = scrollsource.top + StripSize;
		
		scrolldest = scrollsource;
		
		dest = destGrafPtr->portRect;
		dest.top = dest.top + y;
		dest.bottom = dest.top + StripSize;
		
		theRect.top = y;
		theRect.bottom = y + StripSize;
		
		if(direction == 0)
		{
			OffsetRect(&scrolldest, BoxSize, 0);
			
			dest.right = dest.left + BoxSize;
			
			theRect.left = theWindowWidth - BoxSize;
			theRect.right = theWindowWidth;
			
			for(x = theWindowWidth - BoxSize; x >= 0; x -= BoxSize)
			{
				StartTiming();
				theRect.left = x;
				CopyBits(&(destGrafPtr->portBits), &(destGrafPtr->portBits),
						&scrollsource, &scrolldest, 0, 0L);
				CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
						&theRect, &dest, 0, 0L);
				theRect.right = x;
				TimeCorrection(CorrectTime);
			}
		}
		else
		{
			OffsetRect(&scrolldest, -BoxSize, 0);
			
			dest.left = dest.right - BoxSize;
			
			theRect.left = 0;
			theRect.right = BoxSize;
			
			for(x = BoxSize; x <= theWindowWidth; x += BoxSize)
			{
				StartTiming();
				theRect.right = x;
				CopyBits(&(destGrafPtr->portBits), &(destGrafPtr->portBits),
						&scrollsource, &scrolldest, 0, 0L);
				CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
						&theRect, &dest, 0, 0L);
				theRect.left = x;
				TimeCorrection(CorrectTime);
			}
		}
		
		direction = 1 - direction;
	}
}