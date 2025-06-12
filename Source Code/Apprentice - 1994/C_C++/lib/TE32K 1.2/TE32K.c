#include	"TE32K.h"

#define	EXTRALINESTARTS		32L
#define	EXTRATEXTBUFF		256L

#define	LEFTARROW			28
#define	RIGHTARROW			29
#define UPARROW				30
#define DOWNARROW			31
#define	TAB					'\t'
#define DELETE				0x08
#define	RETURN				0x0D
#define	ENTER				0x03



Handle			TE32KScrpHandle = nil;
long			TE32KScrpLength = 0L;
TE32KHandle		clickedTE32KH = nil;



static long		LineEndIndex(long,TE32KHandle);
static void		CalParagraph(long,TE32KHandle,long *,long *);
static long 	paraLines(long,TE32KHandle);
static void 	updateLine(long,TE32KHandle,int,LongRect *);
static void		invertSelRange(long,long,TE32KHandle);
static void		xorCaret(TE32KHandle);
static long		indexToLine(long,TE32KHandle);
static int		shiftKeyDown(void);
static void 	MyClicker(void);
static void 	MyClickLoop(void);




void	TE32KSetFontStuff(int txFont,int txFace,int txMode,int txSize,TE32KHandle theTE32KHandle)
{
register int		i;
int					oldFont,oldFace,oldSize,oldMode;
GrafPtr				oldPort;
FontInfo			theFontInfo;


	(**theTE32KHandle).txFont = txFont;
	(**theTE32KHandle).txFace = txFace;
	(**theTE32KHandle).txMode = txMode;
	(**theTE32KHandle).txSize = txSize;
	
	GetPort(&oldPort);
	SetPort((**theTE32KHandle).inPort);
	oldFont = ((**theTE32KHandle).inPort)->txFont;
	oldFace = ((**theTE32KHandle).inPort)->txFace;
	oldSize = ((**theTE32KHandle).inPort)->txSize;
	oldMode = ((**theTE32KHandle).inPort)->txMode;
	
	TextFont((**theTE32KHandle).txFont);
	TextFace((**theTE32KHandle).txFace);
	TextSize((**theTE32KHandle).txSize);
	TextMode((**theTE32KHandle).txMode);
	
	
	for (i=0;i<256;i++)
		(**theTE32KHandle).theCharWidths[i] = CharWidth((unsigned char) i);
	
	GetFontInfo(&theFontInfo);
	
	(**theTE32KHandle).lineHeight = theFontInfo.ascent + theFontInfo.descent + theFontInfo.leading;
	(**theTE32KHandle).fontAscent = theFontInfo.ascent;
	
	if ((**theTE32KHandle).tabChars)
		(**theTE32KHandle).tabWidth = (**theTE32KHandle).tabChars * (**theTE32KHandle).theCharWidths[' '];
	
	TextFont(oldFont);
	TextFace(oldFace);
	TextSize(oldSize);
	TextMode(oldMode);
	
	SetPort(oldPort);
}





void	SetLongRect(LongRect *theLongRect,long left,long top,long right,long bottom)
{
	theLongRect->left = left;
	theLongRect->top = top;
	theLongRect->right = right;
	theLongRect->bottom = bottom;
}





void	RectToLongRect(Rect *theRect,LongRect *theLongRect)
{
	theLongRect->left = (long) theRect->left;
	theLongRect->top = (long) theRect->top;
	theLongRect->right = (long) theRect->right;
	theLongRect->bottom = (long) theRect->bottom;
}




void	LongRectToRect(LongRect *theLongRect,Rect *theRect)
{
	if (theLongRect->left < -32768L)
		theRect->left = (int) -32768;
	else if (theLongRect->left > 32767L)
		theRect->left = (int) 32767;
	else
		theRect->left = (int) theLongRect->left;
	
	if (theLongRect->top < -32768L)
		theRect->top = (int) -32768;
	else if (theLongRect->top > 32767L)
		theRect->top = (int) 32767;
	else
		theRect->top = (int) theLongRect->top;
	
	if (theLongRect->right < -32768L)
		theRect->right = (int) -32768;
	else if (theLongRect->right > 32767L)
		theRect->right = (int) 32767;
	else
		theRect->right = (int) theLongRect->right;
	
	if (theLongRect->bottom < -32768L)
		theRect->bottom = (int) -32768;
	else if (theLongRect->bottom > 32767L)
		theRect->bottom = (int) 32767;
	else
		theRect->bottom = (int) theLongRect->bottom;
}



void    OffsetLongRect(LongRect *theLongRect, long x, long y)
{
        theLongRect->left += x;
        theLongRect->top += y;
        theLongRect->right += x;
        theLongRect->bottom += y;
}




static long	indexToLine(long selIndex,TE32KHandle theTE32KHandle)
{
register long	i,delta;

	if (theTE32KHandle)
	{
		if (selIndex<=0L || (**theTE32KHandle).nLines<=1L || (**theTE32KHandle).teLength<1L)
			return(0L);
		
		else if (selIndex >= (**theTE32KHandle).teLength)
			return((long) ((**theTE32KHandle).nLines - 1L));
		
		else
		{
			i = ((**theTE32KHandle).nLines) >> 1;
			
			delta = ((**theTE32KHandle).nLines) >> 1;
			if (delta < 1L)
				delta = 1L;
			
			while (delta > 0L)
			{
				if (selIndex == (**theTE32KHandle).lineStarts[i])
					delta = 0L;
				
				else if (selIndex > (**theTE32KHandle).lineStarts[i])
				{
					if (selIndex < (**theTE32KHandle).lineStarts[i+1])
						delta = 0L;
					
					else
						i += delta;
				}
				
				else
					i -= delta;
				
				if (delta)
				{
					delta >>= 1;
					
					if (delta < 1L)
						delta = 1L;
				}
			}
		}
		
		if (i < 0L)
			i = 0L;
		else if (i >= (**theTE32KHandle).nLines)
			i = (**theTE32KHandle).nLines - 1L;
		
		return((long) i);
	}
	
	else
		return((long) 0L);
}







static void	xorCaret(TE32KHandle theTE32KHandle)
{
GrafPtr			oldPort;
PenState		oldPenState;
Point			selPt;
RgnHandle		oldClipRgn;
Rect			theClipRect;

	if (theTE32KHandle && (**theTE32KHandle).active && (**theTE32KHandle).selStart==(**theTE32KHandle).selEnd)
	{
		if (!(**theTE32KHandle).caretState && ((**theTE32KHandle).selStart < 0 || (**theTE32KHandle).selEnd > (**theTE32KHandle).teLength))
			return;
		
		GetPort(&oldPort);
		SetPort((**theTE32KHandle).inPort);
		
		GetPenState(&oldPenState);
		oldClipRgn = NewRgn();
		GetClip(oldClipRgn);
		
		theClipRect.left = (int) ((**theTE32KHandle).viewRect.left);
		theClipRect.top = (int) ((**theTE32KHandle).viewRect.top);
		theClipRect.right = (int) ((**theTE32KHandle).viewRect.right);
		theClipRect.bottom = (int) ((**theTE32KHandle).viewRect.bottom);
		
		ClipRect(&theClipRect);
		
		PenNormal();
		
		PenMode(patXor);
		
		if ((**theTE32KHandle).selPoint.h < -32768L)
			selPt.h = (int) -32768;
		else if ((**theTE32KHandle).selPoint.h > 32767L)
			selPt.h = (int) 32767;
		else
			selPt.h = (int) (**theTE32KHandle).selPoint.h;
		
		if ((**theTE32KHandle).selPoint.v < -32768L)
			selPt.v = (int) -32768;
		else if ((**theTE32KHandle).selPoint.v > 32767L)
			selPt.v = (int) 32767;
		else
			selPt.v = (int) (**theTE32KHandle).selPoint.v;
		
		MoveTo(selPt.h - 1,selPt.v);
		Line(0,-(**theTE32KHandle).fontAscent);
		
		(**theTE32KHandle).caretTime = TickCount() + GetCaretTime();
		(**theTE32KHandle).caretState = !(**theTE32KHandle).caretState;
		
		SetClip(oldClipRgn);
		DisposeRgn(oldClipRgn);
		
		SetPenState(&oldPenState);
		SetPort(oldPort);
	}
}




void	TE32KInit()
{
	TE32KScrpHandle = NewHandle(0L);
	TE32KScrpLength = 0L;
}






TE32KHandle		TE32KNew(LongRect *destRect,LongRect *viewRect)
{
TE32KHandle		newTE32KHandle;
Handle			hText;
GrafPtr			activePort;
FontInfo		theFontInfo;
LongPoint		selPt;

	newTE32KHandle = (TE32KHandle) NewHandle((long) sizeof(TE32KRec) + (long) sizeof(long)*EXTRALINESTARTS);
	if (MemError() || StripAddress(newTE32KHandle)==nil)
		return((TE32KHandle) nil);
	
	hText = NewHandle(EXTRATEXTBUFF);
	if (MemError() || StripAddress(hText)==nil)
	{
		DisposHandle((Handle) newTE32KHandle);
		return((TE32KHandle) nil);
	}
	
	(**newTE32KHandle).destRect = *destRect;
	(**newTE32KHandle).viewRect = *viewRect;
	
	GetPort(&activePort);
	GetFontInfo(&theFontInfo);
	
	(**newTE32KHandle).lineHeight = theFontInfo.ascent + theFontInfo.descent + theFontInfo.leading;
	(**newTE32KHandle).fontAscent = theFontInfo.ascent;
	
	(**newTE32KHandle).selStart = 0L;
	(**newTE32KHandle).selEnd = 0L;
	
	(**newTE32KHandle).teLength = 0L;
	(**newTE32KHandle).hText = hText;
	
	(**newTE32KHandle).txFont = activePort->txFont;
	(**newTE32KHandle).txFace = activePort->txFace;
	(**newTE32KHandle).txMode = activePort->txMode;
	(**newTE32KHandle).txSize = activePort->txSize;
	
	(**newTE32KHandle).inPort = activePort;
	
	(**newTE32KHandle).tabWidth = 25;
	(**newTE32KHandle).tabChars = 0;
	
	(**newTE32KHandle).maxLineWidth = 32767;
	
	(**newTE32KHandle).clikStuff = FALSE;
	
	(**newTE32KHandle).crOnly = 0x00;
	
	(**newTE32KHandle).nLines = 1L;
	(**newTE32KHandle).lineStarts[0] = 0L;
	(**newTE32KHandle).lineStarts[1] = 0L;
	
	(**newTE32KHandle).active = TRUE;
	(**newTE32KHandle).caretState = FALSE;
	(**newTE32KHandle).caretTime = TickCount();
	
	(**newTE32KHandle).clickTime = TickCount();
	(**newTE32KHandle).clickLoc = -1L;
	
	TE32KGetPoint((**newTE32KHandle).selStart,&selPt,newTE32KHandle);
	
	(**newTE32KHandle).selPoint = selPt;
	
	(**newTE32KHandle).clikLoop = nil;
	
	TE32KSetFontStuff((**newTE32KHandle).txFont,(**newTE32KHandle).txFace,(**newTE32KHandle).txMode,(**newTE32KHandle).txSize,newTE32KHandle);
	
	return((TE32KHandle) newTE32KHandle);
}





void TE32KDispose(TE32KHandle theTE32KHandle)
{
	if (theTE32KHandle)
	{
		if ((**theTE32KHandle).hText)
			DisposHandle((**theTE32KHandle).hText);
		
		DisposHandle(theTE32KHandle);
	}
}




void	TE32KCalText(TE32KHandle theTE32KHandle)
{
register unsigned char	*charPtr;
register long			charCount;
register int			*theCharWidths,lineLength,crOnly,maxLineWidth;
register unsigned char	ch;
long					nLines,maxLineStarts,sizeTE32KHandle;
unsigned char			*charBase;
Point					cursorPt;
int						rightSide,destLeftSide,tabWidth,maxRewind;
unsigned char			*oldCharPtr;
long					oldCharCount,tempOffset;

	if (theTE32KHandle)
	{
		(**theTE32KHandle).lineStarts[0] = 0L;		/* assume the worst can happen and prepare for it */
		(**theTE32KHandle).lineStarts[1] = 0L;
		(**theTE32KHandle).nLines = 1L;
		
		sizeTE32KHandle  = GetHandleSize((Handle) theTE32KHandle);
		maxLineStarts = (sizeTE32KHandle - (long) sizeof(TE32KRec))/(long) sizeof(long) - 2;
		
		crOnly = (**theTE32KHandle).crOnly;
		maxLineWidth = (**theTE32KHandle).maxLineWidth;
		
		lineLength = 0L;
		nLines = 0L;
		
		charBase = (unsigned char *) *((**theTE32KHandle).hText);
		charPtr = charBase;
		charCount = (**theTE32KHandle).teLength;
		
		
		if (charCount > 0L)
		{
			rightSide = (int) ((**theTE32KHandle).destRect.right);
			destLeftSide = (int) ((**theTE32KHandle).destRect.left + 1L);
			cursorPt.h = destLeftSide;
			tabWidth = (long) (**theTE32KHandle).tabWidth;
			
			theCharWidths = (**theTE32KHandle).theCharWidths;
			
			while (charCount--)
			{
				ch = *charPtr++;
				lineLength++;
				
				if (!crOnly)
				{
					if (ch == TAB)
						cursorPt.h = destLeftSide + ((cursorPt.h - destLeftSide + tabWidth)/tabWidth)*tabWidth;
					else if (ch != '\r' && ch != '\n')
						cursorPt.h += theCharWidths[ch];
				}
				
				if ((ch == '\r' || ch == '\n') || (ch != ' ' && cursorPt.h >= rightSide) || (!crOnly && lineLength > maxLineWidth))
				{
					if ((ch != ' ' && cursorPt.h >= rightSide) || (!crOnly && lineLength > maxLineWidth))
					{
						/* I should probably add a hook for custom word-breaking */
						
						maxRewind = charPtr - charBase - (**theTE32KHandle).lineStarts[nLines];
						oldCharPtr = charPtr;
						oldCharCount = charCount;
						
						charPtr--;
						charCount++;
						maxRewind--;
						
						while (*charPtr != ' ' && maxRewind > 0)
						{
							charPtr--;
							charCount++;
							maxRewind--;
						}
						
						if (maxRewind <= 0)
						{
							charPtr = oldCharPtr;
							charCount = oldCharCount;
						}
						
						else
						{
							charPtr++;
							charCount--;
						}
					}
					
					if (nLines >= maxLineStarts)
					{
						tempOffset = charPtr - charBase;
												
						sizeTE32KHandle = (long) sizeof(TE32KRec) + (long) sizeof(long)*(nLines + EXTRALINESTARTS);
						maxLineStarts = (sizeTE32KHandle - (long) sizeof(TE32KRec))/(long) sizeof(long) - 2;
						
						SetHandleSize((Handle) theTE32KHandle,sizeTE32KHandle);
						
						if (MemError())
							return;
						
						charBase = (unsigned char *) *((**theTE32KHandle).hText);
						charPtr = charBase + tempOffset;
						theCharWidths = (**theTE32KHandle).theCharWidths;
					}
					
					(**theTE32KHandle).lineStarts[++nLines] = charPtr - charBase;
					
					cursorPt.h = destLeftSide;
					lineLength = 0L;
				}
			}
			
			if (nLines >= maxLineStarts)
			{
				sizeTE32KHandle = (long) sizeof(TE32KRec) + (long) sizeof(long)*(nLines + EXTRALINESTARTS);
				
				SetHandleSize((Handle) theTE32KHandle,sizeTE32KHandle);
				
				if (MemError())
					return;
			}
			
			(**theTE32KHandle).lineStarts[++nLines] = charPtr - charBase;
			
			(**theTE32KHandle).nLines = nLines;
		}
	}
}





