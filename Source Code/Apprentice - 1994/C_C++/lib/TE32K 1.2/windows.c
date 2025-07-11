#include "TE32K.h"
#include <stdio.h>


#define	INDENT	5

extern	WindowPtr		theWPtr;
extern	TE32KHandle		theTEH32K;
extern	ControlHandle	theVScroll,theHScroll;
extern	Cursor			editCursor,waitCursor;
extern	char			changed;
extern	SFReply			mySFReply;
extern	int				defaultFont,defaultFontSize;


SetUpCursors()
{
CursHandle	hCurs;
	
	hCurs = GetCursor(iBeamCursor);
	editCursor = **hCurs;
	
	hCurs = GetCursor(watchCursor);
	waitCursor = **hCurs;
}



DoMouseDown(theEvent)
EventRecord	*theEvent;
{
WindowPtr	whichWindow;
int			windowCode;

	windowCode = FindWindow(theEvent->where,&whichWindow);
	
	switch (windowCode = FindWindow(theEvent->where,&whichWindow)) 
	{
		case inMenuBar:
			DoCommand(MenuSelect(theEvent->where));
			break;
			
		case inSysWindow:
			SystemClick(theEvent,whichWindow);
			break;
			
		case inContent:
			if (whichWindow != FrontWindow())
				SelectWindow(whichWindow);
			
			else if (IsOurWindow(whichWindow))
				DoContent(whichWindow,theEvent);
			
			break;
		
		case inDrag:
			if (whichWindow != FrontWindow())
				SelectWindow(whichWindow);
			
			else if (IsOurWindow(whichWindow))
				DoDrag(whichWindow,theEvent);
			
			break;
			
		case inGrow:
			if (whichWindow != FrontWindow())
				SelectWindow(whichWindow);
			
			else if (IsOurWindow(whichWindow))
				DoGrow(whichWindow,theEvent);
			
			break;
		
		case inZoomIn:
		case inZoomOut:
			if (whichWindow != FrontWindow())
				SelectWindow(whichWindow);
			
			else if (IsOurWindow(whichWindow) && TrackBox(whichWindow,theEvent->where,windowCode))
				DoZoom(whichWindow,windowCode);
			
			break;
		
		case inGoAway:
			if (whichWindow != FrontWindow())
				SelectWindow(whichWindow);
			
			else if (IsOurWindow(whichWindow) && TrackGoAway(whichWindow,theEvent->where))
				DoCloseWindow(whichWindow);
			
			break;
	}
}






IsOurWindow(whichWindow)
WindowPtr	whichWindow;
{
	if (theWPtr && whichWindow == theWPtr)
		return(TRUE);
	else
		return(FALSE);
}



DoActivateDeactivate(whichWindow,adFlag)
WindowPtr	whichWindow;char	adFlag;
{
Rect			tempRect;
GrafPtr			oldPort;

	if (whichWindow == theWPtr)
	{
		GetPort(&oldPort);
		SetPort(whichWindow);
		
		if (adFlag)
		{
			HiliteControl(theVScroll,0);
			HiliteControl(theHScroll,0);
			TE32KActivate(theTEH32K);
		}
		else
		{
			HiliteControl(theVScroll,255);
			HiliteControl(theHScroll,255);
			TE32KDeactivate(theTEH32K);
		}
		
		DrawGrowIcon(whichWindow);
		
		SetPort(oldPort);
		
		return(TRUE);
	}
	
	else
		return(FALSE);
}






DoCloseWindow(whichWindow)
WindowPtr	whichWindow;
{
int		userChoice;

	if (theWPtr && whichWindow==theWPtr)
	{
		if (changed)
		{
			userChoice = YesNoCancel("Save changes?",3);
			
			if (userChoice == 3)
				return(FALSE);
			
			else if (userChoice == 1 && !SaveTextFile(theTEH32K,&mySFReply))
				return(FALSE);
		}		
		
		if (theVScroll) DisposeControl(theVScroll);
		if (theHScroll) DisposeControl(theHScroll);
		if (theTEH32K) TE32KDispose(theTEH32K);
		if (theWPtr) DisposeWindow(theWPtr);
		
		theWPtr = 0L;
		theTEH32K = 0L;
		theVScroll = 0L;
		theHScroll = 0L;
		
		return(TRUE);
	}
	
	else
		return(FALSE);
}



