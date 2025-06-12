#define FILE_NUM 14
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Find

/************************************************************************
 * structure to keep track of what we're finding
 ************************************************************************/
typedef struct {
	Str255 what;									/* the string we're finding */
	short onBox;									/* the mailbox we're currently looking in */
	short onByte; 								/* current position in message */
	short onMessage;							/* the message we're currently looking in */
	short onField;								/* for comp messages, the field we're in */
	short findType; 							/* message, box, any box, alias */
	MyWindowPtr dlog; 						/* our dialog, if any */
	TOCHandle tocH; 							/* current toc */
	MessHandle messH; 						/* current message */
	Boolean openedBox;						/* we opened the TOC (may need to close it) */
	Boolean openedMess; 					/* we opened the message (may need to close) */
} FindVars, *FindPtr, **FindHandle;
FindHandle FG;

#define What (*FG)->what
#define OnBox (*FG)->onBox
#define OnMessage (*FG)->onMessage
#define OnField (*FG)->onField
#define OnByte (*FG)->onByte
#define FindType (*FG)->findType
#define Dlog (*FG)->dlog
#define TocH (*FG)->tocH
#define MessH (*FG)->messH
#define OpenedBox (*FG)->openedBox
#define OpenedMess (*FG)->openedMess

/************************************************************************
 * private functions
 ************************************************************************/
	void DoFindDialog(void);
	void RestartFind(void);
	short InitFind(void);
	short BoxNumber(TOCHandle tocH);
	TOCHandle BoxByNumber(short boxNum,Boolean *opened);
	MyWindowPtr FindTopUserWindow(void);
	Boolean FindClose(MyWindowPtr win);
	Boolean FindHit(EventRecord *event,DialogPtr dlog,short item);
	Boolean FindMenu(MyWindowPtr win, int menu, int item, short modifiers);
	void DoFindOK(void);
	Boolean DoCompFind(Boolean allowWrap);
	Boolean DoMessFind(Boolean allowWrap);
	void PrimeByTxe(void);
	short FindInTeh(TEHandle teh);
	short FindSub(UPtr sub,UHandle	text,short offset);
	short GrabFindTOC(void);
	short GrabFindMessage(void);
	void ReportFindFailure(void);
	void FindCursor(Point mouse);
	void EnterSelection(TEHandle teh);
	long FindByteOffset(UPtr sub,UPtr buffer);
	Boolean FindPSub(UPtr sub, UPtr string);
	void CloseFindMess(void);
	void CloseFindBox(void);

#pragma segment Main
/************************************************************************
 * EnableFindMenu - do the enabling for the find menu
 ************************************************************************/
void EnableFindMenu(void)
{
	MyWindowPtr win = FindTopUserWindow();
	MenuHandle mh = GetMHandle(FIND_HIER_MENU);
	Boolean myWindow = win && IsMyWindow(win);
	Boolean notAlias = !myWindow ||
											myWindow && win->qWindow.windowKind != ALIAS_WIN;
	Boolean notBox = !myWindow ||
										myWindow && win->qWindow.windowKind != MBOX_WIN &&
															 win->qWindow.windowKind != CBOX_WIN;
	
	if (notAlias)
	{
		EnableItem(GetMHandle(EDIT_MENU),EDIT_FIND_ITEM);
		EnableItem(mh,0);
		EnableIf(mh,FIND_ENTER_ITEM,win&&win->hasSelection);
		EnableIf(mh,FIND_AGAIN_ITEM,FG && *What);
		EnableIf(mh,FIND_NEXT_ITEM,FG && *What);
		EnableIf(mh,FIND_NEXT_MESS_ITEM,FG && *What);
		EnableIf(mh,FIND_NEXT_BOX_ITEM,FG && *What);
	}
	else
	{
		DisableItem(GetMHandle(EDIT_MENU),EDIT_FIND_ITEM);
		DisableItem(mh,0);
	}
}

#pragma segment Find
/************************************************************************
 * DoFind - handle the user choosing find
 ************************************************************************/
