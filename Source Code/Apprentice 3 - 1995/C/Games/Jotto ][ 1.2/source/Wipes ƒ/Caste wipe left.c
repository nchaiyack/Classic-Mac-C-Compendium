#include "timing.h"

#define CorrectTime 1
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CasteWipeRL(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* This takes a strip (starting with the leftmost strip, moving right) and
   copies it into all the strips starting at the right and moving left until
   its in the right place. */
   
pascal short CasteWipeRL(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			srcx, barpos;
	Rect			src, dest;
	Boolean			everyOther;
	short			gap;
	
	gap=theWindowWidth/50;
	everyOther=FALSE;
	src.top = boundsRect.top;
	src.bottom = boundsRect.bottom;
	
	for(srcx = boundsRect.left; srcx < boundsRect.right; srcx += gap)
	{
		for(barpos = boundsRect.left; barpos + gap < boundsRect.right; barpos += gap) {}
		for(; barpos >= srcx; barpos -= gap)
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
