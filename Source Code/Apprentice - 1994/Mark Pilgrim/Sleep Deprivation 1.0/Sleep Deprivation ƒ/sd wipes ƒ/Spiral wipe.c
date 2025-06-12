/**********************************************************************\

File:		Spiral wipe.c

Purpose:	This module handles clearing the screen in a funky
			manner.  See the comments below for more details.
			

Sleep Deprivation -- graphic effects on sleep
Copyright (C) 1993 Mark Pilgrim & Dave Blumenthal

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

#include "msg timing.h"

#define BOXSIZE 20
#define CorrectTime 1

void SpiralGyra(GrafPtr, Pattern*);

/* Start in the topleft corner, facing downwards.  Copy until you hit (a) the
   edge of the screen, or (b) bits you've already copied.  Then turn counter-
   clockwise and do it again.  */
   
void SpiralGyra(GrafPtr thePtr, Pattern *thePattern)
{
	int			stop,sbottom,sleft,sright,iterrow,itercol,direction;
	Rect		source;
	Boolean		everyOther;
	int			width,height;

	width=thePtr->portRect.right-thePtr->portRect.left;
	height=thePtr->portRect.bottom-thePtr->portRect.top;
	
	everyOther=FALSE;
	stop=0;
	sbottom=height/BOXSIZE-(height%BOXSIZE ? 0 : 1);
	sleft=0;
	sright=width/BOXSIZE-(width%BOXSIZE ? 0 : 1);
	direction=3;
	iterrow=stop;
	itercol=sleft;
	while ((stop<=sbottom)&&(sleft<=sright))
	{
		StartTiming();
		source.top=iterrow*BOXSIZE;         /* Yes, I know I should recode this */
		source.bottom=source.top+BOXSIZE;   /* to take out multiplication. */
		source.left=itercol*BOXSIZE;        /* If it matter that much to you, */
		source.right=source.left+BOXSIZE;   /* you do it. */

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
}
