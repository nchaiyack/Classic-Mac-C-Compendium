#define FILE_NUM 48
/* Copyright (c) 1992 by Qualcomm, Inc. */
/* written by Steven Dorner */
#pragma load EUDORA_LOAD
#pragma segment STE

#define CanEdit (*ste)->canEdit
#define HBar (*ste)->hBar
#define VBar (*ste)->vBar
#define TE (*ste)->te
#define EncloseR (*ste)->encloseR
#define DontFrame (*ste)->dontFrame
#define GrowBox (*ste)->growBox
/************************************************************************
 * Handling for scrolling TE fields within a window
 ************************************************************************/
	void GetSTERects(STEHandle ste,Rect *enclosingRect,Rect *viewRect);
	pascal Boolean STEClikLoop(void);
	void STEScrollWithBars(STEHandle ste,short h,short v);

/************************************************************************
 * NewSTE - create a new one
 ************************************************************************/
STEHandle NewSTE(MyWindowPtr win,Rect *enclosingRect,Boolean hBar,Boolean vBar,Boolean canEdit)
{
	STEHandle ste;
	Handle grumble;
	SAVE_PORT;
	
	SetPort(win);
	
	/*
	 * the record itself
	 */
	if (!(ste = NewZH(STE))) goto fail;
	if (!(grumble = TENew(enclosingRect,enclosingRect))) goto fail;
	TE = grumble;
	CanEdit = canEdit;
	(*TE)->txFont = FontID;
	(*TE)->txSize = FontSize;
	(*TE)->fontAscent = FontAscent;
	(*TE)->lineHeight = FontLead;
	
	TEAutoView(True,TE);
	if (hBar || vBar)
		(*TE)->clikLoop = STEClikLoop;
	
	/*
	 * scroll bars
	 */
	if (hBar)
	{
		if (!(grumble = GetNewControl(SCROLL_CNTL,win))) goto fail;
		HBar = grumble;
		SetCtlAction(HBar,ScrollAction);
	}
	if (vBar)
	{
		if (!(grumble = GetNewControl(SCROLL_CNTL,win))) goto fail;
		VBar = grumble;
		SetCtlAction(VBar,ScrollAction);
	}
	
	/*
	 * size properly and we're done
	 */
	if (enclosingRect) ResizeSTE(ste,enclosingRect);
	REST_PORT;
	return(ste);

	fail:
		REST_PORT;
		STEDispose(ste);
		return(nil);
}
	
#pragma segment Main
/************************************************************************
 * STEDispose - so long
 ************************************************************************/
void STEDispose(STEHandle ste)
{
	if (ste)
	{
		if (HBar) DisposeControl(HBar);
		if (VBar) DisposeControl(VBar);
		if (TE) TEDispose(TE);
		DisposeHandle(ste);
	}
}
#pragma segment STE
/************************************************************************
 * STEClick - handle a mouse click.  Returns True if event was handled.
 ************************************************************************/
Boolean STEClick(STEHandle ste,EventRecord *event)
{
	Point pt;
	MyWindowPtr win = (*TE)->inPort;
	Rect vR,eR;
	
	GetSTERects(ste,&eR,&vR);
	
	pt = event->where; GlobalToLocal(&pt);
	if (!PtInRect(pt,&eR)) return(False);
	if (HandleControl(pt,win))
		;
	else if (PtInRect(pt,&vR))
	{
		TEClick(pt,(event->modifiers&shiftKey)!=0,TE);
		if (ClickType==Triple) TESelPara(TE);
		win->hasSelection = (*TE)->selStart != (*TE)->selEnd;
		STESetScrolls(ste);
	}
	return(True);
}

/************************************************************************
 * ResizeSTE - size it properly, or handle scrolling, etc.
 ************************************************************************/
