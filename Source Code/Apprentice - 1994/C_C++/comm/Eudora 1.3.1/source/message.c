#define FILE_NUM 25
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Message
TEHandle GetMessText(MessType **messH);
int AppendMessage(TOCType **fromTocH,int fromN,TOCType **toTocH,Boolean copy);
int MessErr;
int FindAndCopyHeader(MessType **origMH,MessType **newMH,UPtr fromHead,short toHead);
int TextFindAndCopyHeader(UPtr body,long size,MessType **newMH,UPtr fromHead,short toHead);
UPtr FindHeaderString(UPtr text,UPtr headerName,long *size);
FixMessTE(MessType **messH,short whichTXE,short oldNl);
void WeedHeaders(UHandle buffer,long *weeded,short toWeed);
MyWindowPtr OpenMessage(TOCType **tocH,int sumNum,MyWindowPtr win, Boolean showIt);
void RemoveSelf(MessHandle messH,short head);
#define HeaderName(num) (GetRString(scratch,HEADER_STRN+num),TrimWhite(scratch),scratch)
void FindFrom(UPtr who, MessHandle messH);
void Attribute(short attrId,MessHandle origMessH,MessHandle newMessH,Boolean atEnd);
void XferCustomTable(MessHandle origMessH,MessHandle newMessH);
/************************************************************************
 * GetAMessage - grab a message
 ************************************************************************/
MyWindowPtr GetAMessage(TOCType **tocH,int sumNum,MyWindowPtr win, Boolean showIt)
{
	MessHandle messH;
#ifdef DEBUG
	if (BUG6) DebugStr("\p;hc;g");
#endif
	if (!tocH || (*tocH)->count<=sumNum) return(nil);
	if (messH = (*tocH)->sums[sumNum].messH)
	{
		if (showIt)
		{
			if (!(*messH)->win->qWindow.visible)
				ShowMyWindow((*messH)->win);
			SelectWindow((*messH)->win);
		}
		UsingWindow((*messH)->win);
		return((*messH)->win);
	}
	else if ((*tocH)->which==OUT)
		return(OpenComp(tocH,sumNum,win,showIt));
	else
		return(OpenMessage(tocH,sumNum,win,showIt));
}

/**********************************************************************
 * OpenMessage - open a message in its own window
 **********************************************************************/
MyWindowPtr OpenMessage(TOCType **tocH,int sumNum,MyWindowPtr win, Boolean showIt)
{
	Str255 title;
	MessType **messH;
	Boolean turvy = showIt && (CurrentModifiers()&optionKey)!=0;
	TEHandle teh;

	CycleBalls();
	if ((*tocH)->sums[sumNum].length > 32765)
	{
		WarnUser(TE_TOO_MUCH,(*tocH)->sums[sumNum].length);
		return(nil);
	}
	
	if ((messH = (MessType **)NewZHandle(sizeof(MessType)))==nil)
		return(nil);
		
	win = GetNewMyWindow(MESSAGE_WIND,win,InFront,False,False);
	if (!win)
	{
		DisposHandle(messH);
		return(nil);
	}
	((WindowPeek)win)->windowKind = MESS_WIN;

	(*tocH)->sums[sumNum].messH = messH;
	(*messH)->win = win;
	(*messH)->sumNum = sumNum;
	(*messH)->tocH = tocH;
	/* apply FLAG_OUT ex post facto */
	if ((*tocH)->sums[sumNum].state==SENT || (*tocH)->sums[sumNum].state==UNSENT)
		(*tocH)->sums[sumNum].flags |= FLAG_OUT;
	
	((WindowPeek)win)->refCon = (long)messH;
	win->close = MessClose;
	win->menu = MessMenu;
	win->gonnaShow = MessGonnaShow;
	win->position = MessagePosition;
	win->cursor = MessCursor;
	win->button = CompButton;	/* it will do */
	win->app1 = MessApp1;
	LL_Push(MessList,messH);

	if (turvy)
		ChangeStrn(PREF_STRN,PREF_SHOW_ALL,PrefIsSet(PREF_SHOW_ALL)?"\pn":"\py");
	teh = GetMessText(messH);
	win->ste = NewSTE(win,&win->contR,False,True,False);
	if (turvy)
		ChangeStrn(PREF_STRN,PREF_SHOW_ALL,PrefIsSet(PREF_SHOW_ALL)?"\pn":"\py");
	
	if (!teh || !win->ste)
	{
		if (teh) TEDispose(teh);
		CloseMyWindow(win);
		return(nil);
	}
	else
	{
		STEInstallText((*teh)->hText,win->ste);
		(*(*(STEHandle)win->ste)->te)->teLength = (*teh)->teLength;
		LDRef(teh)->hText = NuHandle(0);
		(*teh)->teLength = 0;
		TEDispose(teh);
		(*messH)->txes[BODY] = (*(STEHandle)win->ste)->te;
		(*messH)->stes[0] = win->ste;
		(*(*messH)->stes[0])->dontFrame = True;
		(*(*messH)->stes[0])->growBox = True;
	}
	MakeMessTitle(title,tocH,sumNum);
	SetWTitle(win,title);
	if (showIt)
		ShowMyWindow(win);
	return(win);
}

/**********************************************************************
 * MakeMessTitle - make a reasonable message title from a summary
 **********************************************************************/
void MakeMessTitle(UPtr title,TOCType **tocH,int sumNum)
{
	Str63 from, date, mailbox, subject;
	Str63 pattern;
	
	PCopy(from,(*tocH)->sums[sumNum].from);
	if (*from >	(*tocH)->sums[sumNum].fromTrunc)
	{
		*from =	MAX(1,(*tocH)->sums[sumNum].fromTrunc);
		from[*from] = 'É';
	}
	
	PCopy(date,(*tocH)->sums[sumNum].date);
	if (*date >	(*tocH)->sums[sumNum].dateTrunc)
	{
		*date =	MAX(1,(*tocH)->sums[sumNum].dateTrunc);
		date[*date] = 'É';
	}
	
	PCopy(mailbox,(*tocH)->name);
	PCopy(subject,(*tocH)->sums[sumNum].subj);
	
	utl_PlugParams(GetRString(pattern,MESS_TITLE_PLUG),title,mailbox,from,date,subject);
}
		
