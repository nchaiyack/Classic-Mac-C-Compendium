/**********************************************************************\

File:		Pour scroll reversed.c

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

#define CorrectTime 2
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short PourScrollReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Scroll down in tiny strips, starting at the right and moving left.  Scroll
   strips 1-(N), then strips 1-(N+1), etc.  When strip 1 is done, don't do it
   anymore!  (Duh, but this was difficult to get right.)  So only scroll strips
   2-(N), and so on. */
   
pascal short PourScrollReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect			*theRect, *dest;
	Rect			scrollsource;
	int				i;
	int				startstrip,endstrip;
	int				ScrollSize, BoxSize;
	int				NumStrips;
	
	ScrollSize=theWindowHeight/50;
	NumStrips=theWindowWidth/4;
	BoxSize=theWindowWidth/NumStrips;
	dest=(Rect*)NewPtr(sizeof(Rect)*NumStrips);
	if (dest==0L)
		return -1;		/* memory error */
	theRect=(Rect*)NewPtr(sizeof(Rect)*NumStrips);
	if (theRect==0L)
	{
		DisposePtr(dest);
		return -1;		/* memory error */
	}
	
	SetRect(&scrollsource, boundsRect.right-BoxSize, boundsRect.top,
		boundsRect.right, boundsRect.bottom);
		
	for (i=0; i<NumStrips; i++)
	{
		dest[i].top = theWindowHeight-ScrollSize;
		dest[i].bottom=theWindowHeight;
		dest[i].left=theWindowWidth-(i+1)*BoxSize;
		dest[i].right=dest[i].left+BoxSize;
		OffsetRect(&(dest[i]), boundsRect.left, boundsRect.top);
		
		theRect[i].top=0;
		theRect[i].bottom=ScrollSize;
		theRect[i].left=theWindowWidth-(i+1)*BoxSize;
		theRect[i].right=theRect[i].left+BoxSize;
		OffsetRect(&(theRect[i]), boundsRect.left, boundsRect.top);
	}
	
	startstrip=0;
	endstrip=1;
	do
	{
		StartTiming();
		
		ScrollTheRect(&scrollsource, 0, -ScrollSize, 0L);
		
		for (i=startstrip; i<endstrip; i++)
		{
			CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
					&theRect[i], &dest[i], 0, 0L);
			theRect[i].bottom+=ScrollSize;
			theRect[i].top+=ScrollSize;
		}
		
		if (endstrip<NumStrips)
		{
			endstrip++;
			scrollsource.left-=BoxSize;
		}
		if (theRect[startstrip].top>=theWindowHeight)
		{
			startstrip++;
			scrollsource.right-=BoxSize;
		}
		
		TimeCorrection(CorrectTime);
	}
	while (startstrip<endstrip);
	
	DisposePtr(dest);
	DisposePtr(theRect);
	
	return 0;
}
