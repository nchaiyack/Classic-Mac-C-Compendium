#include "timing.h"

#define CorrectTime 2
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CircularWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Trace a region from the center to the topleft corner, over <BlockSize> pixels,
   and back to the center.  Fill that in and move the region parameters +BlockSize.
   Repeat for all sides. */

pascal short CircularWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	RgnHandle	curregion;
	short			cx,cy,lastx,lasty;
	short			BlockSize;
	
	BlockSize=theWindowWidth/40;
	cx = boundsRect.left + theWindowWidth / 2;
	cy = boundsRect.top + theWindowHeight / 2;

	curregion=NewRgn();
	lastx=boundsRect.left;
	do                                            /* top quadrant */
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(lastx,boundsRect.top);
			Line(BlockSize,0);
			LineTo(cx,cy);
		CloseRgn(curregion);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&boundsRect, &boundsRect, 0, curregion);
		lastx+=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (lastx<boundsRect.right);
	
	lasty=boundsRect.top;
	do                                            /* right quadrant */
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(boundsRect.right,lasty);
			Line(0,BlockSize);
			LineTo(cx,cy);
		CloseRgn(curregion);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&boundsRect, &boundsRect, 0, curregion);
		lasty+=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (lasty<boundsRect.bottom);
	
	lastx=boundsRect.right;
	do                                            /* bottom quadrant */
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(lastx,boundsRect.bottom);
			Line(-BlockSize, 0);
			LineTo(cx,cy);
		CloseRgn(curregion);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&boundsRect, &boundsRect, 0, curregion);
		lastx-=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (lastx>boundsRect.left-BlockSize);
	
	lasty=boundsRect.bottom;
	do                                            /* left quadrant */
	{
		StartTiming();
		SetEmptyRgn(curregion);
		MoveTo(cx,cy);
		OpenRgn();
			LineTo(boundsRect.left,lasty);
			Line(0, -BlockSize);
			LineTo(cx,cy);
		CloseRgn(curregion);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&boundsRect, &boundsRect, 0, curregion);
		lasty-=BlockSize;
		TimeCorrection(CorrectTime);
	}
	while (lasty>boundsRect.top-BlockSize);
	
	DisposeRgn(curregion);
	
	return 0;
}
	