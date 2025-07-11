#define FILE_NUM 28
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment MyWindow
#define W_INSET 1
#define W_MIN 72
void DrawMyGrow(MyWindowPtr);
void HideMyControl(ControlHandle cntrlH);
void ShowMyControl(ControlHandle cntrlH);

/**********************************************************************
 * GetNewMyWindow - create a new window from a template
 **********************************************************************/
MyWindowPtr GetNewMyWindow(short resId,UPtr wStorage,WindowPtr behind,Boolean hBar, Boolean vBar)
{
	MyWindowPtr win;
	Rect oldContR;
	
	if (wStorage == nil)
	{
		if (HandyWindow)
		{
			wStorage = HandyWindow;
			HandyWindow = nil;
		}
		else if ((wStorage=NewPtr(sizeof(MyWindow)))==nil)
			return (nil);
	}
	WriteZero(wStorage, sizeof(MyWindow));
		
	win = GetNewWindow(resId, wStorage, behind);
	if (win==nil) return(nil);
	
	SetPort(win);
	win->isDialog = False;
	UsingWindow(win);
		
	/*
	 * add scroll bar(s)
	 */
	if (hBar)
	{
		if (!(win->hBar = GetNewControl(SCROLL_CNTL,win)))
			WarnUser(NO_CONTROL,ResError());
		else
		{
			SetCtlAction(win->hBar,ScrollAction);
			SetCtlMin(win->hBar,0);
			SetCtlValue(win->hBar,0);
			SetCtlMax(win->hBar,0);
		}
	}

	if (vBar)
	{
		if (!(win->vBar = GetNewControl(SCROLL_CNTL,win)))
			WarnUser(NO_CONTROL,ResError());
		else
		{
			SetCtlAction(win->vBar,ScrollAction);
			SetCtlMin(win->vBar,0);
			SetCtlValue(win->vBar,0);
			SetCtlMax(win->vBar,0);
		}
	}

	win->hPitch = FontWidth;
	win->vPitch = FontLead;
	win->minSize.h = win->minSize.v = W_MIN;
	TextFont(FontID);
	TextSize(FontSize);
	
	/*
	 * do size-related computations
	 */
	WriteZero(&oldContR,sizeof(Rect));
	MyWindowDidResize(win,&oldContR);
	
	/*
	 * hooray!
	 */
	win->isActive = FALSE;		/* let the activation event handle this */
	
	return(win);
}

/**********************************************************************
 * UpdateMyWindow - handle an update event for a window
 **********************************************************************/
void UpdateMyWindow(MyWindowPtr win)
{
	GrafPtr oldPort;
	int maxH, maxV;
	Rect r;
	ControlHandle cntl;
	
	/*
	 * set the port
	 */
	GetPort(&oldPort);
	SetPort(win);
	
	/*
	 * let the window manager move regions for us
	 */
	BeginUpdate(win);
	
	/*
	 * erase the invalid region
	 */
	SetRect(&r,-INFINITY,-INFINITY,INFINITY,INFINITY);
	EraseRect(&r);
	
	/*
	 * redraw the contents
	 */
	maxH = win->hMax; maxV = win->vMax;
	if (win->update) (*win->update)(win);
	else if (win->txe)
		TEUpdate(&((WindowPtr)win)->portRect,win->txe);
	else if (win->ste)
		STEUpdate(win->ste);
	
	/*
	 * draw the controls, etc.
	 */
	if (!win->isDialog)
	{
	  DrawMyControls(win);
		if (!win->isActive || InBG)
		  for (cntl=win->qWindow.controlList;cntl;cntl=(*cntl)->nextControl)
			  if ((*cntl)->contrlRfCon == 'SCRL') FRAME_RECT(&(*cntl)->contrlRect);
	}
	else
	{
		DrawDialog(win);
		HiliteButtonOne(win);
	}
	if (!win->isRunt) DrawMyGrow(win);

	/*
	 * put stuff back
	 */
	EndUpdate(win);
	
	/*
	 * the update routine may have necessitated a change in the scroll bars
	 */
	if (win->isActive&&
			(win->hBar||win->vBar)&&(maxH!=win->hMax||maxV!=win->vMax))
		DrawMyControls(win);
			
	/*
	 * reset the port
	 */
	SetPort(oldPort);
}

/**********************************************************************
 * move a window
 **********************************************************************/
