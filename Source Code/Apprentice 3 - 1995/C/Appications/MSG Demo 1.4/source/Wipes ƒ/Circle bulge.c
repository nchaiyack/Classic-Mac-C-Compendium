/**********************************************************************\

File:		Circle bulge.c

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

#define	startgap			1
#define	gapratio			6
#define	zeropointH			15
#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CircleBulge(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* This copies ovals that always run from the left of the screen to the right,
   but start really squashed and get geometrically taller.  */
   
pascal short CircleBulge(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect		theRect;
	RgnHandle	curregion;
	Point		zeropoint;
	int			cy=theWindowHeight/2;
	int			gap=startgap;
	
	theRect.left=boundsRect.left;
	theRect.right=boundsRect.right;
	theRect.top=boundsRect.top+cy-gap;
	theRect.bottom=boundsRect.top+cy+gap;
		
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

		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&boundsRect, &boundsRect, 0, curregion);

		theRect.top-=gap;
		theRect.bottom+=gap;     /* make the oval taller */
		gap++;
		gap+=gap/gapratio;       /* make the oval grow faster next time */
		
		TimeCorrection(CorrectTime);
	}
	while (!(PtInRgn(zeropoint, curregion)));	/* quit when we hit zeropoint */

	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&boundsRect, &boundsRect, 0, 0L);	/* copy the whole screen to end it */
	
	DisposeRgn(curregion);
	
	return 0;
}
