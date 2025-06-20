#define FILE_NUM 33
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * Progress monitoring software
 ************************************************************************/
#pragma load EUDORA_LOAD
#pragma segment Progress

/************************************************************************
 * other stuff
 ************************************************************************/
typedef struct {
	short percent;
	Str255 message;
	int on, total;
	short n;
	Handle next;
} ProgressBlock, *ProgressBPtr, **ProgressBHandle;
Boolean ProgressOff;

MyWindowPtr ProgWindow;

/************************************************************************
 * Private functions
 ************************************************************************/
void ProgressUpdate(MyWindowPtr win);
void DisposProgress(ProgressBHandle prbl);
void CenterProgress(MyWindowPtr win);
void InvalProgress(Boolean message,Boolean percent);
void DrawProgressBar(void);

/************************************************************************
 * ByteProgress - keep track of the number of bytes transmitted so far
 ************************************************************************/
void ByteProgress(UPtr message, int onLine, int totLines)
{
	ProgressBlock **prbl;
	
	CycleBalls();
	if (!ProgWindow) return;
	
	prbl = (ProgressBlock **)((WindowPeek)ProgWindow)->refCon;
	
	if (onLine>=0) (*prbl)->on = onLine;
	else (*prbl)->on -= onLine;
	if (totLines) (*prbl)->total = totLines;
	
	if (!(*prbl)->total) (*prbl)->total = 1;
	Progress(((*prbl)->on*100)/(*prbl)->total,message);
}

/************************************************************************
 * OpenProgress - create the progress window
 ************************************************************************/
int OpenProgress(void)
{
	int err;
	ProgressBlock **prbl;
	MyWindowPtr win;
	
	if (ProgWindow || PrefIsSet(PREF_NO_PROGRESS)) return(0);
	if ((win=GetNewMyWindow(PROGRESS_WIND,nil,InFront,False,False))==nil)
	{
		WarnUser(COULDNT_WIN,err=MemError());
		return(err);
	}
	
	InsetRect(&win->contR,8,8);
	
	prbl = NewZHandle(sizeof(ProgressBlock));
	if (!prbl) {WarnUser(MEM_ERR,err=MemError()); DisposeWindow(win); return(err);}
	
	((WindowPeek)win)->refCon = (long) prbl;
	win->update = ProgressUpdate;
	(*prbl)->percent = -1;
	SetPort(win);
	TextFont(0);
	TextSize(12);
	ProgWindow = win;
	win->isRunt = True;
	if (InBG) DisableProgress(); else CenterProgress(ProgWindow);
	ShowWindow(win);
	return(noErr);
}

/************************************************************************
 * CloseProgress - close the progress window
 ************************************************************************/
void CloseProgress(void)
{
	if (!ProgWindow) return;
	DisposProgress((Handle)((WindowPeek)ProgWindow)->refCon);
	CloseMyWindow(ProgWindow);
	ProgWindow = nil;
}

/************************************************************************
 * DisposProgress - get rid of the progress chain
 ************************************************************************/
void DisposProgress(ProgressBHandle prbl)
{
	if (prbl==nil) return;
	DisposProgress((*prbl)->next);
	DisposHandle(prbl);
}

/************************************************************************
 * Progress - record progress in the progress window
 ************************************************************************/
void Progress(short percent,UPtr message)
{
	ProgressBlock **prbl;
	GrafPtr oldPort;
	Str255 scratch;
	
	if (!ProgWindow) return;
	prbl = (ProgressBlock **)((WindowPeek)ProgWindow)->refCon;
	if (percent!=NoChange && percent!=(*prbl)->percent || message)
	{
		if (percent!=NoChange && percent!=(*prbl)->percent)
		{
		  if ((*prbl)->percent == NoBar || percent==NoBar)
			{
				InvalContent(ProgWindow);
				(*prbl)->percent = percent;
			}
			else
			{
				(*prbl)->percent = percent;
				DrawProgressBar();
			}
		}
		if (message)
		{
			register UPtr to,from;
			register unsigned short count;
			for (to=scratch+1,from=message+1,count= *message;
					 count; count--,from++)
			{
				if (*from >= ' ') *to++ = *from;
			}

			*scratch = to - scratch - 1;
			if (!EqualString(scratch,LDRef(prbl)->message,False,True))
			{
				PCopy((*prbl)->message,scratch);
				InvalProgress(True,False);
				Log(LOG_PROG,LDRef(prbl)->message);
			}
			UL(prbl);
		}
		GetPort(&oldPort);
    if (FrontWindow()!=ProgWindow)
			SelectWindow(ProgWindow);
		if (InBG) DisableProgress(); else EnableProgress();
		UpdateMyWindow(ProgWindow);
	}
}

/************************************************************************
 * InvalProgress - invalidate the selected part of the progress window
 ************************************************************************/
void InvalProgress(Boolean message,Boolean percent)
{
	Rect messageRect,percentRect;
	ProgressBlock **prbl = (ProgressBlock **)((WindowPeek)ProgWindow)->refCon;
	SAVE_PORT;
	SetPort(ProgWindow);
	messageRect = percentRect = ProgWindow->contR;
	if ((*prbl)->percent >= 0)
		messageRect.bottom=percentRect.top = (percentRect.top+percentRect.bottom)/2;
	if (message) InvalRect(&messageRect);
	if (percent) InvalRect(&percentRect);
	REST_PORT;
}


