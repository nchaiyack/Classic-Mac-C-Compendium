/**********************************************************************\

File:		Skipaline U-D.c

Purpose:	Graphic effect from offscreen bitmap to main window (on
			screen).  See comments below for more description.


Jotto ][ -=- a simple word game, revisited
Copyright (C) 1993 Mark Pilgrim

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
#include "wipe dispatch.h"

#define CorrectTime 1

/* Copy even-numbered rows starting at the top and moving down, and copy odd-
   numbered rows starting at the bottom and moving up. */
   
void Skipaline(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, int theWindowHeight, int theWindowWidth)
{
	Rect		thisone,thatone;
	
	thisone.top=thisone.left=thatone.left=0;
	thisone.bottom=1;
	thatone.bottom=theWindowHeight;
	thatone.top=theWindowHeight-1;
	thisone.right=thatone.right=theWindowWidth;
	
	while (thisone.top<theWindowHeight)
	{
		StartTiming();
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&thisone, &thisone, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&thatone, &thatone, 0, 0L);
		thisone.top+=2;             /* even row goes down by 2 */
		thisone.bottom+=2;
		thatone.top-=2;             /* odd row goes up by 2 */
		thatone.bottom-=2;
		TimeCorrection(CorrectTime);
	}
}