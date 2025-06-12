/*
TitleBarHeight.c
Returns the height of a window's title bar, in pixels.

HISTORY:
12/29/91 dgp extracted from Zoom.c
*/
#include "VideoToolbox.h"

int TitleBarHeight(WindowPtr window)
{
	WindowPtr oldPort;
	int headRoom;
	Point pt;
	
	GetPort(&oldPort);
	SetPort(window);
	pt.v=window->portRect.top;
	LocalToGlobal(&pt);
	headRoom=pt.v-1-(*((WindowPeek)window)->strucRgn)->rgnBBox.top;
	SetPort(oldPort);
	return headRoom;
}