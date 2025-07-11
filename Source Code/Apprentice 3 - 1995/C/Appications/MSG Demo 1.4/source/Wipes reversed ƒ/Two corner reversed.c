/**********************************************************************\

File:		Two corner reversed.c

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

#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short TwoCornerReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* One region starts at the top of the screen and moves down the left side
   (making triangles with the bottomright of the screen).  The other region starts
   at the bottom of the screen and moves up the right side (making triangles with
   the topleft of the screen). */
   
pascal short TwoCornerReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	RgnHandle		curregion;
	int				gap;
	int				BlockSize;
	
	BlockSize=theWindowHeight/25;
	curregion=NewRgn();
	gap=theWindowHeight;
	do
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(0,0);
		OpenRgn();
			Line(theWindowWidth,gap);
			Line(0,-BlockSize);
			LineTo(0,0);
			MoveTo(theWindowWidth,theWindowHeight);	/* region is discontinuous */
			Line(-theWindowWidth,-gap);				/* but this is much faster */
			Line(0,BlockSize);						/* than two regions & two  */
			LineTo(theWindowWidth,theWindowHeight);	/* CopyBits */
		CloseRgn(curregion);
		OffsetRgn(curregion, boundsRect.left, boundsRect.top);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&boundsRect, &boundsRect, 0, curregion);
		gap-=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (gap>=0);
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);	/* in case we missed any bits */
	
	DisposeRgn(curregion);
	
	return 0;
}
