#include "timing.h"

#define CorrectTime 1
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short SkipalineLR2PassReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Copy even-numbered columns starting at the left and moving right, and odd-
   numbered columns starting at the right and moving left.  */
   
pascal short SkipalineLR2PassReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect		thisone,thatone;
	Boolean		everyOther=FALSE;
	
	SetRect(&thisone, boundsRect.left, boundsRect.top, boundsRect.left+1, boundsRect.bottom);
	SetRect(&thatone, boundsRect.right-1, boundsRect.top, boundsRect.right, boundsRect.bottom);
	if (theWindowWidth%2)
		OffsetRect(&thatone, -1, 0);
	
	while (thatone.right>=boundsRect.left)
	{
		StartTiming();
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&thatone, &thatone, 0, 0L);
		thatone.left-=2;      /* right column moves left by 2 */
		thatone.right-=2;
		if (everyOther)
			TimeCorrection(CorrectTime);
		everyOther=!everyOther;
	}
	
	while (thisone.left<boundsRect.right)
	{
		StartTiming();
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&thisone, &thisone, 0, 0L);
		thisone.left+=2;      /* left column moves right by 2 */
		thisone.right+=2;
		if (everyOther)
			TimeCorrection(CorrectTime);
		everyOther=!everyOther;
	}
	
	return 0;
}
