/**********************************************************************\

File:		Caste wipe.c

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

pascal short CasteWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* This takes a strip (starting with the topmost strip, moving down) and copies
   it into all the strips starting with the bottom and moving up until it's in
   the right place. */
   
pascal short CasteWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	int				srcy, barpos;
	Rect			src, dest;
	Boolean			everyOther;
	int				gap;
	
	gap=theWindowHeight/50;
	everyOther=FALSE;
	src.left = boundsRect.left;
	src.right = boundsRect.right;
	
	for(srcy = boundsRect.top; srcy < boundsRect.bottom; srcy += gap)
	{
		for(barpos = boundsRect.top; barpos + gap < boundsRect.bottom; barpos += gap);
		for(; barpos >= srcy; barpos -= gap)
		{
			StartTiming();
			src.top = srcy;
			src.bottom = srcy + gap;
			dest = src;
			dest.top = barpos;
			dest.bottom = barpos + gap;
			CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
					&src, &dest, 0, 0L);
			if (everyOther)                      /* simulates time correction 0.5 */
				TimeCorrection(CorrectTime);
			everyOther=!everyOther;
		}
	}
	
	return 0;
}
