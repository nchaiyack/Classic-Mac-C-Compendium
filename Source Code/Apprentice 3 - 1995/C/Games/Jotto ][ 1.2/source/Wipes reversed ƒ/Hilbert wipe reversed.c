#include "timing.h"

/* This fills the screen by a Hilbert space-filling curve, which is defined by
   two mutually recursive drawing functions:
   
   X = left, Y, draw, right, X, draw, X, right, draw, Y, left
   Y = right, X, draw, left, Y, draw, Y, left, draw, X, right
   
   Start by drawing X at the highest recursion level from the bottomleft of the
   screen. (At recursion level 1, X and Y are null functions.)
   
   [Note that the procedure below actually draws the Hilbert curve flipped
   horizontally on the screen, so it starts drawing at the bottomright and
   works leftward.]
*/

#define	RecursionLevel	4
#define CorrectTime 1
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal void HilbertRecurseReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect,
	short level, short whichpattern, short* x, short* y);
pascal short HilbertWipeReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

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

static short			HilbertDirection;
static short			vBlockSize;
static short			hBlockSize;
static RgnHandle	boundsRgn;

pascal void HilbertRecurseReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr,
	Rect boundsRect, short level, short whichpattern, short* x, short* y)
{
	short			i;
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
				SetRect(&source, theWindowWidth-*x-hBlockSize, *y-vBlockSize,
					theWindowWidth-*x, *y);
				OffsetRect(&source, boundsRect.left, boundsRect.top);
				CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
								&source, &source, 0, boundsRgn);
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
					HilbertRecurseReversed(sourceGrafPtr,destGrafPtr,boundsRect,level-1,0,x,y);
				break;
			case 0x69:    /* call Y */
				if (level>1)
					HilbertRecurseReversed(sourceGrafPtr,destGrafPtr,boundsRect,level-1,1,x,y);
				break;
		}
	}
}

pascal short HilbertWipeReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			curx, cury;
	short			answer, i;
	
	answer=1;
	for (i=0; i<RecursionLevel; i++)
		answer*=2;
	vBlockSize=1+theWindowHeight/answer;	/* used to be 20 */
	hBlockSize=1+theWindowWidth/answer;		/* used to be 32 */
	HilbertDirection=0;
	boundsRgn=NewRgn();
	RectRgn(boundsRgn, &boundsRect);
	cury=theWindowHeight;
	curx=0;
	HilbertRecurseReversed(sourceGrafPtr,destGrafPtr,boundsRect,RecursionLevel,0,&curx,&cury);
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&boundsRect, &boundsRect, 0, 0L);  /* in case we missed any bits */
	
	DisposeRgn(boundsRgn);
	
	return 0;
}
