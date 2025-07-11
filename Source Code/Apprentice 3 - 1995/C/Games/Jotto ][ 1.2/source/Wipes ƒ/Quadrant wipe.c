#include "timing.h"

#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define NUM_ITERATIONS	25

pascal short QuadrantWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* 4 regions, splitting the screen into 4 quadrants.  Wipe the screen right in
   the top-left quadrant, down in the top-right, left in the bottom-right,
   up in the bottom-left. */
   
pascal short QuadrantWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			x;
	Rect		tldest, trdest, bldest, brdest;
	Rect		tlbounds, trbounds, blbounds, brbounds;
	RgnHandle	tlrgn, trrgn, blrgn, brrgn;
	short			cx,cy;
	short			BoxSize, HBoxSize;
	
	BoxSize=theWindowHeight/(NUM_ITERATIONS*2);
	HBoxSize=theWindowWidth/(NUM_ITERATIONS*2);
	
	cx = boundsRect.left + theWindowWidth / 2;
	cy = boundsRect.top + theWindowHeight / 2;
	
	tlbounds=trbounds=blbounds=brbounds=tldest=trdest=bldest=brdest=boundsRect;
	tlbounds.right=trbounds.left=blbounds.right=brbounds.left=
		tldest.right=trdest.left=bldest.right=brdest.left=cx;
	tlbounds.bottom=trbounds.bottom=blbounds.top=brbounds.top=
		tldest.bottom=trdest.bottom=bldest.top=brdest.top=cy;
	tlrgn=NewRgn();
	trrgn=NewRgn();
	blrgn=NewRgn();
	brrgn=NewRgn();
	RectRgn(tlrgn, &tlbounds);
	RectRgn(trrgn, &trbounds);
	RectRgn(blrgn, &blbounds);
	RectRgn(brrgn, &brbounds);
	tldest.right=tldest.left+HBoxSize;
	trdest.bottom=trdest.top+BoxSize;
	brdest.left=brdest.right-HBoxSize;
	bldest.top=bldest.bottom-BoxSize;
	
	for (x=0; x<NUM_ITERATIONS; x++)
	{
		StartTiming();
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&tldest, &tldest, 0, tlrgn);
		tldest.left+=HBoxSize;
		tldest.right+=HBoxSize;
		
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&trdest, &trdest, 0, trrgn);
		trdest.top+=BoxSize;
		trdest.bottom+=BoxSize;
		
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&brdest, &brdest, 0, brrgn);
		brdest.left-=HBoxSize;
		brdest.right-=HBoxSize;
		
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&bldest, &bldest, 0, blrgn);
		bldest.top-=BoxSize;
		bldest.bottom-=BoxSize;
		
		TimeCorrection(CorrectTime);
	}
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);
	
	DisposeRgn(tlrgn);
	DisposeRgn(trrgn);
	DisposeRgn(blrgn);
	DisposeRgn(brrgn);
	
	return 0;
}
