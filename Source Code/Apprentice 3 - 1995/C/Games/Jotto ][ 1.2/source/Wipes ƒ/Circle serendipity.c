#include "timing.h"

#define CorrectTime 2
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CircleSerendipity(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* I haven't the slightest idea why this works.  Hence the name.  -MP */

pascal short CircleSerendipity(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect			theRect;
	RgnHandle		curregion;
	Point			zeropoint;
	short			gap;
	
	gap=theWindowHeight/40;
	SetRect(&theRect, boundsRect.left, boundsRect.top-2*theWindowHeight, boundsRect.right,
		boundsRect.bottom-theWindowHeight);
		
	curregion=NewRgn();
	SetEmptyRgn(curregion);
	OpenRgn();
		FrameOval(&theRect);
	CloseRgn(curregion);
	zeropoint.v=boundsRect.bottom;
	zeropoint.h=boundsRect.left;
	do
	{
		StartTiming();
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&boundsRect, &boundsRect, 0, curregion);
		OffsetRgn(curregion, 0, gap);
		TimeCorrection(CorrectTime);
	}
	while (!(PtInRgn(zeropoint, curregion)));

	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&boundsRect, &boundsRect, 0, 0L);
	
	DisposeRgn(curregion);
	
	return 0;
}
