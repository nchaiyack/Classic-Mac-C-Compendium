#include "flight.h"

extern	WindowPtr		theWind;	/*the main window*/

BitMap	oldMap;
GrafPort	offPort;


SetBack()
{
	SetPort(&offPort);
}

MkPort(theWindow)
WindowPtr	theWindow;
{
	Rect	windRect;
	QDPtr	newSpace;
	int		rwBytes;

	windRect = theWindow->portRect;
	windRect.bottom = windRect.bottom - windRect.top;
	windRect.top = 0;
	windRect.right = windRect.right - windRect.left;
	windRect.left = 0;

	OpenPort(&offPort);
	offPort.portRect = windRect;
	offPort.portBits.bounds = windRect;
	SetRectRgn(offPort.visRgn, 0, 0, windRect.right, windRect.bottom);
	SetRectRgn(offPort.clipRgn, 0, 0, windRect.right, windRect.bottom);

	rwBytes = ((windRect.right + 15) / 16) * 2;
	newSpace = NewPtr((long)rwBytes * windRect.bottom);
	if (MemErr)
		return;

	offPort.portBits.rowBytes = rwBytes;
	offPort.portBits.baseAddr = newSpace;
	SetPort(&offPort);
	EraseRect(&offPort.portRect);
}

GetWind(theWindow)
WindowPtr	theWindow;
{
	CopyBits(&offPort.portBits, &theWindow->portBits,
			&offPort.portRect, &theWindow->portRect,
			srcCopy, 0L);
}
/*	rowBytes = ((width + 15) / 16) * 2;*/
