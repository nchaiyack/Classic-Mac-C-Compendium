/**********************************************************************\

File:		SlideWipe reversed.c

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

#define CorrectTime 1
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short SlideWipeReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Split the screen into 10 sections, then alternatively scroll left and right,
   starting at the bottom section and working toward the top. */
   
pascal short SlideWipeReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	int				x, y;
	Rect			theRect, dest;
	Rect			scrollsource;
	int				direction;
	int				BoxSize, StripSize;
	
	BoxSize=theWindowWidth/25;
	StripSize=theWindowHeight/10;
	
	direction = 0;
	for(y = theWindowHeight-StripSize; y >= 0; y -= StripSize)
	{
		scrollsource = boundsRect;
		scrollsource.top+=y;
		scrollsource.bottom = scrollsource.top + StripSize;
		
		dest = scrollsource;
		
		theRect.top = boundsRect.top + y;
		theRect.bottom = theRect.top + StripSize;
		
		if(direction == 0)
		{
			dest.right = dest.left + BoxSize;
			
			theRect.left = boundsRect.right-BoxSize;
			theRect.right = boundsRect.right;
			
			for(x = boundsRect.right - BoxSize; x >= boundsRect.left; x -= BoxSize)
			{
				StartTiming();
				theRect.left = x;
				ScrollTheRect(&scrollsource, BoxSize, 0, 0L);
				CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
						&theRect, &dest, 0, 0L);
				theRect.right = x;
				TimeCorrection(CorrectTime);
			}
			CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
					&scrollsource, &scrollsource, 0, 0L);
		}
		else
		{
			dest.left = dest.right - BoxSize;
			
			theRect.left = boundsRect.left;
			theRect.right = theRect.left + BoxSize;
			
			for(x = boundsRect.left+BoxSize; x <= boundsRect.right; x += BoxSize)
			{
				StartTiming();
				theRect.right = x;
				ScrollTheRect(&scrollsource, -BoxSize, 0, 0L);
				CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
						&theRect, &dest, 0, 0L);
				theRect.left = x;
				TimeCorrection(CorrectTime);
			}
			CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
					&scrollsource, &scrollsource, 0, 0L);
		}
		
		direction = 1 - direction;
	}
	
	return 0;
}
