#include "ep.const.h"
#include "ep.extern.h"

long	oldBackground	= whiteColor;
long	oldFore			= blackColor;

eUpdate (Resized) /************************************************************/
Boolean	Resized;
{
	Rect	tr;

	if (Resized) {
		ClipRect(&eWindow->portRect);
		EraseRect(&eWindow->portRect);
	}
	DrawPicture(ebmPict,&eWindow->portRect);
	myDrawGrowIcon();
}

eClose() /************************************************************/
{
	SkelWhoa ();
}

eClobber() /************************************************************/
{
GrafPtr	thePort;

	GetPort (&thePort);				/* grafport of window to dispose*/
	DisposeWindow ((WindowPtr) thePort);
}

eActivate() /************************************************************/
{
}

eMouse() /************************************************************/
{
}

eKey() /************************************************************/
{
}

myDrawGrowIcon()
{
	Rect	tr;

	tr.bottom = eWindow->portRect.bottom - 2;
	tr.right  = eWindow->portRect.right  - 2;
	tr.top    = eWindow->portRect.bottom - 11;
	tr.left   = eWindow->portRect.right  - 11;

	BackColor(oldBackground);
	ForeColor(oldFore);
	FrameRect(&tr);
	
	OffsetRect(&tr,-3,-3);
	InsetRect (&tr, 1, 1);
	EraseRect(&tr);
	FrameRect(&tr);
}