DoGrow(whichWindow,theEvent)
WindowPtr	whichWindow;EventRecord	*theEvent;
{
Rect			tempRect;
long			newSize;
GrafPtr			oldPort;

	if (theWPtr && whichWindow==theWPtr)
	{
		SetRect(&tempRect,100,50,32767,32767);
		newSize = GrowWindow(whichWindow,theEvent->where,&tempRect);
		SizeWindow(whichWindow,LoWord(newSize),HiWord(newSize),0xff);
		tempRect = whichWindow->portRect;
		GetPort(&oldPort);
		SetPort(whichWindow);
		EraseRect(&tempRect);
		InvalRect(&tempRect);
		
		MoveControl(theVScroll,tempRect.right+1-16,tempRect.top-1);
		SizeControl(theVScroll,16,whichWindow->portRect.bottom-14-(whichWindow->portRect.top-1));
		
		MoveControl(theHScroll,tempRect.left-1,tempRect.bottom+1-16);
		SizeControl(theHScroll,whichWindow->portRect.right-14-(whichWindow->portRect.left-1),16);
		
		AdjustForResizedWindow();
		
		SetPort(oldPort);
		
		return(TRUE);
	}
	
	else
		return(FALSE);
}




DoZoom(whichWindow,windowCode)
WindowPtr	whichWindow;int windowCode;
{
Rect			tempRect;
GrafPtr			oldPort;

	if (theWPtr && whichWindow==theWPtr)
	{
		GetPort(&oldPort);
		SetPort(whichWindow);
		
		tempRect = whichWindow->portRect;
		EraseRect(&tempRect);
		
		ZoomWindow(whichWindow, windowCode, 0);
		
		tempRect = whichWindow->portRect;
		EraseRect(&tempRect);
		
		MoveControl(theVScroll,tempRect.right+1-16,tempRect.top-1);
		SizeControl(theVScroll,16,whichWindow->portRect.bottom-14-(whichWindow->portRect.top-1));
		
		MoveControl(theHScroll,tempRect.left-1,tempRect.bottom+1-16);
		SizeControl(theHScroll,whichWindow->portRect.right-14-(whichWindow->portRect.left-1),16);
		
		AdjustForResizedWindow();
		
		tempRect = whichWindow->portRect;
		InvalRect(&tempRect);
		
		SetPort(oldPort);
		
		return(TRUE);
	}
	
	else
		return(FALSE);
}



DoDrag(whichWindow,theEvent)
WindowPtr	whichWindow;EventRecord	*theEvent;
{
Rect			tempRect;

	if (theWPtr && whichWindow == theWPtr)
	{
		SetRect(&tempRect,screenBits.bounds.left+10,screenBits.bounds.top+25,screenBits.bounds.right-10,screenBits.bounds.bottom-25);
		DragWindow(whichWindow,theEvent->where,&tempRect);
		return(TRUE);
	}
	
	else
		return(FALSE);
}




AdjustScrollBar()
{
int		ctlVal,screenLines,numLines,oldVal;

	ctlVal = ((*theTEH32K)->viewRect.top - (*theTEH32K)->destRect.top)/(*theTEH32K)->lineHeight;
	screenLines = ((*theTEH32K)->viewRect.bottom - (*theTEH32K)->viewRect.top)/(**theTEH32K).lineHeight;
	numLines = (**theTEH32K).nLines;
	
	if (ctlVal > numLines-screenLines)
		SetCtlMax(theVScroll,ctlVal);
	else
		SetCtlMax(theVScroll,(numLines-screenLines > 0) ? numLines-screenLines : 0);
	
	oldVal = GetCtlValue(theVScroll);
	
	if (oldVal != ctlVal)
		SetCtlValue(theVScroll,ctlVal);
	
	
	
	ctlVal = ((*theTEH32K)->viewRect.left - (*theTEH32K)->destRect.left)/(*theTEH32K)->lineHeight;
	numLines = GetCtlMax(theHScroll);
	
	if (ctlVal < 0)
		ctlVal = 0;
	else if (ctlVal > numLines)
		ctlVal = numLines;
	
	oldVal = GetCtlValue(theHScroll);
	
	if (oldVal != ctlVal)
		SetCtlValue(theHScroll,ctlVal);
}