void DragMyWindow(MyWindowPtr win,Point thePt)
{
	Rect bounds;
	
	/*
	 * calculate a bounding rectangle
	 */
	bounds = qd.screenBits.bounds;
	bounds.top += GetMBarHeight();
	InsetRect(&bounds,W_INSET,W_INSET);
	
	/*
	 * do it
	 */
	DragWindow(win,thePt,&bounds);
	
	win->saveSize = True;
	SFWTC = True;
}

/**********************************************************************
 * grow a window
 **********************************************************************/
void GrowMyWindow(MyWindowPtr win,Point thePt)
{
	GrafPtr oldPort;
	Rect bounds;
	long size;
	Point *newSize = (Point *)&size;
	Rect oldContR;
	
	/*
	 * save port
	 */
	GetPort(&oldPort);
	SetPort(win);

	/*
	 * calculate a bounding rectangle
	 */
	bounds.top=win->minSize.v;
	bounds.left=win->minSize.h;
	bounds.bottom = bounds.right = INFINITY;
	
	/*
	 * grow it
	 */
	size = GrowWindow(win,thePt,&bounds);
	if (size)
	{
		Rect r;
		
		/*
		 * invalidate grow region
		 */
		BlockMove(&((WindowPtr)win)->portRect,&r,sizeof(Rect));
		r.left = r.right - GROW_SIZE;
		r.top = r.bottom - GROW_SIZE;
		InvalRect(&r);
		
		/*
		 * resize the window, for real
		 */
		oldContR = win->contR;
		SizeWindow(win,newSize->h,newSize->v,TRUE);
		
		/*
		 * diddle it
		 */
		MyWindowDidResize(win,&oldContR);
	
		/*
		 * update it
		 */
		UpdateMyWindow(win);
		
		win->saveSize = True;
	}
	
	/*
	 * restore port
	 */
	SetPort(oldPort);
	SFWTC = True;
}

/**********************************************************************
 * zoom a window
 **********************************************************************/
void ZoomMyWindow(MyWindowPtr win,Point thePt,int partCode)
{
	GrafPtr oldPort;
	Rect oldStd = StdState(win);
	Rect oldUsr = UserState(win);
	Rect curState = CurState(win);
	Rect newStd;
	Boolean wasZoomed = AboutSameRect(&curState,&oldStd);
	
	/*
	 * save port
	 */
	GetPort(&oldPort);
	SetPort(win);
	
	/*
	 * watch that mouse...
	 */
	if (thePt.h == -1 || TrackBox(win,thePt,partCode))
	{
		Rect r;
		
		/*
		 * invalidate grow region
		 */
		BlockMove(&((WindowPtr)win)->portRect,&r,sizeof(Rect));
		r.left = r.right - GROW_SIZE;
		r.top = r.bottom - GROW_SIZE;
		InvalRect(&r);
		
		/*
		 * erase the window (grumble)
		 */
		EraseRect(&r);
		
		/*
		 * figure new zoom rect
		 */
		FigureZoom(win);
		newStd = StdState(win);
		
		/*
		 * if we were not zoomed, save our current state as the user state
		 */
		if (!wasZoomed) UserState(win) = curState;
		
		/*
		 * figure out what WE think the part code should be
		 */
		partCode = (wasZoomed && AboutSameRect(&newStd,&oldStd)) ? inZoomIn : inZoomOut;

		/*
		 * zoom it
		 *
		 */
		ZoomWindow(win,partCode,FALSE);
		
		/*
		 * if the window was in the "zoomed" state before, preserve the old
		 * user state; ZoomWindow will have set it to the previous state, which
		 * was the old standard state, which is not a very useful thing to do.
		 */
		if (wasZoomed)
			UserState(win) = oldUsr;
		
		/*
		 * note size change
		 */
		MyWindowDidResize(win,nil);
		
		/*
		 * update it all (grumble, grumble)
		 */
		InvalRect(&((WindowPtr)win)->portRect);

		/*
		 * update it
		 */
		UpdateMyWindow(win);
	}
	
	/*
	 * restore
	 */
	SetPort(oldPort);
	SFWTC = True;
}

/**********************************************************************
 * OffsetWindow - bump a window to its proper place
 **********************************************************************/
