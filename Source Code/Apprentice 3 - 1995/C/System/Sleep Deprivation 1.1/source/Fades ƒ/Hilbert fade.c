/**********************************************************************\

File:		Hilbert wipe fade.c

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

/* This fills the screen by a Hilbert space-filling curve, which is defined by
   two mutually recursive drawing functions:
   
   X = left, Y, draw, right, X, draw, X, right, draw, Y, left
   Y = right, X, draw, left, Y, draw, Y, left, draw, X, right
   
   Start by drawing X at the highest recursion level from the bottomleft of the
   screen. (At recursion level 1, X and Y are null functions.)
*/

#define	RecursionLevel	4
#define CorrectTime 1
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal void HilbertFadeRecurse(Rect boundsRect, Pattern *thePattern,
	int level, int whichpattern, int* x, int* y);
pascal short HilbertFade(Rect boundsRect, Pattern *thePattern);

typedef char	Hilby[11];

static Hilby	HilbertPattern[]=
{
	{
		0x02,0x69,0x00,0x01,0x42,0x00,0x42,0x01,0x00,0x69,0x02
	},
	{
		0x01,0x42,0x00,0x02,0x69,0x00,0x69,0x02,0x00,0x42,0x01
	}
};

static int			HilbertDirection;
static int			vBlockSize;
static int			hBlockSize;

pascal void HilbertFadeRecurse(Rect boundsRect, Pattern *thePattern,
	int level, int whichpattern, int* x, int* y)
{
	int				i;
	Rect			source;
	
	for (i=0; i<11; i++)
	{
		switch (HilbertPattern[whichpattern][i])
		{
			case 0x01:     /* turn left */
				HilbertDirection--;
				if (HilbertDirection<0) HilbertDirection=3;
				break;
			case 0x02:     /* turn right */
				HilbertDirection++;
				if (HilbertDirection==4) HilbertDirection=0;
				break;
			case 0x00:    /* draw */
				StartTiming();
				SetRect(&source, *x, *y-vBlockSize, *x+hBlockSize, *y);
				OffsetRect(&source, boundsRect.left, boundsRect.top);
				SectRect(&source, &boundsRect, &source);
				FillRect(&source, *thePattern);
				switch (HilbertDirection)
				{
					case 0:
						*x+=hBlockSize;
						break;
					case 1:
						*y-=vBlockSize;
						break;
					case 2:
						*x-=hBlockSize;
						break;
					case 3:
						*y+=vBlockSize;
						break;
				}
				TimeCorrection(CorrectTime);
				break;
			case 0x42:    /* call X */
				if (level>1)
					HilbertFadeRecurse(boundsRect,thePattern,level-1,0,x,y);
				break;
			case 0x69:    /* call Y */
				if (level>1)
					HilbertFadeRecurse(boundsRect,thePattern,level-1,1,x,y);
				break;
		}
	}
}

pascal short HilbertFade(Rect boundsRect, Pattern *thePattern)
{
	int			curx, cury;
	int			answer, i;
	
	answer=1;
	for (i=0; i<RecursionLevel; i++)
		answer*=2;
	vBlockSize=1+theWindowHeight/answer;	/* used to be 20 */
	hBlockSize=1+theWindowWidth/answer;		/* used to be 32 */
	HilbertDirection=0;
	cury=theWindowHeight;
	curx=0;
	HilbertFadeRecurse(boundsRect,thePattern,RecursionLevel,0,&curx,&cury);
	FillRect(&boundsRect, *thePattern);		/* in case we missed any bits */
	
	return 0;
}
