/**********************************************************************\

File:		Spiral fade.c

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

#define CorrectTime 1
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short SpiralGyraFade(Rect boundsRect, Pattern *thePattern);

/* Start in the topleft corner, facing downwards.  Copy until you hit (a) the
   edge of the screen, or (b) bits you've already copied.  Then turn counter-
   clockwise and do it again.  */
   
pascal short SpiralGyraFade(Rect boundsRect, Pattern *thePattern)
{
	int				stop,sbottom,sleft,sright,iterrow,itercol,direction;
	Rect			source;
	Boolean			everyOther;
	int				BOXSIZE;
	RgnHandle		boundsRgn;
	
	boundsRgn=NewRgn();
	SetRectRgn(boundsRgn, boundsRect.left, boundsRect.top, boundsRect.right, boundsRect.bottom);
	BOXSIZE=theWindowHeight/15;
	everyOther=FALSE;
	stop=0;
	sbottom=theWindowHeight/BOXSIZE-(theWindowHeight%BOXSIZE ? 0 : 1);
	sleft=0;
	sright=theWindowWidth/BOXSIZE-(theWindowWidth%BOXSIZE ? 0 : 1);
	direction=3;
	iterrow=stop;
	itercol=sleft;
	while ((stop<=sbottom)&&(sleft<=sright))
	{
		StartTiming();
		source.top=iterrow*BOXSIZE;
		source.bottom=source.top+BOXSIZE;
		source.left=itercol*BOXSIZE;
		source.right=source.left+BOXSIZE;
		OffsetRect(&source, boundsRect.left, boundsRect.top);
		FillRect(&source, *thePattern);
		switch (direction)
		{
			case 0:  /* facing right */
				if (itercol==sright)
				{
					sbottom--;
					direction++;
					iterrow--;
				}
				else itercol++;
				break;
			case 1:  /* facing up */
				if (iterrow==stop)   /* that reads "s top," not "stop" */
				{
					sright--;
					direction++;
					itercol--;
				}
				else iterrow--;
				break;
			case 2:  /* facing left */
				if (itercol==sleft)
				{
					stop++;
					direction++;
					iterrow++;
				}
				else itercol--;
				break;
			case 3:  /* facing down */
				if (iterrow==sbottom)
				{
					sleft++;
					direction=0;
					itercol++;
				}
				else iterrow++;
				break;
		}
		if (everyOther)
			TimeCorrection(CorrectTime);
		everyOther=!everyOther;
	}
	
	DisposeRgn(boundsRgn);
	
	return 0;
}
