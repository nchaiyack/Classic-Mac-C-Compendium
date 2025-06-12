/**********************************************************************\

File:		Two corner fade reversed.c

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

pascal short TwoCornerFadeReversed(Rect boundsRect, Pattern *thePattern);

/* One region starts at the top of the screen and moves down the left side
   (making triangles with the bottomright of the screen).  The other region starts
   at the bottom of the screen and moves up the right side (making triangles with
   the topleft of the screen). */
   
pascal short TwoCornerFadeReversed(Rect boundsRect, Pattern *thePattern)
{
	RgnHandle		curregion, boundsRgn, sectrgn;
	int				gap;
	int				BlockSize;
	
	BlockSize=theWindowHeight/25;
	boundsRgn=NewRgn();
	SetRectRgn(boundsRgn, boundsRect.left, boundsRect.top, boundsRect.right, boundsRect.bottom);
	sectrgn=NewRgn();
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
		SectRgn(curregion, boundsRgn, sectrgn);
		FillRgn(sectrgn, *thePattern);
		gap-=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (gap>=0);
	
	FillRect(&boundsRect, *thePattern);		/* in case we missed any bits */
	
	DisposeRgn(curregion);
	DisposeRgn(boundsRgn);
	DisposeRgn(sectrgn);
	
	return 0;
}
