#define FILE_NUM 5
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Boxes
	void BoxTrack(TOCType **tocH,int anchor,Boolean shift);
	void BoxCenter(MyWindowPtr win, short mNum);
	void DoSaveSelectedAs(TOCHandle tocH);
	void ShowBoxSizes(MyWindowPtr win);
	void BoxSetPriorities(TOCHandle tocH,Byte priority);
	Boolean BoxDragDividers(Point pt, MyWindowPtr win);
	Boolean BoxFindDivider(Point pt,short *which);
#pragma segment Balloon
/************************************************************************
 * BoxHelp - help for mailboxes
 ************************************************************************/
void BoxHelp(MyWindowPtr win, Point mouse)
{
	short mNum;
	TOCHandle tocH = (TOCHandle) win->qWindow.refCon;
	Rect r;
	short hnum=0;
	short column;

	mNum = mouse.v/win->vPitch + GetCtlValue(win->vBar);
	r = win->contR;
	if (PtInRect(mouse,&win->contR))
	{
		if (BoxFindDivider(mouse,&column))
		{
			r.left = (*BoxLines)[column]-1; r.right = r.left+3;
			HelpRect(&r,MBOX_HELP_STRN+column+1,90);
		}
		else
		{
			if (column) r.left = (*BoxLines)[column-1]+1;
			if (column<WID_LIMIT) r.right = (*BoxLines)[column]-1;
			if (column)
				HelpRect(&r,MBOX_HELP_STRN+WID_LIMIT+column,90);
			else
				HelpPict(&r,STATUS_HELP_PICT,90);
		}
	}
	else if (mouse.v>win->contR.bottom && GetRLong(BOX_SIZE_SIZE)>mouse.h)
	{
		SetRect(&r,win->contR.left,win->contR.bottom,
							 GetRLong(BOX_SIZE_SIZE),win->contR.bottom+GROW_SIZE);
		HelpRect(&r,MBOX_HELP_STRN+2*WID_LIMIT+1,90);
	}
	
}
#pragma segment Boxes

/**********************************************************************
 * BoxUpdate - handle an update event for a mailbox window
 **********************************************************************/
void BoxUpdate(MyWindowPtr win)
{
	WindowPeek qw = (WindowPeek) win;
	TOCType **tocH = (TOCType **) qw->refCon;
	MSumType *sum;
	int min, max;
	int line;
	int h,v;
	Str31 states,scratch;
	Rect r,pr,cr;
	short h0;
	short greyMax;
	
	/*
	 * redraw it all if we need to calculate the window width
	 */
	if (win->hMax < 0)
	{
		(*tocH)->needRedo = 0;
		return;
	}
	
	/*
	 * get ready
	 */
	ClipRect(&win->contR);

	LDRef(tocH);
	min = GetCtlValue(win->vBar);
	max = min+RoundDiv(win->contR.bottom-win->contR.top,win->vPitch)+1;
	max = MIN((*tocH)->count-1,max);
	h = h0 = -GetCtlValue(win->hBar)*win->hPitch;
	
	/*
	 * draw the grey lines
	 */
	if (!PrefIsSet(PREF_NO_LINES))
	{
		PenPat(&qd.gray);
		greyMax = (max-min+1)*(short)win->vPitch;
		for (v=0;v<=greyMax;v+=win->vPitch)
		{
			MoveTo(0,v);
			LineTo(INFINITY,v);
		}
#ifdef NEVER
		MoveTo(3*FontWidth/2,0);
		Line(0,INFINITY);
#endif
		for (line=0;line<WID_LIMIT;line++)
		{
			h = h0 + (*BoxLines)[line];
			MoveTo(h,0);
			LineTo(h,INFINITY);
		}
	}
		
	/*
	 * now, draw the contents
	 */
	sum=(*tocH)->sums+min;
	if (max>(*tocH)->count-1) max = (*tocH)->count-1;
	PenPat(&qd.black);
	GetRString(states,STATE_LABELS);
	v = (1-(GetCtlValue(win->vBar)-min))*win->vPitch - FontDescent;
	r.left = 0;
	r.right = INFINITY;
	pr.left = (3*FontWidth)/2;
	pr.top = (win->vPitch-16)/2;
	pr.bottom = pr.top+16;
	pr.right = pr.left+16;
	OffsetRect(&pr,0,(min-GetCtlValue(win->vBar))*win->vPitch);
	for (line=0; line<=(max-min); line++,sum++,v+=win->vPitch,OffsetRect(&pr,0,win->vPitch))
	{
		cr.bottom = MIN(v+FontDescent+1,win->contR.bottom);
		cr.top = MAX(win->contR.top,cr.bottom-win->vPitch);
		h = h0;
		cr.left = h+1; cr.right = h0+(*BoxLines)[WID_STAT]-1;
		ClipRect(&cr);
		MoveTo(h+FontWidth/2,v);
		DrawChar(states[sum->state]);
		DrawPriority(&pr,sum->priority);
		h = h0 + (*BoxLines)[WID_STAT];
				
		cr.left = h+1; cr.right = h0+(*BoxLines)[WID_FROM]-1;
		cr.right = MIN(cr.right,win->contR.right);
		if (cr.right-cr.left+2>=FontWidth)
		{
			ClipRect(&cr);
			MoveTo(h+FontWidth/2,v);
			DrawTruncString(sum->from,sum->fromTrunc);
		}
		h = h0 + (*BoxLines)[WID_FROM];
				
		cr.left = h+1; cr.right = h0+(*BoxLines)[WID_DATE]-1;
		cr.right = MIN(cr.right,win->contR.right);
		if (cr.right-cr.left+2>=FontWidth)
		{
			ClipRect(&cr);
			MoveTo(h+FontWidth/2,v);
			DrawTruncString(sum->date,sum->dateTrunc);
		}
		h = h0 + (*BoxLines)[WID_DATE];
		
		cr.left = h+1; cr.right = h0+(*BoxLines)[WID_SIZE]-1;
		cr.right = MIN(cr.right,win->contR.right);
		if (cr.right-cr.left+2>=FontWidth)
		{
		 	ClipRect(&cr);
			NumToString((sum->length+1023)/1024,scratch);
			MoveTo(cr.right-FontWidth/4-StringWidth(scratch),v);
			DrawString(scratch);
		}
		h = h0 + (*BoxLines)[WID_SIZE];
		
	  ClipRect(&win->contR);
		cr.left = h+1; cr.right = win->contR.right-1;
		if (cr.right-cr.left+2>=FontWidth)
		{
			MoveTo(h+FontWidth/2,v);
			DrawString(sum->subj);
		}
		
		if (sum->selected && win->isActive)
		{
			r.left = 0;
			r.right = INFINITY;
			r.bottom = v + FontDescent;
			r.top = r.bottom - win->vPitch;
			BitClr((Ptr)HiliteMode, pHiliteBit);
			InvertRect(&r);
		}
	}
	
	/*
	 * cleanup
	 */
	UL(tocH);
	InfiniteClip(win);
	ShowBoxSizes(win);
}


