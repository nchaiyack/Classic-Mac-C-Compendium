#define FILE_NUM 24
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Message

	TEHandle MakeSubjectTXE(MessHandle messH);
	void MessUpdate(MyWindowPtr win);
	void MessSubjRect(MyWindowPtr win,Rect *r);
	Boolean MessScroll(MyWindowPtr win,int h,int v);
	void MessHelp(MyWindowPtr win,Point mouse);
	short MyFSWrite(short refN,long *bytes,UPtr buffer);
	void AddXlateTables(MyWindowPtr win,MenuHandle pmh);
	void NewXlateTable(MyWindowPtr win,MenuHandle pmh,short item);
	void MessZoomSize(MyWindowPtr win,Rect *zoom);
#pragma segment Main
/**********************************************************************
 * MessClose - close a message window
 **********************************************************************/
Boolean MessClose(MyWindowPtr win)
{
	MessType **messH = (MessType **)((WindowPeek)win)->refCon;
	TOCType **tocH = (*messH)->tocH;
	int sumNum = (*messH)->sumNum;
	
	if (Win2Body(win) && WinTEH(win) && Win2Body(win)!=WinTEH(win)) MessSwapTXE(messH);
	if ((*messH)->stes[0]) STEDispose((*messH)->stes[0]);
	if ((*messH)->stes[1]) STEDispose((*messH)->stes[1]);
	LL_Remove(MessList,messH,(MessType **));
	DisposHandle(messH);
	(*tocH)->sums[sumNum].messH = nil;
	return(True);
}
#pragma segment Message

/**********************************************************************
 * MessMenu - handle menu choices for a message window
 **********************************************************************/
Boolean MessMenu(MyWindowPtr win,int menu,int item,short modifiers)
{
	Str63 scratch, which;
	MessType **messH = (MessType **)((WindowPeek)win)->refCon;
	TOCType **tocH = (*messH)->tocH;
	int sumNum = (*messH)->sumNum;
	Boolean result = False;
	long dirId;
	short function;
	
	switch (menu)
	{
		case FILE_MENU:
			switch(item)
			{
				case FILE_PRINT_ITEM:
				case FILE_PRINT_SELECT_ITEM:
					PrintOneMessage((*messH)->win,item==FILE_PRINT_SELECT_ITEM);
					result = True;
					break;
				case FILE_SAVE_AS_ITEM:
					SaveMessageAs(messH);
					result = True;
					break;
			}
			break;
		case EDIT_MENU:
			if (win->ro)
				switch (item)
				{
					case EDIT_COPY_ITEM:
						result = TESomething(win,item,0,modifiers);
						break;
				}
			else
			{
				result = TESomething(win,item,0,modifiers);
				if (item==EDIT_PASTE_ITEM || item==EDIT_QUOTE_ITEM)
				{
					Boolean foundOne=False;
					TEHandle teh = (*Win2MessH(win))->txes[SUBJ_HEAD-1];
					char *spot = *(*teh)->hText;
					char *end = spot + (*teh)->teLength;
					for (;spot<end;spot++)
						if (*spot=='\n') {*spot=' '; foundOne=True;}
					if (foundOne) {ResizeSTE(win->ste,nil);}
				}
			}
			break;
		case MESSAGE_MENU:
			switch (item)
			{
				case MESSAGE_REPLY_ITEM:
					DoReplyMessage(win,modifiers,0,True);
					result = True;
					break;
				case MESSAGE_REDISTRIBUTE_ITEM:
					DoRedistributeMessage(win,0);
					result = True;
					break;
				case MESSAGE_FORWARD_ITEM:
					DoForwardMessage(win,0);
					result = True;
					break;
				case MESSAGE_SALVAGE_ITEM:
					DoSalvageMessage(win);
					result = True;
					break;
				case MESSAGE_DELETE_ITEM:
					if (CloseMyWindow(win))
					{
						DeleteMessage(tocH,sumNum);
						if ((*tocH)->win)
						{
							if (!PrefIsSet(PREF_NO_AUTO_OPEN) && sumNum<(*tocH)->count &&
									(*tocH)->sums[sumNum].state==UNREAD)
							{
								SelectBoxRange(tocH,sumNum,sumNum,False,-1,-1);
								BoxOpen((*tocH)->win);
							}
							else
								BoxSelectAfter((*tocH)->win,sumNum);
						}
					}
					result = True;
					break;
			}
			break;
		case REPLY_TO_HIER_MENU:
			DoReplyMessage(win,modifiers,item,True);
			result = True;
			break;
		case FORWARD_TO_HIER_MENU:
			DoForwardMessage(win,item);
			result = True;
			break;
		case REDIST_TO_HIER_MENU:
			DoRedistributeMessage(win,item);
			result = True;
			break;
		case SPECIAL_MENU:
			switch(item)
			{
				case SPECIAL_MAKE_NICK_ITEM:
					MakeMessNick(win,modifiers);
					result = True;
					break;
			}
			break;
		case PRIOR_MENU:
			SetPriority(tocH,(*messH)->sumNum,Display2Prior(item));
			result = True;
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
					PCopy(scratch,(*tocH)->name);
					if (GetTransferParams(menu,item,&dirId,which) &&
							(dirId!=(*tocH)->dirId || !EqualString(which,scratch,False,True)))
					{
						MoveMessage(tocH,sumNum,dirId,which,(modifiers&optionKey)!=0);
						if ((*tocH)->win && !(modifiers&optionKey))
						{
							BoxSelectAfter((*tocH)->win,sumNum);
							if (!PrefIsSet(PREF_NO_AUTO_OPEN) && (sumNum=BoxNextSelected(tocH,-1))>=0 && (*tocH)->sums[sumNum].state==UNREAD)
								BoxOpen((*tocH)->win);
						}
					}
					result=True;
					break;
				default:
					break;
			}
	}
	return(result);
} 

