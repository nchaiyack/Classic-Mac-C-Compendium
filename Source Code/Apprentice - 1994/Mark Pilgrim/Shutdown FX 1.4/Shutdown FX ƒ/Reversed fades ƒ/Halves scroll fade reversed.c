/**********************************************************************\

File:		Halves scroll fade reversed.c

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

#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short HalvesScrollFadeReversed(Rect boundsRect, Pattern *thePattern);

/* 2 regions, split down the middle of the screen.  Scroll the screen down in one
   region and up in the other. */
   
pascal short HalvesScrollFadeReversed(Rect boundsRect, Pattern *thePattern)
{
	int			x;
	Rect		topdest, bottomdest;
	Rect		topscrollsource, bottomscrollsource;
	int			cx;
	int			BoxSize;
	
	BoxSize=theWindowHeight/25;
	cx = boundsRect.left + theWindowWidth / 2;
	
	topscrollsource=topdest=bottomscrollsource=bottomdest=boundsRect;
	topscrollsource.left=topdest.left=bottomscrollsource.right=bottomdest.right=cx;
	topdest.bottom=topdest.top+BoxSize;
	bottomdest.top=bottomdest.bottom-BoxSize;
	
	for(x = theWindowHeight - BoxSize; x >= 0; x -= BoxSize)
	{
		StartTiming();
		ScrollRect(&topscrollsource, 0, BoxSize, 0L);
		FillRect(&topdest, *thePattern);
		
		ScrollRect(&bottomscrollsource, 0, -BoxSize, 0L);
		FillRect(&bottomdest, *thePattern);
		
		TimeCorrection(CorrectTime);
	}
	
	FillRect(&boundsRect, *thePattern);
	
	return 0;
}
