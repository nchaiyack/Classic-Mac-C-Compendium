#define FILE_NUM 7
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Outgoing

/************************************************************************
 * private function declarations
 ************************************************************************/
TEHandle GetCompTexts(MessType **messH);
void MakeCompTitle(UPtr string, TOCType **tocH, int sumNum);
int WriteComp(MessType **messH, short refN, long offset);
int WriteCompHeader(short refN,MessType **messH,int which);
void SuckHeaderText(UPtr string,int size,TEHandle teh); 
void CompBecameVisible(MyWindowPtr win);

/**********************************************************************
 * OpenComp - open an outgoing message
 **********************************************************************/
MyWindowPtr OpenComp(TOCType **tocH, int sumNum, MyWindowPtr win, Boolean showIt)
{
	Str255 title;
	MessType **messH;
	ControlHandle grumble;
	
	CycleBalls();
	if ((messH = (MessType **)NewZHandle(sizeof(MessType)))==nil)
		return(nil);
		
	win = GetNewMyWindow(MESSAGE_WIND,win,InFront,False,True);
	if (!win)
	{
		DisposHandle(messH);
		return(nil);
	}
	((WindowPeek)win)->windowKind = COMP_WIN;
	
	SetPort(win);
	
	(*tocH)->sums[sumNum].messH = messH;
	MakeCompTitle(title,tocH,sumNum);
	
	(*messH)->win = win;
	(*messH)->sumNum = sumNum;
	(*messH)->tocH = tocH;
	
	((WindowPeek)win)->refCon = (long)messH;
	win->close = CompClose;
	win->scroll = CompScroll;
	win->didResize = CompDidResize;
	win->click = CompClick;
	win->menu = CompMenu;
	win->key = CompKey;
	win->button = CompButton;
	win->vPitch = FontLead;
	win->hPitch = FontWidth;
	win->position = MessagePosition;
	win->help = CompHelp;
	win->gonnaShow = CompBecameVisible;
	win->textChanged = CompHasChanged;
	win->zoomSize = CompZoomSize;

	LL_Push(MessList,messH);
	
	if (PrefIsSet(PREF_ICON_BAR))
	{
		(*tocH)->sums[sumNum].flags |= FLAG_ICON_BAR;
		win->topMargin = GetRLong(COMP_TOP_MARGIN);
		grumble = GetNewControl(SEND_NOW_CNTL,win);
		(*messH)->sendButton = grumble;
		SetGreyControl((*messH)->sendButton,(*tocH)->sums[sumNum].state==SENT);
	}
	else
		(*tocH)->sums[sumNum].flags &= ~FLAG_ICON_BAR;

	if (GetCompTexts(messH)==nil)
	{
		CloseMyWindow(win);
		return(nil);
	}
	
	TextFont(0);
	TextSize(0);
	win->update = CompUpdate;
	if (showIt)
		ShowMyWindow(win);
	InvalContent(win);
	SetWTitle(win,title);
	AttachSelect((*messH)->txes[ATTACH_HEAD-1]);
	
	UpdateMyWindow(win);
	
	return(win);
}

/************************************************************************
 * CompBecameVisible - select an appropriate field
 ************************************************************************/
void CompBecameVisible(MyWindowPtr win)
{
	MessHandle messH = Win2MessH(win);
	
	if (!(*(*messH)->txes[TO_HEAD-1])->teLength)
		CompActivateField(messH,TO_HEAD-1);
	else if (!(*(*messH)->txes[SUBJ_HEAD-1])->teLength) 
		CompActivateField(messH,SUBJ_HEAD-1);
	else
		CompActivateField(messH,HEAD_LIMIT-1);
	if ((*win->vBar)->contrlMax)
		ScrollIt(win,0,INFINITY);
	else
		HiliteControl(win->vBar,0);
}

/**********************************************************************
 * CountCompLines - count the number of lines in a message under
 * composition.
 **********************************************************************/
