#define FILE_NUM 8
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Outgoing
/************************************************************************
 * private functions
 ************************************************************************/
	void CompSwitchFields(MessType **messH, Boolean forward);
	void CompAttach(MyWindowPtr win);
	void CompFixDisplay(MyWindowPtr win,short oldBottom,short newBottom,Boolean body);
	void ScrollWinRect(MyWindowPtr win, Rect *scroll, Rect *good, short offset);
	void DrawIconBar(MyWindowPtr win);
	void PlotFlag(Rect *r,unsigned long flags,unsigned long which);
	void ScrollSlice(MyWindowPtr win, short top, short bottom, short offset);
	void CompAdjBar(MyWindowPtr win);
	uLong QueueDate2Secs(UPtr timeStr,UPtr dateStr);
	Rect *CompIconRect(Rect *r,MessHandle messH,short index);
#define I_WIDTH 32
#pragma segment Balloon
/************************************************************************
 * CompHelp - provide help for composition windows
 ************************************************************************/
void CompHelp(MyWindowPtr win,Point mouse)
{
	MessHandle messH = (MessHandle)((WindowPeek)win)->refCon;
	int tx;
	TEHandle teh;
	Rect vr;
	short hnum=1;
	ControlHandle sendButton=(*messH)->sendButton;
	short i;
	
	for (tx=0;tx<HEAD_LIMIT;hnum++,tx++)
	{
		teh = (*messH)->txes[tx];
		vr = (*teh)->viewRect;
		if (PtInRect(mouse,&vr))
		{
			HelpRect(&vr,COMP_HELP_STRN+hnum,0);
			return;
		}
	}
	
	if (GetPriorityRect(win,&vr))
	{
		if (PtInRect(mouse,&vr))
		{
			PriorMenuHelp(win,&vr);
			return;
		}
		
		vr = (*sendButton)->contrlRect;
		if (PtInRect(mouse,&vr))
		{
			if (PrefIsSet(PREF_AUTO_SEND)) hnum++;
			HelpRect(&vr,COMP_HELP_STRN+hnum,100);
			return;
		}
		hnum += 2;
		
		if (win->topMargin)
		{
			for (i=0;i<ICON_BAR_NUM;i++,hnum+=2)
			{
				if (PtInRect(mouse,CompIconRect(&vr,messH,i)))
				{
					switch(i)
					{
						case 0:
							if (SumOf(messH)->flags&FLAG_SIG) hnum++;
							break;
						case 1:
							if (SumOf(messH)->flags&FLAG_WRAP_OUT) hnum++;
							break;
						case 2:
							if (SumOf(messH)->flags&FLAG_TABS) hnum++;
							break;
						case 3:
							if (SumOf(messH)->flags&FLAG_KEEP_COPY) hnum++;
							break;
						case 4:
							if (SumOf(messH)->flags&FLAG_BX_TEXT) hnum++;
							break;
					}
					HelpRect(&vr,COMP_HELP_STRN+hnum,100);
					return;
				}
			}
		}
	}
}

#pragma segment Main
/**********************************************************************
 * CompClose - close a composition window.	save the contents, iff
 * they're dirty.
 **********************************************************************/
Boolean CompClose(MyWindowPtr win)
{
	MessType **messH = (MessType **)((WindowPeek)win)->refCon;
	int which;
	
	if (win->isDirty && !NoSaves)
	{
		which = WannaSave(win);
		if (which==WANNA_SAVE_CANCEL || which==CANCEL_ITEM) return(False);
		else if (which == WANNA_SAVE_SAVE && !SaveComp(win)) return(False);
	}
	
	for (which=0;which<HEAD_LIMIT;which++)
	{
		if ((*messH)->txes[which])
		{
			TEDispose((*messH)->txes[which]);
			(*messH)->txes[which] = nil;
		}
	}
	LL_Remove(MessList,messH,(MessType **));
	(*(*messH)->tocH)->sums[(*messH)->sumNum].messH = nil;
	win->txe = nil;
	
	if (((*(*messH)->tocH)->sums[(*messH)->sumNum].flags & FLAG_NBK)!=0)
		DeleteSum((*messH)->tocH,(*messH)->sumNum);
	DisposHandle(messH);
	return(True);
}
#pragma segment Outgoing


/************************************************************************
 * CompZoomSize - figure the zoomed size of a comp window
 ************************************************************************/
void CompZoomSize(MyWindowPtr win,Rect *zoom)
{
#pragma unused(win)
	short hi,wi;
	
	if (!(wi=GetRLong(PREF_STRN+PREF_MWIDTH))) wi=GetRLong(DEF_MWIDTH);
	wi *= win->hPitch; wi += GROW_SIZE; wi+=2*TE_HMARGIN;
	
	zoom->right = zoom->left + MIN(zoom->right-zoom->left,wi);
	
	if (hi=GetRLong(PREF_STRN+PREF_MHEIGHT))
	{
		hi *= win->vPitch;
		zoom->bottom = MIN(zoom->bottom,zoom->top+hi);
	}
}
	