void OffsetWindow(MyWindowPtr win)
{
	short offset;
	Point corner;
	Rect oldContR;
	
	SetPort(win);
	offset = GetMBarHeight();
	if (IsMyWindow(win))
	{
		if (win->position)
		{
			oldContR = win->contR; 
			if ((*win->position)(False,win))
			{
				MyWindowDidResize(win,nil);
				return;
			}
		}
		if (PrefIsSet(PREF_ZOOM_OPEN) && !win->isRunt)
		{
			Point pt;
#ifdef WHAT_THE_HECK
			Rect r = UserState(win);
			void *oldPos = win->position;
			win->position = nil;
			MoveWindow(win,INFINITY/2,INFINITY/2,False);
			ShowWindow(win);
			EmptyRgn(win->qWindow.updateRgn);
			UpdateMyWindow(win);
			HideWindow(win);
#endif WHAT_THE_HECK
			pt.h = pt.v = -1;
			ZoomMyWindow(win,pt,0);
			return;
#ifdef WHAT_THE_HECK
			UserState(win) = r;
			win->position = oldPos;
#endif WHAT_THE_HECK
		}
	}
	utl_StaggerWindow(&((GrafPtr)win)->portRect,1,offset,&corner,
										GetRLong(PREF_STRN+PREF_NW_DEV));
	MoveWindow(win,corner.h,corner.v,False);
	if (IsMyWindow(win)) MyWindowDidResize(win,nil);
	SFWTC = True;
}

/**********************************************************************
 * MyWindowDidResize - handle a window whose size has changed
 **********************************************************************/
void MyWindowDidResize(MyWindowPtr win,Rect *oldContR)
{
	Rect r;
	Rect oldCont;
	SAVE_PORT;
	SetPort(win);
	
	if (!oldContR) SetRect(&oldCont,0,0,0,0);
	else oldCont = *oldContR;
	
	/*
	 * make a couple of copies of the portRect
	 */
	r = ((GrafPtr)win)->portRect;
	r.top += win->topMargin;
	win->contR = r;
	
	/*
	 * handle scroll bars
	 */
	if (win->hBar)
	{
		MoveMyCntl(win,win->hBar,r.left-1,r.bottom-GROW_SIZE,r.right-GROW_SIZE+2-r.left,GROW_SIZE+1);
		win->contR.bottom -= GROW_SIZE;
	}
	if (win->vBar)
	{
		MoveMyCntl(win,win->vBar,r.right-GROW_SIZE,r.top-1,GROW_SIZE+1,r.bottom-r.top-GROW_SIZE+2);
		win->contR.right -= GROW_SIZE;
	}
									
	/*
	 * grow region
	 */
	r.left = r.right - GROW_SIZE;
	r.top = r.bottom - GROW_SIZE;
	InvalRect(&r);
	
	/*
	 * window-specific adjustments
	 */
	if (win->didResize)
		(*win->didResize)(win,&oldCont);
	
	/*
	 * now the scroll bars
	 */
	MyWindowMaxes(win,win->hMax,win->vMax);
	REST_PORT;
}

/**********************************************************************
 * MoveMyCntl - move a control attached to a window
 **********************************************************************/
void MoveMyCntl(MyWindowPtr win,ControlHandle cntl,int h,int v,int w,int t)
{
#pragma unused(win)
	Rect r;
	Boolean oldVis = (*cntl)->contrlVis;
	
	if (oldVis)
	{
		HideControl(cntl);
		r=(*cntl)->contrlRect;
		InsetRect(&r,-1,-1);
		InvalRect(&r);
	}
	MoveControl(cntl,h,v);
	SizeControl(cntl,w,t);
	if (oldVis)
	{
		(*cntl)->contrlVis = True;
		r=(*cntl)->contrlRect;
		InsetRect(&r,-1,-1);
		InvalRect(&r);
	}
}

/**********************************************************************
 * ScrollMyWindow - scroll a window
 **********************************************************************/
void ScrollMyWindow(MyWindowPtr win,int h,int v)
{
	RgnHandle updateRgn;
	GrafPtr oldPort;
	
	GetPort(&oldPort);
	SetPort(win);
	
	if (!h && !v) return;
	
	if (win->ste)
	  STEScroll(win->ste,h,v);
	else if (!win->scroll || (*win->scroll)(win,h,v))
	{
		/*
		 * create a region for ScrollRect to use
		 */
		updateRgn = NewRgn();
		if (win->isActive && win->txe) INVAL_RECT(&(*win->txe)->selRect);
		
		ScrollRect(&win->contR,h*win->hPitch,v*win->vPitch,updateRgn);
		
		/*
		 * update region maintenance
		 */
		InvalRgn(updateRgn);
		DisposeRgn(updateRgn);
	}
	SetPort(oldPort);
}