void DoFind(short item)
{
	MyWindowPtr win;
	if (InitFind()) return;
	
	RestartFind();
	PrimeByTxe();
	
	switch (item)
	{
		case FIND_FIND_ITEM:
			DoFindDialog();
			return;
			break;
		case FIND_NEXT_ITEM:
			FindType = FDL_NEXT;
			break;
		case FIND_NEXT_MESS_ITEM:
			FindType = FDL_NEXT;
			OnMessage++;
			break;
		case FIND_NEXT_BOX_ITEM:
			FindType = FDL_NEXT;
			if (TocH) OnMessage = (*TocH)->count;
			break;
		case FIND_ENTER_ITEM:
			OpenedBox = OpenedMess = False;
			win = FindTopUserWindow();
			if (win && WinTEH(win) && win->hasSelection)
				EnterSelection(WinTEH(win));
			return;
			break;
	}
	DoFindOK();
}

/************************************************************************
 * Boolean FindHit - a hit on an enabled item in the Find box
 * the return value should be true if the main loop needs to do
 * something with event, false if the main loop can safely ignore it
 ************************************************************************/
Boolean FindHit(EventRecord *event,DialogPtr dlog,short item)
{
#pragma unused(event);
	Str255 scratch;
	
	if (item==FDL_CANCEL)
	{
		CloseMyWindow(dlog);
		return(False);
	}
	
	GetDIText(dlog,FDL_TEXT,scratch);
	PCopy(What,scratch);

	RestartFind();
	PrimeByTxe();
		
	switch(item)
	{
		case FDL_NEXT:
			FindType = FDL_NEXT;
			break;
		case FDL_NEXT_MESS:
			FindType = FDL_NEXT;
			OnMessage++;
			break;
		case FDL_NEXT_BOX:
			FindType = FDL_NEXT;
			if (TocH) OnMessage = (*TocH)->count;
			break;
		case FDL_SENSITIVE:
			ChangeStrn(PREF_STRN,PREF_SENSITIVE,
									PrefIsSet(PREF_SENSITIVE)?"\pn":"\py");
			SetDItemState(Dlog,FDL_SENSITIVE,PrefIsSet(PREF_SENSITIVE));
			return(False);
		case FDL_TEXT:
			OpenedBox = OpenedMess = False;
			return(False);
		case FDL_SUM_ONLY:
			ChangeStrn(PREF_STRN,PREF_SUM_ONLY,
									PrefIsSet(PREF_SUM_ONLY)?"\pn":"\py");
			SetDItemState(Dlog,FDL_SUM_ONLY,PrefIsSet(PREF_SUM_ONLY));
			return(False);
	}
	DoFindOK();
	
	return(False);
}

/************************************************************************
 * FindMenu - handle the menu items that must be handled for the find window
 ************************************************************************/
Boolean FindMenu(MyWindowPtr win, int menu, int item, short modifiers)
{
#pragma unused(modifiers)
	switch(menu)
	{
		case EDIT_MENU:
			(void) DoModelessEdit(win,item);
			break;
		default:
			return(False);
	}
	return(True);
}

/************************************************************************
 * DoFindDialog - make the Find dialog box frontmost
 ************************************************************************/
void DoFindDialog(void)
{
	Str255 what;
	if (InitFind()) return;
	if (!Dlog)
	{
		DialogPtr grumble;
		grumble = GetNewMyDialog(FIND_DLOG,nil,InFront,BottomCenterDialog);
		if (!grumble)
			{WarnUser(MEM_ERR,MemError()); return;}
		Dlog = grumble;
		Dlog->close = FindClose;
		Dlog->menu = FindMenu;
		Dlog->cursor = FindCursor;
		Dlog->hit = FindHit;
		Dlog->position = PositionPrefsTitle;
		Dlog->hot = True;
	}
	PCopy(what,What);
	SetDIText(Dlog,FDL_TEXT,what);
	SetDItemState(Dlog,FDL_SENSITIVE,PrefIsSet(PREF_SENSITIVE));
	SetDItemState(Dlog,FDL_SUM_ONLY,PrefIsSet(PREF_SUM_ONLY));
	if (!((WindowPeek)Dlog)->visible)
	{
		if (Get1NamedResource(SAVE_POS_TYPE,
													LDRef(((WindowPeek)Dlog)->titleHandle)))
			ShowMyWindow(Dlog);
		else
			ShowWindow(Dlog);
		UL(((WindowPeek)Dlog)->titleHandle);
	}
	SelectWindow(Dlog);
	SelIText(Dlog,FDL_TEXT,0,INFINITY);
	RestartFind();
	SFWTC = True;
}

