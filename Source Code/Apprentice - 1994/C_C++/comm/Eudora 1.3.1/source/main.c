#define FILE_NUM 22
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Main
#ifdef	KERBEROS
#include				<krb.h>
#endif

#define TICKS2MINS 3600

	Boolean HandleEvent(EventRecord *event);
	extern void UnloadSegments(void);
	void HouseKeeping(long activeTicks);
	void DoKeyDown(WindowPtr topWin,EventRecord *event);
	void DoMouseDown(WindowPtr topWin,EventRecord *event);
	Boolean DoUpdate(WindowPtr topWin,EventRecord *event);
	Boolean DoActivate(WindowPtr topWin,EventRecord *event);
	Boolean DoApp4(WindowPtr topWin,EventRecord *event);
	Boolean DoApp1(MyWindowPtr topWin,EventRecord *event);
	void DoContentClick(MyWindowPtr topWin,MyWindowPtr win,EventRecord *event);
	void NotifyNewMail(void);
	void NewMailSound(void);
	Boolean DoDisk(long message);
	void TendNotificationManager(Boolean isActive);
	long CloseUnused(void);

#ifndef KERBEROS
void main()
#else
void c_main()
#endif
{
	EventRecord theEvent;
	MyWindowPtr topWin;
	Boolean active=True;
	long houseTicks=0;
	long activeTicks=TickCount();
	
	/*
	 * perform set-up, including initializing mac managers
	 */
	UnloadSegments();
	Initialize();
	UnloadSegments();

	/*
	 * run the event loop
	 */
	while (!EjectBuckaroo && !Done)
	{
#ifdef DEBUG
	if (BUG3 && !FrontWindow())
	{
		Str63 s;
		long grow, newFree;
		static long oldFree=1L<<30;
		(void) MaxMem(&grow);
		newFree = FreeMem();
		if (oldFree>newFree)
		{
			DebugStr(ComposeString(s,"\p#%d;ht;g",oldFree-newFree));
			AlertStr(OK_ALRT,Stop,ComposeString(s,
			 "\pLeak? was %d, now %d (%d)",oldFree,newFree,oldFree-newFree));
		}
		oldFree = newFree;
	}
#endif
#ifdef DEBUG
		if (BUG6) DebugStr("\p;hc;g");
#endif
		CommandPeriod = False;
		
		/*
		 * allocate a window here, in case we need it later
		 */
		if (!HandyWindow)
		{
			HandyWindow = New(MyWindow);
			if (HandyWindow != nil) WriteZero(HandyWindow,sizeof(MyWindow));
		}
		
		/*
		 * handle any events we might come across 
		 */
		active = GrabEvent(&theEvent) ? HandleEvent(&theEvent) : False;
		if (active)
			activeTicks = TickCount();
		
		TendNotificationManager(active);
		
		if (TickCount()-houseTicks > 120)
		{
			HouseKeeping(TickCount()-activeTicks);
			houseTicks = TickCount();
		}

#ifndef SLOW_CLOSE
		/*
     * destroy defunct streams
	   */
		TcpFastFlush(False);
#endif
			
		/*
		 * blink caret, if need be
		 */
		topWin = FrontWindow();
		if (topWin && IsMyWindow(topWin) && !InBG)
		{
			if (topWin->txe) TEIdle(topWin->txe);
			else if (topWin->ste) TEIdle((*(STEHandle)topWin->ste)->te);
		}
		
		/*
		 * write dirty toc's
		 */
		if (!PrefIsSet(PREF_CORVAIR)) FlushTOCs(False,True);
		
		/*
		 * cursor & help
		 */
		if (SFWTC) SetCursorByLocation();
		if (theEvent.what==nullEvent && HasHelp && !InBG && HMGetBalloons())
			DoHelp(topWin);
					
		/*
		 * here's an interesting little goodie
		 */
		if (theEvent.what==nullEvent && !InBG && AliasWinIsOpen() &&
				FrontWindow() && IsMyWindow(FrontWindow()) &&
				((MyWindowPtr)(FrontWindow()))->txe && PrefIsSet(PREF_NICK_FOLLOW))
		{
			switch (((MyWindowPtr)(FrontWindow()))->qWindow.windowKind)
			{
				case COMP_WIN:
				case TEXT_WIN:
					FinishAlias(FrontWindow(),False,True);
					break;
				default: break;
			}
		}

		/*
		 * Unload all segments
		 */
		UnloadSegments();
		
		/*
		 * debugging...
		 */
#ifdef DEBUG
		{
			extern void **AliasGlobals;
			ASSERT(!AliasGlobals || Notes && *Notes);
		}
#endif
		
		/*
		 * all windows are fair game
		 */
		NotUsingAllWindows();
		if (EjectBuckaroo) DoQuit();
	}
	if (PrefIsSet(PREF_AUTO_EMPTY)) EmptyTrash();
	FlushTOCs(True,False);
	Cleanup();
}
/************************************************************************
 * HouseKeeping - periodic cleanup functions
 ************************************************************************/