/**********************************************************************
 * GetMessText - put the text of a message into a window's txe record
 **********************************************************************/
TEHandle GetMessText(MessType **messH)
{
	MyWindowPtr win = (*messH)->win;
	TOCType **tocH = (*messH)->tocH;
	int sumNum = (*messH)->sumNum;
	UHandle buffer = nil;
	TEHandle teh = nil;
	Rect dRect;
	long weeded;
	short tableId;
	Handle table;
	
	/*
	 * allocate space for the text
	 */
	if ((buffer=NuHandle((*tocH)->sums[sumNum].length))==nil)
	{
		WarnUser(NO_MESS_BUF,MemError());
		return(nil);
	}
	
	/*
	 * read it
	 */
	LDRef(buffer);
	if (MessErr=ReadMessage(tocH,sumNum,*buffer))
		goto failure;
	UL(buffer);
	
	/*
	 * now, set up the TERec
	 */
	dRect = win->contR;
	InsetRect(&dRect,4,0);
	if ((teh=TENew(&dRect,&win->contR))==nil)
	{
		WarnUser(MESS_TE,MemError());
		goto failure;
	}
	
	/*
	 * weed headers?
	 */
	if (!PrefIsSet(PREF_SHOW_ALL))
	{
		WeedHeaders(buffer,&weeded,BADHEAD_STRN);
		(*messH)->weeded = weeded;
	}
	else
		(*messH)->weeded = 0;
	WeedHeaders(buffer,&weeded,FROM_STRN);
	(*messH)->weeded += weeded;
	
	/*
	 * translate?
	 */
	tableId = (*tocH)->sums[sumNum].tableId;
	if (tableId==DEFAULT_TABLE) tableId = GetRLong(PREF_STRN+PREF_IN_XLATE);
	if (tableId!=NO_TABLE && (table=GetResource('taBL',tableId)))
	{
		Byte *p,*end,*t;
		long size = GetHandleSize(buffer);
		end = *buffer+size;
		t = *table;
		for (p=*buffer;p<end;p++) *p = t[*p];
	}
	
	/*
	 * put in the text...
	 */
	DisposHandle((*teh)->hText);
	(*teh)->hText = buffer;
	(*teh)->teLength = GetHandleSize(buffer);


	win->ro = True;
	return(teh);
	
failure:
	if (buffer) DisposHandle(buffer);
	if (teh) TEDispose(teh);
	return(nil);
}

/**********************************************************************
 * ReadMessage - read a given message into a preallocated buffer
 **********************************************************************/
int ReadMessage(TOCType **tocH,int sumNum,UPtr buffer)
{
	long count;
	Str255 name;
	
	BlockMove((*tocH)->name,name,(*(*tocH)->name)+1);
	count = (*tocH)->sums[sumNum].length;

	if (!(MessErr=BoxFOpen(tocH)))
	  if ((MessErr=SetFPos((*tocH)->refN,fsFromStart,
											   (*tocH)->sums[sumNum].offset)) ||
			  (MessErr=FSRead((*tocH)->refN,&count,buffer)))
			FileSystemError(READ_MBOX,name,MessErr);

	return(MessErr);
}

/**********************************************************************
 * MoveMessage - transfer a message from one box to another
 * called when the transfer menu is invoked with a message frontmost
 **********************************************************************/
int MoveMessage(TOCType **tocH,int sumNum,long dirId,UPtr toWhich,Boolean copy)
{
	TOCType **toTocH;

	CycleBalls();

	if ((toTocH = TOCByName(dirId,toWhich))==nil)
		return(1);
		
	CycleBalls();
	MessErr=AppendMessage(tocH,sumNum,toTocH,copy);
	if (MessErr) return(MessErr);
	(void) BoxFClose(tocH);
	(void) BoxFClose(toTocH);

	CycleBalls();
	if (!copy) DeleteSum(tocH,sumNum);
		
	CheckBox(FrontWindow());
	return(MessErr);
}

/**********************************************************************
 * AppendMessage - add a message to a mailbox.	Message comes from another
 * mailbox.  Things in here are a little touchy, as there are several
 * things to do, any one of which could fail.  In order, this is
 * what is done:
 * 1. Move the bytes from one mailbox to the other.
 * 2. Copy the message summary from one toc to the other, updating
 *		tocH and sumNum in the message handle (if any).
 * 3. If the message window is open, fix pointers so the message
 *		belongs to the new box, not the old one.
 * Steps 1 and 2 could fail.	In either case, no real harm is done,
 * except that we might waste some space in the new mailbox.
 * Step 3 shouldn't ever fail.
 **********************************************************************/
