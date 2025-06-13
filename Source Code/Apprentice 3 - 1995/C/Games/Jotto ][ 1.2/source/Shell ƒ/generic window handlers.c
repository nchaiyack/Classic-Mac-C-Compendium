#include "generic window handlers.h"
#include "text twiddling.h"
#include "window layer.h"
#include "main.h"

#define kGrowBoxSize	15

enum { key_LeftArrow=0x1c, key_RightArrow, key_UpArrow, key_DownArrow };
enum { key_PageUp=0x0b, key_PageDown };
enum { key_Home=0x01 };
enum { key_End=0x04 };

extern	Boolean			gInProgress;	/* see environment.h */
extern	Boolean			gCustomCursor;

void GenericCopybits(WindowPtr theWindow, WindowPtr offscreenWindowPtr, Boolean active)
{
	Rect			tempRect;
	
	if (active)
	{
		DrawGrowIcon(theWindow);
    }
	else
	{
		tempRect.bottom=theWindow->portRect.bottom;
		tempRect.right=theWindow->portRect.right;
		tempRect.left=tempRect.right-kGrowBoxSize+1;
		tempRect.top=tempRect.bottom-kGrowBoxSize+1;
		EraseRect(&tempRect);
	}
	
	UpdateControls(theWindow, theWindow->visRgn);
	
	tempRect=theWindow->portRect;
	tempRect.bottom-=kGrowBoxSize;
	tempRect.right-=kGrowBoxSize;
	CopyBits(	&(offscreenWindowPtr->portBits),
				&(theWindow->portBits),
				&tempRect, &tempRect, 0, 0L);
}

void GenericResizeControls(WindowPtr theWindow)
{
	TEHandle		hTE;
	ControlHandle	vScrollBar, hScrollBar;
	
	hTE=GetWindowTE(theWindow);
	vScrollBar=GetWindowVScrollBar(theWindow);
	hScrollBar=GetWindowHScrollBar(theWindow);
	AdjustScrollSizes(theWindow, hTE, vScrollBar, hScrollBar);
	AdjustViewRect(hTE);
	TECalText(hTE);
	AdjustForEndScroll(vScrollBar, hTE);
	AdjustVScrollBar(vScrollBar, hTE);
}

void GenericGetGrowSize(WindowPtr theWindow, Rect *sizeRect)
{
	SetRect(sizeRect, 200, 48+kGrowBoxSize+1, 32766, 32767);
}

Boolean GenericKeyPressedInWindow(WindowPtr theWindow, unsigned char theChar)
{
	TEHandle		hTE;
	ControlHandle	vScrollBar;
	
	if (gInProgress)
		return TRUE;
	
	hTE=GetWindowTE(theWindow);
	vScrollBar=GetWindowVScrollBar(theWindow);
	
	switch (theChar)
	{
		case key_PageUp:
			ScrollActionProc(vScrollBar, inPageUp);
			break;
		case key_PageDown:
			ScrollActionProc(vScrollBar, inPageDown);
			break;
		case key_Home:
			TEPinScroll(0, TEGetHeight((**hTE).nLines, 1, hTE), hTE);
			break;
		case key_End:
			TEPinScroll(0, -TEGetHeight((**hTE).nLines, 1, hTE), hTE);
			break;
		default:
			return FALSE;
			break;
	}
	
	AdjustVScrollBar(vScrollBar, hTE);
	
	return TRUE;
}

Boolean GenericMouseClickedInWindow(WindowPtr theWindow, Point thePoint, Boolean isEditable)
{
	short			partCode;
	ControlHandle	theControl;
	short			scrollDistance;
	short			oldSetting;
	ControlActionUPP	scrollActionUPP=NewControlActionProc(ScrollActionProc);
	TEHandle		hTE;
	
	if (gInProgress)
		return TRUE;
	
	hTE=GetWindowTE(theWindow);
	
	if (PtInRect(thePoint, &((**hTE).viewRect)))
	{
		TEClick(thePoint, (GetTheModifiers()&512) ? TRUE : FALSE, hTE);
		return TRUE;
	}
	else
	{
		partCode=FindControl(thePoint, theWindow, &theControl);
		if (theControl==GetWindowVScrollBar(theWindow))
		{
			switch (partCode)
			{
				case inThumb:
					oldSetting=GetControlValue(theControl);
					partCode=TrackControl(theControl, thePoint, 0L);
					if (partCode==inThumb)
					{
						scrollDistance=oldSetting-GetControlValue(theControl);
						if (scrollDistance!=0)
						{
							hTE=GetWindowTE(theWindow);
							TEPinScroll(0, scrollDistance*(**hTE).lineHeight, hTE);
						}
					}
					break;
				case inUpButton:
				case inDownButton:
				case inPageUp:
				case inPageDown:
					partCode=TrackControl(theControl, thePoint, scrollActionUPP);
					break;
			}
			
			return TRUE;
		}
	}
	
	return FALSE;
}

void GenericActivate(WindowPtr theWindow)
{
	GrafPtr			curPort;
	
	GetPort(&curPort);
	SetPort(theWindow);
	TEActivate(GetWindowTE(theWindow));
	HiliteControl(GetWindowVScrollBar(theWindow), 0);
	HiliteControl(GetWindowHScrollBar(theWindow), 0);
	DrawGrowIcon(theWindow);
	UpdateControls(theWindow, theWindow->visRgn);
	SetPort(curPort);
}

void GenericDeactivate(WindowPtr theWindow)
{
	Rect			tempRect;
	GrafPtr			curPort;
	
	GetPort(&curPort);
	SetPort(theWindow);
	TEDeactivate(GetWindowTE(theWindow));
	gCustomCursor=FALSE;
	HiliteControl(GetWindowVScrollBar(theWindow), 255);
	HiliteControl(GetWindowHScrollBar(theWindow), 255);
	tempRect.bottom=theWindow->portRect.bottom;
	tempRect.right=theWindow->portRect.right;
	tempRect.left=tempRect.right-kGrowBoxSize+1;
	tempRect.top=tempRect.bottom-kGrowBoxSize+1;
	EraseRect(&tempRect);
	UpdateControls(theWindow, theWindow->visRgn);
	SetPort(curPort);
}