/************************************************************************
 * RestartFind - re-initialize the find function
 ************************************************************************/
void RestartFind(void)
{
	MyWindowPtr win = FindTopUserWindow();
	short oldOnMessage = OnMessage;
	short oldOnBox = OnBox;
	TOCHandle oldTocH = TocH;
	
	if (!FG) return;
	FindType = FDL_NEXT;
	OnBox = OnField = OnMessage = 0;
	OnByte = -1;
	TocH = MessH = nil;
	if (win)
	{
		switch (win->qWindow.windowKind)
		{
			case MBOX_WIN:
			case CBOX_WIN:
				FindType = FDL_NEXT;
				TocH = (TOCHandle) win->qWindow.refCon;
				OnBox = BoxNumber(TocH);
				break;
			case COMP_WIN:
			case MESS_WIN:
				FindType = FDL_FIND;
				MessH = (MessHandle) win->qWindow.refCon;
				TocH = (*MessH)->tocH;
				OnBox = BoxNumber(TocH);
				OnMessage = (*MessH)->sumNum;
				break;
			case ALIAS_WIN:
				break;
		}
	}
	if (OnBox!=oldOnBox || OnMessage!=oldOnMessage) {OpenedMess = OpenedBox = False;}
}

/************************************************************************
 * InitFind - get the find stuff ready to go
 ************************************************************************/
short InitFind(void)
{
	if (FG) return(0);
	if ((FG=NewZH(FindVars))==nil) return(WarnUser(MEM_ERR,MemError()));
	RestartFind();
	return(0);
}

/************************************************************************
 * Box Numbering:
 * Boxes are numbered almost as their positions in the mailbox menu.
 * The differences are:
 *	 The separator line is ignored
 *	 The In, Out, and Trash boxes appear LAST, not FIRST
 ************************************************************************/
/************************************************************************
 * BoxNumber - what number is a given toc?
 ************************************************************************/
short BoxNumber(TOCHandle tocH)
{
	Str31 name;
	MenuHandle mh;
	short item;
	short num,n;
	long dirId;
	
	n = GetHandleSize(BoxCount)/sizeof(BoxCountElem);
	PCopy(name,(*tocH)->name);
	dirId = (*tocH)->dirId;
	mh = GetMHandle((dirId == MyDirId) ? MAILBOX_MENU : FindDirLevel(dirId));
	item = FindItemByName(mh,name);
	for (num=0;num<n;num++)
		if ((*BoxCount)[num].item==item && (*BoxCount)[num].dirId==dirId)
			return(num);
	return(n);
}

/************************************************************************
 * BoxByNumber - return the toc of a box with a given number?
 ************************************************************************/
TOCHandle BoxByNumber(short boxNum,Boolean *opened)
{
	Str31 name;
	TOCHandle tocH;
	long dirId;
	short menuId;
	
	*opened = False;
	
	if (boxNum>=GetHandleSize(BoxCount)/sizeof(BoxCountElem)) return(nil);
	dirId = (*BoxCount)[boxNum].dirId;
	menuId = (dirId==MyDirId) ? MAILBOX_MENU : FindDirLevel(dirId);
	MyGetItem(GetMHandle(menuId),(*BoxCount)[boxNum].item,name);
	
	if (tocH=FindTOC((*BoxCount)[boxNum].dirId,name)) return(tocH);
	
	if (GetMailbox((*BoxCount)[boxNum].dirId,name,False)) return(nil);
	*opened=True;
	return(FindTOC((*BoxCount)[boxNum].dirId,name));
}

