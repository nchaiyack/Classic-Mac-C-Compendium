#include "timing.h"
#include "hgr.h"

#define CorrectTime 4
#define theWindowWidth (boundsRect->right-boundsRect->left)
#define theWindowHeight (boundsRect->bottom-boundsRect->top)

void HGRFade(Rect *boundsRect)
{
	Rect			thisone;
	int				i;
	int				numRows;
	
	numRows=theWindowHeight/24;
	for (i=0; i<numRows; i++)
	{
		StartTiming();
		
		SetRect(&thisone, boundsRect->left, boundsRect->top+i, boundsRect->right,
			boundsRect->top+i+1);
		
		while (thisone.top<boundsRect->bottom)
		{
			FillRect(&thisone, &qd.black);
			thisone.top+=numRows;
			thisone.bottom+=numRows;
		}
		TimeCorrection(CorrectTime);
	}
}

void HGRReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect *boundsRect)
{
	Rect			thisone;
	int				i;
	int				numRows;
	
	numRows=theWindowHeight/24;
	for (i=numRows-1; i>=0; i--)
	{
		StartTiming();
		
		SetRect(&thisone, boundsRect->left, boundsRect->top+i, boundsRect->right,
			boundsRect->top+i+1);
		
		while (thisone.top<boundsRect->bottom)
		{
			CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits),
				&thisone, &thisone, 0, 0L);
			thisone.top+=numRows;
			thisone.bottom+=numRows;
		}
		TimeCorrection(CorrectTime);
	}
}
