#define FILE_NUM 43
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * various useful functions with at least a tenuous connection to windows
 **********************************************************************/
#pragma load EUDORA_LOAD
#pragma segment Util

/**********************************************************************
 * Get the maximum size of a window
 **********************************************************************/
void GetDisplayRect(Rect *rectPtr)
{
	rectPtr->top = qd.screenBits.bounds.top + 2*GetMBarHeight();
	rectPtr->bottom = qd.screenBits.bounds.bottom;
	rectPtr->left = qd.screenBits.bounds.left;
	rectPtr->right = qd.screenBits.bounds.right;
	
	InsetRect(rectPtr,2,2); 	/* leave a few pixels */
}

/**********************************************************************
 * force all windows to be redrawn
 **********************************************************************/
void RedrawAllWindows(void)
{
	WindowPeek theWindow;
	Rect r;
	
	/*
	 * invalidate all windows
	 */
	SetRect(&r,-INFINITY,-INFINITY,INFINITY,INFINITY);
	for (theWindow = FrontWindow();
		 theWindow != nil ;
		 theWindow = theWindow->nextWindow)
		RectRgn(theWindow->updateRgn,&r);
}

/**********************************************************************
 * SetDIText - set the text of a particular dialog item
 **********************************************************************/
void SetDIText(DialogPtr dialog,int item,UPtr text)
{
	short type;
	Handle itemH;
	Rect box;
	
	GetDItem(dialog,item,&type,&itemH,&box);
	if (itemH!=nil)
		SetIText(itemH,text);
}

/**********************************************************************
 * GetDIText - get the text of a particular dialog item
 **********************************************************************/
void GetDIText(DialogPtr dialog,int item,UPtr text)
{
	short type;
	Handle itemH;
	Rect box;
	
	GetDItem(dialog,item,&type,&itemH,&box);
	if (itemH!=nil)
		GetIText(itemH,text);
}

/************************************************************************
 * SetDIPopup - set a popup control to point to a string
 ************************************************************************/
UPtr SetDIPopup(DialogPtr pd,short item,UPtr toName)
{
	ControlHandle ch;
	short itemType;
	Rect itemRect;
	short which;

	GetDItem(pd,item,&itemType,&ch,&itemRect);
	which = FindItemByName((MenuHandle)*(long *)*(*ch)->contrlData,toName);
	SetCtlValue(ch,which);
	return(toName);
}

/************************************************************************
 * GetDIPopup - get the string a popup control is looking at
 ************************************************************************/
UPtr GetDIPopup(DialogPtr pd,short item,UPtr whatName)
{
	ControlHandle ch;
	short itemType;
	Rect itemRect;

	GetDItem(pd,item,&itemType,&ch,&itemRect);
	GetItem((MenuHandle)*(long *)*(*ch)->contrlData,GetCtlValue(ch),whatName);
	return(whatName);
}
	
/************************************************************************
 * SetDItemState - set the state of an item in a dialog
 ************************************************************************/
short SetDItemState(DialogPtr pd,short dItem,short on)
{
	ControlHandle ch;
	short itemType;
	Rect itemRect;

	GetDItem(pd,dItem,&itemType,&ch,&itemRect);
	SetCtlValue(ch,on);
	return(on);
}

/************************************************************************
 * GetDItemState - get the state of an item in a dialog
 ************************************************************************/
short GetDItemState(DialogPtr pd,short dItem)
{
	ControlHandle ch;
	short itemType;
	Rect itemRect;

	GetDItem(pd,dItem,&itemType,&ch,&itemRect);
	return(GetCtlValue(ch));
}

/************************************************************************
 * DlgFilter - filter for normal dialogs
 ************************************************************************/
