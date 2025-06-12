#define FILE_NUM 29
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * handling the Ph panel
 **********************************************************************/
#pragma load EUDORA_LOAD
#pragma segment Ph

/************************************************************************
 * the structure for the ph window
 ************************************************************************/
typedef struct
{
	STEHandle qSTE;
	STEHandle aSTE;
	Rect qRect;
	Rect aRect;
	ControlHandle okButton;
	ControlHandle fingerButton;
	Str63 label;
	short fallback;
	MyWindowPtr win;
} PhType, *PhPtr, **PhHandle;
#define QSTE (*PhGlobals)->qSTE
#define ASTE (*PhGlobals)->aSTE
#define Win (*PhGlobals)->win
#define QRect (*PhGlobals)->qRect
#define ARect (*PhGlobals)->aRect
#define Fallback (*PhGlobals)->fallback
#define OkButton (*PhGlobals)->okButton
#define FingerButton (*PhGlobals)->fingerButton
#define Win (*PhGlobals)->win
#define Label (*PhGlobals)->label
#define QText (*(*QSTE)->te)->hText
#define AText (*(*ASTE)->te)->hText
#define QLen (*(*QSTE)->te)->teLength
#define ALen (*(*ASTE)->te)->teLength
PhHandle PhGlobals; 		/* for the ph window */
#define IsFinger (!UseCTB&&PrefIsSet(PREF_FINGER))

/************************************************************************
 * private functions
 ************************************************************************/
	void PhClick(MyWindowPtr win, EventRecord *event);
	void PhKey(MyWindowPtr win, EventRecord *event);
	Boolean PhMenu(MyWindowPtr win, int menu, int item, short modifiers);
	void PhUpdate(MyWindowPtr win);
	void PhCursor(Point mouse);
	void PhDidResize(MyWindowPtr win, Rect *oldContR);
	void PerformQuery(Boolean isPh);
	void PhButton(MyWindowPtr win,ControlHandle button,long modifiers,short part);
	Boolean PhClose(MyWindowPtr win);
	int QiLine(short *ordinal, UPtr text, long *size);
	short ContactQi(void);
	short SendQiCommand(UPtr cmd);
	short GetQiResponse(void);
	void CloseQi(void);
	void PhSetSTE(STEHandle ste);
	void DoFinger(UPtr cmd);
	void PhPosition(Boolean save, MyWindowPtr win);
	Boolean PhApp1(MyWindowPtr win, EventRecord *event);
	void PhHelp(MyWindowPtr win,Point mouse);
	void IncrementPhServer(UPtr serverName);
	void MakeFallbackName(UPtr serverName, short fallback);
	void NotePhServer(UPtr serverName);
	void PhActivate(void);

/************************************************************************
 * OpenPh - open the ph window
 ************************************************************************/
void OpenPh(void)
{
	Rect r;
	
	CycleBalls();
	if (PhGlobals)
	{
		SelectWindow(Win);
		return;
	}
	if (!(PhGlobals=NewZH(PhType)))
		{WarnUser(MEM_ERR,MemError()); goto fail;}
		
	LDRef(PhGlobals);

	if (!(Win = GetNewMyWindow(PH_WIND,nil,InFront,False,False)))
		{WarnUser(MEM_ERR,MemError()); goto fail;}
	if (!(OkButton = GetNewControl(PH_CNTL,Win)))
		{WarnUser(MEM_ERR,MemError()); goto fail;}
	if (!(FingerButton = GetNewControl(FINGER_CNTL,Win)))
		{WarnUser(MEM_ERR,MemError()); goto fail;}
	SetPort(Win);
	TextFont(FontID);
	TextSize(FontSize);
	r = Win->contR;
	if (!(QSTE = NewSTE(Win,&r,False,False,False)))
		{WarnUser(MEM_ERR,MemError()); goto fail;}
	if (!(ASTE = NewSTE(Win,&r,False,True,False)))
		{WarnUser(MEM_ERR,MemError()); goto fail;}
	UL(PhGlobals);
	NewPhHost();
	Win->close = PhClose;
	Win->activate = PhActivate;
	Win->click = PhClick;
	Win->cursor = PhCursor;
	Win->menu = PhMenu;
	Win->key = PhKey;
	Win->button = PhButton;
	PhSetSTE(QSTE);
	Win->position = PhPosition;
	Win->dontControl = True;
	Win->vPitch = FontLead;
	Win->qWindow.windowKind = PH_WIN;
	Win->update = PhUpdate;
	Win->app1 = PhApp1;
	Win->help = PhHelp;
	Win->minSize.h = 300;
	Win->minSize.v = 8*FontLead;
	Win->didResize = PhDidResize;
	MyWindowDidResize(Win,&r);
	ShowMyWindow(Win);
	ShowControl(OkButton);
	ShowControl(FingerButton);
	return;

fail:
	PhClose(Win);
	return;
}


