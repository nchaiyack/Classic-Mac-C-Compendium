/**********************************************************************\

File:		Diagonal fade down-right.c

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

#define CorrectTime 2
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short DiagonalFadeDownRight(Rect boundsRect, Pattern *thePattern);

/* If you make a long enough region starting at the top-left corner and
   making a strip up and right of <BoxSize> width, all you have to do is move
   the region down until the entire screen is filled.  Dave thinks ideas like
   this should be taken out and shot, but it works. */
   
pascal short DiagonalFadeDownRight(Rect boundsRect, Pattern *thePattern)
{
	int				maxmax;
	RgnHandle		archie, boundsRgn, sectrgn;
	int				BoxSize;
	Point			zeroPoint;
	
	zeroPoint.h=boundsRect.right;
	zeroPoint.v=boundsRect.bottom;
	
	BoxSize=theWindowWidth/40;
	
	maxmax=(theWindowWidth>theWindowHeight) ? theWindowWidth : theWindowHeight;
	maxmax+=BoxSize;
	boundsRgn=NewRgn();
	RectRgn(boundsRgn, &boundsRect);
	sectrgn=NewRgn();
	archie=NewRgn();
	OpenRgn();
		MoveTo(boundsRect.left-BoxSize, boundsRect.top);
		Line(0,-BoxSize);
		Line(maxmax,-maxmax);
		Line(BoxSize,0);
		Line(-maxmax,maxmax);
	CloseRgn(archie);

	do
	{
		StartTiming();
		OffsetRgn(archie,0,BoxSize);
		SectRgn(archie, boundsRgn, sectrgn);
		FillRgn(sectrgn, *thePattern);
		TimeCorrection(CorrectTime);
	}
	while (!PtInRgn(zeroPoint, archie));
	
	DisposeRgn(archie);
	DisposeRgn(boundsRgn);
	DisposeRgn(sectrgn);
	
	return 0;
}
