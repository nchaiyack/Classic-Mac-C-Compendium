/**********************************************************************\

File:		Pour scroll fade reversed.c

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

#define CorrectTime 2
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short PourScrollFadeReversed(Rect boundsRect, Pattern *thePattern);

/* Scroll down in tiny strips, starting at the right and moving left.  Scroll
   strips 1-(N), then strips 1-(N+1), etc.  When strip 1 is done, don't do it
   anymore!  (Duh, but this was difficult to get right.)  So only scroll strips
   2-(N), and so on. */
   
pascal short PourScrollFadeReversed(Rect boundsRect, Pattern *thePattern)
{
	Rect			*dest;
	int				*iter;
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
	iter=(int*)NewPtr(sizeof(int)*NumStrips);
	if (iter==0L)
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
		
		iter[i]=0;
	}
	
	startstrip=0;
	endstrip=1;
	do
	{
		StartTiming();
		
		ScrollRect(&scrollsource, 0, -ScrollSize, 0L);
		
		for (i=startstrip; i<endstrip; i++)
		{
			FillRect(&dest[i], *thePattern);
			iter[i]+=ScrollSize;
		}
		
		if (endstrip<NumStrips)
		{
			endstrip++;
			scrollsource.left-=BoxSize;
		}
		if (iter[startstrip]>=theWindowHeight)
		{
			startstrip++;
			scrollsource.right-=BoxSize;
		}
		
		TimeCorrection(CorrectTime);
	}
	while (startstrip<endstrip);
	
	DisposePtr(dest);
	DisposePtr(iter);
	
	return 0;
}
