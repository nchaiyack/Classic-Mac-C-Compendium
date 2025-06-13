/**********************************************************************\

File:		Full scroll left fade.c

Purpose:	Graphic effect to fade main screen to solid pattern.
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

pascal short FullScrollLeftFade(Rect boundsRect, Pattern *thePattern);

/* Take the whole screen minus the lefthand strip; move it into the whole screen
   shifted left by BoxSize; take the lefthand strip of the source window and
   move it into the right strip of the dest.  */
   
pascal short FullScrollLeftFade(Rect boundsRect, Pattern *thePattern)
{
	int				x, y;
	Rect			dest;
	int				BoxSize;
	
	BoxSize=theWindowWidth/25;
	
	dest = boundsRect;
	dest.left=dest.right-BoxSize;				/* righthand strip */
	
	for(x = theWindowWidth - BoxSize; x >= 0; x -= BoxSize)
	{
		StartTiming();
		ScrollTheRect(&boundsRect, -BoxSize, 0, 0L);
		FillRect(&dest, *thePattern);
		TimeCorrection(CorrectTime);
	}
	
	FillRect(&boundsRect, *thePattern);

	return 0;
}