/**********************************************************************
 * BoxClick - handle a click in the content region of a window
 **********************************************************************/
void BoxClick(MyWindowPtr win,EventRecord *event)
{
	Point pt;
	short startNum,endNum,mNum;
	TOCType **tocH;
	Boolean cmd,shift;
	
	pt = event->where;
	GlobalToLocal(&pt);
	if (!PtInRect(pt,&win->contR)) return;
	if (BoxDragDividers(pt,win)) return;
	shift = (event->modifiers & shiftKey) != 0;
	cmd = (event->modifiers & cmdKey) != 0;
	mNum = pt.v/win->vPitch + GetCtlValue(win->vBar);
	tocH = (TOCType **)((WindowPeek)win)->refCon;
	if (shift)
	{
		for (startNum=0;startNum<(*tocH)->count;startNum++)
			if ((*tocH)->sums[startNum].selected) break;
		if (startNum < (*tocH)->count)
		{
		  if (startNum < mNum) endNum = mNum;
			else
			{
				for (endNum=startNum+1;endNum<(*tocH)->count;endNum++)
					if (!(*tocH)->sums[endNum].selected) break;
				endNum--;
				if (!(*tocH)->sums[endNum].selected) endNum = startNum;
				startNum = mNum;
			}
		}
		else startNum=endNum=mNum;
	}
	else startNum=endNum=mNum;

	SelectBoxRange(tocH,startNum,endNum,cmd,-1,-1);
	BoxTrack(tocH,mNum==startNum?endNum:startNum,cmd&&!shift);
	EnableMenus(win);
	if (ClickType==Double)
		DoDependentMenu(win,FILE_MENU,FILE_OPEN_ITEM,event->modifiers);
}

 /************************************************************************
 * BoxTrack - track the mouse in a mailbox window
 ************************************************************************/
void BoxTrack(TOCType **tocH,int anchor,Boolean cmd)
{
	MyWindowPtr win = (*tocH)->win;
	Point pt;
	int lastSpot=anchor;
	int spot;
	int vVal,vMin,vMax,topVis,botVis;
	long lastTicks=0;
	
	while (StillDown())
		if (TickCount()-lastTicks>3)
		{
			lastTicks = TickCount();
			vVal = GetCtlValue(win->vBar);
			vMin = GetCtlMin(win->vBar);
			vMax = GetCtlMax(win->vBar);
			topVis = vVal;
			botVis = (*tocH)->count - (vMax-vVal+1);
			GetMouse(&pt);
			spot = vVal + pt.v/(int)win->vPitch;
			if (spot<vMin) spot=vMin;
			else if (spot>=(*tocH)->count) spot=(*tocH)->count-1;
			if (spot<topVis &&	vVal>vMin || spot>botVis && vVal<vMax)
			{
				if (spot<topVis)
					ScrollIt(win,0,topVis-spot);
				else
					ScrollIt(win,0,botVis-spot);
			}
			if (spot!=lastSpot)
			{
				if (!cmd)
					SelectBoxRange(tocH,anchor,spot,cmd,0,0);
				else
				{
					SelectBoxRange(tocH,anchor,spot,cmd,anchor,lastSpot);
					SelectBoxRange(tocH,anchor,lastSpot,cmd,anchor,spot);
				}
				lastSpot = spot;
			}
		}
}

/************************************************************************
 * SelectBoxRange - make a particular range in a mailbox the current selection.
 ************************************************************************/
