#include "text layer.h"
#include "program globals.h"
#if USE_DRAG
#include "drag layer.h"
#endif
#include "window layer.h"
#include "memory layer.h"
#include <FixMath.h>

#define RequireWindow(x)	if (theWindow==0L) return x
#define RequireTE(x)		{	\
								hTE=GetWindowTE(theWindow);	\
								if (hTE==0L)	return x;	}

enum { key_LeftArrow=0x1c, key_RightArrow, key_UpArrow, key_DownArrow };

Boolean AnyTextInScrapQQ(void)
{
	long			dummy;
	
	LoadScrap();
	return (GetScrap(0L, 'TEXT', &dummy)!=noTypeErr);
}

void SetTheText(WindowRef theWindow, Ptr data, long count)
{
	TEHandle		hTE;
	ControlHandle	vScrollBar;
	
	RequireWindow();
	RequireTE();
	vScrollBar=GetWindowVScrollBar(theWindow);
	
	TESetText(data, count, hTE);
	TESetSelect(0, 0, hTE);
	if (vScrollBar!=0L)
		AdjustVScrollBar(vScrollBar, hTE);
#if USE_DRAG
	ResetHiliteRgn(theWindow);
#endif
}

Boolean AnyTextQQ(WindowRef theWindow)
{
	TEHandle		hTE;
	
	RequireWindow(FALSE);
	RequireTE(FALSE);
	
	return ((**hTE).teLength!=0);
}

Boolean AnyHighlightedQQ(WindowRef theWindow)
{
	TEHandle		hTE;
	
	RequireWindow(FALSE);
	RequireTE(FALSE);
	
	return ((**hTE).selStart!=(**hTE).selEnd);
}

short SelectionStart(WindowRef theWindow)
{
	TEHandle		hTE;
	
	RequireWindow(-1);
	RequireTE(-1);
	
	return (**hTE).selStart;
}

short SelectionEnd(WindowRef theWindow)
{
	TEHandle		hTE;
	
	RequireWindow(-1);
	RequireTE(-1);
	
	return (**hTE).selEnd;
}

Boolean InsertBeforeStart(WindowRef theWindow, Str255 theStr)
{
	TEHandle		hTE;
	unsigned long	len;
	unsigned long	oldLen;
	
	RequireWindow(FALSE);
	RequireTE(FALSE);
	
	len=theStr[0];
	hTE=GetWindowTE(theWindow);
	oldLen=(**hTE).teLength;
	if (oldLen+len>32767)
		return FALSE;
	
	TEInsert(&theStr[1], len, hTE);
	SetWindowIsModified(theWindow, TRUE);
#if USE_DRAG
	ResetHiliteRgn(theWindow);
#endif
	return TRUE;
}

Boolean InsertAfterEnd(WindowRef theWindow, Str255 theStr)
{
	TEHandle		hTE;
	unsigned long	len;
	unsigned long	oldLen;
	unsigned long	oldSelStart;
	unsigned long	oldSelEnd;
	
	RequireWindow(FALSE);
	RequireTE(FALSE);
	
	len=theStr[0];
	hTE=GetWindowTE(theWindow);
	oldLen=(**hTE).teLength;
	if (oldLen+len>32767)
		return FALSE;
	
	oldSelStart=(**hTE).selStart;
	oldSelEnd=(**hTE).selEnd;
	TESetSelect(oldSelEnd, oldSelEnd, hTE);
	TEInsert(&theStr[1], len, hTE);
	TESetSelect(oldSelStart, oldSelEnd, hTE);
	SetWindowIsModified(theWindow, TRUE);
#if USE_DRAG
	ResetHiliteRgn(theWindow);
#endif
	return TRUE;
}

void GetSelectionString(WindowRef theWindow, Str255 theStr)
{
	TEHandle		hTE;
	short			selStart, selEnd;
	
	RequireWindow();
	RequireTE();
	
	selStart=SelectionStart(theWindow);
	selEnd=SelectionEnd(theWindow);
	if (selEnd-selStart>255)
		selEnd=selStart+255;
	Mymemcpy((Ptr)&theStr[1], (Ptr)((unsigned long)(*((**hTE).hText))+selStart), selEnd-selStart);
	theStr[0]=selEnd-selStart;
}

