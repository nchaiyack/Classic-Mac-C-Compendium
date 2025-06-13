#include "timing.h"

#define CorrectTime 2
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short DiagonalWipeDownRight(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* If you make a long enough region starting at the top-left corner and
   making a strip up and right of <BoxSize> width, all you have to do is move
   the region down until the entire screen is filled.  Dave thinks ideas like
   this should be taken out and shot, but it works. */
   
pascal short DiagonalWipeDownRight(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			maxmax;
	RgnHandle		archie;
	short			BoxSize;
	Point			zeroPoint;
	
	zeroPoint.h=boundsRect.right;
	zeroPoint.v=boundsRect.bottom;
	
	BoxSize=theWindowWidth/40;
	
	maxmax=(theWindowWidth>theWindowHeight) ? theWindowWidth : theWindowHeight;
	maxmax+=BoxSize;
	archie=NewRgn();
	OpenRgn();
		MoveTo(boundsRect.left-BoxSize, boundsRect.top);
		Line(0,-BoxSize);
		Line(maxmax,-maxmax);
		Line(BoxSize,0);
		Line(-maxmax,maxmax);
	CloseRgn(archie);

	do
	{
		StartTiming();
		OffsetRgn(archie,0,BoxSize);
		CopyBits(&(sourceGrafPtr->portBits),&(destGrafPtr->portBits),
			&boundsRect,&boundsRect,0,archie);
		TimeCorrection(CorrectTime);
	}
	while (!PtInRgn(zeroPoint, archie));
	
	DisposeRgn(archie);
	
	return 0;
}
