#define FILE_NUM 15
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Misc

Boolean CloseRemaining(WindowPeek win);
short ReallyChangePassword(UPtr user,UPtr host,UPtr old,UPtr new);
short SendPwCommand(short cmd,UPtr arg);
short CheckResult(UPtr buffer);
/************************************************************************
 * DoSendQueue - send queued messages, including setup
 ************************************************************************/
short DoSendQueue(void)
{
	short err=0;
#ifdef POPSECURE
	Str255 accnt;

	GetPref(accnt,PREF_POP);
	if (!*Password && GetPassword(accnt,Password,sizeof(Password),ENTER))
		return(1);
#endif

	AlertsTimeout = PrefIsSet(PREF_AUTO_DISMISS);
	OpenProgress();
#ifdef POPSECURE
	if (UseCTB && (!UUPCOut || !UUPCIn)) err=DialThePhone();
#else
	if (!UUPCOut && UseCTB) err=DialThePhone();
#endif
	if (!err) err=SendTheQueue();
	if (UseCTB) HangUpThePhone();
	CloseProgress();
	AlertsTimeout = False;
	SetSendQueue();
	return(err);
}

/************************************************************************
 * SendTheQueue - send queued messages, assuming cnxn is setup
 ************************************************************************/
short SendTheQueue(void)
{
	TOCType **tocH=nil;
	int sumNum = -1;
	Str255 server;
	int port;
	MessType **messH;
	Handle table;
	short err,code=0;
	short tableId;
	long gmtSecs = GMTDateTime();
	short defltTableId;
	UPtr tablePtr=NuPtr(256);
	short lastId = 0;
	
	defltTableId = GetRLong(PREF_STRN+PREF_OUT_XLATE);
	if (!NewTables || defltTableId==DEFAULT_TABLE) defltTableId = TRANS_OUT_TABL;

	GetSMTPInfo(server);
	port = GetRLong(SMTP_PORT);
	
#ifdef POPSECURE
	if (!POPSecure && (err=VetPOP())) goto done;
#endif
	ComposeLogR(LOG_SEND,nil,START_SEND_LOG,server,port);
	if (err=StartSMTP(server,port)) goto done;

	if (!(tocH=GetOutTOC())) goto done;

	if (!NewTables && !TransOut && (table=GetResource('taBL',TRANS_OUT_TABL)))
	{
		BlockMove(*table,tablePtr,256);
		TransOut = tablePtr;
		lastId = TRANS_OUT_TABL;
	}
	for (sumNum=0; sumNum<(*tocH)->count && code<600; sumNum++)
		if ((*tocH)->sums[sumNum].state == QUEUED &&
		    (*tocH)->sums[sumNum].seconds<=gmtSecs)
		{
		  TimeStamp(tocH,sumNum,0,0);
			
			/*
			 * ready a translation table, if needed
			 */
			tableId = (*tocH)->sums[sumNum].tableId;
			if (tableId==DEFAULT_TABLE) tableId=defltTableId;
			if (tableId != lastId)
			{
				if (tableId!=NO_TABLE && tablePtr && (table = GetResource('taBL',tableId)))
				{
					BlockMove(*table,tablePtr,256);
					TransOut = tablePtr;
					lastId = tableId;		/* so we don't have to fetch it next time */
				}
				else
					TransOut = nil;	/* no table */
			}
			
			/*
			 * actually send the message
			 */
			if (!(code=(UUPCOut ? UUPCSendMessage(tocH,sumNum) :
														SendMessage(tocH,sumNum))))
			{
				messH = (*tocH)->sums[sumNum].messH;
				if (messH && (*messH)->win)
					CloseMyWindow((*messH)->win);
#ifdef DEBUG
if (BUG4)
{
	TimeStamp(tocH,sumNum,0,0);
}
else
{
#endif
				SetState(tocH,sumNum,SENT);
				if (((*tocH)->sums[sumNum].flags&FLAG_KEEP_COPY)==0)
				{
					DeleteMessage(tocH,sumNum);
					sumNum--; 		/* back up, since we deleted the message */
				}
#ifdef DEBUG
}
#endif
			}
			else if (code==550)
				(*tocH)->sums[sumNum].flags |= FLAG_ADDRERR;
		}
done:
	if (tablePtr) DisposPtr(tablePtr);
	TransOut = nil;
	(void) EndSMTP();
	if (tocH && (*tocH)->win && sumNum>=0)
		BoxSelectAfter((*tocH)->win,sumNum);
		
	if (!AmQuitting && tocH)
	{
		SendQueue = 0;
		for (sumNum=(*tocH)->count-1; sumNum>=0; sumNum--)
		{
			if ((*tocH)->sums[sumNum].flags&FLAG_ADDRERR)
			{
				(*tocH)->sums[sumNum].flags &= ~FLAG_ADDRERR;
				if (!GetAMessage(tocH,sumNum,nil,True)) break;
			}
		}
	}
	SetSendQueue();
	FlushTOCs(True,False);	/* save memory, in case Out and Trash are large,
														 and we're going on to do a check */
	return(err);
}

