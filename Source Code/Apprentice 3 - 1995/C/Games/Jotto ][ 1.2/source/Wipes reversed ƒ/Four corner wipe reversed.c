#include "timing.h"

#define CorrectTime 1
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short FourCornerReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Take four rects, one at each corner, and make them bigger, growing on each
   side of the screen.  This means lots of overlap copying, but the timing masks
   it and Quickdraw may even take care of some of it. (?) */

pascal short FourCornerReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect		tl,tr,bl,br;
	short			cx,cy;
	short			VBarGap, HBarGap;
	
	VBarGap=theWindowWidth/100;
	HBarGap=theWindowHeight/100;

	cx = boundsRect.left + theWindowWidth/2;
	cy = boundsRect.top + theWindowHeight/2;
	
	tl.top=tr.top=tl.bottom=tr.bottom=boundsRect.top;
	tl.left=bl.left=tl.right=bl.right=boundsRect.left;
	tr.right=br.right=tr.left=br.left=boundsRect.right;
	bl.bottom=br.bottom=bl.top=br.top=boundsRect.bottom;
	while (tl.right<cx)
	{
		StartTiming();
		tl.bottom+=HBarGap;
		tr.bottom+=HBarGap;
		bl.top-=HBarGap;
		br.top-=HBarGap;
		tl.right+=VBarGap;
		bl.right+=VBarGap;
		tr.left-=VBarGap;
		br.left-=VBarGap;
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&tl, &tl, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&tr, &tr, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&bl, &bl, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&br, &br, 0, 0L);
		TimeCorrection(CorrectTime);
	}
	
	return 0;
}
