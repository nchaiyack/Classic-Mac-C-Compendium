#include "timing.h"

#define CorrectTime 4
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short HGRReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Copy even-numbered rows starting at the top and moving down, and copy odd-
   numbered rows starting at the bottom and moving up. */
   
pascal short HGRReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect			thisone;
	short				i;
	short				numRows;
	
	numRows=theWindowHeight/24;
	for (i=numRows-1; i>=0; i--)
	{
		StartTiming();
		
		SetRect(&thisone, boundsRect.left, boundsRect.top+i, boundsRect.right,
			boundsRect.top+i+1);
		
		while (thisone.top<boundsRect.bottom)
		{
			CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&thisone, &thisone, 0, 0L);
			thisone.top+=numRows;
			thisone.bottom+=numRows;
		}
		TimeCorrection(CorrectTime);
	}
	
	return 0;
}
