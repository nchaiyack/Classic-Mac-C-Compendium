#include "generic window handlers.h"
#include "text layer.h"
#include "drag layer.h"
#include "window layer.h"
#include "key layer.h"
#include "main.h"
#include "program globals.h"

#if USE_STYLED_TEXT
#include "styled text layer.h"
#endif

#define kGrowBoxSize	15

extern	Boolean			gInProgress;	/* see environment.h */
extern	Boolean			gCustomCursor;

void GenericCopybits(WindowRef theWindow, WindowRef offscreenWindowRef, Boolean active,
	Boolean drawGrowIconLines)
{
	Rect			tempRect;
	
	if (active)
	{
		DrawGrowIconNoLines(theWindow, drawGrowIconLines);
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
	CopyBits(	&(offscreenWindowRef->portBits),
				&(theWindow->portBits),
				&tempRect, &tempRect, 0, 0L);
}

void GenericResizeControls(WindowRef theWindow, short destOverload, short headerHeight,
	Boolean isStyled)
{
#if !USE_STYLED_TEXT
	#pragma unused(isStyled)
#endif
	
	TEHandle		hTE;
	ControlHandle	vScrollBar, hScrollBar;
	
	hTE=GetWindowTE(theWindow);
	vScrollBar=GetWindowVScrollBar(theWindow);
	hScrollBar=GetWindowHScrollBar(theWindow);
	AdjustScrollSizes(theWindow, hTE, vScrollBar, hScrollBar, destOverload, headerHeight);
#if USE_STYLED_TEXT
	if (!isStyled)
#endif
		AdjustViewRect(hTE);
	TECalText(hTE);
#if USE_STYLED_TEXT
	if (isStyled)
	{
		StyledAdjustForEndScroll(vScrollBar, hTE);
		StyledAdjustVScrollBar(vScrollBar, hTE);
	}
	else
	{
#endif
		AdjustForEndScroll(vScrollBar, hTE);
		AdjustVScrollBar(vScrollBar, hTE);
#if USE_STYLED_TEXT
	}
#endif
	ResetHiliteRgn(theWindow);
}

void GenericGetGrowSize(WindowRef theWindow, Rect *sizeRect)
{
	#pragma unused(theWindow)

	SetRect(sizeRect, 200, 48+kGrowBoxSize+1, 32766, 32767);
}

void GenericIdleInWindow(WindowRef theWindow, Point mouseLoc)
{
	TEHandle		hTE;
	Boolean			inDragRgn;
	
	if (gInProgress)
		return;
	
	hTE=GetWindowTE(theWindow);
	if (hTE==0L)
		return;
	
	TEIdle(hTE);
	
	if (PtInRect(mouseLoc, &((**hTE).viewRect)))
	{
		if (WindowIsDraggableQQ(theWindow))
			inDragRgn=CursorInDraggableRgn(mouseLoc, theWindow);
		else
			inDragRgn=FALSE;
		
		if (inDragRgn)
		{
			gCustomCursor=FALSE;
		}
		else if (!gCustomCursor)
		{
			SetCursor(*GetCursor(iBeamCursor));
			gCustomCursor=TRUE;
		}
	}
	else
	{
		gCustomCursor=FALSE;
	}
}

Boolean GenericKeyPressedInWindow(WindowRef theWindow, unsigned char theChar, Boolean isEditable,
	Boolean isStyled)
{
#if !USE_STYLED_TEXT
	#pragma unused(isStyled)
#endif
	
	TEHandle		hTE;
	ControlHandle	vScrollBar;
	
	if (gInProgress)
	{
		gInProgress=FALSE;
		return TRUE;
	}
	
	if (isEditable)
		ObscureCursor();
	hTE=GetWindowTE(theWindow);
	vScrollBar=GetWindowVScrollBar(theWindow);
	
	switch (theChar)
	{
		case key_PageUp:
#if USE_STYLED_TEXT
			if (isStyled)
				StyledScrollActionProc(vScrollBar, inPageUp);
			else
#endif
				ScrollActionProc(vScrollBar, inPageUp);
			break;
		case key_PageDown:
#if USE_STYLED_TEXT
			if (isStyled)
				StyledScrollActionProc(vScrollBar, inPageDown);
			else
#endif
				ScrollActionProc(vScrollBar, inPageDown);
			break;
		case key_Home:
			TEPinScroll(0, TEGetHeight((**hTE).nLines, 1, hTE), hTE);
			break;
		case key_End:
			TEPinScroll(0, -TEGetHeight((**hTE).nLines, 1, hTE), hTE);
			break;
		case key_UpArrow:
		case key_DownArrow:
		case key_LeftArrow:
		case key_RightArrow:
			if ((isEditable) && (ShiftKeyWasDown()))
			{
				HandleShiftArrow(hTE, theChar);
				ResetHiliteRgn(theWindow);
				SetWindowLastFindPosition(theWindow, (**hTE).selStart);
				break;
			}
			/* no break here intentionally */
		default:
			return FALSE;
			break;
	}
	
#if USE_STYLED_TEXT
	if (isStyled)
		StyledAdjustVScrollBar(vScrollBar, hTE);
	else
#endif
		AdjustVScrollBar(vScrollBar, hTE);
	
	return TRUE;
}

Boolean GenericMouseClickedInWindow(WindowRef theWindow, Point thePoint, Boolean isEditable,
	Boolean dynamicScroll, Boolean isStyled)
{
#if !USE_STYLED_TEXT
	#pragma unused(isStyled)
#endif
	
	short			partCode;
	ControlHandle	theControl;
	short			scrollDistance;
	short			oldSetting;
	ControlActionUPP	scrollActionUPP;
	
	TEHandle		hTE;
	
	if (gInProgress)
		return TRUE;
	
	hTE=GetWindowTE(theWindow);
	
	if ((PtInRect(thePoint, &((**hTE).viewRect))) && (isEditable))
	{
		TEClick(thePoint, ShiftKeyWasDown(), hTE);
		ResetHiliteRgn(theWindow);
		SetWindowLastFindPosition(theWindow, SelectionStart(theWindow));
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
					if (dynamicScroll)
					{
#if USE_STYLED_TEXT
						if (isStyled)
							StyledTrackThumb(theWindow, thePoint, theControl);
						else
#endif
							MyTrackThumb(theWindow, thePoint, theControl);
					}
					else
					{
						oldSetting=GetControlValue(theControl);
						partCode=TrackControl(theControl, thePoint, 0L);
						if (partCode==inThumb)
						{
							scrollDistance=oldSetting-GetControlValue(theControl);
							if (scrollDistance!=0)
							{
								hTE=GetWindowTE(theWindow);
#if USE_STYLED_TEXT
								if (isStyled)
									TEPinScroll(0, scrollDistance, hTE);
								else
#endif
									TEPinScroll(0, scrollDistance*(**hTE).lineHeight, hTE);
							}
						}
					}
					break;
				case inUpButton:
				case inDownButton:
				case inPageUp:
				case inPageDown:
#if USE_STYLED_TEXT
					if (isStyled)
						scrollActionUPP=NewControlActionProc(StyledScrollActionProc);
					else
#endif
						scrollActionUPP=NewControlActionProc(ScrollActionProc);
					partCode=TrackControl(theControl, thePoint, scrollActionUPP);
					DisposeRoutineDescriptor(scrollActionUPP);
					break;
			}
			
			ResetHiliteRgn(theWindow);
			
			return TRUE;
		}
		else if (theControl==GetWindowHScrollBar(theWindow))
		{
			switch (partCode)
			{
				case inThumb:
					if (dynamicScroll)
					{
#if USE_STYLED_TEXT
						if (isStyled)
							StyledTrackThumb(theWindow, thePoint, theControl);
						else
#endif
							MyTrackThumb(theWindow, thePoint, theControl);
					}
					else
					{
						oldSetting=GetControlValue(theControl);
						partCode=TrackControl(theControl, thePoint, 0L);
						if (partCode==inThumb)
						{
							scrollDistance=oldSetting-GetControlValue(theControl);
							if (scrollDistance!=0)
							{
								hTE=GetWindowTE(theWindow);
								TEPinScroll(scrollDistance, 0, hTE);
							}
						}
					}
					break;
				case inUpButton:
				case inDownButton:
				case inPageUp:
				case inPageDown:
					scrollActionUPP=NewControlActionProc(HScrollActionProc);
					partCode=TrackControl(theControl, thePoint, scrollActionUPP);
					DisposeRoutineDescriptor(scrollActionUPP);
					break;
			}
			
			ResetHiliteRgn(theWindow);
			
			return TRUE;
		}
	}
	
	return FALSE;
}

void GenericActivate(WindowRef theWindow, Boolean drawGrowIconLines)
{
	GrafPtr			curPort;
	
	GetPort(&curPort);
	SetPort(theWindow);
	TEActivate(GetWindowTE(theWindow));
	HiliteControl(GetWindowVScrollBar(theWindow), 0);
	HiliteControl(GetWindowHScrollBar(theWindow), 0);
	DrawGrowIconNoLines(theWindow, drawGrowIconLines);
	UpdateControls(theWindow, theWindow->visRgn);
	SetPort(curPort);
	SetWindowIsActive(theWindow, TRUE);
}

void GenericDeactivate(WindowRef theWindow)
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
	SetWindowIsActive(theWindow, FALSE);
}