int AppendMessage(TOCType **fromTocH,int fromN,TOCType **toTocH,Boolean copy)
{
	UHandle buffer = nil;
	MSumType sum;
	long eof;
	short err=0;
	MessHandle fromMH;
	
	/*
	 * open the relevant mailboxes
	 */
	MessErr = BoxFOpen(fromTocH);
	if (MessErr) return(MessErr);
	MessErr = BoxFOpen(toTocH);
	if (MessErr) return(MessErr);
	
	/*
	 * if it's an outgoing message, save it first
	 */
	fromMH = (*fromTocH)->sums[fromN].messH;
	if (fromMH)
		if ((*fromTocH)->which==OUT)
		{
			MyWindowPtr win = (*fromMH)->win;
			if ((win->isDirty||(*fromTocH)->sums[fromN].flags&FLAG_NBK) &&
					!SaveComp(win)) return(1);
		}
		else if (!(*fromMH)->win->ro) MessSwapTXE(fromMH);
		
	/*
	 * copy the message from one to the other
	 */
	eof = FindTOCSpot(toTocH,(*fromTocH)->sums[fromN].length);
	MessErr = CopyFBytes((*fromTocH)->refN,(*fromTocH)->sums[fromN].offset,
		(*fromTocH)->sums[fromN].length,(*toTocH)->refN,eof);
	if (MessErr)
	{
		FileSystemError(COPY_FAILED,(*toTocH)->name,MessErr);
		return(MessErr);
	}
	(void) SetEOF((*toTocH)->refN,eof+(*fromTocH)->sums[fromN].length);
	
	/*
	 * now, create a new summary for the copied message, and put it in the
	 * new TOC.
	 */
	sum = (*fromTocH)->sums[fromN];
	sum.offset = eof;
	sum.selected = False;
	sum.messH = nil;			/* break connection with open message window */
	if (!sum.seconds) sum.seconds = GMTDateTime();
	if ((*fromTocH)->which==OUT)
	{
		if (sum.state!=SENT) sum.state=UNSENT;
		sum.tableId = NO_TABLE;		/* don't translate */
	}
		
	if ((*toTocH)->count)
	{
		MessErr = PtrAndHand(&sum,toTocH,sizeof(sum));
		if (MessErr) return(MessErr);
	}
	else
		(*toTocH)->sums[0] = sum;
	(*toTocH)->dirty = True;
	(*toTocH)->needRedo = MIN((*toTocH)->needRedo,(*toTocH)->count);
	(*toTocH)->count++;
	
	/*
	 * the message window, if any, should be closed
	 */
	if (!copy && (*fromTocH)->sums[fromN].messH)
		CloseMyWindow((*(MessHandle)(*fromTocH)->sums[fromN].messH)->win);

	return(noErr);
}


/**********************************************************************
 * MoveSelectedMessages - transfer all selected messages from one mail
 * box to another.
 **********************************************************************/
int MoveSelectedMessages(TOCType **tocH,long dirId,UPtr toWhich,Boolean copy)
{
	TOCType **toTocH;
	int sumNum;
	int lastSelected = -1;
	Str31 trashName;
	short oldCount;
	Boolean toTrash = dirId==MyDirId && EqualString(toWhich,GetRString(trashName,TRASH),False,True);
	
	if ((toTocH = TOCByName(dirId,toWhich))==nil)
		return(1);
	
	if (!PrefIsSet(PREF_EASY_DELETE) && (toTrash || (*tocH)->which==OUT))
	{
		for (sumNum = 0; sumNum < (*tocH)->count; sumNum++)
			if ((*tocH)->sums[sumNum].selected &&
				((*tocH)->sums[sumNum].state==UNREAD ||
				(*tocH)->sums[sumNum].state==QUEUED)) break;
		if (sumNum<(*tocH)->count)
		{
			Str31 sent,delete;
			GetRString(sent,(*tocH)->sums[sumNum].state==QUEUED ? SENT_VERB:READ_VERB);
			GetRString(delete,toTrash ? DELETE_VERB : TRANSFER_VERB);
			MyParamText(sent,delete,"","");
			if (ReallyDoAnAlert(TRASH_UNREAD_ALRT,Caution)!=1)
				return(0);
		}
	}
	
	for (sumNum = 0; sumNum < (*tocH)->count; sumNum++)
	{
		if ((*tocH)->sums[sumNum].selected)
		{
			CycleBalls();
			oldCount = (*tocH)->count;
			MessErr=AppendMessage(tocH,sumNum,toTocH,copy);
			if (MessErr) break;
			if (oldCount!=(*tocH)->count)
			{
				lastSelected = sumNum;
				sumNum--;
			}
			else if (!copy)
			{
				DeleteSum(tocH,sumNum);
				lastSelected = sumNum;
				sumNum--; 		/* back up, so we can try again */
			}
		}
	}
	(void) BoxFClose(tocH);
	(void) BoxFClose(toTocH);

	if ((*tocH)->win && !copy) BoxSelectAfter((*tocH)->win,lastSelected);
	CheckBox(FrontWindow());
		
	return(MessErr);
}

/**********************************************************************
 * DeleteMessage - delete a summary from a toc, and fix the screen, too
 **********************************************************************/
void DeleteMessage(TOCType **tocH, int sumNum)
{
	MessType **messH = (MessType **)(*tocH)->sums[sumNum].messH;
	Boolean dirt = 0;
	Str63 trashName;
	int oldN = (*tocH)->count;
	
	if ((*tocH)->which!=TRASH)
	{
		GetRString(trashName,TRASH);
		MoveMessage(tocH,sumNum,MyDirId,trashName,False);
	}
	else
	{
		if (messH) CloseMyWindow((*messH)->win);
		DeleteSum(tocH,sumNum);
	}
} 

/************************************************************************
 * MessageError - return the most recent error code from these functions
 ************************************************************************/
int MessageError(void)
{
	return(MessErr);
}

/************************************************************************
 * WeedHeaders - weed a message's headers, leaving only the interesting
 * ones.
 ************************************************************************/
void WeedHeaders(UHandle buffer,long *weeded,short toWeed)
{
	UPtr spot = LDRef(buffer);
	UPtr done = spot;
	UPtr end = spot + GetHandleSize(buffer);
	long size;
	Str31 badName;
	short bad;
	
	while (spot<end)
	{
		if (*spot=='\n') break;
		for (bad=1;*GetRString(badName,toWeed+bad);bad++)
			if (!striscmp(spot,badName+1))
			{
				while (spot<end)
					if (*spot++=='\n')
						if (*spot!=' ' && *spot!='\t') break;
				goto nextHead;
			}
		while (spot<end)
			if ((*done++ = *spot++)=='\n')
				if (*spot!=' ' && *spot!='\t') break;
		nextHead:;
	}
	
	while (spot<end) *done++ = *spot++;
	size = done - *buffer;
	if (weeded) *weeded = end-done;
	HUnlock(buffer);
	SetHandleBig(buffer,size);
}

/************************************************************************
 * SetMessText - stick some text into one of the fields of a message.
 ************************************************************************/
