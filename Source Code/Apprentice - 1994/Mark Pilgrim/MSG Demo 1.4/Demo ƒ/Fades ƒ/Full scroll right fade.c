/**********************************************************************\

File:		Full scroll right fade.c

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

pascal short FullScrollLRFade(Rect boundsRect, Pattern *thePattern);

/* Take the whole screen - the righthand strip; move it into the whole screen
   shifted right by BoxSize; take the righthand strip of the source window and
   move it into the lefthand strip of the dest.  */
   
pascal short FullScrollLRFade(Rect boundsRect, Pattern *thePattern)
{
	int				x, y;
	Rect			dest;
	int				BoxSize;
	
	BoxSize=theWindowWidth/25;
	
	dest = boundsRect;
	dest.right=dest.left+BoxSize;				/* lefthand strip */
	
	for(x = theWindowWidth - BoxSize; x >= 0; x -= BoxSize)
	{
		StartTiming();
		ScrollTheRect(&boundsRect, BoxSize, 0, 0L);
		FillRect(&dest, *thePattern);
		TimeCorrection(CorrectTime);
	}
	
	FillRect(&boundsRect, *thePattern);
	
	return 0;
}
