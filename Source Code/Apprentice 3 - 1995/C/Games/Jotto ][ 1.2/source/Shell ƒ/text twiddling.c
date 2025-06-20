#include "text twiddling.h"
#include "window layer.h"

static	asm void MyDrawHook(void);

static	ProcPtr		gOldDrawHook;

Boolean AnyTextInScrapQQ(void)
{
	long			dummy;
	
	LoadScrap();
	return (GetScrap(0L, 'TEXT', &dummy)!=noTypeErr);
}

void SetTheText(WindowPtr theWindow, Ptr data, long count)
{
	TEHandle		hTE;
	ControlHandle	vScrollBar;
	
	hTE=GetWindowTE(theWindow);
	if (hTE==0L)
		return;
	
	vScrollBar=GetWindowVScrollBar(theWindow);
	
	TESetText(data, count, hTE);
	TESetSelect(0, 0, hTE);
	if (vScrollBar!=0L)
		AdjustVScrollBar(vScrollBar, hTE);
}

Boolean AnyTextQQ(WindowPtr theWindow)
{
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	if (hTE==0L)
		return FALSE;
	else
		return ((**hTE).teLength!=0);
}

Boolean AnyHighlightedQQ(WindowPtr theWindow)
{
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	if (hTE==0L)
		return FALSE;
	else
		return ((**hTE).selStart!=(**hTE).selEnd);
}

short SelectionStart(WindowPtr theWindow)
{
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	if (hTE==0L)
		return -1;
	
	return (**hTE).selStart;
}

short SelectionEnd(WindowPtr theWindow)
{
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	if (hTE==0L)
		return -1;
	
	return (**hTE).selEnd;
}

Boolean InsertBeforeStart(WindowPtr theWindow, Str255 theStr)
{
	TEHandle		hTE;
	unsigned long	len;
	unsigned long	oldLen;
	
	len=theStr[0];
	hTE=GetWindowTE(theWindow);
	oldLen=(**hTE).teLength;
	if (oldLen+len>32767)
		return FALSE;
	
	TEInsert(&theStr[1], len, hTE);
	return TRUE;
}

Boolean InsertAfterEnd(WindowPtr theWindow, Str255 theStr)
{
	TEHandle		hTE;
	unsigned long	len;
	unsigned long	oldLen;
	unsigned long	oldSelStart;
	unsigned long	oldSelEnd;
	
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
	return TRUE;
}

short TotalNumberOfLines(TEHandle hTE)
{
	short			numLines;
	
	numLines=(**hTE).nLines;
	if (*((unsigned char*)((long)(*((**hTE).hText))+(**hTE).teLength-1))==0x0d)
		numLines++;
	
	return numLines;
}

pascal void ScrollActionProc(ControlHandle theHandle, short partCode)
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
	short			oldValue, oldMax;
	short			numLines, max, value;
	
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
}

short CurrentLineNumber(TEHandle hTE)
{
	short			i;
	short			currentEndPoint;
	short			lineNumber;
	short			numLines;
	
	currentEndPoint=(**hTE).selEnd;
	lineNumber=0;
	numLines=(**hTE).nLines;
	for (i=0; i<numLines; i++)
	{
		if ((**hTE).lineStarts[i]<currentEndPoint)
			lineNumber++;
		else
			return lineNumber;
	}
	
	return numLines;
}

pascal Boolean MyClikLoop(void)
{
	Point			thePoint;
	TEHandle		hTE;
	WindowPtr		theWindow;
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
		
		AdjustVScrollBar(GetWindowVScrollBar(theWindow), hTE);
	}
	
	SetPort(curPort);
	
	return TRUE;
}

void AdjustForEndScroll(ControlHandle theControl, TEHandle hTE)
{
	short			numLines;
	short			offset;
	
	numLines=TotalNumberOfLines(hTE);
	offset=numLines-GetControlMaximum(theControl)-
		(((**hTE).viewRect.bottom-(**hTE).viewRect.top)/(**hTE).lineHeight);
	if (offset<0)
		TEPinScroll(0, -offset*((**hTE).lineHeight), hTE);
}

#define kTextMargin	2

/* Return a rectangle that is inset from the portRect by the size of
	the scrollbars and a little extra margin. */

void GetTERect(WindowPtr window, Rect *teRect, Boolean adjustForScrollBars)
{
	*teRect = window->portRect;
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

void AdjustScrollSizes(WindowPtr window, TEHandle hTE, ControlHandle vScrollBar,
	ControlHandle hScrollBar)
{
	Rect		teRect;
	
	GetTERect(window, &teRect, TRUE);							/* start with TERect */
	(**hTE).viewRect = teRect;
	(**hTE).destRect.left=(**hTE).viewRect.left;
	(**hTE).destRect.right=(**hTE).viewRect.right;
	MoveControl(vScrollBar, window->portRect.right - kScrollbarAdjust, -1);
	SizeControl(vScrollBar, kScrollbarWidth, (window->portRect.bottom - 
				window->portRect.top) - (kScrollbarAdjust - kScrollTweek));
	MoveControl(hScrollBar, -1, window->portRect.bottom - kScrollbarAdjust);
	SizeControl(hScrollBar, (window->portRect.right - 
				window->portRect.left) - (kScrollbarAdjust - kScrollTweek),
				kScrollbarWidth);
}

void AdjustTE(TEHandle hTE, ControlHandle vScrollBar, ControlHandle hScrollBar)
{
	TEPtr		te;
	
	te = *hTE;
	TEScroll((te->viewRect.left - te->destRect.left) -
			GetControlValue(hScrollBar),
			((te->viewRect.top - te->destRect.top)/te->lineHeight) -
				GetControlValue(vScrollBar),
			hTE);
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

void ZapDrawHook(TEHandle hTE)
{
	ProcPtr			temp;
	
	temp=(ProcPtr)MyDrawHook;
	TECustomHook(intDrawHook, &temp, hTE);
	gOldDrawHook=temp;
}

void RestoreDrawHook(TEHandle hTE)
{
	ProcPtr			temp;
	
	temp=gOldDrawHook;
	TECustomHook(intDrawHook, &temp, hTE);
}

static	asm void MyDrawHook(void)
{
	rts
}

#define kGrowBoxSize	15