void SetMessText(MessType **messH,short whichTXE,UPtr string,short size)
{
	TEHandle teh = (*messH)->txes[whichTXE];
	int oldNl = CountTeLines(teh);

	TESetText(string,size,teh);
	FixMessTE(messH,whichTXE,oldNl);
}

/************************************************************************
 * AppendMessText - stick some text after one of the fields of a message.
 ************************************************************************/
void AppendMessText(MessType **messH,short whichTXE,UPtr string,short size)
{
	TEHandle teh = (*messH)->txes[whichTXE];
	int oldNl = CountTeLines(teh);

	NoScrollTESetSelect(INFINITY,INFINITY,teh);
	TEInsert(string,size,teh);
	FixMessTE(messH,whichTXE,oldNl);
}

/************************************************************************
 * FixMessTE - fix up a te in a message
 ************************************************************************/
FixMessTE(MessType **messH,short whichTXE,short oldNl)
{
	MyWindowPtr win = (*messH)->win;
	TEHandle oldTXE;
	short newNl;
	
	if (win)
	{
		oldTXE = win->txe;
		win->txe = (*messH)->txes[whichTXE];
		newNl = CountTeLines(win->txe);
		CompTxChanged(win,oldNl,newNl,False);
		INVAL_RECT(&(*win->txe)->destRect);
		win->txe = oldTXE;
	}
}


/************************************************************************
 * DoIterativeThingy - do something over all selected messages
 ************************************************************************/
void DoIterativeThingy(TOCType **tocH,int item,long modifiers,short toWhom)
{
	int sumNum;
	MessType **messH;
	MyWindowPtr win;
	short lastSelected = -1;
	Str255 title;

#ifdef PERF
	PerfControl(ThePGlobals,True);
#endif PERF

	if (item==MESSAGE_DELETE_ITEM && (*tocH)->which!=TRASH)
	{
		GetRString(title,TRASH);
		MoveSelectedMessages(tocH,MyDirId,title,False);
		return;
	}
	
	OpenProgress();
	
	for (sumNum=(*tocH)->count-1;!EjectBuckaroo && sumNum>=0;sumNum--)
	{
		if ((*tocH)->sums[sumNum].selected)
		{
			lastSelected = sumNum;
			MakeMessTitle(title,tocH,sumNum);
			Progress(NoChange,title);
			switch(item)
			{
				case MESSAGE_DELETE_ITEM:
					DeleteMessage(tocH,sumNum);
					break;
				default:
					messH = (*tocH)->sums[sumNum].messH;
					if (messH || GetAMessage(tocH,sumNum,nil,False))
					{
						win = (*(MessType **)(*tocH)->sums[sumNum].messH)->win;
						UsingWindow(win);
						switch(item)
						{
							case MESSAGE_SALVAGE_ITEM:
								DoSalvageMessage(win);
								break;
							case MESSAGE_FORWARD_ITEM:
								DoForwardMessage(win,toWhom);
								break;
							case MESSAGE_REPLY_ITEM:
								DoReplyMessage(win,modifiers,toWhom,True);
								break;
							case MESSAGE_REDISTRIBUTE_ITEM:
								DoRedistributeMessage(win,toWhom);
								break;
						}
						if (!messH)
							CloseMyWindow(win);
						else
							NotUsingWindow(win);
					}
				}
			}
			MonitorGrow();
	}
	CloseProgress();
	BoxSelectAfter((*tocH)->win,lastSelected);
	CheckBox(FrontWindow());
#ifdef PERF
	PerfControl(ThePGlobals,False);
#endif
}

/************************************************************************
 * BoxNextSelected - find first selection in a mailbox
 ************************************************************************/
short BoxNextSelected(TOCHandle tocH,short afterNum)
{
	int sNum, count;
	
	count = (*tocH)->count;
	
	for (sNum=afterNum+1;sNum<count;sNum++)
		if ((*tocH)->sums[sNum].selected) return(sNum);
	
	return(-1);
}

#pragma segment MsgOps
/************************************************************************
 * DoSalvageMessage - glean what you can from a bounced message's headers
 ************************************************************************/
short DoSalvageMessage(MyWindowPtr win)
{
	MessType **origMessH = (MessType **)((WindowPeek)win)->refCon;
	MessType **newMessH;
	MyWindowPtr newWin;
	Str255 scratch;
	
	if (newWin=DoComposeNew(0))
	{
		newMessH = (MessType **)newWin->qWindow.refCon;
		XferCustomTable(origMessH,newMessH);		
		if (win->qWindow.windowKind==COMP_WIN)
		{
			short tx;
			for (tx=0;tx<sizeof((*newMessH)->txes)/sizeof(TEHandle);tx++)
			{
				TESetText(LDRef((*(*origMessH)->txes[tx])->hText),
					(*(*origMessH)->txes[tx])->teLength,(*newMessH)->txes[tx]);
				UL((*(*origMessH)->txes[tx])->hText);
			}
			(*(*newMessH)->tocH)->dirty = True;
			SumOf(newMessH)->flags = SumOf(origMessH)->flags;
		}
		else
		{
			UPtr spot, oldSpot, beginning;
			long size, total;
			Boolean toFound;
			Str63 received;
			
			beginning = spot = LDRef((*Win2Body(win))->hText);
			total = size = GetHandleSize((*Win2Body(win))->hText);
			
			/*
			 * find the last "Received:" header
			 */
			GetRString(received,RECEIVED_HEAD); TrimWhite(received);
			for (oldSpot = nil;
					 spot=FindHeaderString(spot,received,&size);
					 oldSpot=spot,spot+=size,size=total-(spot-beginning-size));
			
			/*
			 * copy the relevant parts
			 */
			if (!oldSpot) oldSpot=beginning;
			{
				if (oldSpot!=beginning)
					SumOf(newMessH)->flags &= ~FLAG_SIG;
				size = total - (oldSpot-beginning);
				TextFindAndCopyHeader(oldSpot,size,newMessH,HeaderName(SUBJ_HEAD),SUBJ_HEAD);
				TextFindAndCopyHeader(oldSpot,size,newMessH,HeaderName(CC_HEAD),CC_HEAD);
				TextFindAndCopyHeader(oldSpot,size,newMessH,HeaderName(ATTACH_HEAD),ATTACH_HEAD);
				toFound = TextFindAndCopyHeader(oldSpot,size,newMessH,HeaderName(TO_HEAD),TO_HEAD);
				TextFindAndCopyHeader(oldSpot,size,newMessH,HeaderName(BCC_HEAD),BCC_HEAD);
				/*
				 * find the body
				 */
				for (spot=oldSpot;spot<oldSpot+size-1;spot++)
					if (spot[0]=='\n' && spot[1]=='\n') break;
				spot += 2;
				if (spot<oldSpot+size-1)
					SetMessText(newMessH,HEAD_LIMIT-1,spot,total-(spot-beginning));
				HUnlock((*Win2Body(win))->hText);
			}
		}
		UpdateSum(newMessH,SumOf(newMessH)->offset,SumOf(newMessH)->length);
		ShowMyWindow(newWin);
		newWin->isDirty = False;
	}
	return(!newWin);
}