void	TE32KUseTextHandle(Handle hText,TE32KHandle theTE32KHandle)
{
LongPoint	selPt;
long int textLength;

	if (theTE32KHandle)
	{
		textLength = GetHandleSize(hText);

		if ((**theTE32KHandle).hText)
			DisposHandle((**theTE32KHandle).hText);
				
		(**theTE32KHandle).hText = hText;
		(**theTE32KHandle).teLength = textLength;
		
		(**theTE32KHandle).selStart = textLength;
		(**theTE32KHandle).selEnd = textLength;
		
		TE32KGetPoint((**theTE32KHandle).selStart,&selPt,theTE32KHandle);
		(**theTE32KHandle).selPoint = selPt;
		
		TE32KCalText(theTE32KHandle);
	}
}





void	TE32KSetText(Ptr textPtr,long textLength,TE32KHandle theTE32KHandle)
{
Handle		hText;
LongPoint	selPt;

	if (theTE32KHandle)
	{
		hText = NewHandle(textLength + EXTRATEXTBUFF);
		
		if (MemError() || StripAddress(hText)==nil)
			return;
		
		if ((**theTE32KHandle).hText)
			DisposHandle((**theTE32KHandle).hText);
		
		HLock(hText);
		BlockMove(textPtr,*hText,textLength);
		HUnlock(hText);
		
		(**theTE32KHandle).hText = hText;
		(**theTE32KHandle).teLength = textLength;
		
		(**theTE32KHandle).selStart = textLength;
		(**theTE32KHandle).selEnd = textLength;
		
		TE32KGetPoint((**theTE32KHandle).selStart,&selPt,theTE32KHandle);
		(**theTE32KHandle).selPoint = selPt;
		
		TE32KCalText(theTE32KHandle);
	}
}



Handle	TE32KGetText(TE32KHandle theTE32KHandle)
{
	if (theTE32KHandle) 
	{
		Handle hText = (**theTE32KHandle).hText;
		HandToHand(&hText);
		return hText;
	} 
	
	else
		return nil;
}




void	TE32KUpdate(LongRect *updateLongRect,TE32KHandle theTE32KHandle)
{
LongRect					tempLongRect;
Rect						theClipRect,viewRect,updateRect;
GrafPtr						oldPort,currentPort;
RgnHandle					oldClipRgn;
register unsigned char		*textPtr;
register long				firstLine,lastLine,i,thisStart,nextStart,tabWidth;
Point						cursorPt;
int							oldFont,oldFace,oldSize,oldMode;
int							rightSide,destLeftSide;
LongPoint					selPt;
unsigned char				oldCaretState;


	if (theTE32KHandle && (**theTE32KHandle).inPort)
	{
		tempLongRect = (**theTE32KHandle).viewRect;
		LongRectToRect(&tempLongRect,&viewRect);
		
		tempLongRect = *updateLongRect;
		tempLongRect.top = (**theTE32KHandle).destRect.top + ((updateLongRect->top - (**theTE32KHandle).destRect.top)/(**theTE32KHandle).lineHeight)*(**theTE32KHandle).lineHeight;
		tempLongRect.bottom = (**theTE32KHandle).destRect.top + ((updateLongRect->bottom - (**theTE32KHandle).destRect.top + (**theTE32KHandle).lineHeight - 1L)/(**theTE32KHandle).lineHeight)*(**theTE32KHandle).lineHeight;
		
		LongRectToRect(&tempLongRect,&updateRect);
		
		if (SectRect(&viewRect,&updateRect,&theClipRect))
		{
			GetPort(&oldPort);
			currentPort = (**theTE32KHandle).inPort;
			SetPort(currentPort);
			
			oldClipRgn = NewRgn();
			GetClip(oldClipRgn);
			ClipRect(&theClipRect);
			
			oldCaretState = (**theTE32KHandle).caretState;
			
			if ((**theTE32KHandle).selStart == (**theTE32KHandle).selEnd && oldCaretState)
				xorCaret(theTE32KHandle);
			
			firstLine = ((long) theClipRect.top - (**theTE32KHandle).destRect.top)/(long) (**theTE32KHandle).lineHeight;
			lastLine = ((long) theClipRect.bottom - (**theTE32KHandle).destRect.top - 1L)/(long) (**theTE32KHandle).lineHeight;
			
			if (firstLine < 0)
				firstLine = 0;
			
			if (lastLine >= (**theTE32KHandle).nLines)
				lastLine = (**theTE32KHandle).nLines - 1L;
			
			if (firstLine > lastLine)
				lastLine = firstLine;
			
			EraseRect(&theClipRect);
			
			if (firstLine < (**theTE32KHandle).nLines && (**theTE32KHandle).teLength > 0L)
			{
				rightSide = theClipRect.right;
				destLeftSide = (int) (**theTE32KHandle).destRect.left + 1L;
				cursorPt.h = destLeftSide;
				cursorPt.v = (int) ((**theTE32KHandle).destRect.top + firstLine * (long) (**theTE32KHandle).lineHeight + (long) (**theTE32KHandle).fontAscent);
				
				oldFont = ((**theTE32KHandle).inPort)->txFont;
				oldFace = ((**theTE32KHandle).inPort)->txFace;
				oldSize = ((**theTE32KHandle).inPort)->txSize;
				oldMode = ((**theTE32KHandle).inPort)->txMode;
				
				TextFont((**theTE32KHandle).txFont);
				TextFace((**theTE32KHandle).txFace);
				TextSize((**theTE32KHandle).txSize);
				TextMode((**theTE32KHandle).txMode);
				
				HLock((**theTE32KHandle).hText);
				
				textPtr = (unsigned char *) *((**theTE32KHandle).hText);
				tabWidth = (long) (**theTE32KHandle).tabWidth;
				
				while (firstLine <= lastLine)
				{
					thisStart = (**theTE32KHandle).lineStarts[firstLine];
					i = thisStart;
					
					nextStart = (**theTE32KHandle).lineStarts[firstLine+1];
					
					if (nextStart > thisStart && (textPtr[nextStart-1] == '\r' || textPtr[nextStart-1] == '\n'))
						nextStart--;
					
					MoveTo(cursorPt.h,cursorPt.v);
					
					while (thisStart < nextStart)
					{
						while (i<nextStart && textPtr[i]!=TAB)
							i++;
						
						if (i > thisStart)
							DrawText(&(textPtr[thisStart]),0,(int) (i - thisStart));
						
						if (i<nextStart && textPtr[i]==TAB)
						{
							MoveTo(destLeftSide + ((currentPort->pnLoc.h - destLeftSide + tabWidth)/tabWidth)*tabWidth,currentPort->pnLoc.v);
							i++;
						}
						
						thisStart = i;
						
						if (currentPort->pnLoc.h > theClipRect.right)
							thisStart = nextStart;
					}
					
					firstLine++;
					cursorPt.v += (**theTE32KHandle).lineHeight;
				}
				
				HUnlock((**theTE32KHandle).hText);
				
				TextFont(oldFont);
				TextFace(oldFace);
				TextSize(oldSize);
				TextMode(oldMode);
			}
			
			if ((**theTE32KHandle).selStart < (**theTE32KHandle).selEnd)
				invertSelRange((**theTE32KHandle).selStart,(**theTE32KHandle).selEnd,theTE32KHandle);
			
			else
			{
				TE32KGetPoint((**theTE32KHandle).selStart,&selPt,theTE32KHandle);
				(**theTE32KHandle).selPoint = selPt;
				
				if (oldCaretState)
					xorCaret(theTE32KHandle);
			}
			
			SetClip(oldClipRgn);
			DisposeRgn(oldClipRgn);
			
			SetPort(oldPort);
		}
	}
}





void	TE32KScroll(long horiz,long vert,TE32KHandle theTE32KHandle)
{
LongRect	updateLongRect;
Rect		scrollRect;
RgnHandle	updateRgn;
GrafPtr		oldPort;
LongPoint	selPt;

	if (theTE32KHandle && (**theTE32KHandle).inPort && (horiz || vert))
	{
		GetPort(&oldPort);
		SetPort((**theTE32KHandle).inPort);
		
		(**theTE32KHandle).destRect.left += horiz;
		(**theTE32KHandle).destRect.top += vert;
		(**theTE32KHandle).destRect.right += horiz;
		(**theTE32KHandle).destRect.bottom += vert;
		
		(**theTE32KHandle).selPoint.h += horiz;
		(**theTE32KHandle).selPoint.v += vert;
		selPt = (**theTE32KHandle).selPoint;
		
		scrollRect.left = (int) ((**theTE32KHandle).viewRect.left);
		scrollRect.top = (int) ((**theTE32KHandle).viewRect.top);
		scrollRect.right = (int) ((**theTE32KHandle).viewRect.right);
		scrollRect.bottom = (int) ((**theTE32KHandle).viewRect.bottom);
		
		if (horiz < ((**theTE32KHandle).viewRect.right-(**theTE32KHandle).viewRect.left) ||
			vert < ((**theTE32KHandle).viewRect.bottom-(**theTE32KHandle).viewRect.top))
		{
			updateRgn = NewRgn();
			
			ScrollRect(&scrollRect,(int) horiz,(int) vert,updateRgn);
			
			updateLongRect.left = (**updateRgn).rgnBBox.left;
			updateLongRect.top = (**updateRgn).rgnBBox.top;
			updateLongRect.right = (**updateRgn).rgnBBox.right;
			updateLongRect.bottom = (**updateRgn).rgnBBox.bottom;
			
			DisposeRgn(updateRgn);
			
			TE32KUpdate(&updateLongRect,theTE32KHandle);
			
			if ((**theTE32KHandle).caretState)
				xorCaret(theTE32KHandle);
			
			(**theTE32KHandle).selPoint = selPt;
			
			xorCaret(theTE32KHandle);
		}
		
		else
		{
			updateLongRect = (**theTE32KHandle).viewRect;
			
			TE32KUpdate(&updateLongRect,theTE32KHandle);
			
			if ((**theTE32KHandle).caretState)
				xorCaret(theTE32KHandle);
			
			(**theTE32KHandle).selPoint = selPt;
			
			xorCaret(theTE32KHandle);
		}
		
		SetPort(oldPort);
	}
}





void	TE32KActivate(TE32KHandle theTE32KHandle)
{
	if (theTE32KHandle && !((**theTE32KHandle).active))
	{
		(**theTE32KHandle).active = TRUE;
		(**theTE32KHandle).caretState = FALSE;
		
		invertSelRange((**theTE32KHandle).selStart,(**theTE32KHandle).selEnd,theTE32KHandle);
	}
}





void	TE32KIdle(TE32KHandle theTE32KHandle)
{
	if (theTE32KHandle && (**theTE32KHandle).active && TickCount() >= (**theTE32KHandle).caretTime)
	{
		if ((**theTE32KHandle).selStart == (**theTE32KHandle).selEnd)
			xorCaret(theTE32KHandle);
	}
}





void	TE32KDeactivate(TE32KHandle theTE32KHandle)
{
	if (theTE32KHandle && (**theTE32KHandle).active)
	{
		if ((**theTE32KHandle).selStart == (**theTE32KHandle).selEnd)
		{
			if ((**theTE32KHandle).caretState)
				xorCaret(theTE32KHandle);
		}
		else
			invertSelRange((**theTE32KHandle).selStart,(**theTE32KHandle).selEnd,theTE32KHandle);
		
		(**theTE32KHandle).active = FALSE;
	}
}