void HouseKeeping(long activeTicks)
{
	static long flushTicks=0;

	if (activeTicks)
	{
		long ivalTicks = TICKS2MINS * GetRLong(PREF_STRN+PREF_INTERVAL);

		/*
		 * warn the user if we're growing low (get it) on memory
		 */
		MonitorGrow();
	
		/*
		 * check mail?
		 */
		if (activeTicks > 60*60)
		{
			if (ivalTicks && (!CheckTicks || TickCount()-CheckTicks > ivalTicks))
			{
				if (!(CurrentModifiers()&optionKey))
				{
					CheckForMail(False);
				}
			}
			if (ForceSend <= GMTDateTime()) {SetSendQueue();DoSendQueue();}
		}
		
		/*
		 * close unneeded TOC's
		 */
		FlushTOCs(True,True);
		
		/*
		 * just for good measure
		 */
		if (TickCount()-flushTicks > 60*GetRLong(FLUSH_SECS))
		{
			FlushVol(nil,MyVRef);
			flushTicks = TickCount();
		}
		
		/*
		 * The resolver may have left us little poodle bombs...
		 */
		FlushHIQ();
		
		/*
		 * and logging...
		 */
		if (!(LogLevel&LOG_EVENT)) CloseLog();
		
		/*
		 * and that pesky settings file
		 */
		UpdateResFile(SettingsRefN);
		
		/*
		 * for debugging
		 */
#ifdef DEBUG
		if (BUG7)
		{
			long bytes;
			MaxMem(&bytes);
		}
#endif
	}
}

/**********************************************************************
 * HandleEvent - handle an event
 **********************************************************************/
Boolean HandleEvent(EventRecord *event)
{
	WindowPtr topWin = FrontWindow(); 	/* in case we need to know */
	Boolean active=False;
	
	if (IsDialogEvent(event))
	{
	  DoModelessEvent(event);
		return(True);
	}

	if (topWin)
	{
		SetPort(topWin);
		UsingWindow(topWin);
	}
	else
		SetPort(InsurancePort);
	/*
	 * slog through all the events...
	 */
	switch (event->what)
	{
		case keyDown:
		case autoKey:
			/*if (HasHelp && HMIsBalloon()) HMRemoveBalloon();*/
			DoKeyDown(topWin,event);
			active = True;
			break;
		
		case mouseUp:
		case mouseDown:
			/*if (HasHelp && HMIsBalloon()) HMRemoveBalloon();*/
			DoMouseDown(topWin,event);
			active = True;
			break;
			
		case updateEvt:
			DoUpdate(topWin,event);
			break;
			
		case activateEvt:
			if (!InBG) DoActivate(topWin,event);
			break;
			
		case app4Evt:
			active = DoApp4(topWin,event);
			break;
			
		case app1Evt:
			active = DoApp1(topWin,event);
			break;
			
		case diskEvt:
			active = DoDisk(event->message);
			break;
	}
	if (!ForceSend) SetSendQueue();	/* a message from beyond */
	EnableMenus(FrontWindow()); 	/* enable the correct menu items */
	return(active);
}

/************************************************************************
 *
 ************************************************************************/
Boolean DoDisk(long message)
{
	if ((message>>16)&0xffff)
	{
		Point pt;
		pt.h = pt.v = 50;
		DILoad();
		(void) DIBadMount(pt,message);
		DIUnload();
	}
	return(True);
}

/**********************************************************************
 * DoApp4 - handle an app4 event; suspend/resume/mouse
 **********************************************************************/
Boolean DoApp4(WindowPtr topWin,EventRecord *event)
{
	if (((event->message>>24)&0xff) == 0xfa)
	{
		SFWTC = True;
		return(False);
	}
	else
	{
		if (((event->message)>>24)&1 == 1)
		{
			if (!InBG)
			{
				EnableProgress();
				SFWTC = True;
			}
			else
				DisableProgress();
			if (CnH) CMResume(CnH,!InBG);
			if (topWin && ((WindowPeek)topWin)->windowKind >= userKind)
				ActivateMyWindow(topWin,!InBG);
		}
		return(!InBG);
	}
}