AdjustForResizedWindow()
{
LongRect	tempLongRect;
Rect		tempRect;

	SetRect(&tempRect,(theWPtr)->portRect.left+INDENT,(theWPtr)->portRect.top+INDENT,(theWPtr)->portRect.right-15-INDENT,(theWPtr)->portRect.bottom-16-INDENT);
	tempRect.bottom = tempRect.top + ((tempRect.bottom - tempRect.top)/(*theTEH32K)->lineHeight)*(*theTEH32K)->lineHeight;
	
	RectToLongRect(&tempRect,&((*theTEH32K)->viewRect));
	
	RectToLongRect(&tempRect,&((*theTEH32K)->destRect));
	(*theTEH32K)->destRect.top -= (long) GetCtlValue(theVScroll)*(*theTEH32K)->lineHeight;
	(*theTEH32K)->destRect.left -= (long) GetCtlValue(theHScroll)*(*theTEH32K)->lineHeight;
	
	(**theTEH32K).destRect.right -= 10;
	
	TE32KFromScrap();
	
	SetCursor(&waitCursor);
	TE32KCalText(theTEH32K);
	InitCursor();
	
	AdjustScrollBar();
}



pascal void myScrollProc(theControl, theCode)
ControlHandle	theControl;int	theCode;
{
long		scrollAmt,lines,numLines;
int			controlMax,controlMin,controlVal;
RgnHandle	updateRgn;
	
	if (theVScroll && theControl==theVScroll)
	{
		controlMax = GetCtlMax(theControl);
		controlMin = GetCtlMin(theControl);
		controlVal = GetCtlValue(theControl);
		
		updateRgn = NewRgn();
		
		switch (theCode) 
		{
			case inUpButton:
				if (controlVal > controlMin)
				{
					SetCtlValue(theControl,controlVal-1);
					
					TE32KScroll(0L,(long) (**theTEH32K).lineHeight,theTEH32K);
				}
				
				break;
				
			case inDownButton: 
				if (controlVal < controlMax)
				{
					SetCtlValue(theControl,controlVal+1);
					
					TE32KScroll(0L,(long) -(**theTEH32K).lineHeight,theTEH32K);
				}
				
				break;
	
			case inPageUp: 
				if (controlVal > controlMin)
				{
					lines = ((*theTEH32K)->viewRect.bottom - (*theTEH32K)->viewRect.top)/(**theTEH32K).lineHeight;
					scrollAmt = (controlVal-lines < controlMin) ? controlVal-controlMin : lines;
					SetCtlValue(theControl,controlVal-scrollAmt);
					TE32KScroll(0L,(long) ((long) scrollAmt * (long) (**theTEH32K).lineHeight),theTEH32K);
				}
				
				break;
	
			case inPageDown: 
				if (controlVal < controlMax)
				{
					lines = ((**theTEH32K).viewRect.bottom - (**theTEH32K).viewRect.top)/(**theTEH32K).lineHeight;
					scrollAmt = (controlVal+lines > controlMax) ? controlMax-controlVal : lines;
					SetCtlValue(theControl,controlVal+scrollAmt);
					TE32KScroll(0L,(long) ((long) -scrollAmt * (long) (**theTEH32K).lineHeight),theTEH32K);
				}
				
				break;
		}
		
		AdjustScrollBar();
		
		DisposeRgn(updateRgn);
	}
	
	else if (theHScroll && theControl==theHScroll)
	{
		controlMax = GetCtlMax(theControl);
		controlMin = GetCtlMin(theControl);
		controlVal = GetCtlValue(theControl);
		
		updateRgn = NewRgn();
		
		switch (theCode) 
		{
			case inUpButton:
				if (controlVal > controlMin)
				{
					SetCtlValue(theControl,controlVal-1);
					
					TE32KScroll((long) (**theTEH32K).lineHeight,0L,theTEH32K);
				}
				
				break;
				
			case inDownButton: 
				if (controlVal < controlMax)
				{
					SetCtlValue(theControl,controlVal+1);
					
					TE32KScroll((long) -(**theTEH32K).lineHeight,0L,theTEH32K);
				}
				
				break;
	
			case inPageUp: 
				if (controlVal > controlMin)
				{
					lines = ((*theTEH32K)->viewRect.right - (*theTEH32K)->viewRect.left)/(2 * (**theTEH32K).lineHeight);
					scrollAmt = (controlVal-lines < controlMin) ? controlVal-controlMin : lines;
					SetCtlValue(theControl,controlVal-scrollAmt);
					TE32KScroll((long) ((long) scrollAmt * (long) (**theTEH32K).lineHeight),0L,theTEH32K);
				}
				
				break;
	
			case inPageDown: 
				if (controlVal < controlMax)
				{
					lines = ((*theTEH32K)->viewRect.right - (*theTEH32K)->viewRect.left)/(2 * (**theTEH32K).lineHeight);
					scrollAmt = (controlVal+lines > controlMax) ? controlMax-controlVal : lines;
					SetCtlValue(theControl,controlVal+scrollAmt);
					TE32KScroll((long) -((long) scrollAmt * (long) (**theTEH32K).lineHeight),0L,theTEH32K);
				}
				
				break;
		}
		
		AdjustScrollBar();
		
		DisposeRgn(updateRgn);
	}
}