void	TE32KGetPoint(long selIndex,LongPoint *selPt,TE32KHandle theTE32KHandle)
{
register unsigned char	*textPtr;
register int			*theCharWidths;
register long			i,thisStart,tabWidth;
long					x,y,lineIndex,destLeftSide;
unsigned char			ch;
LongPoint				origPt;
int						clikStuff;

	if (theTE32KHandle)
	{
		if (selIndex<=0L || (**theTE32KHandle).teLength<1L)
		{
			selPt->h = (**theTE32KHandle).destRect.left + 1L;
			selPt->v = (**theTE32KHandle).destRect.top + (**theTE32KHandle).fontAscent;
			(**theTE32KHandle).clikStuff = FALSE;
			
			return;
		}
		
		clikStuff = (**theTE32KHandle).clikStuff;
		(**theTE32KHandle).clikStuff = FALSE;
		
		origPt = *selPt;
		
		i = indexToLine(selIndex,theTE32KHandle);
		
		y = (**theTE32KHandle).destRect.top + ((**theTE32KHandle).lineHeight * i) + (**theTE32KHandle).fontAscent;
		
		selPt->v = y;
		
		if (!(**theTE32KHandle).crOnly && clikStuff && i > 0 && selIndex == (**theTE32KHandle).lineStarts[i])
		{
			i--;
			selPt->v -= (**theTE32KHandle).lineHeight;
		}
		
		else if (selIndex < (**theTE32KHandle).lineStarts[i] || (selIndex == (**theTE32KHandle).lineStarts[i] && i < 1))
		{
			selPt->h = (**theTE32KHandle).destRect.left + 1L;
			return;
		}
		
		
		HLock((**theTE32KHandle).hText);
		
		lineIndex  = i;
		textPtr = (unsigned char *) *((**theTE32KHandle).hText);
		
		destLeftSide = (**theTE32KHandle).destRect.left + 1L;
		x = destLeftSide;
		
		thisStart = (**theTE32KHandle).lineStarts[lineIndex];
		
		theCharWidths = (**theTE32KHandle).theCharWidths;
		
		if (textPtr[selIndex-1] != '\r' && textPtr[selIndex-1] != '\n')
		{
			tabWidth = (long) (**theTE32KHandle).tabWidth;
			
			while (thisStart < selIndex)
			{
				ch = textPtr[thisStart++];
				
				if (ch == TAB)
					x = destLeftSide + ((x - destLeftSide + tabWidth)/tabWidth)*tabWidth;
				
				else
					x += theCharWidths[ch];
			}
		}
		
		HUnlock((**theTE32KHandle).hText);
		
		selPt->h = x;
	}
}




long	TE32KGetOffset(LongPoint *selPt,TE32KHandle theTE32KHandle)
{
register unsigned char	*textPtr;
register int			*theCharWidths;
register long			i,delta,firstChar,lastChar,tabWidth;
unsigned char			done;
long					x,y,selIndex,horiz,destLeftSide;

	if (theTE32KHandle)
	{
		if ((**theTE32KHandle).teLength < 1L)
			return(0L);
		
		horiz = selPt->h;
		
		y = selPt->v - (**theTE32KHandle).destRect.top;
		
		i = y / (long) (**theTE32KHandle).lineHeight;
		
		if (i < 0L)
			return(0L);
		
		if (i >= (**theTE32KHandle).nLines)
			return((**theTE32KHandle).teLength);
		
		theCharWidths = (**theTE32KHandle).theCharWidths;
		
		HLock((**theTE32KHandle).hText);
		
		textPtr = (unsigned char *) *((**theTE32KHandle).hText);
		
		destLeftSide = (**theTE32KHandle).destRect.left + 1L;
		x = destLeftSide;
		delta = 0L;
		
		firstChar = (**theTE32KHandle).lineStarts[i];
		lastChar = (**theTE32KHandle).lineStarts[i+1L];
		
		tabWidth = (long) (**theTE32KHandle).tabWidth;
		
		if (firstChar<lastChar && x+delta<horiz)
		{
			done = FALSE;
		
			while (!done)
			{
				if (textPtr[firstChar] != TAB)
					delta = (long) theCharWidths[textPtr[firstChar]];
				
				else
					delta = (destLeftSide + ((x - destLeftSide + tabWidth)/tabWidth)*tabWidth) - x;
				
				firstChar++;
				
				if (firstChar >= lastChar)
				{	
					if (textPtr[lastChar - 1L] == '\r' || textPtr[lastChar - 1L] == '\n')
						selIndex = lastChar - 1L;
					else
						selIndex = lastChar;
					
					done = TRUE;
				}
				
				else if (x+delta >= horiz)
				{
					if (horiz >= x + (delta >> 1))
						selIndex = firstChar;
					else
						selIndex = --firstChar;
					
					done = TRUE;
				}
				
				else
					x += delta;
			}
		}
		
		else
			selIndex = firstChar;

		HUnlock((**theTE32KHandle).hText);
		
		return(selIndex);
	}
}







static void	invertSelRange(long selStart,long selEnd,TE32KHandle theTE32KHandle)
{
long		firstLine,lastLine,oldMode;
Rect		viewRect,tempRect1,tempRect2,tempRect3,theRect;
LongPoint	selPt;
GrafPtr		oldPort;


	if (theTE32KHandle && (**theTE32KHandle).active)
	{
		if ((**theTE32KHandle).caretState)
			xorCaret(theTE32KHandle);
		
		if (selStart == selEnd)
		{
			TE32KGetPoint(selStart,&selPt,theTE32KHandle);
			(**theTE32KHandle).selPoint = selPt;
			(**theTE32KHandle).selStart = selStart;
			(**theTE32KHandle).selEnd = selStart;
			
			xorCaret(theTE32KHandle);
			
			return;
		}
		
		
		viewRect.left = (int) ((**theTE32KHandle).viewRect.left);
		viewRect.top = (int) ((**theTE32KHandle).viewRect.top);
		viewRect.right = (int) ((**theTE32KHandle).viewRect.right);
		viewRect.bottom = (int) ((**theTE32KHandle).viewRect.bottom);
		
		GetPort(&oldPort);
		SetPort((**theTE32KHandle).inPort);
		
		if (selStart > selEnd)
		{
			firstLine = selStart;
			selStart = selEnd;
			selEnd = firstLine;
		}
		
		firstLine = indexToLine(selStart,theTE32KHandle);
		lastLine = indexToLine(selEnd,theTE32KHandle);
		
		
		TE32KGetPoint(selStart,&selPt,theTE32KHandle);
		
		selPt.v -= (**theTE32KHandle).fontAscent;
		
		if (selStart <= (**theTE32KHandle).lineStarts[firstLine])
			selPt.h--;
		
		if (selPt.h < -32768L)
			tempRect1.left = (int) -32768;
		else if (selPt.h > 32767L)
			tempRect1.left = (int) 32767;
		else
			tempRect1.left = (int) selPt.h;
		
		if (selPt.v < -32768L)
			tempRect1.top = (int) -32768;
		else if (selPt.v > 32767L)
			tempRect1.top = (int) 32767;
		else
			tempRect1.top = (int) selPt.v;
		
		
		if (firstLine != lastLine)
		{
			tempRect1.right = viewRect.right;
			tempRect1.bottom = tempRect1.top + (**theTE32KHandle).lineHeight;
		}
		
		else
		{
			TE32KGetPoint(selEnd,&selPt,theTE32KHandle);
			
			selPt.v -= (**theTE32KHandle).fontAscent;
			selPt.v += (**theTE32KHandle).lineHeight;
			
			if (selPt.h < -32768L)
				tempRect1.right = (int) -32768;
			else if (selPt.h > 32767L)
				tempRect1.right = (int) 32767;
			else
				tempRect1.right = (int) selPt.h;
			
			if (selPt.v < -32768L)
				tempRect1.bottom = (int) -32768;
			else if (selPt.v > 32767L)
				tempRect1.bottom = (int) 32767;
			else
				tempRect1.bottom = (int) selPt.v;
		}
		
		if (SectRect(&viewRect,&tempRect1,&theRect))
		{
			oldMode = HiliteMode;
			HiliteMode = 1;
			InvertRect(&theRect);
			HiliteMode = oldMode;
		}
		

		
		if (lastLine > firstLine + 1L)
		{
			TE32KGetPoint(selEnd,&selPt,theTE32KHandle);
			
			tempRect2.left = viewRect.left;
			tempRect2.top = tempRect1.bottom;
			tempRect2.right = viewRect.right;
			
			selPt.v -= (**theTE32KHandle).fontAscent;
			
			if (selPt.v < -32768L)
				tempRect2.bottom = (int) -32768;
			else if (selPt.v > 32767L)
				tempRect2.bottom = (int) 32767;
			else
				tempRect2.bottom = (int) selPt.v;
			
			selPt.v += (**theTE32KHandle).fontAscent;
			
			if (SectRect(&viewRect,&tempRect2,&theRect))
			{
				oldMode = HiliteMode;
				HiliteMode = 1;
				InvertRect(&theRect);
				HiliteMode = oldMode;
			}
		}
		
		if (lastLine > firstLine && selEnd > (**theTE32KHandle).lineStarts[lastLine])
		{
			if (lastLine == firstLine + 1L)
			{
				TE32KGetPoint(selEnd,&selPt,theTE32KHandle);
			}
			
			selPt.v -= (**theTE32KHandle).fontAscent;
			
			if (selPt.v < -32768L)
				tempRect3.top = (int) -32768;
			else if (selPt.v > 32767L)
				tempRect3.top = (int) 32767;
			else
				tempRect3.top = (int) selPt.v;
			
			selPt.v += (**theTE32KHandle).lineHeight;
			
			if (selPt.v < -32768L)
				tempRect3.bottom = (int) -32768;
			else if (selPt.v > 32767L)
				tempRect3.bottom = (int) 32767;
			else
				tempRect3.bottom = (int) selPt.v;
			
			
			tempRect3.left = viewRect.left;
			
			if (selPt.h < -32768L)
				tempRect3.right = (int) -32768;
			else if (selPt.h > 32767L)
				tempRect3.right = (int) 32767;
			else
				tempRect3.right = (int) selPt.h;
			
			
			if (SectRect(&viewRect,&tempRect3,&theRect))
			{
				oldMode = HiliteMode;
				HiliteMode = 1;
				InvertRect(&theRect);
				HiliteMode = oldMode;
			}
		}
		
		
		SetPort(oldPort);
	}
}





