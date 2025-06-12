/**********************************************************************\

File:		Skipaline U-D fade.c

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

#define CorrectTime 1
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short SkipalineFade(Rect boundsRect, Pattern *thePattern);

/* Copy even-numbered rows starting at the top and moving down, and copy odd-
   numbered rows starting at the bottom and moving up. */
   
pascal short SkipalineFade(Rect boundsRect, Pattern *thePattern)
{
	Rect		thisone,thatone;
	
	SetRect(&thisone, boundsRect.left, boundsRect.top, boundsRect.right, boundsRect.top+1);
	SetRect(&thatone, boundsRect.left, boundsRect.bottom-1, boundsRect.right, boundsRect.bottom);
	if (theWindowHeight%2)
		OffsetRect(&thatone, 0, -1);
	
	while (thisone.top<boundsRect.bottom)
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
	
	return 0;
}