void ResizeSTE(STEHandle ste,Rect *newRect)
{
	Rect encR, viewR;
	short extreme;
	short offset;
	short topOffset;
	Boolean vis;
	short h,v;

	if (newRect)
	{
		vis = ((WindowPeek)(*TE)->inPort)->visible;
		/*
		 * invalidate the old rectangle
		 */
		GetSTERects(ste,&encR,&viewR);
		InvalRect(&encR);
				
		/*
		 * recalculate stuff
		 */
		if (vis) topOffset = TopOffset(TE);
		EncloseR = encR = *newRect;/* new scroll bar locations */
		if (HBar) encR.bottom -= GROW_SIZE;
		if (VBar)
		{
			encR.right -= GROW_SIZE;
			extreme = encR.bottom;
			if (!HBar && GrowBox) extreme -= GROW_SIZE-1;
			MoveMyCntl((*TE)->inPort,VBar,encR.right-1,encR.top,
									GROW_SIZE+1,extreme-encR.top);
		}
		if (HBar)
		{
			extreme = encR.right;
			if (!VBar && GrowBox) extreme -= GROW_SIZE;
			MoveMyCntl((*TE)->inPort,HBar,encR.left,encR.bottom-1,
													extreme-encR.left,GROW_SIZE+1);
		}

		InsetRect(&encR,TE_HMARGIN,TE_VMARGIN);		/* and the edit box itself */
		offset = (encR.bottom-encR.top)%(*TE)->lineHeight;
		encR.top += offset/2;
		encR.bottom -= offset-offset/2;
		offset = (*TE)->viewRect.top-encR.top;	/* amount of adjustment */
		(*TE)->viewRect = encR;
		(*TE)->destRect.left = encR.left;
		(*TE)->destRect.right = encR.right;
		if (!vis)
		{
			(*TE)->destRect.top = encR.top;
			(*TE)->destRect.bottom = encR.bottom;
		}
		else (*TE)->destRect.top -= offset;		/* keep view and dest in synch */
		NoScrollTECalText(TE);
		INVAL_RECT(&(*TE)->viewRect);
		if (vis) MakeTopOffset(TE,topOffset);
	}
	
	/*
	 * take care of the scroll bars
	 */
	GetSTERects(ste,&encR,&viewR);
	h=v=0;
	if (VBar)
	{
		BarMax(VBar,CountTeLines(TE),viewR.bottom-viewR.top,FontLead);
		v = RoundDiv(viewR.top-(*TE)->destRect.top,FontLead);
		if (v>GetCtlMax(VBar)) v = v-GetCtlMax(VBar);
		else v = 0;
	}
	if (HBar)
	{
		BarMax(HBar,TEMaxLine(TE),viewR.right-viewR.left,FontWidth);
		h = RoundDiv(viewR.left-(*TE)->destRect.left,FontWidth);
		if (h>GetCtlMax(HBar)) h = h-GetCtlMax(HBar);
		else h = 0;
	}
	STEScroll(ste,h,v);
	STESetScrolls(ste);
}

/************************************************************************
 * GetSTERects - extract the enclosing and view rectangles
 ************************************************************************/
void GetSTERects(STEHandle ste,Rect *enclosingRect,Rect *viewRect)
{
	*enclosingRect = EncloseR;
	*viewRect = (*TE)->viewRect;
}

/************************************************************************
 * STESetText - set the text of a scrolling area
 ************************************************************************/
void STESetText(UPtr text,short length,STEHandle ste)
{
	SAVE_PORT;
	SetPort((*TE)->inPort);
	INVAL_RECT(&(*TE)->viewRect);
	TESetText("",0,TE);
	STEScrollWithBars(ste,INFINITY,INFINITY);
	TESetText(text,length,TE);
	ResizeSTE(ste,nil);
	REST_PORT;
}

/************************************************************************
 * STEAppendText - add to the text of a scrolling area
 ************************************************************************/
void STEAppendText(UPtr text,short length,STEHandle ste)
{
	SAVE_PORT;
	SetPort((*TE)->inPort);
	TESetSelect(INFINITY,INFINITY,TE);
	TEInsert(text,length,TE);
	ResizeSTE(ste,nil);
	REST_PORT;
}

/************************************************************************
 * STEInstallText - install a whole new text in a scrolling area
 ************************************************************************/
void STEInstallText(Handle text,STEHandle ste)
{
	SAVE_PORT;
	SetPort((*TE)->inPort);
	DisposeHandle((*TE)->hText);
	(*TE)->hText = text;
	(*TE)->teLength = GetHandleSize(text);
	TECalText(TE);
	ResizeSTE(ste,nil);
	INVAL_RECT(&(*TE)->viewRect);
	REST_PORT;
}

/************************************************************************
 * STECursor - set the cursor properly
 ************************************************************************/
Boolean STECursor(STEHandle ste)
{
	Point pt;
	Rect eR, vR;
	Boolean found=False;
	
	GetMouse(&pt);
	GetSTERects(ste,&eR,&vR);
	if (CursorInRect(pt,eR,MouseRgn))
	{
		if (CursorInRect(pt,vR,MouseRgn))
			SetMyCursor(iBeamCursor);
		else
			SetMyCursor(arrowCursor);
		found = True;
	}
	else
		SetMyCursor(arrowCursor);
	return(found);
}

/************************************************************************
 * STEShowInsert - show the insertion point
 ************************************************************************/
