#define FILE_NUM 26
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Util

/************************************************************************
 * GetNewMyDialog - get a new dialog, with a bit extra
 ************************************************************************/
MyWindowPtr GetNewMyDialog(short template,UPtr wStorage,WindowPtr behind,
	void (*centerFunction)())
{
	MyWindowPtr win;
	
	if (wStorage == nil)
	{
		if (HandyWindow)
		{
			wStorage = HandyWindow;
			HandyWindow = nil;
		}
		else if ((wStorage=NuPtr(sizeof(MyWindow)))==nil)
			return (nil);
	}
	WriteZero(wStorage, sizeof(MyWindow));
	
	(*centerFunction)(template);
	win = GetNewDialog(template, wStorage, behind);
	if (win==nil) return(nil);
	win->isDialog = win->isRunt = True;
	SetPort(win);
	win->contR = ((GrafPtr)win)->portRect;
	win->qWindow.refCon = CREATOR;
	return(win);
}

/************************************************************************
 * DoModelessEvent - handle (the result of) an event in a modeless dialog
 ************************************************************************/
Boolean DoModelessEvent(EventRecord *event)
{
	long select;
	Boolean result;
	DialogPtr dlog = FrontWindow();
	short item;
	char key = event->message & charCodeMask;
	
	/*
	 * check for cmdkey equivalents
	 */
	if (event->what==keyDown)
	{
		if (event->modifiers&cmdKey)
		{
			if (select=MyMenuKey(event))
			{
				DoMenu(FrontWindow(),select,event->modifiers);
				result = 0;
				goto done;
			}
			else if (key=='.')
			{
				result = (*((MyWindowPtr)dlog)->hit)(event,dlog,2);
				goto done;
			}
		}
		else if (key==returnChar || key==enterChar)
		{
				result = (*((MyWindowPtr)dlog)->hit)(event,dlog,1);
				goto done;
		}
	}

	/*
	 * do the event
	 */
	result = DialogSelect(event,&dlog,&item) ?
		(*((MyWindowPtr)dlog)->hit)(event,dlog,item) : False;
	
	/*
	 * if drawing, outline the ok button
	 */
	if (event->what==updateEvt || event->what==activateEvt) HiliteButtonOne(dlog);
	
	if (event->what==app4Evt) SFWTC = True;

done:
	EnableMenus(FrontWindow());
	return(result);
}

/************************************************************************
 * DoModelessEdit - handle the Edit menu for a modeless dialog
 ************************************************************************/
Boolean DoModelessEdit(MyWindowPtr win,short item)
{
	short err;
	switch(item)
	{
		case EDIT_CUT_ITEM:
			DlgCut(win);
			if ((err=ZeroScrap()) || (err=TEToScrap()))
				WarnUser(COPY_FAILED,err);
			break;
		case EDIT_COPY_ITEM:
			DlgCopy(win);
			if ((err=ZeroScrap()) || (err=TEToScrap()))
				WarnUser(COPY_FAILED,err);
			break;
		case EDIT_PASTE_ITEM:
			if (InfoScrap()->scrapSize == 0)
				SysBeep(20);
			else if (err=TEFromScrap())
				WarnUser(PASTE_FAILED,err);
			else
				DlgPaste(win);
			break;
		case EDIT_CLEAR_ITEM:
			DlgDelete(win);
			break;
		case EDIT_SELECT_ITEM:
			SelIText(win,((DialogPeek)win)->editField+1,0,INFINITY);
			break;
		case EDIT_UNDO_ITEM:
			break;
		default:
			return(False);
	}
	return(True);
}
