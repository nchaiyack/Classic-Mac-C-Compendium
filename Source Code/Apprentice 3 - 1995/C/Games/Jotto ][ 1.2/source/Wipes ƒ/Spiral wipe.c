#include "timing.h"

#define CorrectTime 1
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short SpiralGyra(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Start in the topleft corner, facing downwards.  Copy until you hit (a) the
   edge of the screen, or (b) bits you've already copied.  Then turn counter-
   clockwise and do it again.  */
   
pascal short SpiralGyra(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			stop,sbottom,sleft,sright,iterrow,itercol,direction;
	Rect			source;
	Boolean			everyOther;
	short			BOXSIZE;
	RgnHandle		boundsRgn;
	
	boundsRgn=NewRgn();
	RectRgn(boundsRgn, &boundsRect);
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
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&source, &source, 0, boundsRgn);
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
