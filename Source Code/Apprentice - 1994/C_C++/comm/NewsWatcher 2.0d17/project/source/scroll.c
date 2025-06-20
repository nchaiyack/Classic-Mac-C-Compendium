/*----------------------------------------------------------------------------

	scroll.c

	This module handles text window scrolling.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <stdlib.h>

#include "glob.h"
#include "draw.h"
#include "scroll.h"
#include "util.h"


/*	MakeVScroller adds a vertical scroll bar to a textedit window */

void MakeVScroller (WindowPtr wind)
{
	TWindow **info;
	Rect windowRect,vScrollRect;
	
	info = (TWindow**)GetWRefCon(wind);
	windowRect = wind->portRect;
	
	SetRect(&vScrollRect,
		windowRect.right-windowRect.left - 15,
		(**info).panelHeight-1,
		windowRect.right-windowRect.left+1,
		windowRect.bottom-windowRect.top-14);
	NewControl(wind,&vScrollRect,"\p",true,0,0,0,scrollBarProc,kVScroll);
}


/* MakeHScroller adds a horizontal scroll bar to a textedit window */

void MakeHScroller (WindowPtr wind, short max)
{
	Rect windowRect,hScrollRect;
	
	windowRect = wind->portRect;
	
	SetRect(&hScrollRect,kSectionMargin,(windowRect.bottom-windowRect.top - 15),
			(windowRect.right-windowRect.left - 14),
			(windowRect.bottom-windowRect.top + 1));
	NewControl(wind,&hScrollRect,"\p",true,0,0,max,scrollBarProc,kHScroll);
}


ControlHandle HScrollCont (WindowPtr wind)
{
	ControlHandle theControl;
	
	theControl = ((WindowPeek)wind)->controlList;
	
	while (theControl != nil && GetCRefCon(theControl) != kHScroll) 
		theControl = (**theControl).nextControl;
	return theControl;
}


ControlHandle VScrollCont (WindowPtr wind)
{
	ControlHandle theControl;
	
	theControl = ((WindowPeek)wind)->controlList;
	
	while (theControl != nil && GetCRefCon(theControl) != kVScroll) 
		theControl = (**theControl).nextControl;
	return theControl;
}


static long sign (long longVar)
{
	if (longVar>=0)
		return(1);
	else
		return(-1);
}


short LinesInText (TEHandle theTE)
{
	short lines;
	Handle textHandle;
	
	lines = (**theTE).nLines;
	textHandle = (**theTE).hText;
	if ( (**theTE).teLength > 0 ) {
		if ( *(*textHandle+((**theTE).teLength - 1)) == CR )
			lines++;
		return(lines);
	}
}


void MoveText (WindowPtr wind, ControlHandle theControl)
{
	long	viewTop,destTop;
	long	scrollValue;
	long	scrollDiff,oldScroll,newScroll;
	short	height;
	TEHandle theTE;
	TWindow **info;
	Rect windowRect,r;
	Handle text;
	long **breaks;
	long offset;
	long length;
	short curSection;
	char *p, *pEnd;
	
	info = (TWindow**)GetWRefCon(wind);
	theTE = (**info).theTE;	

	if (GetCRefCon(theControl) == kVScroll) {
		viewTop = (**theTE).viewRect.top;
		destTop = (**theTE).destRect.top;
		oldScroll = viewTop - destTop;
		scrollValue = GetCtlValue(theControl);
		height = (**theTE).lineHeight;
		newScroll = scrollValue * height;
		scrollDiff = oldScroll - newScroll;
		if (abs(scrollDiff)>32000) {
			TEScroll(0,sign(scrollDiff) * 32000,theTE);
			SysBeep(30);
		} else if (scrollDiff != 0) {
			TEScroll(0,scrollDiff,theTE);
		}
	} else {
		curSection = GetCtlValue(theControl);
		(**info).curSection = curSection;
		SetPort(wind);
		windowRect = wind->portRect;
		SetRect(&r,kTextMargin,windowRect.bottom-13,kSectionMargin-2,
			windowRect.bottom-2);
		EraseRect(&r);
		DrawSectionMessage(wind);
		text = (**info).fullText;
		HLock(text);
		breaks = (**info).sectionBreaks;
		offset = (*breaks)[(**info).curSection];
		length = (*breaks)[(**info).curSection+1] - offset;
		if (curSection == 0 && !(**info).headerShown) {
			p = *text + offset;
			pEnd = p + length;
			while (p < pEnd) {
				if (*p == CR && *(p+1) == CR) break;
				p++;
			}
			p += 2;
			while (p < pEnd && *p == CR) p++;
			if (p < pEnd) {
				offset = p - *text;
				length = pEnd - p;
			}
		}
		TESetText(*text+offset,length,theTE);
		HUnlock(text);
		TESetSelect(0,0,theTE);
		AdjustScrollBar(wind);
		SetCtlValue(VScrollCont(wind),0);
		(**theTE).destRect = (**theTE).viewRect;
		EraseRect(&(**theTE).destRect);
		TEUpdate(&(**theTE).destRect,theTE);
	}
}