void SelectBoxRange(TOCType **tocH,int start,int end,Boolean cmd,int eStart,int eEnd)
{
	MyWindowPtr win = (*tocH)->win;
	/*
	 * topVis and botVis are overly generous, since it won't hurt to be so
	 */
	int topVis = GetCtlValue(win->vBar);
	int botVis = (*tocH)->count-(GetCtlMax(win->vBar)-GetCtlValue(win->vBar));
	int sNum;
	int r1, r2;
	GrafPtr oldPort;
	
	if (!(*tocH)->count) return;
	GetPort(&oldPort); SetPort(win);
	if (end<start) {r1=start;start=end;end=r1;} 	/* swap */
	if (eEnd<eStart) {r1=eStart;eStart=eEnd;eEnd=r1;} 		/* swap */
	
	r1 = start < (*tocH)->count ? start : (*tocH)->count;
	r2 = end < (*tocH)->count ? end : (*tocH)->count-1;
	
	win->hasSelection = False;
	
	/*
	 * first range; from the beginning to just before the new selection range
	 */
	if (cmd)
	{
		for (sNum=0;sNum<r1;sNum++)
			if ((*tocH)->sums[sNum].selected)
			{
				win->hasSelection = True;
				break;
			}
	}
	else
	{
		for (sNum=0;sNum<r1;sNum++)
			if ((*tocH)->sums[sNum].selected)
			{
				(*tocH)->sums[sNum].selected = False;
				if (topVis <= sNum && sNum <= botVis && win->isActive)
					InvertLine(win,sNum);
			}
	}
	
	/*
	 * the new selection range
	 */
	if (cmd)
	{
		for (sNum=r1;sNum<=r2;sNum++)
			if (sNum<eStart || sNum > eEnd)
			{
				(*tocH)->sums[sNum].selected = !(*tocH)->sums[sNum].selected;
				if (topVis <= sNum && sNum <= botVis && win->isActive)
					InvertLine(win,sNum);
				win->hasSelection = win->hasSelection || (*tocH)->sums[sNum].selected;
			}
	}
	else
	{
		for (sNum=r1;sNum<=r2;sNum++)
		{
			if (!(*tocH)->sums[sNum].selected)
			{
				(*tocH)->sums[sNum].selected = True;
				if (topVis <= sNum && sNum <= botVis && win->isActive)
					InvertLine(win,sNum);
			}
		}
		win->hasSelection = r1<=r2;
	}
	
	/*
	 * above the new selection range
	 */
	if (cmd)
	{
		if (!win->hasSelection)
			for (sNum=r2+1;sNum<(*tocH)->count;sNum++)
				if ((*tocH)->sums[sNum].selected)
				{
					win->hasSelection = True;
					break;
				}
	}
	else
	{
		for (sNum=r2+1;sNum<(*tocH)->count;sNum++)
			if ((*tocH)->sums[sNum].selected)
			{
				(*tocH)->sums[sNum].selected = False;
				if (topVis <= sNum && sNum <= botVis && win->isActive)
					InvertLine(win,sNum);
			}
	}
	
	SetPort(oldPort);
}

/**********************************************************************
 * BoxActivate - perform extra mailbox processing for activate/deactivate
 **********************************************************************/
void BoxActivate(MyWindowPtr win)
{
	TOCType **tocH;
	int topVis;
	int botVis;
	int sNum;
	Rect r;
	MSumType *sum;
	
	SetPort(win);
	if (win->hasSelection)
	{
		ClipRect(&win->contR);
		tocH = (TOCType **)((WindowPeek)win)->refCon;
		topVis = GetCtlValue(win->vBar);
		botVis = (*tocH)->count -
									(GetCtlMax(win->vBar)-GetCtlValue(win->vBar));
		if (botVis==(*tocH)->count) botVis--;
		LDRef(tocH);
		sum = (*tocH)->sums+topVis;
		for (sNum=topVis; sNum<=botVis; sNum++,sum++)
			if (sum->selected) InvertLine(win,sNum);
		UL(tocH);
		SetRect(&r,0,0,INFINITY,INFINITY);
		ClipRect(&r);
	}
	if (win->isActive) EnableMenus(win);
}

/**********************************************************************
 * BoxMenu - handle a menu choice for a mailbox
 **********************************************************************/