void	TE32KClick(Point startPoint,unsigned char extend,TE32KHandle theTE32KHandle)
{
register long				selIndex,selAnchor,selLast,teLength;
register unsigned char		*textPtr;
LongPoint					selPt,tempPt;
Point						mousePt;
GrafPtr						oldPort;
unsigned char				ch;
long						oldClickLoc,wordStart,wordEnd;


	if (theTE32KHandle && (**theTE32KHandle).active)
	{
		clickedTE32KH = theTE32KHandle;
		
		teLength = (**theTE32KHandle).teLength;
		
		selPt.h = (long) startPoint.h;
		selPt.v = (long) startPoint.v;
		
		selIndex = TE32KGetOffset(&selPt,theTE32KHandle);
		
		oldClickLoc = (**theTE32KHandle).clickLoc;
		(**theTE32KHandle).clickLoc = selIndex;

		if ((**theTE32KHandle).caretState)
			xorCaret(theTE32KHandle);
		
		if (!extend && teLength > 0L && TickCount() < (**theTE32KHandle).clickTime + GetDblTime() && oldClickLoc == selIndex)
		{
			if ((**theTE32KHandle).selStart != (**theTE32KHandle).selEnd)
				invertSelRange((**theTE32KHandle).selStart,(**theTE32KHandle).selEnd,theTE32KHandle);
			
			selAnchor = selIndex;
			selIndex = selAnchor;
			textPtr = (unsigned char *) *((**theTE32KHandle).hText);
			
			while (selIndex > 0L)
			{
				ch = textPtr[selIndex-1L];
				
				if ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z') || (ch>='0' && ch<='9'))
					selIndex--;
				else
					break;
			}
			
			if (selIndex < 0L)
				(**theTE32KHandle).selStart =  0L;
			else
				(**theTE32KHandle).selStart = selIndex;
			
			wordStart = (**theTE32KHandle).selStart;
			selIndex = selAnchor;
			teLength = (**theTE32KHandle).teLength;
			
			while (selIndex < teLength)
			{
				ch = textPtr[selIndex];
				
				if ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z') || (ch>='0' && ch<='9'))
					selIndex++;
				else
					break;
			}
			
			if (selIndex > teLength)
				(**theTE32KHandle).selEnd =  teLength;
			else
				(**theTE32KHandle).selEnd = selIndex;
			
			wordEnd = (**theTE32KHandle).selEnd;
			
			invertSelRange((**theTE32KHandle).selStart,(**theTE32KHandle).selEnd,theTE32KHandle);
			
			while (Button()) 
			{
				if ((**theTE32KHandle).clikLoop)
					(*((**theTE32KHandle).clikLoop)) ();
				
				GetMouse(&mousePt);
				selPt.h = mousePt.h;
				selPt.v = mousePt.v;
			
				selIndex = TE32KGetOffset(&selPt,theTE32KHandle);
				
				if (selIndex < 0)
					selIndex = 0;
				
				else if (selIndex > teLength)
					selIndex = teLength;
				
				if (selIndex < wordStart)
					while (selIndex > 0) 
					{
						ch = textPtr[selIndex - 1];
						
						if ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z') || (ch>='0' && ch<='9'))
							selIndex--;
						else
							break;
					}
				
				else if (selIndex > wordEnd)
					while (selIndex < teLength) 
					{
						ch = textPtr[selIndex];
						
						if ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z') || (ch>='0' && ch<='9'))
							selIndex++;
						else
							break;
					}
			
				if (selIndex < wordEnd && wordEnd < (**theTE32KHandle).selEnd) 
				{
					invertSelRange(wordEnd, (**theTE32KHandle).selEnd, theTE32KHandle);
					(**theTE32KHandle).selEnd = wordEnd;
				}
			
				if (selIndex > wordStart && wordStart > (**theTE32KHandle).selStart) 
				{
					invertSelRange((**theTE32KHandle).selStart, wordStart, theTE32KHandle);
					(**theTE32KHandle).selStart = wordStart;
				}
			
				if (selIndex < (**theTE32KHandle).selStart || (selIndex > (**theTE32KHandle).selStart && selIndex < wordStart)) 
				{
					invertSelRange(selIndex, (**theTE32KHandle).selStart, theTE32KHandle);
					(**theTE32KHandle).selStart = selIndex;
				} 
				
				else if (selIndex > (**theTE32KHandle).selEnd || (selIndex < (**theTE32KHandle).selEnd && selIndex > wordEnd)) 
				{
					invertSelRange((**theTE32KHandle).selEnd, selIndex, theTE32KHandle);
					(**theTE32KHandle).selEnd = selIndex;
				}
			}
		}
		
		else
		{
			if (!extend)
			{
				if ((**theTE32KHandle).selStart != (**theTE32KHandle).selEnd)
					invertSelRange((**theTE32KHandle).selStart,(**theTE32KHandle).selEnd,theTE32KHandle);
				
				(**theTE32KHandle).selStart = selIndex;
				(**theTE32KHandle).selEnd = selIndex;
					
				(**theTE32KHandle).clikStuff = FALSE;
				TE32KGetPoint(selIndex,&selPt,theTE32KHandle);
				
				if ((**theTE32KHandle).crOnly)
					(**theTE32KHandle).selPoint = selPt;
				
				else
				{
					(**theTE32KHandle).clikStuff = TRUE;
					TE32KGetPoint(selIndex,&tempPt,theTE32KHandle);
					
					if ((selPt.h - startPoint.h)*(selPt.h - startPoint.h) + (selPt.v - startPoint.v)*(selPt.v - startPoint.v) <
						(tempPt.h - startPoint.h)*(tempPt.h - startPoint.h) + (tempPt.v - startPoint.v)*(tempPt.v - startPoint.v))
							(**theTE32KHandle).selPoint = selPt;
					else
							(**theTE32KHandle).selPoint = tempPt;
				}
				
				xorCaret(theTE32KHandle);
			}
				
				
			
			if (extend || StillDown())
			{
				if (extend)
				{
					if (selIndex >= (**theTE32KHandle).selEnd)
					{
						selAnchor = (**theTE32KHandle).selStart;
						selLast = (**theTE32KHandle).selEnd;
					}
					else
					{
						selAnchor = (**theTE32KHandle).selEnd;
						selLast = (**theTE32KHandle).selStart;
					}
				}
				else
				{
					selAnchor = selIndex;
					selLast = selIndex;
				}
				
				GetPort(&oldPort);
				SetPort((**theTE32KHandle).inPort);
				
				if (extend)
					goto DOHILITE;
				
				while (StillDown())
				{
					if (selIndex >= selAnchor)
					{
						(**theTE32KHandle).selStart = selAnchor;
						(**theTE32KHandle).selEnd = selIndex;
					}
					else
					{
						(**theTE32KHandle).selStart = selIndex;
						(**theTE32KHandle).selEnd = selAnchor;
					}
					
					if ((**theTE32KHandle).clikLoop)
						(*((**theTE32KHandle).clikLoop)) ();
					
					GetMouse(&mousePt);
					
					selPt.h = (long) mousePt.h;
					selPt.v = (long) mousePt.v;
					
					selIndex = TE32KGetOffset(&selPt,theTE32KHandle);
	
	DOHILITE:
					if (selLast >= selAnchor)
					{
						if (selIndex > selLast)
							invertSelRange(selLast,selIndex,theTE32KHandle);
						
						else if (selIndex>=selAnchor && selIndex<selLast)
							invertSelRange(selIndex,selLast,theTE32KHandle);
						
						else if (selIndex<selAnchor)
						{
							invertSelRange(selAnchor,selLast,theTE32KHandle);
							invertSelRange(selIndex,selAnchor,theTE32KHandle);
						}
					}
					
					else
					{
						if (selIndex < selLast)
							invertSelRange(selIndex,selLast,theTE32KHandle);
						
						else if (selIndex<=selAnchor && selIndex>selLast)
							invertSelRange(selLast,selIndex,theTE32KHandle);
						
						else if (selIndex>selAnchor)
						{
							invertSelRange(selLast,selAnchor,theTE32KHandle);
							invertSelRange(selAnchor,selIndex,theTE32KHandle);
						}
					}
					
					selLast = selIndex;
				}
				
				SetPort(oldPort);
				
				
				if (selIndex >= selAnchor)
				{
					(**theTE32KHandle).selStart = selAnchor;
					(**theTE32KHandle).selEnd = selIndex;
				}
				else
				{
					(**theTE32KHandle).selStart = selIndex;
					(**theTE32KHandle).selEnd = selAnchor;
				}
			}
		}
		
		(**theTE32KHandle).clickTime = TickCount();
		
		clickedTE32KH = 0L;
	}
}





void	TE32KSetSelect(long selStart,long selEnd,TE32KHandle theTE32KHandle)
{
	if (theTE32KHandle)
	{
		if ((**theTE32KHandle).active)
		{
			if ((**theTE32KHandle).selStart != (**theTE32KHandle).selEnd)
				invertSelRange((**theTE32KHandle).selStart,(**theTE32KHandle).selEnd,theTE32KHandle);
			
			else if ((**theTE32KHandle).caretState)
				xorCaret(theTE32KHandle);
		}
				
		if (selStart <= selEnd)
		{
			(**theTE32KHandle).selStart = selStart;
			(**theTE32KHandle).selEnd = selEnd;
		}
		else
		{
			(**theTE32KHandle).selStart = selEnd;
			(**theTE32KHandle).selEnd = selStart;
		}
		
		if ((**theTE32KHandle).active)
			invertSelRange((**theTE32KHandle).selStart,(**theTE32KHandle).selEnd,theTE32KHandle);
	}
}





OSErr TE32KToScrap(void)
{
OSErr err;

	if (TE32KScrpHandle && TE32KScrpLength > 0L)
	{
		if ((err = ZeroScrap()) != noErr)
			return(err);

		HLockHi(TE32KScrpHandle);

		if ((err = PutScrap(TE32KScrpLength,'TEXT',(Ptr) *TE32KScrpHandle)) != noErr)
			return(err);

		HUnlock(TE32KScrpHandle);

		return(noErr);
	}
	
	return(noScrapErr);
}




OSErr TE32KFromScrap(void)
{
long offset;
OSErr err;

	if (TE32KScrpHandle && (err = GetScrap(0L,'TEXT',&offset)) > 0L)
	{
		TE32KScrpLength = GetScrap(TE32KScrpHandle,'TEXT',&offset);
		
		if (TE32KScrpLength > 0L)
			return(noErr);
		
		else if (TE32KScrpLength == 0L)
			return(noTypeErr);
		
		else
			return(TE32KScrpLength);
	}
	
	if (TE32KScrpHandle == NULL)
		return(noScrapErr);
	
	else
		return(err);
}




void	TE32KCopy(TE32KHandle theTE32KHandle)
{
	if (theTE32KHandle && TE32KScrpHandle && (**theTE32KHandle).selStart < (**theTE32KHandle).selEnd)
	{
		SetHandleSize(TE32KScrpHandle,(**theTE32KHandle).selEnd - (**theTE32KHandle).selStart);
		
		if (!MemError() && GetHandleSize(TE32KScrpHandle) >= ((**theTE32KHandle).selEnd - (**theTE32KHandle).selStart))
		{
			TE32KScrpLength = (**theTE32KHandle).selEnd - (**theTE32KHandle).selStart;
			
			HLock(TE32KScrpHandle);
			HLock((**theTE32KHandle).hText);
			
			BlockMove(*((**theTE32KHandle).hText) + (**theTE32KHandle).selStart,*TE32KScrpHandle,TE32KScrpLength);
			
			HUnlock((**theTE32KHandle).hText);
			HUnlock(TE32KScrpHandle);
		}
	}
}





void	TE32KCut(TE32KHandle theTE32KHandle)
{
	if (theTE32KHandle && TE32KScrpHandle && (**theTE32KHandle).selStart < (**theTE32KHandle).selEnd)
	{
		TE32KCopy(theTE32KHandle);
		TE32KDelete(theTE32KHandle);
	}
}



void	TE32KDelete(TE32KHandle theTE32KHandle)
{
LongRect			updateRect;
register long		*theLine,*otherLine,theLineStart,i,delta;
long				firstLine,lastLine;
Rect				tempRect;
RgnHandle			updateRgn;
GrafPtr				oldPort;


	if (theTE32KHandle && (**theTE32KHandle).selStart < (**theTE32KHandle).selEnd)
	{
		invertSelRange((**theTE32KHandle).selStart,(**theTE32KHandle).selEnd,theTE32KHandle);
				
		firstLine = indexToLine((**theTE32KHandle).selStart,theTE32KHandle);
		lastLine = indexToLine((**theTE32KHandle).selEnd,theTE32KHandle);
		
		updateRect = (**theTE32KHandle).viewRect;
		updateRect.top = (**theTE32KHandle).destRect.top + (firstLine + 1L) * (**theTE32KHandle).lineHeight;
		LongRectToRect(&updateRect,&tempRect);
		
		GetPort(&oldPort);
		SetPort((**theTE32KHandle).inPort);
				
		updateRgn = NewRgn();
		ScrollRect(&tempRect,0,-(**theTE32KHandle).lineHeight * (lastLine - firstLine),updateRgn);
		tempRect = (**updateRgn).rgnBBox;
		DisposeRgn(updateRgn);
		
		SetPort(oldPort);
		
		if ((**theTE32KHandle).selEnd != (**theTE32KHandle).teLength)
		{
			HLock((**theTE32KHandle).hText);
			BlockMove(*((**theTE32KHandle).hText) + (**theTE32KHandle).selEnd,*((**theTE32KHandle).hText) + (**theTE32KHandle).selStart,(**theTE32KHandle).teLength - (**theTE32KHandle).selEnd);
			HUnlock((**theTE32KHandle).hText);
		}
		
		delta = (**theTE32KHandle).selEnd - (**theTE32KHandle).selStart;
		
		theLine = &((**theTE32KHandle).lineStarts[firstLine + 1L]);
		otherLine = &((**theTE32KHandle).lineStarts[lastLine + 1L]);
		i = (**theTE32KHandle).nLines - lastLine;
		
		while (i--)
		{
			theLineStart = *(otherLine++);
			theLineStart -= delta;
			*(theLine++) = theLineStart;
		}
		
		(**theTE32KHandle).teLength -= delta;
		(**theTE32KHandle).selEnd = (**theTE32KHandle).selStart;
		(**theTE32KHandle).nLines -= (lastLine - firstLine);
		
		RectToLongRect(&tempRect,&updateRect);
		TE32KUpdate(&updateRect,theTE32KHandle);	/* update scrolled stuff */
		
		updateLine(firstLine,theTE32KHandle,TRUE,0L);
	}
}