/************************************************************************
 * 
 ************************************************************************/
void PhActivate(void)
{
	OutlineControl(IsFinger ? FingerButton : OkButton,!InBG&&Win->isActive);
	(*FingerButton)->contrlRfCon = UseCTB ? 'GREY' : 0;
}

/************************************************************************
 * PhPosition - ph window position
 ************************************************************************/
void PhPosition(Boolean save, MyWindowPtr win)
{
	Str31 ph;
	MyGetItem(GetMHandle(SPECIAL_MENU),SPECIAL_PH_ITEM,ph);
	SetWTitle(Win,ph);
	PositionPrefsTitle(save,win);
}

/************************************************************************
 * PhButton - handle a button click in the ph window
 ************************************************************************/
void PhButton(MyWindowPtr win,ControlHandle button,long modifiers,short part)
{
#pragma unused(win,modifiers)
	if (part==inButton) PerformQuery(button==OkButton);
	Win->hasSelection = MyWinHasSelection(win);
}

/************************************************************************
 * PhClick - handle a click in the ph window
 ************************************************************************/
void PhClick(MyWindowPtr win, EventRecord *event)
{
	Rect r;
	Point mouse = event->where;
	TEHandle newTeh=nil;
	
	GlobalToLocal(&mouse);
	
	r = QRect;
	if (PtInRect(mouse,&r))
	{
		PhSetSTE(QSTE);
		STEClick(QSTE,event);
	}
	else
	{
		r = ARect;
		if (PtInRect(mouse,&r))
		{
			PhSetSTE(ASTE);
			STEClick(ASTE,event);
	  }
		else HandleControl(mouse,win);
	}
	Win->hasSelection = MyWinHasSelection(win);
}

/************************************************************************
 * PhApp1 - handle an app1 event
 ************************************************************************/
Boolean PhApp1(MyWindowPtr win, EventRecord *event)
{
#pragma unused(win)
	STEApp1(ASTE,event);
	return(True);
}

/************************************************************************
 * PhKey - keystrokes in the ph window
 ************************************************************************/
void PhKey(MyWindowPtr win, EventRecord *event)
{
	short key = event->message & 0xff;

	if (event->modifiers & cmdKey) {SysBeep(20L); return;}				/* no command keys! */
	if (key==returnChar || key==enterChar)
	{
		Undo.didClick = True;
		PerformQuery(!IsFinger);
		Win->hasSelection = MyWinHasSelection(win);
		return;
	}
	else if (key==tabChar)
	{
		PhSetSTE(win->ste==QSTE ? ASTE:QSTE);
		NoScrollTESetSelect(0,INFINITY,(*(STEHandle)win->ste)->te);
	}
	else if (win->ro)
	{
		PhSetSTE(QSTE);
		NoScrollTESetSelect(0,INFINITY,(*QSTE)->te);
	}
	
	if (key!=tabChar) TESomething(win,TEKEY,key,event->modifiers);

	Win->hasSelection = MyWinHasSelection(win);
}

/************************************************************************
 * PhMenu - menu selections in the ph window
 ************************************************************************/
Boolean PhMenu(MyWindowPtr win, int menu, int item, short modifiers)
{
#pragma unused(modifiers)
	
	switch (menu)
	{
		case FILE_MENU:
			switch(item)
			{
				case FILE_PRINT_ITEM:
				case FILE_PRINT_SELECT_ITEM:
				{
					win->txe = (*ASTE)->te;
					PrintOneMessage(win,item==FILE_PRINT_SELECT_ITEM);
					win->txe = nil;
					return(True);
				}
			}
			break;
		case EDIT_MENU:
			return(TESomething(win,item,0,modifiers));
			break;
	}
	return(False);
}