DoContent(whichWindow,theEvent)
WindowPtr	whichWindow;EventRecord	*theEvent;
{
GrafPtr			oldPort;
ControlHandle	whichControl;
int				cntlCode;
long			oldVal,ctlVal;
Rect			tempRect;

	if (theWPtr && whichWindow==theWPtr)
	{
		GetPort(&oldPort);
		SetPort(whichWindow);
		
		LongRectToRect(&((**theTEH32K).viewRect),&tempRect);
		
		GlobalToLocal(&(theEvent->where));
		
		cntlCode = FindControl(theEvent->where,whichWindow,&whichControl);
		
		if (theVScroll && whichControl == theVScroll && cntlCode != 0)
		{
			if (cntlCode == inThumb)
			{
				oldVal = GetCtlValue(whichControl);
				
				TrackControl(whichControl,theEvent->where,0L);
				
				TE32KScroll(0L,(long) ((oldVal - (long) GetCtlValue(whichControl)) * (long) (**theTEH32K).lineHeight),theTEH32K);
			}
			
			else
				TrackControl(whichControl,theEvent->where,myScrollProc);
		}
		
		else if (theHScroll && whichControl == theHScroll && cntlCode != 0)
		{
			if (cntlCode == inThumb)
			{
				oldVal = GetCtlValue(whichControl);
				
				TrackControl(whichControl,theEvent->where,0L);
				
				TE32KScroll((long) ((oldVal - GetCtlValue(whichControl)) * (**theTEH32K).lineHeight),0L,theTEH32K);
			}
			
			else
				TrackControl(whichControl,theEvent->where,myScrollProc);
		}
		
		else if (PtInRect(theEvent->where,&tempRect))
		{
			if (theEvent->modifiers & shiftKey)
				TE32KClick(theEvent->where,TRUE,theTEH32K);
			else
				TE32KClick(theEvent->where,FALSE,theTEH32K);
		}
		
		SetPort(oldPort);
		
		return(TRUE);
	}
	
	else
		return(FALSE);
}




UpdateWindow(whichWindow)
WindowPtr	whichWindow;
{
GrafPtr			oldPort;
Rect			tempRect;
LongRect		tempLongRect;
RgnHandle		updateRgn;

	if (whichWindow != theWPtr)
		return(FALSE);

	GetPort(&oldPort);
	SetPort(whichWindow);
	
	PenNormal();
	
	BeginUpdate(whichWindow);
	
	tempRect = whichWindow->portRect;
	EraseRect(&tempRect);
	
	if (whichWindow == theWPtr)
	{
		DrawGrowIcon(whichWindow);
		DrawControls(whichWindow);
		
		updateRgn = ((WindowPeek) theWPtr)->updateRgn;
		
		tempLongRect.left = (**updateRgn).rgnBBox.left;
		tempLongRect.top = (**updateRgn).rgnBBox.top;
		tempLongRect.right = (**updateRgn).rgnBBox.right;
		tempLongRect.bottom = (**updateRgn).rgnBBox.bottom;
		
		RectToLongRect(&tempRect,&tempLongRect);
		TE32KUpdate(&tempLongRect,theTEH32K);
	}
	
	EndUpdate(whichWindow);
	
	SetPort(oldPort);
	
	return(TRUE);
}