/**********************************************************************
 * DoQuit - let's get outta here
 **********************************************************************/
void DoQuit()
{
  uLong never = 3600*GetRLong(NEVER_WARN);
	Str31 hours;
	uLong now = GMTDateTime();
	Boolean remember = !EjectBuckaroo;
	
	Done = EjectBuckaroo = False;
	AmQuitting = True;
	if (ForceSend > now && ForceSend - now < never)
	{
		GetRString(hours,NEVER_WARN);
	  switch (AlertStr(QUIT_MQ_ALRT,Stop,hours))
		{
		  case QMQ_SEND_ALL:
				WarpQueue(0);
				DoSendQueue();
				break;
			case QMQ_SEND:
				WarpQueue(never);
			  DoSendQueue();
				break;
			case QMQ_DONT:
			  break;
			default:
				AmQuitting = False;
				return;
		}
	}
	if (SendQueue)
	{
		switch (ReallyDoAnAlert(QUIT_QUEUE_ALRT,Stop))
		{
			case QQL_SEND:
				DoSendQueue();
				break;
			case QQL_QUIT:
				break;
			default:
				AmQuitting = False;
				return;
		}
	}
	if (remember) RememberOpenWindows();
	if (!CloseAll()) {AmQuitting=False; return;}
	
	Done = True;
}

/************************************************************************
 * CloseAll - close all windows
 ************************************************************************/
Boolean CloseAll(void)
{
	if (!CloseAllMessages()) return(False);
	if (!CloseAllBoxes()) return(False);
	if (!CloseRemaining(FrontWindow())) return(False);
}

/************************************************************************
 * CloseRemaining - get rid of any other windows there might be
 ************************************************************************/
Boolean CloseRemaining(WindowPeek win)
{
	if (!win) return(True);
	if (!CloseRemaining(win->nextWindow)) return(False);
	if (IsMyWindow(win)) return(CloseMyWindow(win));
	else if (win->windowKind==dialogKind)
	{
		NukeUndo(win);
		DisposDialog(win);
	}
	return(True);
}

/**********************************************************************
 * DoAboutUIUCmail - put up the about box
 **********************************************************************/
void DoAboutUIUCmail()
{
	Str63 versionString;
	Str255 partition;
	VersRecHndl version;
	
	if (version = (VersRecHndl) GetResource('vers',1))
	{
		PCopy(versionString,(*version)->shortVersion+*(*version)->shortVersion+1);
		ReleaseResource(version);
	}
	else
		*versionString = 0;
	
	ComposeRString(partition,MEM_PARTITION,CurrentSize()/(1K),
												EstimatePartitionSize()/(1K));
		
	SetDAFont(GetFontID("\pPalatino"));
	MyParamText(versionString,partition,"","");
	(void) ReallyDoAnAlert(ABOUT_ALRT,Normal);
	SetDAFont(0);
}

/**********************************************************************
 * NotImplemented - tell the user a feature is not implemented
 **********************************************************************/
void NotImplemented(void)
{
	DoAnAlert(Stop,"\pSorry, I'm (still) too stupid to do that.");
}


/**********************************************************************
 * DoPageSetup - carry on the Page Setup dialog with the user
 **********************************************************************/