void AdjustScrollBar (WindowPtr wind)
{
	short windowLines,currentLines;
	TEHandle theTE;
	TWindow **info;
	
	info = (TWindow**)GetWRefCon(wind);
	theTE = (**info).theTE;	

	windowLines = ((**theTE).viewRect.bottom - (**theTE).viewRect.top) / 
		(**theTE).lineHeight;
	if ((currentLines = LinesInText(theTE)) > windowLines)
		SetCtlMax(VScrollCont(wind),currentLines - windowLines);
	else
		SetCtlMax(VScrollCont(wind),0);
}


void ScrollChar (WindowPtr wind, short charPos, Boolean toBottom)
{
	short theLine, windowLines, nLines;
	TEHandle theTE;
	TWindow **info;
	
	info = (TWindow**)GetWRefCon(wind);
	theTE = (**info).theTE;	
	nLines = (**theTE).nLines;
	
	theLine = 0;
	while (theLine < nLines && (**theTE).lineStarts[theLine + 1] <= charPos )
		theLine++;
	if (toBottom) {
		windowLines = ((**theTE).viewRect.bottom - (**theTE).viewRect.top) / (**theTE).lineHeight;
		theLine = theLine - (windowLines - 1);
	}
	SetCtlValue(VScrollCont(wind),theLine);
	MoveText(wind,VScrollCont(wind));
}


void CheckInsertion (WindowPtr wind)
{
	short topLine,bottomLine,windowLines;
	TEHandle theTE;
	TWindow **info;
	
	info = (TWindow**)GetWRefCon(wind);
	theTE = (**info).theTE;	
	
	windowLines = ((**theTE).viewRect.bottom - (**theTE).viewRect.top) / (**theTE).lineHeight;
	topLine = GetCtlValue(VScrollCont(wind));
	bottomLine = topLine + windowLines;
	if (bottomLine > (**theTE).nLines) {
		bottomLine = (**theTE).nLines;
	}
	if (GetCtlMax(VScrollCont(wind)) == 0)
		MoveText(wind,VScrollCont(wind));
	else if ((**theTE).selEnd < (**theTE).lineStarts[topLine])
		ScrollChar(wind,(**theTE).selStart,false);
	else if ((**theTE).selStart >= (**theTE).lineStarts[bottomLine])
		ScrollChar(wind,(**theTE).selEnd,true);
}



static pascal void scroll_action (ControlHandle scrollBar, short part)
{
	short	scrollAmt = 0,pageSize;
	short	theCtlValue;
	TEHandle theTE;
	long refCon;
	TWindow **info;
	
	info = (TWindow**)GetWRefCon(FrontWindow());
	theTE = (**info).theTE;	
	refCon = GetCRefCon(scrollBar);
	
	pageSize =  ((**theTE).viewRect.bottom - (**theTE).viewRect.top) / (**theTE).lineHeight - 1;
	switch (part) {
		case inUpButton:
			scrollAmt = -1;
			break;
		case inDownButton:
			scrollAmt = 1;
			break;
		case inPageUp:
			scrollAmt = (refCon == kVScroll) ? -pageSize : -1;
			break;
		case inPageDown:
			scrollAmt = (refCon == kVScroll) ? pageSize : 1;
			break;
	}
	theCtlValue = GetCtlValue(scrollBar)+scrollAmt;
	if (refCon == kVScroll && (part == inPageDown || part == inPageUp)
		|| theCtlValue <= GetCtlMax(scrollBar) && theCtlValue >= GetCtlMin(scrollBar))
	{
		SetCtlValue(scrollBar,GetCtlValue(scrollBar)+scrollAmt);
		MoveText(FrontWindow(),scrollBar);
	}
}


void DoScrollers (ControlHandle scrollBar, short part, Point localMouse)
{
	if (part == inThumb) {
		TrackControl(scrollBar,localMouse,nil);
		MoveText(FrontWindow(),scrollBar);
	}
	else
		TrackControl(scrollBar,localMouse,(ProcPtr) scroll_action);
}	