/**********************************************************************
 * CompScroll - scroll a composition window (not a pretty sight...)
 **********************************************************************/
Boolean CompScroll(MyWindowPtr win,int h,int v)
{
#pragma unused(h)
	MessType **messH = (MessType **)((WindowPeek)win)->refCon;
	int tx;
	short offset = win->vPitch * v;
	
	/*
	 * move the fields
	 */
	for (tx=0; tx<HEAD_LIMIT; tx++)
		OffsetTE(win,(*messH)->txes[tx],offset);
	
	/*
	 * make sure sanity prevails
	 */
	CompAdjBar(win);
	
	return(True);
}

/**********************************************************************
 * CompDidResize - take care of a resized composition window
 **********************************************************************/
void CompDidResize(MyWindowPtr win, Rect *oldContR)
{
#pragma unused(oldContR)
	MessType **messH = (MessType **)((WindowPeek)win)->refCon;
	short v;
	short h;
	TEHandle teh;
	Str255 scratch;
	short which;
	short cTop = win->contR.top;
	short cBot = win->contR.bottom;
	ControlHandle sendButton;
	
	/*
	 * the headers
	 */
	SetPort(win);
	TextFont(FontID);
	TextSize(FontSize);
	v = win->contR.top - (GetCtlValue(win->vBar) * (short)win->vPitch);
	h = StringWidth(GetRString(scratch,HEADER_LABEL_STRN+ATTACH_HEAD));
	h += FontWidth;
	for (which=TO_HEAD; which < HEAD_LIMIT; which++)
	{
		teh = (*messH)->txes[which-1];
		(*teh)->viewRect.top = MAX(cTop,v);
		(*teh)->viewRect.left = h;
		(*teh)->viewRect.right = win->contR.right;
		(*teh)->destRect = (*teh)->viewRect;
		(*teh)->destRect.top = v;
		INSET_RECT(&(*teh)->destRect,2,0);
		TECalText(teh);
		v += win->vPitch*CountTeLines(teh);
		(*teh)->destRect.bottom = v;
		(*teh)->viewRect.bottom = MIN(v,cBot);
	}
	
	if ((*(*messH)->txes[HEAD_LIMIT-1])->viewRect.bottom < cBot)
		(*(*messH)->txes[HEAD_LIMIT-1])->viewRect.bottom--;
		
	/*
	 * the body
	 */
	teh = (*messH)->txes[HEAD_LIMIT-1];
	v += win->vPitch;
	(*teh)->viewRect.top = MAX(cTop,v);
	(*teh)->viewRect.bottom = win->contR.bottom;
	(*teh)->viewRect.left = win->contR.left;
	(*teh)->viewRect.right = win->contR.right;
	(*teh)->destRect = (*teh)->viewRect;
	(*teh)->destRect.top = v;
	INSET_RECT(&(*teh)->destRect,2,0);
	TECalText(teh);
	v += win->vPitch*CountTeLines(teh);
	(*teh)->destRect.bottom = MAX(v,win->contR.bottom);
	
	/*
	 * and the scroll bar
	 */
	MyWindowMaxes(win,0,CountCompLines(messH)+1
		+(win->contR.bottom-win->contR.top)/win->vPitch/2); /* .5 window slop */
	CompAdjBar(win);
	
	/*
	 * the send button
	 */
	if (sendButton = (*messH)->sendButton)
	{
		Rect r = (*sendButton)->contrlRect;
		OFFSET_RECT(&(*sendButton)->contrlRect,
			((GrafPtr)win)->portRect.right-(r.right-r.left)/4-r.right,
			(win->topMargin-3-r.bottom+r.top)/2-r.top);
	}
		
	if (win->txe && win->isActive) TEActivate(win->txe);
	InvalRect(&((GrafPtr)win)->portRect); 	/* sorry... */
}

/**********************************************************************
 * CompClick - handle a click in a comp window.
 **********************************************************************/