pascal Boolean DlgFilter(DialogPtr dgPtr,EventRecord *event,short *item)
{
	Rect r;
	short type,itemN;
	Handle itemH;
	Point mouse;
	static Point oldMouse;
	GrafPtr oldPort;
	long select;
	extern ConnHandle CnH;
	Boolean oldCmdPeriod=CommandPeriod;
	
	if (event->what==mouseDown || event->what==keyDown)
		AlertTicks=0;
	if (CnH) CMIdle(CnH);
	if (!MiniMainLoop(event))
		if (!CommandPeriod)
			return(False);
		else
		{
			*item = CANCEL_ITEM;
			CommandPeriod = oldCmdPeriod;
			return(True);
		}
	if (event->what==keyDown || event->what==autoKey) SpecialKeys(event);
	if (event->what==keyDown || event->what==autoKey)
	{
		short key = event->message & charCodeMask;
		if (event->modifiers&cmdKey)
		{
			short oldMenu = TheMenu;
			MenuHandle mh = GetMHandle(EDIT_MENU);
			if (mh)
			{
				EnableItem(mh,0);
				if (select=MenuKey(key))
				{
					if (((select>>16)&0xffff)==EDIT_MENU)
						(void) DoModelessEdit(dgPtr,select&0xffff);
					if (oldMenu) HiliteMenu(oldMenu);
				}
				event->what=nullEvent;
				DisableItem(GetMHandle(EDIT_MENU),0);
			}
		}
		else
			switch (key)
			{
				case enterChar:
				case returnChar:
					*item = ((DialogPeek)dgPtr)->aDefItem ? ((DialogPeek)dgPtr)->aDefItem : 1;
					return(True);
					break;
			}
	}
	else if (event->what==activateEvt || event->what==updateEvt)
	{
		HiliteButtonOne(dgPtr);
	}
	else if (event->what==nullEvent)
	{
		if (AlertsTimeout && AlertTicks && AlertTicks<TickCount())
		{
			*item = ((DialogPeek)dgPtr)->aDefItem ? ((DialogPeek)dgPtr)->aDefItem : 1;
			AlertTicks = 0;
			return(True);
		}
		GetPort(&oldPort);
		SetPort(dgPtr);
		GetMouse(&mouse);
		if (mouse.h==oldMouse.h && mouse.v==oldMouse.v) return(False);
		oldMouse = mouse;
		for (itemN=1;;itemN++)
		{
			itemH = nil;
			GetDItem(dgPtr,itemN,&type,&itemH,&r);
			if (!itemH) break;
			if (PtInRect(mouse,&r))
			{
				SetTopCursor((type & editText) ? iBeamCursor : arrowCursor);
				break;
			}
		}
		SetPort(oldPort);
	}
	return(False);
}

/************************************************************************
 * CenterDialog - try to get a dialog centered on the screen
 ************************************************************************/
void CenterDialog(int template)
{
	DialogTHndl dTempl;
	Rect r = qd.screenBits.bounds;
#ifdef CAROLYN
	SetRect(&r,0,0,512,342);
#endif
	r.top += GetMBarHeight(); 			
	if ((dTempl=(DialogTHndl)GetResource('ALRT',template)) ||
			(dTempl=(DialogTHndl)GetResource('DLOG',template)))
		CenterRectIn(&(*dTempl)->boundsRect,&r);
}

/************************************************************************
 * TopCenterDialog - try to get a dialog centered on (the top of) the screen
 ************************************************************************/
void TopCenterDialog(int template)
{
	DialogTHndl dTempl;
	Rect r = qd.screenBits.bounds;
#ifdef CAROLYN
	SetRect(&r,0,0,512,342);
#endif
	r.top += GetMBarHeight(); 			
	if ((dTempl=(DialogTHndl)GetResource('ALRT',template)) ||
			(dTempl=(DialogTHndl)GetResource('DLOG',template)))
		TopCenterRectIn(&(*dTempl)->boundsRect,&r);
}

/************************************************************************
 * BottomCenterDialog - try to get a dialog centered on (the bottom of)
 * the screen
 ************************************************************************/
void BottomCenterDialog(int template)
{
	DialogTHndl dTempl;
	Rect r = qd.screenBits.bounds;
#ifdef CAROLYN
	SetRect(&r,0,0,512,342);
#endif
	r.top += GetMBarHeight(); 			
	if ((dTempl=(DialogTHndl)GetResource('ALRT',template)) ||
			(dTempl=(DialogTHndl)GetResource('DLOG',template)))
		BottomCenterRectIn(&(*dTempl)->boundsRect,&r);
}

/************************************************************************
 * ThirdCenterDialog - try to get a dialog centered on (the top 1/3 of)
 * the screen
 ************************************************************************/
void ThirdCenterDialog(int template)
{
	DialogTHndl dTempl;
	Rect r = qd.screenBits.bounds;
#ifdef CAROLYN
	SetRect(&r,0,0,512,342);
#endif
	r.top += GetMBarHeight(); 			
	if ((dTempl=(DialogTHndl)GetResource('ALRT',template)) ||
			(dTempl=(DialogTHndl)GetResource('DLOG',template)))
		ThirdCenterRectIn(&(*dTempl)->boundsRect,&r);
}

