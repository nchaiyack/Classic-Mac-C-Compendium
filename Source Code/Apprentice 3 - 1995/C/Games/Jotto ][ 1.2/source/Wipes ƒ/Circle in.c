#include "timing.h"

#define	gap			4		/* difference between one radius and the next */
#define CorrectTime 2
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CircleIn(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Take a really big circle, then a slightly smaller circle (-gap), then
   take the region in between them and copy that, then decrease the outer
   circle radius by gap.  Thus: circle in, by successive donut-like copies. */

pascal short CircleIn(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect			theRect;
	short			cx, cy;
	RgnHandle		curregion,lastregion,diffregion;
	Point			zeroPoint;
	short			StartRadius;
	
	zeroPoint.h=boundsRect.left;
	zeroPoint.v=boundsRect.top;

	cx = theWindowWidth / 2;
	cy = theWindowHeight / 2;
	
	lastregion=NewRgn();
	StartRadius=0;
	do
	{
		StartRadius+=2*gap;
		theRect.left=cx-StartRadius;     /* circumscribing rectangle for outer circle */
		theRect.right=cx+StartRadius;
		theRect.top=cy-StartRadius;
		theRect.bottom=cy+StartRadius;
		OffsetRect(&theRect, boundsRect.left, boundsRect.top);
		SetEmptyRgn(lastregion);
		OpenRgn();
			FrameOval(&theRect);        /* first circle */
		CloseRgn(lastregion);
	}
	while (!PtInRgn(zeroPoint, lastregion));
	
	curregion=NewRgn();
	diffregion=NewRgn();

	while (theRect.right-theRect.left>0)
	{
		StartTiming();
		theRect.left+=gap;
		theRect.right-=gap;
		theRect.top+=gap;
		theRect.bottom-=gap;
		SetEmptyRgn(curregion);
		OpenRgn();
			FrameOval(&theRect);   /* inner circle */
		CloseRgn(curregion);
		DiffRgn(lastregion,curregion,diffregion);   /* donut we need */
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&boundsRect, &boundsRect, 0, diffregion);
		CopyRgn(curregion,lastregion);    /* outer circle = inner circle */
		TimeCorrection(CorrectTime);
	}
	
	DisposeRgn(curregion);
	DisposeRgn(lastregion);
	DisposeRgn(diffregion);
	
	return 0;
}