/**********************************************************************
 * DoApp1 - handle an app1 event; scrolling by saratoga keys
 **********************************************************************/
Boolean DoApp1(MyWindowPtr topWin,EventRecord *event)
{
	if (topWin && IsMyWindow(topWin))
	{
		if (topWin->app1 && (*topWin->app1)(topWin,event))
			;
	  else if (topWin->ste) STEApp1(topWin->ste,event);
		else if (topWin->vBar)
			switch(event->message & charCodeMask)
			{
				case homeChar:
					ScrollIt(topWin,INFINITY,INFINITY);
					break;
				case endChar:
					ScrollIt(topWin,0,-INFINITY);
					break;
				case pageUpChar:
					ScrollIt(topWin,0,
						RoundDiv(topWin->contR.bottom-topWin->contR.top,topWin->vPitch)-1);
					break;
				case pageDownChar:
					ScrollIt(topWin,0,
						RoundDiv(topWin->contR.top-topWin->contR.bottom,topWin->vPitch)+1);
					break;
			}
	}
	return(True);
}

/**********************************************************************
 * DoKeyDown - handle a keystroke aimed at a window
 **********************************************************************/
void DoKeyDown(WindowPtr topWin,EventRecord *event)
{
	long select;
	short c = UnadornMessage(event)&0xff;

	/*
	 * handle key equivalents
	 */
	if (event->modifiers&cmdKey)
	{
		if (select=MyMenuKey(event))
		{
			DoMenu(topWin,select,event->modifiers);
			return;
		}
	}
	if (topWin && ((WindowPeek)topWin)->windowKind >= userKind)
	{
		MyWindowPtr mw = (MyWindowPtr) topWin;
		TEHandle teh = WinTEH(mw);
		
		SetPort(mw);
		if (mw->key)
			(*mw->key)(mw,event);
		else if (event->modifiers&cmdKey)
			SysBeep(20L);
		else if (!teh)
			AlertStr(NOT_HOME_ALRT, Stop, nil);
		else if (mw->ro && DirtyKey(event->message))
			AlertStr(READ_ONLY_ALRT, Stop, nil);
		else
		{
			TEActivate(teh);
			if ((event->message&0xff)==tabKey)
				TEFakeTab(mw,GetRLong(TAB_DISTANCE));
			else
				TESomething(mw,TEKEY,event->message&charCodeMask,event->modifiers);
		}
	}
	else if (event->modifiers&cmdKey)
		SysBeep(20L);
	else
		StopAlert(NOT_HOME_ALRT, nil);
}

pascal void Hook(void)
{
	if (RunType != Production && CurrentModifiers()&controlKey)
	{
		Handle fkey3;
		if (fkey3 = GetResource('FKEY',3))
		{
			(*(ProcPtr)LDRef(fkey3))();
			UL(fkey3);
		}
		else SysBeep(10);
	}
}

/**********************************************************************
 * DoMouseDown - handle a mouse-down event, goodness knows where
 **********************************************************************/
void DoMouseDown(WindowPtr topWin,EventRecord *event)
{
	static Point downSpot;	/* Point of previous mouseDown */
	static long upTicks;		/* time of previous mouseUp */
	long dblTime;						/* double-click time from PRAM */
	long wPart; 						/* window part where click occurred */
	MyWindowPtr win;				/* window where click occurred */
	short tolerance;
	long mSelect;
	
	if (event->what==mouseUp)
	{
		upTicks = event->when-1;
		return;
	}
	
	dblTime = GetDblTime();
	tolerance = GetRLong(DOUBLE_TOLERANCE);
	
	/*
	 * figure out whether this is a double or triple click,
	 * and update double and triple click times
	 */
	if (event->when!=upTicks && event->when-upTicks<dblTime)
	{
		int dx = event->where.h - downSpot.h;
		int dy = event->where.v - downSpot.v;
		if (ABS(dx)<tolerance && ABS(dy)<tolerance && !(event->modifiers&cmdKey))
		{
			/* upgrade the click */
			ClickType++;
			if (ClickType > Triple) ClickType = Single;
		}
		else
			ClickType = Single;
	}
	else
		ClickType = Single;
		
	upTicks = event->when;			/* ignore two downs in a row as far as clicktype */
	downSpot = event->where;
	
	/*
	 * where was the click?
	 */
	wPart = FindWindow(event->where,&win);
	if (CnH && win && (long)CnH==win->qWindow.refCon)
		CMEvent(CnH,event);
	else
		switch (wPart)
		{
			case inMenuBar:
				EnableMenuItems();
				mSelect = MenuSelect(event->where);
				DoMenu(topWin,mSelect,CurrentModifiers());
				break;
			case inSysWindow:
				SystemClick(event,win);
				break;
			case inDrag:
				DragMyWindow(win,event->where);
				break;
			case inZoomIn:
			case inZoomOut:
				if (IsMyWindow(win)) win->saveSize = True;
				ZoomMyWindow(win,event->where,wPart);
				break;
			case inGrow:
				if (!(win->qWindow.windowKind==dialogKind ||
						win->qWindow.windowKind>=userKind && win->isRunt))
				{
					GrowMyWindow(win,event->where);
					break;
				}
				/* fall-through is deliberate */
			case inContent:
				Undo.didClick = True;
				DoContentClick(topWin,win,event);
				break;
			case inGoAway:
				if (event->modifiers & optionKey)
					CloseAll();
				else
					GoAwayMyWindow(win,event->where);
				break;
		}
}

