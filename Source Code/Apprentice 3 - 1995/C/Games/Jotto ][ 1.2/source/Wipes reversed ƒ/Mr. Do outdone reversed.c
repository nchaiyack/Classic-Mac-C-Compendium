#include "timing.h"

#define	BoxSize	2
#define CorrectTime 3
#define theWindowHeight (boundsRect.bottom-boundsRect.top)
#define theWindowWidth (boundsRect.right-boundsRect.left)

pascal short MrDoOutdoneReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);

/* 25 regions on screen, in a 5 x 5 grid.  Regions alternate as to whether they
   scroll up or down. */
   
pascal short MrDoOutdoneReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect)
{
	short			x, y;
	short			vgap,hgap;
	Rect		theRect, dest;
	Rect		bounds[25];
	
	vgap=theWindowHeight/5;
	hgap=theWindowWidth/5;
	
	for (x=0; x<25; x++)
	{
		switch (x)
		{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
				bounds[x].top=0;
				break;
			case 15:
			case 16:
			case 17:
			case 18:
			case 5:
				bounds[x].top=vgap;
				break;
			case 14:
			case 23:
			case 24:
			case 19:
			case 6:
				bounds[x].top=vgap*2;
				break;
			case 13:
			case 22:
			case 21:
			case 20:
			case 7:
				bounds[x].top=vgap*3;
				break;
			case 12:
			case 11:
			case 10:
			case 9:
			case 8:
				bounds[x].top=vgap*4;
				break;
		}
		switch (x)
		{
			case 0:
			case 15:
			case 14:
			case 13:
			case 12:
				bounds[x].left=0;
				break;
			case 1:
			case 16:
			case 23:
			case 22:
			case 11:
				bounds[x].left=hgap;
				break;
			case 2:
			case 17:
			case 24:
			case 21:
			case 10:
				bounds[x].left=hgap*2;
				break;
			case 3:
			case 18:
			case 19:
			case 20:
			case 9:
				bounds[x].left=hgap*3;
				break;
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				bounds[x].left=hgap*4;
				break;
		}
		bounds[x].bottom=bounds[x].top+vgap;
		bounds[x].right=bounds[x].left+hgap;
		OffsetRect(&(bounds[x]), boundsRect.left, boundsRect.top);
	}
	
	for (x=BoxSize; x<vgap; x+=BoxSize)
	{		
		StartTiming();
		for (y=0; y<25; y++)
		{
			if (!(y%2))   /* these scroll up */
			{
				dest=bounds[y];
				dest.top=dest.bottom-BoxSize;
				
				theRect=bounds[y];
				theRect.top+=x-BoxSize;
				theRect.bottom=theRect.top+BoxSize;
			
				ScrollTheRect(&bounds[y], 0, -BoxSize, 0L);
				CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
						&theRect, &dest, 0, 0L);
			}
			else    /* these scroll down */
			{
				dest=bounds[y];
				dest.bottom=dest.top+BoxSize;
				
				theRect=bounds[y];
				theRect.bottom-=x-BoxSize;
				theRect.top=theRect.bottom-BoxSize;
				
				ScrollTheRect(&bounds[y], 0, BoxSize, 0L);
				CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
						&theRect, &dest, 0, 0L);
			}
		}
		TimeCorrection(CorrectTime);
	}
	
	CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
		&boundsRect, &boundsRect, 0, 0L);		/* in case we missed any */
	
	return 0;
}