/************************************************************************
 * FindTopUserWindow - find the topmost (non-find) user window
 ************************************************************************/
MyWindowPtr FindTopUserWindow(void)
{
	WindowPeek win;
	
	for (win=FrontWindow();win;win = win->nextWindow)
		if (win->visible && win->windowKind>userKind && win->windowKind!=FIND_WIN)
			return(win);
	return(nil);
}

/************************************************************************
 * FindClose - close the find window
 ************************************************************************/
Boolean FindClose(MyWindowPtr win)
{
#pragma unused(win)
	Dlog = nil;
	return(True);
}

/************************************************************************
 * DoFindOK - continue with our find
 ************************************************************************/
void DoFindOK(void)
{
	MyWindowPtr win = FindTopUserWindow();
	short kind=win->qWindow.windowKind;
	short startBox = OnBox;
	short startMessage = OnMessage;
	Boolean wrapped = False;
	short nBoxes = GetHandleSize(BoxCount)/sizeof(BoxCountElem);
	Str63 outName,name;
	EventRecord event;
	long pTicks;
	
	if (!*What) return; 									/* nothing to find */
	GetRString(outName,OUT);
	CycleBalls();
	
	switch (FindType)
	{
		case FDL_FIND:
			if (kind==COMP_WIN)
			{
				if (!DoCompFind(True)) ReportFindFailure();
			}
			else if (kind==MESS_WIN)
			{
				if (!DoMessFind(True)) ReportFindFailure();
			}
			break;
		case FDL_NEXT:
			do
			{
				CycleBalls();
				if (WNE(everyEvent,&event,0))
				{
					(void) MiniMainLoop(&event);
					if (CommandPeriod) break;
				}
				if (OnBox >= nBoxes)
				{
					OnBox = OnMessage = 0;
					OnByte = -1;
					wrapped = True;
					continue; /* try the checks again */
				}
				if (GrabFindTOC()) break;
				if (OnMessage < (*TocH)->count)
				{
					if ((*TocH)->sums[OnMessage].length < INFINITY)
					{
						if (TickCount()-pTicks > 5 || !OnMessage || OnMessage+1==(*TocH)->count)
						{
							OpenProgress();
							Progress(((OnMessage+1)*100)/(*TocH)->count,LDRef(TocH)->name);
							UL(TocH);
							pTicks = TickCount();
						}
						if (OnByte<0 && (FindPSub(What,(*TocH)->sums[OnMessage].subj) ||
								FindPSub(What,(*TocH)->sums[OnMessage].from) ||
								FindPSub(What,(*TocH)->sums[OnMessage].date)))
						{
							if (GrabFindMessage()) break;
							ShowMyWindow((*MessH)->win);
							SelectWindow((*MessH)->win);
							OnByte = 0;
							goto returnSpot;
						}
						if (OnByte<0) OnByte = 0;
						if (!PrefIsSet(PREF_SUM_ONLY))
						{
							if (GrabFindMessage()) break;
							PCopy(name,(*TocH)->name);
							if (EqualString(name,outName,False,True))
							{
								if (DoCompFind(False)) goto returnSpot;
							}
							else
							{
								if (DoMessFind(False)) goto returnSpot;
							}
						}
					}
					OnMessage++;
					OnByte = -1;
					OnField = 0;
				}
				else
				{
					OnBox++;
					OnMessage = OnField = 0;
					OnByte = -1;
				}
			}
			while (!(wrapped && OnBox==startBox && OnMessage >= startMessage));
			CloseFindMess();
			CloseFindBox();
			if (!CommandPeriod) ReportFindFailure();
			break;
	}
returnSpot:
	CloseProgress();
}