void	TE32KInsert(Ptr textPtr,register long textLength,TE32KHandle theTE32KHandle)
{
register long				*theLine,*otherLine,i,numCRs;
long						firstLine,teLength,maxLineStarts,sizeTE32KHandle;
register unsigned char		*charPtr,*charBase;
RgnHandle					updateRgn;
Rect						tempRect;
GrafPtr						oldPort;


	if (theTE32KHandle && textPtr && textLength > 0L)
	{
		if ((**theTE32KHandle).selStart < (**theTE32KHandle).selEnd)
			invertSelRange((**theTE32KHandle).selStart,(**theTE32KHandle).selEnd,theTE32KHandle);
		
		if (textLength == 1L)
		{
			TE32KKey(*((unsigned char *) textPtr),theTE32KHandle);
		}
		
		else
		{
			firstLine = indexToLine((**theTE32KHandle).selStart,theTE32KHandle);
			
			teLength = (**theTE32KHandle).teLength + textLength;
			
			if (GetHandleSize((**theTE32KHandle).hText) < teLength)
			{
				SetHandleSize((**theTE32KHandle).hText,teLength + EXTRATEXTBUFF);
				
				if (MemError() || GetHandleSize((**theTE32KHandle).hText) < teLength + EXTRATEXTBUFF)
					return;
			}
			
			HLock((**theTE32KHandle).hText);
			
			if ((**theTE32KHandle).teLength - (**theTE32KHandle).selStart)
				BlockMove(*((**theTE32KHandle).hText) + (**theTE32KHandle).selStart,*((**theTE32KHandle).hText) + (**theTE32KHandle).selStart + textLength,(**theTE32KHandle).teLength - (**theTE32KHandle).selStart);
			
			BlockMove(textPtr,*((**theTE32KHandle).hText) + (**theTE32KHandle).selStart,textLength);
			
			HUnlock((**theTE32KHandle).hText);
			
			i = textLength;
			numCRs = 0L;
			charPtr = (unsigned char *) textPtr;
			
			while (i--)
			{
				if (*charPtr == '\r' || *charPtr == '\n')
					numCRs++;
				
				charPtr++;
			}
			
			if (numCRs)
			{
				sizeTE32KHandle  = GetHandleSize((Handle) theTE32KHandle);
				maxLineStarts = (sizeTE32KHandle - (long) sizeof(TE32KRec))/(long) sizeof(long) - 2;
				
				if ((**theTE32KHandle).nLines + numCRs >= maxLineStarts)
				{
					sizeTE32KHandle = (long) sizeof(TE32KRec) + (long) sizeof(long)*((**theTE32KHandle).nLines + numCRs + EXTRALINESTARTS);
					maxLineStarts = (sizeTE32KHandle - (long) sizeof(TE32KRec))/(long) sizeof(long) - 2;
					
					SetHandleSize((Handle) theTE32KHandle,sizeTE32KHandle);
					
					if (MemError())
						return;
				}
				
				theLine = &((**theTE32KHandle).lineStarts[(**theTE32KHandle).nLines]);
				otherLine = &((**theTE32KHandle).lineStarts[(**theTE32KHandle).nLines + numCRs]);
				i = (**theTE32KHandle).nLines - firstLine;
				
				while (i--)
					*(otherLine--) = *(theLine--) + textLength;
				
				charPtr = (unsigned char *) (*((**theTE32KHandle).hText) + (**theTE32KHandle).selStart) + 1L;
				charBase = (unsigned char *) *((**theTE32KHandle).hText);
				theLine = &((**theTE32KHandle).lineStarts[firstLine + 1L]);
				i = numCRs;
				
				while (i--)
				{
					while (*charPtr != '\r' && *charPtr != '\n')
						charPtr++;
					
					charPtr++;
					
					*theLine++ = charPtr - charBase;
				}
				
				LongRectToRect(&((**theTE32KHandle).viewRect),&tempRect);
				tempRect.top = (**theTE32KHandle).destRect.top + (firstLine + 1L) * (**theTE32KHandle).lineHeight;
				
				GetPort(&oldPort);
				SetPort((**theTE32KHandle).inPort);
				
				updateRgn = NewRgn();
				ScrollRect(&tempRect,0,(**theTE32KHandle).lineHeight * numCRs,updateRgn);
				DisposeRgn(updateRgn);
				
				SetPort(oldPort);
			}
			
			else
			{
				theLine = &((**theTE32KHandle).lineStarts[firstLine + 1L]);
				i = (**theTE32KHandle).nLines - firstLine;
				
				while (i--)
					*(theLine++) += textLength;
			}
			
			
			(**theTE32KHandle).teLength = teLength;
			(**theTE32KHandle).selStart += textLength;
			(**theTE32KHandle).selEnd = (**theTE32KHandle).selStart;
			(**theTE32KHandle).nLines += numCRs;
			
			do
			{
				updateLine(firstLine,theTE32KHandle,TRUE,0L);
				
				if (numCRs)
				{
					theLine = (**theTE32KHandle).lineStarts;
					charPtr = (unsigned char *) *((**theTE32KHandle).hText);
					
					do
					{
						firstLine++;
						
					} while (firstLine < (**theTE32KHandle).nLines && charPtr[theLine[firstLine] - 1L] != '\r' && charPtr[theLine[firstLine] - 1L] != '\n');
					
				}
				
			} while (numCRs--);
		}
	}
}






void	TE32KPaste(TE32KHandle theTE32KHandle)
{
	if (theTE32KHandle && TE32KScrpHandle && TE32KScrpLength > 0L)
	{
		if ((**theTE32KHandle).selStart < (**theTE32KHandle).selEnd)
			TE32KDelete( theTE32KHandle);
		
		HLock(TE32KScrpHandle);
		
		TE32KInsert(*TE32KScrpHandle,TE32KScrpLength,theTE32KHandle);
		
		HUnlock(TE32KScrpHandle);
	}
}




Handle	TE32KScrapHandle()
{
	return(TE32KScrpHandle);
}


long	TE32KGetScrapLen()
{
	return(TE32KScrpLength);
}


void	TE32KSetScrapLen(long newLength)
{
	TE32KScrpLength = newLength;
}




void	TE32KSelView(TE32KHandle theTE32KHandle)
{
register long	deltaV,deltaH,screenLines,lineHeight,viewTop,viewBot,selPtV,ascent;

	if (theTE32KHandle && (**theTE32KHandle).active)
	{
		if ((**theTE32KHandle).selStart == (**theTE32KHandle).selEnd)
		{
			selPtV = (**theTE32KHandle).selPoint.v;
			viewTop = (**theTE32KHandle).viewRect.top;
			viewBot = (**theTE32KHandle).viewRect.bottom;
			lineHeight = (**theTE32KHandle).lineHeight;
			ascent = (**theTE32KHandle).fontAscent;
			
			deltaV = viewTop - (**theTE32KHandle).destRect.top;
			deltaV = deltaV - (deltaV/lineHeight)*lineHeight;
			
			if (selPtV - ascent < viewTop)
			{
				deltaV += viewTop - (selPtV - ascent);
			}
			
			else if (selPtV > viewBot)
			{
				screenLines = (viewBot - viewTop) / lineHeight;
				
				deltaV -= (selPtV - ascent + lineHeight) - (viewTop + screenLines * lineHeight);
			}
			
			
			if ((**theTE32KHandle).selPoint.h <= (**theTE32KHandle).viewRect.left)
			{
				deltaH = (**theTE32KHandle).viewRect.left - (**theTE32KHandle).selPoint.h;
				deltaH = (2L + deltaH/(**theTE32KHandle).lineHeight) * (**theTE32KHandle).lineHeight;
				
				if ((**theTE32KHandle).destRect.left + deltaH > (**theTE32KHandle).viewRect.left)
					deltaH = (**theTE32KHandle).viewRect.left - (**theTE32KHandle).destRect.left;
			}
			
			else if ((**theTE32KHandle).selPoint.h > (**theTE32KHandle).viewRect.right)
			{
				deltaH = (**theTE32KHandle).selPoint.h - (**theTE32KHandle).viewRect.right;
				deltaH = -(2L + deltaH/(**theTE32KHandle).lineHeight) * (**theTE32KHandle).lineHeight;
			}
			
			else
				deltaH = 0L;
			
			if (deltaV || deltaH)
				TE32KScroll(deltaH,deltaV,theTE32KHandle);
		}
	}
}





static	DoDeleteKey(TE32KHandle theTE32KHandle)
{
Rect			tempRect;
RgnHandle		updateRgn;
int				chWidth;
long			firstLine;
register int	*theCharWidths;
register long	i,*lineStarts,selIndex,*otherLine;
LongRect		updateRect;
LongPoint		selPt;
unsigned char	ch,prevChar;
GrafPtr			oldPort;

	if ((**theTE32KHandle).selStart < (**theTE32KHandle).selEnd)
		TE32KDelete(theTE32KHandle);
	
	else if ((**theTE32KHandle).selStart > 0L)
	{
		ch = ((unsigned char *) *((**theTE32KHandle).hText))[(**theTE32KHandle).selStart - 1L];
		
		if ((**theTE32KHandle).selStart >= 2L)
			prevChar = ((unsigned char *) *((**theTE32KHandle).hText))[(**theTE32KHandle).selStart - 2L];
		else
			prevChar = '\0';
		
		firstLine = indexToLine((**theTE32KHandle).selStart,theTE32KHandle);
		
		HLock((**theTE32KHandle).hText);
		BlockMove(*((**theTE32KHandle).hText) + (**theTE32KHandle).selStart,*((**theTE32KHandle).hText) + (**theTE32KHandle).selStart - 1L,(**theTE32KHandle).teLength - (**theTE32KHandle).selEnd);
		HUnlock((**theTE32KHandle).hText);
		
		(**theTE32KHandle).teLength--;
		(**theTE32KHandle).selStart--;
		(**theTE32KHandle).selEnd = (**theTE32KHandle).selStart;
		
		if ((ch == '\r' || ch == '\n') && ((**theTE32KHandle).crOnly || (**theTE32KHandle).teLength == (**theTE32KHandle).selStart || prevChar == '\r' || prevChar == '\n'))
		{
			lineStarts = &((**theTE32KHandle).lineStarts[firstLine]);
			otherLine = &((**theTE32KHandle).lineStarts[firstLine + 1L]);
			
			i = (**theTE32KHandle).nLines - firstLine;
			
			while (i--)
			{
				selIndex = *(otherLine++);
				*(lineStarts++) = --selIndex;
			}
			
			(**theTE32KHandle).nLines--;
			
			if (firstLine > 0L)
				firstLine--;
			
			updateRect = (**theTE32KHandle).viewRect;
			updateRect.top = (**theTE32KHandle).destRect.top + firstLine * (**theTE32KHandle).lineHeight;
			updateRect.bottom = updateRect.top + (**theTE32KHandle).lineHeight;
			TE32KUpdate(&updateRect,theTE32KHandle);
			
			LongRectToRect(&updateRect,&tempRect);
			tempRect.top = tempRect.bottom;
			
			if ((**theTE32KHandle).viewRect.bottom < -32768L)
				tempRect.bottom = -32768;
			else if ((**theTE32KHandle).viewRect.bottom > 32767L)
				tempRect.bottom = 32767;
			else
				tempRect.bottom = (int) (**theTE32KHandle).viewRect.bottom;
			
			
			GetPort(&oldPort);
			SetPort((**theTE32KHandle).inPort);
		
			updateRgn = NewRgn();
			ScrollRect(&tempRect,0,-(**theTE32KHandle).lineHeight,updateRgn);
			tempRect = (**updateRgn).rgnBBox;
			DisposeRgn(updateRgn);
			
			SetPort(oldPort);
			
			TE32KGetPoint((**theTE32KHandle).selStart,&selPt,theTE32KHandle);
			(**theTE32KHandle).selPoint = selPt;
			
			RectToLongRect(&tempRect,&updateRect);
			
			if ((**theTE32KHandle).caretState)
				xorCaret(theTE32KHandle);
		
			TE32KUpdate(&updateRect,theTE32KHandle);
		}
		
		else
		{
			lineStarts = &((**theTE32KHandle).lineStarts[(**theTE32KHandle).nLines]);
			i = (**theTE32KHandle).nLines - firstLine;
			
			if (ch == '\r' || ch == '\n')
				i++;
			
			
			while (i--)
				(*(lineStarts--))--;
			
			theCharWidths = (**theTE32KHandle).theCharWidths;
			
			if (ch == TAB)
				chWidth = (**theTE32KHandle).tabWidth;
			else
				chWidth = theCharWidths[ch];
			
			if (ch == '\r' || ch == '\n')
			{
				firstLine--;
				
				updateRect.top = (**theTE32KHandle).destRect.top + firstLine * (**theTE32KHandle).lineHeight;
				updateRect.bottom = updateRect.top + (**theTE32KHandle).lineHeight;
				updateRect.left = (**theTE32KHandle).viewRect.left;
				updateRect.right = (**theTE32KHandle).viewRect.right;

			}
			
			else
			{
				updateRect.top = (**theTE32KHandle).destRect.top + firstLine * (**theTE32KHandle).lineHeight;
				updateRect.bottom = updateRect.top + (**theTE32KHandle).lineHeight;
				updateRect.left = (**theTE32KHandle).selPoint.h - chWidth;
				updateRect.right = (**theTE32KHandle).viewRect.right;
			}
			
			if ((**theTE32KHandle).caretState)
				xorCaret(theTE32KHandle);
			
			if ((**theTE32KHandle).crOnly)
				TE32KUpdate(&updateRect,theTE32KHandle);
			
			else
			{
				updateLine(firstLine,theTE32KHandle,TRUE,&updateRect);
			}
		}
	}
}





