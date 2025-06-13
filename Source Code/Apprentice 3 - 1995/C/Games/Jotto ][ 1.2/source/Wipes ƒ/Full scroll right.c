#include "timing.h"

#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short FullScrollLR(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Take the whole screen - the righthand strip; move it into the whole screen
   shifted right by BoxSize; take the righthand strip of the source window and
   move it into the lefthand strip of the dest.  */
   
pascal short FullScrollLR(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			x;
	Rect			theRect, dest;
	short			BoxSize;
	
	BoxSize=theWindowWidth/25;
	
	dest = boundsRect;
	dest.right=dest.left+BoxSize;				/* lefthand strip */
	
	SetRect(&theRect, boundsRect.right-BoxSize, boundsRect.top, boundsRect.right, boundsRect.bottom);
	
	for(x = theWindowWidth - BoxSize; x >= 0; x -= BoxSize)
	{
		StartTiming();
		ScrollTheRect(&boundsRect, BoxSize, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theRect, &dest, 0, 0L);
		theRect.right-=BoxSize;
		theRect.left-=BoxSize;
		TimeCorrection(CorrectTime);
	}
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);
	
	return 0;
}
