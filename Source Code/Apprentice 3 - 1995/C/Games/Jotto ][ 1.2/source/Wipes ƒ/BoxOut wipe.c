#include "timing.h"

#define		BOXES	50	/* increase this for a finer grained but more timely effect */
#define CorrectTime 2
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short BoxOutWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

pascal short BoxOutWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect	theRect;
	short		cenx, ceny;
	short		boxnum;
	
	cenx = theWindowWidth / 2;
	ceny = theWindowHeight / 2;
	
	for(boxnum = 0; boxnum <= BOXES; boxnum++)
	{
		StartTiming();
		
		/* this is not the most efficient method, but the overhead of the
		multiplies and the redundant copying is hidden by the timing mechanism */
		theRect.left = cenx - ((boxnum * cenx) / BOXES);
		theRect.right = cenx + ((boxnum * cenx) / BOXES);
		theRect.top = ceny - ((boxnum * ceny) / BOXES);
		theRect.bottom = ceny + ((boxnum * ceny) / BOXES);
		OffsetRect(&theRect, boundsRect.left, boundsRect.top);
		
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&theRect, &theRect, 0, 0L);
		TimeCorrection(CorrectTime);
	}
	
	return 0;
}
