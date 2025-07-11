/* PPPop, verison 1.2  June 6, 1995  Rob Friefeld

   AdjustWindowPosition = make sure window is on the screen. Not for windows
   bigger than the screen.
*/

#include "PositionWindow.h"

void AdjustWindowPosition(WindowPtr window)
{	
	Rect		tempRect, bounds;
	Point		theTopLeft = {0,0};
	short		curTop, curLeft;
	short		slop = 0;

	if (window == nil) return;
		
	SetPort(window);
	GetScreenBounds(window, &bounds);

	tempRect = window->portRect;
	LocalToGlobal( &TopLeft(tempRect));
	LocalToGlobal( &BotRight(tempRect));
	
	if (tempRect.left < bounds.left) {
		tempRect.left = bounds.left + slop;
		MoveWindow(window, tempRect.left, tempRect.top, FALSE);
	}

	if (tempRect.top < bounds.top) {
		tempRect.top = bounds.top + slop;
		MoveWindow(window,tempRect.left,tempRect.top, FALSE);
	}

	tempRect = window->portRect;
	LocalToGlobal( &TopLeft(tempRect));
	LocalToGlobal( &BotRight(tempRect));

	if (tempRect.right > bounds.right) {
		tempRect.left = bounds.right -
				(tempRect.right - tempRect.left) - slop;
		MoveWindow(window,tempRect.left,tempRect.top,FALSE);
	}
	
	if (tempRect.bottom > bounds.bottom) {
		tempRect.top = bounds.bottom -
				(tempRect.bottom - tempRect.top) - slop;
		MoveWindow(window,tempRect.left,tempRect.top,FALSE);
	}
}
	





void GetScreenBounds(WindowPtr window, Rect *bounds)
{
	GDHandle	gdNthDevice, gdOnThisDevice;
	GrafPtr		savePort;
	Rect		windRect, theSect;
	long		sectArea, greatestArea;
	short		wTitleHeight;
	Boolean		sectFlag;
	OSErr		err;
	long		feature;

	if (!gHasColorQD) {		// only one screen possible
		*bounds = qd.screenBits.bounds;
		return;
	}

	GetPort(&savePort);
	SetPort(window);
	windRect = window->portRect;
	LocalToGlobal( &TopLeft(windRect));
	LocalToGlobal( &BotRight(windRect));
	wTitleHeight = windRect.top - 1 - (**(((WindowPeek)window)->strucRgn)).rgnBBox.top;
	windRect.top = windRect.top - wTitleHeight;

	gdNthDevice = GetDeviceList();
	gdOnThisDevice = gdNthDevice;		// in case window is nowhere
	greatestArea = 0;

	while (gdNthDevice != nil) {
		if (TestDeviceAttribute(gdNthDevice, screenDevice) &&
				TestDeviceAttribute(gdNthDevice, screenActive)) {
			sectFlag = SectRect(&windRect, &((**gdNthDevice).gdRect), &theSect);
			sectArea = (long)(theSect.right - theSect.left) * 
				(long)(theSect.bottom - theSect.top);
			if (sectArea > greatestArea) {
				greatestArea = sectArea;
				gdOnThisDevice = gdNthDevice;
			}
		}
		gdNthDevice = GetNextDevice(gdNthDevice);
	}

	*bounds = (**gdOnThisDevice).gdRect;

	SetPort(savePort);
}

	