short TotalNumberOfLines(TEHandle hTE)
{
	short			numLines;
	
	if (hTE==0L)
		return -1;
	
	numLines=(**hTE).nLines;
	if (*((unsigned char*)((long)(*((**hTE).hText))+(**hTE).teLength-1))==0x0d)
		numLines++;
	
	return numLines;
}
void HandleShiftArrow(TEHandle hTE, unsigned char theChar)
{
	short			offset, destOffset;
	short			lineNum, lineStart;
	short			offsetFromLineStart;
	
	switch (theChar)
	{
		case key_UpArrow:
			offset=(**hTE).selStart;
			lineNum=LineNumberFromOffset(hTE, offset);
			if (lineNum==0)
				destOffset=0;
			else
			{
				lineStart=LineStart(hTE, lineNum);
				offsetFromLineStart=offset-lineStart;
				if ((offset==(**hTE).teLength) && ((*((**hTE).hText))[offset-1]=='\r'))
					destOffset=LineStart(hTE, lineNum);
				else
				{
					destOffset=LineStart(hTE, lineNum-1)+offsetFromLineStart;
					if (destOffset>=lineStart)
						destOffset=lineStart-1;
				}
			}
			if (destOffset!=offset)
				TESetSelect(destOffset, (**hTE).selEnd, hTE);
			break;
		case key_DownArrow:
			offset=(**hTE).selEnd;
			lineNum=LineNumberFromOffset(hTE, offset);
			lineStart=LineStart(hTE, lineNum);
			offsetFromLineStart=offset-lineStart;
			destOffset=LineStart(hTE, lineNum+1)+offsetFromLineStart;
			if (destOffset>=(**hTE).teLength)
				destOffset=(**hTE).teLength;
			else if (destOffset>LineStart(hTE, lineNum+2))
				destOffset=LineStart(hTE, lineNum+2);
			if (destOffset!=(**hTE).selEnd)
				TESetSelect((**hTE).selStart, destOffset, hTE);
			break;
		case key_LeftArrow:
			if ((**hTE).selStart>0)
				TESetSelect((**hTE).selStart-1, (**hTE).selEnd, hTE);
			break;
		case key_RightArrow:
			if ((**hTE).selEnd<(**hTE).teLength)
				TESetSelect((**hTE).selStart, (**hTE).selEnd+1, hTE);
			break;
	}
}

void MyTrackThumb(WindowRef theWindow, Point thePoint, ControlHandle theControl)
{
	#pragma unused(thePoint)

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
				TEPinScroll(0, scrollDistance*(**hTE).lineHeight, hTE);
			else
				TEPinScroll(scrollDistance, 0, hTE);
		}
	}
}

pascal void ScrollActionProc(ControlHandle theHandle, short partCode)
{
	short			scrollDistance;
	TEHandle		hTE;
	WindowRef		theWindow;
	
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
			scrollDistance=(**hTE).lineHeight;
			break;
		case inPageUp:
		case inPageDown:
			scrollDistance=(**hTE).viewRect.bottom-(**hTE).viewRect.top-(**hTE).lineHeight;
			break;
		default:
			scrollDistance=0;
			break;
	}
	
	if ((partCode==inDownButton) || (partCode==inPageDown))
		scrollDistance=-scrollDistance;
	
	MyMoveScrollBox(theHandle, scrollDistance/(**hTE).lineHeight);
	
	if (scrollDistance!=0)
	{
		TEPinScroll(0, scrollDistance, hTE);
	}
}

pascal void HScrollActionProc(ControlHandle theHandle, short partCode)
{
	short			scrollDistance;
	TEHandle		hTE;
	WindowRef		theWindow;
	
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
			scrollDistance=20;
			break;
		case inPageUp:
		case inPageDown:
			scrollDistance=(**hTE).viewRect.right-(**hTE).viewRect.left-20;
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
		TEPinScroll(scrollDistance, 0, hTE);
	}
}

void MyMoveScrollBox(ControlHandle theControl, short scrollDistance)
{
	short			oldSetting, setting, max;
	
	oldSetting=GetControlValue(theControl);
	max=GetControlMaximum(theControl);
	setting=oldSetting-scrollDistance;
	if (setting<0)
		setting=0;
	else if (setting>max)
		setting=max;
	
	SetControlValue(theControl, setting);
}

void AdjustVScrollBar(ControlHandle theControl, TEHandle hTE)
{
	short			oldMax, oldValue;
	short			numLines, max, value;
	
	if ((**hTE).viewRect.bottom==(**hTE).viewRect.top)
		return;
	
	oldMax=GetControlMaximum(theControl);
	oldValue=GetControlValue(theControl);
	numLines=TotalNumberOfLines(hTE);
	max=numLines-(((**hTE).viewRect.bottom-(**hTE).viewRect.top)/((**hTE).lineHeight));
	if (max<0)
		max=0;
	SetControlMaximum(theControl, max);
	value=((**hTE).viewRect.top-(**hTE).destRect.top)/((**hTE).lineHeight);
	if (value<0)
		value=0;
	else if (value>max)
		value=max;
	SetControlValue(theControl, value);
	if ((oldMax!=max) || (oldValue!=value))
		ShowControl(theControl);
}

