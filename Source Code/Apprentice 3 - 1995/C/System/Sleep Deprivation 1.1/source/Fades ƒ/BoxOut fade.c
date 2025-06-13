/**********************************************************************\

File:		BoxOut fade.c

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

#define		BOXES	50	/* increase this for a finer grained but more timely effect */
#define CorrectTime 2
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short BoxOutFade(Rect boundsRect, Pattern *thePattern);

pascal short BoxOutFade(Rect boundsRect, Pattern *thePattern)
{
	Rect	theRect;
	int		cenx, ceny;
	int		boxnum;
	
	cenx = theWindowWidth / 2;
	ceny = theWindowHeight / 2;
	
	for(boxnum = 0; boxnum <= BOXES; boxnum++)
	{
		StartTiming();
		
		/* this is not the most efficient method, but the overhead of the
		multiplies and the redundant copying is hidden by the timing mechanism */
		theRect.left = cenx - ((boxnum * cenx) / BOXES);
		theRect.right = cenx + ((boxnum * cenx) / BOXES);
		theRect.top = ceny - ((boxnum * ceny) / BOXES);
		theRect.bottom = ceny + ((boxnum * ceny) / BOXES);
		OffsetRect(&theRect, boundsRect.left, boundsRect.top);
		
		FillRect(&theRect, *thePattern);
		TimeCorrection(CorrectTime);
	}
	
	return 0;
}