/**********************************************************************
 * MyWindowMaxes	- set the max values for the scroll bars
 * takes the maximum number of units in each direction, and subtracts
 * the number of units represented by the window.
 **********************************************************************/
void MyWindowMaxes(MyWindowPtr win,int hMax,int vMax)
{
	int h=0,v=0;
	
	if (win->hBar)
	{
		win->hMax = hMax;
		h = BarMax(win->hBar,hMax,win->contR.right-win->contR.left,win->hPitch);
	}
	if (win->vBar)
	{
		win->vMax = vMax;
		v = BarMax(win->vBar,vMax,win->contR.bottom-win->contR.top,win->vPitch);
	}
	ScrollMyWindow(win,h,v);
}

/**********************************************************************
 * BarMax - set the max value for a scroll bar
 **********************************************************************/
int BarMax(ControlHandle cntl,int max,int winSize,int pitch)
{
		int old;
		
		old = GetCtlValue(cntl);
		max -= winSize/pitch;
		if (max < 0) max = 0;
		SetCtlMax(cntl,max);
		return(old>max ? old-max : 0);
}

/**********************************************************************
 * ScrollAction - action procedure for scroll bar
 **********************************************************************/
pascal void ScrollAction(ControlHandle theCtl,short partCode)
{
	MyWindowPtr win = (*theCtl)->contrlOwner;
	short inc=0,page;
	Boolean isH;
	Rect r = (*theCtl)->contrlRect;
	static uLong lastTicks;
	
	if (TickCount()-lastTicks<4) return;
	else lastTicks = TickCount();
  isH = r.right-r.left > r.bottom-r.top;
	page = (isH ? RoundDiv(r.right-r.left,win->hPitch)
							 : RoundDiv(r.bottom-r.top,win->vPitch))-1;

	switch(partCode)
	{
		case inUpButton:
			inc = -1;
			break;
		case inDownButton:
			inc = 1;
			break;
		case inPageUp:
			inc = -page;
			break;
		case inPageDown:
			inc = page;
			break;
	}
	
	if (inc) inc = -IncMyCntl(theCtl,inc);
	if (inc)
	{
		if (isH)
			ScrollMyWindow(win,inc,0);
		else
			ScrollMyWindow(win,0,inc);
		UpdateMyWindow(win);
	}
}


/************************************************************************
 * WinTEH - grab the active terec for a window
 ************************************************************************/
TEHandle WinTEH(MyWindowPtr win)
{
	if (IsMyWindow(win))
	{
		if (win->txe) return(win->txe);
		if (win->ste) return((*(STEHandle)win->ste)->te);
	}
	return(nil);
}

/************************************************************************
 * ScrollIsH - is a scroll bar a horizontal one?
 ************************************************************************/
Boolean ScrollIsH(ControlHandle cntl)
{
	Rect r = (*cntl)->contrlRect;

  return(r.right-r.left > r.bottom-r.top);
}

/**********************************************************************
 * ActivateMyWindow - handle an activate event for one of my windows
 **********************************************************************/
void ActivateMyWindow(MyWindowPtr win,Boolean active)
{
	GrafPtr oldPort;
	ControlHandle cntl;
	TEHandle teh = WinTEH(win);
	
	if (!win) return;
	GetPort(&oldPort);
	SetPort(win);
	
	win->isActive = active;
	
	for (cntl=win->qWindow.controlList;cntl;cntl=(*cntl)->nextControl)
	{
	  if ((*cntl)->contrlRfCon == 'SCRL')
		{
			if (!active)
				HideMyControl(cntl);
			else if (!InBG)
				ShowMyControl(cntl);
		}
	}
			
	if (!win->isRunt) DrawMyGrow(win);

	if (win->activate)
		(*win->activate)(win);

	if (teh)
	{
		if (win->isActive)
			TEActivate(teh);
		else
			TEDeactivate(teh);
	}
	
	if (win->isDialog)
	{
		MenuHandle mh = GetMHandle(EDIT_MENU);
		if (mh)
		{
			EnableIf(mh,EDIT_COPY_ITEM,active);
			EnableIf(mh,EDIT_CUT_ITEM,active);
			EnableIf(mh,EDIT_PASTE_ITEM,active);
			EnableIf(mh,EDIT_CLEAR_ITEM,active);
			EnableIf(mh,EDIT_UNDO_ITEM,active);
		}
		HiliteButtonOne(win);
	}
	
	if (active) SFWTC=True;
	
	SetPort(oldPort);
}

