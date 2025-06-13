/**********************************************************************\

File:		HGR fade.c

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

pascal short HGRFade(Rect boundsRect, Pattern *thePattern);

/* Copy even-numbered rows starting at the top and moving down, and copy odd-
   numbered rows starting at the bottom and moving up. */
   
pascal short HGRFade(Rect boundsRect, Pattern *thePattern)
{
	Rect			thisone;
	int				i;
	int				numRows;
	
	numRows=theWindowHeight/24;
	for (i=0; i<numRows; i++)
	{
		StartTiming();
		
		SetRect(&thisone, boundsRect.left, boundsRect.top+i, boundsRect.right,
			boundsRect.top+i+1);
		
		while (thisone.top<boundsRect.bottom)
		{
			FillRect(&thisone, *thePattern);
			thisone.top+=numRows;
			thisone.bottom+=numRows;
		}
		TimeCorrection(CorrectTime);
	}
	
	return 0;
}