/************************************************************************
 * SaveMessageAs - save a message in text only form
 ************************************************************************/
void SaveMessageAs(MessHandle messH)
{
	long creator;
	short vRefN, refN;
	short err;
	Str31 scratch,name;
	
	/*
	 * tickle stdfile
	 */
	GetPref(scratch,PREF_CREATOR);
	if (*scratch!=4) GetRString(scratch,TEXT_CREATOR);
	BlockMove(scratch+1,&creator,4);
	MakeMessFileName((*messH)->tocH,(*messH)->sumNum,name);
	ExcludeHeaders = PrefIsSet(PREF_EXCLUDE_HEADERS);
	Paragraphs = PrefIsSet(PREF_PARAGRAPHS);
	if (err=SFPutOpen(name,&vRefN,creator,'TEXT',&refN,SaveAsFilter,SAVEAS_DLOG))
		return;
	
	/*
	 * do it
	 */
	err = SaveAsToOpenFile(refN,messH);
	
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
 * SaveAsToOpenFile - continue the message saving process
 ************************************************************************/
short SaveAsToOpenFile(short refN,MessHandle messH)
{
	long bytes;
	short err=0, itx;
	UPtr where;
	TEHandle teh=BodyOf(messH);
	Str63 scratch;
	
	if (!Paragraphs)
	{
		if ((*messH)->txes[0])
		{
			for (itx=ExcludeHeaders?HEAD_LIMIT-1:0;!err && itx<HEAD_LIMIT;itx++)
			{
				if (itx<HEAD_LIMIT-1)
				{
					GetRString(scratch,HEADER_STRN+itx+1);
					PCatC(scratch,' ');
					bytes = *scratch;
					(void) FSWrite(refN,&bytes,scratch+1);
				}
				bytes = (*(*messH)->txes[itx])->teLength;
				err = FSWrite(refN,&bytes,LDRef((*(*messH)->txes[itx])->hText));
				UL((*(*messH)->txes[itx])->hText);
				bytes = 1;
				if (!err) err = FSWrite(refN,&bytes,"\n");
			}
		}
		else
		{
			where = LDRef((*teh)->hText);
			bytes = (*teh)->teLength;
			if (ExcludeHeaders)
			{
				where += SumOf(messH)->bodyOffset - (*messH)->weeded;
				bytes -= SumOf(messH)->bodyOffset - (*messH)->weeded;
				while (bytes > 1 && *where == '\n') {where++;bytes--;}
				while (bytes>1&&where[bytes-1]==where[bytes-2]&&where[bytes-1]=='\n')
					bytes--;
			}
			err = FSWrite(refN,&bytes,where);
			UL((*teh)->hText);
		}
	}
	else
	{
		if ((*messH)->txes[0])
		{
			for (itx=ExcludeHeaders?HEAD_LIMIT-1:0;!err && itx<HEAD_LIMIT;itx++)
			{
				if (itx<HEAD_LIMIT-1)
				{
					GetRString(scratch,HEADER_STRN+itx+1);
					PCatC(scratch,' ');
					bytes = *scratch;
					(void) FSWrite(refN,&bytes,scratch+1);
				}
				err = UnwrapSave(LDRef((*(*messH)->txes[itx])->hText),
												 (*(*messH)->txes[itx])->teLength,0,refN);
				UL((*(*messH)->txes[itx])->hText);
				if (itx<HEAD_LIMIT-1 && !err)
				{
					bytes = 1;
					err = FSWrite(refN,&bytes,"\n");
				}
			}
		}
		else
		{
			where = LDRef((*teh)->hText);
			bytes = (*teh)->teLength;
			if (ExcludeHeaders)
			{
				where += SumOf(messH)->bodyOffset - (*messH)->weeded;
				bytes -= SumOf(messH)->bodyOffset - (*messH)->weeded;
				while (bytes > 1 && *where == '\n') {where++;bytes--;}
				while (bytes>1&&where[bytes-1]==where[bytes-2]&&where[bytes-1]=='\n')
					bytes--;
			}
			err = UnwrapSave(where,bytes,0,refN);
			UL((*teh)->hText);
		}
	}
	return(err);
}

/************************************************************************
 * UnwrapSave - save and unwrap a message
 ************************************************************************/
#define flushBChars() do {																			\
	if (bytes=bSpot-buffer) 																			\
	{ 																														\
		 err = refN ? FSWrite(refN,&bytes,buffer) : 								\
		 (WrapHandle ? PtrAndHand(buffer,WrapHandle,bytes) : noErr);\
		 if (err) goto done;																				\
		 bSpot = buffer;																						\
	} 																														\
} while (0)
#define putBChar(theC) do { 																		\
	*bSpot++ = theC;																							\
	if (bSpot==bEnd) flushBChars(); 															\
} while (0)
int UnwrapSave(UPtr text, short length, short offset, short refN)
{
	struct LineInfo
	{
			int length;
			int indent;
			int quote;
			int needReturn;
	};
	struct LineInfo lines[2];
	struct LineInfo *this, *last;
	int flip;
	int c;
	UPtr tSpot;
	int begin;
	int spaces;
	UPtr buffer = NuPtr(BUFFER_SIZE);
	UPtr bSpot, bEnd;
	int err=0;
	long bytes;
	Str31 s;
	Byte quote=GetRString(s,QUOTE_PREFIX)[1];
	short i;
	
	
	if (!buffer) return(MemError());
	bEnd = buffer+BUFFER_SIZE;
	bSpot = buffer;

	WriteZero(lines,sizeof(lines));
	this = lines;
	last = lines+1;
	flip=this->length=this->indent=this->needReturn=last->length=last->indent=0;
	last->needReturn=begin=1;

	for (tSpot=text+offset;tSpot<text+length;tSpot++)
	{
		c = *tSpot;
		if (c=='\n')
		{
			this->needReturn =	this->needReturn ||
								this->indent>20 || this->length<40;
			if (this->needReturn)
			{
				if (this->length==0 && !last->needReturn)
				{
					putBChar('\n');
				}
				putBChar('\n');
			}
			else
				putBChar(' ');
			last = this;
			flip = 1-flip;
			this = lines + flip;
			this->length = this->quote = this->indent = this->needReturn = 0;
			begin = 1;
			spaces = 0;
		}
		else if (c==' ')
		{
			if (begin) this->indent++;
			else spaces++;
		}
		else if (begin && c==quote)
		{
			this->quote++;
		}
		else
		{
			if (!begin)
			{
				if (spaces>4 && SendWDS!=WrapSendWDS)
				{
					putBChar('\t');
					this->needReturn = 1;
				}
				else if (spaces)
				{
					putBChar(' ');
					this->length++;
				}
			}
			else if ((this->indent>last->indent || this->quote!=last->quote) &&
							 !last->needReturn)
			{
				putBChar('\n');
				for (i=0;i<this->quote;i++) putBChar(c);
				this->length += this->quote;
			}
			else if (this->quote && last->needReturn)
			{
				for (i=0;i<this->quote;i++) putBChar(quote);
				this->length += this->quote;
			}
			begin = 0;
			spaces = 0;
			putBChar(c);
			this->length++;
		}
	}
	flushBChars();
done:
	DisposPtr(buffer);
	return(err);
}

