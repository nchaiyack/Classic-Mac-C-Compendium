#include "timing.h"

#define CorrectTime 1
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short CasteWipeDown(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* This takes a strip (starting with the bottommost strip, moving up) and copies
   it into all the strips starting with the top and moving down until it's in
   the right place. */
   
pascal short CasteWipeDown(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			srcy, barpos;
	Rect			src, dest;
	Boolean			everyOther;
	short			gap;
	
	gap=theWindowHeight/50;
	everyOther=FALSE;
	src.left = boundsRect.left;
	src.right = boundsRect.right;
	
	for(srcy = boundsRect.bottom-gap; srcy >= boundsRect.top; srcy -= gap)
	{
		for(barpos = boundsRect.bottom; barpos >= boundsRect.top+gap; barpos -= gap) {}
		for(; barpos <= srcy; barpos += gap)
		{
			StartTiming();
			src.top = srcy;
			src.bottom = srcy + gap;
			dest = src;
			dest.top = barpos;
			dest.bottom = barpos + gap;
			CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
					&src, &dest, 0, 0L);
			if (everyOther)                      /* simulates time correction 0.5 */
				TimeCorrection(CorrectTime);
			everyOther=!everyOther;
		}
	}
	
	return 0;
}