void CompClick(MyWindowPtr win, EventRecord *event)
{
	MessType **messH = (MessType **)((WindowPeek)win)->refCon;
	Point pt;
	int tx;
	TEHandle teh;
	Rect vr;
	short i;
	Boolean in;
	Rect pr;
	
	SetPort(win);
	pt = event->where;
	GlobalToLocal(&pt);
	
	if (GetPriorityRect(win,&pr) && PtInRect(pt,&pr))
		PriorityMenu(win);
	else if (PtInRect(pt,&win->contR))
	{
		for (tx=0;tx<HEAD_LIMIT;tx++)
		{
			teh = (*messH)->txes[tx];
			vr = (*teh)->viewRect;
			if (PtInRect(pt,&vr))
			{
				CompTxeTo(win,teh);
				/*(*teh)->clikStuff = 255;*/
				TEClick(pt, (event->modifiers&shiftKey)!=0, teh);
				if ((*teh)->selStart==(*teh)->selEnd)
				{
					TEDeactivate(teh);
					(*teh)->clikStuff = 255;
					TESetSelect((*teh)->selStart,(*teh)->selEnd,teh);
					TEActivate(teh);
				}
				if (tx==ATTACH_HEAD-1)
					AttachSelect(win->txe);
				win->hasSelection = (*teh)->selEnd != (*teh)->selStart;
				break;
			}
		}
		EnableMenus(FrontWindow());
	}
	else if (SumOf(messH)->state!=SENT)
	{
		for (i=0;i<ICON_BAR_NUM;i++)
		{
			if (PtInRect(pt,CompIconRect(&vr,messH,i)))
			{
				in = False;
				do
				{
					GetMouse(&pt);
					if (in!=PtInRect(pt,&vr))
					{
						in = !in;
						InvertRect(&vr);
					}
				}
				while (StillDown());
				if (in)
				{
					long flags = SumOf(messH)->flags;
					InvertRect(&vr);
					vr.right = (vr.right+vr.left)/2;
					InvalRect(&vr);
					switch(i)
					{
						case 0:
							if (flags&FLAG_SIG) flags &= ~FLAG_SIG;
							else flags|=FLAG_SIG;
							break;
						case 1:
							if (flags&FLAG_WRAP_OUT) flags &= ~FLAG_WRAP_OUT;
							else flags|=FLAG_WRAP_OUT;
							break;
						case 2:
							if (flags&FLAG_TABS) flags &= ~FLAG_TABS;
							else flags|=FLAG_TABS;
							break;
						case 3:
							if (flags&FLAG_KEEP_COPY) flags &= ~FLAG_KEEP_COPY;
							else flags|=FLAG_KEEP_COPY;
							break;
						case 4:
							if (flags&FLAG_BX_TEXT) flags &= ~FLAG_BX_TEXT;
							else flags|=FLAG_BX_TEXT;
							break;
					}
					SumOf(messH)->flags = flags;
				}
				break;
			}
		}
	}
}

/************************************************************************
 * CompTxeTo - set the txe of a comp window
 ************************************************************************/
void CompTxeTo(MyWindowPtr win,TEHandle teh)
{
	if (teh != win->txe)
	{
		MessType **messH = (MessType **)((WindowPeek)win)->refCon;
		if (win->txe)
			TEDeactivate(win->txe);
		TEActivate(teh);
		win->txe = teh;
		win->ro = (*messH)->txRo[FindCompTx(messH,teh)];
	}
}

/**********************************************************************
 * CompMenu - handle menu choice for composition window
 **********************************************************************/
