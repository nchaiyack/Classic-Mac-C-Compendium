/**********************************************************************\

File:		Mr. Do reversed.c

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

#define		BoxSize	4
#define CorrectTime 1
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short MrDoReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* 25 regions, in a 5 x 5 grid.  Go around to each region in a spiral pattern,
   starting with the center square, and alternatively scroll it up or down. */
   
pascal short MrDoReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	int			x, y;
	int			vgap,hgap;
	Rect		theRect, dest;
	Rect		bounds[25];
	Boolean		everyOther;
	
	vgap=theWindowHeight/5;
	hgap=theWindowWidth/5;
	
	for (x=0; x<25; x++)
	{
		switch (x)
		{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
				bounds[x].top=0;
				break;
			case 15:
			case 16:
			case 17:
			case 18:
			case 5:
				bounds[x].top=vgap;
				break;
			case 14:
			case 23:
			case 24:
			case 19:
			case 6:
				bounds[x].top=vgap*2;
				break;
			case 13:
			case 22:
			case 21:
			case 20:
			case 7:
				bounds[x].top=vgap*3;
				break;
			case 12:
			case 11:
			case 10:
			case 9:
			case 8:
				bounds[x].top=vgap*4;
				break;
		}
		switch (x)
		{
			case 0:
			case 15:
			case 14:
			case 13:
			case 12:
				bounds[x].left=0;
				break;
			case 1:
			case 16:
			case 23:
			case 22:
			case 11:
				bounds[x].left=hgap;
				break;
			case 2:
			case 17:
			case 24:
			case 21:
			case 10:
				bounds[x].left=hgap*2;
				break;
			case 3:
			case 18:
			case 19:
			case 20:
			case 9:
				bounds[x].left=hgap*3;
				break;
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				bounds[x].left=hgap*4;
				break;
		}
		bounds[x].bottom=bounds[x].top+vgap;
		bounds[x].right=bounds[x].left+hgap;
		OffsetRect(&(bounds[x]), boundsRect.left, boundsRect.top);
	}
	
	for (y=24; y>=0; y--)
	{		
		if (!(y%2))   /* these scroll up */
		{
			dest=bounds[y];
			dest.top=dest.bottom-BoxSize;
			
			theRect=bounds[y];
			theRect.bottom=theRect.top+BoxSize;
			
			for (x=bounds[y].bottom-bounds[y].top-BoxSize; x>0; x-=BoxSize)
			{
				StartTiming();
				ScrollTheRect(&bounds[y], 0, -BoxSize, 0L);
				CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
						&theRect, &dest, 0, 0L);
				theRect.bottom+=BoxSize;
				theRect.top+=BoxSize;
				if (everyOther)
					TimeCorrection(CorrectTime);
				everyOther=!everyOther;
			}
		}
		else    /* these scroll down */
		{
			dest=bounds[y];
			dest.bottom=dest.top+BoxSize;
			
			theRect=bounds[y];
			theRect.top=theRect.bottom-BoxSize;
			
			for(x = bounds[y].bottom-bounds[y].top-BoxSize; x > 0; x -= BoxSize)
			{
				StartTiming();
				ScrollTheRect(&bounds[y], 0, BoxSize, 0L);
				CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
						&theRect, &dest, 0, 0L);
				theRect.bottom-=BoxSize;
				theRect.top-=BoxSize;
				if (everyOther)
					TimeCorrection(CorrectTime);
				everyOther=!everyOther;
			}
		}
		
		CopyBits(&(sourceGrafPtr->portBits),&(destGrafPtr->portBits),
					&bounds[y],&bounds[y],0,0L);
	}
	
	return 0;
}