/************************************************************************
 * RemoveSelf - Remove "me" from a list of addresses
 *	Ray Davison, SFU
 ************************************************************************/
void RemoveSelf(MessHandle messH,short head)
{
				Str63 temp,dummy;
				UHandle rawMyself, cookedMyself;
				UHandle rawAddress, spewHandle;
				UHandle myself=NewHandle(0);
				long offset, meOffset;
				Boolean removed = False;
				Handle text = (*(*messH)->txes[head-1])->hText;
				short nLines = CountTeLines((*messH)->txes[head-1]);
				
				/* Get a definition of who I am */
				
				GetRString(temp, ME);
				PtrAndHand(temp+1, myself, temp[0]);
				PtrAndHand(",", myself, 1);
				GetPOPInfo(temp, dummy);
				PtrAndHand(temp+1, myself, temp[0]);
				PtrAndHand(",", myself, 1);
				GetReturnAddr(temp ,True);
				PtrAndHand(temp+1, myself, temp[0]);
				rawMyself = SuckAddresses(myself, GetHandleSize(myself), False);
				cookedMyself = ExpandAliases(rawMyself, 0, False);
				DisposHandle(rawMyself);
				DisposHandle(myself);
				
				/* expand the text */
				
				rawAddress = SuckAddresses(text, GetHandleSize(text), True);
				SetHandleSize(text, 0);
				
				/* Remove myself from address */
				LDRef(cookedMyself);
				for (offset=0; (*rawAddress)[offset]; offset += (*rawAddress)[offset]+2)
				{
								/* clean up the address */
								LDRef(rawAddress);
								spewHandle = SuckPtrAddresses((*rawAddress)+offset+1,
												(*rawAddress)[offset], False);
								UL(rawAddress);
								LDRef(spewHandle);
								/* look for this in the "me" addresses */
								for (meOffset=0; (*cookedMyself)[meOffset]; meOffset += (*cookedMyself)[meOffset]+2)
								{
								if (EqualString(*spewHandle,*cookedMyself+meOffset,False,True))
									break;
								}
								DisposHandle(spewHandle);
				
								/* if we didn't find it, then add this address to the result */
								if (!(*cookedMyself)[meOffset])
								{
												LDRef(rawAddress);
												if (GetHandleSize(text)) PtrAndHand(", ", text, 2);
												PtrAndHand((*rawAddress)+offset+1,text,(*rawAddress)[offset]);
												HUnlock(rawAddress);
								}
								else removed = True;
				}
				
				DisposHandle(cookedMyself);
				DisposHandle(rawAddress);
				if (removed)
				{
					(*(*messH)->txes[head-1])->teLength = GetHandleSize(text);
					FixMessTE(messH,head-1,nLines);
				}
}

/************************************************************************
 * ReopenMessage - reopen the current message
 ************************************************************************/
MyWindowPtr ReopenMessage(MyWindowPtr win)
{
	TOCHandle tocH = (*Win2MessH(win))->tocH;
	short sumNum = (*Win2MessH(win))->sumNum;
	
	CloseMyWindow(win);
	return(OpenMessage(tocH,sumNum,nil,True));
}

/************************************************************************
 * FindFrom - find a (nicely formatted) From address
 ************************************************************************/
void FindFrom(UPtr who, MessHandle messH)
{
	UPtr found;
	Str31 header;
  long len;
	
	if ((*(*messH)->tocH)->which==OUT)
	{
	  len = (*(*messH)->txes[FROM_HEAD-1])->teLength;
		*who = MIN(63,len);
		BlockMove(*(*(*messH)->txes[FROM_HEAD-1])->hText,who+1,*who);
		if (*who) BeautifyFrom(who);
	}
	else
	{
	 	len = (*BodyOf(messH))->teLength;
		GetRString(header,FROM_HEAD+HEADER_STRN);
		if (found = FindHeaderString(LDRef((*BodyOf(messH))->hText),header,&len))
		{
			*who = MIN(len,254);
			BlockMove(found,who+1,*who);
			who[*who+1] = 0;
			BeautifyFrom(who);
		}
		else *who = 0;
		UL((*BodyOf(messH))->hText);
	}
}

/************************************************************************
 * QuoteLines - put a quote prefix before the specified TextEdit lines
 ************************************************************************/
