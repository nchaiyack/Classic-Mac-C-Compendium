#include "timing.h"

static	long		oldTicks;

pascal void StartTiming(void)
{
	oldTicks=TickCount();
}

pascal void TimeCorrection(short tickCount)
{
	long		newTicks;
	
	newTicks=TickCount();
	while (newTicks-oldTicks<tickCount)
		Delay(1L, &newTicks);
}

pascal void ScrollTheRect(Rect *r, short dh, short dv, RgnHandle unused)
{
	Rect			theDestRect;
	GrafPtr			thePort;
	RgnHandle		temprgn;
	
	SetRect(&theDestRect, (*r).left+dh, (*r).top+dv, (*r).right+dh, (*r).bottom+dv);
	temprgn=NewRgn();
	RectRgn(temprgn, r);
	GetPort(&thePort);
	CopyBits(&(thePort->portBits), &(thePort->portBits), r, &theDestRect, 0, temprgn);
	DisposeRgn(temprgn);
}