int CountCompLines(MessType **messH)
{
	int lines = 0;
	int h=0;
	
	for (h=0;h<HEAD_LIMIT;h++)
		if ((*messH)->txes[h] && CountTeLines((*messH)->txes[h]))
			lines += CountTeLines((*messH)->txes[h]);
		else
			lines++;
	
	return(lines);
}

/**********************************************************************
 * GetCompTexts - get the fields of an under-composition message
 * First, we read ALL the message into a buffer.	Then, we grab the
 * header items, stuff them one by one into appropriate TERec's.  After
 * the headers are safely tucked away, we move the body to the
 * beginning of the buffer, and make a TERec out of that.
 * This routine assumes that the out box has been created in a very
 * particular format; to wit:
 *		Sendmail-style from line
 *		To:
 *		From:
 *		Subject:
 *		Cc:
 *		Date:
 *		<blank line>
 *		body of message
 * The header items must NOT contain newlines, and MUST be presented in
 * the proper order.
 **********************************************************************/
TEHandle GetCompTexts(MessType **messH)
{
	MyWindowPtr win = (*messH)->win;
	int sumNum = (*messH)->sumNum;
	TOCType **tocH = (*messH)->tocH;
	UHandle buffer = nil;
	int which, err;
	Rect template;
	char *cp, *ep;
	TEHandle teh;
	
	/*
	 * allocate space for the text
	 */
	if ((buffer=NuHandle((*tocH)->sums[sumNum].length+1))==nil)
	{
		WarnUser(NO_MESS_BUF,MemError());
		return(nil);
	}
	
	/*
	 * read it
	 */
	LDRef(buffer);
	if (err=ReadMessage(tocH,sumNum,*buffer))
		goto failure;
	UL(buffer);
	
	/*
	 * now, set up the TERec's
	 */
	template = win->contR;			/* some size; it doesn't matter */
	InsetRect(&template,1,0);
	OffsetRect(&template,0,win->contR.bottom);
	for (which = 0; which < HEAD_LIMIT; which++)
	{
		if ((teh=TENew(&template,&template))==nil)
		{
			WarnUser(MESS_TE,MemError());
			goto failure;
		}
		(*messH)->txes[which] = teh;
		(*teh)->txFont = FontID;
		(*teh)->txSize = FontSize;
		(*teh)->lineHeight = FontLead;
		(*teh)->fontAscent = FontAscent;
		(*teh)->clikLoop = MyClikLoop;
		(*messH)->txRo[which] = (*tocH)->sums[sumNum].state==SENT;
	}
	(*messH)->txRo[ATTACH_HEAD-1] = True;
#ifndef JENNY
	(*messH)->txRo[FROM_HEAD-1] = True;
#endif	
	/*
	 * put in the text...
	 */
	cp = LDRef(buffer);
	cp[(*tocH)->sums[sumNum].length] = '\n';	 /* a sentinel */
	while (*cp++ != '\n');		/* skip sendmail from line */

	/*
	 * the headers
	 */
	CycleBalls();
	for (which=TO_HEAD; which < HEAD_LIMIT; which++)
	{
		teh = (*messH)->txes[which-1];
		if (cp-*buffer >= (*tocH)->sums[sumNum].length) goto failure;
		for (ep=cp; *ep!='\n'; ep++);
		if (ep-*buffer >= (*tocH)->sums[sumNum].length) goto failure;
		while (cp<ep && *cp++!=':');
		if (*cp == ' ') cp++;

		if (cp<ep)
			TESetText(cp,ep-cp,teh);
		else
			TESetText("",0,teh);
		cp = ep+1;
	}
	
	/*
	 * the body
	 */
	CycleBalls();
	ep+=2;	/* skip newline and header/body separator */
	teh = (*messH)->txes[HEAD_LIMIT-1];
	if (ep-*buffer < (*tocH)->sums[sumNum].length)
	{
		BlockMove(ep,*buffer,(*tocH)->sums[sumNum].length - (ep-*buffer));
		SetHandleBig(buffer,(*tocH)->sums[sumNum].length - (ep-*buffer));
		UL(buffer);
		DisposHandle((*teh)->hText);
		(*teh)->hText = buffer;
		buffer = nil;
		(*teh)->teLength = GetHandleSize((*teh)->hText);
	}
	else
		TESetText("",0,teh);
	(*messH)->txRo[ATTACH_HEAD-1] = 1;
	if (buffer) ZapHandle(buffer);

	/*
	 * do precise sizing
	 */
	MyWindowDidResize(win,&win->contR);

	return((*messH)->txes[TO_HEAD]);
	
failure:
	if (buffer) DisposHandle(buffer);
	for (which = 0; which < HEAD_LIMIT; which++)
		if ((*messH)->txes[which-1])
			TEDispose((*messH)->txes[which-1]);
	return(nil);

}