Boolean CompMenu(MyWindowPtr win, int menu, int item, short modifiers)
{
#pragma unused(modifiers)
	MessType **messH = (MessType **)((WindowPeek)win)->refCon;
	TOCType **tocH = (*messH)->tocH;
	int sumNum = (*messH)->sumNum;
	Str63 which,boxName;
	long dirId;
	uLong when;
	short function,state;
	Boolean now = False;
	
	if (menu==MESSAGE_MENU && item==MESSAGE_QUEUE_ITEM && modifiers&optionKey)
	  item = MESSAGE_MOD_Q_ITEM;
	switch (menu)
	{
		case FILE_MENU:
			switch (item)
			{
				case FILE_SAVE_ITEM:
					(void) SaveComp(win);
					return(True);
				case FILE_SAVE_AS_ITEM:
					SaveMessageAs(messH);
					return(True);
				case FILE_PRINT_ITEM:
				case FILE_PRINT_SELECT_ITEM:
					PrintOneMessage((*messH)->win,item==FILE_PRINT_SELECT_ITEM);
					return(True);
					break;
			}
			break;
		case EDIT_MENU:
			if (item==EDIT_WRAP_ITEM && modifiers&optionKey) item++;
			return(TESomething(win,item,0,modifiers));
			break;
			
		case MESSAGE_MENU:
			switch (item)
			{
				case MESSAGE_DELETE_ITEM:
					DeleteMessage(tocH,sumNum);
					if ((*tocH)->win)
						BoxSelectAfter((*tocH)->win,sumNum);
					return(True);
				case MESSAGE_QUEUE_ITEM:
				  when = 0;
					now = PrefIsSet(PREF_AUTO_SEND);
				queueit:
					if (SaveComp(win))
					{
						if ((*tocH)->sums[sumNum].state == UNSENDABLE)
							DoNumBigAlert(Stop,CANT_QUEUE);
						else if (CloseMyWindow(win))
						{
							SetState(tocH,sumNum,QUEUED);
							TimeStamp(tocH,sumNum,when,ZoneSecs());
							SetSendQueue();
							if (now && SendQueue) DoSendQueue();
						}
					}
					SetSendQueue();
					return(True);
				case MESSAGE_MOD_Q_ITEM:
				  when = (*tocH)->sums[sumNum].seconds;
					if (ModifyQueue(&state,&when))
				  {
					  if (now = (state==SENT)) state=QUEUED;
					  if (state==QUEUED) goto queueit;
						if ((*tocH)->sums[sumNum].state==QUEUED)
						  SetState(tocH,sumNum,SENDABLE);
						SetSendQueue();
					}
					return(True);
				case MESSAGE_FORWARD_ITEM:
					DoForwardMessage(win,0);
					return(True);
					break;
				case MESSAGE_ATTACH_ITEM:
					CompAttach(win);
					return(True);
					break;
				case MESSAGE_SALVAGE_ITEM:
					DoSalvageMessage(win);
					return(True);
					break;
			}
			break;
		case FORWARD_TO_HIER_MENU:
			DoForwardMessage(win,item);
			return(True);
			break;
		case SPECIAL_MENU:
			switch(item)
			{
				case SPECIAL_MAKE_NICK_ITEM:
					MakeCompNick(win);
					return(True);
					break;
			}
			break;
		default:
			if (menu==TRANSFER_MENU)
			{
				dirId = MyDirId;
				function = TRANSFER;
			}
			else if (menu<1||menu>=FIND_HIER_MENU) break;
			else
			{
				dirId = (*BoxMap)[menu % MAX_BOX_LEVELS];
				function = (menu-1)/MAX_BOX_LEVELS;
			}
			switch (function)
			{
				case TRANSFER:
					PCopy(boxName,(*tocH)->name);
					if (GetTransferParams(menu,item,&dirId,which) &&
							(dirId!=(*tocH)->dirId || !EqualString(which,boxName,False,True)))
					{
						MoveMessage(tocH,sumNum,dirId,which,(modifiers&optionKey)!=0);
						if (!(modifiers&optionKey) && (*tocH)->win)
							BoxSelectAfter((*tocH)->win,sumNum);
					}
					return(True);
					break;
				default:
					break;
			}
			break;
	}
	return(False);
}

/**********************************************************************
 * CompUpdate - draw the contents of a composition window
 **********************************************************************/
void CompUpdate(MyWindowPtr win)
{
	MessType **messH = (MessType **)((WindowPeek)win)->refCon;
	int which;
	int h,v,w;
	Str63 scratch;
	Rect visR;
	Rect destR;
	TEHandle teh;
	
	PenPat(&qd.gray);
	TextFont(FontID);
	TextSize(FontSize);
	h = StringWidth(GetRString(scratch,HEADER_LABEL_STRN+ATTACH_HEAD))+FontWidth;
	for (which=0;which<HEAD_LIMIT-1;which++)
	{
		teh = (*messH)->txes[which];
		visR = (*teh)->viewRect;
		v = (*teh)->destRect.top + FontAscent;
		w = StringWidth(GetRString(scratch,HEADER_LABEL_STRN+which+1));
		SectRect(&win->contR,&visR,&visR);
		if (visR.top!=visR.bottom && visR.left!=visR.right)
		{
			MoveTo(h-w,v);
			DrawString(scratch);
			TEUpdate(&visR, teh);
		}
	}
	teh = (*messH)->txes[HEAD_LIMIT-1];
	visR = (*teh)->viewRect;
	SectRect(&win->contR,&visR,&visR);
	TEUpdate(&visR, teh);
	destR = (*(*messH)->txes[HEAD_LIMIT-1])->destRect;
	if (destR.top-win->vPitch/2 > win->topMargin)
		{MoveTo(0,destR.top-win->vPitch/2); Line(win->contR.right,0);}
	PenPat(&qd.black);
	if (SumOf(messH)->flags&FLAG_ICON_BAR) DrawIconBar(win);
}

/************************************************************************
 * DrawIconBar - draw the icon bar in a window
 * SICN 's are 16x16
 * We center them top & bottom, and another 16 pixels for a checkmark
 ************************************************************************/
