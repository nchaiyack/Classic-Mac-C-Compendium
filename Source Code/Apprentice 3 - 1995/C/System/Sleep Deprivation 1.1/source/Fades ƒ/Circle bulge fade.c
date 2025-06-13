/**********************************************************************\

File:		Circle bulge fade.c

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

#define	startgap			1
#define	gapratio			6
#define	zeropointH			15
#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CircleBulgeFade(Rect boundsRect, Pattern *thePattern);

/* This copies ovals that always run from the left of the screen to the right,
   but start really squashed and get geometrically taller.  */
   
pascal short CircleBulgeFade(Rect boundsRect, Pattern *thePattern)
{
	Rect		theRect;
	RgnHandle	curregion, boundsRgn, sectrgn;
	Point		zeropoint;
	int			cy=theWindowHeight/2;
	int			gap=startgap;
	
	theRect.left=boundsRect.left;
	theRect.right=boundsRect.right;
	theRect.top=boundsRect.top+cy-gap;
	theRect.bottom=boundsRect.top+cy+gap;
		
	boundsRgn=NewRgn();
	SetRectRgn(boundsRgn, boundsRect.left, boundsRect.top, boundsRect.right, boundsRect.bottom);
	sectrgn=NewRgn();
	curregion=NewRgn();
	zeropoint.v=boundsRect.top;
	zeropoint.h=boundsRect.left+zeropointH;
	do
	{
		StartTiming();

		SetEmptyRgn(curregion);
		OpenRgn();
			FrameOval(&theRect);  /* this makes the region for copying */
		CloseRgn(curregion);

		SectRgn(curregion, boundsRgn, sectrgn);
		FillRgn(sectrgn, *thePattern);

		theRect.top-=gap;
		theRect.bottom+=gap;     /* make the oval taller */
		gap++;
		gap+=gap/gapratio;       /* make the oval grow faster next time */
		
		TimeCorrection(CorrectTime);
	}
	while (!(PtInRgn(zeropoint, curregion)));	/* quit when we hit zeropoint */

	FillRect(&boundsRect, *thePattern);			/* fill the whole screen to end it */
	
	DisposeRgn(curregion);
	DisposeRgn(boundsRgn);
	DisposeRgn(sectrgn);
	
	return 0;
}