void DoPageSetup()
{
	short err;
	
	/*
	 * try to open the printer
	 */
	PrOpen();
	if (PrError())
	{
		WarnUser(NO_PRINTER,PrError());
		return;
	}
	
	/*
	 * make sure we have some sort of page setup record
	 */
	if (PageSetup==nil) PageSetup = GetResource(PRINT_RTYPE,PRINT_CSOp);
	if (PageSetup==nil)
	{
		PageSetup = NuHandle(sizeof(TPrint));
		if (PageSetup==nil)
		{
			WarnUser(COULDNT_SETUP,MemError());
			PrClose();
			return;
		}
		PrintDefault(PageSetup);
		
		/*
		 * save it in our prefs file
		 */
		if (err=SettingsHandle(PRINT_RTYPE,nil,PRINT_CSOp,PageSetup))
			WarnUser(SAVE_SETUP,err);
		else
			UpdateResFile(SettingsRefN);
	}

	/*
	 * let the user see it
	 */
	PushCursor(arrowCursor);
	if (PrStlDialog(PageSetup))
	{
		ChangedResource(PageSetup); 				/* save it in prefs file */
		UpdateResFile(SettingsRefN);
		if (ResError())
			WarnUser(SAVE_SETUP,ResError());
	}
	PopCursor();
	
	/*
	 * done
	 */
	PrClose();
}

/************************************************************************
 * DoComposeNew - start a new outgoing message
 ************************************************************************/
MyWindowPtr DoComposeNew(short toWhom)
{
	TOCType **tocH;
	MSumType sum;
	Byte buffer[1024];
	long len;
	short err;
	MyWindowPtr newWin;
	
	if (!(tocH=GetOutTOC())) DieWithError(FATAL,1);
	
	CreateMessageBody(buffer);
	len = strlen(buffer);
	
	WriteZero(&sum,sizeof(MSumType));
	sum.offset = FindTOCSpot(tocH,len);
	sum.length = len;
	sum.state = UNSENDABLE;
	sum.flags |= FLAG_NBK;
	sum.flags |= FLAG_OUT;
	sum.tableId = DEFAULT_TABLE;
	if (PrefIsSet(PREF_WRAP_OUT)) sum.flags |= FLAG_WRAP_OUT;
	if (PrefIsSet(PREF_SIG)) sum.flags |= FLAG_SIG;
	if (PrefIsSet(PREF_BX_TEXT)) sum.flags |= FLAG_BX_TEXT;
	if (PrefIsSet(PREF_KEEP_OUTGOING)) sum.flags |= FLAG_KEEP_COPY;
	if (PrefIsSet(PREF_TAB_IN_TEXT)) sum.flags |= FLAG_TABS;

	sum.seconds = 0;
	if (BoxFOpen(tocH)) return(nil);

	if (err=SetFPos((*tocH)->refN,fsFromStart,sum.offset))
	{
		FileSystemError(READ_MBOX,(*tocH)->name,err);
		return(nil);
	}
	
	if (err=FSWrite((*tocH)->refN,&len,buffer))
	{
		FileSystemError(WRITE_MBOX,(*tocH)->name,err);
		return(nil);
	}
	(void) SetEOF((*tocH)->refN,sum.offset+len);
	BoxFClose(tocH);
	
	SaveMessageSum(&sum,tocH);
	
	newWin = OpenComp(tocH,(*tocH)->count-1,nil,False);
	if (newWin && toWhom)
	{
		MessHandle newMessH = (MessType **)newWin->qWindow.refCon;
		MyGetItem(GetMHandle(NEW_TO_HIER_MENU),toWhom,buffer);
		SetMessText(newMessH,TO_HEAD-1,buffer+1,*buffer);
		(*newMessH)->win->isDirty = False;
	}
	return(newWin);
}

/************************************************************************
 * EmptyTrash - empty the trash mailbox
 ************************************************************************/
void EmptyTrash(void)
{
	Str63 name;
				Str15 suffix;
	TOCType **tocH;
	short refN;
	
	
	GetRString(name,TRASH);

	/*
	 * open and truncate the mailbox
	 */
	if (!FSHOpen(name,MyVRef,MyDirId,&refN,fsRdWrPerm))
	{
		SetEOF(refN,0);
		FSClose(refN);
	}

	tocH = FindTOC(MyDirId,name);
	if (tocH && (*tocH)->win)
	{
		(*tocH)->count = 0;
		(*tocH)->dirty = True;
		SetHandleSize(tocH,sizeof(TOCType));
		MyWindowMaxes((*tocH)->win,0,0);
		MyWindowDidResize((*tocH)->win,nil);
		InvalContent((*tocH)->win);
	}
	else
	{
		/*
			* delete the toc
			*/
		PCat(name,GetRString(suffix,TOC_SUFFIX));
		HDelete(MyVRef,MyDirId,name);
	}

}

/************************************************************************
 * CloseAllBoxes
 ************************************************************************/