/************************************************************************
 * MessKey - handle a keydown in a message window
 ************************************************************************/
void MessKey(MyWindowPtr win, EventRecord *event)
{
	MessHandle messH = (MessHandle)win->qWindow.refCon;
	TOCHandle tocH = (*messH)->tocH;
	long uLetter = UnadornMessage(event)&charCodeMask;
	
	if (leftArrowChar<=uLetter && uLetter<=downArrowChar &&
			((event->modifiers&cmdKey)!=0 ?
				!PrefIsSet(PREF_NO_CMD_ARROW):PrefIsSet(PREF_PLAIN_ARROW)))
	{
		NextMess(tocH,messH,uLetter,event->modifiers);
		return;
	}
	else if ((*messH)->txes[SUBJ_HEAD-1] &&
					 (uLetter==tabChar || uLetter==enterChar))
	{
		MessSwapTXE(messH);
		if (WinTEH(win) != BodyOf(messH)) TESetSelect(0,INFINITY,WinTEH(win));
	}
	else if (event->modifiers&cmdKey)
	{
		if ('1'<=uLetter && uLetter<='5')
			SetPriority(tocH,(*messH)->sumNum,Display2Prior(uLetter-'0'));
		else
			SysBeep(20L);
		return;
	}
	else if (win->ro && DirtyKey(event->message))
		AlertStr(READ_ONLY_ALRT, Stop, nil);
	else if (!win->ro && (uLetter==upArrowChar || uLetter==downArrowChar
					 || uLetter==returnChar))
		SysBeep(20L);
	else
	{
		TEActivate((*(STEHandle)win->ste)->te);
		TESomething(win,TEKEY,(short)(event->message & 0xff),event->modifiers);
	}
}