/**********************************************************************
 * MakeCompTitle - title a composition window
 **********************************************************************/
void MakeCompTitle(UPtr string, TOCType **tocH, int sumNum)
{
	Str255 scratch;

	*string = 0;
	PCat(string,* (*tocH)->sums[sumNum].from ?
				(*tocH)->sums[sumNum].from : GetRString(scratch,NO_TO));
	if (*(*tocH)->sums[sumNum].date)
	{
		PCat(string,"\p, ");
		PCat(string,(*tocH)->sums[sumNum].date);
	}
	PCat(string,"\p, ");
	PCat(string,* (*tocH)->sums[sumNum].subj ?
			 (*tocH)->sums[sumNum].subj : GetRString(scratch,NO_SUBJECT));
}

/**********************************************************************
 * FindCompTx - figure out which of the fields a given txe is.
 **********************************************************************/
int FindCompTx(MessType **messH, TEHandle txe)
{
	int tx;
	
	for (tx=0;tx<HEAD_LIMIT;tx++)
		if ((*messH)->txes[tx] == txe)
			return(tx);
	
	return (-1);
}

/************************************************************************
 * SaveComp - save a composition window
 ************************************************************************/
Boolean SaveComp(MyWindowPtr win)
{
	MessType **messH = (MessType **)((WindowPeek)win)->refCon;
	TOCType **tocH = (*messH)->tocH;
	long bytes = CountCompBytes(messH);
	long offset;
	int err;
	
	 offset = FindTOCSpot(tocH,bytes);
		
	if ((err=BoxFOpen(tocH)) || (err = WriteComp(messH,(*tocH)->refN,offset)))
	{
		FileSystemError(WRITE_MBOX,(*tocH)->name,err);
		return(False);
	}
		
	(*tocH)->sums[(*messH)->sumNum].offset = offset;
	(*tocH)->sums[(*messH)->sumNum].length = bytes;
	(*tocH)->sums[(*messH)->sumNum].flags &= ~FLAG_NBK;
	if ((*tocH)->refN) (void) SetEOF((*tocH)->refN,offset+bytes);
	UpdateSum(messH, offset, bytes);
	(*tocH)->dirty = True;
	win->isDirty = False;
	return(True);
}
	
/************************************************************************
 * CountCompBytes - count up the bytes in a message under composition
 ************************************************************************/
long CountCompBytes(MessType **messH)
{
	int which;
	long bytes = 0;
	Str255 scratch;
	TEHandle teh;
	int thisOne;
	
	/*
	 * headers
	 */
	for (which=0;which<HEAD_LIMIT-1;which++)
	{
		teh = (*messH)->txes[which];
		thisOne = (*teh)->teLength;
		if ((*(*teh)->hText)[thisOne-1] != '\n') thisOne++;
		bytes += thisOne;
		GetRString(scratch,HEADER_STRN+which+1);
		bytes += *scratch + 1;
	}
	
	/*
	 * body
	 */
	teh = (*messH)->txes[HEAD_LIMIT-1];
	thisOne = (*teh)->teLength;
	if ((*(*teh)->hText)[thisOne-1] != '\n') thisOne++;
	bytes += thisOne;
	
	bytes += 1; /* a blank line */

	SumToFrom(nil,scratch);
	bytes += strlen(scratch);
	return (bytes);
}