Boolean CloseAllBoxes(void)
{
	TOCType **thisTOC,**nextTOC;
	
	for (thisTOC=TOCList; thisTOC; thisTOC=nextTOC)
	{
		nextTOC = (*thisTOC)->next;
		if (!CloseMyWindow((*thisTOC)->win)) return(False);
	}
	return(True);
}

/************************************************************************
 *
 ************************************************************************/
Boolean CloseAllMessages(void)
{
	MessType **thisMess, **nextMess;
	
	for (thisMess=MessList; thisMess; thisMess=nextMess)
	{
		nextMess = (*thisMess)->next;
		if (!CloseMyWindow((*thisMess)->win)) return(False);
	}
	return(True);
}


/************************************************************************
 * AddSelectionAsTo - add the current selection to the to list
 ************************************************************************/
void AddSelectionAsTo(void)
{
	MyWindowPtr win = FrontWindow();
	Str63 scratch;
	short len;
	TEHandle teh = WinTEH(win);
	
	if (!teh) return;
	
	len = (*teh)->selEnd - (*teh)->selStart;
	if (!len) return;
	
	if (len>sizeof(scratch)-1)
	{
		WarnUser(TO_TOO_LONG,len);
		return;
	}
	
	BlockMove(*(*teh)->hText+(*teh)->selStart,scratch+1,len);
	*scratch = len;
	AddStringAsTo(scratch);
}

/************************************************************************
 * AddStringAsTo - add a string to the to list
 ************************************************************************/
void AddStringAsTo(UPtr string)
{ 
	short spot,result;
	MenuHandle mh;
	Str63 oldItem;
	short menu;

	mh = GetMHandle(NEW_TO_HIER_MENU);
	for (spot=1;spot<=CountMItems(mh);spot++)
	{
		MyGetItem(mh,spot,oldItem);
		result = IUCompString(string,oldItem);
		if (result==0) return;
		if (result<0) break;
	}
	
	for (menu=NEW_TO_HIER_MENU;menu<=INSERT_TO_HIER_MENU;menu++)
		MyInsMenuItem(GetMHandle(menu),string,spot-1);
	
	ToMenusChanged();
}

/************************************************************************
 * ToMenusChanged - see that the to menus get written out
 ************************************************************************/
void ToMenusChanged(void)
{
	MenuHandle mh;
	
	mh = GetResource('MENU',NEW_TO_HIER_MENU+1000);
	if (mh)
	{
		Handle savedProcId = (*mh)->menuProc;
		(*mh)->menuProc = nil;
		ChangedResource(mh);
		WriteResource(mh);
		(*mh)->menuProc = savedProcId;
	}
}

#pragma segment Balloon
/************************************************************************
 * HelpRect - show a standard help balloon
 ************************************************************************/
void HelpRect(Rect *tipRect,short resSelect,short percentRight)
{
	HMMessageRecord hmmr;
	Point tip;
	static short oldSelect=0;
	
	if (oldSelect==resSelect && HMIsBalloon()) return;
	oldSelect = resSelect;
	LocalToGlobal((Point*)tipRect);
	LocalToGlobal((Point*)tipRect + 1);
	tip.h = tipRect->left + (percentRight*(tipRect->right-tipRect->left))/100;
	tip.v = (tipRect->bottom+tipRect->top)/2;
	hmmr.u.hmmStringRes.hmmResID = (resSelect/200)*200;
	hmmr.u.hmmStringRes.hmmIndex = resSelect%200;
	hmmr.hmmHelpType = khmmStringRes;
	HMShowBalloon(&hmmr,tip,tipRect,nil,0,0,0);
}

/************************************************************************
 * HelpPict - show a help balloon with a pict in it
 ************************************************************************/
void HelpPict(Rect *tipRect,short resId,short percentRight)
{
	HMMessageRecord hmmr;
	Point tip;
	static short oldId=0;
	
	if (oldId==resId && HMIsBalloon()) return;
	oldId = resId;
	LocalToGlobal((Point*)tipRect);
	LocalToGlobal((Point*)tipRect + 1);
	tip.h = tipRect->left + (percentRight*(tipRect->right-tipRect->left))/100;
	tip.v = (tipRect->bottom+tipRect->top)/2;
	hmmr.u.hmmPict = resId;
	hmmr.hmmHelpType = khmmPict;
	HMShowBalloon(&hmmr,tip,tipRect,nil,0,0,0);
}

/************************************************************************
 * HelpRectString - show a standard help balloon, but provide a string
 *  instead of a resource id.
 ************************************************************************/