Boolean BoxMenu(MyWindowPtr win,int menu,int item,short modifiers)
{
	Str63 scratch, which;
	TOCType **tocH = (TOCType **)((WindowPeek)win)->refCon;
	long dirId;
	short function;
	short state;
	uLong when;
	MSumPtr sum;
	
	if (menu==MESSAGE_MENU && item==MESSAGE_QUEUE_ITEM && modifiers&optionKey)
	  item = MESSAGE_MOD_Q_ITEM;
	switch (menu)
	{
		case FILE_MENU:
			switch(item)
			{
				case FILE_OPEN_ITEM:
					if (win->hasSelection)
						BoxOpen(win);
					else
						return(False);
					break;
				case FILE_SAVE_AS_ITEM:
					DoSaveSelectedAs(tocH);
					return(True);
					break;
				case FILE_PRINT_ITEM:
				case FILE_PRINT_SELECT_ITEM:
					if (win->hasSelection)
						(void) PrintSelectedMessages(tocH,item==FILE_PRINT_SELECT_ITEM);
					else
						return(False);
					break;
				default:
					return(False);
			}
			break;
		
		case EDIT_MENU:
			switch(item)
			{
				case EDIT_CLEAR_ITEM:
					DoIterativeThingy(tocH,MESSAGE_DELETE_ITEM,modifiers,0);
					break;
				case EDIT_SELECT_ITEM:
					SelectBoxRange(tocH,0,(*tocH)->count-1,False,0,0);
					break;
				default:
					return(False);
					break;
			}
			break;
			
		case SORT_HIER_MENU:
			SetMyCursor(watchCursor);
			switch(item)
			{
				case SORT_STATUS_ITEM:
					SortTOC(tocH,(modifiers&optionKey)?RevSumStatCompare:SumStatCompare);
					break;
				case SORT_PRIORITY_ITEM:
					SortTOC(tocH,(modifiers&optionKey)?RevSumPriorCompare:SumPriorCompare);
					break;
				case SORT_SUBJECT_ITEM:
					SortTOC(tocH,(modifiers&optionKey)?RevSumSubjCompare:SumSubjCompare);
					break;
				case SORT_SENDER_ITEM:
					SortTOC(tocH,(modifiers&optionKey)?RevSumFromCompare:SumFromCompare);
					break;
				case SORT_TIME_ITEM:
					SortTOC(tocH,(modifiers&optionKey)?RevSumTimeCompare:SumTimeCompare);
					break;
			}
			break;
			
		case MESSAGE_MENU:
			switch(item)
			{
				case MESSAGE_DELETE_ITEM:
				case MESSAGE_FORWARD_ITEM:
				case MESSAGE_REPLY_ITEM:
				case MESSAGE_REDISTRIBUTE_ITEM:
				case MESSAGE_SALVAGE_ITEM:
					DoIterativeThingy(tocH,item,modifiers,0);
					break;
				case MESSAGE_QUEUE_ITEM:
					QueueSelectedMessages(tocH,QUEUED,0);
					if (SendQueue && PrefIsSet(PREF_AUTO_SEND))
						DoSendQueue();
					break;
				case MESSAGE_MOD_Q_ITEM:
				  when = 0;
					for (sum=(*tocH)->sums;sum<(*tocH)->sums+(*tocH)->count;sum++)
						if (sum->state==QUEUED) {when = sum->seconds;break;}
					if (ModifyQueue(&state,&when))
					{
					  Boolean now = state==SENT;
						if (now) state = QUEUED;
					  QueueSelectedMessages(tocH,state,when);
						if (SendQueue && now) DoSendQueue();
					}
					break;
				default:
					return(False);
			}
			break;

		case SPECIAL_MENU:
			switch(item)
			{
				case SPECIAL_MAKE_NICK_ITEM:
					if ((*tocH)->which==OUT) MakeCboxNick(win);
					else MakeMboxNick(win,modifiers);
					break;
				default:
					return(False);
			}
			break;
			
		case REPLY_TO_HIER_MENU:
			DoIterativeThingy(tocH,MESSAGE_REPLY_ITEM,modifiers,item);
			break;
		case FORWARD_TO_HIER_MENU:
			DoIterativeThingy(tocH,MESSAGE_FORWARD_ITEM,modifiers,item);
			break;
		case REDIST_TO_HIER_MENU:
			DoIterativeThingy(tocH,MESSAGE_REDISTRIBUTE_ITEM,modifiers,item);
			break;
		case PRIOR_MENU:
			BoxSetPriorities(tocH,Display2Prior(item));
			break;
		default:
			if (menu==TRANSFER_MENU)
			{
				dirId = MyDirId;
				function = TRANSFER;
			}
			else if (menu<1||menu>=FIND_HIER_MENU) return(False);
			else
			{
				dirId = (*BoxMap)[menu % MAX_BOX_LEVELS];
				function = (menu-1)/MAX_BOX_LEVELS;
			}
			switch (function)
			{
				case TRANSFER:
					PCopy(scratch,(*tocH)->name);
					if (GetTransferParams(menu,item,&dirId,which) &&
							(dirId!=(*tocH)->dirId || !EqualString(which,scratch,False,True)))
						MoveSelectedMessages(tocH,dirId,which,(modifiers&optionKey)!=0);
					break;
				default:
					return(False);
					break;
			}
			break;
	}
	return(True);
}

/**********************************************************************
 * BoxClose - close a mailbox window, saving the toc if it's dirty
 **********************************************************************/
Boolean BoxClose(MyWindowPtr win)
{
	TOCType **tocH = (TOCType **)((WindowPeek)win)->refCon;
	short sumNum;
	Boolean justHide = False;
	Boolean superClose = PrefIsSet(PREF_SUPERCLOSE) && win->qWindow.visible;
	Boolean squish = NeedAutoCompact(tocH);
	long dirId = (*tocH)->dirId;
	Str31 name;
	
	PCopy(name,(*tocH)->name);
	
	/*
	 * first, close all associated message windows
	 */
	for (sumNum=(*tocH)->count-1;sumNum>=0;sumNum--)
	{
		if ((*tocH)->sums[sumNum].messH)
		{
			if (superClose)
			{
				if (!CloseMyWindow((*(MessType **)(*tocH)->sums[sumNum].messH)->win))
					return(False);
			}
			else
				justHide = True;
		}
	}
	
	/*
	 * write if we're dirty
	 */
	if ((*tocH)->dirty) WriteTOC(tocH);
	BoxFClose(tocH);

	if (justHide)
	{
		HideWindow(win);
		return(False);
	}

	/*
	 * now, unlink ourselves from the list
	 */
	LL_Remove(TOCList,tocH,(TOCType **));
	
	/*
	 * bye, bye love
	 */
	DisposHandle(tocH);
	
	/*
	 * compact?
	 */
	if (squish) CompactMailbox(dirId,name);
	
	return(True); 
}

