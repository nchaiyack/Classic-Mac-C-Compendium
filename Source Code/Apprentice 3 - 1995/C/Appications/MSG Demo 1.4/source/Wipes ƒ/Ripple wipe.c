/**********************************************************************\

File:		Ripple wipe.c

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

pascal short RippleWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Like the Caste wipe up, except the screen is split into 6 sections, and the
   caste wipe is performed in each section -- hence the ripple effect. */
   
pascal short RippleWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	int				bigy, littley, barpos;
	Rect			src, dest;
	int				BigRippleSize;
	int				gap;
	RgnHandle		boundsRgn;
	
	BigRippleSize=theWindowHeight/6;	/* used to be 52 */
	gap=BigRippleSize/6;				/* used to be 8 */
	boundsRgn=NewRgn();
	RectRgn(boundsRgn, &boundsRect);
	src.left = boundsRect.left;
	src.right = boundsRect.right;
	
	for(bigy = 0; bigy < theWindowHeight; bigy += BigRippleSize)
	{
		for(littley = bigy; littley < bigy + BigRippleSize; littley += gap)
		{
			for(barpos = bigy; barpos + gap < bigy + BigRippleSize; barpos += gap);
			for(; barpos >= littley; barpos -= gap)
			{
				StartTiming();
				src.top = boundsRect.top + littley;
				src.bottom = boundsRect.top + littley + gap;
				dest = src;
				dest.top = boundsRect.top + barpos;
				dest.bottom = dest.top + gap;
				CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
						&src, &dest, 0, boundsRgn);
				TimeCorrection(CorrectTime);
			}
		}
	}
	
	DisposeRgn(boundsRgn);
	
	return 0;
}
