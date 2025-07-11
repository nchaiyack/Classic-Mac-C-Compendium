/**********************************************************************\

File:		Full scroll left.c

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

#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short FullScrollLeft(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Take the whole screen minus the lefthand strip; move it into the whole screen
   shifted left by BoxSize; take the lefthand strip of the source window and
   move it into the right strip of the dest.  */
   
pascal short FullScrollLeft(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	int				x, y;
	Rect			theRect, dest;
	int				BoxSize;
	
	BoxSize=theWindowWidth/25;
	
	dest = boundsRect;
	dest.left=dest.right-BoxSize;				/* righthand strip */
	
	SetRect(&theRect, boundsRect.left, boundsRect.top, boundsRect.left+BoxSize, boundsRect.bottom);
	
	for(x = theWindowWidth - BoxSize; x >= 0; x -= BoxSize)
	{
		StartTiming();
		ScrollTheRect(&boundsRect, -BoxSize, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theRect, &dest, 0, 0L);
		theRect.right+=BoxSize;
		theRect.left+=BoxSize;
		TimeCorrection(CorrectTime);
	}
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);
	
	return 0;
}