static	DoArrowKeys(unsigned char ch,TE32KHandle theTE32KHandle)
{
LongPoint		selPt,tempPt1,tempPt2;
long			firstLine,selIndex;
unsigned char	currentChar;

	if ((**theTE32KHandle).caretState)
		xorCaret(theTE32KHandle);
	
	if (!shiftKeyDown())
	{
		if (ch==LEFTARROW || ch==RIGHTARROW)
		{
			if ((**theTE32KHandle).selStart < (**theTE32KHandle).selEnd)
				invertSelRange((**theTE32KHandle).selStart,(**theTE32KHandle).selEnd,theTE32KHandle);
			
			if (ch==LEFTARROW && (**theTE32KHandle).selStart > 0L)
			{
				currentChar = ((unsigned char *) *(**theTE32KHandle).hText)[(**theTE32KHandle).selStart - 1L];
				
				firstLine = indexToLine((**theTE32KHandle).selStart,theTE32KHandle);
				
				(**theTE32KHandle).clikStuff = FALSE;
				TE32KGetPoint((**theTE32KHandle).selStart,&selPt,theTE32KHandle);
				
				if (!(**theTE32KHandle).crOnly && firstLine > 0 && (**theTE32KHandle).selStart == (**theTE32KHandle).lineStarts[firstLine] && currentChar != '\r'  && currentChar != '\n' && (**theTE32KHandle).selPoint.h == selPt.h && (**theTE32KHandle).selPoint.v == selPt.v)
				{
					(**theTE32KHandle).clikStuff = TRUE;
					TE32KGetPoint((**theTE32KHandle).selStart,&selPt,theTE32KHandle);
					
					(**theTE32KHandle).selPoint = selPt;
					(**theTE32KHandle).selEnd = (**theTE32KHandle).selStart;
					
					return;
				}
				
				else
				{
					(**theTE32KHandle).selStart--;
					(**theTE32KHandle).selEnd = (**theTE32KHandle).selStart;
				}
			}
			else if (ch==RIGHTARROW && (**theTE32KHandle).selEnd < (**theTE32KHandle).teLength)
			{
				currentChar = ((unsigned char *) *(**theTE32KHandle).hText)[(**theTE32KHandle).selEnd];
				
				firstLine = indexToLine((**theTE32KHandle).selEnd,theTE32KHandle);
				
				(**theTE32KHandle).clikStuff = TRUE;
				TE32KGetPoint((**theTE32KHandle).selEnd,&selPt,theTE32KHandle);
				
				if (!(**theTE32KHandle).crOnly && (**theTE32KHandle).selEnd == (**theTE32KHandle).lineStarts[firstLine] && (**theTE32KHandle).selPoint.h == selPt.h && (**theTE32KHandle).selPoint.v == selPt.v)
				{
					(**theTE32KHandle).clikStuff = FALSE;
					TE32KGetPoint((**theTE32KHandle).selEnd,&selPt,theTE32KHandle);
					
					(**theTE32KHandle).selPoint = selPt;
					(**theTE32KHandle).selStart = (**theTE32KHandle).selEnd;
					
					return;
				}
				
				else if (!(**theTE32KHandle).crOnly && firstLine < (**theTE32KHandle).nLines - 1L && (**theTE32KHandle).selStart + 1L == (**theTE32KHandle).lineStarts[firstLine + 1L] && currentChar != '\r' && currentChar != '\n')
				{
					(**theTE32KHandle).selEnd++;
					
					(**theTE32KHandle).clikStuff = TRUE;
					TE32KGetPoint((**theTE32KHandle).selEnd,&selPt,theTE32KHandle);
					
					(**theTE32KHandle).selPoint = selPt;
					(**theTE32KHandle).selStart = (**theTE32KHandle).selEnd;
					
					return;
				}
				
				else
				{
					(**theTE32KHandle).selEnd++;
					(**theTE32KHandle).selStart = (**theTE32KHandle).selEnd;
				}
			}
			
			invertSelRange((**theTE32KHandle).selStart,(**theTE32KHandle).selEnd,theTE32KHandle);
		}
		
		else if (ch==UPARROW || ch==DOWNARROW)
		{
			if ((**theTE32KHandle).selStart < (**theTE32KHandle).selEnd)
			{
				invertSelRange((**theTE32KHandle).selStart,(**theTE32KHandle).selEnd,theTE32KHandle);
				
				if (ch == DOWNARROW)
					TE32KGetPoint((**theTE32KHandle).selEnd,&selPt,theTE32KHandle);
				else
					TE32KGetPoint((**theTE32KHandle).selStart,&selPt,theTE32KHandle);
				
				(**theTE32KHandle).selPoint = selPt;
			}
				
				
			if (ch==UPARROW)
			{
				selPt = (**theTE32KHandle).selPoint;
				
				firstLine = indexToLine((**theTE32KHandle).selStart,theTE32KHandle);
				
				if (firstLine > 0L)
				{
					selPt.v -= (**theTE32KHandle).lineHeight;
					(**theTE32KHandle).selStart = TE32KGetOffset(&selPt,theTE32KHandle);
					(**theTE32KHandle).selEnd = (**theTE32KHandle).selStart;
					firstLine = indexToLine((**theTE32KHandle).selStart,theTE32KHandle);
					
					if (!(**theTE32KHandle).crOnly && (**theTE32KHandle).selStart == (**theTE32KHandle).lineStarts[firstLine])
					{
						(**theTE32KHandle).clikStuff = FALSE;
						TE32KGetPoint((**theTE32KHandle).selStart,&tempPt1,theTE32KHandle);
						
						(**theTE32KHandle).clikStuff = TRUE;
						TE32KGetPoint((**theTE32KHandle).selStart,&tempPt2,theTE32KHandle);
						
						if ((selPt.h - tempPt1.h)*(selPt.h - tempPt1.h) + (selPt.v - tempPt1.v)*(selPt.v - tempPt1.v) <
							(selPt.h - tempPt2.h)*(selPt.h - tempPt2.h) + (selPt.v - tempPt2.v)*(selPt.v - tempPt2.v))
								(**theTE32KHandle).selPoint = tempPt1;
						else
								(**theTE32KHandle).selPoint = tempPt2;
						
						return;
					}
					
					else
					{
						(**theTE32KHandle).clikStuff = FALSE;
						TE32KGetPoint((**theTE32KHandle).selStart,&selPt,theTE32KHandle);
						(**theTE32KHandle).selPoint = selPt;
					}
				}
			}
			
			else if (ch == DOWNARROW)
			{
				selPt = (**theTE32KHandle).selPoint;
				
				firstLine = indexToLine((**theTE32KHandle).selStart,theTE32KHandle);
				
				if (firstLine < (**theTE32KHandle).nLines)
				{
					selPt.v += (**theTE32KHandle).lineHeight;
					(**theTE32KHandle).selEnd = TE32KGetOffset(&selPt,theTE32KHandle);
					(**theTE32KHandle).selStart = (**theTE32KHandle).selEnd;
					firstLine = indexToLine((**theTE32KHandle).selEnd,theTE32KHandle);
					
					if (!(**theTE32KHandle).crOnly && (**theTE32KHandle).selStart == (**theTE32KHandle).lineStarts[firstLine])
					{
						(**theTE32KHandle).clikStuff = FALSE;
						TE32KGetPoint((**theTE32KHandle).selStart,&tempPt1,theTE32KHandle);
						
						(**theTE32KHandle).clikStuff = TRUE;
						TE32KGetPoint((**theTE32KHandle).selStart,&tempPt2,theTE32KHandle);
						
						if ((selPt.h - tempPt1.h)*(selPt.h - tempPt1.h) + (selPt.v - tempPt1.v)*(selPt.v - tempPt1.v) <
							(selPt.h - tempPt2.h)*(selPt.h - tempPt2.h) + (selPt.v - tempPt2.v)*(selPt.v - tempPt2.v))
								(**theTE32KHandle).selPoint = tempPt1;
						else
								(**theTE32KHandle).selPoint = tempPt2;
						
						return;
					}
					
					else
					{
						(**theTE32KHandle).clikStuff = FALSE;
						TE32KGetPoint((**theTE32KHandle).selEnd,&selPt,theTE32KHandle);
						(**theTE32KHandle).selPoint = selPt;
					}
				}
			}
			
			(**theTE32KHandle).selEnd = (**theTE32KHandle).selStart;
			
			invertSelRange((**theTE32KHandle).selStart,(**theTE32KHandle).selEnd,theTE32KHandle);
		}
	}
	
	else
	{
		if (ch==LEFTARROW)
		{
			if ((**theTE32KHandle).selStart > 0L)
			{
				invertSelRange((**theTE32KHandle).selStart - 1L,(**theTE32KHandle).selStart,theTE32KHandle);
				(**theTE32KHandle).selStart--;
			}
		}
		
		else if (ch==RIGHTARROW)
		{
			if ((**theTE32KHandle).selEnd < (**theTE32KHandle).teLength)
			{
				invertSelRange((**theTE32KHandle).selEnd,(**theTE32KHandle).selEnd + 1L,theTE32KHandle);
				(**theTE32KHandle).selEnd++;
			}
		}
		
		else if (ch==UPARROW)
		{
			firstLine = indexToLine((**theTE32KHandle).selStart,theTE32KHandle);
			
			if (firstLine > 0L)
			{
				TE32KGetPoint((**theTE32KHandle).selStart,&selPt,theTE32KHandle);
				selPt.v -= (**theTE32KHandle).lineHeight;
				selIndex = TE32KGetOffset(&selPt,theTE32KHandle);
				
				invertSelRange(selIndex,(**theTE32KHandle).selStart,theTE32KHandle);
				
				(**theTE32KHandle).selStart = selIndex;
			}
		}
		
		else if (ch==DOWNARROW)
		{
			firstLine = indexToLine((**theTE32KHandle).selEnd,theTE32KHandle);
			
			if (firstLine < (**theTE32KHandle).nLines - 1L)
			{
				TE32KGetPoint((**theTE32KHandle).selEnd,&selPt,theTE32KHandle);
				selPt.v += (**theTE32KHandle).lineHeight;
				selIndex = TE32KGetOffset(&selPt,theTE32KHandle);
				
				invertSelRange((**theTE32KHandle).selEnd,selIndex,theTE32KHandle);
				
				(**theTE32KHandle).selEnd = selIndex;
			}
		}
	}
}





static	DoNormalChar(unsigned char ch,TE32KHandle theTE32KHandle)
{
Rect			tempRect;
RgnHandle		updateRgn;
int				chWidth,destLeftSide;
register int	*theCharWidths;
long			teLength,firstLine;
register long	i,*lineStarts,delta;
LongPoint		selPt;
unsigned char	prevChar;
GrafPtr			oldPort;
int				oldFont,oldFace,oldSize,oldMode;
	
	teLength = (**theTE32KHandle).teLength + 1L;
	
	if (GetHandleSize((**theTE32KHandle).hText) < teLength)
	{
		SetHandleSize((**theTE32KHandle).hText,teLength + EXTRATEXTBUFF);
		
		if (MemError() || GetHandleSize((**theTE32KHandle).hText) < teLength)
			return;
	}
	
	if ((**theTE32KHandle).caretState)
		xorCaret(theTE32KHandle);
	
	selPt = (**theTE32KHandle).selPoint;
	
	selPt.h--;
	
	firstLine = indexToLine((**theTE32KHandle).selStart,theTE32KHandle);
	if ((**theTE32KHandle).selStart > 0L)
		prevChar = ((unsigned char *) *(**theTE32KHandle).hText)[(**theTE32KHandle).lineStarts[firstLine] - 1L];
	else
		prevChar = '\r';
	
	if ((**theTE32KHandle).crOnly || prevChar =='\r'  || prevChar =='\n' || !(ch == ' ' && (**theTE32KHandle).selStart == (**theTE32KHandle).lineStarts[firstLine]))
	{
		if (selPt.h < -32768L)
			tempRect.left = -32768;
		else if (selPt.h > 32767L)
			tempRect.left = 32767;
		else
			tempRect.left = (int) selPt.h;
		
		if ((**theTE32KHandle).viewRect.right < -32768L)
			tempRect.right = -32768;
		else if ((**theTE32KHandle).viewRect.right > 32767L)
			tempRect.right = 32767;
		else
			tempRect.right = (int) (**theTE32KHandle).viewRect.right;
		
		selPt.v -= (**theTE32KHandle).fontAscent;
		
		if (selPt.v < -32768L)
			tempRect.top = -32768;
		else if (selPt.v > 32767L)
			tempRect.top = 32767;
		else
			tempRect.top = (int) selPt.v;
		
		tempRect.bottom = tempRect.top + (**theTE32KHandle).lineHeight;
		
		GetPort(&oldPort);
		SetPort((**theTE32KHandle).inPort);
		
		oldFont = ((**theTE32KHandle).inPort)->txFont;
		oldFace = ((**theTE32KHandle).inPort)->txFace;
		oldSize = ((**theTE32KHandle).inPort)->txSize;
		oldMode = ((**theTE32KHandle).inPort)->txMode;
		
		TextFont((**theTE32KHandle).txFont);
		TextFace((**theTE32KHandle).txFace);
		TextSize((**theTE32KHandle).txSize);
		TextMode((**theTE32KHandle).txMode);
		
		theCharWidths = (**theTE32KHandle).theCharWidths;
		
		if (ch == TAB)
		{
			destLeftSide = (**theTE32KHandle).destRect.left + 1L;
			delta = (**theTE32KHandle).tabWidth;
			chWidth = (destLeftSide + ((tempRect.left + 1 - destLeftSide + delta)/delta)*delta) - (tempRect.left + 1);
		}
		else
			chWidth = theCharWidths[ch];
		
		if (tempRect.left < tempRect.right)
		{	
			updateRgn = NewRgn();
			ScrollRect(&tempRect,chWidth,0,updateRgn);
			
			if (tempRect.left+1 + chWidth > tempRect.right)
				ClipRect(&tempRect);
			
			MoveTo(tempRect.left+1,tempRect.top + (**theTE32KHandle).fontAscent);
			if (ch != TAB)
				DrawChar(ch);
			
			if (tempRect.left+1 + chWidth > tempRect.right)
			{
				tempRect.left = -32768;
				tempRect.top = -32768;
				tempRect.right = 32767;
				tempRect.bottom = 32767;
				ClipRect(&tempRect);
			}
			
			DisposeRgn(updateRgn);
		}
		
		TextFont(oldFont);
		TextFace(oldFace);
		TextSize(oldSize);
		TextMode(oldMode);
		
		SetPort(oldPort);
	}
	
	HLock((**theTE32KHandle).hText);
	BlockMove(*((**theTE32KHandle).hText) + (**theTE32KHandle).selStart,*((**theTE32KHandle).hText) + (**theTE32KHandle).selStart + 1L,(**theTE32KHandle).teLength - (**theTE32KHandle).selStart);
	HUnlock((**theTE32KHandle).hText);
	
	((unsigned char *) *((**theTE32KHandle).hText))[(**theTE32KHandle).selStart] = ch;
	
	lineStarts = &((**theTE32KHandle).lineStarts[(**theTE32KHandle).nLines]);
	i = (**theTE32KHandle).nLines - firstLine;
	
	if (!(**theTE32KHandle).crOnly && prevChar != '\r'  && prevChar != '\n' && ch == ' ' && (**theTE32KHandle).selStart == (**theTE32KHandle).lineStarts[firstLine])
		i++;
	
	while (i--)
		(*(lineStarts--))++;
	
	
	(**theTE32KHandle).teLength++;
	(**theTE32KHandle).selStart++;
	(**theTE32KHandle).selEnd = (**theTE32KHandle).selStart;
	(**theTE32KHandle).selPoint.h += (long) chWidth;
	
	if (!(**theTE32KHandle).crOnly)
		updateLine(firstLine,theTE32KHandle,FALSE,0L);
	
	xorCaret(theTE32KHandle);
}





