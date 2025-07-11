#include "timing.h"

#define CorrectTime 1
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short SlideWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Split the screen into 10 sections, then scroll each section left or right
   (alternating), starting with the top section and working towards the bottom. */
   
pascal short SlideWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			x, y;
	Rect			theRect, dest;
	Rect			scrollsource;
	short			direction;
	short			BoxSize, StripSize;

	BoxSize=theWindowWidth/25;
	StripSize=theWindowHeight/10;
	
	direction = 0;
	for(y = 0; y < theWindowHeight; y += StripSize)
	{
		scrollsource = boundsRect;
		scrollsource.top+=y;
		scrollsource.bottom = scrollsource.top + StripSize;
		
		dest = scrollsource;
		
		theRect.top = boundsRect.top + y;
		theRect.bottom = theRect.top + StripSize;
		
		if(direction == 0)
		{
			dest.right = dest.left + BoxSize;
			
			theRect.left = boundsRect.right-BoxSize;
			theRect.right = boundsRect.right;
			
			for(x = boundsRect.right - BoxSize; x >= boundsRect.left; x -= BoxSize)
			{
				StartTiming();
				theRect.left = x;
				ScrollTheRect(&scrollsource, BoxSize, 0, 0L);
				CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
						&theRect, &dest, 0, 0L);
				theRect.right = x;
				TimeCorrection(CorrectTime);
			}
			CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
					&scrollsource, &scrollsource, 0, 0L);
		}
		else
		{
			dest.left = dest.right - BoxSize;
			
			theRect.left = boundsRect.left;
			theRect.right = theRect.left + BoxSize;
			
			for(x = boundsRect.left+BoxSize; x <= boundsRect.right; x += BoxSize)
			{
				StartTiming();
				theRect.right = x;
				ScrollTheRect(&scrollsource, -BoxSize, 0, 0L);
				CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
						&theRect, &dest, 0, 0L);
				theRect.left = x;
				TimeCorrection(CorrectTime);
			}
			CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
					&scrollsource, &scrollsource, 0, 0L);
		}
		
		direction = 1 - direction;
	}
	
	return 0;
}
