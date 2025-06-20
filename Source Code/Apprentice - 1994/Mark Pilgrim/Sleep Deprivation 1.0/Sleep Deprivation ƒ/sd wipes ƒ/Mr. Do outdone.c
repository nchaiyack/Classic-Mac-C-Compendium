/**********************************************************************\

File:		Mr. Do outdone.c

Purpose:	This module handles clearing the screen in a funky
			manner.  See the comments below for more details.
			

Sleep Deprivation -- graphic effects on sleep
Copyright (C) 1993 Mark Pilgrim & Dave Blumenthal

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

#define		BoxSize	2
#define CorrectTime 3

void MrDoOutdone(GrafPtr, Pattern*);

/* 25 regions on screen, in a 5 x 5 grid.  Regions alternate as to whether they
   scroll up or down. */
   
void MrDoOutdone(GrafPtr thePtr, Pattern *thePattern)
{
	int			x, y;
	int			vgap,hgap;
	Rect		theRect, dest;
	Rect		scrollsource, scrolldest;
	Rect		bounds[25];
	int			width,height;
	
	width=thePtr->portRect.right-thePtr->portRect.left;
	height=thePtr->portRect.bottom-thePtr->portRect.top;
		
	vgap=height/5;
	hgap=width/5;
	
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
		bounds[x].left+=thePtr->portRect.left;
		bounds[x].top+=thePtr->portRect.top;
		bounds[x].bottom=bounds[x].top+vgap;
		bounds[x].right=bounds[x].left+hgap;
	}
	
	for (x=BoxSize; x<vgap; x+=BoxSize)
	{		
		StartTiming();
		for (y=0; y<25; y++)
		{
			if (y%2)   /* these scroll up */
			{
				scrollsource=bounds[y];
				scrollsource.top+=BoxSize;
				scrolldest=scrollsource;
				OffsetRect(&scrolldest,0,-BoxSize);
				
				dest=bounds[y];
				dest.top=dest.bottom-BoxSize;
				
				CopyBits(&(thePtr->portBits), &(thePtr->portBits),
						&scrollsource, &scrolldest, 0, 0L);
				FillRect(&dest, *thePattern);
			}
			else    /* these scroll down */
			{
				scrollsource=bounds[y];
				scrollsource.bottom-=BoxSize;
				scrolldest = scrollsource;
				OffsetRect(&scrolldest, 0, BoxSize);
				
				dest=bounds[y];
				dest.bottom=dest.top+BoxSize;
				
				CopyBits(&(thePtr->portBits), &(thePtr->portBits),
						&scrollsource, &scrolldest, 0, 0L);
				FillRect(&dest, *thePattern);
				
			}
		}
		TimeCorrection(CorrectTime);
	}
	
	SetRect(&dest, thePtr->portRect.left,
		thePtr->portRect.top, thePtr->portRect.right,
		thePtr->portRect.bottom);
	FillRect(&dest, *thePattern);
}