void MyClicker()
{
int			controlMax,controlMin,controlVal,lineHeight;
Rect		viewRect;
Point		mousePoint;
RgnHandle	saveClip;
long		hDelta,vDelta;

	LongRectToRect(&((**theTEH32K).viewRect),&viewRect);
	lineHeight = (**theTEH32K).lineHeight;

	hDelta = 0L;
	vDelta = 0L;
	
	GetMouse(&mousePoint);
	
	if (!PtInRect(mousePoint,&viewRect))
	{
		controlMax = GetCtlMax(theVScroll);
		controlMin = GetCtlMin(theVScroll);
		controlVal = GetCtlValue(theVScroll);
		
		if (mousePoint.v>viewRect.bottom && controlVal<controlMax)
		{
			vDelta = -lineHeight;
			SetCtlValue(theVScroll,controlVal+1);
		}
		
		else if (mousePoint.v<viewRect.top && controlVal>controlMin)
		{
			vDelta = lineHeight;
			SetCtlValue(theVScroll,controlVal-1);
		}
		
		controlMax = GetCtlMax(theHScroll);
		controlMin = GetCtlMin(theHScroll);
		controlVal = GetCtlValue(theHScroll);
		
		if (mousePoint.h>viewRect.right && controlVal<controlMax)
		{
			hDelta = -lineHeight;
			SetCtlValue(theHScroll,controlVal+1);
		}
		
		else if (mousePoint.h<viewRect.left && controlVal>controlMin)
		{
			hDelta = lineHeight;
			SetCtlValue(theHScroll,controlVal-1);
		}
	}
	
	if (hDelta || vDelta)
	{
		saveClip = NewRgn();
		GetClip(saveClip);
		ClipRect(&(theWPtr->portRect));
		
		TE32KScroll(hDelta,vDelta,theTEH32K);
		
		SetClip(saveClip);
		DisposeRgn(saveClip);
	}
}




void MyClickLoop()
{
	asm
	{
		movem.l		d1-d7/a0-a6,-(sp)
		jsr			MyClicker
		movem.l		(sp)+,d1-d7/a0-a6
		moveq.l		#1,d0
		rts
	}
}




DoShowWindow()
{
Rect			tempRect;
FontInfo		theFontInfo;
LongRect		tempLongRect;

	if (theWPtr)
		SelectWindow(theWPtr);
		
	else
	{
		SetRect(&tempRect,20,40,500,320);
		
		theWPtr = NewWindow (0L,&tempRect,"\pUntitled",TRUE,zoomDocProc,(WindowPtr) -1L,TRUE,0L);
		if (StripAddress(theWPtr)==0L)
		{
			ErrorAlert("Insufficient memory to open window");
			return;
		}
		
		SetPort(theWPtr);
		
		TextFont(defaultFont);
		TextSize(defaultFontSize);
		TextFace(0);
		TextMode(srcCopy);
		
		GetFontInfo(&theFontInfo);
		
		SetRect(&tempRect,(theWPtr)->portRect.right-15,(theWPtr)->portRect.top-1,(theWPtr)->portRect.right+1,(theWPtr)->portRect.bottom-14);
		theVScroll = NewControl(theWPtr,&tempRect,"\p",TRUE,0,0,0,scrollBarProc,0L);
		
		if (StripAddress(theVScroll)==0L)
		{
			DisposeWindow(theWPtr);
			theWPtr = 0L;
			theVScroll = 0L;
			ErrorAlert("Insufficient memory to open edit record");
			return;
		}
		
		SetRect(&tempRect,(theWPtr)->portRect.left-1,(theWPtr)->portRect.bottom+1-16,(theWPtr)->portRect.right-14,(theWPtr)->portRect.bottom+1);
		theHScroll = NewControl(theWPtr,&tempRect,"\p",TRUE,0,0,255,scrollBarProc,0L);
		
		if (StripAddress(theVScroll)==0L)
		{
			DisposeWindow(theWPtr);
			DisposeControl(theVScroll);
			theWPtr = 0L;
			theVScroll = 0L;
			theHScroll = 0L;
			ErrorAlert("Insufficient memory to open edit record");
			return;
		}
		
		SetRect(&(tempRect),(theWPtr)->portRect.left+INDENT,(theWPtr)->portRect.top+INDENT,(theWPtr)->portRect.right-15-INDENT,(theWPtr)->portRect.bottom-16-INDENT);
		RectToLongRect(&tempRect,&tempLongRect);
		theTEH32K = TE32KNew(&tempLongRect,&tempLongRect);
	
		if (StripAddress(theTEH32K)==0L)
		{
			DisposeControl(theVScroll);
			DisposeControl(theHScroll);
			DisposeWindow(theWPtr);
			theWPtr = 0L;
			theVScroll = 0L;
			theHScroll = 0L;
			ErrorAlert("Insufficient memory to open edit record");
			return;
		}
		
		(**theTEH32K).destRect.right -= 10;
		
		TE32KFromScrap();
		
		(*theTEH32K)->destRect.bottom = (*theTEH32K)->destRect.top + (((*theTEH32K)->destRect.bottom - (*theTEH32K)->destRect.top)/(*theTEH32K)->lineHeight)*(*theTEH32K)->lineHeight;
		(*theTEH32K)->viewRect.bottom = (*theTEH32K)->viewRect.top + (((*theTEH32K)->viewRect.bottom - (*theTEH32K)->viewRect.top)/(*theTEH32K)->lineHeight)*(*theTEH32K)->lineHeight;
		
		(**theTEH32K).clikLoop = (ProcPtr) MyClickLoop;
		
		changed = FALSE;
		
		mySFReply.good = FALSE;
	}
}