/************************************************************************
 * NextMess - skip to the next message
 ************************************************************************/
void NextMess(TOCHandle tocH,MessHandle messH,short whichWay,long modifiers)
{
	MyWindowPtr win;
	short next;
	
	switch (whichWay)
	{
		case upArrowChar:
		case leftArrowChar:
			next = (*messH)->sumNum-1;
			break;
		case downArrowChar:
		case rightArrowChar:
			next = (*messH)->sumNum+1;
			break;
		default:
			return;
	}
	if (next!=(*messH)->sumNum)
	{
		if (!(modifiers & optionKey)) CloseMyWindow((*messH)->win);
		if (next<0 || next>=(*tocH)->count) return;
		if ((*tocH)->sums[next].messH)
		{
			win=(*(MessHandle)(*tocH)->sums[next].messH)->win;
			ShowMyWindow(win);
			SelectWindow(win);
		}
		else
			(void) GetAMessage(tocH,next,nil,True);
		SelectBoxRange(tocH,next,next,False,-1,-1);
		BoxCenterSelection((*tocH)->win);
	}
}

/************************************************************************
 * SaveAsFilter - look for hits on our two special items
 ************************************************************************/
pascal Boolean SaveAsFilter(DialogPtr dgPtr,EventRecord *event,short *item)
{
	Rect r;
	short type;
	Handle itemH;
	Point mouse;
	GrafPtr oldPort;
	
	GetPort(&oldPort);
	SetPort(dgPtr);
	if (event->what==nullEvent || event->what==updateEvt)
	{
		if (GetDItemState(dgPtr,SADL_PARAGRAPHS)!=Paragraphs)
			SetDItemState(dgPtr,SADL_PARAGRAPHS,Paragraphs);
		if (GetDItemState(dgPtr,SADL_EXCLUDE_HEADERS)==ExcludeHeaders)
			SetDItemState(dgPtr,SADL_EXCLUDE_HEADERS,!ExcludeHeaders);
	}
	else if (event->what==mouseDown)
	{
		mouse = event->where;
		GlobalToLocal(&mouse);
		itemH = nil;
		GetDItem(dgPtr,SADL_PARAGRAPHS,&type,&itemH,&r);
		if (!itemH) goto done;
		if (PtInRect(mouse,&r))
		{
			if (TrackControl(itemH,mouse,nil))
			{
				Paragraphs = !GetDItemState(dgPtr,SADL_PARAGRAPHS);
				SetDItemState(dgPtr,SADL_PARAGRAPHS,Paragraphs);
			}
			goto done;
		}
		else
		{
			GetDItem(dgPtr,SADL_EXCLUDE_HEADERS,&type,&itemH,&r);
			if (!itemH) goto done;
			if (PtInRect(mouse,&r))
			{
				if (TrackControl(itemH,mouse,nil))
				{
					ExcludeHeaders = GetDItemState(dgPtr,SADL_EXCLUDE_HEADERS);
					SetDItemState(dgPtr,SADL_EXCLUDE_HEADERS,!ExcludeHeaders);
				}
				goto done;
			}
		}
	}
	SetPort(oldPort);
	return(DlgFilter(dgPtr,event,item));
	
	done:
		SetPort(oldPort);
		return(False);
}

