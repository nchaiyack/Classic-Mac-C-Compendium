/**********************************************************************\

File:		Four-corner scroll.c

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

#define CorrectTime 4
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short FourCornerScroll(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Four scrolls in one, each confined to a section of the screen which includes
   the center and one side, and all four scrolls moving towards the center.
   Kinda slow on my Mac IIci in 8-bit; fine in 4-bit or fewer. */
   
pascal short FourCornerScroll(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	int			x;
	Rect		theTopRect, topdest, theBottomRect, bottomdest;
	Rect		theLeftRect, leftdest, theRightRect, rightdest;
	Rect		topscrollsource, topscrolldest, bottomscrollsource, bottomscrolldest;
	Rect		leftscrollsource, leftscrolldest, rightscrollsource, rightscrolldest;
	RgnHandle	toprgn,bottomrgn,leftrgn,rightrgn;
	int			cx,cy;
	int			HBoxSize,BoxSize;
	
	BoxSize=theWindowHeight/50;
	HBoxSize=theWindowWidth/50;
	
	cx = boundsRect.left + theWindowWidth / 2;
	cy = boundsRect.top + theWindowHeight / 2;

	toprgn=NewRgn();
	SetEmptyRgn(toprgn);
	MoveTo(boundsRect.left, boundsRect.top);
	OpenRgn();                               /* top region */
		Line(theWindowWidth,0);
		LineTo(cx,cy);
		LineTo(boundsRect.left, boundsRect.top);
	CloseRgn(toprgn);
	
	bottomrgn=NewRgn();
	SetEmptyRgn(bottomrgn);
	MoveTo(boundsRect.left, boundsRect.bottom);
	OpenRgn();                               /* bottom region */
		Line(theWindowWidth, 0);
		LineTo(cx,cy);
		LineTo(boundsRect.left, boundsRect.bottom);
	CloseRgn(bottomrgn);
	
	leftrgn=NewRgn();
	SetEmptyRgn(leftrgn);
	MoveTo(boundsRect.left, boundsRect.top);
	OpenRgn();                               /* left region */
		Line(0,theWindowHeight);
		LineTo(cx,cy);
		LineTo(boundsRect.left, boundsRect.top);
	CloseRgn(leftrgn);
	
	rightrgn=NewRgn();
	SetEmptyRgn(rightrgn);
	MoveTo(boundsRect.right, boundsRect.top);
	OpenRgn();                               /* right region */
		Line(0, theWindowHeight);
		LineTo(cx,cy);
		LineTo(boundsRect.right, boundsRect.top);
	CloseRgn(rightrgn);
	
	topscrollsource=bottomscrollsource=leftscrollsource=rightscrollsource=
		topdest=bottomdest=leftdest=rightdest=boundsRect;
	
	topscrollsource.bottom-=cy-boundsRect.top+BoxSize;
	topscrolldest = topscrollsource;
	OffsetRect(&topscrolldest, 0, BoxSize);		/* dest. strip for top scroll */
	topdest.bottom=topdest.top+BoxSize;			/* dest. strip for new data on top */
	SetRect(&theTopRect, boundsRect.left, cy-BoxSize, boundsRect.right, cy);
	
	bottomscrollsource.top+=cy-boundsRect.top+BoxSize;
	bottomscrolldest=bottomscrollsource;
	OffsetRect(&bottomscrolldest, 0, -BoxSize);
	bottomdest.top=bottomdest.bottom-BoxSize;
	SetRect(&theBottomRect, boundsRect.left, cy, boundsRect.right, cy+BoxSize);
	
	leftscrollsource.right-=cx-boundsRect.left+HBoxSize;
	leftscrolldest=leftscrollsource;
	OffsetRect(&leftscrolldest,HBoxSize,0);
	leftdest.right=leftdest.left+HBoxSize;
	SetRect(&theLeftRect, cx-HBoxSize, boundsRect.top, cx, boundsRect.bottom);
	
	rightscrollsource.left+=cx-boundsRect.left+HBoxSize;
	rightscrolldest=rightscrollsource;
	OffsetRect(&rightscrolldest,-HBoxSize,0);
	rightdest.left=rightdest.right-HBoxSize;
	SetRect(&theRightRect, cx, boundsRect.top, cx+HBoxSize, boundsRect.bottom);
	
	/* for each section (top, bottom, left, right) we need to do the scroll part,
	which takes a section of the dest. window and moves it, then we need to copy
	in the new data from the source window */
	for(x = cy - BoxSize; x >= boundsRect.top; x -= BoxSize)
	{
		StartTiming();
		CopyBits(&(destGrafPtr->portBits), &(destGrafPtr->portBits),
				&topscrollsource, &topscrolldest, 0, toprgn);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theTopRect, &topdest, 0, toprgn);
		theTopRect.bottom-=BoxSize;
		theTopRect.top-=BoxSize;
		
		CopyBits(&(destGrafPtr->portBits), &(destGrafPtr->portBits),
				&bottomscrollsource, &bottomscrolldest, 0, bottomrgn);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theBottomRect, &bottomdest, 0, bottomrgn);
		theBottomRect.top+=BoxSize;
		theBottomRect.bottom+=BoxSize;
		
		CopyBits(&(destGrafPtr->portBits), &(destGrafPtr->portBits),
				&leftscrollsource, &leftscrolldest, 0, leftrgn);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theLeftRect, &leftdest, 0, leftrgn);
		theLeftRect.left-=HBoxSize;
		theLeftRect.right-=HBoxSize;
		
		CopyBits(&(destGrafPtr->portBits), &(destGrafPtr->portBits),
				&rightscrollsource, &rightscrolldest, 0, rightrgn);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theRightRect, &rightdest, 0, rightrgn);
		theRightRect.left+=HBoxSize;
		theRightRect.right+=HBoxSize;
		TimeCorrection(CorrectTime);
	}
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);    /* in case we missed any bits */

	DisposeRgn(toprgn);
	DisposeRgn(bottomrgn);
	DisposeRgn(leftrgn);
	DisposeRgn(rightrgn);
	
	return 0;
}