void DrawIconBar(MyWindowPtr win)
{
	MessHandle messH = Win2MessH(win);
	unsigned long flags = SumOf(messH)->flags;
	Rect r;
	short i,v;
	Str31 sbName,shdName;
	static short icons[]={FLAG_SIG,FLAG_WRAP_OUT,FLAG_TABS,FLAG_KEEP_COPY,FLAG_BX_TEXT};
	
	TextFont(0);
	TextSize(0);
	
	for (i=0;i<ICON_BAR_NUM;i++)
	{
		CompIconRect(&r,messH,i);
		PlotFlag(&r,flags,icons[i]);
	}

	v = win->topMargin-1;
	MoveTo(0,v); LineTo(INFINITY,v);
	MoveTo(0,v-2); LineTo(INFINITY,v-2);
	if (SumOf((MessHandle)win->qWindow.refCon)->state==SENT)
	{
		Rect r;
		PenState oldState;
	
		SetRect(&r,0,0,INFINITY,v-3);
		GetPenState(&oldState);
		PenMode(patBic);
		PenPat(&qd.gray);
		PaintRect(&r);
		SetPenState(&oldState);
	}
	
	GetCTitle((*messH)->sendButton,sbName);
	GetRString(shdName,PrefIsSet(PREF_AUTO_SEND)?SEND_BUTTON:QUEUE_BUTTON);
	if (!EqualString(sbName,shdName,False,True))
		SetCTitle((*messH)->sendButton,shdName);

	/*
	 * draw priority
	 */
	if (GetPriorityRect(win,&r)) DrawPriority(&r,SumOf(messH)->priority);
}

/************************************************************************
 * PlotFlag - plot a sicn and associated flag
 ************************************************************************/
void PlotFlag(Rect *r,unsigned long flags,unsigned long which)
{
	Handle theSICN;
	Rect r2;
	
	if (flags&which)
	{
		MoveTo(r->left,r->bottom-6);
		DrawChar(checkMark);
	}
	
	if (theSICN=GetResource('SICN',ICON_BAR_BASE_SICN+which))
	{
		SetRect(&r2,r->left+16,r->bottom-20,r->left+32,r->bottom-4);
		PlotSICN(&r2,theSICN,0);
	}
}

/**********************************************************************
 * CompKey - react to a keystroke in a composition window
 **********************************************************************/
void CompKey(MyWindowPtr win, EventRecord *event)
{
	MessType **messH = (MessType **)((WindowPeek)win)->refCon;
	TOCType **tocH = (*messH)->tocH;
	long uLetter = UnadornMessage(event)&charCodeMask;

	if (!(event->modifiers & cmdKey) &&
		(!win->txe || (event->message & charCodeMask) == TABKEY))
	{
		if (!win->ro && win->txe==(*messH)->txes[HEAD_LIMIT-1] &&
				!(event->modifiers&shiftKey)
				&& ((*tocH)->sums[(*messH)->sumNum].flags&FLAG_TABS))
			TEFakeTab(win,GetRLong(TAB_DISTANCE));
		else
			CompSwitchFields(messH,!(event->modifiers & shiftKey));
	}
	else
	{
		if (leftArrowChar<=uLetter && uLetter<=downArrowChar &&
				(event->modifiers&cmdKey)!=0)
		{
			if (PrefIsSet(PREF_NO_CMD_ARROW))
				TESomething(win,TEKEY,(short)(event->message & 0xff),event->modifiers);
			else
				NextMess(tocH,messH,uLetter,event->modifiers);
			return;
		}
		else if (event->modifiers&cmdKey)
		{
			if ('1'<=uLetter && uLetter<='5')
				SetPriority(tocH,(*messH)->sumNum,Display2Prior(uLetter-'0'));
			else
				SysBeep(20L);
			return;
		}
		else
		{
			if (win->ro && DirtyKey(event->message))
			{
				if ((event->message & charCodeMask)==backSpace &&
						FindCompTx(messH,win->txe)==ATTACH_HEAD-1 &&
						SumOf(messH)->state!=SENT)
				{
					TESomething(win,TECLEAR,0,event->modifiers);
				}
				else
					AlertStr(READ_ONLY_ALRT, Stop, nil);
			}
			else if ((*win->txe)->teLength+1 >= INFINITY)
				WarnUser(TE_TOO_MUCH,(*win->txe)->teLength+1-INFINITY);
			else
			{
				TESomething(win,TEKEY,(short)(event->message & 0xff),event->modifiers);
				win->isDirty = win->isDirty || DirtyKey(event->message);
			}
		}
	}
}

/************************************************************************
 * ScrollSlice - scroll a slice of a window
 * with a zero offset, just invalidates
 ************************************************************************/
void ScrollSlice(MyWindowPtr win, short top, short bottom, short offset)
{
	Rect scroll;
	RgnHandle rgn=NewRgn();
	
	top = MAX(top,win->contR.top);
	SetRect(&scroll,win->contR.left,top,win->contR.right,bottom);
	if (offset && rgn)
	{
		ScrollRect(&scroll,0,offset,rgn);
		if (offset>0)
			scroll.bottom = scroll.top + offset;
		else
			scroll.top = MAX(scroll.bottom + offset,win->contR.top);
	}
	InvalRect(&scroll);
	DisposeRgn(rgn);
}

/************************************************************************
 * CompSwitchFields - switch fields in a composition window
 ************************************************************************/