/************************************************************************
 * DoCompFind - find in a comp message
 ************************************************************************/
Boolean DoCompFind(Boolean allowWrap)
{
	MyWindowPtr win = (*MessH)->win;
	TEHandle teh;
	short offset;
	Boolean wasInMiddle = OnByte || OnField;
	
	while ((offset = FindInTeh((*MessH)->txes[OnField]))<0)
	{
		OnByte = 0;
		OnField++;
		if (OnField >= HEAD_LIMIT) break;
	}
	
	if (offset>=0)
	{
		teh = (*MessH)->txes[OnField];
		if (!win->qWindow.visible)
			ShowMyWindow(win);
		SelectWindow(win);
		TESetSelect(offset,offset+*What,teh);
		win->hasSelection = True;
		OnByte = offset+*What;
		if (teh != win->txe)
		{
			if (win->txe)
				TEDeactivate(win->txe);
			TEActivate(teh);
			win->txe = teh;
			win->ro = (*MessH)->txRo[OnField];
		}
		ShowInsertion(win,InsertAny);
		SFWTC=True;
		return(True);
	}
	else if (wasInMiddle && allowWrap)
	{
		OnByte = OnField = 0;
		return(DoCompFind(allowWrap));
	}
	else
		return(False);
}

/************************************************************************
 * DoMessFind - find in the current message
 ************************************************************************/
Boolean DoMessFind(Boolean allowWrap)
{
	Boolean wasInMiddle = OnByte!=0;
	MyWindowPtr win = (*MessH)->win;
	short offset = FindInTeh(Win2Body(win));
	if (offset>=0)
	{
		if (!win->qWindow.visible)
			ShowMyWindow(win);
		SelectWindow(win);
		if ((*(STEHandle)win->ste)->te != Win2Body(win)) MessSwapTXE(MessH);
		NoScrollTESetSelect(offset,offset+*What,Win2Body(win));
		ShowInsertion((*MessH)->win,InsertAny);
		(*MessH)->win->hasSelection = True;
		OnByte = offset+*What;
		TEActivate(Win2Body(win));
		ShowInsertion(win,InsertAny);
		SFWTC=True;
		return(True);
	}
	else if (wasInMiddle && allowWrap)
	{
		OnByte = 0;
		return(DoMessFind(allowWrap));
	}
	else
		return(False);
}

/************************************************************************
 * PrimeByTxe - use the current selection point to prime the find software
 ************************************************************************/
void PrimeByTxe(void)
{
	MyWindowPtr win = FindTopUserWindow();
	TEHandle teh;
	
	if (IsMyWindow(win) && win->qWindow.windowKind==MESS_WIN)
		if ((*(STEHandle)win->ste)->te != Win2Body(win))
			MessSwapTXE((MessHandle)win->qWindow.refCon);

	if (win && (teh=WinTEH(win)))
	{
		OnByte = (*teh)->selEnd;
		if (win->qWindow.windowKind==COMP_WIN)
			OnField = FindCompTx((MessHandle)win->qWindow.refCon,teh);
	}
}

/************************************************************************
 * FindInTeh - find a string in a given text handle.
 ************************************************************************/
short FindInTeh(TEHandle teh)
{
	short length = (*teh)->teLength;
	short offset;
	
	if (OnByte + *What > length) return(-1);
	
	offset = FindSub(What,(*teh)->hText,OnByte);
	OnByte = offset;
	if (offset>=length) return(-1);
	return(offset);
}

/************************************************************************
 * FindPSub - find a pascal substring in a pascal string
 ************************************************************************/
Boolean FindPSub(UPtr sub, UPtr string)
{
	unsigned char buffer[254*2+1];
	BlockMove(string+1,buffer,*string);
	BlockMove(sub+1,buffer+*string,*sub);
	sub[*sub+1] = buffer[*sub+*string] = 0;
	return(FindByteOffset(sub+1,buffer)<*string);
}

/************************************************************************
 * FindSub - find a substring in some text. 
 * Brute force.
 ************************************************************************/