/************************************************************************
 * MessClick - handle a click in the message window
 ************************************************************************/
void MessClick(MyWindowPtr win,EventRecord *event)
{
	Point pt = event->where;
	Rect r,pr;
	STEHandle ste=nil;
	MessHandle messH = Win2MessH(win);
	
	GlobalToLocal(&pt);
	r = (*(*messH)->stes[0])->encloseR;
	if (GetPriorityRect(win,&pr) && PtInRect(pt,&pr))
		PriorityMenu(win);
	else if (PtInRect(pt,&r)) ste = (*messH)->stes[0];
	else if ((*messH)->stes[1])
	{
		r = (*(*messH)->stes[1])->encloseR;
		if (PtInRect(pt,&r)) ste = (*messH)->stes[1];
	}
	
	if (ste)
	{
	  if (win->ste != ste) MessSwapTXE(messH);
		STEClick(win->ste,event);
	}
	else HandleControl(pt,win);
}

/************************************************************************
 * MessGonnaShow - get ready to show a message window
 ************************************************************************/
void MessGonnaShow(MyWindowPtr win)
{
	MessHandle messH = (MessHandle)win->qWindow.refCon;
	short margin;
	
	win->didResize = MessDidResize;
	win->key = MessKey;
	win->update = MessUpdate;
	win->help = MessHelp;
	win->dontControl = True;
	win->zoomSize = MessZoomSize;
	if (PrefIsSet(PREF_ICON_BAR))
	{
		margin = 2*FontLead+FontDescent+2;
		win->topMargin = win->contR.top = margin;
		if (MakeSubjectTXE(messH))
			win->click = MessClick;
		else
			win->topMargin = win->contR.top = 0;
		ResizeSTE((*messH)->stes[0],&win->contR);
	}
	MyWindowDidResize(win,nil);
	TEActivate(WinTEH(win));
}

/************************************************************************
 * MakeSubjectTXE - make a TERec for the subject
 ************************************************************************/
TEHandle MakeSubjectTXE(MessHandle messH)
{
	Str63 subject;
	Rect view;
	MyWindowPtr win=(*messH)->win;
	STEHandle ste;
	GrafPtr oldPort;
	
	GetPort(&oldPort);
	SetPort(win);
	
	PCopy(subject,SumOf(messH)->subj);
	
	MessSubjRect(win,&view);

	if (ste=NewSTE(win,&view,False,False,True))
	{
		(*(*ste)->te)->crOnly = -1;
		(*ste)->dontFrame = True;
		STESetText(subject+1,*subject,ste);
		(*messH)->txes[SUBJ_HEAD-1] = (*ste)->te;
		(*messH)->stes[1] = ste;
	}
	
	SetPort(oldPort);
	return(ste ? (*ste)->te : nil);
}

/************************************************************************
 * MessUpdate - update a message window
 ************************************************************************/