void CompSwitchFields(MessType **messH, Boolean forward)
{
	MyWindowPtr win = (*messH)->win;
	short newtx;
	
	newtx = FindCompTx(messH,win->txe);
	newtx = win->txe ?
			(forward ? (newtx+1)%HEAD_LIMIT : (newtx-1+HEAD_LIMIT)%HEAD_LIMIT)
		 : (forward ? 0 : HEAD_LIMIT-1);
	while (!(*messH)->win->ro && (*messH)->txRo[newtx])
		 newtx = forward ? (newtx+1)%HEAD_LIMIT : (newtx-1+HEAD_LIMIT)%HEAD_LIMIT;

	HFC(win);
	CompActivateField(messH,newtx); 
}

/************************************************************************
 * CompActivateField - make a particular field in a comp window active
 ************************************************************************/
void CompActivateField(MessType **messH, short newtx)
{
	MyWindowPtr win = (*messH)->win;
	if (win->txe)
		TEDeactivate(win->txe);
	win->txe = (*messH)->txes[newtx];
	win->ro = (*messH)->txRo[newtx];
	TEActivate(win->txe);
	TESetSelect(0,INFINITY,win->txe);
	win->hasSelection = (*win->txe)->selEnd != (*win->txe)->selStart;
	ShowInsertion(win,InsertAny);
	EnableMenus(FrontWindow());
}

/************************************************************************
 * CompAttach - attach a document to a message
 ************************************************************************/
void CompAttach(MyWindowPtr win)
{
	Str255 text;
	Str63 name;
	Str63 scratch;
	short vRefN;
	MessType **messH = (MessType **)((WindowPeek)win)->refCon;
	long dirId;
	
	if (MySFGetFile(nil,&dirId,&vRefN,&name,-1,nil))
	{
		ComposeRString(text,ATTACH_FMT,GetMyVolName(vRefN,scratch),dirId,name);
		if ((*(*messH)->txes[ATTACH_HEAD-1])->teLength)
			AppendMessText(messH,ATTACH_HEAD-1," ",1);
		AppendMessText(messH,ATTACH_HEAD-1,text+1,*text);
		AttachSelect((*messH)->txes[ATTACH_HEAD-1]);
		win->isDirty = True;
	}
}

/**********************************************************************
 * CompUnattach - remove an attachment
 **********************************************************************/
void CompUnattach(MyWindowPtr win)
{
	MessType **messH = (MessType **)((WindowPeek)win)->refCon;
	TEHandle teh = win->txe;

	win->txe = (*messH)->txes[ATTACH_HEAD-1];
	
	TESomething(win,TECLEAR, 0, 0);
	AttachSelect(win->txe);
	win->isDirty = True;
	win->txe = teh;
}

/************************************************************************
 * AttachSelect - broaden the current selection to include the entire
 * text of an attachment
 ************************************************************************/
void AttachSelect(TEHandle teh)
{
	short selStart = (*teh)->selStart;
	short selEnd = (*teh)->selEnd;
	int found=0;
	short anchorSpot = 0;
	short colonCount=0;
	short i,end;
	UPtr text;

	end = (*teh)->teLength;
	text = *(*teh)->hText;
	for (i=0;i<end;i++)
	{
		if (!found && i>=selStart) found = 1;
		if (i && i>=selEnd && found==1) found++;
		if (text[i]==':')
		{
			colonCount++;
			if (colonCount>4) colonCount=1;
			if (colonCount==1)
			{
				if (!found) anchorSpot=i;
				if (found==2)
				{
					TESetSelect(anchorSpot,i,teh);
					return;
				}
			}
		}
	}
	TESetSelect(anchorSpot,INFINITY,teh); 					
}

/************************************************************************
 * CompButton - handle the clicking of a button in the comp window
 ************************************************************************/
void CompButton(MyWindowPtr win,ControlHandle buttonHandle,long modifiers,short part)
{
	MessHandle messH=Win2MessH(win);
	if (buttonHandle==(*messH)->sendButton)
		{if (part==inButton) CompMenu(win,MESSAGE_MENU,MESSAGE_QUEUE_ITEM,modifiers);}
}

/************************************************************************
 * CompTxChanged - react when a teRec in a comp window has changed.
 ************************************************************************/