/**********************************************************************
 * BoxOpen - open some messages from within a mailbox
 **********************************************************************/
void BoxOpen(MyWindowPtr win)
{
	MyWindowPtr **wps, *wp, lastWin=nil, w;
	int count;
	TOCType **tocH = (TOCType **)((WindowPeek)win)->refCon;
	int sum,limit;
	int opened;
	
	/*
	 * count the ones we need to open
	 */
	count = 0;
	for (limit=(*tocH)->count,sum=0; sum<limit; sum++)
	{
		if ((*tocH)->sums[sum].selected)
			if ((*tocH)->sums[sum].messH)
			{
				WindowPeek winPeek = (*(MessType **)(*tocH)->sums[sum].messH)->win;
				if (!winPeek->visible)
					ShowMyWindow(winPeek);
				SelectWindow(winPeek);
			}
			else
				count++;
	}
	
	/*
	 * create window records for them.	Doing them in one block
	 * now, in this segment, will avoid heap fragmentation.  We'll
	 * dispose of them later
	 */
	if (!count) return;
	wps = NuHandle(count*sizeof(MyWindowPtr));
	if (wps==nil)
	{
		WarnUser(COULDNT_WIN,MemError());
		return;
	}
	else
	{
		opened = count;
		while (opened--)
		{
			wp = NuPtr(sizeof(MyWindow));
			(*wps)[opened] = wp;
		}
		
		opened = 0;
		for (sum=limit-1;!EjectBuckaroo && sum>=0; sum--)
		{
			if ((*tocH)->sums[sum].selected && !(*tocH)->sums[sum].messH)
			{
				if (!(*wps)[opened])
				{
					WarnUser(COULDNT_WIN,0);
					break;
				}
				w = (*wps)[opened];
				(*wps)[opened++]=nil;
				if (!(w=GetAMessage(tocH,sum,w,True))) break;
				NotUsingWindow(w);
				if (lastWin) ActivateMyWindow(lastWin,False);
				lastWin = w;
			}
			MonitorGrow();
		}
		for (opened=0;opened<count; opened++)
			if ((*wps)[opened]) DisposPtr((*wps)[opened]);
		DisposHandle(wps);
	}
}

/************************************************************************
 * BoxKey - handle keystrokes in a mailbox window
 ************************************************************************/
void BoxKey(MyWindowPtr win, EventRecord *event)
{
	TOCType **tocH = (TOCType **)((WindowPeek)win)->refCon;
	short c = event->message & charCodeMask;
	Boolean cmd = (event->modifiers & (cmdKey|shiftKey)) != 0;
	int mNum = -1;
	long uLetter = UnadornMessage(event)&charCodeMask;
	
	if (leftArrowChar <= uLetter && uLetter <= downArrowChar &&
			win->hasSelection &&
			((event->modifiers&cmdKey)!=0 ?
				!PrefIsSet(PREF_NO_CMD_ARROW):PrefIsSet(PREF_PLAIN_ARROW)))
		c = enterChar;
		
	switch (c)
	{
		case leftArrowChar:
		case upArrowChar:
			for (mNum=0;mNum<(*tocH)->count;mNum++)
			 if ((*tocH)->sums[mNum].selected) break;
			mNum--;
			if (mNum<0) mNum = 0;
			break;
		case rightArrowChar:
		case downArrowChar:
			for (mNum=(*tocH)->count-1;mNum>=0;mNum--)
				if ((*tocH)->sums[mNum].selected) break;
			mNum++;
			if (mNum==(*tocH)->count) mNum--;
			break;
		case deleteKey:
			BoxMenu(win,EDIT_MENU,EDIT_CLEAR_ITEM,event->modifiers);
			break;
		case returnChar:
		case enterChar:
			if (win->hasSelection)
			{
				BoxOpen(win);
				break;
			}
			/* fall through is deliberate for no selection */
		default:
			if (event->modifiers & cmdKey)
			{
				if ('1'<=c && c<= '5') BoxSetPriorities(tocH,Display2Prior(c-'0'));
				else SysBeep(20L);
			}
			else
			  AlertStr(NOT_HOME_ALRT, Stop, nil);
			break;
	}
	if (mNum > -1)
	{
		SelectBoxRange(tocH,mNum,mNum,cmd,-1,-1);
		BoxCenter(win,mNum);
	}
}

/************************************************************************
 * BoxCenter - center a mailbox around a given line
 ************************************************************************/
void BoxCenter(MyWindowPtr win, short mNum)
{
	TOCType **tocH = (TOCType **)((WindowPeek)win)->refCon;
	int topVis = GetCtlValue(win->vBar);
	int botVis = (*tocH)->count-1-(GetCtlMax(win->vBar)-GetCtlValue(win->vBar));
	
	if (mNum<topVis || mNum>botVis)
	{
		UpdateMyWindow(win);	/* this is a hack.  ScrollIt should be smarter */
		ScrollIt(win,0,(topVis+botVis)/2-mNum);
	}
}

/************************************************************************
 * BoxSelectAfter - select the message on or after a given message,
 * if no messages are already selected
 ************************************************************************/
