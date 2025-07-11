/**********************************************************************\

File:		Scissors fade reversed.c

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

#define CorrectTime 3
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short ScissorsFadeReversed(Rect boundsRect, Pattern *thePattern);

/* A region in two parts, starting at the middle of the bottom side and working
   toward the bottom corners, then up the left and right sides. */
   
pascal short ScissorsFadeReversed(Rect boundsRect, Pattern *thePattern)
{
	RgnHandle	curregion, boundsRgn, sectrgn;
	int			cx,gap,lastx,lasty;
	int			BlockSize;
	
	BlockSize=theWindowHeight/25;
	cx = theWindowWidth / 2;

	boundsRgn=NewRgn();
	RectRgn(boundsRgn, &boundsRect);
	sectrgn=NewRgn();
	curregion=NewRgn();
	lastx=theWindowWidth/2;
	gap=theWindowWidth/2+BlockSize;
	do
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,0);
		OpenRgn();
			LineTo(lastx,theWindowHeight);		/* get the right half on bottom side */
			LineTo(gap,theWindowHeight);
			LineTo(cx,0);
			LineTo(theWindowWidth-lastx,theWindowHeight);	/* left 1/2 on bottom */
			LineTo(theWindowWidth-gap,theWindowHeight);
			LineTo(cx,0);
		CloseRgn(curregion);
		OffsetRgn(curregion, boundsRect.left, boundsRect.top);
		SectRgn(curregion, boundsRgn, sectrgn);
		FillRgn(sectrgn, *thePattern);
		lastx=gap;
		gap+=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (gap<=theWindowWidth+BlockSize);
	
	gap=theWindowHeight-BlockSize;
	lasty=theWindowHeight;
	do
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,0);
		OpenRgn();
			LineTo(theWindowWidth,lasty);   /* get the right half */
			LineTo(theWindowWidth,gap);
			LineTo(cx,0);
			LineTo(0,lasty);                   /* get the left half */
			LineTo(0,gap);
			LineTo(cx,0);
		CloseRgn(curregion);
		OffsetRgn(curregion, boundsRect.left, boundsRect.top);
		SectRgn(curregion, boundsRgn, sectrgn);
		FillRgn(sectrgn, *thePattern);
		lasty=gap;
		gap-=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (gap>=0);
	
	FillRect(&boundsRect, *thePattern);			/* in case we missed any bits */
	
	DisposeRgn(curregion);
	DisposeRgn(boundsRgn);
	DisposeRgn(sectrgn);
	
	return 0;
}
