#include "timing.h"

#define CorrectTime 4
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short HGR2(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Copy even-numbered rows starting at the top and moving down, and copy odd-
   numbered rows starting at the bottom and moving up. */
   
pascal short HGR2(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect			thisone;
	short				i;
	short				numCols;
	
	numCols=theWindowWidth/24;
	for (i=0; i<numCols; i++)
	{
		StartTiming();
		
		SetRect(&thisone, boundsRect.left+i, boundsRect.top, boundsRect.left+i+1,
			boundsRect.bottom);
		
		while (thisone.left<boundsRect.right)
		{
			CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&thisone, &thisone, 0, 0L);
			thisone.left+=numCols;
			thisone.right+=numCols;
		}
		TimeCorrection(CorrectTime);
	}
	
	return 0;
}