void BoxSelectAfter(MyWindowPtr win, short mNum)
{
	TOCType **tocH = (TOCType **)((WindowPeek)win)->refCon;
	
	if (mNum >=0 && BoxNextSelected(tocH,-1)<0)
	if (win->hasSelection = (*tocH)->count)
	{
		mNum = MIN(mNum,(*tocH)->count-1);
		(*tocH)->sums[mNum].selected = True;
		BoxCenter((*tocH)->win,mNum);
	}
}

/************************************************************************
 * BoxCenterSelection - center a selection in a mailbox window
 ************************************************************************/
void BoxCenterSelection(MyWindowPtr win)
{
	TOCType **tocH = (TOCType **)((WindowPeek)win)->refCon;
	int top, bottom;
	
	for (top=0;top<(*tocH)->count;top++)
		if ((*tocH)->sums[top].selected) break;
	for (bottom=(*tocH)->count-1;bottom>=0;bottom--)
		if ((*tocH)->sums[bottom].selected) break;
	if (top<=bottom) BoxCenter(win,(top+bottom)/2);
}

/************************************************************************
 *
 ************************************************************************/
Boolean BoxPosition(Boolean save,MyWindowPtr win)
{
	Rect r;
	Boolean zoomed;
	TOCHandle tocH = (TOCHandle) win->qWindow.refCon;
	Str31 name;
	PCopy(name,(*tocH)->name);
	
	if (save)
	{
		utl_SaveWindowPos(win,&r,&zoomed);
		SavePosFork((*tocH)->vRef,(*tocH)->dirId,name,&r,zoomed);
	}
	else
	{
		if (!RestorePosFork((*tocH)->vRef,(*tocH)->dirId,name,&r,&zoomed))
			{UL(tocH); return(False);}
		utl_RestoreWindowPos(win,&r,zoomed,1,FigureZoom,DefPosition);
	}
	return(True);
}

/************************************************************************
 * SaveMessageAs - save a message in text only form
 ************************************************************************/
void DoSaveSelectedAs(TOCHandle tocH)
{
	long creator;
	short vRefN, refN;
	short err;
	Str31 scratch,name;
	int sumNum;
	MessType **messH;
	short lastSelected = -1;
	Str255 title;
	
	/*
	 * tickle stdfile
	 */
	GetPref(scratch,PREF_CREATOR);
	if (*scratch!=4) GetRString(scratch,TEXT_CREATOR);
	BlockMove(scratch+1,&creator,4);
	for (sumNum=0;sumNum<(*tocH)->count;sumNum++)
		if ((*tocH)->sums[sumNum].selected) break;
	MakeMessFileName(tocH,sumNum,name);
	ExcludeHeaders = PrefIsSet(PREF_EXCLUDE_HEADERS);
	Paragraphs = PrefIsSet(PREF_PARAGRAPHS);
	if (err=SFPutOpen(name,&vRefN,creator,'TEXT',&refN,SaveAsFilter,SAVEAS_DLOG))
		return;
	
	OpenProgress();
	
	for (;!err && sumNum<(*tocH)->count;sumNum++)
		if ((*tocH)->sums[sumNum].selected)
		{
			MakeMessTitle(title,tocH,sumNum);
			Progress(NoChange,title);
			messH = (*tocH)->sums[sumNum].messH;
			if (messH || GetAMessage(tocH,sumNum,nil,False))
			{
				err = SaveAsToOpenFile(refN,(*tocH)->sums[sumNum].messH);
				if (!messH)
					CloseMyWindow((*(MessHandle)(*tocH)->sums[sumNum].messH)->win);
			}
		}

	CloseProgress();

	/*
	 * close
	 */
	(void) FSClose(refN);
	
	/*
	 * report error
	 */
	if (err)
	{
		FileSystemError(COULDNT_SAVEAS,name,err);
		FSDelete(name,vRefN);
	}
}

/************************************************************************
 * MakeMessFileName - make a default name for save as
 ************************************************************************/
void MakeMessFileName(TOCHandle tocH,short sumNum, UPtr name)
{
	UPtr spot;
	short len = MIN(31,*(*tocH)->sums[sumNum].subj);
	
	BlockMove((*tocH)->sums[sumNum].subj,name,len+1);
	*name = len;
	for (spot=name+1;spot<=name+len;spot++)
		if (*spot==':') *spot = '-';
}

/************************************************************************
 * ShowBoxSizes - title a mailbox window
 ************************************************************************/
void ShowBoxSizes(MyWindowPtr win)
{
	TOCHandle tocH = (TOCHandle) win->qWindow.refCon;
	Str31 string;
	long usedBytes,totalBytes;
	Str31 name;
	
	usedBytes = (*tocH)->usedK;
	totalBytes = (*tocH)->totalK;
	PCopy(name,(*tocH)->name);
	ComposeRString(string,BOX_SIZE_FMT,(*tocH)->count,usedBytes,
								 totalBytes-usedBytes);
	MoveTo(win->contR.left+4,win->contR.bottom+GROW_SIZE-2);
	TextSize(GetRLong(BOX_SIZE_FONT_SIZE));
	DrawString(string);
	TextSize(FontSize);
	MoveTo(win->contR.left,win->contR.bottom);
	Line(INFINITY,0);
	MoveTo(win->contR.left+GetRLong(BOX_SIZE_SIZE)-1,win->contR.bottom);
	Line(0,GROW_SIZE+2);
}

/************************************************************************
 * BoxDidResize - handle the size display area
 ************************************************************************/
