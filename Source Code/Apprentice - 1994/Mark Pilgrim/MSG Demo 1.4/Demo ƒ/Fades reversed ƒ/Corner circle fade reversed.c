/**********************************************************************\

File:		Corner circle fade reversed.c

Purpose:	Graphic effect to fade main window to solid pattern.
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

#define	gap			8		/* difference between one radius and the next */
#define CorrectTime 2

pascal short CornerCircleFadeReversed(Rect boundsRect, Pattern *thePattern);

/* Make progressively larger circle regions, centered at the top-left corner
   of the screen, until you are covering the bottom-right of the screen.  Copy
   regions that are the difference between the boundsRect and this circle, then
   make the circle smaller.  Quickdraw takes care of all the excess space off
   the screen that you include in the region. */
   
pascal short CornerCircleFadeReversed(Rect boundsRect, Pattern *thePattern)
{
	Rect			theRect;
	RgnHandle		curregion,lastregion,diffregion, boundsRgn, sectrgn;
	Point			zeroPoint;
	int				StartRadius;
	
	zeroPoint.h=boundsRect.right;
	zeroPoint.v=boundsRect.bottom;

	boundsRgn=NewRgn();
	RectRgn(boundsRgn, &boundsRect);
	sectrgn=NewRgn();
	lastregion=NewRgn();
	StartRadius=0;
	do
	{
		StartRadius+=4*gap;
		theRect.left=-StartRadius;     /* circumscribing rectangle for outer circle */
		theRect.right=StartRadius;
		theRect.top=-StartRadius;
		theRect.bottom=StartRadius;
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
