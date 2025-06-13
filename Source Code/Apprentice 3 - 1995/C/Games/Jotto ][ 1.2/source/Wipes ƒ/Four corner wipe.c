#include "timing.h"

#define CorrectTime 1
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short FourCorner(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Take 4 bars, two on each axis, and move them towards different corners.
   This means lots of overlap copying, but the timing masks it and Quickdraw
   may even take care of some of it. (?) */

pascal short FourCorner(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect		vsource1,hsource1,vsource2,hsource2;
	short			vbar,hbar,cx,cy;
	short			VBarGap, HBarGap;
	
	VBarGap=theWindowWidth/100;
	HBarGap=theWindowHeight/100;

	vbar=VBarGap;
	hbar=HBarGap;
	cx = boundsRect.left + theWindowWidth/2;
	cy = boundsRect.top + theWindowHeight/2;
	vsource1.top=vsource2.top=boundsRect.top;
	hsource2.left=hsource1.left=boundsRect.left;
	vsource1.bottom=vsource2.bottom=boundsRect.bottom;
	hsource1.right=hsource2.right=boundsRect.right;
	while (vbar+boundsRect.left<cx+VBarGap)
	{
		StartTiming();
		vsource1.left=cx-vbar;
		vsource1.right=vsource1.left+VBarGap;
		vsource2.right=cx+vbar;
		vsource2.left=vsource2.right-VBarGap;
		hsource1.top=cy-hbar;
		hsource1.bottom=hsource1.top+HBarGap;
		hsource2.bottom=cy+hbar;
		hsource2.top=hsource2.bottom-HBarGap;
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&vsource1, &vsource1, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&hsource1, &hsource1, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&vsource2, &vsource2, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&hsource2, &hsource2, 0, 0L);
		vbar+=VBarGap;
		hbar+=HBarGap;
		TimeCorrection(CorrectTime);
	}
	
	return 0;
}