/**********************************************************************
 * DoContentClick - handle a click in the content region of a window
 **********************************************************************/
void DoContentClick(MyWindowPtr topWin,MyWindowPtr win,EventRecord *event)
{
	Point pt;
	Rect r;
	
	if (win != (MyWindowPtr)topWin)
	{
		SelectWindow(win);
		if (IsMyWindow(win) && win->hot) HandleEvent(event); /* resubmit */
	}
	else
	{
		SetPort(win);
		pt = event->where;
		GlobalToLocal(&pt);
		if (!win->dontControl && HandleControl(pt,win))
			;
		else if (((WindowPeek)win)->windowKind > userKind)
		{
			if (win->click)
				(*win->click)(win,event);
			else if (win->ste && ((r=(*(STEHandle)win->ste)->encloseR),PtInRect(pt,&r)))
				STEClick(win->ste,event);
		}
	}
}


/************************************************************************
 * 
 ************************************************************************/
Boolean HandleControl(Point pt, MyWindowPtr win)
{
	int part;
	int oldValue, difference;
	ControlHandle cntl;

	if (part = FindControl(pt,win,&cntl))
	{
		if ((*cntl)->contrlRfCon!='GREY')
		{
			oldValue = GetCtlValue(cntl);
			part = TrackControl(cntl,pt,part==inThumb ? nil : (UPtr)(-1));
			if (part==inThumb)
			{
				difference = oldValue - GetCtlValue(cntl);
				if (difference)
				{
					if (ScrollIsH(cntl))
						ScrollMyWindow(win,difference,0);
					else
						ScrollMyWindow(win,0,difference);
					UpdateMyWindow(win);
				}
			}
			else if (win->button)
				(*win->button)(win,cntl,CurrentModifiers(),part);
		}
	  return(True);
	}
	return(False);
}

/**********************************************************************
 * DoUpdate - handle an update event
 **********************************************************************/
Boolean DoUpdate(WindowPtr topWin,EventRecord *event)
{
#pragma unused(topWin)
	MyWindowPtr win;
	
	win = (MyWindowPtr)event->message;
	if (CnH && win->qWindow.refCon==(long)CnH)
	{
		CMEvent(CnH,event);
		return(False);
	}
	if (IsMyWindow(win))
	{
		UpdateMyWindow(win);
		return(False);
	}
	return(True);	/* somebody else better do this one */
}

/**********************************************************************
 * DoActivate - handle an activate event (and a prior deactivate, to boot)
 **********************************************************************/
Boolean DoActivate(WindowPtr topWin,EventRecord *event)
{
#pragma unused(topWin)
	MyWindowPtr win;
	Boolean active = !InBG && (event->modifiers&activeFlag)!=0;
	Boolean yours = False;
	
	win = (MyWindowPtr) event->message;
	if (CnH && (long)CnH==win->qWindow.refCon)
		CMActivate(CnH,active);
	else if (IsMyWindow(win))
	{
		if (active != win->isActive)
			ActivateMyWindow(win,(event->modifiers&activeFlag)!=0);
	}
	else
		yours=True;
	SFWTC=True;
	return(yours);
}

/**********************************************************************
 * WarnUser - tell the user that something bad is happening
 **********************************************************************/
int WU(int errorStr,int errorNum,int file,int line)
{
	Str255 message;
	
	ComposeRString(message,WU_FMT,errorStr,errorNum,file,line);
	(void) AlertStr(OK_ALRT,Caution,message);
	return(errorNum);
}

