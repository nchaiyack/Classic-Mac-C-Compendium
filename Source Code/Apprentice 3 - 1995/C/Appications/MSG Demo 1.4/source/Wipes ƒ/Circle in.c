/**********************************************************************\

File:		Circle in.c

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

#define	gap			4		/* difference between one radius and the next */
#define CorrectTime 2
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CircleIn(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Take a really big circle, then a slightly smaller circle (-gap), then
   take the region in between them and copy that, then decrease the outer
   circle radius by gap.  Thus: circle in, by successive donut-like copies. */

pascal short CircleIn(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect			theRect;
	int				cx, cy;
	RgnHandle		curregion,lastregion,diffregion;
	Point			zeroPoint;
	int				StartRadius;
	
	zeroPoint.h=boundsRect.left;
	zeroPoint.v=boundsRect.top;

	cx = theWindowWidth / 2;
	cy = theWindowHeight / 2;
	
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
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&boundsRect, &boundsRect, 0, diffregion);
		CopyRgn(curregion,lastregion);    /* outer circle = inner circle */
		TimeCorrection(CorrectTime);
	}
	
	DisposeRgn(curregion);
	DisposeRgn(lastregion);
	DisposeRgn(diffregion);
	
	return 0;
}