pascal Boolean AutoScroll (void)
{
	RgnHandle	oldClip;
	Point		mouseLoc;
	Rect		textRect,tempRect;
	short		deltaX = 0,deltaY = 0;
	ControlHandle sBar;
	TEHandle theTE;
	TWindow **info;
	
	info = (TWindow**)GetWRefCon(FrontWindow());
	theTE = (**info).theTE;	
	
	oldClip = NewRgn();
	GetClip(oldClip);
	tempRect = FrontWindow()->portRect;
	ClipRect(&tempRect);
	GetMouse(&mouseLoc);
	textRect = (**theTE).viewRect;
	if (mouseLoc.v < textRect.top)
		deltaY = -1;
	else if (mouseLoc.v > textRect.bottom)
		deltaY = 1;
	if (deltaY) {
		sBar = VScrollCont(FrontWindow());
		if (GetCtlValue(sBar)+deltaY <= GetCtlMax(sBar) &&
			GetCtlValue(sBar)+deltaY >= GetCtlMin(sBar)) {
			SetCtlValue(sBar,GetCtlValue(sBar)+deltaY);
			MoveText(FrontWindow(),sBar);
		}
	}
	SetClip(oldClip);
	DisposeRgn(oldClip);
	return true;
}


void ScrollTextLineUp (WindowPtr wind)
{
	ControlHandle scrollBar;
	short newCtlValue;
	
	scrollBar = VScrollCont(wind);	
	newCtlValue = GetCtlValue(scrollBar) - 1;
	if (newCtlValue < 0) return;
	SetCtlValue(scrollBar, newCtlValue);
	MoveText(wind, scrollBar);
}


void ScrollTextLineDown (WindowPtr wind)
{
	ControlHandle scrollBar;
	short newCtlValue;
	
	scrollBar = VScrollCont(wind);	
	newCtlValue = GetCtlValue(scrollBar) + 1;
	if (newCtlValue > GetCtlMax(scrollBar)) return;
	SetCtlValue(scrollBar, newCtlValue);
	MoveText(wind, scrollBar);
}


void ScrollTextPageUp (WindowPtr wind)
{
	TWindow **info;
	TEHandle theTE;
	ControlHandle scrollBar;
	short	pageSize, newCtlValue;
	
	info = (TWindow**)GetWRefCon(wind);
	theTE = (**info).theTE;
	scrollBar = VScrollCont(wind);	
	pageSize =  ((**theTE).viewRect.bottom - (**theTE).viewRect.top) / (**theTE).lineHeight - 1;
	newCtlValue = GetCtlValue(scrollBar) - pageSize;
	if (newCtlValue < 0) newCtlValue = 0;
	SetCtlValue(scrollBar, newCtlValue);
	MoveText(wind, scrollBar);
}


void ScrollTextPageDown (WindowPtr wind)
{
	TWindow **info;
	TEHandle theTE;
	ControlHandle scrollBar;
	short	pageSize, newCtlValue, ctlMax;
	
	info = (TWindow**)GetWRefCon(wind);
	theTE = (**info).theTE;
	scrollBar = VScrollCont(wind);	
	pageSize =  ((**theTE).viewRect.bottom - (**theTE).viewRect.top) / (**theTE).lineHeight - 1;
	ctlMax = GetCtlMax(scrollBar);
	newCtlValue = GetCtlValue(scrollBar) + pageSize;
	if (newCtlValue > ctlMax) newCtlValue = ctlMax;
	SetCtlValue(scrollBar, newCtlValue);
	MoveText(wind, scrollBar);
}


void ScrollTextHome (WindowPtr wind)
{
	ControlHandle scrollBar;
	
	scrollBar = VScrollCont(wind);	
	SetCtlValue(scrollBar, 0);
	MoveText(wind, scrollBar);
}


void ScrollTextEnd (WindowPtr wind)
{
	ControlHandle scrollBar;
	
	scrollBar = VScrollCont(wind);	
	SetCtlValue(scrollBar, GetCtlMax(scrollBar));
	MoveText(wind, scrollBar);
}


void ScrollTextLeft (WindowPtr wind)
{
	ControlHandle scrollBar;
	short newCtlValue;
	
	scrollBar = HScrollCont(wind);
	if (scrollBar == nil) return;	
	newCtlValue = GetCtlValue(scrollBar) - 1;
	if (newCtlValue < 0) return;
	SetCtlValue(scrollBar, newCtlValue);
	MoveText(wind, scrollBar);
}


void ScrollTextRight (WindowPtr wind)
{
	ControlHandle scrollBar;
	short newCtlValue;
	
	scrollBar = HScrollCont(wind);
	if (scrollBar == nil) return;	
	newCtlValue = GetCtlValue(scrollBar) + 1;
	if (newCtlValue > GetCtlMax(scrollBar)) return;
	SetCtlValue(scrollBar, newCtlValue);
	MoveText(wind, scrollBar);
}