/**********************************************************************
 * DieWithError - die with the given error message
 **********************************************************************/
void DWE(int errorStr,int errorNum,int file,int line)
{
	Str255 fatal;
	Str255 message;
	Str255 number;
	Str255 debugStr;
	
	GetRString(fatal,FATAL);
	GetRString(message,errorStr);
	NumToString((long)errorNum,number);
	ComposeRString(debugStr,FILE_LINE_FMT,file,line);
	MyParamText(fatal,message,number,debugStr);
	(void) ReallyDoAnAlert(ERR_ALRT,Stop);
	Cleanup();
	ExitToShell();
}

/**********************************************************************
 * DumpData - put up an alert that shows some data.
 **********************************************************************/
void DumpData(UPtr description, UPtr data,int length)
{
	Str255 asAscii;
	Str255 asHex;
	static char hex[]="0123456789abcdef";
	char *ac, *hx;
	char *from;
	
	if (length > 255/2) length = 255/2;
	
	/*
	 * prepare display strings
	 */
	*asAscii = 2*length;
	*asHex = 2*length;
	ac=asAscii+1;
	hx=asHex+1;
	for (from=data; from<data+length;from++)
	{
		*ac++ = optSpace;
		if (*from==' ')
			*ac++ = optSpace;
		else if (*from<' ')
			*ac++='.';
		else
			*ac++ = *from;
		*hx++ = hex[((*from)>>4)&0xf];
		*hx++ = hex[(*from)&0xf];
	}
	
	SetDAFont(monaco);
	MyParamText(description,asAscii,asHex,"");
	switch (Alert(DUMP_ALRT,nil))
	{
		case DEBUG_BUTTON: Debugger(); break;
		case EXIT_BUTTON:  Cleanup(); ExitToShell(); break;
	}
	SetDAFont(applFont);
} 

/**********************************************************************
 * monitor the heap situation
 **********************************************************************/
void MonitorGrow(void)
{
	static long memLeft = SPARE_SIZE;
	long roomLeft = GetHandleSize(SpareSpace);
	long roomAvailable;
	
	if (roomLeft < memLeft)
	{
		UnloadUnneeded(1L<<29);
		FlushTOCs(True,True);
		UnloadUnneeded(1L<<29);
		if (memLeft==SPARE_SIZE || roomLeft+10K<memLeft || roomLeft<MEM_CRITICAL)
			MemoryWarning();
	}
	
	if (roomLeft < memLeft || roomLeft<MEM_CRITICAL)
	{
		memLeft = roomLeft;
		DisposHandle(SpareSpace);
		roomAvailable = CompactMem(SPARE_SIZE);
		if (roomAvailable < SPARE_SIZE)
			roomAvailable = MaxMem(&roomLeft);
		if (roomAvailable > SPARE_SIZE) roomAvailable = SPARE_SIZE;
		MakeGrow(roomAvailable);
	}
}

/************************************************************************
 * MemoryWarning - warn the user about how much memory is left.
 ************************************************************************/
MemoryWarning(void)
{
	Str255 partitionString,warningString;
	long currentSize, estSize, spareSize;
	
	currentSize = CurrentSize();
	estSize = EstimatePartitionSize();
	spareSize = GetHandleSize(SpareSpace);
	estSize = MAX(estSize,SPARE_SIZE-spareSize+currentSize);
	(void) ComposeRString(partitionString,MEM_PARTITION,currentSize/(1K),
												estSize/(1K));
	GetRString(warningString,MEM_LOW);
	MyParamText(warningString,partitionString,"\p\n\n\n","");
	if (ReallyDoAnAlert(MEMORY_ALRT,Caution)==MEMORY_QUIT) EjectBuckaroo=True;
}

/************************************************************************
 * CurrentSize - how big is our partition?
 ************************************************************************/
long CurrentSize(void)
{
	SizeHandle sizeH;
	long size=0;
	
	if (sizeH = GetResource('SIZE',0))
	{
		size = (*sizeH)->prefSize;
		ReleaseResource(sizeH);
		return(size);
	}
	else return(DefaultSize());
}

/************************************************************************
 * DefaultSize - how big is Eudora's normal partition?
 ************************************************************************/
long DefaultSize(void)
{
	SizeHandle sizeH;
	long size=0;
	
	if (sizeH = GetResource('SIZE',-1))
	{
		size = (*sizeH)->prefSize;
		ReleaseResource(sizeH);
	}
	return(size);
}

