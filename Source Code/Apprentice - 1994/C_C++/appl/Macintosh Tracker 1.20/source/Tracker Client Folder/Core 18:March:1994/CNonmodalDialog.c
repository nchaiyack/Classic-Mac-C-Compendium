/* CNonmodalDialog.c */

#include "CNonmodalDialog.h"


void		CNonmodalDialog::MakeMyGrafPtr(LongPoint Start, LongPoint Extent)
	{
		Rect		BoundsRect;

		BoundsRect = RectOf(Start.x,Start.y,Start.x+Extent.x,Start.y+Extent.y);
		MyGrafPtr = NewWindow(NIL,&BoundsRect,"\p",True,documentProc,(void*)-1,
			False,(long)this);
		if (MyGrafPtr == NIL)
			{
				PRERR(ForceAbort,"NewWindow failed to allocate memory.");
			}
	}