/************************************************************************
 * WriteComp - write a comp message to a particular spot in a particular
 * file.
 ************************************************************************/
int WriteComp(MessType **messH, short refN, long offset)
{
	int which;
	long count;
	UHandle text;
	int err;
	Str255 fromLine;
	long spot;
	
	/*
	 * sendmail-style From line
	 */
	CycleBalls();
	SetFPos(refN, fsFromStart, offset);
	SumToFrom(nil,fromLine);
	count = strlen(fromLine);
	if (err=FSWrite(refN,&count,fromLine)) return(err);
	
	/*
	 * headers
	 */
	for (which=0;which<HEAD_LIMIT-1;which++)
		if (err = WriteCompHeader(refN,messH,which)) return(err);

	/*
	 * blank line
	 */
	count = 1;
	if (err = FSWrite(refN,&count,"\n")) return(err);
	
	/*
	 * body
	 */
	GetFPos(refN,&spot);
	(*(*messH)->tocH)->sums[(*messH)->sumNum].bodyOffset = spot-offset;
	(*(*messH)->tocH)->dirty = True;
	CycleBalls();
	text = (*(*messH)->txes[HEAD_LIMIT-1])->hText;
	count = (*(*messH)->txes[HEAD_LIMIT-1])->teLength;
	LDRef(text);
	err = FSWrite(refN,&count,*text);
	UL(text);
	if (err) return(err);
	if ((*text)[count-1] != '\n')
	{
		count = 1;
		err = FSWrite(refN,&count,"\n");
	}
	return(err);
}

/************************************************************************
 * UpdateSum - stick values from comp message into sum
 ************************************************************************/
void UpdateSum(MessType **messH, long offset, long length)
{
	TOCType **tocH = (*messH)->tocH;
	int sumNum = (*messH)->sumNum;
	MSumType *sum = (*tocH)->sums + sumNum;

	LDRef(tocH);
	SuckHeaderText(sum->from,sizeof(sum->from),(*messH)->txes[TO_HEAD-1]);
	if (!*sum->from)
		SuckHeaderText(sum->from,sizeof(sum->from),(*messH)->txes[BCC_HEAD-1]); 
	SuckHeaderText(sum->subj,sizeof(sum->subj),(*messH)->txes[SUBJ_HEAD-1]);
	sum->offset = offset;
	sum->length = length;
	if (*sum->from)
	{
		if (sum->state==UNSENDABLE) SetState(tocH,sumNum,SENDABLE);
	}
	else if (sum->state != UNSENDABLE)
	{
		SetState(tocH,sumNum,UNSENDABLE);
	}
	BeautifySum(sum);
	/* calculate fromTrunc */
	
	/*
	 * retitle window
	 */
	if ((*messH)->win)
	{
		Str255 scratch;
		MakeCompTitle(scratch,tocH,(*messH)->sumNum);
		SetWTitle((*messH)->win,scratch);
	}
	
	/*
	 * finally, invalidate the line in the toc
	 */
	CalcSumLengths(tocH,(*messH)->sumNum);
	InvalSum(tocH,(*messH)->sumNum);
	UL(tocH); 
}

/************************************************************************
 * WriteCompHeader - write a particular header to a file
 ************************************************************************/
