/**********************************************************************\

File:		Full scroll up fade.c

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

#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short FullScrollUpFade(Rect boundsRect, Pattern *thePattern);

/* Take the full screen minus the top strip, move it into the whole screen
   shifted up by BoxSize, then take the topmost source strip and put it
   in the bottom strip of the dest. window. */
   
pascal short FullScrollUpFade(Rect boundsRect, Pattern *thePattern)
{
	int			x, y;
	Rect		dest;
	int			BoxSize;
	
	BoxSize=theWindowHeight/25;
	
	dest = boundsRect;
	dest.top=dest.bottom-BoxSize;				/* bottom strip for new data */
	
	for(x = theWindowHeight - BoxSize; x >= 0; x -= BoxSize)
	{
		StartTiming();
		ScrollTheRect(&boundsRect, 0, -BoxSize, 0L);
		FillRect(&dest, *thePattern);
		TimeCorrection(CorrectTime);
	}
	
	FillRect(&boundsRect, *thePattern);
	
	return 0;
}