void HelpRectString(Rect *tipRect,UPtr string,short percentRight)
{
	HMMessageRecord hmmr;
	Point tip;
	
	LocalToGlobal((Point*)tipRect);
	LocalToGlobal((Point*)tipRect + 1);
	tip.h = tipRect->left + (percentRight*(tipRect->right-tipRect->left))/100;
	tip.v = (tipRect->bottom+tipRect->top)/2;
	PCopy(hmmr.u.hmmString,string);
	hmmr.hmmHelpType = khmmString;
	HMShowBalloon(&hmmr,tip,tipRect,nil,0,0,0);
}

/************************************************************************
 * DoHelp - do help for a window
 ************************************************************************/
void DoHelp(MyWindowPtr win)
{
	Point mouse;
	GrafPtr oldPort;
	
	if (!win) return;
	GetPort(&oldPort);
	SetPort(win);
	GetMouse(&mouse);
	
	if (PtInRect(mouse,&((GrafPtr)win)->portRect) && win->help)
		(*win->help)(win,mouse);

	SetPort(oldPort);
}
#pragma segment Misc
		
/************************************************************************
 * HandleWindowChoice - handle the user choosing a window menu item
 ************************************************************************/
void HandleWindowChoice(short item)
{
	MyWindowPtr win;
	
	for (win=FrontWindow();win;win=win->qWindow.nextWindow)
		if (win->qWindow.visible && !--item)
			SelectWindow(win);
}


/************************************************************************
 * ChangePassword - change the user's password
 ************************************************************************/
short ChangePassword(void)
{
	Str31 user, old, new, candidate;
	Str255 account;
	short err;
	
	*Password = 0;			/* make the user reenter it */
	POPSecure = 0;
	GetPref(account,PREF_POP);
	if (!(err=GetPassword(account,old,sizeof(old),ENTER)))
	{
		if (!(err=GetPassword(account,candidate,sizeof(candidate),NEW)) &&
				!(err=GetPassword(account,new,sizeof(new),VERIFY)))
		{
			if (!EqualString(candidate,new,True,True))
			{
				WarnUser(PW_MISMATCH,0);
				err = 1;
			}
			else
			{
				GetPOPInfo(user,account);
				PCopy(Password,old);
				err = ReallyChangePassword(user,account,old,new);
			}
		}
	}
	if (err) InvalidatePasswords(True,False);
	return(err);
}

/************************************************************************
 * ReallyChangePassword - change the user's password, really
 ************************************************************************/
short ReallyChangePassword(UPtr user,UPtr host,UPtr old,UPtr new)
{
	enum {PW_USER=801,PW_PASS,PW_NEWPASS,PW_QUIT};
	short err=1;
	long port = GetRLong(PW_PORT);
	Str255 buffer,junk;
	
	OpenProgress();
	if (!UseCTB || !DialThePhone())
		if (!ConnectTrans(host,port,False))
		{
			if (!CheckResult(buffer) &&
					!SendPwCommand(PW_USER,user) && !CheckResult(buffer) &&
					!SendPwCommand(PW_PASS,old) && !CheckResult(buffer) &&
					!SendPwCommand(PW_NEWPASS,new) && !CheckResult(buffer))
			{
				err = 0;
				SilenceTrans(True);
				if (!SendPwCommand(PW_QUIT,"")) CheckResult(junk);
				PCopy(Password,new);
				POPSecure = True;
			}
		}
	if (UseCTB) HangUpThePhone();
	DestroyTrans();
	CloseProgress();
	if (!err) {SetAlertBeep(False);AlertStr(OK_ALRT,Normal,buffer);SetAlertBeep(True);}
	return(err);
}

/************************************************************************
 * SendPwCommand - send a password command
 ************************************************************************/
short SendPwCommand(short cmd,UPtr arg)
{
	Str255 buffer;
	ComposeRString(buffer,cmd,arg,NewLine);
	return (SendTrans(1,buffer+1,*buffer));
}

/************************************************************************
 * CheckResult - did the command succeed?
 ************************************************************************/
short CheckResult(UPtr buffer)
{
	short err;
	
	do
	{
		err = GetReply(buffer+1,253,False);
		*buffer = strlen(buffer+1);
		if (err>=400 && err<600)
		{
			Str255 pwErr;
			GetRString(pwErr,PW_ERROR);
			MyParamText(pwErr,buffer,"","");
			ReallyDoAnAlert(OK_ALRT,Stop);
		}
		else Progress(NoBar,buffer);
	}
	while (err<200);
	return(err>399);
}