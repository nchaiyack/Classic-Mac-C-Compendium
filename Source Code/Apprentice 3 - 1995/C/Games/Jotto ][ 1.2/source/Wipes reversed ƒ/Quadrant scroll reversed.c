#include "timing.h"

#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define NUM_ITERATIONS	25

pascal short QuadrantScrollReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* 4 regions, splitting the screen into 4 quadrants.  Scroll the screen left in
   the top-left quadrant, up in the top-right, right in the bottom-right,
   down in the bottom-left. */
   
pascal short QuadrantScrollReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			x;
	Rect		tlsource, trsource, blsource, brsource;
	Rect		tldest, trdest, bldest, brdest;
	Rect		tlscroll, trscroll, blscroll, brscroll;
	short			cx,cy;
	short			BoxSize, HBoxSize;
	
	BoxSize=theWindowHeight/(NUM_ITERATIONS*2);
	HBoxSize=theWindowWidth/(NUM_ITERATIONS*2);
	
	cx = boundsRect.left + theWindowWidth / 2;
	cy = boundsRect.top + theWindowHeight / 2;
	
	tlscroll=trscroll=blscroll=brscroll=tldest=trdest=bldest=brdest=boundsRect;
	tlscroll.right=trscroll.left=blscroll.right=brscroll.left=
		tldest.right=trdest.left=bldest.right=brdest.left=cx;
	tlscroll.bottom=trscroll.bottom=blscroll.top=brscroll.top=
		tldest.bottom=trdest.bottom=bldest.top=brdest.top=cy;
	brdest.right=brdest.left+HBoxSize;
	bldest.bottom=bldest.top+BoxSize;
	tldest.left=tldest.right-HBoxSize;
	trdest.top=trdest.bottom-BoxSize;
	
	SetRect(&tlsource, boundsRect.left, boundsRect.top, boundsRect.left+HBoxSize, cy);
	SetRect(&trsource, cx, boundsRect.top, boundsRect.right, boundsRect.top+BoxSize);
	SetRect(&brsource, boundsRect.right-HBoxSize, cy, boundsRect.right, boundsRect.bottom);
	SetRect(&blsource, boundsRect.left, boundsRect.bottom-BoxSize, cx, boundsRect.bottom);
	
	for (x=0; x<NUM_ITERATIONS; x++)
	{
		StartTiming();
		ScrollTheRect(&brscroll, HBoxSize, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&brsource, &brdest, 0, 0L);
		brsource.left-=HBoxSize;
		brsource.right-=HBoxSize;
		
		ScrollTheRect(&blscroll, 0, BoxSize, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&blsource, &bldest, 0, 0L);
		blsource.top-=BoxSize;
		blsource.bottom-=BoxSize;
		
		ScrollTheRect(&tlscroll, -HBoxSize, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&tlsource, &tldest, 0, 0L);
		tlsource.left+=HBoxSize;
		tlsource.right+=HBoxSize;
		
		ScrollTheRect(&trscroll, 0, -BoxSize, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&trsource, &trdest, 0, 0L);
		trsource.top+=BoxSize;
		trsource.bottom+=BoxSize;
		
		TimeCorrection(CorrectTime);
	}
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);
	
	return 0;
}