/************************************************************************
 * GlobalizeRgn - offset a region to global coords
 ************************************************************************/
void GlobalizeRgn(RgnHandle rgn)
{
	Point o;
	
	o.h = o.v = 0;
	LocalToGlobal(&o);
	OffsetRgn(rgn,o.h,o.v);
}

/************************************************************************
 * LocalizeRgn - offset a region to local coords
 ************************************************************************/
void LocalizeRgn(RgnHandle rgn)
{
	Point o;
	
	o.h = o.v = 0;
	GlobalToLocal(&o);
	OffsetRgn(rgn,o.h,o.v);
}

/************************************************************************
 * HiliteButtonOne - hilite the default button in a dialog
 ************************************************************************/
void HiliteButtonOne(DialogPtr dgPtr)
{
	GrafPtr oldPort;
	short type;
	Handle itemH;
	Rect r;
	
	GetDItem(dgPtr,((DialogPeek)dgPtr)->aDefItem ? ((DialogPeek)dgPtr)->aDefItem : 1,&type,&itemH,&r);
	if (type==btnCtrl+ctrlItem)
	{
		GetPort(&oldPort);
		SetPort(dgPtr);
		OutlineControl(itemH,!InBG&&FrontWindow()==dgPtr);
		SetPort(oldPort);
	}
}

/************************************************************************
 * PlotSICN, courtesy of Apple DTS
 ************************************************************************/
void PlotSICN(Rect *theRect, SICNHand theSICN, long theIndex) {
			 auto char	 state; 	/*saves original flags of 'SICN' handle*/
			 auto BitMap srcBits; /*built up around 'SICN' data so we can
_CopyBits*/

			 /* check the index for a valid value */
			 if ((GetHandleSize(theSICN) / sizeof(SICN)) > theIndex) {

					 /* store the resource's current locked/unlocked condition */
					 state = HGetState(theSICN);

					 /* lock the resource so it won't move during the _CopyBits call
*/
					 HLock(theSICN);

					 /* set up the small icon's bitmap */
					 srcBits.baseAddr = (Ptr) (*theSICN)[theIndex];
					 srcBits.rowBytes = 2;
					 SetRect(&srcBits.bounds, 0, 0, 16, 16);

					 /* draw the small icon in the current grafport */
					 CopyBits(&srcBits,&(*qd.thePort).portBits,&srcBits.bounds,
										theRect,srcOr,nil);

					 /* restore the resource's locked/unlocked condition */
					 HSetState(theSICN, state);
			 }
}

/************************************************************************
 *
 ************************************************************************/
void SavePosPrefs(UPtr name,Rect *r, Boolean zoomed)
{
	PositionHandle rez;
	
	if (!(rez=Get1NamedResource(SAVE_POS_TYPE,name)))
	{
		rez = NewH(PositionType);
		if (!rez) return;
		AddResource(rez,SAVE_POS_TYPE,Unique1ID(SAVE_POS_TYPE),name);
		if (ResError()) {DisposHandle(rez); return;}
	}
	(*rez)->r = *r;
	(*rez)->zoomed = zoomed;
	ChangedResource(rez);
}

/************************************************************************
 *
 ************************************************************************/
void SavePosFork(short vRef,long dirId,UPtr name,Rect *r, Boolean zoomed)
{
	short refN;
	Str31 scratch;
	HFileInfo info;
	long oldmdDat;
	short err;
	
	if (HGetFileInfo(vRef,dirId,name,&info)) return;
	oldmdDat = info.ioFlMdDat;
	
	refN=HOpenResFile(vRef,dirId,name,fsRdWrPerm);
	if (refN<0)
	{
		HCreateResFile(vRef,dirId,name);
		if (err=ResError()) return;
		refN=HOpenResFile(vRef,dirId,name,fsRdWrPerm);
		err=ResError();
	}
	if (refN>0)
	{
		SavePosPrefs(GetRString(scratch,POSITION_NAME),r,zoomed);
		if (refN != SettingsRefN)
		{
			CloseResFile(refN);
			if (!HGetFileInfo(vRef,dirId,name,&info))
			{
				info.ioFlMdDat = oldmdDat;
				HSetFileInfo(vRef,dirId,name,&info);
			}
		}
	}
}

