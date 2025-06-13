/**********************************************************************\

File:		Circle serendipity fade.c

Purpose:	Graphic effect to fade main screen to solid pattern.
			See comments below for more description.

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

#define CorrectTime 2
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CircleSerendipityFade(Rect boundsRect, Pattern *thePattern);

/* I haven't the slightest idea why this works.  Hence the name.  -MP */

pascal short CircleSerendipityFade(Rect boundsRect, Pattern *thePattern)
{
	Rect			theRect;
	RgnHandle		curregion, boundsRgn, sectrgn;
	Point			zeropoint;
	int				gap;
	
	gap=theWindowHeight/40;
	SetRect(&theRect, boundsRect.left, boundsRect.top-2*theWindowHeight, boundsRect.right,
		boundsRect.bottom-theWindowHeight);
		
	boundsRgn=NewRgn();
	SetRectRgn(boundsRgn, boundsRect.left, boundsRect.top, boundsRect.right, boundsRect.bottom);
	sectrgn=NewRgn();
	curregion=NewRgn();
	SetEmptyRgn(curregion);
	OpenRgn();
		FrameOval(&theRect);
	CloseRgn(curregion);
	zeropoint.v=boundsRect.bottom;
	zeropoint.h=boundsRect.left;
	do
	{
		StartTiming();
		SectRgn(curregion, boundsRgn, sectrgn);
		FillRgn(sectrgn, *thePattern);
		OffsetRgn(curregion, 0, gap);
		TimeCorrection(CorrectTime);
	}
	while (!(PtInRgn(zeropoint, curregion)));

	FillRect(&boundsRect, *thePattern);
	
	DisposeRgn(curregion);
	DisposeRgn(boundsRgn);
	DisposeRgn(sectrgn);
	
	return 0;
}