void BoxDidResize(MyWindowPtr win, Rect *oldContR)
{
#pragma unused(oldContR)
	ControlHandle hBar = win->hBar;
	RedoTOC((TOCHandle)win->qWindow.refCon);
	if (hBar)
	{
		ControlHandle hBar = win->hBar;
		Rect r = (*hBar)->contrlRect;
		short delta = GetRLong(BOX_SIZE_SIZE);
		
		r.left += delta;
		MoveMyCntl(win,hBar,r.left,r.top,r.right-r.left,r.bottom-r.top);
		SetRect(&r,win->contR.left,oldContR->bottom,win->contR.left+delta,
							 oldContR->bottom+GROW_SIZE);
		InvalRect(&r);
	}
}

/************************************************************************
 * InvalBoxSizeBox - invalidate a window's size display
 ************************************************************************/
void InvalBoxSizeBox(MyWindowPtr win)
{
	Rect r;
	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(win);
	r = win->contR;
	r.top = r.bottom;
	r.bottom = r.top+GROW_SIZE;
	r.right = r.left + GetRLong(BOX_SIZE_SIZE);
	InsetRect(&r,1,1);
	InvalRect(&r);
	SetPort(oldPort);
}

/************************************************************************
 * BoxGonnaShow - get ready to show a mailbox window
 ************************************************************************/
void BoxGonnaShow(MyWindowPtr win)
{
	TOCHandle tocH = (TOCHandle) win->qWindow.refCon;
	SAVE_PORT;
	SetPort(win);
	
	CalcAllSumLengths(tocH);
	(*tocH)->needRedo = 0;
	win->update = BoxUpdate;
	MyWindowDidResize(win,nil);
	ScrollIt(win,0,-INFINITY);
	REST_PORT;
}

/************************************************************************
 * PriorityString - turn a priority into a string
 ************************************************************************/
UPtr PriorityString(UPtr string,Byte priority)
{
	/*
	 * normalize
	 */
	priority=Prior2Display(priority);
	GetRString(string,PRIOR_STRN+5+priority);
	return(string);
}

/************************************************************************
 * SetPriority - set a message's priority
 ************************************************************************/
void SetPriority(TOCHandle tocH,short sumNum,short priority)
{
	MessHandle messH;
	Rect r;
	if (Prior2Display(priority)!=Prior2Display((*tocH)->sums[sumNum].priority))
	{
		/* invalidate priority box in the toc */
		InvalTocBox(tocH,sumNum,WID_STAT);
		
		/* set the priority display in the message */
		if ((messH=(*tocH)->sums[sumNum].messH) && (*messH)->win->qWindow.visible)
		{
			if (GetPriorityRect((*messH)->win,&r))
			{
				SAVE_PORT;
				SetPort((*messH)->win);
				InvalRect(&r);
				REST_PORT;
			}
		}
	}
	(*tocH)->sums[sumNum].priority = priority;
	(*tocH)->dirty = True;
}

/************************************************************************
 * InvalTocBox - invalidate one area of a mailbox window
 ************************************************************************/
void InvalTocBox(TOCHandle tocH,short sumNum,short box)
{
	Rect r;
	
	if ((*tocH)->win)
	{
		SAVE_PORT;
		SetPort((*tocH)->win);
	
		r.top = (sumNum-GetCtlValue((*tocH)->win->vBar))*(*tocH)->win->vPitch;
		r.bottom = r.top + (*tocH)->win->vPitch;
		r.left = box ? (*BoxLines)[box-1] : 0;
		r.right = box < WID_LIMIT ? (*BoxLines)[box] : (*tocH)->win->contR.right;
		InvalRect(&r);
		REST_PORT;
	}
}


/************************************************************************
 * BoxSetPriorities - set the priorities of the selected messages
 ************************************************************************/
void BoxSetPriorities(TOCHandle tocH,Byte priority)
{
	short sumNum;
	
	for (sumNum=0;sumNum<(*tocH)->count;sumNum++)
		if ((*tocH)->sums[sumNum].selected) SetPriority(tocH,sumNum,priority);
}

/************************************************************************
 * BoxDragDividers - see if the user is trying to drag dividers, and help
 * him if he is.
 ************************************************************************/
