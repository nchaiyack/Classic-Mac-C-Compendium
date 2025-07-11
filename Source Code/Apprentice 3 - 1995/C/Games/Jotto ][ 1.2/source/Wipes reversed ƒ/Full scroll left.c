#include "timing.h"

#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short FullScrollLeft(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Take the whole screen minus the lefthand strip; move it into the whole screen
   shifted left by BoxSize; take the lefthand strip of the source window and
   move it into the right strip of the dest.  */
   
pascal short FullScrollLeft(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			x;
	Rect			theRect, dest;
	short			BoxSize;
	
	BoxSize=theWindowWidth/25;
	
	dest = boundsRect;
	dest.left=dest.right-BoxSize;				/* righthand strip */
	
	SetRect(&theRect, boundsRect.left, boundsRect.top, boundsRect.left+BoxSize, boundsRect.bottom);
	
	for(x = theWindowWidth - BoxSize; x >= 0; x -= BoxSize)
	{
		StartTiming();
		ScrollTheRect(&boundsRect, -BoxSize, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theRect, &dest, 0, 0L);
		theRect.right+=BoxSize;
		theRect.left+=BoxSize;
		TimeCorrection(CorrectTime);
	}
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);
	
	return 0;
}