/************************************************************************
 * EstimatePartitionSize - estimate the "right" size for Eudora
 ************************************************************************/
long EstimatePartitionSize(void)
{
	HFileInfo info;
  Str63 scratch;
	Str31 suffix;
	uLong drain = 0;
	static short strids[] = {IN,OUT,TRASH,ALIAS_FILE};
	short id;
	MyWindowPtr win;
	
	GetRString(suffix,TOC_SUFFIX);
	
	for (id=0;id<sizeof(strids)/sizeof(short);id++)
	{
		GetRString(scratch,strids[id]);
		if (strids[id]!=ALIAS_FILE) PCat(scratch,suffix);
		if (!HGetFileInfo(MyVRef,MyDirId,scratch,&info))
			drain+=info.ioFlLgLen;
	}
	
	for (win=FrontWindow();win;win=win->qWindow.nextWindow)
	{
		drain += sizeof(MyWindow);
		switch (win->qWindow.windowKind)
		{
			case COMP_WIN:
			case MESS_WIN:
				drain += (*BodyOf((MessHandle)win->qWindow.refCon))->teLength;
				break;
			case TEXT_WIN:
				drain += (*(*(STEHandle)win->ste)->te)->teLength;
				break;
			case MBOX_WIN:
			case CBOX_WIN:
				if (!(*(TOCHandle)win->qWindow.refCon)->which)
					drain += GetHandleSize((TOCHandle)win->qWindow.refCon);
				break;
			default:
				drain += 1K;
		}
	}
	 return(DefaultSize()+drain-24K);
}

/**********************************************************************
 * create some space for the grow zone function
 **********************************************************************/
void MakeGrow(long howMuch)
{
	if (SpareSpace = NuHandle(howMuch)) *(long *)*SpareSpace = 'SPAR';
}

/**********************************************************************
 * grow that zone...
 **********************************************************************/
pascal long GrowZone(unsigned long needed)
{
	long roomLeft;
	long freed=0;
	long theA5=SetCurrentA5();
	Handle dontMove = GZSaveHnd();
	PScrapStuff scrap;
	
	if (freed = UnloadUnneeded(needed))
		;
	else if (scrap=InfoScrap(),scrap->scrapHandle&&dontMove!=scrap->scrapHandle&&GetHandleSize(scrap->scrapHandle))
	{
		freed = GetHandleSize(scrap->scrapHandle);
		if (UnloadScrap()) ZeroScrap();				/* if we can't write it, kill it */
		return(freed);
	}
#ifndef SLOW_CLOSE
	else if (FastList)
	{
		TcpFastFlush(True);
		return(1);
	}
#endif
	else if (roomLeft = GetHandleSize(SpareSpace))
	{ 
		freed = needed <= roomLeft ? needed : roomLeft;
		SetHandleSize(SpareSpace,roomLeft - freed);
#ifdef NEVER
		if (RunType != Production) DebugStr("\pSPARE!");
#endif
	}
	else if (Undo.text && Undo.text != dontMove)
	{
		freed = GetHandleSize(Undo.text);
		ZapHandle(Undo.text);
	}
	else if (DamagedTOC && DamagedTOC != dontMove)
	{
		freed = GetHandleSize(DamagedTOC);
		ZapHandle(DamagedTOC);
	}
	else
	{
		freed = CloseUnused();
	}
	(void) SetA5(theA5);
	return(freed);
}

/************************************************************************
 * CloseUnused - close unused windows
 ************************************************************************/
long CloseUnused(void)
{
	static short kinds[] = {MB_WIN,PH_WIN,TEXT_WIN,ALIAS_WIN,MESS_WIN,COMP_WIN};
	MyWindowPtr win,deadWin=nil;
	short kind;
	
	for (kind=0;kind<sizeof(kinds)/sizeof(short);kind++)
	{
		for (win=FrontWindow();win;win=win->qWindow.nextWindow)
		{
			if (win->qWindow.windowKind==kinds[kind] && !win->isDirty && !win->inUse)
				if (kinds[kind]!=MESS_WIN || Win2Body(win)==WinTEH(win)) deadWin = win;
		}
		if (deadWin)
		{
			if (!CloseMyWindow(deadWin))
			{
				UsingWindow(deadWin);		/* couldn't close it ??? */
				deadWin = nil;
				kind--;	/* try again with this kind */
			}
			else
			{
				return(1);
			}
		}
	}
	return(0);
}
		
