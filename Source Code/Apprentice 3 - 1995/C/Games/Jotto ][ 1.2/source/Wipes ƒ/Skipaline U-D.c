#include "timing.h"

#define CorrectTime 1
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short Skipaline(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Copy even-numbered rows starting at the top and moving down, and copy odd-
   numbered rows starting at the bottom and moving up. */
   
pascal short Skipaline(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect		thisone,thatone;
	
	SetRect(&thisone, boundsRect.left, boundsRect.top, boundsRect.right, boundsRect.top+1);
	SetRect(&thatone, boundsRect.left, boundsRect.bottom-1, boundsRect.right, boundsRect.bottom);
	if (theWindowHeight%2)
		OffsetRect(&thatone, 0, -1);
	
	while (thisone.top<boundsRect.bottom)
	{
		StartTiming();
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&thisone, &thisone, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&thatone, &thatone, 0, 0L);
		thisone.top+=2;             /* even row goes down by 2 */
		thisone.bottom+=2;
		thatone.top-=2;             /* odd row goes up by 2 */
		thatone.bottom-=2;
		TimeCorrection(CorrectTime);
	}
	
	return 0;
}