/**********************************************************************
 * InvertLine - invert a line in one of my windows.  assumes origin
 * is set correctly.
 **********************************************************************/
void InvertLine(MyWindowPtr win,int line)
{
	Rect r;
	
	r.top = (line-GetCtlValue(win->vBar))*win->vPitch;
	r.bottom = r.top + win->vPitch;
	r.left = 0;
	r.right = INFINITY;
	SectRect(&r,&win->contR,&r);
	BitClr((Ptr)HiliteMode, pHiliteBit);
	InvertRect(&r);
}

/**********************************************************************
 * CloseMyWindow - close a window, asking for confirmation if necessary
 **********************************************************************/
Boolean CloseMyWindow(MyWindowPtr win)
{
	Boolean rmHelp = HasHelp && win==(MyWindowPtr)FrontWindow();

	if (win->saveSize && win->position) (*win->position)(True,win);
	if (win->close && !(*win->close)(win)) return(False);
	if (win->txe) TEDispose(win->txe);
	NukeUndo(win);
	
	win->isDialog ? DisposDialog(win) : DisposeWindow(win);
	SFWTC = True;
	if (rmHelp) HMRemoveBalloon();

	return(True);
}

/**********************************************************************
 * GoAwayMyWindow - track a click in the go away box
 **********************************************************************/
void GoAwayMyWindow(WindowPtr win,Point pt)
{
	SetPort(win);
	if (TrackGoAway(win,pt))
		CloseMyWindow(win);
}

/**********************************************************************
 * DrawMyGrow - draw a grow icon, taking into account what scroll bars
 * the window has
 **********************************************************************/
#define BIGSIDE 8
#define SMALLSIDE 6
#define OFFSET 4
void DrawMyGrow(MyWindowPtr win)
{
	Rect r;
	
	/*
	 * draw the borders of the scroll bars (if any, and if the window
	 * is inactive (the scroll bars will be drawn elsewhere if the
	 * window is active)
	 */
	if (!win->isActive)
	{
		if (win->hBar)
		{
			MoveTo(win->contR.left,win->contR.bottom);
			LineTo(INFINITY,win->contR.bottom);
		}
		if (win->vBar)
		{
			MoveTo(win->contR.right,win->contR.top);
			LineTo(win->contR.right,INFINITY);
		}
	}

	r = ((GrafPtr)win)->portRect;
	r.left = r.right - GROW_SIZE;
	r.top = r.bottom - GROW_SIZE;
	{
		RgnHandle oldClip = ((GrafPtr)win)->clipRgn;
		((GrafPtr)win)->clipRgn=NewRgn();
		ClipRect(&r);
		DrawGrowIcon(win);
		DisposeRgn(((GrafPtr)win)->clipRgn);
		((GrafPtr)win)->clipRgn = oldClip;
	}
}

/**********************************************************************
 * HideMyControl - hide a control, but do it right
 **********************************************************************/
void HideMyControl(ControlHandle cntrlH)
{
	(*cntrlH)->contrlVis = 0;
	WhiteRect(&(*cntrlH)->contrlRect);
	VALID_RECT(&(*cntrlH)->contrlRect);
}

/**********************************************************************
 * ShowMyControl - show a control, but do it right
 **********************************************************************/
void ShowMyControl(ControlHandle cntrlH)
{
	ShowControl(cntrlH);
	VALID_RECT(&(*cntrlH)->contrlRect);
}

/************************************************************************
 * ShowMyWindow - make a window visible
 ************************************************************************/
void ShowMyWindow(MyWindowPtr win)
{
	short kind = win->qWindow.windowKind;
	
	if (win->qWindow.visible) return;	/* do nothing */
	if (win->gonnaShow) (*win->gonnaShow)(win);
	OffsetWindow(win);
	ShowWindow(win);
	SelectWindow(win);
	ActivateMyWindow(win,!InBG);
	SetRectRgn(win->qWindow.updateRgn,-INFINITY/2,-INFINITY/2,INFINITY/2,INFINITY/2);
	SFWTC = True;
}

