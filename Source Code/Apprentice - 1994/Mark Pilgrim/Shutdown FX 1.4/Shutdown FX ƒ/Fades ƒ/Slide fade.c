/**********************************************************************\

File:		Slide fade.c

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

#define CorrectTime 1
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short SlideFade(Rect boundsRect, Pattern *thePattern);

/* Split the screen into 10 sections, then scroll each section left or right
   (alternating), starting with the top section and working towards the bottom. */
   
pascal short SlideFade(Rect boundsRect, Pattern *thePattern)
{
	int				x, y;
	Rect			dest;
	Rect			scrollsource;
	int				direction;
	int				BoxSize, StripSize;
	
	BoxSize=theWindowWidth/25;
	StripSize=theWindowHeight/10;
	
	direction = 0;
	for(y = 0; y < theWindowHeight; y += StripSize)
	{
		scrollsource = boundsRect;
		scrollsource.top+=y;
		scrollsource.bottom = scrollsource.top + StripSize;
		
		dest = scrollsource;
		if(direction == 0)
		{
			dest.right=dest.left+BoxSize;
		
			for(x = boundsRect.right - BoxSize; x >= boundsRect.left; x -= BoxSize)
			{
				StartTiming();
				ScrollRect(&scrollsource, BoxSize, 0, 0L);
				SectRect(&dest, &boundsRect, &dest);
				FillRect(&dest, *thePattern);
				TimeCorrection(CorrectTime);
			}
			FillRect(&scrollsource, *thePattern);
		}
		else
		{
			dest.left = dest.right - BoxSize;
			
			for(x = boundsRect.left+BoxSize; x <= boundsRect.right; x += BoxSize)
			{
				StartTiming();
				ScrollRect(&scrollsource, -BoxSize, 0, 0L);
				SectRect(&dest, &boundsRect, &dest);
				FillRect(&dest, *thePattern);
				TimeCorrection(CorrectTime);
			}
			FillRect(&scrollsource, *thePattern);
		}
		
		direction = 1 - direction;
	}
	
	return 0;
}
