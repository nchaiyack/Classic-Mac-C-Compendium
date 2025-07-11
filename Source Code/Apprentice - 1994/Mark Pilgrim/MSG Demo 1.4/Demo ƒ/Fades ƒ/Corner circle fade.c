/**********************************************************************\

File:		Corner circle fade.c

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

pascal short CornerCircleFade(Rect boundsRect, Pattern *thePattern);

/* Make progressively larger circle regions, centered at the top-left corner
   of the screen.  Quickdraw takes care of all the excess space off the screen
   that you include in the region. */
   
pascal short CornerCircleFade(Rect boundsRect, Pattern *thePattern)
{
	Rect			theRect;
	int				cx, cy;
	RgnHandle		curregion, boundsRgn, sectrgn;
	Point			zeropoint;
	
	zeropoint.v=boundsRect.bottom;
	zeropoint.h=boundsRect.right;
	
	SetRect(&theRect, boundsRect.left-gap, boundsRect.top-gap, boundsRect.left+gap,
		boundsRect.top+gap);
	boundsRgn=NewRgn();
	RectRgn(boundsRgn, &boundsRect);
	sectrgn=NewRgn();
	curregion=NewRgn();
	do
	{
		StartTiming();
		SetEmptyRgn(curregion);
		OpenRgn();
			FrameOval(&theRect);
		CloseRgn(curregion);
		SectRgn(curregion, boundsRgn, sectrgn);
		FillRgn(sectrgn, *thePattern);
		theRect.left-=gap;
		theRect.right+=gap;
		theRect.top-=gap;
		theRect.bottom+=gap;
		TimeCorrection(CorrectTime);
	}
	while (!(PtInRgn(zeropoint,curregion)));
	
	DisposeRgn(curregion);
	DisposeRgn(boundsRgn);
	DisposeRgn(sectrgn);
	
	return 0;
}