/************************************************************************
 * PhUpdate - update the ph window
 ************************************************************************/
void PhUpdate(MyWindowPtr win)
{
#pragma unused(win)
	Rect r;
	Str63 scratch;

	STEUpdate(QSTE);
	STEUpdate(ASTE);
	
	r = QRect;
	MoveTo(r.left,r.top - 6);
	PCopy(scratch,Label);
	DrawString(scratch);
		
	/*
	 * outline the button
	 */
	OutlineControl(IsFinger ? FingerButton : OkButton,!InBG&&win->isActive);
}

/************************************************************************
 * PhCursor - set the cursor for the ph window
 ************************************************************************/
void PhCursor(Point mouse)
{
#pragma unused(mouse)
	if (!STECursor(QSTE) && !STECursor(ASTE))
	{
		SetMyCursor(arrowCursor);
	}
}	

/************************************************************************
 * PhDidResize - handle resizing of the ph window
 ************************************************************************/
void PhDidResize(MyWindowPtr win, Rect *oldContR)
{
#pragma unused(win,oldContR)
	Rect r;
	Point p;
	short hi = (*OkButton)->contrlRect.bottom - (*OkButton)->contrlRect.top;
	short wi = (*OkButton)->contrlRect.right - (*OkButton)->contrlRect.left;
	
	/*
	 * put the Do It button where it belongs
	 */
	p.h = Win->contR.right - GROW_SIZE - wi;
	p.v = Win->contR.top + GROW_SIZE/2;
	OFFSET_RECT(&(*FingerButton)->contrlRect,p.h-(*FingerButton)->contrlRect.left,
							p.v-(*FingerButton)->contrlRect.top);
	p.v += GROW_SIZE/2 + hi;
	OFFSET_RECT(&(*OkButton)->contrlRect,p.h-(*OkButton)->contrlRect.left,
							p.v-(*OkButton)->contrlRect.top);

	/*
	 * calculate the frame for the command box
	 */
	r = Win->contR;
	r.bottom = (*OkButton)->contrlRect.bottom;
	r.top = r.bottom - FontLead - FontDescent - TE_VMARGIN*2;
	OffsetRect(&r,0,-(hi-r.bottom+r.top)/2);
	r.left += GROW_SIZE;
	r.right = (*OkButton)->contrlRect.left - GROW_SIZE/2;
	QRect = r;
	
	/*
	 * calculate the frame for the answer box
	 */
	r.top = (*OkButton)->contrlRect.bottom + GROW_SIZE/2;
	r.bottom = win->contR.bottom - GROW_SIZE+1;
	r.right = win->contR.right - GROW_SIZE+1;
	ARect = r;
	
	/*
	 * now for the TE's
	 */
	r = QRect;
	ResizeSTE(QSTE,&r);
	r = ARect;
	ResizeSTE(ASTE,&r);
	InvalRect(&win->contR);
}

#pragma segment Main
/************************************************************************
 * PhClose - close the ph window
 ************************************************************************/
Boolean PhClose(MyWindowPtr win)
{
#pragma unused(win)
	if (PhGlobals)
	{
		if (QSTE) STEDispose(QSTE);
		if (ASTE) STEDispose(ASTE);
		DisposHandle(PhGlobals);
		PhGlobals = nil;
	}
	return(True);
}
#pragma segment Ph

/************************************************************************
 * PerformQuery - perform a ph query
 ************************************************************************/