void MessUpdate(MyWindowPtr win)
{
	Rect pr;
	
	if (win->qWindow.visible)
	{
		MessHandle messH = (MessHandle)win->qWindow.refCon;
		TOCHandle tocH = (*messH)->tocH;
		if (SumOf(messH)->state==UNREAD)
			SetState(tocH,(*messH)->sumNum,READ);
		if ((*messH)->txes[BODY])
			STEUpdate((*messH)->stes[0]);
		if ((*messH)->txes[SUBJ_HEAD-1])
		{
			Str31 label;
			TEHandle teh = (*messH)->txes[SUBJ_HEAD-1];

			/*
			 * draw separators
			 */
			MoveTo(0,win->topMargin); Line(INFINITY,0);
			Move(0,-2); Line(-INFINITY,0);
						
			/*
			 * draw priority
			 */
			if (GetPriorityRect(win,&pr)) DrawPriority(&pr,SumOf(messH)->priority);
				
			/*
			 * draw label
			 */
			MoveTo(pr.right+FontWidth,(3*FontLead)/2);
			GetRString(label,HEADER_STRN+SUBJ_HEAD);
			DrawString(label);
						
			/*
			 * and the text
			 */
			STEUpdate((*messH)->stes[1]);
		}
	}
}

/************************************************************************
 * MessSwapTXE - switch txe's in a message window
 ************************************************************************/
void MessSwapTXE(MessHandle messH)
{
	MyWindowPtr win = (*messH)->win;
	TEHandle subTeh = (*messH)->txes[SUBJ_HEAD-1];
	TEHandle bodTeh = (*messH)->txes[BODY];
	TEHandle newTeh = (WinTEH(win)==bodTeh) ? subTeh : bodTeh;

	TEDeactivate(WinTEH(win));
	TEActivate(newTeh);
	if (newTeh==bodTeh)
  {
		win->ste = (*messH)->stes[0];
		win->ro = True;
	}
	else
	{
		win->ste = (*messH)->stes[1];
		win->ro = False;
	}
	if (win->ro)
	{
		Str63 newSubj;
		Str63 oldSubj;
		Str255 title;
		
		win->showInsert = nil;
		*newSubj = MIN(63,(*subTeh)->teLength);
		BlockMove(*(*subTeh)->hText,newSubj+1,*newSubj);
		PCopy(oldSubj,SumOf(messH)->subj);
		if (!EqualString(oldSubj,newSubj,True,True))
		{
			PCopy(SumOf(messH)->subj,newSubj);
			InvalSum((*messH)->tocH,(*messH)->sumNum);
			CalcSumLengths((*messH)->tocH,(*messH)->sumNum);
			(*(*messH)->tocH)->dirty = True;
			MakeMessTitle(title,(*messH)->tocH,(*messH)->sumNum);
			SetWTitle(win,title);
		}
	}
	else win->showInsert = NOOP;
}

/************************************************************************
 * MessDidResize - resize a message window
 ************************************************************************/
void MessDidResize(MyWindowPtr win, Rect *oldContR)
{
	MessHandle messH = (MessHandle)win->qWindow.refCon;
	Rect view;
	TEHandle bodTeh = (*messH)->txes[BODY];
	TEHandle subTeh = (*messH)->txes[SUBJ_HEAD-1];
	
	win->contR = ((GrafPtr)win)->portRect;
	if (!subTeh)
		TextDidResize(win,oldContR);
	else
	{
		MessSubjRect(win,&view); 
		ResizeSTE((*messH)->stes[1],&view);
		win->contR.top = win->topMargin;
		view = win->contR;
		view.right+=1;
		ResizeSTE((*messH)->stes[0],&view);
	}
}

/************************************************************************
 * MessSubjRect - find the rect for the subject teh
 ************************************************************************/
void MessSubjRect(MyWindowPtr win,Rect *r)
{
	Str31 label;
	MessHandle messH = Win2MessH(win);
	short hi = win->vPitch+2*TE_VMARGIN+FontDescent;
	Rect pr;
	
	GetPriorityRect(win,&pr);

	*r = win->contR;
	r->left += pr.right + 
		StringWidth(GetRString(label,HEADER_STRN+SUBJ_HEAD))+2*win->hPitch;
	r->top = (3*win->vPitch)/2-FontAscent-TE_VMARGIN-1;
	r->bottom = r->top+hi;
	r->right -= GROW_SIZE;
}

/************************************************************************
 * MessCursor - set the cursor properly
 ************************************************************************/
