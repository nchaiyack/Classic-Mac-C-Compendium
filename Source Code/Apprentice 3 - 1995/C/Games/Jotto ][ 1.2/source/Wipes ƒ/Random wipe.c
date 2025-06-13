#include "timing.h"

#define		SUB_HOR		20
#define		SUB_VER		20
#define		AREA		(SUB_HOR * SUB_VER)
#define CorrectTime 1
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define theWindowHeight (boundsRect.bottom-boundsRect.top)

pascal short RandomWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Basically, we divide the window into a bunch of blocks, and copy
each to the screen in random order. */
pascal short RandomWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			order[AREA];
	short			i;
	long			randtemp;
	short			ordertemp;
	Rect			subBox;
	Boolean			everyOther;
	
	everyOther=FALSE;
	for(i = 0; i < AREA; i++)
		order[i] = i;
	
	for(i = (AREA - 1); i >= 0; i--) {
		randtemp = ((((long)Random()) +32767) * (i + 1)) / 65535;
		
		ordertemp = order[randtemp];
		order[randtemp] = order[i];
		order[i] = ordertemp;
	}
	
	for(i = 0; i < AREA; i++) {
		StartTiming();
		subBox.top = ((order[i] / SUB_VER) * theWindowHeight) / SUB_VER;
		subBox.left = ((order[i] % SUB_HOR) * theWindowWidth) / SUB_HOR;
		subBox.bottom = (((order[i] / SUB_VER) + 1) * theWindowHeight) / SUB_VER;
		subBox.right = (((order[i] % SUB_HOR) + 1) * theWindowWidth) / SUB_HOR;
		OffsetRect(&subBox, boundsRect.left, boundsRect.top);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&subBox, &subBox, 0, 0L);
		if (everyOther)
			TimeCorrection(CorrectTime);
		everyOther=!everyOther;
	}
	
	return 0;
}