void PerformQuery(Boolean isPh)
{
	Str255 cmd;
	Str255 ph;
	Handle table=nil;
	short inId,outId;
	
	if (isPh == IsFinger)
	{
	  Rect r;
		ChangeStrn(PREF_STRN,PREF_FINGER,isPh?"\pn":"\py");
		r = (*FingerButton)->contrlRect;
		r.bottom = (*OkButton)->contrlRect.bottom;
		InsetRect(&r,-4,-4);
		InvalRect(&r);
	}
	NoScrollTESetSelect(0,INFINITY,(*QSTE)->te);
	PhSetSTE(QSTE);
	if (isPh)
		MyGetItem(GetMHandle(SPECIAL_MENU),SPECIAL_PH_ITEM,ph);
	else
		GetRString(ph,FINGER);
	if (!QLen)
	{
		SetWTitle(Win,ph);
		return;
	}
	MakePStr(cmd,*QText,QLen);
	PCatC(ph,':');
	PCat(ph,cmd);
	SetWTitle(Win,ph);
	STESetText("",0,ASTE);
	
	/*
	 * install translation tables
	 */
	if (NewTables)
	{
		 inId = GetRLong(PREF_STRN+PREF_IN_XLATE);
		 outId = GetRLong(PREF_STRN+PREF_OUT_XLATE);
	}
	else
	{
		inId = TRANS_IN_TABL;
		outId = TRANS_OUT_TABL;
	}
	if (inId && (table=GetResource('taBL',inId)))
	{
	  HNoPurge(table);
		if (TransIn = NuPtr(256)) BlockMove(*table,TransIn,256);
		HPurge(table);
	}
	if (outId && (table=GetResource('taBL',outId)))
	{
		HNoPurge(table);
		if (TransOut = NuPtr(256)) BlockMove(*table,TransOut,256);
		HPurge(table);
	}

	/*
	 * do it
	 */
	if (!UseCTB || !DialThePhone())
	{
		if (!isPh)
			DoFinger(cmd);
		else if (!ContactQi())
		{
			if (!SendQiCommand(cmd)) GetQiResponse();
			UpdateMyWindow(Win);
			CloseQi();
		}
		DestroyTrans();
	}
  if (UseCTB) HangUpThePhone();
	
	/*
	 * clean up tables
	 */
	if (TransIn) {DisposPtr(TransIn);TransIn=nil;}
	if (TransOut) {DisposPtr(TransOut);TransOut=nil;}
}

/************************************************************************
 * ContactQi - make contact with the QI server
 ************************************************************************/
short ContactQi(void)
{
	Str255 serverName;
	short port;
	short err;

	MakeFallbackName(serverName,Fallback);
	port = GetRLong(PH_PORT);
	err=ConnectTrans(serverName,port,!UseCTB);
	while (!UseCTB&&err==openFailed)
	{
		DestroyTrans();
		if (isdigit(serverName[1])) {Fallback++;break;}	/* don't fallback on ip addresses. */
		IncrementPhServer(serverName);
		err=ConnectTrans(serverName,port,True);
	}
	if (err&&Fallback)
	{
		Fallback = 0;
		SilenceTrans(False);
		NotePhServer(GetRString(serverName,NOONE));
	}
	else if (Fallback)
		NotePhServer(serverName);
	return(err);
}

/************************************************************************
 * NotePhServer - tell the user who we're talking to.
 ************************************************************************/
void NotePhServer(UPtr serverName)
{
	Str255 msg;
	ComposeRString(msg,PH_SUCCEED,serverName);
	STEAppendText(msg+1,*msg,ASTE);
}

/************************************************************************
 * IncrementPhServer - go on to the next ph server
 ************************************************************************/
void IncrementPhServer(UPtr serverName)
{
	Str255 fmt,msg,newServer;
	
	Fallback++;
	MakeFallbackName(newServer,Fallback);
	GetRString(fmt,PH_FAIL);
	utl_PlugParams(fmt,msg,serverName,newServer,nil,nil);
	STEAppendText(msg+1,*msg,ASTE);
	PCopy(serverName,newServer);
}

/************************************************************************
 * MakeFallbackName - add the appropriate numeral to a hostname
 ************************************************************************/
void MakeFallbackName(UPtr serverName, short fallback)
{
	char *spot;
	Str15 num;

	GetPref(serverName,PREF_PH);
	if (!*serverName) GetRString(serverName,PH_HOST);
	if (fallback)
	{
		NumToString(fallback,num);
		spot = strchr(serverName+1,'.');
		if (!spot) spot=serverName+*serverName+1;
		BlockMove(spot,spot+*num,*serverName-(spot-serverName)+2);
		BlockMove(num+1,spot,*num);
		*serverName += *num;
	}
}

/************************************************************************
 * SendQiCommand - send the current command to qi
 ************************************************************************/