/************************************************************************
 *
 ************************************************************************/
Boolean RestorePosPrefs(UPtr name,Rect *r, Boolean *zoomed)
{
	PositionHandle rez;
	
	if (rez=Get1NamedResource(SAVE_POS_TYPE,name))
	{
		*r = (*rez)->r;
		*zoomed = (*rez)->zoomed;
		return(True);
	}
	return(False);
}

/************************************************************************
 *
 ************************************************************************/
Boolean RestorePosFork(short vRef,long dirId,UPtr name,Rect *r, Boolean *zoomed)
{
	Str31 scratch;
	short refN;
	Boolean done;
	
	if ((refN=HOpenResFile(vRef,dirId,name,fsRdPerm))>0)
	{
		done = RestorePosPrefs(GetRString(scratch,POSITION_NAME),r,zoomed);
		if (refN != SettingsRefN) CloseResFile(refN);
		return(done);
	}
	return(False);
}

/************************************************************************
 *
 ************************************************************************/
Boolean PositionPrefsTitle(Boolean save,MyWindowPtr win)
{
	Rect r;
	Boolean zoomed;
	
	if (save)
	{
		utl_SaveWindowPos(win,&r,&zoomed);
		SavePosPrefs(LDRef(((WindowPeek)win)->titleHandle),&r,zoomed);
	}
	else
	{
		if (!RestorePosPrefs(LDRef(((WindowPeek)win)->titleHandle),&r,&zoomed))
			{UL(((WindowPeek)win)->titleHandle); return(False);}
		utl_RestoreWindowPos(win,&r,zoomed,1,FigureZoom,DefPosition);
	}
	UL(((WindowPeek)win)->titleHandle);
	return(True);
}

/************************************************************************
 *
 ************************************************************************/
void DefPosition(MyWindowPtr win,Rect *r)
{
	Point corner;
	utl_StaggerWindow(&((GrafPtr)win)->portRect,1,GetMBarHeight(),&corner,
										GetRLong(PREF_STRN+PREF_NW_DEV));
	*r = ((GrafPtr)win)->portRect;
	OffsetRect(r,corner.h-r->left,corner.v-r->top);
}

/************************************************************************
 * GreyOutRoundRect - grey a rectangle
 ************************************************************************/
void GreyOutRoundRect(Rect *r,short r1, short r2)
{
	PenState oldState;
	
	GetPenState(&oldState);
	PenMode(patBic);
	PenPat(&qd.gray);
	PaintRoundRect(r,r1,r2);
	SetPenState(&oldState);
}

/************************************************************************
 * DrawMyControls - draw a window's controls
 ************************************************************************/
void DrawMyControls(WindowPeek win)
{
	ControlHandle cntl;
	GrafPtr oldPort;
	
	GetPort(&oldPort);
	SetPort(win);
	DrawControls(win);
	for (cntl=win->controlList;cntl;cntl=(*cntl)->nextControl)
		if ((*cntl)->contrlRfCon=='GREY') GreyControl(cntl);
	SetPort(oldPort);
}

/************************************************************************
 * SanitizeSize - make sure a rect is small enough to fit onscreen
 ************************************************************************/
void SanitizeSize(Rect *r)
{
	Rect gray = (**(RgnHandle *)GrayRgn)->rgnBBox;
	short maxWi = gray.right-gray.left-4;
	short maxHi = gray.bottom-gray.top - GetMBarHeight() - 2;
	
	if (r->right-r->left>maxWi) r->right = r->left+maxWi;
	if (r->bottom-r->top>maxHi) r->bottom = r->top+maxHi;
}


/************************************************************************
 * MyWinHasSelection - is there a selection in one of the te's?
 ************************************************************************/
Boolean MyWinHasSelection(MyWindowPtr win)
{
	TEHandle teh;
	
	if (win->txe) teh = win->txe;
	else if (win->ste) teh = (*(STEHandle)win->ste)->te;
	else return(False);
	
	return((*teh)->selStart != (*teh)->selEnd);
}
/************************************************************************
 * HotRect - SFPutFile-style frame
 ************************************************************************/
