/**********************************************************************\

File:		Circle in fade.c

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

#define	gap			4		/* difference between one radius and the next */
#define CorrectTime 2
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CircleInFade(Rect boundsRect, Pattern *thePattern);

/* Take a really big circle, then a slightly smaller circle (-gap), then
   take the region in between them and copy that, then decrease the outer
   circle radius by gap.  Thus: circle in, by successive donut-like copies. */

pascal short CircleInFade(Rect boundsRect, Pattern *thePattern)
{
	Rect			theRect;
	int				cx, cy;
	RgnHandle		curregion,lastregion,diffregion, boundsRgn, sectrgn;
	Point			zeroPoint;
	int				StartRadius;
	
	zeroPoint.h=boundsRect.left;
	zeroPoint.v=boundsRect.top;

	cx = theWindowWidth / 2;
	cy = theWindowHeight / 2;
	
	boundsRgn=NewRgn();
	SetRectRgn(boundsRgn, boundsRect.left, boundsRect.top, boundsRect.right, boundsRect.bottom);
	sectrgn=NewRgn();
	lastregion=NewRgn();
	StartRadius=0;
	do
	{
		StartRadius+=2*gap;
		theRect.left=cx-StartRadius;     /* circumscribing rectangle for outer circle */
		theRect.right=cx+StartRadius;
		theRect.top=cy-StartRadius;
		theRect.bottom=cy+StartRadius;
		OffsetRect(&theRect, boundsRect.left, boundsRect.top);
		SetEmptyRgn(lastregion);
		OpenRgn();
			FrameOval(&theRect);        /* first circle */
		CloseRgn(lastregion);
	}
	while (!PtInRgn(zeroPoint, lastregion));
	
	curregion=NewRgn();
	diffregion=NewRgn();

	while (theRect.right-theRect.left>0)
	{
		StartTiming();
		theRect.left+=gap;
		theRect.right-=gap;
		theRect.top+=gap;
		theRect.bottom-=gap;
		SetEmptyRgn(curregion);
		OpenRgn();
			FrameOval(&theRect);   /* inner circle */
		CloseRgn(curregion);
		DiffRgn(lastregion,curregion,diffregion);   /* donut we need */
		SectRgn(diffregion, boundsRgn, sectrgn);
		FillRgn(sectrgn, *thePattern);
		CopyRgn(curregion,lastregion);    /* outer circle = inner circle */
		TimeCorrection(CorrectTime);
	}
	
	DisposeRgn(curregion);
	DisposeRgn(lastregion);
	DisposeRgn(diffregion);
	DisposeRgn(boundsRgn);
	DisposeRgn(sectrgn);
	
	return 0;
}