short CurrentLineNumber(TEHandle hTE)
{
	if (hTE==0L)
		return -1;
	
	return LineNumberFromOffset(hTE, (**hTE).selStart);
}

short LineNumberFromOffset(TEHandle hTE, short offset)
{
	short			i;
	short			lineNumber;
	short			numLines;
	
	if (hTE==0L)
		return -1;
	
	lineNumber=-1;
	numLines=(**hTE).nLines;
	for (i=0; i<numLines; i++)
	{
		if ((**hTE).lineStarts[i]<=offset)
			lineNumber++;
		else
			return lineNumber;
	}
	
	return numLines-1;
}

short LineStart(TEHandle hTE, short lineNum)
{
	if (hTE==0L)
		return -1;
	
	return (**hTE).lineStarts[lineNum];
}

pascal Boolean MyClikLoop(void)
{
	Point			thePoint;
	TEHandle		hTE;
	WindowRef		theWindow;
	GrafPtr			curPort;
	
	theWindow=GetFrontDocumentWindow();
	if (theWindow==0L)
		theWindow=FrontWindow();
	if (theWindow==0L)
		return FALSE;
	
	hTE=GetWindowTE(theWindow);
	if (hTE==0L)
		return FALSE;
	
	GetPort(&curPort);
	SetPort(theWindow);
	GetMouse(&thePoint);
	if (!PtInRect(thePoint, &((**hTE).viewRect)))
	{
		if (thePoint.v<((**hTE).viewRect.top))
		{
			if ((**hTE).selStart>0)
				TEPinScroll(0, (**hTE).lineHeight, hTE);
		}
		else if (thePoint.v>(**hTE).viewRect.bottom)
		{
			if ((**hTE).selEnd<(**hTE).teLength)
				TEPinScroll(0, -(**hTE).lineHeight, hTE);
		}
		
		ClickLoopAddOn(theWindow);
	}
	
	SetPort(curPort);
	
	return TRUE;
}

void ClickLoopAddOn(WindowRef theWindow)
{
	RgnHandle		rgnHandle;
	
	rgnHandle=NewRgn();
	GetClip(rgnHandle);
	ClipRect(&(theWindow->portRect));
	AdjustVScrollBar(GetWindowVScrollBar(theWindow), GetWindowTE(theWindow));
	SetClip(rgnHandle);
	DisposeRgn(rgnHandle);
}

void AdjustForEndScroll(ControlHandle theControl, TEHandle hTE)
{
	short			numLines;
	short			offset;
	
	if (hTE==0L)
		return;
	
	numLines=TotalNumberOfLines(hTE);
	offset=numLines-GetControlMaximum(theControl)-
		(((**hTE).viewRect.bottom-(**hTE).viewRect.top)/(**hTE).lineHeight);
	if (offset<0)
		TEPinScroll(0, -offset*((**hTE).lineHeight), hTE);
}

#define kTextMargin	2

/* Return a rectangle that is inset from the portRect by the size of
	the scrollbars and a little extra margin. */

void GetTERect(WindowRef theWindow, Rect *teRect, Boolean adjustForScrollBars)
{
	RequireWindow();
	
	*teRect = theWindow->portRect;
	InsetRect(teRect, kTextMargin, kTextMargin);	/* adjust for margin */
	if (adjustForScrollBars)
	{
		teRect->bottom = teRect->bottom - 15;		/* and for the scrollbars */
		teRect->right = teRect->right - 15;
	}
}


/* Update the TERec's view rect so that it is the greatest multiple of
	the lineHeight that still fits in the old viewRect. */

void AdjustViewRect(TEHandle docTE)
{
	TEPtr		te;
	
	if (docTE==0L)
		return;
	
	te = *docTE;
	te->viewRect.bottom = (((te->viewRect.bottom - te->viewRect.top) / te->lineHeight)
							* te->lineHeight) + te->viewRect.top;
}


/*	Re-calculate the position and size of the viewRect and the scrollbars.
	kScrollTweek compensates for off-by-one requirements of the scrollbars
	to have borders coincide with the growbox. */

#define kScrollbarWidth		16
#define kScrollbarAdjust	(kScrollbarWidth-1)
#define kScrollTweek		2

