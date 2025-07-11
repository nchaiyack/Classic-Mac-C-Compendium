/**********************************************************************\

File:		Split scroll U-D fade.c

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

#define CorrectTime 4
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short SplitScrollUDFade(Rect boundsRect, Pattern *thePattern);

/* Draw a line from the topleft to the bottomright of the screen and split the
   screen into two regions.  The top region scrolls down and the bottom region
   scrolls up. */
   
pascal short SplitScrollUDFade(Rect boundsRect, Pattern *thePattern)
{
	int			x;
	Rect		topdest, bottomdest;
	Rect		topscrollsource, topscrolldest, bottomscrollsource, bottomscrolldest;
	RgnHandle	toprgn,bottomrgn;
	RgnHandle	temprgn, temp2rgn;
	RgnHandle	topsectrgn, bottomsectrgn;
	int			BoxSize;
	GrafPtr		destGrafPtr;
	
	GetPort(&destGrafPtr);
	
	temprgn=NewRgn();
	temp2rgn=NewRgn();
	topsectrgn=NewRgn();
	bottomsectrgn=NewRgn();
	
	BoxSize=theWindowHeight/25;
	toprgn=NewRgn();
	SetEmptyRgn(toprgn);
	MoveTo(0,0);
	OpenRgn();
		Line(theWindowWidth,0);
		Line(0,theWindowHeight);
		LineTo(0,0);
	CloseRgn(toprgn);
	OffsetRgn(toprgn, boundsRect.left, boundsRect.top);
	
	bottomrgn=NewRgn();
	SetEmptyRgn(bottomrgn);
	MoveTo(0,0);
	OpenRgn();
		Line(0,theWindowHeight);
		Line(theWindowWidth,0);
		LineTo(0,0);
	CloseRgn(bottomrgn);
	OffsetRgn(bottomrgn, boundsRect.left, boundsRect.top);
	
	topscrollsource=boundsRect;
	topscrollsource.bottom-=BoxSize;
	topscrolldest = topscrollsource;
	OffsetRect(&topscrolldest, 0, BoxSize);
	topdest = boundsRect;
	topdest.bottom=topdest.top+BoxSize;
	
	bottomscrollsource=boundsRect;
	bottomscrollsource.top+=BoxSize;
	bottomscrolldest=bottomscrollsource;
	OffsetRect(&bottomscrolldest, 0, -BoxSize);
	bottomdest=boundsRect;
	bottomdest.top=bottomdest.bottom-BoxSize;
	
	RectRgn(temprgn, &topdest);
	SectRgn(toprgn, temprgn, topsectrgn);
	
	RectRgn(temp2rgn, &bottomdest);
	SectRgn(bottomrgn, temp2rgn, bottomsectrgn);
	
	for(x = theWindowHeight; x >= 0; x -= BoxSize)
	{
		StartTiming();
		CopyBits(&(destGrafPtr->portBits), &(destGrafPtr->portBits),
				&topscrollsource, &topscrolldest, 0, toprgn);		
		CopyBits(&(destGrafPtr->portBits), &(destGrafPtr->portBits),
				&bottomscrollsource, &bottomscrolldest, 0, bottomrgn);
		
		FillRgn(bottomsectrgn, *thePattern);
		FillRgn(topsectrgn, *thePattern);
		
		TimeCorrection(CorrectTime);
	}
	
	DisposeRgn(toprgn);
	DisposeRgn(bottomrgn);
	DisposeRgn(temprgn);
	DisposeRgn(temp2rgn);
	DisposeRgn(topsectrgn);
	DisposeRgn(bottomsectrgn);
	
	return 0;
}