DoKey(theChar)
unsigned char	theChar;
{
	if (theWPtr && theWPtr == FrontWindow())
	{
		TE32KKey(theChar,theTEH32K);
		
		TE32KSelView(theTEH32K);
		
		changed = TRUE;
		
		AdjustScrollBar();
		
		return(TRUE);
	}
	
	else
		return(FALSE);
}


DoIdle()
{
	if (theWPtr && theWPtr == FrontWindow())
	{
		TE32KIdle(theTEH32K);
		
		return(TRUE);
	}
	
	else
		return(FALSE);
}





MaintainCursor()
{
Point		pt;
GrafPtr		oldPort;
Rect		tempRect;

	if (theWPtr && FrontWindow() == theWPtr)
	{
		GetPort(&oldPort);
		SetPort(theWPtr);
		
		SetRect(&tempRect,(theWPtr)->portRect.left+INDENT,(theWPtr)->portRect.top+INDENT,(theWPtr)->portRect.right-15-INDENT,(theWPtr)->portRect.bottom-16-INDENT);
		
		GetMouse(&pt);
		
		if (PtInRect(pt,&tempRect))
			SetCursor(&editCursor);
		
		else
			SetCursor(&arrow);
		
		return(TRUE);
		
		SetPort(oldPort);
	}
	
	return(FALSE);
}




DoCut()
{
	if (theWPtr && FrontWindow() == theWPtr && theTEH32K)
	{
		SetCursor(&waitCursor);
		TE32KCut(theTEH32K);
		ZeroScrap();
		TE32KToScrap();
		InitCursor();
		
		changed = TRUE;
		
		TE32KSelView(theTEH32K);
		AdjustScrollBar();
		
		return(TRUE);
	}
	
	else
		return(FALSE);
}



DoCopy()
{
	if (theWPtr && FrontWindow() == theWPtr && theTEH32K)
	{
		SetCursor(&waitCursor);
		TE32KCopy(theTEH32K);
		ZeroScrap();
		TE32KToScrap();
		InitCursor();
		
		return(TRUE);
	}
	
	else
		return(FALSE);
}




DoPaste()
{
	if (theWPtr && FrontWindow() == theWPtr && theTEH32K)
	{
		SetCursor(&waitCursor);
		TE32KFromScrap();
		TE32KPaste(theTEH32K);
		InitCursor();
		
		changed = TRUE;
		
		TE32KSelView(theTEH32K);
		AdjustScrollBar();
		
		return(TRUE);
	}
	
	else
		return(FALSE);
}





DoClear()
{
	if (theWPtr && FrontWindow() == theWPtr && theTEH32K)
	{
		SetCursor(&waitCursor);
		TE32KDelete(theTEH32K);
		InitCursor();
		
		changed = TRUE;
		
		TE32KSelView(theTEH32K);
		AdjustScrollBar();
		
		return(TRUE);
	}
	
	else
		return(FALSE);
}



DoSelectAll()
{
	if (theWPtr && FrontWindow() == theWPtr && theTEH32K)
	{
		SetCursor(&waitCursor);
		TE32KSetSelect(0L,(**theTEH32K).teLength,theTEH32K);
		AdjustScrollBar();
		InitCursor();
		
		return(TRUE);
	}
	
	else
		return(FALSE);
}