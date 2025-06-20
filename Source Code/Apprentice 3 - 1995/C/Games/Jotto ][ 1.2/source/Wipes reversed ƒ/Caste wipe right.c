#include "timing.h"

#define CorrectTime 1
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CasteWipeRight(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* This takes a strip (starting with the rightmost strip, moving left) and
   copies it into all the strips starting at the left and moving right until
   it's in the right place. */
   
pascal short CasteWipeRight(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			srcx, barpos;
	Rect			src, dest;
	Boolean			everyOther;
	short			gap;
	
	gap=theWindowWidth/50;
	everyOther=FALSE;
	src.top = boundsRect.top;
	src.bottom = boundsRect.bottom;
	
	for(srcx = boundsRect.right-gap; srcx >= boundsRect.left; srcx -= gap)
	{
		for(barpos = boundsRect.right; barpos >= boundsRect.left+gap; barpos -= gap) {}
		for(; barpos <= srcx; barpos += gap)
		{
			StartTiming();
			src.left = srcx;
			src.right = srcx + gap;
			dest = src;
			dest.left = barpos;
			dest.right = barpos + gap;
			CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
					&src, &dest, 0, 0L);
			if (everyOther)                        /* really, we need time */
				TimeCorrection(CorrectTime);       /* correction 0.5, but  */
			everyOther=!everyOther;                /* this will do (gag)   */
		}
	}
	
	return 0;
}