/************************************************************************
 * MyClikLoop - autoscroll a TERec
 ************************************************************************/
pascal Boolean MyClikLoop(void)
{
	MyWindowPtr win=FrontWindow();
	static long lastTicks;
	Rect r;
	
	if (TickCount()-lastTicks>3 && ((WindowPeek)win)->windowKind >= userKind)
	{
		Point pt;
		int deltaV=0;
		
		if (ClickType==Triple) TESelPara(win->txe);
		lastTicks = TickCount();
		GetMouse(&pt);
		if (pt.v < win->contR.top)
			deltaV = (win->contR.top-pt.v)*3/(int)win->vPitch/4+1;
		else if (pt.v > win->contR.bottom)
			deltaV = (win->contR.bottom-pt.v)*3/(int)win->vPitch/4-1;
		if (deltaV)
		{
			if (((WindowPeek)win)->windowKind==COMP_WIN)
			{
				r = win->contR;
				if (deltaV>0)
					r.bottom = r.top + (deltaV+2)*win->vPitch;
				else
					r.top = r.bottom - (2-deltaV)*win->vPitch;
				InvalRect(&r);
			}
			ScrollIt(win,0,deltaV);
			if (((WindowPeek)win)->windowKind==COMP_WIN)
			{
				SetRectRgn(((GrafPtr)win)->clipRgn,(*win->txe)->viewRect.left,
								(*win->txe)->viewRect.top,(*win->txe)->viewRect.right,
								(*win->txe)->viewRect.bottom);
			}
		}
	}
	return(True);
}

/************************************************************************
 * ContentClip - set a windows's clipping region to its content region
 ************************************************************************/
void ContentClip(MyWindowPtr win)
{
	SetPort(win);
	ClipRect(&win->contR);
}

/************************************************************************
 * InfiniteClip - set a window's clipping region to infinity
 ************************************************************************/
void InfiniteClip(MyWindowPtr win)
{
	Rect r;
	SetPort(win);
	SetRect(&r,-INFINITY,-INFINITY,INFINITY,INFINITY);
	ClipRect(&r);
}

/************************************************************************
 * ScrollIt - scroll a window, doing all the nasty things that have to
 * be done.
 ************************************************************************/
void ScrollIt(MyWindowPtr win,int deltaH,int deltaV)
{
	Handle oldClip;
	int hMin,hMax,hVal;
	int vMin,vMax,vVal;
	
	SetPort(win);
	if (deltaH && win->hBar)
	{
		hMin = GetCtlMin(win->hBar);
		hMax = GetCtlMax(win->hBar);
		hVal = GetCtlValue(win->hBar);
		if (deltaH<0)
		{
			if (deltaH<hVal-hMax) deltaH = hVal-hMax;
		}
		else
		{
			if (deltaH>hVal-hMin) deltaH = hVal-hMin;
		}
	}
	else deltaH = 0;
	if (deltaV && win->vBar)
	{
		vMin = GetCtlMin(win->vBar);
		vMax = GetCtlMax(win->vBar);
		vVal = GetCtlValue(win->vBar);
		if (deltaV<0)
		{
			if (deltaV<vVal-vMax) deltaV = vVal-vMax;
		}
		else
		{
			if (deltaV>vVal-vMin) deltaV = vVal-vMin;
		}
	}
	else deltaV = 0;
	
	if (deltaH || deltaV)
	{ 	
		oldClip = win->qWindow.port.clipRgn;
		win->qWindow.port.clipRgn = NewRgn();
		InfiniteClip(win);
		if (deltaH) SetCtlValue(win->hBar,hVal-deltaH);
		if (deltaV) SetCtlValue(win->vBar,vVal-deltaV);
		ScrollMyWindow(win,deltaH,deltaV);
		UpdateMyWindow(win);
		DisposeRgn(win->qWindow.port.clipRgn);
		win->qWindow.port.clipRgn = oldClip;
	}
}

/************************************************************************
 * EraseUpdateRgn - erase a window's update region
 ************************************************************************/