/**********************************************************************
 * FileSystemError - report an error regarding the file system
 **********************************************************************/
int FSE(int context, UPtr name, int err, int file,int line)
{
	Str255 text;
	Str255 contextStr;
	Str63 debugStr;
	Str127 nameStr;
	int offset = 0;
	
	PCopy(nameStr,name);
	NumToString(err,text);
	PCatC(text,' ');
	if (err < wrPermErr) offset += wrPermErr-dirNFErr;
	if (err < volGoneErr) offset += volGoneErr+127;
	err += offset;
	err = dirFulErr - err + 2;
	if (err > 37) err = 37;
	GetRString(contextStr,FILE_STRN+err);
	PCat(text,contextStr);
	GetRString(contextStr,context);
	ComposeRString(debugStr,FILE_LINE_FMT,file,line);
	MyParamText(contextStr,nameStr,text,debugStr);
	(void) ReallyDoAnAlert(BIG_OK_ALRT,Stop);
	return(err);
}

/************************************************************************
 * MiniMainLoop - handle only a limited set of events.	returns the event
 * if someone else needs to handle it
 ************************************************************************/
Boolean MiniMainLoop(EventRecord *event)
{
	MyWindowPtr topWin=FrontWindow();
	
#ifdef DEBUG
	if (BUG6) DebugStr("\p;hc;g");
	if (BUG7)
	{
		long bytes;
		MaxMem(&bytes);
	}
#endif
	
	TcpFastFlush(False);	/* give lingering connections a chance to die */

	switch (event->what)
	{
		case keyDown:
			if (((event->message&charCodeMask)=='.') && (event->modifiers&cmdKey) ||
					((event->message&charCodeMask)==escChar) &&
					 (((event->message&keyCodeMask)>>8)==escKey))
			{
				CommandPeriod = True;
				return(False);
			}
			break;
		case app4Evt:
			if (((event->message)>>24)&1 == 1)
				DoApp4(topWin,event);
			return(False);
		case updateEvt:
			return(DoUpdate(topWin,event));
			break;
		case activateEvt:
			return(DoActivate(topWin,event));
			break;
		case mouseDown:
			if (IsSwitch(event) && Switch())
			{
				event->what=nullEvent;
				InBG = True;
				return(False);
			}
			break;
	}

	if (CnH && topWin && topWin->qWindow.refCon==(long)CnH)
	{
		CMEvent(CnH,event);
		return(False);
	}
	
	return(True); 
}

/************************************************************************
 * CheckForMail - need I say more?
 ************************************************************************/
void CheckForMail(Boolean setTicks)
{
	short err=0;
	long interval=TICKS2MINS * GetRLong(PREF_STRN+PREF_INTERVAL);
	short gotSome=0;
	Boolean sendToo = SendQueue && PrefIsSet(PREF_SEND_CHECK);
	Str255 user;
	
	GetAlrtStage() = 0;
	SetSendQueue(); FlushTOCs(True,True);
	if (interval && (setTicks || !CheckTicks)) CheckTicks = TickCount();
#ifndef KERBEROS
	GetPref(user,PREF_POP);
	if (UUPCIn || *Password || !GetPassword(user,Password,sizeof(Password),ENTER))
#else
	if (UUPCIn || GetPassword())
#endif
	{
		AlertsTimeout = PrefIsSet(PREF_AUTO_DISMISS);
		OpenProgress();
#ifdef POPSECURE
		if (UseCTB && !UUPCIn) err=DialThePhone();
#else
		if (UseCTB && (!UUPCIn || (!UUPCOut&&sendToo))) err=DialThePhone();
#endif
		if (!err)
		{
			if (sendToo)
			{
				err=SendTheQueue();
				CommandPeriod = False;	/* clear condition, continue with POP */
				if (!UUPCIn && !UUPCOut && UseCTB && !err) CTBNavigateSTRN(NAVMID);
			}
			if (!UseCTB || !err)
			{
				Handle table;
				if (!NewTables && !TransIn && (table=GetResource('taBL',TRANS_IN_TABL)))
				{
				  HNoPurge(table);
					if (TransIn=NuPtr(256)) BlockMove(*table,TransIn,256);
					HPurge(table);
				}

				gotSome = UUPCIn ? GetUUPCMail(True) : GetMyMail(True);
			}
		}
		if (UseCTB) HangUpThePhone();
		CloseProgress();
		EnableProgress();
		if (gotSome>0) NotifyNewMail();
		AlertsTimeout = False;
		if (TransIn) {DisposPtr(TransIn); TransIn=nil;}
	}
	if (UseCTB) HangUpThePhone();	/* just making sure... */
	if (interval && CheckTicks+interval<TickCount()+45*60)
	{
		CheckTicks += interval*((TickCount()-CheckTicks+1)/interval);
		if (CheckTicks+interval<TickCount()+45*60) CheckTicks += interval;
	}
}