short FindSub(UPtr sub,UHandle	text,short offset)
{
	long size = GetHandleSize(text);
	short result = 0;
	
	if (PtrAndHand(sub+1,text,*sub+1))
		return(size);
	sub[*sub+1] = (*text)[size+*sub] = 0;
	result = FindByteOffset(sub+1,*text+offset);
	SetHandleBig(text,size);
	return(result+offset);
}

/************************************************************************
 * FindByteOffset - find a substring in some text.
 * Brute force.  Sub *must* be in buffer!!!!!  Each string should be NULL
 * terminated.
 ************************************************************************/
long FindByteOffset(UPtr sub,UPtr buffer)
{
	UPtr anchor, spot, subSpot;
	short result;
	
	if (PrefIsSet(PREF_SENSITIVE))
	{
		for (anchor = buffer;;anchor++)
		{
			for (spot=anchor,subSpot=sub;
					 *spot==*subSpot && *subSpot;
					 spot++,subSpot++);
			if (!*subSpot)
			{
				result = anchor - buffer;
				break;
			}
		}
	}
	else
	{
		for (anchor = buffer;;anchor++)
		{
			if (!striscmp(sub,anchor))
			{
				result = anchor - buffer;
				break;
			}
		}
	}
	return(result);
}

/************************************************************************
 * GrabFindTOC - open the current toc
 ************************************************************************/
short GrabFindTOC(void)
{
	Boolean opened;
	TOCHandle tocH;
	
	tocH = BoxByNumber(OnBox,&opened);
	if (opened || tocH!=TocH)
	{
		CloseFindMess();
		CloseFindBox();
		OpenedBox = opened;
	}
	TocH = tocH;
	return (!TocH);
}

/************************************************************************
 * GrabFindMessage - open the current message
 ************************************************************************/
short GrabFindMessage(void)
{
	Boolean opened=False;
	if (!TocH) return(1);
	if (!(*TocH)->sums[OnMessage].messH)
	{
		if (GetAMessage(TocH,OnMessage,nil,0)) opened = True;
	}
	if (MessH!=(*TocH)->sums[OnMessage].messH || opened)
	{
		CloseFindMess();
		OpenedMess = opened;
	}
	MessH=(*TocH)->sums[OnMessage].messH;
	return(!MessH);
}


/************************************************************************
 * 
 ************************************************************************/
void CloseFindMess(void)
{
	if (MessH && OpenedMess)
	  CloseMyWindow((*MessH)->win);
	OpenedMess = False;
	MessH = nil;
}

/************************************************************************
 * 
 ************************************************************************/
void CloseFindBox(void)
{
	if (TocH && OpenedBox)
	  CloseMyWindow((*TocH)->win);
	OpenedBox = False;
	TocH = nil;
}

/************************************************************************
 * ReportFindFailure - just like it says
 ************************************************************************/
void ReportFindFailure(void)
{
	Str255 what;
	PCopy(what,What);
	AlertStr(NOT_FOUND_ALRT,Note,what);
}

/************************************************************************
 * FindCursor - set the cursor properly for the find window
 ************************************************************************/
void FindCursor(Point mouse)
{
	Rect r;
	Handle itemH;
	short type;
	
	itemH = nil;
	GetDItem(Dlog,FDL_TEXT,&type,&itemH,&r);
	if (itemH && CursorInRect(mouse,r,MouseRgn))
		SetMyCursor(iBeamCursor);
	else
		SetMyCursor(arrowCursor);
}

/************************************************************************
 * EnterSelection - enter the selection from the current txe in the
 * find window
 ************************************************************************/
void EnterSelection(TEHandle teh)
{
	int length = MIN(254,(*teh)->selEnd - (*teh)->selStart);
	Str255 what;
	
	BlockMove(*(*teh)->hText + (*teh)->selStart,What+1,length);
	*What = length;
	PCopy(what,What);
	if (Dlog) SetDIText(Dlog,FDL_TEXT,what);
}
