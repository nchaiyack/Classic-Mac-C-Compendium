#include "timing.h"

#define	gap			4		/* difference between one radius and the next */
#define CorrectTime 2
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CircleOut(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Make a circular region and use it as the mask in CopyBits.  Lots of overlap,
   but masked by timing correction.  If you want to optimize it, look at the
   donut region code in "Circle in.c" */
   
pascal short CircleOut(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect			theRect;
	short			cx, cy;
	RgnHandle		curregion;
	Point			zeroPoint;
	
	zeroPoint.h=boundsRect.left;
	zeroPoint.v=boundsRect.top;
	
	cx = theWindowWidth / 2;
	cy = theWindowHeight / 2;
	
	theRect.left=cx-gap;         /* circumscribing rectangle for circle */
	theRect.right=cx+gap;
	theRect.top=cy-gap;
	theRect.bottom=cy+gap;
	OffsetRect(&theRect, boundsRect.left, boundsRect.top);
	
	curregion=NewRgn();
	
	do
	{
		StartTiming();
		SetEmptyRgn(curregion);
		OpenRgn();
			FrameOval(&theRect);   /* the circle region */
		CloseRgn(curregion);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&boundsRect, &boundsRect, 0, curregion);
		theRect.left-=gap;        /* make the rect bigger (and thus the circle) */
		theRect.right+=gap;
		theRect.top-=gap;
		theRect.bottom+=gap;
		TimeCorrection(CorrectTime);
	}
	while (!PtInRgn(zeroPoint, curregion));
	
	DisposeRgn(curregion);
	
	return 0;
}