/************************************************************************
 * NotifyNewMail - notify the user that new mail has arrived, via the
 * notification manager.
 ************************************************************************/
void NotifyNewMail(void)
{
	TOCHandle tocH;
	if (PrefIsSet(PREF_NEW_SOUND)) NewMailSound();
	if (!PrefIsSet(PREF_NO_OPEN_IN) && (tocH=GetInTOC()))
	{
			short i=(*tocH)->count;
			while (i--) if ((*tocH)->sums[i].state!=UNREAD) break;
			i++;
			RedoTOC(tocH);
			ShowMyWindow((*tocH)->win);
			SelectBoxRange(tocH,i,i,False,-1,-1);
			ScrollIt((*tocH)->win,0,-INFINITY);
			SelectWindow((*tocH)->win);
#ifdef NEVER
		}
		else
		{
			UpdateMyWindow((*tocH)->win);
		  ScrollIt((*tocH)->win,0,-INFINITY);
		}
#endif
	}

	if (InBG && PrefIsSet(PREF_NEW_ALERT) || !PrefIsSet(PREF_NO_APPLE_FLASH))
	{
		if (MyNMRec) return;				/* already done */
		MyNMRec = New(struct NMRec);
		if (!MyNMRec) return; 			/* couldn't allocate memory (bad) */
		WriteZero(MyNMRec,sizeof(*MyNMRec));
		MyNMRec->qType = nmType;
		MyNMRec->nmMark = 1;
		MyNMRec->nmRefCon = TickCount();
		if (!PrefIsSet(PREF_NO_APPLE_FLASH))
			MyNMRec->nmIcon = GetResource('SICN',FLAG_SICN);
		if (InBG && PrefIsSet(PREF_NEW_ALERT))
		{
			Str255 scratch;
			GetRString(scratch,NEW_MAIL);
			MyNMRec->nmStr = NuPtr(*scratch+1);
			if (MyNMRec->nmStr) PCopy(MyNMRec->nmStr,scratch);
		}
		if (NMInstall(MyNMRec))
		{
			DisposPtr(MyNMRec->nmStr);
			DisposPtr(MyNMRec);
			MyNMRec = nil;
		}
	}
	if (!InBG && PrefIsSet(PREF_NEW_ALERT))
	{
		AlertTicks=GetRLong(ALERT_TIMEOUT)*60+TickCount();
		(void) ReallyDoAnAlert(NEW_MAIL_ALRT,Note);
		AlertTicks = 0;
	}
}

/************************************************************************
 * NewMailSound - play the sound for new mail
 ************************************************************************/
void NewMailSound(void)
{
	Handle sound=GetResource('snd ',NEW_MAIL_SND);
	if (sound)
	{
		HNoPurge(sound);
		(void) SndPlay(nil,sound,False);
		HPurge(sound);
	}
	else
		SysBeep(20);
}

/************************************************************************
 *
 ************************************************************************/
void FlushHIQ(void)
{
	HostInfoQHandle hiq, nextHIQ;
	for (hiq=HIQ;hiq;hiq=nextHIQ)
	{
		nextHIQ = (*hiq)->next;
		if ((*hiq)->hi.rtnCode!=inProgress && (*hiq)->hi.rtnCode!=cacheFault)
		{
			LL_Remove(HIQ,hiq,(HostInfoQHandle));
			DisposHandle(hiq);
		}
	}
#ifndef KERBEROS
	if (!HIQ) CloseResolver();
#endif
}

/************************************************************************
 * TendNotificationManager - see if an NM rec is active, and if it should
 * be taken down
 ************************************************************************/
void TendNotificationManager(Boolean isActive)
{
	if (MyNMRec)
	{
		long ticksSince = TickCount()-MyNMRec->nmRefCon;
		if (isActive && ticksSince>10 || AlertsTimeout &&
				MyNMRec->nmRefCon && ticksSince>GetRLong(ALERT_TIMEOUT)*60)
		{
			NMRemove(MyNMRec);
			if (MyNMRec->nmStr) DisposPtr(MyNMRec->nmStr);
			DisposPtr(MyNMRec);
			MyNMRec = nil;
		}
	}
}