void CompTxChanged(MyWindowPtr win, int oldNl, int newNl, Boolean scroll)
{
	MessType **messH = (MessType **)((WindowPeek)win)->refCon;
	short tx;
	short deltaV;
	short oldBottom, newBottom;
	TEHandle teh = WinTEH(win);
	Boolean body = teh==(*messH)->txes[HEAD_LIMIT-1];
		
	win->isDirty = True;

	if (newNl != oldNl)
	{
		/*
		 * setup stuff
		 */
		deltaV = (newNl-oldNl)*(*teh)->lineHeight;
		oldBottom = (*teh)->destRect.bottom;
				
		/*
		 * resize our field
		 */
		(*teh)->destRect.bottom = (*teh)->destRect.top+newNl*win->vPitch;
		(*teh)->viewRect.bottom = MIN((*teh)->destRect.bottom,win->contR.bottom);
		newBottom = (*teh)->destRect.bottom;
		if (body)
		{
			newBottom += (win->contR.bottom-win->contR.top)/2;
			(*teh)->viewRect.bottom = win->contR.bottom;
		}
			
		/*
		 * those fields after this one may need to be moved
		 */
		for (tx=FindCompTx(messH,teh)+1;tx<HEAD_LIMIT;tx++)
			OffsetTE(win,(*messH)->txes[tx],deltaV);
		
		CompFixDisplay(win,oldBottom,newBottom,body);
	}

	if (scroll) {UpdateMyWindow(win);ShowInsertion(win,InsertAny);}
	
	/*
	 * now, see if the bar needs to be adjusted
	 */
	CompAdjBar(win);
	
	/*
	 * details, details
	 */
	win->hasSelection = (*teh)->selEnd != (*teh)->selStart;
}

/************************************************************************
 * CompFixDisplay - fix the display of a comp window after CompTxChanged
 ************************************************************************/
void CompFixDisplay(MyWindowPtr win,short oldBottom,short newBottom,Boolean body)
{
	short movement = newBottom - oldBottom;
	
	if (movement > 0)
	{
		/*
		 * inserted text
		 */
		/*
		 * invalidate the new area; TE may not be thinking like we are
		 * This is not necessary if we're the body
		 */
		if (!body) ScrollSlice(win,oldBottom,newBottom,0);
		
		/*
		 * scroll everything below the new bottom down, and invalidate newly
		 * emptied section.
		 */
		ScrollSlice(win,newBottom,win->contR.bottom,movement);
	}
	else
	{
		/*
		 * deleted text
		 */
		/*
		 * below the deleted area
		 */
		ScrollSlice(win,newBottom,win->contR.bottom,movement);
	}
}

/************************************************************************
 *
 ************************************************************************/
void CompAdjBar(MyWindowPtr win)
{
	MessHandle messH = (MessHandle)win->qWindow.refCon;
	short top = (*(*messH)->txes[0])->destRect.top;
	short contSize = win->contR.bottom-win->contR.top;
	short bottom = top+CountCompLines(messH)*win->vPitch + contSize/2;
	short barSize = (*win->vBar)->contrlMax*win->vPitch+contSize;
	
	if (barSize > bottom - top)
	{
		/*
		 * the scroll bar thinks the window is bigger than it is
		 */
		if (GetCtlMax(win->vBar)>GetCtlValue(win->vBar))
		{
			/*
			 * the bar thinks we can scroll down; wrong
			 */
			bottom = MAX(bottom,win->contR.bottom);
			SetCtlMax(win->vBar,(bottom-top-contSize)/win->vPitch);
		}
	}
	else if (barSize < bottom - top)
		/*
		 * the scroll bar is too small
		 */
		SetCtlMax(win->vBar,(bottom-top-contSize)/win->vPitch);
}

Boolean ModifyQueue(short *state,uLong *when)
{
	DialogPtr md;
	short dItem,item;
	Boolean done;
	Str63 timeStr,dateStr;
	uLong secs = *when ? *when+ZoneSecs() : LocalDateTime();

	if ((md=GetNewMyDialog(MODQ_DLOG,nil,InFront,ThirdCenterDialog))==nil)
	{
		WarnUser(MEM_ERR,MemError());
		return(False);
	}

	IUTimeString(secs,False,timeStr);
	IUDateString(secs,shortDate,dateStr);
	SetDIText(md,MQDL_TIME,timeStr);
	SetDIText(md,MQDL_DATE,dateStr);
	SelIText(md,MQDL_TIME,0,INFINITY);

	((MyWindowPtr)md)->update = DlgUpdate;
	
	if (*when)
	  SetDItemState(md,MQDL_LATER,True);
	else if (PrefIsSet(PREF_AUTO_SEND))
	  SetDItemState(md,MQDL_QUEUE,True);
	else
		SetDItemState(md,MQDL_NOW,True);

	/*
	 * now, display the dialog
	 */
	ShowWindow(md);
	HiliteButtonOne(md);
	PushCursor(arrowCursor);
	do
	{
		done=False;
		ModalDialog(DlgFilter,&dItem);
		switch (dItem)
		{
			case MQDL_LATER:
				SelIText(md,MQDL_TIME,0,INFINITY);
				/* fall-through */
			case MQDL_TIME:
			case MQDL_DATE:
			  dItem = MQDL_LATER;
				/* fall-through */
			default:
				for (item=MQDL_NOW;item<=MQDL_UNQUEUE;item++)
				  if (GetDItemState(md,item)!=(item==dItem))
					  SetDItemState(md,item,item==dItem);
				break;
			case MQDL_OK:
			case MQDL_CANCEL:
			case CANCEL_ITEM:
				done = True;
				break;
		}
		if (done && dItem==MQDL_OK && GetDItemState(md,MQDL_LATER))
		{
			GetDIText(md,MQDL_DATE,dateStr);
			GetDIText(md,MQDL_TIME,timeStr);
			if (!(*when=QueueDate2Secs(timeStr,dateStr))) done=False;
		}
	}
	while(!done);
	PopCursor();
	
	if (done = (dItem!=MQDL_CANCEL && dItem!=CANCEL_ITEM))
	{
	  if (GetDItemState(md,MQDL_UNQUEUE))
		  *state = SENDABLE;
		else
		  *state = GetDItemState(md,MQDL_NOW) ? SENT : QUEUED;
		if (!GetDItemState(md,MQDL_LATER)) *when = 0;
	}
	DisposDialog(md);

  if (done) SetSendQueue();
	return(done);
}