static	DoReturnChar(TE32KHandle theTE32KHandle)
{
Rect			tempRect;
RgnHandle		updateRgn;
long			teLength,firstLine,lastLine,deltaLines,numAffected,tempFirstLine;
register long	i,*lineStarts,selIndex,*otherLine;
LongPoint		selPt;
LongRect		updateRect;
unsigned char	prevChar,doWrap;
GrafPtr			oldPort;

	teLength = GetHandleSize((Handle) theTE32KHandle);
	lastLine  = (teLength - (long) sizeof(TE32KRec))/(long) sizeof(long) - 2;
	
	if ((**theTE32KHandle).nLines + 1L >= lastLine)
	{
		teLength = (long) sizeof(TE32KRec) + (long) sizeof(long)*((**theTE32KHandle).nLines + 1L + EXTRALINESTARTS);
		
		SetHandleSize((Handle) theTE32KHandle,teLength);
		
		if (MemError()  || GetHandleSize(theTE32KHandle) < teLength)
			return;
	}
	
	
	teLength = (**theTE32KHandle).teLength + 1L;
	
	if (GetHandleSize((**theTE32KHandle).hText) < teLength)
	{
		SetHandleSize((**theTE32KHandle).hText,teLength + EXTRATEXTBUFF);
		
		if (MemError() || GetHandleSize((**theTE32KHandle).hText) < teLength)
			return;
	}
	
	
	
	if ((**theTE32KHandle).selStart > 0L)
		prevChar = ((unsigned char *) *((**theTE32KHandle).hText))[(**theTE32KHandle).selStart - 1L];
	else
		prevChar = '\r';
	
	if ((**theTE32KHandle).caretState)
		xorCaret(theTE32KHandle);
	
	HLock((**theTE32KHandle).hText);
	BlockMove(*((**theTE32KHandle).hText) + (**theTE32KHandle).selStart,*((**theTE32KHandle).hText) + (**theTE32KHandle).selStart + 1L,(**theTE32KHandle).teLength - (**theTE32KHandle).selStart);
	HUnlock((**theTE32KHandle).hText);
	
	((unsigned char *) *((**theTE32KHandle).hText))[(**theTE32KHandle).selStart] = RETURN;
	
	firstLine = indexToLine((**theTE32KHandle).selStart,theTE32KHandle);
	
	lineStarts = &((**theTE32KHandle).lineStarts[(**theTE32KHandle).nLines]);
	otherLine = &((**theTE32KHandle).lineStarts[(**theTE32KHandle).nLines + 1L]);
	i = (**theTE32KHandle).nLines - firstLine;
		
	while (i--)
	{
		selIndex = *(lineStarts--);
		*(otherLine--) = ++selIndex;
	
	}
	
	(**theTE32KHandle).lineStarts[firstLine + 1L] = (**theTE32KHandle).selStart + 1L;
	
	(**theTE32KHandle).nLines++;
	(**theTE32KHandle).teLength++;
	(**theTE32KHandle).selStart++;
	(**theTE32KHandle).selEnd = (**theTE32KHandle).selStart;
	
	LongRectToRect(&((**theTE32KHandle).viewRect),&tempRect);
	
	selPt = (**theTE32KHandle).selPoint;
	selPt.v -= (**theTE32KHandle).fontAscent;
	selPt.v += (**theTE32KHandle).lineHeight;
	
	if (selPt.v < -32768L)
		tempRect.top = -32768;
	else if (selPt.v > 32767L)
		tempRect.top = 32767;
	else
		tempRect.top = (int) selPt.v;
	
	GetPort(&oldPort);
	SetPort((**theTE32KHandle).inPort);
			
	updateRgn = NewRgn();
	ScrollRect(&tempRect,0,(**theTE32KHandle).lineHeight,updateRgn);
	DisposeRgn(updateRgn);
	
	SetPort(oldPort);
	
	if (!(**theTE32KHandle).crOnly)
	{
		doWrap = FALSE;
		tempFirstLine = firstLine;
		
		if (tempFirstLine > 0L && LineEndIndex(tempFirstLine - 1L,theTE32KHandle) != (**theTE32KHandle).lineStarts[tempFirstLine])
		{
			doWrap = TRUE;
			tempFirstLine--;
		}
		
		else if (LineEndIndex(tempFirstLine,theTE32KHandle) != (**theTE32KHandle).lineStarts[tempFirstLine + 1L])
			doWrap = TRUE;
		
		
		if (doWrap)
		{
			CalParagraph(tempFirstLine,theTE32KHandle,&deltaLines,&numAffected);
			
			if (deltaLines == 0L)
			{
				updateRect = (**theTE32KHandle).viewRect;
				updateRect.top = (**theTE32KHandle).destRect.top + tempFirstLine * (**theTE32KHandle).lineHeight;
				updateRect.bottom = updateRect.top + (**theTE32KHandle).lineHeight * numAffected;
			}
			
			else if (deltaLines > 0L)
			{
				firstLine += deltaLines;
				
				LongRectToRect(&((**theTE32KHandle).viewRect),&tempRect);
				tempRect.top = (**theTE32KHandle).destRect.top + (tempFirstLine + numAffected - deltaLines) * (**theTE32KHandle).lineHeight;
				
				GetPort(&oldPort);
				SetPort((**theTE32KHandle).inPort);
				
				updateRgn = NewRgn();
				ScrollRect(&tempRect,0,(**theTE32KHandle).lineHeight * deltaLines,updateRgn);
				DisposeRgn(updateRgn);
				
				SetPort(oldPort);
				
				updateRect = (**theTE32KHandle).viewRect;
				updateRect.top = (**theTE32KHandle).destRect.top + tempFirstLine * (**theTE32KHandle).lineHeight;
				updateRect.bottom = updateRect.top + (**theTE32KHandle).lineHeight * numAffected;
			}
			
			else
			{
				firstLine += deltaLines;
				
				LongRectToRect(&((**theTE32KHandle).viewRect),&tempRect);
				tempRect.top = (**theTE32KHandle).destRect.top + (tempFirstLine - 1L + numAffected) * (**theTE32KHandle).lineHeight;
				
				GetPort(&oldPort);
				SetPort((**theTE32KHandle).inPort);
	
				updateRgn = NewRgn();
				ScrollRect(&tempRect,0,(**theTE32KHandle).lineHeight * deltaLines,updateRgn);
				DisposeRgn(updateRgn);
				
				SetPort(oldPort);
				
				updateRect = (**theTE32KHandle).viewRect;
				updateRect.top = (**theTE32KHandle).destRect.top + tempFirstLine * (**theTE32KHandle).lineHeight;
				updateRect.bottom = updateRect.top + (**theTE32KHandle).lineHeight * numAffected;
			}
			
			TE32KUpdate(&updateRect,theTE32KHandle);
		}
		
		
		firstLine++;
		
		CalParagraph(firstLine,theTE32KHandle,&deltaLines,&numAffected);
		
		if (deltaLines > 0L)
		{
			LongRectToRect(&((**theTE32KHandle).viewRect),&tempRect);
			tempRect.top = (**theTE32KHandle).destRect.top + (firstLine + numAffected - deltaLines) * (**theTE32KHandle).lineHeight;
			
			GetPort(&oldPort);
			SetPort((**theTE32KHandle).inPort);
	
			updateRgn = NewRgn();
			ScrollRect(&tempRect,0,(**theTE32KHandle).lineHeight * deltaLines,updateRgn);
			DisposeRgn(updateRgn);
			
			SetPort(oldPort);
		}
		
		else if (deltaLines < 0L)
		{
			LongRectToRect(&((**theTE32KHandle).viewRect),&tempRect);
			tempRect.top = (**theTE32KHandle).destRect.top + (firstLine - 1L + numAffected) * (**theTE32KHandle).lineHeight;
			
			GetPort(&oldPort);
			SetPort((**theTE32KHandle).inPort);
	
			updateRgn = NewRgn();
			ScrollRect(&tempRect,0,(**theTE32KHandle).lineHeight * deltaLines,updateRgn);
			DisposeRgn(updateRgn);
			
			SetPort(oldPort);
		}
		
		updateRect = (**theTE32KHandle).viewRect;
		updateRect.top = (**theTE32KHandle).destRect.top + (firstLine - 1L) * (**theTE32KHandle).lineHeight;
		updateRect.bottom = updateRect.top + (**theTE32KHandle).lineHeight * (numAffected + 1L);
		
		if ((**theTE32KHandle).caretState)
			xorCaret(theTE32KHandle);
		
		TE32KUpdate(&updateRect,theTE32KHandle);
		
		TE32KGetPoint((**theTE32KHandle).selStart,&selPt,theTE32KHandle);
		(**theTE32KHandle).selPoint = selPt;
	}
	
	else
	{
		updateRect = (**theTE32KHandle).viewRect;
		TE32KGetPoint((**theTE32KHandle).selStart,&selPt,theTE32KHandle);
		(**theTE32KHandle).selPoint = selPt;
		
		if ((**theTE32KHandle).nLines - firstLine >= 2L && (**theTE32KHandle).lineStarts[firstLine+1L]+1L < (**theTE32KHandle).lineStarts[firstLine + 2L])
		{
			updateRect.top = (**theTE32KHandle).destRect.top + firstLine * (**theTE32KHandle).lineHeight;
			updateRect.bottom = updateRect.top + (**theTE32KHandle).lineHeight + (**theTE32KHandle).lineHeight;
			TE32KUpdate(&updateRect,theTE32KHandle);
		}
		else
		{
			TE32KGetPoint((**theTE32KHandle).selStart,&selPt,theTE32KHandle);
			(**theTE32KHandle).selPoint = selPt;
			xorCaret(theTE32KHandle);
		}
	}
}



static	OverTypeSelection(unsigned char ch,TE32KHandle theTE32KHandle)
{
	TE32KDelete(theTE32KHandle);
	
	if (ch==RETURN)
		DoReturnChar(theTE32KHandle);
	
	else if (ch==TAB || ch >= (unsigned char) 0x20)
		DoNormalChar(ch,theTE32KHandle);
}









void	TE32KKey(unsigned char ch,TE32KHandle theTE32KHandle)
{
	if (theTE32KHandle && (**theTE32KHandle).active)
	{
		ObscureCursor();
		
		if (ch == ENTER)
			ch = RETURN;
			
		
		if (ch == DELETE)
			DoDeleteKey(theTE32KHandle);
		
		else if (ch==LEFTARROW || ch==RIGHTARROW || ch==UPARROW || ch==DOWNARROW)
			DoArrowKeys(ch,theTE32KHandle);
		
		else if ((**theTE32KHandle).selStart < (**theTE32KHandle).selEnd && (ch >= 0x20 || ch==TAB || ch==RETURN))
			OverTypeSelection(ch,theTE32KHandle);
		
		else if (ch==TAB || ch >= (unsigned char) 0x20)
			DoNormalChar(ch,theTE32KHandle);
		
		else if (ch==RETURN)
			DoReturnChar(theTE32KHandle);
		
		if ((**theTE32KHandle).selStart == (**theTE32KHandle).selEnd && !(**theTE32KHandle).caretState)
			xorCaret(theTE32KHandle);
	}
}





static long paraLines(long firstLine, TE32KHandle theTE32KHandle)
{
long					lastLine,nLines;
register unsigned char	*charBase;
register long			*lineStarts;

	if ((**theTE32KHandle).crOnly)
		return(1L);
	
	lastLine = firstLine + 1L;
	nLines = (**theTE32KHandle).nLines;
	charBase = (unsigned char	*) *((**theTE32KHandle).hText);
	lineStarts = &((**theTE32KHandle).lineStarts[lastLine]);
	
	while (lastLine < nLines && charBase[*lineStarts - 1L] != '\r' && charBase[*lineStarts - 1L] != '\n')
	{
		lastLine++;
		lineStarts++;
	}
	
	return(lastLine - firstLine);
}





static long	LineEndIndex(long firstLine,TE32KHandle theTE32KHandle)
{
register unsigned char	*charPtr;
register long			charCount;
register int			*theCharWidths,crOnly,maxLineWidth,lineWidth;
register unsigned char	ch;
unsigned char			*charBase;
Point					cursorPt;
int						rightSide,destLeftSide,tabWidth;
int						lineStatus;
unsigned char			*oldCharPtr;
long					maxRewind;
	
	if ((**theTE32KHandle).crOnly)
		return((**theTE32KHandle).lineStarts[firstLine + 1L]);
	
	maxLineWidth = (**theTE32KHandle).maxLineWidth;
	crOnly = (**theTE32KHandle).crOnly;
	
	charBase = (unsigned char *) *((**theTE32KHandle).hText);
	charPtr = charBase + (**theTE32KHandle).lineStarts[firstLine];
	charCount = (**theTE32KHandle).teLength - (**theTE32KHandle).lineStarts[firstLine];
	
	if (charCount > (**theTE32KHandle).teLength)
		charCount = (**theTE32KHandle).teLength;
	
	lineStatus = 0;
	lineWidth = 0;
	
	if (charCount)
	{
		rightSide = (int) ((**theTE32KHandle).destRect.right);
		destLeftSide = (int) ((**theTE32KHandle).destRect.left + 1L);
		cursorPt.h = destLeftSide;
		tabWidth = (long) (**theTE32KHandle).tabWidth;
		
		theCharWidths = (**theTE32KHandle).theCharWidths;
		
		ch = ' ';
		
		while (charCount-- && ch != '\r' && ch != '\n')
		{
			ch = *charPtr++;
			lineWidth++;
			
			if (ch == TAB)
				cursorPt.h = destLeftSide + ((cursorPt.h - destLeftSide + tabWidth)/tabWidth)*tabWidth;
			else if (ch != '\r' && ch != '\n')
				cursorPt.h += theCharWidths[ch];
			
			if ((cursorPt.h >= rightSide && ch != ' ') || (!crOnly && lineWidth > maxLineWidth))
			{
				maxRewind = charPtr - charBase - (**theTE32KHandle).lineStarts[firstLine];
				oldCharPtr = charPtr;
				
				charPtr--;
				maxRewind--;
				
				while (*charPtr != ' ' && maxRewind > 0)
				{
					charPtr--;
					maxRewind--;
				}
				
				if (maxRewind <= 0)
					charPtr = oldCharPtr;
				
				else
					charPtr++;
				
				charCount = 0;
			}
		}
	}
	
	return(charPtr - charBase);
}