void QuoteLines(MessType **messH,short whichTXE,int from,int to,short pfid)
{
	GrafPtr oldPort;
	int oldNl;
	MyWindowPtr win=(*messH)->win;
	TEHandle teh = (*messH)->txes[whichTXE];
	long ticks = TickCount();
	short lastSpot, spot;
	short this;
	short expand=0;
	short bSize;
	UPtr copied;
	Str15 prefix;
	
	GetPort(&oldPort);SetPort((*teh)->inPort);
	
	GetRString(prefix,pfid);
	if (!*prefix) return;
	oldNl = CountTeLines(teh);
#define HARD_NL(i) (!(*teh)->lineStarts[i] ||\
				(*(*teh)->hText)[(*teh)->lineStarts[i]-1] == '\n')

	for (this=to;this>=from;this--)
		if (HARD_NL(this)) expand += *prefix;
	if (expand)
	{
		SetHandleBig((*teh)->hText,(*teh)->teLength+expand);
		if (MemError()) return;
		lastSpot = (*teh)->teLength;
		copied = *(*teh)->hText+lastSpot+expand;
		for (this=to;this>=from;this--)
			if (HARD_NL(this))
			{
				spot = (*teh)->lineStarts[this];
				bSize = lastSpot-spot;
				BlockMove(*(*teh)->hText+spot,copied-bSize,bSize);
				copied -= bSize + *prefix;
				BlockMove(prefix+1,copied,*prefix);
				lastSpot = spot;
			}
		(*teh)->teLength += expand;
		TECalText(teh);
		INVAL_RECT(&(*teh)->viewRect);
		UpdateMyWindow(win);
	}
	win->txe = teh;
	CompTxChanged((*teh)->inPort,oldNl,CountTeLines(teh),True);
	SetPort(oldPort);
}

/************************************************************************
 * PrependMessText - stick some text before one of the fields of a message.
 ************************************************************************/
void PrependMessText(MessType **messH,short whichTXE,UPtr string,short size)
{
	TEHandle teh = (*messH)->txes[whichTXE];
	int oldNl = CountTeLines(teh);

	NoScrollTESetSelect(0,0,teh);
	TEInsert(string,size,teh);
	FixMessTE(messH,whichTXE,oldNl);
}

/************************************************************************
 * FindHeaderString - pick the given header out of a message, by name
 ************************************************************************/
UPtr FindHeaderString(UPtr text,UPtr headerName,long *size)
{
	UPtr spot,end;
	char header[MAX_HEADER];
	
	for (end = text+*size; text<end; text = spot+1)
	{
		for (spot=text;spot<end;spot++) if (*spot == '\n') break;
		BlockMove(text,header+1,*headerName);
		*header = *headerName;
		if (EqualString(header,headerName,False,False))
		{
			text += *headerName;
			while (*text==' ' || *text=='\t') text++;
			for (spot--; IsWhite(*spot); spot--);
			*size = spot-text+1;
			return(spot>=text ? text : nil);
		}
	}
	return(nil);
}
/************************************************************************
 * DoReplyMessage - craft a reply to a message
 ************************************************************************/
MyWindowPtr DoReplyMessage(MyWindowPtr win, long modifiers, short toWhom,Boolean vis)
{
	MessHandle origMessH = (MessType **)((WindowPeek)win)->refCon;
	MessHandle newMessH;
	Str255 subj,scratch;
	short bodyOffset = -1;
	MyWindowPtr newWin=nil;
	short nLines, r;
	TEHandle teh;
	short len;
	Boolean all = PrefIsSet(PREF_REPLY_ALL);
	Boolean quote = !(modifiers & shiftKey);
	Boolean notMe = PrefIsSet(PREF_NOT_ME);
	
	if (modifiers&optionKey) all = !all;
	
	if (newWin=DoComposeNew(toWhom))
	{
		newMessH = (MessHandle) newWin->qWindow.refCon;
		
		XferCustomTable(origMessH,newMessH);
		
		/* handle the subject */
		FindAndCopyHeader(origMessH,newMessH,HeaderName(SUBJ_HEAD),SUBJ_HEAD);
		GetRString(scratch,REPLY_INTRO);
		BlockMove(*(*(*newMessH)->txes[SUBJ_HEAD-1])->hText,subj+1,*scratch);
		*subj = *scratch;
		if (!EqualString(subj,scratch,False,False))
			PrependMessText(newMessH,SUBJ_HEAD-1,scratch+1,*scratch);
			
		/* reply to sender */
		if (!toWhom)
		{
			for (r=1;*GetRString(scratch,REPLY_STRN+r);r++)
			{
				TrimWhite(scratch);
				if (FindAndCopyHeader(origMessH,newMessH,scratch,TO_HEAD)) break;
			}
	  }
		
		/* bring over the other recipients and cc's, if desired */
		if (all && !toWhom)
		{
			FindAndCopyHeader(origMessH,newMessH,HeaderName(TO_HEAD),TO_HEAD);
			FindAndCopyHeader(origMessH,newMessH,HeaderName(CC_HEAD),CC_HEAD);

			/* remove self, if desired */
			if (notMe)
			{
				RemoveSelf(newMessH,TO_HEAD);
				RemoveSelf(newMessH,CC_HEAD);
			}
		}
		
		if (quote)
		{
			bodyOffset = SumOf(origMessH)->bodyOffset-(*origMessH)->weeded;
			while ((*(*Win2Body(win))->hText)[bodyOffset] == '\n' &&
						 bodyOffset<(*Win2Body(win))->teLength) bodyOffset++;
			len = (*Win2Body(win))->teLength-bodyOffset;
		}
		else if ((*Win2Body(win))->selStart != (*Win2Body(win))->selEnd)
		{
			bodyOffset = (*Win2Body(win))->selStart;
			len = (*Win2Body(win))->selEnd - bodyOffset;
		}
		
		if (bodyOffset >= 0)
		{
			while (len && (*(*Win2Body(win))->hText)[bodyOffset+len-1]=='\n') len--;
			{
				UPtr mpw_bug = LDRef((*Win2Body(win))->hText);
				SetMessText(newMessH,HEAD_LIMIT-1,mpw_bug+bodyOffset,len);
			}
			HUnlock((*Win2Body(win))->hText);
			UpdateMyWindow(newWin);
			
			/*
			 * make sure the last line is blank
			 */
			teh = (*newMessH)->txes[HEAD_LIMIT-1];
			nLines = (*teh)->nLines-1;
			if (!nLines ||
					(*teh)->lineStarts[nLines-1]-(*teh)->lineStarts[nLines-2]!=1)
				AppendMessText(newMessH,HEAD_LIMIT-1,"\n",1);

			/*
			 * quote them
			 */
			QuoteLines(newMessH,HEAD_LIMIT-1,0,nLines,QUOTE_PREFIX);
			
			/*
			 * annotate
			 */
			Attribute(all ? ATTRIBUTION:REP_SEND_ATTR,origMessH,newMessH,False);
		}

		/*
		 * copy priority (not my idea)
		 */
		if (!PrefIsSet(PREF_NO_XF_PRIOR))
			SumOf(newMessH)->origPriority = SumOf(newMessH)->priority =
				SumOf(origMessH)->origPriority;
		
		if (vis)
		{
			SetState((*origMessH)->tocH,(*origMessH)->sumNum,REPLIED);
			UpdateSum(newMessH,SumOf(newMessH)->offset,SumOf(newMessH)->length);
			ShowMyWindow(newWin);
		}
		newWin->isDirty = False;
	}
	return(newWin);
}