uLong QueueDate2Secs(UPtr timeStr,UPtr dateStr)
{
  uLong when;
	uLong now = LocalDateTime();
  long lenUsed;
	LongDateRec dateLDR,timeLDR;
	DateCacheRecord dc;
	DateTimeRec dtr;
	uShort dateRet=fatalDateTime;
	short timeRet;
	
	InitDateCache(&dc);
	WriteZero(&dateLDR,sizeof(dateLDR));
	WriteZero(&timeLDR,sizeof(timeLDR));
	timeRet = String2Time(timeStr+1,*timeStr,&dc,&lenUsed,&timeLDR);
	dateRet = String2Date(dateStr+1,*dateStr,&dc,&lenUsed,&dateLDR);
	if (dateRet > fatalDateTime && timeRet > fatalDateTime)
	{
	  WarnUser(DATE_ERROR,0);
		return(0);
	}
	Secs2Date(LocalDateTime(),&dtr);
	if (timeRet < fatalDateTime)
	{
	  dtr.hour = timeLDR.od.oldDate.hour;
		dtr.minute = timeLDR.od.oldDate.minute;
		dtr.second = timeLDR.od.oldDate.second;
	}
	if (dateRet < fatalDateTime)
	{
	  dtr.year = dateLDR.od.oldDate.year;
		dtr.month = dateLDR.od.oldDate.month;
		dtr.day = dateLDR.od.oldDate.day;
		dtr.dayOfWeek = dateLDR.od.oldDate.dayOfWeek;
	}
	Date2Secs(&dtr,&when);
	if (when < now && dateRet >= fatalDateTime && when + 24*3600 > now) when += 24*3600;
	if (when < now) {WarnUser(THE_PAST,0); return(0);}
	return(when-ZoneSecs());
}

void WarpQueue(uLong secs)
{
  TOCHandle tocH = GetOutTOC();
	MSumPtr sum;
  uLong now = GMTDateTime();
	
	if (tocH)
	{
	  for (sum=(*tocH)->sums;sum<(*tocH)->sums+(*tocH)->count;sum++)
		  if (sum->state == QUEUED && sum->seconds)
			{
			  if (!secs) sum->seconds = 0;
				else if (sum->seconds-now < secs) sum->seconds = now;
			}
		SetSendQueue();
	}
}

/************************************************************************
 * CompHasChanged - called after a text change
 ************************************************************************/
void CompHasChanged(MyWindowPtr win,TEHandle teh,short oldNl,short newNl)
{
	CompTxeTo(win,teh);
	CompTxChanged(win,oldNl,newNl,False);
}

/************************************************************************
 * CompIconRect - get the rect for a particular icon in the icon bar
 ************************************************************************/
Rect *CompIconRect(Rect *r,MessHandle messH,short index)
{
	short h1,h2,width,advance;
	Rect tr;
	
	/*
	 * the left and right margins of the icon area
	 */
	GetPriorityRect((*messH)->win,&tr);
	h1 = tr.right-tr.left + I_WIDTH;
	tr = (*(*messH)->sendButton)->contrlRect;
	h2 = tr.right-tr.left + I_WIDTH;
	
	/*
	 * how wide is the whole thing?
	 */
	tr = ((GrafPtr)(*messH)->win)->portRect;
	width = tr.right-tr.left-h1-h2;

	/*
	 * now, calc the advance width
	 */
	advance = (width - ICON_BAR_NUM*I_WIDTH)/(ICON_BAR_NUM-1)+I_WIDTH;
	
	/*
	 * ok, now we can make an answer
	 */
	SetRect(r,h1+index*advance,1,h1+index*advance+I_WIDTH,(*messH)->win->topMargin-4);
  return(r);
}