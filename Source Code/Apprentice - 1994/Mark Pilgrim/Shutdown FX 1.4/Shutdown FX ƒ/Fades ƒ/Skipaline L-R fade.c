/**********************************************************************\

File:		Skipaline L-R fade.c

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

pascal short SkipalineLRFade(Rect boundsRect, Pattern *thePattern);

/* Copy even-numbered columns starting at the left and moving right, and odd-
   numbered columns starting at the right and moving left.  */
   
pascal short SkipalineLRFade(Rect boundsRect, Pattern *thePattern)
{
	Rect		thisone,thatone;
	Boolean		everyOther=FALSE;
	
	SetRect(&thisone, boundsRect.left, boundsRect.top, boundsRect.left+1, boundsRect.bottom);
	SetRect(&thatone, boundsRect.right-1, boundsRect.top, boundsRect.right, boundsRect.bottom);
	if (theWindowWidth%2)
		OffsetRect(&thatone, -1, 0);
	
	while (thisone.left<boundsRect.right)
	{
		StartTiming();
		FillRect(&thisone, *thePattern);
		FillRect(&thatone, *thePattern);
		thisone.left+=2;      /* left column moves right by 2 */
		thisone.right+=2;
		thatone.left-=2;      /* right column moves left by 2 */
		thatone.right-=2;
		if (everyOther)
			TimeCorrection(CorrectTime);
		everyOther=!everyOther;
	}
	
	return 0;
}