#define	NUMTEMPLINES	32

static void	CalParagraph(long firstLine,TE32KHandle theTE32KHandle,long *theDeltaLines,long *theNumAffected)

{
register unsigned char	*charPtr;
register int			*theCharWidths;
register long			charCount,*lineStarts,*otherLine,i;
register long			crOnly,lineWidth,maxLineWidth;
register unsigned char	ch;
register long			nLines;
long					maxLineStarts,sizeTE32KHandle,oldCharCount;
unsigned char			*charBase;
Point					cursorPt;
int						rightSide,destLeftSide,tabWidth,maxRewind;
unsigned char			*oldCharPtr;
long					tempLineStarts[NUMTEMPLINES],oldNumLines,deltaLines;
	
	if ((**theTE32KHandle).crOnly)
	{
		*theDeltaLines = 0L;
		*theNumAffected = 0L;
		return;
	}
	
	deltaLines = 0L;
	
	oldNumLines = paraLines(firstLine,theTE32KHandle);
	
	for (i=0;i<oldNumLines && i <NUMTEMPLINES;i++)
		tempLineStarts[i] = (**theTE32KHandle).lineStarts[firstLine + i];
		
	sizeTE32KHandle  = GetHandleSize((Handle) theTE32KHandle);
	maxLineStarts = (sizeTE32KHandle - (long) sizeof(TE32KRec))/(long) sizeof(long) - 2;
	
	nLines = 0;
	tempLineStarts[nLines] = (**theTE32KHandle).lineStarts[firstLine];
	
	crOnly = (**theTE32KHandle).crOnly;
	maxLineWidth = (**theTE32KHandle).maxLineWidth;
	lineWidth = 0;
	
	charBase = (unsigned char *) *((**theTE32KHandle).hText);
	charPtr = charBase + (**theTE32KHandle).lineStarts[firstLine];
	
	charCount = (**theTE32KHandle).teLength - (**theTE32KHandle).lineStarts[firstLine];
	ch = *charPtr;
	
	if (charCount > 0L)
	{
		rightSide = (int) ((**theTE32KHandle).destRect.right);
		destLeftSide = (int) ((**theTE32KHandle).destRect.left + 1L);
		cursorPt.h = destLeftSide;
		tabWidth = (long) (**theTE32KHandle).tabWidth;
		
		theCharWidths = (**theTE32KHandle).theCharWidths;
		
		ch = ' ';
		
		while (ch != '\r' && ch != '\n' && charCount--)
		{
			ch = *charPtr++;
			lineWidth++;
			
			if (ch != '\r' && ch != '\n')
			{
				if (ch == TAB)
					cursorPt.h = destLeftSide + ((cursorPt.h - destLeftSide + tabWidth)/tabWidth)*tabWidth;
				else
					cursorPt.h += theCharWidths[ch];
				
				if ((cursorPt.h >= rightSide && ch != ' ') || (!crOnly && lineWidth > maxLineWidth))
				{
					maxRewind = charPtr - charBase - tempLineStarts[nLines];
					oldCharPtr = charPtr;
					oldCharCount = charCount;
					
					charPtr--;
					charCount++;
					maxRewind--;
					
					while (*charPtr != ' ' && maxRewind > 0)
					{
						charPtr--;
						charCount++;
						maxRewind--;
					}
					
					if (maxRewind <= 0)
					{
						charPtr = oldCharPtr;
						charCount = oldCharCount;
					}
					
					else
					{
						charPtr++;
						charCount--;
					}
					
					nLines++;
					
					if (nLines < NUMTEMPLINES)
					{
						if (tempLineStarts[nLines] == charPtr - charBase)
						{
							oldNumLines = nLines;
							goto STOPWRAPPING;
						}
						else
							tempLineStarts[nLines] = charPtr - charBase;
					}
					
					else
						goto STOPWRAPPING;
					
					cursorPt.h = destLeftSide;
					lineWidth = 0;
				}
			}
		}
		
		nLines++;
		
		if (nLines < NUMTEMPLINES)
			tempLineStarts[nLines] = charPtr - charBase;

STOPWRAPPING:

		deltaLines = nLines - oldNumLines;

		if (nLines >= NUMTEMPLINES)
		{
			TE32KCalText(theTE32KHandle);
			deltaLines = (**theTE32KHandle).nLines - firstLine - oldNumLines;
		}
		
		else
		{
			if (deltaLines == 0L)
			{
				for (i = 1;i <= nLines;i++)
					(**theTE32KHandle).lineStarts[firstLine + i] = tempLineStarts[i];
			}
			
			else if (deltaLines < 0L)
			{
				lineStarts = &((**theTE32KHandle).lineStarts[firstLine + 1L]);
				
				for (i = 1;i <= nLines;i++)
					*(lineStarts++) = tempLineStarts[i];
				
				otherLine = &((**theTE32KHandle).lineStarts[firstLine + oldNumLines + 1L]);
				i = (**theTE32KHandle).nLines - firstLine - oldNumLines + 1L;
				
				while (i--)
					*(lineStarts++) = *(otherLine++);
				
				(**theTE32KHandle).nLines += deltaLines;
			}
			
			else
			{
				if ((**theTE32KHandle).nLines + deltaLines >= maxLineStarts)
				{
					sizeTE32KHandle = (long) sizeof(TE32KRec) + (long) sizeof(long)*((**theTE32KHandle).nLines + deltaLines + EXTRALINESTARTS);
					maxLineStarts = (sizeTE32KHandle - (long) sizeof(TE32KRec))/(long) sizeof(long) - 2;
					
					SetHandleSize((Handle) theTE32KHandle,sizeTE32KHandle);
					
					if (MemError())
					{
						nLines = (**theTE32KHandle).nLines;
						deltaLines = (**theTE32KHandle).nLines;
						goto EXITPOINT;
					}
				}
				
				lineStarts = &((**theTE32KHandle).lineStarts[(**theTE32KHandle).nLines]);
				otherLine = &((**theTE32KHandle).lineStarts[(**theTE32KHandle).nLines + deltaLines]);
				i = (**theTE32KHandle).nLines - firstLine - oldNumLines;
				
				while (i--)
					*(otherLine--) = *(lineStarts--);
					
				for (i = nLines;i >= 0;i--)
					*(otherLine--) = tempLineStarts[i];
				
				(**theTE32KHandle).nLines += deltaLines;
			}
		}
	}
	
EXITPOINT:
	*theNumAffected = nLines;
	*theDeltaLines = deltaLines;
}





static void updateLine(register long firstLine, TE32KHandle theTE32KHandle,int doFirst, LongRect *updateClipRect)
{
Rect			tempRect;
RgnHandle		updateRgn;
LongRect		updateRect;
LongPoint		selPt;
unsigned char	doWrap;
long			deltaLines,numAffected;
GrafPtr			oldPort;


	updateRect = (**theTE32KHandle).viewRect;
	updateRect.top = (**theTE32KHandle).destRect.top + firstLine * (**theTE32KHandle).lineHeight;
	updateRect.bottom = updateRect.top + (**theTE32KHandle).lineHeight;
	
	if (updateClipRect)
	{
		if (updateRect.top < updateClipRect->top)
			updateRect.top = updateClipRect->top;
		if (updateRect.bottom > updateClipRect->bottom)
			updateRect.bottom = updateClipRect->bottom;
		if (updateRect.left < updateClipRect->left)
			updateRect.left = updateClipRect->left;
		if (updateRect.right > updateClipRect->right)
			updateRect.right = updateClipRect->right;
	}
	
	doWrap = FALSE;
	
	if (firstLine > 0L && LineEndIndex(firstLine - 1L,theTE32KHandle) != (**theTE32KHandle).lineStarts[firstLine])
	{
		doWrap = TRUE;
		firstLine--;
	}
	
	else if (LineEndIndex(firstLine,theTE32KHandle) != (**theTE32KHandle).lineStarts[firstLine + 1L])
		doWrap = TRUE;
	
	
	if (!doWrap && doFirst)
		TE32KUpdate(&updateRect,theTE32KHandle);
	
	else if (doWrap)
	{
		CalParagraph(firstLine,theTE32KHandle,&deltaLines,&numAffected);
		
		if (deltaLines == 0L)
		{
			updateRect = (**theTE32KHandle).viewRect;
			updateRect.top = (**theTE32KHandle).destRect.top + firstLine * (**theTE32KHandle).lineHeight;
			updateRect.bottom = updateRect.top + (**theTE32KHandle).lineHeight * numAffected;
		}
		
		else if (deltaLines > 0L)
		{
			LongRectToRect(&((**theTE32KHandle).viewRect),&tempRect);
			tempRect.top = (**theTE32KHandle).destRect.top + (firstLine + numAffected - deltaLines) * (**theTE32KHandle).lineHeight;
			
			GetPort(&oldPort);
			SetPort((**theTE32KHandle).inPort);
	
			updateRgn = NewRgn();
			ScrollRect(&tempRect,0,(**theTE32KHandle).lineHeight * deltaLines,updateRgn);
			DisposeRgn(updateRgn);
			
			SetPort(oldPort);
			
			updateRect = (**theTE32KHandle).viewRect;
			updateRect.top = (**theTE32KHandle).destRect.top + firstLine * (**theTE32KHandle).lineHeight;
			updateRect.bottom = updateRect.top + (**theTE32KHandle).lineHeight * numAffected;
		}
		
		else
		{
			LongRectToRect(&((**theTE32KHandle).viewRect),&tempRect);
			tempRect.top = (**theTE32KHandle).destRect.top + (firstLine - 1L + numAffected) * (**theTE32KHandle).lineHeight;
			
			GetPort(&oldPort);
			SetPort((**theTE32KHandle).inPort);
	
			updateRgn = NewRgn();
			ScrollRect(&tempRect,0,(**theTE32KHandle).lineHeight * deltaLines,updateRgn);
			
			SetPort(oldPort);
			
			updateRect.left = (**updateRgn).rgnBBox.left;
			updateRect.top = (**updateRgn).rgnBBox.top;
			updateRect.right = (**updateRgn).rgnBBox.right;
			updateRect.bottom = (**updateRgn).rgnBBox.bottom;
			
			DisposeRgn(updateRgn);
			
			TE32KUpdate(&updateRect,theTE32KHandle);
			
			updateRect = (**theTE32KHandle).viewRect;
			updateRect.top = (**theTE32KHandle).destRect.top + firstLine * (**theTE32KHandle).lineHeight;
			updateRect.bottom = updateRect.top + (**theTE32KHandle).lineHeight * numAffected;
		}
		
		TE32KUpdate(&updateRect,theTE32KHandle);
	}

	TE32KGetPoint((**theTE32KHandle).selStart,&selPt,theTE32KHandle);
	(**theTE32KHandle).selPoint = selPt;
}






static int	shiftKeyDown()
{
KeyMap	theKeyMap;

	GetKeys(theKeyMap);
	
	if (theKeyMap[1] & 0x01)
		return(TRUE);
	else
		return(FALSE);
}




static void MyClicker(void)
{
int			lineHeight;
Rect		viewRect;
Point		mousePoint;
RgnHandle	saveClip;
long		hDelta,vDelta;

	if (clickedTE32KH)
	{
		LongRectToRect(&((**clickedTE32KH).viewRect),&viewRect);
		lineHeight = (**clickedTE32KH).lineHeight;
	
		hDelta = 0L;
		vDelta = 0L;
		
		GetMouse(&mousePoint);
		
		if (!PtInRect(mousePoint,&viewRect))
		{
			if (mousePoint.v > viewRect.bottom && (**clickedTE32KH).viewRect.bottom < (**clickedTE32KH).destRect.top + (long) lineHeight * (**clickedTE32KH).nLines)
				vDelta = -lineHeight;
			
			else if (mousePoint.v < viewRect.top && (**clickedTE32KH).viewRect.top > (**clickedTE32KH).destRect.top)
				vDelta = lineHeight;
			
			
			if (mousePoint.h > viewRect.right && (**clickedTE32KH).viewRect.right < (**clickedTE32KH).destRect.right)
				hDelta = -lineHeight;
			
			else if (mousePoint.h<viewRect.left && (**clickedTE32KH).viewRect.left > (**clickedTE32KH).destRect.left)
				hDelta = lineHeight;
		}
		
		if (hDelta || vDelta)
		{
			saveClip = NewRgn();
			GetClip(saveClip);
			viewRect = (*((**clickedTE32KH).inPort)).portRect;
			ClipRect(&viewRect);
			
			TE32KScroll(hDelta,vDelta,clickedTE32KH);
			
			SetClip(saveClip);
			DisposeRgn(saveClip);
		}
	}
}




static void MyClickLoop(void)
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




void	TE32KAutoView(char autoView, TE32KHandle theTE32KHandle)
{
	if (theTE32KHandle)
	{
		if (!autoView)
			(**theTE32KHandle).clikLoop = nil;
		else
			(**theTE32KHandle).clikLoop = (TE32KProcPtr) MyClickLoop;
	}
}




