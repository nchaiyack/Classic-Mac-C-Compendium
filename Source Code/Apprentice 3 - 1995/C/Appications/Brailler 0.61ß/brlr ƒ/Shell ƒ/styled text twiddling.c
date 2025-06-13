#include "styled text twiddling.h"
#include "text twiddling.h"
#include "window layer.h"

pascal void StyledScrollActionProc(ControlHandle theHandle, short partCode)
{
	short			scrollDistance;
	TEHandle		hTE;
	WindowPtr		theWindow;
	
	theWindow=(**theHandle).contrlOwner;
	if (theWindow==0L)
		return;
	
	hTE=GetWindowTE(theWindow);
	if (hTE==0L)
		return;
	
	switch (partCode)
	{
		case inUpButton:
		case inDownButton:
			scrollDistance=10;
			break;
		case inPageUp:
		case inPageDown:
			scrollDistance=(**hTE).viewRect.bottom-(**hTE).viewRect.top-10;
			break;
		default:
			scrollDistance=0;
			break;
	}
	
	if ((partCode==inDownButton) || (partCode==inPageDown))
		scrollDistance=-scrollDistance;
	
	MyMoveScrollBox(theHandle, scrollDistance);
	
	if (scrollDistance!=0)
	{
		TEPinScroll(0, scrollDistance, hTE);
	}
}

void StyledAdjustVScrollBar(ControlHandle theControl, TEHandle hTE)
{
	short			max, value;
	
	max=TEGetHeight(1, (**hTE).nLines, hTE)-((**hTE).viewRect.bottom-(**hTE).viewRect.top);
	if (max<0)
		max=0;
	SetControlMaximum(theControl, max);
	value=(**hTE).viewRect.top-(**hTE).destRect.top;
	if (value<0)
		value=0;
	else if (value>max)
		value=max;
	SetControlValue(theControl, value);
}

void StyledAdjustForEndScroll(ControlHandle theControl, TEHandle hTE)
{
	short			numLines;
	short			offset;
	
	if (hTE==0L)
		return;
	
	numLines=TEGetHeight(1, (**hTE).nLines, hTE);
	offset=numLines-GetControlMaximum(theControl)-
		(((**hTE).viewRect.bottom-(**hTE).viewRect.top));
	if (offset<0)
		TEPinScroll(0, -offset, hTE);
}

void StyledTrackThumb(WindowPtr theWindow, Point thePoint, ControlHandle theControl)
{
	TEHandle		hTE;
	Point			mouseLoc;
	Rect			theControlRect;
	unsigned short	value, max, min;
	short			newValue;
	short			scrollDistance;
	unsigned short	pixelRange;
	unsigned short	valueRange;
	long			pixelOffset;
	long			pixelTweak;
	long			temp;
	Boolean			isVertical;
	
	hTE=GetWindowTE(theWindow);
	theControlRect=(**theControl).contrlRect;
	isVertical=(theControlRect.bottom-theControlRect.top)>(theControlRect.right-theControlRect.left);
	min=GetControlMinimum(theControl);
	max=GetControlMaximum(theControl);
	if (isVertical)
	{
		pixelRange=theControlRect.bottom-theControlRect.top-3*(theControlRect.right-theControlRect.left);
		pixelTweak=-theControlRect.top-(theControlRect.right-theControlRect.left)*3/2;
	}
	else
	{
		pixelRange=theControlRect.right-theControlRect.left-3*(theControlRect.bottom-theControlRect.top);
		pixelTweak=-theControlRect.left-(theControlRect.bottom-theControlRect.top)*3/2;
	}
	valueRange=max-min;
	SetPort(theWindow);
	
	while (StillDown())
	{
		value=GetControlValue(theControl);
		GetMouse(&mouseLoc);
		if (isVertical)
			pixelOffset=mouseLoc.v;
		else
			pixelOffset=mouseLoc.h;
		pixelOffset+=pixelTweak;
		temp=valueRange;
		newValue=Fix2Long(FixMul(Long2Fix(pixelOffset), FixRatio(temp, pixelRange)));
		if (newValue<0)
			newValue=0;
		if (newValue>max)
			newValue=max;
		scrollDistance=value-newValue;
		if (scrollDistance!=0)
		{
			SetControlValue(theControl, newValue);
			if (isVertical)
				TEPinScroll(0, scrollDistance, hTE);
			else
				TEPinScroll(scrollDistance, 0, hTE);
		}
	}
}