Boolean BoxDragDividers(Point pt, MyWindowPtr win)
{
	short line;
	long ticks = TickCount()+GetDblTime();
	Point mouse;
	short slop = GetRLong(DOUBLE_TOLERANCE);
	RgnHandle greyRgn;
	Rect r,bounds;
	short h0;
	short dh;
	short n;
	long dvdh;
	TOCHandle tocH = (TOCHandle)win->qWindow.refCon;
	SAVE_PORT;
	
	if (BoxFindDivider(pt,&line))
	{
		while (TickCount()<ticks)
		{
			if (!StillDown()) return(False);
			GetMouse(&mouse);
			if (ABS(mouse.v-pt.v)>slop) return(False);
			if (ABS(mouse.h-pt.h)>slop) break;
		}
		if (greyRgn = NewRgn())
		{
			GetMouse(&mouse);
			r = bounds = win->contR;
			r.left = pt.h;
			r.right = pt.h+1;
			RectRgn(greyRgn,&r);
			h0 = -win->hPitch*GetCtlValue(win->hBar);
			bounds.left = line ? (*BoxLines)[line-1] : 0;
			bounds.left += h0;
			dvdh = DragGrayRgn(greyRgn,mouse,&bounds,&win->contR,hAxisOnly,nil);
			dh = dvdh&0xffff;
			/*
			 * I don't understand why I have to do this; I guess I don't understand
			 * DragGrayRgn.
			 */
			GetMouse(&mouse);
			if (mouse.h<=pt.h && dh>0) dh = 0;
			if (pt.h+dh<bounds.left) dh = 0;
			
			if (dh&0x7fff && (dh = RoundDiv(dh,FontWidth)*FontWidth))
			{
				n = WID_LIMIT;
				while (n-->line) (*BoxLines)[n] += dh;
				(*BoxLines)[line] == MAX((*BoxLines)[line],bounds.left+3);

				SaveBoxLines();
				for (win=FrontWindow();win;win=win->qWindow.nextWindow)
					if (win->qWindow.windowKind==MBOX_WIN||win->qWindow.windowKind==CBOX_WIN)
					{
						SetPort(win);
						CalcAllSumLengths(tocH);
						(*tocH)->needRedo = 0;
						InvalContent(win);
						MyWindowDidResize(win,nil);
					}
				REST_PORT;
			}
		}
		return(True);
	}
	return(False);
}

/************************************************************************
 * BoxFindDivider - is the current point over a dividing line?
 * Returns true if it is, and puts the line number in which.
 * puts the line to the right of the mouse in which if it isn't
 ************************************************************************/
Boolean BoxFindDivider(Point pt,short *which)
{
	short i, h;
	MyWindowPtr win=FrontWindow();
	short h0 = -win->hPitch*GetCtlValue(win->hBar);
	
	h = 0;
	for (i=0;i<WID_LIMIT;i++)
	{
		h = h0 + (*BoxLines)[i];
		if (h-1<=pt.h && pt.h<=h+1) {*which=i;return(True);}
		if (h>pt.h) {*which=i;return(False);}
	}
	*which = WID_LIMIT;
	return(False);
}


/************************************************************************
 * BoxCursor - change the cursor appropriately
 ************************************************************************/
void BoxCursor(Point mouse)
{
	MyWindowPtr win=FrontWindow();
	short h0 = -win->hPitch*GetCtlValue(win->hBar);
	short which;
	Rect r = win->contR;

	if (!CursorInRect(mouse,r,MouseRgn))
		SetMyCursor(arrowCursor);
	else
	{
		if (BoxFindDivider(mouse,&which))
		{
			r.left = h0 + (*BoxLines)[which]-1;
			r.right = r.left+3;
			SetMyCursor(DIVIDER_CURS);
		}
		else
		{
			SetMyCursor(plusCursor);
			if (!which)
				r.right = h0 + (*BoxLines)[0]-1;
			else
			{
				r.left = h0 + (*BoxLines)[which-1]+1;
				if (which<WID_LIMIT)
					r.right = h0 + (*BoxLines)[which]-1;
			}
		}
		RectRgn(MouseRgn,&r);
	}
}


/************************************************************************
 * RedoTOC - figure out how wide a toc is
 ************************************************************************/
void RedoTOC(TOCHandle tocH)
{
	short subMax;
	short subStart;
	short subWidth;
	short sumNum;
	MyWindowPtr win = (*tocH)->win;
	short maxValid = MIN((*tocH)->needRedo,(*tocH)->maxValid);

	if (maxValid < (*tocH)->count)
	{
		Rect r;
		SAVE_PORT;
		SetPort(win);
		/*
		 * invalidate all the new ones
		 */
		SetRect(&r,-INFINITY/2,-INFINITY/2,INFINITY/2,INFINITY/2);
		r.top = win->vPitch * (maxValid-GetCtlValue(win->vBar));
		InvalRect(&r);
		REST_PORT;
		(*tocH)->maxValid = (*tocH)->count;
		UpdateMyWindow(win);
		MyWindowMaxes(win,win->hMax,(*tocH)->count);
	}

	if ((*tocH)->needRedo < (*tocH)->count)
	{
		/*
		 * measure all the subject lines
		 */
		subStart = (*BoxLines)[WID_LIMIT-1];
		subMax = 0;
		if (FontIsFixed)
		{
			for (sumNum=(*tocH)->needRedo;sumNum<(*tocH)->count;sumNum++)
				subMax = MAX(subMax,*(*tocH)->sums[sumNum].subj);
			subMax *= win->hPitch;
		}
		else
		{
			SAVE_PORT;
			SetPort(win);
			LDRef(tocH);
			for (sumNum=(*tocH)->needRedo;sumNum<(*tocH)->count;sumNum++)
			{
				subWidth = StringWidth((*tocH)->sums[sumNum].subj);
				subMax = MAX(subMax,subWidth);
			}
			UL(tocH);
			REST_PORT;
		}
		
		/*
		 * add in widths of other fields, and convert from chars to pixels
		 */
		subMax += (*BoxLines)[WID_LIMIT-1];
		subMax = RoundDiv(subMax,win->hPitch);
		subMax += 1; /* fudge */
		
		/*
		 * install new maximums; take the old max into account only if we're
		 * doing a partial
		 */
		if ((*tocH)->needRedo) subMax = MAX(subMax,win->hMax);
		(*tocH)->needRedo = (*tocH)->count;			/* don't need to do this again */
		MyWindowMaxes(win,subMax,(*tocH)->count);
	}
}