/************************************************************************
 * Attribute - make an attribution
 ************************************************************************/
void Attribute(short attrId,MessHandle origMessH,MessHandle newMessH,Boolean atEnd)
{
	Str63 template,date;
	Str127 who;
	Str255 attribution;
	
	GetRString(template,attrId);
	if (*template)
	{
		MSumPtr sum = LDRef((*origMessH)->tocH)->sums+(*origMessH)->sumNum;
		FindFrom(who,origMessH);
		if (*who)
		{
			PCopy(date,sum->date);
			if (date[1]==optSpace) date[1] = ' ';
			utl_PlugParams(template,attribution,who,date,sum->subj,nil);
			if (atEnd)
				AppendMessText(newMessH,HEAD_LIMIT-1,attribution+1,*attribution);
			else
				PrependMessText(newMessH,HEAD_LIMIT-1,attribution+1,*attribution);
		}
		UL((*origMessH)->tocH);
	}
}

/************************************************************************
 * DoRedistributeMessage - craft a reply to a message
 ************************************************************************/
short DoRedistributeMessage(MyWindowPtr win,short toWhom)
{
	MessType **origMessH = (MessType **)((WindowPeek)win)->refCon;
	MessType **newMessH;
	Str255 scratch, who;
	int bodyOffset;
	MyWindowPtr newWin;
	TEHandle body;
	
	if (newWin=DoComposeNew(toWhom))
	{
		newMessH = (MessType **)newWin->qWindow.refCon;
		XferCustomTable(origMessH,newMessH);		
		SetMessText(newMessH,FROM_HEAD-1,"",0);
		FindAndCopyHeader(origMessH,newMessH,HeaderName(SUBJ_HEAD),SUBJ_HEAD);
		FindAndCopyHeader(origMessH,newMessH,HeaderName(FROM_HEAD),FROM_HEAD);
		FindAndCopyHeader(origMessH,newMessH,HeaderName(ATTACH_HEAD),ATTACH_HEAD);
		GetReturnAddr(who,True);
		*scratch = (*(*newMessH)->txes[FROM_HEAD-1])->teLength;
		BlockMove(*(*(*newMessH)->txes[FROM_HEAD-1])->hText,scratch+1,*scratch);
		if (!EqualString(scratch,who,False,True))
		{
			ComposeRString(scratch,REDIST_ANNOTATE,who);
			AppendMessText(newMessH,FROM_HEAD-1,scratch+1,*scratch);
		}
		bodyOffset = (*(*origMessH)->tocH)->sums[(*origMessH)->sumNum].bodyOffset;
		bodyOffset -= (*origMessH)->weeded;
		body = Win2Body(win);
		while((*(*body)->hText)[bodyOffset]=='\n' && bodyOffset<(*body)->teLength-1)
			bodyOffset++;
		SetMessText(newMessH,HEAD_LIMIT-1,
						 LDRef((*Win2Body(win))->hText)+bodyOffset,
						 (*Win2Body(win))->teLength-bodyOffset);
		HUnlock((*Win2Body(win))->hText);
		if (SumOf(origMessH)->state!=SENT && SumOf(origMessH)->state!=UNSENT)
			SumOf(newMessH)->flags &= ~FLAG_SIG;

		/*
		 * copy priority
		 */
		SumOf(newMessH)->origPriority = SumOf(newMessH)->priority =
			SumOf(origMessH)->origPriority;
		
		SetState((*origMessH)->tocH,(*origMessH)->sumNum,REDIST);
		UpdateSum(newMessH,SumOf(newMessH)->offset,SumOf(newMessH)->length);
		ShowMyWindow(newWin);
		newWin->isDirty = False;
	}
	return(!newWin);
}

/************************************************************************
 * DoForwardMessage - forward a message to someone
 ************************************************************************/