void MessCursor(Point mouse)
{
	MyWindowPtr win=FrontWindow();
	MessHandle messH = Win2MessH(win);
	
	if (CursorInRect(mouse,(*(*(*messH)->stes[0])->te)->viewRect,MouseRgn) ||
			win->topMargin && CursorInRect(mouse,(*(*(*messH)->stes[1])->te)->viewRect,MouseRgn))
		SetMyCursor(iBeamCursor);
	else
		SetMyCursor(arrowCursor);
}


/************************************************************************
 * GetPriorityRect - where does the priority thing belong?
 ************************************************************************/
Boolean GetPriorityRect(MyWindowPtr win,Rect *pr)
{
	short v;
	if (!win->topMargin)
	{
		SetRect(pr,0,0,0,0);
		return(False);
	}
	else
	{
		v = (win->topMargin-16)/2;
		SetRect(pr,v,v,v+16,v+16);
#ifdef NEVER
		pr->right += FontWidth*10;
#endif
		return(True);
	}
}

/************************************************************************
 * DrawPriority - draw the message priority
 ************************************************************************/
void DrawPriority(Rect *pr,short p)
{
	SICNHand theSICN;
	Rect ir = *pr;
	ir.right = ir.left+16;
	
	p = Prior2Display(p);
	if (theSICN = GetResource('SICN',PRIOR_SICN_BASE+p-1))
		PlotSICN(&ir,theSICN,0);
	if (((WindowPeek)qd.thePort)->windowKind!=CBOX_WIN &&((WindowPeek)qd.thePort)->windowKind!=MBOX_WIN)
	{
#ifdef NEVER
		Str31 string;
		MoveTo(ir.right+FontWidth/2,(3*FontLead)/2);
		GetRString(string,PRIOR_STRN+p);
		TextFont(FontID);
		TextSize(FontSize);
		DrawString(string);
#endif
		ir = *pr;
		if (FontLead+FontDescent > 16) InsetRect(&ir,0,(16-FontDescent-FontLead)/2);
		FrameRect(&ir);
		MoveTo(ir.left+1,ir.bottom);
		Line(ir.right-ir.left-1,0);
		Line(0,ir.top-ir.bottom+1);
	}
}

/************************************************************************
 * PriorityMenu - put up the priority menu, and set the priority
 ************************************************************************/
short PriorityMenu(MyWindowPtr win)
{
	MenuHandle pmh = GetMenu(PRIOR_MENU);
	Rect pr;
	MessHandle messH = Win2MessH(win);
	short p = SumOf(messH)->priority;
	long res;
	short item;
	
	if (pmh)
	{
		AddXlateTables(win,pmh);
		GetPriorityRect(win,&pr);
		BitClr((Ptr)HiliteMode, pHiliteBit);
		p = Prior2Display(p);
		LocalToGlobal((Point *)&pr);
		
		InsertMenu(pmh,-1);
		res = PopUpMenuSelect(pmh,pr.top,pr.left,p);
		item = res&0xffff0000 ? res&0xff : 0;
		if (item && item > XLATE_BAR_ITEM) NewXlateTable(win,pmh,item);
		DeleteMenu(PRIOR_MENU);
		
		if (item && item<=5)
		{
			p = Display2Prior(item);
			SetPriority((*messH)->tocH,(*messH)->sumNum,p);
		}
		else res = 0;
		
		DisposeMenu(pmh);
	}
	return(p);
}


/************************************************************************
 * AddXlateTables - add named translate tables to the current menu
 ************************************************************************/
void AddXlateTables(MyWindowPtr win,MenuHandle pmh)
{
	Boolean added=False;
	short i,n;
	Handle h;
	Str31 name;
	long type;
	short id;
	short nowId = SumOf(Win2MessH(win))->tableId;
	Boolean isOut = (*(*Win2MessH(win))->tocH)->which == OUT;
	short defltId = GetRLong(PREF_STRN+(isOut ? PREF_OUT_XLATE : PREF_IN_XLATE));
	
	n = CountResources('taBL');
	SetResLoad(False);
	for (i=1;i<=n;i++)
		if (h=GetIndResource('taBL',i))
		{
			GetResInfo(h,&id,&type,name);
			ReleaseResource(h);
			if (!ResError() && *name && ((id%2)==0)==isOut)
			{
				if (!added)
				{
					added = True;
					AppendMenu(pmh,"\p-");
				}
				MyAppendMenu(pmh,name);
				if (id==defltId) SetItemStyle(pmh,CountMItems(pmh),outline);
				if (id==nowId || (id==defltId && nowId==DEFAULT_TABLE))
					SetItemMark(pmh,CountMItems(pmh),checkMark);
			}
		}
	SetResLoad(True);
}

