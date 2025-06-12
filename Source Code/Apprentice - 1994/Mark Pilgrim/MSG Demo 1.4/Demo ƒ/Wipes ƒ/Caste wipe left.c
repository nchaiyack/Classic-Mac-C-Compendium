/**********************************************************************\

File:		Caste wipe left.c

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
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CasteWipeRL(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* This takes a strip (starting with the leftmost strip, moving right) and
   copies it into all the strips starting at the right and moving left until
   its in the right place. */
   
pascal short CasteWipeRL(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	int				srcx, barpos;
	Rect			src, dest;
	Boolean			everyOther;
	int				gap;
	
	gap=theWindowWidth/50;
	everyOther=FALSE;
	src.top = boundsRect.top;
	src.bottom = boundsRect.bottom;
	
	for(srcx = boundsRect.left; srcx < boundsRect.right; srcx += gap)
	{
		for(barpos = boundsRect.left; barpos + gap < boundsRect.right; barpos += gap);
		for(; barpos >= srcx; barpos -= gap)
		{
			StartTiming();
			src.left = srcx;
			src.right = srcx + gap;
			dest = src;
			dest.left = barpos;
			dest.right = barpos + gap;
			CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
					&src, &dest, 0, 0L);
			if (everyOther)                        /* really, we need time */
				TimeCorrection(CorrectTime);       /* correction 0.5, but  */
			everyOther=!everyOther;                /* this will do (gag)   */
		}
	}
	
	return 0;
}
