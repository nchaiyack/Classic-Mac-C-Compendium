#include "timing.h"

#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)
#define CorrectTime 3

pascal short BoxInWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* Basically, there are four bars -- one starts at the top and moves down;
   one starts at the bottom and moves up; one starts at the left and moves
   right; one starts at the right and moves left.  There's a lot of overlap
   of bitcopying, but it's masked by the timing correction */
   
pascal short BoxInWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	Rect		vsource1,vsource2, hsource1, hsource2;
	short			vbar,hbar;
	short			HBarGap,VBarGap;
	
	VBarGap=theWindowWidth/50;
	HBarGap=theWindowHeight/50;
	vbar=boundsRect.left;
	hbar=boundsRect.top;
	vsource1.top=vsource2.top=boundsRect.top;
	vsource1.bottom=vsource2.bottom=boundsRect.bottom;
	hsource1.left=hsource2.left=boundsRect.left;
	hsource1.right=hsource2.right=boundsRect.right;
	while (vbar<boundsRect.left+theWindowWidth/2+VBarGap)
	{
		StartTiming();
		vsource1.left=vbar;
		vsource1.right=vsource1.left+VBarGap;
		vsource2.right=2*boundsRect.left+theWindowWidth-vbar;
		vsource2.left=vsource2.right-VBarGap;
		hsource1.top=hbar;
		hsource1.bottom=hsource1.top+HBarGap;
		hsource2.bottom=2*boundsRect.top+theWindowHeight-hbar;
		hsource2.top=hsource2.bottom-HBarGap;
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&vsource1, &vsource1, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&hsource1, &hsource1, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&vsource2, &vsource2, 0, 0L);
		CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
			&hsource2, &hsource2, 0, 0L);
		vbar+=VBarGap;
		hbar+=HBarGap;
		TimeCorrection(CorrectTime);
	}
	
	return 0;
}