void EraseUpdateRgn(WindowPeek win)
{
	Point origin;
	
	SetPort(win);
	origin.h = origin.v = 0;
	LocalToGlobal(&origin);
	OffsetRgn(win->updateRgn,-origin.h,-origin.v);
	EraseRgn(win->updateRgn);
	OffsetRgn(win->updateRgn,origin.h,origin.v);
}

/************************************************************************
 * InvalContent - invalidate a window's content region
 ************************************************************************/
void InvalContent(MyWindowPtr win)
{
	GrafPtr oldPort;
	
	GetPort(&oldPort);
	SetPort(win);
	InvalRect(&win->contR);
	SetPort(oldPort);
}

/************************************************************************
 * CalcCntlInc - do the necessary drudge work before changing a control
 ************************************************************************/
short CalcCntlInc(ControlHandle cntl,short tentativeInc)
{
	short current,max,min,new;
	
	if (!tentativeInc) return(0);
	new = current = GetCtlValue(cntl);
	if (tentativeInc<0)
	{
		min = GetCtlMin(cntl);
		if (current > min) new = MAX(min,current+tentativeInc);
	}
	else
	{
		max = GetCtlMax(cntl);
		if (current < max) new = MIN(max,current+tentativeInc);
	}
	
	return(new-current);
}

/************************************************************************
 * IncMyCntl - change a control.  Returns the change in value.
 ************************************************************************/
short IncMyCntl(ControlHandle cntl,short inc)
{
	inc = CalcCntlInc(cntl,inc);
	if (inc) SetCtlValue(cntl,GetCtlValue(cntl)+inc);
	return(inc);
}

/************************************************************************
 * UsingWindow - mark a window as in use
 ************************************************************************/
void UsingWindow(MyWindowPtr win)
{
	if (IsMyWindow(win)) win->inUse = True;
}

/************************************************************************
 * NotUsingWindow - mark a window as not in use
 ************************************************************************/
void NotUsingWindow(MyWindowPtr win)
{
	if (IsMyWindow(win)) win->inUse = False;
}

/************************************************************************
 * NotUsingAllWindows - mark all windows as not in use
 ************************************************************************/
void NotUsingAllWindows(void)
{
	MyWindowPtr win;
	
	for (win=FrontWindow();win;win=win->qWindow.nextWindow)
		NotUsingWindow(win);
}

/************************************************************************
 * FigureZoom - figure out the proper zoomed state for a window
 ************************************************************************/
void FigureZoom(MyWindowPtr win)
{
	GDHandle gd;
	Rect zoom,regular;
	Boolean hasMB;
	short titleHi;
	Point corner;
	Boolean isVis = win->qWindow.visible;
	
	utl_GetWindGD(win,&gd,&zoom,&regular,&hasMB);
	InsetRect(&zoom,2,2);
	zoom.left += GetRLong(DESK_LEFT_STRIP);
	zoom.right -= GetRLong(DESK_RIGHT_STRIP);
	zoom.bottom -= GetRLong(DESK_BOTTOM_STRIP);
	zoom.top += GetRLong(DESK_TOP_STRIP);
	if (hasMB) zoom.top += GetMBarHeight();
	if (isVis)
		titleHi = (*win->qWindow.contRgn)->rgnBBox.top -
							(*win->qWindow.strucRgn)->rgnBBox.top;
	else
		titleHi = titleBarHeight;
	zoom.top += titleHi;
	
	if (IsMyWindow(win) && win->zoomSize) (*win->zoomSize)(win,&zoom);
	corner.h = corner.v = 0;
	
	win->qWindow.visible = False;	/* make staggerwindow ignore us */
	utl_StaggerWindow(&zoom,1,GetMBarHeight(),&corner,GDIndexOf(gd));
	win->qWindow.visible = isVis;	/* put visible back */
	OffsetRect(&zoom,corner.h-zoom.left,corner.v-zoom.top);
	
	StdState(win) = zoom;
}

/************************************************************************
 * GDIndexOf - find the index of a given gd
 ************************************************************************/
short GDIndexOf(GDHandle gd)
{
	short i = 0;
	GDHandle candidate;
	Rect r;
	Boolean b;
	
	if (utl_HaveColor())
	{
	  for (utl_GetIndGD(++i,&candidate,&r,&b);candidate;utl_GetIndGD(++i,&candidate,&r,&b))
		  if (candidate==gd) return(i);
	}
	return(0);
}