void AdjustScrollSizes(WindowRef window, TEHandle hTE, ControlHandle vScrollBar,
	ControlHandle hScrollBar, short destOverload, short headerHeight)
{
	Rect		teRect;
	
	if (window==0L)
		return;
	if (hTE==0L)
		return;
	
	GetTERect(window, &teRect, TRUE);							/* start with TERect */
	(**hTE).viewRect = teRect;
	(**hTE).viewRect.top+=headerHeight;
	(**hTE).destRect.left=(**hTE).viewRect.left;
	(**hTE).destRect.right=(**hTE).viewRect.right+destOverload;
	MoveControl(vScrollBar, window->portRect.right - kScrollbarAdjust, headerHeight-1);
	SizeControl(vScrollBar, kScrollbarWidth, (window->portRect.bottom - 
				window->portRect.top) - (kScrollbarAdjust - kScrollTweek) - headerHeight);
	MoveControl(hScrollBar, -1, window->portRect.bottom - kScrollbarAdjust);
	SizeControl(hScrollBar, (window->portRect.right - 
				window->portRect.left) - (kScrollbarAdjust - kScrollTweek),
				kScrollbarWidth);
}

void AdjustTE(TEHandle hTE, ControlHandle vScrollBar, ControlHandle hScrollBar)
{
	TEPtr		te;
	
	if (hTE==0L)
		return;
	if (vScrollBar==0L)
		return;
	if (hScrollBar==0L)
		return;
	
	te = *hTE;
	TEScroll((te->viewRect.left - te->destRect.left) -
			GetControlValue(hScrollBar),
			((te->viewRect.top - te->destRect.top)/te->lineHeight) -
				GetControlValue(vScrollBar),
			hTE);
}

void DrawGrowIconNoLines( WindowRef wp, Boolean showLines)
{
    RgnHandle  saveClip;
    GrafPtr    savePort;
    Rect       growBox;
    
    if (showLines)
    {
    	DrawGrowIcon(wp);
    }
    else
    {
    
	    saveClip = NewRgn();
	    if( saveClip == NULL )
	        return;
	
	    GetPort( &savePort );
	    SetPort( wp );
	
	    GetClip( saveClip );
	   
	    growBox.right  = wp->portRect.right;
	    growBox.bottom = wp->portRect.bottom;
	    growBox.left   = growBox.right - 15;
	    growBox.top    = growBox.bottom - 15;
	
	    ClipRect( &growBox );
	    DrawGrowIcon( wp );
	
	    SetClip( saveClip );
	    SetPort( savePort );
	    DisposeRgn( saveClip );
	}
}

void DrawTheShadowBox(Rect theRect, Boolean eraseBackground)
{
	theRect.right-=2;
	theRect.bottom-=2;
	if (eraseBackground)
		EraseRect(&theRect);
	FrameRect(&theRect);
	MoveTo(theRect.left+3, theRect.bottom+1);
	Line(theRect.right-theRect.left-2, 0);
	Line(0, -theRect.bottom+theRect.top+3);
	MoveTo(theRect.left+3, theRect.bottom);
	Line(theRect.right-theRect.left-3, 0);
	Line(0, -theRect.bottom+theRect.top+4);
}

/* Note: this PowerMac click loop procedure is not correct.  This will not run properly if you
   compile and run it for native PowerMac. -MP, 1/23/95 */

#if powerc
pascal Boolean PPCClickLoopProc(TEPtr pTE)
{
	WindowRef		theWindow;
	
	theWindow=GetFrontDocumentWindow();
	if (theWindow==0L)
		theWindow=FrontWindow();
	if (theWindow==0L)
		return FALSE;
	CallTEClickLoopProc(GetWindowOldClickLoopProc(theWindow), pTE);
	ClickLoopAddOn(theWindow);
	return TRUE;
}
#endif

void SetTheDrawingFont(short fontNum, short fontSize, short fontFace, short fontMode)
{
	TextFont(fontNum);
	TextSize(fontSize);
	TextFace(fontFace);
	TextMode(fontMode);
}

void UseDefaultDrawingFont(void)
{
	TextFont(GetSysFont());
	TextSize(GetDefFontSize());
	TextFace(0);
	TextMode(srcCopy);
}

short MyHideTE(TEHandle hTE)
{
	short			oldBottom;
	
	oldBottom=(**hTE).viewRect.bottom;
	(**hTE).viewRect.bottom=(**hTE).viewRect.top;
	return oldBottom;
}

void MyShowTE(TEHandle hTE, short viewBottom)
{
	(**hTE).viewRect.bottom=viewBottom;
	(**hTE).destRect=(**hTE).viewRect;
	TECalText(hTE);
}

short CalculateFontHeight(short font, short fontSize)
{
	FontInfo		theFontInfo;
	
	SetTheDrawingFont(font, fontSize, 0, 0);
	GetFontInfo(&theFontInfo);
	UseDefaultDrawingFont();
	return theFontInfo.ascent+theFontInfo.descent+theFontInfo.leading;
}

short MyGetDefaultFont(void)
{
	return GetSysFont();
}

short MyGetDefaultFontSize(void)
{
	return GetDefFontSize();
}
