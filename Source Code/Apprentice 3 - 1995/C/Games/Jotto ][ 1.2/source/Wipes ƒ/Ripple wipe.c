#include "timing.h"

#define CorrectTime 1
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short RippleWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Like the Caste wipe up, except the screen is split into 6 sections, and the
   caste wipe is performed in each section -- hence the ripple effect. */
   
pascal short RippleWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			bigy, littley, barpos;
	Rect			src, dest;
	short			BigRippleSize;
	short			gap;
	RgnHandle		boundsRgn;
	
	BigRippleSize=theWindowHeight/6;	/* used to be 52 */
	gap=BigRippleSize/6;				/* used to be 8 */
	boundsRgn=NewRgn();
	RectRgn(boundsRgn, &boundsRect);
	src.left = boundsRect.left;
	src.right = boundsRect.right;
	
	for(bigy = 0; bigy < theWindowHeight; bigy += BigRippleSize)
	{
		for(littley = bigy; littley < bigy + BigRippleSize; littley += gap)
		{
			for(barpos = bigy; barpos + gap < bigy + BigRippleSize; barpos += gap) {}
			for(; barpos >= littley; barpos -= gap)
			{
				StartTiming();
				src.top = boundsRect.top + littley;
				src.bottom = boundsRect.top + littley + gap;
				dest = src;
				dest.top = boundsRect.top + barpos;
				dest.bottom = dest.top + gap;
				CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
						&src, &dest, 0, boundsRgn);
				TimeCorrection(CorrectTime);
			}
		}
	}
	
	DisposeRgn(boundsRgn);
	
	return 0;
}
