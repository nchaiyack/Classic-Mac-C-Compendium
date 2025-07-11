/**********************************************************************\

File:		HGR2 fade.c

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

#define CorrectTime 4
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short HGR2Fade(Rect boundsRect, Pattern *thePattern);

/* Copy even-numbered rows starting at the top and moving down, and copy odd-
   numbered rows starting at the bottom and moving up. */
   
pascal short HGR2Fade(Rect boundsRect, Pattern *thePattern)
{
	Rect			thisone;
	int				i;
	int				numCols;
	
	numCols=theWindowWidth/24;
	for (i=0; i<numCols; i++)
	{
		StartTiming();
		
		SetRect(&thisone, boundsRect.left+i, boundsRect.top, boundsRect.left+i+1,
			boundsRect.bottom);
		
		while (thisone.left<boundsRect.right)
		{
			FillRect(&thisone, *thePattern);
			thisone.left+=numCols;
			thisone.right+=numCols;
		}
		TimeCorrection(CorrectTime);
	}
	
	return 0;
}