void HotRect(Rect *r,Boolean on)
{
	Rect fr = *r;

	PenState oldPen;
	InsetRect(&fr,-3,-3);
	GetPenState(&oldPen);
	PenPat(on ? &qd.black : &qd.white);
	PenSize(2,2);
	FrameRect(&fr);
	SetPenState(&oldPen);
}

/************************************************************************
 * CursorInRect - see if the mouse is in a rectangle.  If so, set mouseRgn
 *  to the rectangle, if not subtract rect from mouseRgn.
 *	Note that the parameter is a Rect, not a Rect *; this avoids a temp var
 *  for rects in handles.
 ************************************************************************/
Boolean CursorInRect(Point pt,Rect r,RgnHandle mouseRgn)
{
	RgnHandle punchRgn;
		
	/*
	 * if we're in the rect...
	 */
	if (PtInRect(pt,&r))
	{
		RectRgn(mouseRgn,&r);
		return(True);
	}
	/*
	 * otherwise, put a hole in the region
	 */
	else if (punchRgn=NewRgn())
	{
		RectRgn(punchRgn,&r);
		DiffRgn(mouseRgn,punchRgn,mouseRgn);
		DisposeRgn(punchRgn);
	}
	return(False);
}

/************************************************************************
 * MaxSizeZoom - size the zoom rect of a window, using the maxes provided
 ************************************************************************/
void MaxSizeZoom(MyWindowPtr win,Rect *zoom)
{
	short zoomHi = zoom->bottom-zoom->top;
	short zoomWi = zoom->right-zoom->left;
	short hi, wi;
	
	if (win->hMax<0) UpdateMyWindow(win);
	hi = win->vMax ?
		win->vMax*win->vPitch+win->topMargin+(win->hBar ? GROW_SIZE : 0) :
		(win->vBar ? win->minSize.v : zoomHi);
	wi = win->hMax ?
		win->hMax*win->hPitch+(win->vBar ? GROW_SIZE : 0) :
		(win->hBar ? win->minSize.h : zoomWi);

	wi = MIN(wi,zoomWi); wi = MAX(wi,win->minSize.h);
	hi = MIN(hi,zoomHi); hi = MAX(hi,win->minSize.v);
	zoom->right = zoom->left+wi;
	zoom->bottom = zoom->top+hi;
}

/************************************************************************
 * CurState - return a windows current state
 ************************************************************************/
Rect CurState(MyWindowPtr win)
{
	Point pt;
	Rect curState;
	
	pt.h = pt.v = 0;
	LocalToGlobal(&pt);
	curState = ((GrafPtr)win)->portRect;
	OffsetRect(&curState, pt.h, pt.v);
	return(curState);
}

/************************************************************************
 * AboutSameRect - are two rects "about equal"
 ************************************************************************/
Boolean AboutSameRect(Rect *r1,Rect *r2)
{
	short *s1 = r1;
	short *s2 = r2;
	short i=4;
	
	for (;i--;s1++,s2++) if (ABS(*s1-*s2)>7) return(False);
	return(True);
}

/************************************************************************
 * OutlineControl - outline the default button
 ************************************************************************/
void OutlineControl(ControlHandle cntl,Boolean blackOrWhite)
{
	Rect r =  (*cntl)->contrlRect;
	PenState oldState;
	
	GetPenState(&oldState);
	PenSize(3,3);
	PenPat(blackOrWhite?((*cntl)->contrlRfCon=='GREY'?&qd.gray:&qd.black):&qd.white);
	InsetRect(&r,-4,-4);
	FrameRoundRect(&r,16,16);
	SetPenState(&oldState);
}

#ifdef DEBUG
/************************************************************************
 * ShowGlobalRgn - show a region in global coords
 ************************************************************************/
void ShowGlobalRgn(RgnHandle globalRgn,UPtr string)
{
	GrafPtr wmgr;
	SAVE_PORT;
	GetWMgrPort(&wmgr);
	SetPort(wmgr);
	InvertRgn(globalRgn);
	if (EmptyRgn(globalRgn)) DebugStr("\pempty!;g");
	DebugStr(string);
	InvertRgn(globalRgn);
	REST_PORT;
}


/************************************************************************
 * 
 ************************************************************************/
void ShowLocalRgn(RgnHandle localRgn,UPtr string)
{
	GlobalizeRgn(localRgn);
	ShowGlobalRgn(localRgn,string);
	LocalizeRgn(localRgn);
}
#endif