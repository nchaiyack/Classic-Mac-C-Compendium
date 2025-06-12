/**********************************************************************\

File:		Skipaline U-D.c

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

#define CorrectTime 1

void Skipaline(GrafPtr, Pattern*);

/* Copy even-numbered rows starting at the top and moving down, and copy odd-
   numbered rows starting at the bottom and moving up. */
   
void Skipaline(GrafPtr thePtr, Pattern *thePattern)
{
	Rect		thisone,thatone;
	int			width,height;

	width=thePtr->portRect.right-thePtr->portRect.left;
	height=thePtr->portRect.bottom-thePtr->portRect.top;
		
	thisone.top=thisone.left=thatone.left=0;
	thisone.bottom=1;
	thatone.bottom=height;
	thatone.top=height-1;
	thisone.right=thatone.right=width;
	
	while (thisone.top<height)
	{
		StartTiming();
		FillRect(&thisone, *thePattern);
		FillRect(&thatone, *thePattern);
		
		thisone.top+=2;             /* even row goes down by 2 */
		thisone.bottom+=2;
		thatone.top-=2;             /* odd row goes up by 2 */
		thatone.bottom-=2;
		TimeCorrection(CorrectTime);
	}
}
