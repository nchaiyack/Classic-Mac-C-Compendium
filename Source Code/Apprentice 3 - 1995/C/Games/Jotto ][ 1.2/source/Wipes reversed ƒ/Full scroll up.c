#include "timing.h"

#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short FullScrollUp(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Take the full screen minus the top strip, move it into the whole screen
   shifted up by BoxSize, then take the topmost source strip and put it
   in the bottom strip of the dest. window. */
   
pascal short FullScrollUp(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			x;
	Rect		theRect, dest;
	short			BoxSize;
	
	BoxSize=theWindowHeight/25;
	
	dest = boundsRect;
	dest.top=dest.bottom-BoxSize;				/* bottom strip for new data */
	
	SetRect(&theRect, boundsRect.left, boundsRect.top, boundsRect.right, boundsRect.top+BoxSize);
	
	for(x = theWindowHeight - BoxSize; x >= 0; x -= BoxSize)
	{
		StartTiming();
		ScrollTheRect(&boundsRect, 0, -BoxSize, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theRect, &dest, 0, 0L);
		theRect.bottom+=BoxSize;
		theRect.top+=BoxSize;
		TimeCorrection(CorrectTime);
	}
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);
	
	return 0;
}