short DoForwardMessage(MyWindowPtr win,short toWhom)
{
	MessType **origMessH = (MessType **)((WindowPeek)win)->refCon;
	MessType **newMessH;
	MyWindowPtr newWin;
	short nLines;
	TEHandle teh;
	Str255 scratch;
	
	if (newWin=DoComposeNew(toWhom))
	{
		newMessH = (MessType **)newWin->qWindow.refCon;
		XferCustomTable(origMessH,newMessH);		
		if (SumOf(origMessH)->flags&FLAG_OUT)
		{
			BuildDateHeader(scratch,SumOf(origMessH)->seconds);
			AppendMessText(newMessH,HEAD_LIMIT-1,scratch+1,*scratch);
			AppendMessText(newMessH,HEAD_LIMIT-1,"\n",1);
		}
		if (win->qWindow.windowKind==COMP_WIN)
		{
			short tx;
			for (tx=0;tx<HEAD_LIMIT;tx++)
			{
				teh = (*origMessH)->txes[tx];
				if ((*teh)->teLength)
				{
					if (tx<HEAD_LIMIT-1)
					{
						GetRString(scratch,HEADER_STRN+tx+1);
						AppendMessText(newMessH,HEAD_LIMIT-1,scratch+1,*scratch);
					}
					AppendMessText(newMessH,HEAD_LIMIT-1,LDRef((*teh)->hText),(*teh)->teLength);
					UL((*teh)->hText);
					if ((*(*teh)->hText)[(*teh)->teLength-1]!='\n')
						AppendMessText(newMessH,HEAD_LIMIT-1,"\n",1);
				}
				if (tx==HEAD_LIMIT-2) AppendMessText(newMessH,HEAD_LIMIT-1,"\n",1);
			}
			TESetText(LDRef((*(*origMessH)->txes[SUBJ_HEAD-1])->hText),
				(*(*origMessH)->txes[SUBJ_HEAD-1])->teLength,
				(*newMessH)->txes[SUBJ_HEAD-1]);
			UL((*(*origMessH)->txes[SUBJ_HEAD-1])->hText);
		}
		else
		{
			FindAndCopyHeader(origMessH,newMessH,HeaderName(SUBJ_HEAD),SUBJ_HEAD);
			/*
			 * stick it in
			 */
			AppendMessText(newMessH,HEAD_LIMIT-1,LDRef((*Win2Body(win))->hText),
															(*Win2Body(win))->teLength);
			HUnlock((*Win2Body(win))->hText);
		}
		
		UpdateMyWindow(newWin);
			
		/*
		 * make sure the last line is blank
		 */
		teh = (*newMessH)->txes[HEAD_LIMIT-1];
		nLines = (*teh)->nLines;
		if ((*teh)->lineStarts[nLines-1]-(*teh)->lineStarts[nLines-2]==1)
			nLines--;
		else
			AppendMessText(newMessH,HEAD_LIMIT-1,"\n",1);

		/*
			* quote them
			*/
		QuoteLines(newMessH,HEAD_LIMIT-1,0,nLines,FWD_QUOTE);
		if (win->qWindow.windowKind!=COMP_WIN)
			SetState((*origMessH)->tocH,(*origMessH)->sumNum,FORWARDED);
	  
		/*
		 * Attributions
		 */
		Attribute(FWD_INTRO,origMessH,newMessH,False);
		Attribute(FWD_TRAIL,origMessH,newMessH,True);

		/*
		 * copy priority (not my idea)
		 */
		if (!PrefIsSet(PREF_NO_XF_PRIOR))
			SumOf(newMessH)->origPriority = SumOf(newMessH)->priority =
				SumOf(origMessH)->origPriority;
		
		UpdateSum(newMessH,SumOf(newMessH)->offset,SumOf(newMessH)->length);
		ShowMyWindow(newWin);
		newWin->isDirty = False;
	}
	return(!newWin);
}

/************************************************************************
 * FindAndCopyHeader - pick the given header out of a message, and
 * copy it to a composition message
 ************************************************************************/
int FindAndCopyHeader(MessType **origMH,MessType **newMH,UPtr fromHead,short toHead)
{
	UPtr body;
	long size;
	int result;
	
	body = LDRef((*BodyOf(origMH))->hText);
	size = SumOf(origMH)->bodyOffset-(*origMH)->weeded;
	result = TextFindAndCopyHeader(body,size,newMH,fromHead,toHead);
	HUnlock((*BodyOf(origMH))->hText);
	return(result);
}

/************************************************************************
 * TextFindAndCopyHeader - pick the given header out of a text block, and
 * copy it to a composition message
 ************************************************************************/
int TextFindAndCopyHeader(UPtr body,long size,MessType **newMH,UPtr fromHead,short toHead)
{
	int oldNl;
	short oldBody;
	MyWindowPtr newWin = (*newMH)->win;
	UPtr bodyEnd = body+size;
	UPtr spot;
		
	if (body = FindHeaderString(body,fromHead,&size))
	{
		TEHandle teh = (*newMH)->txes[toHead-1];
		oldNl = CountTeLines(teh);
		
		for(;;)
		{
			oldBody = (*teh)->teLength;
			NoScrollTESetSelect(oldBody,oldBody,teh);
			if (oldBody)
			{
				for (spot=*(*teh)->hText+oldBody-1;spot>*(*teh)->hText;spot--)
					if (!IsWhite(*spot))
					{
						if (*spot!=',')
							TEInsert(", ",2,teh);
						else if (spot==*(*teh)->hText+oldBody-1)
							TEInsert(" ",1,teh);
						break;
					}
			}
			TEInsert(body,size,teh);
			
			body += size;
			while (IsWhite(*body)&&body<bodyEnd) body++;			/* skip to newline */
			body++; 																					/* skip newline */
			if (body<bodyEnd && IsWhite(*body)) 							/* continuation */
			{
				while (IsWhite(*body)&&body<bodyEnd) body++;
				for (spot=body;spot<bodyEnd&&*spot!='\n';spot++);
				do {spot--;} while (IsWhite(*spot));
				size = spot-body+1;
				if (!size) break;
			}
			else
				break;
		}
		
		INVAL_RECT(&(*teh)->destRect);
		FixMessTE(newMH,toHead-1,oldNl);
	}
	return(body!=nil);
}

/************************************************************************
 * XferCustomTable - transfer a custom table from a message to its reply
 * (or forward or redirect)
 ************************************************************************/
void XferCustomTable(MessHandle origMessH,MessHandle newMessH)
{
	short origTable;
	Boolean isOut;
	
	/*
	 * under the old regime, we don't do this step
	 */
	if (!NewTables) return;
	
	origTable = SumOf(origMessH)->tableId;

	/*
	 * if the original message had the default table, give the default table
	 * to the new message (which has already been done, so return)
	 */
	if (origTable == DEFAULT_TABLE) return;
	
	isOut = (SumOf(origMessH)->flags&FLAG_OUT)!=0;
	if (origTable == GetRLong(PREF_STRN+(isOut?PREF_OUT_XLATE:PREF_IN_XLATE)))
		return;
	
	/*
	 * if original was outgoing, use same table
	 * also use same table if that table is no table
	 */
	if (isOut || !origTable)
		SumOf(newMessH)->tableId = origTable;
		
	/*
	 * Otherwise, look for an out table that corresponds to the In table
	 * and use that.
	 */
	else if (GetResource('taBL',origTable+1))
		SumOf(newMessH)->tableId = origTable+1;
}
