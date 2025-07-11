/**********************************************************************\

File:		Circle out fade.c

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

pascal short CircleOutFade(Rect boundsRect, Pattern *thePattern);

/* Make a circular region and use it as the mask in CopyBits.  Lots of overlap,
   but masked by timing correction.  If you want to optimize it, look at the
   donut region code in "Circle in.c" */
   
pascal short CircleOutFade(Rect boundsRect, Pattern *thePattern)
{
	Rect			theRect;
	int				cx, cy;
	RgnHandle		curregion, boundsRgn, sectrgn;
	Point			zeroPoint;
	
	zeroPoint.h=boundsRect.left;
	zeroPoint.v=boundsRect.top;
	
	cx = theWindowWidth / 2;
	cy = theWindowHeight / 2;
	
	theRect.left=cx-gap;         /* circumscribing rectangle for circle */
	theRect.right=cx+gap;
	theRect.top=cy-gap;
	theRect.bottom=cy+gap;
	OffsetRect(&theRect, boundsRect.left, boundsRect.top);
	
	boundsRgn=NewRgn();
	SetRectRgn(boundsRgn, boundsRect.left, boundsRect.top, boundsRect.right, boundsRect.bottom);
	sectrgn=NewRgn();
	curregion=NewRgn();
	
	do
	{
		StartTiming();
		SetEmptyRgn(curregion);
		OpenRgn();
			FrameOval(&theRect);   /* the circle region */
		CloseRgn(curregion);
		SectRgn(curregion, boundsRgn, sectrgn);
		FillRgn(sectrgn, *thePattern);
		theRect.left-=gap;        /* make the rect bigger (and thus the circle) */
		theRect.right+=gap;
		theRect.top-=gap;
		theRect.bottom+=gap;
		TimeCorrection(CorrectTime);
	}
	while (!PtInRgn(zeroPoint, curregion));
	
	DisposeRgn(curregion);
	DisposeRgn(boundsRgn);
	DisposeRgn(sectrgn);
	
	return 0;
}
