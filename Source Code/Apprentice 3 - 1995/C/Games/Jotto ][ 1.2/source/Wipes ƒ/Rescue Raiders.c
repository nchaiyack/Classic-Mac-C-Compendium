#include "timing.h"

#define CorrectTime 5
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short RescueRaiders(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* One region in 8 parts.  Each part of the region either starts at a corner
   or in the middle of a side and moves progressively clockwise until the
   entire screen is filled. Named after a similar effect in the game Rescue
   Raiders on the Apple ][e (now called Armor Alleyª on the Mac, but it doesn't
   have this effect in it anymore). */
   
pascal short RescueRaiders(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	RgnHandle	curregion;
	short			cx,cy,lastx,lasty;
	short			BlockSize, VBlockSize;
	
	cx = theWindowWidth / 2;
	cy = theWindowHeight / 2;
	BlockSize=theWindowWidth/25;
	VBlockSize=theWindowHeight/25;

	lasty=lastx=0;
	curregion=NewRgn();
	do
	{
		StartTiming();

		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(lastx,0);
			Line(BlockSize,0);
			LineTo(theWindowWidth-lastx-BlockSize,theWindowHeight);
			Line(BlockSize,0);
			LineTo(cx,cy);
			
			LineTo(cx+lastx,0);
			Line(BlockSize,0);
			LineTo(cx-lastx-BlockSize,theWindowHeight);
			Line(BlockSize,0);
			LineTo(cx,cy);
			
			LineTo(theWindowWidth,lasty);
			Line(0,VBlockSize);
			LineTo(0,theWindowHeight-lasty-VBlockSize);
			Line(0,VBlockSize);
			LineTo(cx,cy);
			
			LineTo(theWindowWidth,cy+lasty);
			Line(0,VBlockSize);
			LineTo(0,cy-lasty-VBlockSize);
			Line(0,VBlockSize);
			LineTo(cx,cy);
		CloseRgn(curregion);
		OffsetRgn(curregion, boundsRect.left, boundsRect.top);
		
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&boundsRect, &boundsRect, 0, curregion);

		lastx+=BlockSize;
		lasty+=VBlockSize;

		TimeCorrection(CorrectTime);
	}
	while (lastx<cx);
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);   /* in case we missed any bits */
	
	DisposeRgn(curregion);
	
	return 0;
}