short SendQiCommand(UPtr cmd)
{
	return(SendTrans(2,cmd+1,*cmd,"\012",1));
}

/************************************************************************
 * GetQiResponse - read a response from the nameserver
 ************************************************************************/
short GetQiResponse(void)
{
	UHandle text = NuHandle(nil);
	Str255 separator;
	Str255 buffer;
	long size;
	short ordinal = 0;
	short lastOrdinal = 0;
	short response=600;
	Boolean tooBig = False;
	
	/*
	 * do it
	 */
	if (text)
	{
		GetRString(separator,PH_SEPARATOR);
		do
		{
			size=sizeof(buffer);
			response = QiLine(&ordinal,buffer,&size);
			if (tooBig) continue;
			if (response==598)
			{
				Str63 query;
				GetRString(query,PH_QUERY);
				response = 0;
				if (SendTrans(3,query+1,*query,LDRef(QText),QLen,"\012",1))
					response = 600;
				UL(QText);
				continue;
			}
			if (ordinal != lastOrdinal)
			{
				(void) PtrAndHand(separator+1,text,*separator);
				lastOrdinal = ordinal;
			}
			if (response < 600)
				if (size+GetHandleSize(text)<32000)
					(void) PtrAndHand(buffer,text,size);
				else
				{
					tooBig = True;
					GetRString(buffer,PH_TOO_BIG);
					(void) PtrAndHand(buffer+1,text,*buffer);
				}
		}
		while(response<200);
		STEAppendText(LDRef(text),GetHandleSize(text),ASTE);
		ZapHandle(text);
	}
	
	if (text) DisposHandle(text);
	return(response);
}

/************************************************************************
 * CloseQi - close the qi connection
 ************************************************************************/
void CloseQi(void)
{
	Str255 buffer;
	long size;
	Str63 quitCmd;
		
	GetRString(quitCmd,PH_QUIT);
	/*
	 * do it
	 */
	if (buffer && !SendTrans(2,quitCmd+1,*quitCmd,"\012",1))
	{
		for (size=sizeof(buffer);
				 QiLine(nil,buffer,&size)<200;
				 size=sizeof(buffer))
			;
	}
	
	DisTrans();
}

/************************************************************************
 * QiLine - read a line from QI
 ************************************************************************/
int QiLine(short *ordinal, UPtr text, long *size)
{
	short result;
	UPtr spot;
	short ord;
	
	if (RecvLine(text,size)) return(600);
	result = atoi(text);
	if (!result)
	{
		*size = 0;
		return(100);
	}
	for (spot=text;*spot!=':';spot++);
	ord = 0;
	for (spot++;*spot>='0' && *spot <= '9';spot++)
		ord = ord * 10 + *spot - '0';
	if (ord)
	{
		*size -= spot-text+1;
		BlockMove(spot+1,text,*size);
	}
	if (ordinal) *ordinal = ord;
	return(result);
}

/************************************************************************
 * PhSetSTE - set the text area
 ************************************************************************/
void PhSetSTE(STEHandle ste)
{
	if (Win->ste == ste) return;
	if (Win->ste) {TEDeactivate((*(STEHandle)Win->ste)->te);}
	Win->ste = ste;
	TEActivate((*ste)->te);
	Win->ro = ste==ASTE;
}

/************************************************************************
 * PhFixFont - fix the font in the Ph window
 ************************************************************************/
void PhFixFont(void)
{
	TEFixup((*ASTE)->te);
	TEFixup((*QSTE)->te);
}

/************************************************************************
 * NewPhHost - install a new ph host
 ************************************************************************/
void NewPhHost(void)
{
	Str63 scratch;
	Str127 newLabel;
	if (PhGlobals)
	{
		GetPref(scratch,PREF_PH);
		if (!*scratch) GetRString(scratch,PH_HOST);
		ComposeRString(newLabel,PH_LABEL,scratch);
		PCopy(Label,newLabel);
		InvalContent(Win);
		Fallback = 0;
	}
}


/************************************************************************
 * DoFinger - do the finger 'protocol'
 ************************************************************************/
