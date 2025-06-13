#include "timing.h"

#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short HalvesScroll(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* 2 regions, split down the middle of the screen.  Scroll the screen up in one
   region and down in the other. */
   
pascal short HalvesScroll(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			x;
	Rect		theTopRect, topdest, theBottomRect, bottomdest;
	Rect		topscrollsource, bottomscrollsource;
	short			cx;
	short			BoxSize;
	
	BoxSize=theWindowHeight/25;
	cx = boundsRect.left + theWindowWidth / 2;

	topscrollsource=topdest=bottomscrollsource=bottomdest=boundsRect;
	topscrollsource.right=topdest.right=bottomscrollsource.left=bottomdest.left=cx;
	topdest.bottom=topdest.top+BoxSize;
	bottomdest.top=bottomdest.bottom-BoxSize;
	SetRect(&theTopRect, boundsRect.left, boundsRect.bottom-BoxSize, cx, boundsRect.bottom);
	SetRect(&theBottomRect, cx, boundsRect.top, boundsRect.right, boundsRect.top+BoxSize);
	
	for(x = theWindowHeight - BoxSize; x >= 0; x -= BoxSize)
	{
		StartTiming();
		ScrollTheRect(&topscrollsource, 0, BoxSize, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theTopRect, &topdest, 0, 0L);
		theTopRect.bottom-=BoxSize;
		theTopRect.top-=BoxSize;
		
		ScrollTheRect(&bottomscrollsource, 0, -BoxSize, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theBottomRect, &bottomdest, 0, 0L);
		theBottomRect.top+=BoxSize;
		theBottomRect.bottom+=BoxSize;
		
		TimeCorrection(CorrectTime);
	}
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);
	
	return 0;
}