int WriteCompHeader(short refN,MessType **messH,int which)
{
	char *bp, *ep;
	register char *cp;
	UHandle text;
	Boolean anyTabs = False;
	Str255 name;
	int err;
	long count;
	
	/*
	 * write out the name of the header
	 */
	GetRString(name,HEADER_STRN+which+1);
	PCatC(name,' ');
	count = *name;
	if (err=FSWrite(refN,&count,name+1)) return(err);
	
	/*
	 * set up for the header text
	 */
	text = (*(*messH)->txes[which])->hText;
	bp = LDRef(text);
	ep = bp + (*(*messH)->txes[which])->teLength;
	
	/*
	 * walk through text, replacing \n with \t
	 */
	for (cp=bp; cp<ep-1; cp++) if (*cp == '\n') {*cp = '\t'; anyTabs=True;};
	
	/*
	 * write it out
	 */
	count = ep-bp;
	err = FSWrite(refN,&count,bp);
	if (!err && (!count || *cp!='\n'))
	{
		count = 1;
		err = FSWrite(refN,&count,"\n");
	}
	
	/*
	 * replace the \t's with \n's
	 */
	if (anyTabs) for (cp=bp; cp<ep; cp++) if (*cp == '\t') *cp = '\n';
	
	UL(text);
	return(err);
}

/************************************************************************
 * SuckHeaderText - get the text of a header for insertion into a summary
 ************************************************************************/
void SuckHeaderText(UPtr string,int size,TEHandle teh)
{
	register char *cp;
	int bytes = (*teh)->teLength;
	
	/*
	 * grab the text
	 */
	if (bytes>size-2) bytes = size-2;
	strncpy(string+1,*(*teh)->hText,bytes);
	string[bytes+1] = 0;
	string[0] = bytes;
	
	/*
	 * change newlines to spaces
	 */
	for (cp=string+1;*cp;cp++) if (*cp=='\n') *cp = ' ';
	
	TrimWhite(string);
}

/**********************************************************************
 * QueueSelectedMessages - queue all selected messages
 **********************************************************************/
int QueueSelectedMessages(TOCType **tocH,short toState,uLong when)
{
	int sumNum;
	int err = 0;
	long zs = ZoneSecs();
	
	for (sumNum = 0; sumNum < (*tocH)->count; sumNum++)
	{
		if ((*tocH)->sums[sumNum].selected)
		{
    	TimeStamp(tocH,sumNum,when,zs);
			if (toState==QUEUED)
			{
				if (*(*tocH)->sums[sumNum].from)
				{
					if ((*tocH)->sums[sumNum].state!=SENT) SetState(tocH,sumNum,QUEUED);
				}
				else
					err = 1;
			}
			else if ((*tocH)->sums[sumNum].state!=SENT)
			{
				SetState(tocH,sumNum,*(*tocH)->sums[sumNum].from? SENDABLE:UNSENDABLE);
			}
		}
	}
done:
	if (err)
		DoNumBigAlert(Stop,CANT_QUEUE);
	SetSendQueue();
	return(err);
}
/************************************************************************
 * CreateMessageBody - put a blank message body into a buffer
 ************************************************************************/
void CreateMessageBody(UPtr buffer)
{
	Str255 s;
	char *ep;
	
	ep = buffer;
	
	/*
	 * from line
	 */
	strcpy(ep,"From "); ep+= 5;
	GetReturnAddr(s,False);strncpy(ep,s+1,*s);ep += *s;*ep++ = ' ';
	LocalDateTimeStr(ep); p2cstr(ep);
	ep += strlen(ep);
	
	/*
	 * headers
	 */
	GetRString(s,HEADER_STRN+TO_HEAD);strncpy(ep,s+1,*s);ep+= *s;*ep++ ='\n';
	GetRString(s,HEADER_STRN+FROM_HEAD);strncpy(ep,s+1,*s);ep+= *s;*ep++ =' ';
	GetReturnAddr(s,True);strncpy(ep,s+1,*s);ep+= *s;*ep++ ='\n';
	GetRString(s,HEADER_STRN+CC_HEAD);strncpy(ep,s+1,*s);ep+= *s;*ep++ ='\n';
	GetRString(s,HEADER_STRN+BCC_HEAD);strncpy(ep,s+1,*s);ep+= *s;*ep++ ='\n';
		
	/*
	 * blank line and "body"
	 */
	strcpy(ep,"\n\n");	
} 