void STEShowInsert(STEHandle ste)
{
	Rect r = (*TE)->viewRect;
	short dv;
	if (dv=TEInsertDelta(TE,&r,InsertAny))
		STEScrollWithBars(ste,0,dv);
	STESetScrolls(ste);
}

/************************************************************************
 * STEUpdate - update an STE
 ************************************************************************/
void STEUpdate(STEHandle ste)
{
	Rect encR,viewR;
	
	GetSTERects(ste,&encR,&viewR);
	if (!DontFrame)
	{
		if (HBar) encR.bottom -= GROW_SIZE;
		if (VBar) encR.right -= GROW_SIZE;
		FrameRect(&encR);
	}
	TEUpdate(&viewR,TE);
}

/************************************************************************
 * STESetScrolls - set the scroll bars according to how the field is scrolled
 ************************************************************************/
void STESetScrolls(STEHandle ste)
{
	Rect dest,view;
	short v;
	GetSTERects(ste,&dest,&view);
	dest = (*TE)->destRect;
	if (HBar)
	{
		v = RoundDiv(view.left - dest.left,FontWidth);
		SetCtlValue(HBar,MAX(0,v));
	}
	if (VBar)
	{
		v = RoundDiv(view.top - dest.top,FontLead);
		SetCtlValue(VBar,MAX(0,v));
	}
}

/************************************************************************
 * STEClikLoop - keep the scroll bars up-to-date
 ************************************************************************/
pascal Boolean STEClikLoop(void)
{
	MyWindowPtr win = FrontWindow();
	static lastTicks;
	
	if (win && TickCount()-lastTicks>3)
	if (win)
	{
		STEHandle ste = ((MyWindowPtr)(FrontWindow()))->ste;
		Point pt;
		Rect viewR,encR;
		if (ClickType==Triple) TESelPara(TE);
		GetSTERects(ste,&encR,&viewR);
		GetMouse(&pt);
		if (pt.v < viewR.top) pt.v = viewR.top-pt.v;
		else if (pt.v > viewR.bottom) pt.v = viewR.bottom-pt.v;
		else pt.v = 0;
		if (pt.h < viewR.left) pt.h = viewR.left-pt.h;
		else if (pt.h > viewR.right) pt.h = viewR.right-pt.h;
		else pt.h = 0;
		if (pt.h || pt.v)
		{
		  pt.h = RoundDiv(pt.h,win->hPitch);
			pt.v = RoundDiv(pt.v,win->vPitch);
		  STEScrollWithBars(ste,pt.h,pt.v);
		}
	}
	return(True);
}

/************************************************************************
 * STEScroll - scroll the area
 ************************************************************************/
void STEScroll(STEHandle ste,short h,short v)
{
  MyWindowPtr win = (*TE)->inPort;
	TEScroll(win->hPitch*h,win->vPitch*v,TE);
}


/************************************************************************
 * STEScrollWithBars - change the values of the scroll bars, and scroll
 ************************************************************************/
void STEScrollWithBars(STEHandle ste,short h,short v)
{
  MyWindowPtr win = (*TE)->inPort;
	RgnHandle oldClip = win->qWindow.port.clipRgn;
	win->qWindow.port.clipRgn = NewRgn();
	InfiniteClip(win);
	h = HBar ? -IncMyCntl(HBar,-h) : 0;
	v = VBar ? -IncMyCntl(VBar,-v) : 0;
	DisposeRgn(win->qWindow.port.clipRgn);
	win->qWindow.port.clipRgn = oldClip;
	STEScroll(ste,h,v);
}

/************************************************************************
 * STETextChanged - the text has changed in a scrolling window
 ************************************************************************/
void STETextChanged(STEHandle ste)
{
	ResizeSTE(ste,nil);
	STEShowInsert(ste);
}

/************************************************************************
 * STEApp1 - handle the special keys
 ************************************************************************/
void STEApp1(STEHandle ste,EventRecord *event)
{
	Rect encR,viewR;
	short page;
	
	GetSTERects(ste,&encR,&viewR);
	page = RoundDiv(viewR.bottom-viewR.top,FontLead)-1;
	
	switch(event->message & charCodeMask)
	{
		case homeChar:
			STEScrollWithBars(ste,INFINITY,INFINITY);
			break;
		case endChar:
			STEScrollWithBars(ste,0,-INFINITY);
			break;
		case pageUpChar:
			STEScrollWithBars(ste,0,page);
			break;
		case pageDownChar:
			STEScrollWithBars(ste,0,-page);
			break;
	}
}