/************************************************************************
 * NewXlateTable - change the translate table for a window
 ************************************************************************/
void NewXlateTable(MyWindowPtr win,MenuHandle pmh,short item)
{
	Boolean makeDefault = (CurrentModifiers()&shiftKey)!=0;
	short newId,mark=False;
	long type;
	Str31 name;
	Handle h;
	Boolean isOut = (*(*Win2MessH(win))->tocH)->which == OUT;
	short id = SumOf(Win2MessH(win))->tableId;

	if (makeDefault) GetItemStyle(pmh,item,&mark);
	else GetItemMark(pmh,item,&mark);
	if (mark)
		newId = NO_TABLE;
	else
	{
		MyGetItem(pmh,item,name);
		SetResLoad(False);
		if (h=GetNamedResource('taBL',name))
		{
			GetResInfo(h,&newId,&type,name);
			if (ResError()) newId = id;	/* do nothing */
			ReleaseResource(h);
		}
		SetResLoad(True);
	}
	if (makeDefault)
	{
		NumToString(newId,name);
		ChangeStrn(PREF_STRN,(isOut?PREF_OUT_XLATE:PREF_IN_XLATE),name);
	}
	else if (id!=newId)
	{
		SumOf(Win2MessH(win))->tableId = newId;
		(*(*Win2MessH(win))->tocH)->dirty = True;
		if (!isOut) ReopenMessage(win);
	}
}


/************************************************************************
 * MessZoomSize - figure the size of a message window
 ************************************************************************/
void MessZoomSize(MyWindowPtr win,Rect *zoom)
{
	short hi,wi;
	short oldRight;
	TEHandle teh = Win2Body(win);
	
	if (!(wi=GetRLong(PREF_STRN+PREF_MWIDTH))) wi=GetRLong(DEF_MWIDTH);
	wi *= win->hPitch; wi += GROW_SIZE+2*TE_HMARGIN;
	zoom->right = zoom->left + MIN(zoom->right-zoom->left,wi);

	oldRight = (*teh)->destRect.right;
	(*teh)->destRect.right = (*teh)->destRect.left + zoom->right-zoom->left-GROW_SIZE-2*TE_HMARGIN;
	TECalText(teh);
	hi = CountTeLines(teh)*win->vPitch + 2*TE_VMARGIN + win->topMargin;
	zoom->bottom = zoom->top + MIN(zoom->bottom-zoom->top,hi);
	(*teh)->destRect.right = oldRight;
}

/************************************************************************
 * MessApp1 - scroll the message, not the subject
 ************************************************************************/
Boolean MessApp1(MyWindowPtr win,EventRecord *event)
{
	STEApp1((*Win2MessH(win))->stes[0],event);
	return(True);
}

#pragma segment Balloon
/************************************************************************
 * MessHelp - help for the message window
 ************************************************************************/
void MessHelp(MyWindowPtr win,Point mouse)
{
	Rect subRect,bodRect,priorRect;

	bodRect = (*Win2Body(win))->viewRect;
	
	if (PtInRect(mouse,&bodRect))
		HelpRect(&bodRect,MESS_HELP,90);
	else if (win->topMargin)
	{
		subRect = (*(*Win2MessH(win))->txes[SUBJ_HEAD-1])->viewRect;
		GetPriorityRect(win,&priorRect);
		
		if (PtInRect(mouse,&subRect))
			HelpRect(&subRect,SUB_EDIT_HELP,90);
		else if (PtInRect(mouse,&priorRect))
			PriorMenuHelp(win,&priorRect);
	}
}

/************************************************************************
 * PriorMenuHelp - help for the priority menu
 ************************************************************************/
void PriorMenuHelp(MyWindowPtr win,Rect *priorRect)
{
	Str255 s1,s2;
	short p = Prior2Display((SumOf(Win2MessH(win)))->priority);
	if (!HMIsBalloon())
	{
		GetRString(s1,PRIOR_MENU_HELP);
		GetRString(s2,PRIOR_STRN+5+p);
		PCat(s1,s2);
		HelpRectString(priorRect,s1,90);
	}
}