void DoFinger(UPtr cmd)
{
	UPtr atSign = strchr(cmd+1,'@');
	short uLen;
	Str255 buffer;
	long size;
	Boolean gotSome=False;
  
	if (atSign)
	{
		uLen = atSign-cmd-1;
		MakePStr(buffer,atSign+1,*cmd - uLen - 1);
		buffer[*buffer+1] = 0;
	}
	else
	{
		GetSMTPInfo(buffer);
		uLen = *cmd;
	}
	
	if (!ConnectTrans(buffer,GetRLong(FINGER_PORT),False))
	{
		if (!SendTrans(2,uLen?cmd+1:" ",uLen?uLen:1,"\n\r",2))
		{
			SilenceTrans(True);
			for (size=sizeof(buffer)-2;!NetRecvLine(buffer,&size);size=sizeof(buffer)-2)
			{
				NoScrollTESetSelect(INFINITY,INFINITY,(*ASTE)->te);
				TEInsert(buffer,size,(*ASTE)->te); SFWTC = True;
			}
			NoScrollTESetSelect(0,0,(*ASTE)->te);
			STETextChanged(ASTE);
		}
		DisTrans();
	}
}

#pragma segment Transport
/************************************************************************
 * RecvLine - get some text from the remote host.
 ************************************************************************/
int NetRecvLine(UPtr line,long *size)
{
	long count;
	UPtr anchor, end;
	short err;
	static int RcvBufferSize;
	long bSize = *size;
	Byte c;
	
	*size = 0;
	if (!RcvBufferSize) RcvBufferSize = GetHandleSize(RcvBuffer);

	anchor = line;
	end = line+bSize-1;
	while (anchor < end) 										/* need more chars */
		if (RcvSpot>=0) 											/* there are some buffered chars */
		{
			UPtr rPtr = *RcvBuffer+RcvSpot;
			if (!DontTranslate && TransIn)
			{
				for(c=*rPtr++;anchor<end;c=*rPtr++)
					if (c && c!='\n')
					{
				 		*anchor++ = TransIn[c];
						if (c=='\r')
						{
							anchor[-1] = '\n';
							break;
						}
					}
			}
			else
			{
				for(c=*rPtr++;anchor<end;c=*rPtr++)
					if (c && c!='\n')
					{
				 		*anchor++ = c;
						if (c=='\r')
						{
							anchor[-1] = '\n';
							break;
						}
					}
			}
			RcvSpot = rPtr - *RcvBuffer;
			if (RcvSpot>RcvSize)									/* newline was sentinel */
				anchor--;
			if (RcvSpot>=RcvSize)									/* we have emptied the buffer */
				RcvSpot = -1;
			if (anchor[-1]=='\n' || anchor>=end)	/* found a valid newline, or filled buffer */
			{
				*size = anchor-line;
				*anchor = 0;										/* terminate */
				return(noErr);
			}
		}
		else
		{
			count = RcvBufferSize-1;
			if (err=RecvTrans(LDRef(RcvBuffer),&count)) 	/* get some chars */
				 count=0;
			UL(RcvBuffer);
			if (count)
			{
				(*RcvBuffer)[count] = '\r'; 											/* sentinel */
				RcvSize = count;
				RcvSpot = 0;
			}
			if (err)
			{
				*size = anchor-line;
				line[*size] = 0;
				return(err);
			}
		}
	*size = anchor - line;
	if (*size) line[*size] = 0; 					/* null terminate */
	return(*size ? noErr : err);
}


#pragma segment Balloon
/************************************************************************
 * PhHelp - do balloon help for the ph window
 ************************************************************************/
void PhHelp(MyWindowPtr win,Point mouse)
{
#pragma unused(win)
	Rect r;
	short hnum=UseCTB?9:(PrefIsSet(PREF_FINGER)?5:1);
	
	r = QRect;
	if (PtInRect(mouse,&r))
		;
	else
	{
		hnum++;
		r = ARect;
		if (PtInRect(mouse,&r))
			;
		else
		{
			hnum++;
			r = (*OkButton)->contrlRect;
			if (PtInRect(mouse,&r))
				;
			else
			{
				hnum++;
				r = (*FingerButton)->contrlRect;
				if (PtInRect(mouse,&r))
					;
				else hnum=100;	/* off the end of the scale */
			}
		}
	}
	HelpRect(&r,PHWIN_HELP_STRN+hnum,100);
}