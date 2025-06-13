#include "timing.h"

#define	startgap			1
#define	gapratio			6
#define	zeropointH			15
#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CircleBulge(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* This copies ovals that always run from the left of the screen to the right,
   but start really squashed and get geometrically taller.  */
   
pascal short CircleBulge(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect		theRect;
	RgnHandle	curregion;
	Point		zeropoint;
	short			cy=theWindowHeight/2;
	short			gap=startgap;
	
	theRect.left=boundsRect.left;
	theRect.right=boundsRect.right;
	theRect.top=boundsRect.top+cy-gap;
	theRect.bottom=boundsRect.top+cy+gap;
		
	curregion=NewRgn();
	zeropoint.v=boundsRect.top;
	zeropoint.h=boundsRect.left+zeropointH;
	do
	{
		StartTiming();

		SetEmptyRgn(curregion);
		OpenRgn();
			FrameOval(&theRect);  /* this makes the region for copying */
		CloseRgn(curregion);

		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&boundsRect, &boundsRect, 0, curregion);

		theRect.top-=gap;
		theRect.bottom+=gap;     /* make the oval taller */
		gap++;
		gap+=gap/gapratio;       /* make the oval grow faster next time */
		
		TimeCorrection(CorrectTime);
	}
	while (!(PtInRgn(zeropoint, curregion)));	/* quit when we hit zeropoint */

	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&boundsRect, &boundsRect, 0, 0L);	/* copy the whole screen to end it */
	
	DisposeRgn(curregion);
	
	return 0;
}