/************************************************************************
 * ProgressUpdate - update the progress window
 ************************************************************************/
void ProgressUpdate(MyWindowPtr win)
{
	Rect aRect;
	ProgressBlock **prbl = (ProgressBlock **)((WindowPeek)ProgWindow)->refCon;
	int width;
	int h,v, wl, wr, wt, wb;
		
	aRect = win->contR;
	InsetRect(&aRect,-6,-6);
	ClipRect(&aRect);
	PenSize(2,2);
	if (InBG) PenPat(&qd.gray);
	FrameRect(&aRect);
	PenNormal();
	
	ClipRect(&win->contR);
	wl = win->contR.left; wr = win->contR.right;
	wt = win->contR.top; wb = win->contR.bottom;
	width = StringWidth(LDRef(prbl)->message); UL(prbl);
	h = (wl+wr-width)/2;
	v = (*prbl)->percent < 0 ? wb : (wb+wt)/2;
	SetRect(&aRect,wl,wt,wr,v);
	EraseRect(&aRect);
	MoveTo(h,wt + (v-wt)/2+4);
	DrawString(LDRef(prbl)->message); UL(prbl);
	
	if ((*prbl)->percent >= 0) DrawProgressBar();
}


/************************************************************************
 * 
 ************************************************************************/
void DrawProgressBar(void)
{
	Rect aRect;
	ProgressBlock **prbl = (ProgressBlock **)((WindowPeek)ProgWindow)->refCon;
	short wr;
	SAVE_PORT;
	SetPort(ProgWindow);
		
	aRect = ProgWindow->contR;
	InsetRect(&aRect,-6,-6);
	aRect.top = (aRect.top+aRect.bottom)/2;
	wr = aRect.right;
	aRect.right = ((wr-aRect.left)*(*prbl)->percent)/100;
	aRect.right = MIN(wr,aRect.right);
	FillRect(&aRect,&qd.black);
	aRect.left = aRect.right;
	aRect.right = wr;
	FillRect(&aRect,&qd.gray);
	REST_PORT;
}

/************************************************************************
 * DisableProgress - make the progress calls do nothing
 ************************************************************************/
void DisableProgress(void)
{
	if (ProgWindow)
		MoveWindow(ProgWindow,INFINITY/2,INFINITY/2,False);
	ProgressOff = True;
}

/************************************************************************
 * EnableProgress - turn the progress calls back on
 ************************************************************************/
void EnableProgress(void)
{
	if (ProgressOff && ProgWindow)
	{
		CenterProgress(ProgWindow);
		UpdateMyWindow(ProgWindow);
	}
	ProgressOff = False;
}

/************************************************************************
 * PushProgress - stash a copy of the progress info
 ************************************************************************/
void PushProgress(void)
{
	ProgressBHandle prbl;
	ProgressBHandle pH;
	
	if (!ProgWindow) return;
	prbl = (ProgressBlock **)((WindowPeek)ProgWindow)->refCon;
	if (pH=NuHandle(sizeof(ProgressBlock)))
	{
		**pH = **prbl;
		(*pH)->next = prbl;
		((WindowPeek)ProgWindow)->refCon = (long) pH;
	}
}

/************************************************************************
 * PopProgress - restore the progress info
 ************************************************************************/
void PopProgress(Boolean messageOnly)
{
	ProgressBHandle prbl;
	ProgressBHandle pNext;
	
	if (!ProgWindow) return;
	prbl = (ProgressBlock **)((WindowPeek)ProgWindow)->refCon;
	if ((*prbl)->next)
	{
		pNext = (*prbl)->next;
		if (messageOnly)
		{
			(*pNext)->percent = (*prbl)->percent;
			(*pNext)->on = (*prbl)->on;
			(*pNext)->total = (*prbl)->total;
			InvalProgress(True,False);
		}
	  else InvalContent(ProgWindow);
			
		((WindowPeek)ProgWindow)->refCon = (long) pNext;
		DisposHandle(prbl);
		UpdateMyWindow(ProgWindow);
	}
}
	
/************************************************************************
 *
 ************************************************************************/
void SetProgressN(short n)
{
	ProgressBlock **prbl;
	if (!ProgWindow) return;
	prbl = (ProgressBlock **)((WindowPeek)ProgWindow)->refCon;
	(*prbl)->n = n;
}

/************************************************************************
 *
 ************************************************************************/
void CenterProgress(MyWindowPtr win)
{
	Rect r = ((GrafPtr)win)->portRect;
	Rect screenR = qd.screenBits.bounds;
	TopCenterRectIn(&r,&screenR);
	MoveWindow(win,r.left,r.top+GetMBarHeight()+1,False);
}

/************************************************************************
 * ProgressIsOpen - is the progress window open?
 ************************************************************************/
Boolean ProgressIsOpen(void)
{
	return(ProgWindow||PrefIsSet(PREF_NO_PROGRESS));
}
