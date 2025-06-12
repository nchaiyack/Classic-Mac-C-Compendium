#define FILE_NUM 11
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Menu
	void SetQItemText(MyWindowPtr win);
	Boolean DoIndependentMenu(int menu,int item,short modifiers);
	Boolean DoDependentMenu(WindowPtr topWin,int menu,int item,short modifiers);
	void BuildWindowsMenu(void);
	void SetCheckItem(void);
	Boolean SameSettingsFile(short vRef,long dirId,UPtr name);
	void InsertRecipient(MyWindowPtr win, short which,Boolean all);
#define TICKS2MINS 3600

/**********************************************************************
 * DoMenu - handle a menu selection
 **********************************************************************/
void DoMenu(WindowPtr topWin,long selection,short modifiers)
{
	short menu = (selection >> 16) & 0xffff;
	short item = selection & 0xffff;
	
	if (LOG_MENU&LogLevel)
	{
		Str31 mStr,iStr;
		PCopy(mStr,"\pnone");
		PCopy(iStr,mStr);
		if (menu)
		{
			PCopy(mStr,(*GetMHandle(menu))->menuData);
			if (item) MyGetItem(GetMHandle(menu),item,iStr);
		}
		ComposeLogS(LOG_MENU,nil,"\p%p %p",mStr,iStr);
	}
	if (menu && item)
	{
		PushCursor(watchCursor);
		if (!(topWin && ((WindowPeek)topWin)->windowKind==dialogKind &&
					menu==EDIT_MENU && DoModelessEdit(topWin,item)) &&
				!DoDependentMenu(topWin,menu,item,modifiers) &&
				!DoIndependentMenu(menu,item,modifiers))
				/*NotImplemented()*/;
		PopCursor();
  }
	HiliteMenu(0);
}

/**********************************************************************
 * DoIndependentMenu - handle a menu selection that does not depend on
 * what window is frontmost.	returns True if selection handled, False else
 **********************************************************************/
Boolean DoIndependentMenu(int menu,int item,short modifiers)
{
	Str255 scratch;
	short function;
	long dirId;
	MenuHandle mh=GetMHandle(menu);
	MyWindowPtr newWin;
	MyWindowPtr win=FrontWindow();
	TEHandle teh = WinTEH(win);
	HFileInfo hfi;
	
	switch (menu)
	{
		case APPLE_MENU:
			switch (item)
			{
				case APPLE_ABOUT_ITEM:
					DoAboutUIUCmail();
					return(True);
				default:
					GetItem(mh,item,scratch);
					(void) OpenDeskAcc(scratch);
					return(True);
			}
			break;

		case EDIT_MENU:
			switch (item)
			{
				case EDIT_SHOW_ITEM:
					ShowInsertion(win,InsertAny);
					return(True);
					break;
				case EDIT_FINISH_ITEM:
					if (IsMyWindow(win))
						FinishAlias(win,(modifiers&optionKey)!=0,False);
					return(True);
					break;
				case EDIT_UNDO_ITEM:
					DoUndo();
					SelectWindow(Undo.win);
					ShowInsertion(Undo.win,InsertAny);
					return(True);
					break;
				case EDIT_SELECT_ITEM:
					{
						if (teh)
						{
							NoScrollTESetSelect(0,INFINITY,teh);
							win->hasSelection = (*teh)->selStart != (*teh)->selEnd;
							return(True);
						}
					}
					break;
			}
			break;

		case FILE_MENU:
			switch (item)
			{
				case FILE_NEW_ITEM:
					(void) OpenText(0,"",nil,True,GetRString(scratch,UNTITLED));
					return(True);
					break;
				case FILE_OPEN_ITEM:
					{
						Str31 name;
						long dirId;
						short vRef;
						SFTypeList tl;
						short numTypes;
						if (modifiers&optionKey)
							numTypes = -1;
						else
						{
							numTypes = 2;
							tl[0] = 'TEXT';
							tl[1] = SETTINGS_TYPE;
						}
						if (MySFGetFile(nil,&dirId,&vRef,name,numTypes,tl))
						{
							FInfo info;
							short err;
							if (err=HGetFInfo(vRef,0L,name,&info))
								FileSystemError(TEXT_READ,name,err);
							else if (info.fdType==SETTINGS_TYPE && info.fdCreator==CREATOR)
							{
								if (!SameSettingsFile(vRef,0L,name))
									OpenNewSettings(vRef,0L,name);
							}
							else
								OpenText(vRef,name,nil,True,nil);
						}
						return(True);
					}
					break;
				case FILE_CLOSE_ITEM:
					if (modifiers&optionKey)
					{
						CloseAll();
						return(True);
					}
					else if (IsMyWindow(FrontWindow()))
					{
						CloseMyWindow(FrontWindow());
						return(True);
					}
					else
						return(False);
					break;
				case FILE_SEND_ITEM:
					DoSendQueue();
					return(True);
				case FILE_CHECK_ITEM:
					CheckForMail((modifiers&shiftKey)!=0);
					return(True);
				case FILE_PAGE_ITEM:
					DoPageSetup();
					return(True);
				case FILE_QUIT_ITEM:
					DoQuit();
					return(True);
				case FILE_BEHIND_ITEM:
					if (FrontWindow()) SendBehind(FrontWindow(),nil);
					return(True);
			}
			break;
		case NEW_TO_HIER_MENU:
			if (newWin=DoComposeNew(item))
			{
				MessType **messH =(MessType **)newWin->qWindow.refCon;
				ShowMyWindow(newWin);
			}
			return(True);
			break;
			
		case FIND_HIER_MENU:
			DoFind(item);
			return(True);
			break;
		
		case MESSAGE_MENU:
			switch(item)
			{
				case MESSAGE_NEW_ITEM:
				if (newWin=DoComposeNew(0))
					ShowMyWindow(newWin);
				return(True);
				break;
			}
			break;
			
		case MAILBOX_MENU:
			MyGetItem(mh,item,scratch);
			(void) GetMailbox(MyDirId,scratch,True);
			return(True);
			break;

		case SPECIAL_MENU:
			switch (item)
			{
				case SPECIAL_ALIASES_ITEM:
					OpenAliases();
					return(True);
				case SPECIAL_NPREFS_ITEM:
					DoNetPreferences();
					return(True);
				case SPECIAL_UPREFS_ITEM:
					DoUserPreferences();
					return(True);
				case SPECIAL_SIG_ITEM:
				{
					short shortjunk; long longjunk;
					if (!GetFileByRef(SettingsRefN,&shortjunk,&longjunk,scratch))
						(void) OpenText(MyVRef,scratch,nil,
													True,GetRString(scratch+64,SIGNATURE));
					return(True);
				}
				case SPECIAL_CTB_ITEM:
					(void) InitCTB(True);
					return(True);
				case SPECIAL_PH_ITEM:
					OpenPh();
					return(True);
				case SPECIAL_FORGET_ITEM:
#ifndef KERBEROS
					InvalidatePasswords(False,False);
#else
					{
								extern void 		forgetsession(char *name, char *inst, char *realm);
								Str63 userName, args;
								char instance[INST_SZ];
								char realm[REALM_SZ];
								
								GetPOPInfo(userName,args);
								p2cstr(userName);
								instance[0] = 0;
								realm[0] = 0;
								if(!krb_get_lrealm(realm, 1)) {
												forgetsession((char *)userName, instance, realm);
												}
								}
								POPSecure = False;
#endif
					return(True);
				case SPECIAL_TRASH_ITEM:
					EmptyTrash();
					return(True);
				case SPECIAL_COMPACT_ITEM:
					RememberOpenWindows();
					GetRString(scratch,TOC_SUFFIX);
					DoCompact(MyDirId,&hfi,*scratch);
					if (!FrontWindow()) RecallOpenWindows();
					return(True);
				case SPECIAL_ADD_TO_ITEM:
					AddSelectionAsTo();
					return(True);
				case SPECIAL_MAILBOX_ITEM:
					OpenMBWin();
					return(True);
				case SPECIAL_CHANGE_ITEM:
					ChangePassword();
					return(True);
			}
			break;
		case REMOVE_TO_HIER_MENU:
			{
				short hM;
				for (hM=NEW_TO_HIER_MENU;hM<=INSERT_TO_HIER_MENU;hM++)
					DelMenuItem(GetMHandle(hM),item);
				ToMenusChanged();
			}
			return(True);
			break;
		case INSERT_TO_HIER_MENU:
			if (IsMyWindow(FrontWindow()))
				InsertRecipient(FrontWindow(),item,(modifiers&optionKey)!=0);
			return(True);
			break;
		case WINDOW_MENU:
			HandleWindowChoice(item);
			break;
#ifdef DEBUG
		case DEBUG_MENU:
			{
				short theMark;
				GetItemMark(mh,item,&theMark);
				if (theMark)
				{
					SetItemMark(mh,item,0);
					BugFlags &= ~(1L<<(item-1));
				}
				else
				{
					SetItemMark(mh,item,checkMark);
					BugFlags |= (1L<<(item-1));
				}
				return(True);
			}
			break;
#endif
		default:
			dirId = (*BoxMap)[menu % MAX_BOX_LEVELS];
			function = (menu-1)/MAX_BOX_LEVELS;
			switch (function)
			{
				case MAILBOX:
					MyGetItem(mh,item,scratch);
					(void) GetMailbox(dirId,scratch,True);
					return(True);
				case TRANSFER:
					break;
			}
	}

	return(False);
}

/**********************************************************************
 * DoDependentMenu - handle a menu selection whose behavior depends
 * on what window is frontmost.  returns True if item handled, False else
 **********************************************************************/
Boolean DoDependentMenu(WindowPtr topWin,int menu,int item,short modifiers)
{
	if (topWin != nil)
	{
		if (((WindowPeek)topWin)->windowKind < 0)
		{
			switch (menu)
			{
				case EDIT_MENU:
					switch(item)
					{
						case EDIT_UNDO_ITEM:
						case EDIT_CUT_ITEM:
						case EDIT_COPY_ITEM:
						case EDIT_PASTE_ITEM:
							(void) SystemEdit(item-1);
							return(True);
						case EDIT_CLEAR_ITEM:
							(void) SystemEdit(item-2);
							return(True);
					}
					break;
				case FILE_MENU:
					if (item==FILE_CLOSE_ITEM)
					{
						CloseDeskAcc(((WindowPeek)topWin)->windowKind);
						return(True);
					}
					break;
			}
		}
		else if ((((WindowPeek)topWin)->windowKind>userKind ||
			 ((WindowPeek)topWin)->windowKind==dialogKind) &&
			((MyWindowPtr)topWin)->menu)
			return((*((MyWindowPtr)topWin)->menu)(topWin,menu,item,modifiers));
	}
	return(False);
}

/**********************************************************************
 * EnableMenuItems - enable menu items, depending on the type and state of
 * the topmost window
 **********************************************************************/
void EnableMenuItems(void)
{
	int kind;
	Boolean dirty = False, selection = False;
	MenuHandle mh;
	WindowPtr qdWin = FrontWindow();
	MyWindowPtr mw = (MyWindowPtr)qdWin;
	Boolean hasTx=False, ro=False;
	Boolean scrapFull = InfoScrap()->scrapSize > 0;
	Boolean outside = False;
	Boolean curMessage = False;
	Boolean force = False;
	Boolean compRo = False;
	Str63 txt;

	/*
	 * figure out just what's in front
	 */
	if (qdWin==(WindowPtr)(-1)) {qdWin = nil; force = True;}
	if (qdWin==nil)
	{
		kind=0;
		dirty=False; selection=False; hasTx=False; ro = True;
	}
	else
	{
		kind = ((WindowPeek)qdWin)->windowKind;
		if (kind >= userKind)
		{
			dirty = mw->isDirty;
			selection = mw->hasSelection;
			hasTx = WinTEH(mw)!=nil;
			ro = mw->ro;
			if (kind==COMP_WIN)
				compRo = SumOf(Win2MessH(mw))->state==SENT;
			
		}
	}
	
	/*
	 * enable menus accordingly, if anything has changed
	 */
	curMessage = kind==MESS_WIN || kind==COMP_WIN ||
								((kind==MBOX_WIN || kind==CBOX_WIN) && selection);
	outside = curMessage && (kind==MESS_WIN || kind==MBOX_WIN);
	mh = GetMHandle(FILE_MENU);
	EnableIf(mh,FILE_CLOSE_ITEM,kind!=0);
	EnableIf(mh,FILE_SAVE_ITEM,dirty && kind!=MBOX_WIN);
	EnableIf(mh,FILE_SAVE_AS_ITEM,curMessage||kind==TEXT_WIN);
	EnableIf(mh,FILE_PRINT_ITEM,curMessage||kind==PH_WIN||kind==TEXT_WIN);
	EnableIf(mh,FILE_PRINT_SELECT_ITEM,selection && hasTx && (curMessage||kind==PH_WIN||kind==TEXT_WIN));
	EnableIf(mh,FILE_BEHIND_ITEM,mw!=nil);
	
	mh = GetMHandle(EDIT_MENU);
	if (kind<userKind)
	{
		Str63 scratch;
		GetRString(scratch,UNDO);
		SetItem(mh,EDIT_UNDO_ITEM,scratch);
		EnableIf(mh,EDIT_UNDO_ITEM,mw!=nil);
	}
	else
		SetUndoMenu();
	EnableIf(mh,EDIT_CUT_ITEM,mw && (kind<userKind || selection && !ro));
	EnableIf(mh,EDIT_COPY_ITEM,mw && (kind<userKind || selection));
	EnableIf(mh,EDIT_PASTE_ITEM,mw && (scrapFull &&
			(!ro && hasTx) || kind<userKind));
	EnableIf(mh,EDIT_QUOTE_ITEM,scrapFull && !ro && hasTx && kind==COMP_WIN);
	EnableIf(mh,EDIT_CLEAR_ITEM,mw && (kind<userKind || selection &&
																(!ro || kind==COMP_WIN)));
	EnableIf(mh,EDIT_WRAP_ITEM,mw && selection && !ro && (kind==TEXT_WIN || kind==COMP_WIN));
	EnableIf(mh,EDIT_SELECT_ITEM,mw!=nil);
	EnableIf(mh,EDIT_SHOW_ITEM,hasTx);
	EnableIf(mh,EDIT_INSERT_TO_ITEM,hasTx&&!ro);
	EnableIf(mh,EDIT_SORT_ITEM,kind==MBOX_WIN||kind==CBOX_WIN);
	EnableIf(mh,EDIT_FINISH_ITEM,hasTx&&!ro);
	
	EnableFindMenu();
			
	mh = GetMHandle(MESSAGE_MENU);
	GetRString(txt,PrefIsSet(PREF_REPLY_ALL)?REPLY_ALL:REPLY);
	SetItem(mh,MESSAGE_REPLY_ITEM,txt);
	EnableIf(mh,MESSAGE_REPLY_ITEM,outside);
	EnableIf(mh,MESSAGE_FORWARD_ITEM,kind==MESS_WIN || kind==COMP_WIN
									|| (kind==MBOX_WIN || kind==CBOX_WIN)&&selection);
	EnableIf(mh,MESSAGE_REDISTRIBUTE_ITEM,outside);
	EnableIf(mh,MESSAGE_SALVAGE_ITEM,kind==MESS_WIN || kind==COMP_WIN
									|| (kind==MBOX_WIN || kind==CBOX_WIN)&&selection);
	EnableIf(mh,MESSAGE_REPLY_TO_ITEM,outside);
	EnableIf(mh,MESSAGE_FORWARD_TO_ITEM,kind==MESS_WIN || kind==COMP_WIN
									|| (kind==MBOX_WIN || kind==CBOX_WIN)&&selection);
	EnableIf(mh,MESSAGE_REDIST_TO_ITEM,outside);
	EnableIf(mh,MESSAGE_QUEUE_ITEM,
										kind==COMP_WIN || kind==CBOX_WIN&&selection);
	EnableIf(mh,MESSAGE_MOD_Q_ITEM,
										kind==COMP_WIN || kind==CBOX_WIN&&selection);
	EnableIf(mh,MESSAGE_DELETE_ITEM,curMessage);
	EnableIf(mh,MESSAGE_ATTACH_ITEM,kind==COMP_WIN&&!compRo);
		
	EnableIf(GetMHandle(TRANSFER_MENU),0,curMessage);

	mh = GetMHandle(SPECIAL_MENU);
	EnableIf(mh,SPECIAL_ADD_TO_ITEM,hasTx && selection);
	EnableIf(mh,SPECIAL_CTB_ITEM,UseCTB);
	EnableIf(mh,SPECIAL_FORGET_ITEM,POPSecure || *Password || *SecondPass);
	EnableIf(mh,SPECIAL_MAKE_NICK_ITEM,curMessage||CanMakeNick());
	
	/*
	 * this is a maverick menu item
	 */
	EnableIf(GetMHandle(FILE_MENU),FILE_SEND_ITEM,SendQueue>0);
	if (kind==COMP_WIN || kind==CBOX_WIN) SetQItemText(mw);
		
	/*
	 * check the mailbox represented by the topmost window
	 */
	CheckBox(qdWin);
	
	/*
	 * build the Windows menu
	 */
	BuildWindowsMenu();
	
	/*
	 * and the mailbox check item...
	 */
	SetCheckItem();
}

/**********************************************************************
 * EnableMenus - enable menus, depending on the type and state of
 * the topmost window
 **********************************************************************/
void EnableMenus(WindowPtr qdWin)
{
	Boolean mBar = False;
	MyWindowPtr mw = (MyWindowPtr)qdWin;
	Boolean curMessage = False;
	short kind;

	/*
	 * figure out just what's in front
	 */
	if (qdWin!=nil)
	{
		kind = ((WindowPeek)qdWin)->windowKind;
		if (kind >= userKind)
		{
			curMessage = kind==MESS_WIN || kind==COMP_WIN ||
								 ((kind==MBOX_WIN || kind==CBOX_WIN) && mw->hasSelection);
		}
	}
	
	/*
	 * enable menus accordingly, if anything has changed
	 */
	mBar=EnableIf(GetMHandle(FILE_MENU),0,True)||mBar;
	mBar=EnableIf(GetMHandle(EDIT_MENU),0,True)||mBar;
	mBar=EnableIf(GetMHandle(MESSAGE_MENU),0,True)||mBar;
	mBar=EnableIf(GetMHandle(TRANSFER_MENU),0,curMessage)||mBar;
	mBar=EnableIf(GetMHandle(SPECIAL_MENU),0,True)||mBar;
	mBar=EnableIf(GetMHandle(WINDOW_MENU),0,qdWin!=nil)||mBar;
	
	/*
	 * draw the menu bar if that's necessary
	 */
	if (mBar) DrawMenuBar();
}

/************************************************************************
 * SetQItemText - set the text of the Queue For Delivery menu item
 ************************************************************************/
void SetQItemText(MyWindowPtr win)
{
	TOCHandle tocH = nil;
	int sumNum = -1;
	
	if (win->qWindow.windowKind==COMP_WIN)
	{
		tocH = (*(MessType **)win->qWindow.refCon)->tocH;
		sumNum = (*(MessType **)win->qWindow.refCon)->sumNum;
	}
	else if (win->qWindow.windowKind==CBOX_WIN)
	{
		tocH = (TOCType **)win->qWindow.refCon;
		for (sumNum=0;sumNum<(*tocH)->count;sumNum++)
			if ((*tocH)->sums[sumNum].selected) break;
	}
	if (tocH && sumNum >= 0)
	{
		if ((*tocH)->sums[sumNum].state==SENT)
		{
			DisableItem(GetMHandle(MESSAGE_MENU),MESSAGE_QUEUE_ITEM);
			DisableItem(GetMHandle(MESSAGE_MENU),MESSAGE_MOD_Q_ITEM);
		}
		else
		{
			EnableItem(GetMHandle(MESSAGE_MENU),MESSAGE_QUEUE_ITEM);
			EnableItem(GetMHandle(MESSAGE_MENU),MESSAGE_MOD_Q_ITEM);
		}
	}
}

/**********************************************************************
 * CheckBox - check the topmost mailbox, and disable in transfer menu
 **********************************************************************/
void CheckBox(MyWindowPtr win)
{
	TOCHandle tocH;
	Str31 name;
	short n;
	
	if (CheckedMenu && CheckedItem)
	{
		SetItemMark(CheckedMenu,CheckedItem,0);
		CheckedMenu = nil;
	}
		
	if (!IsMyWindow(win)) return;
	if (win->qWindow.windowKind==MBOX_WIN)
		tocH = (TOCHandle) win->qWindow.refCon;
	else if (win->qWindow.windowKind==MESS_WIN)
		tocH = (*(MessHandle) win->qWindow.refCon)->tocH;
	else
	{
		if (win->qWindow.windowKind==CBOX_WIN ||
					 win->qWindow.windowKind==COMP_WIN)
			SetItemMark(CheckedMenu=GetMHandle(MAILBOX_MENU),
								CheckedItem=MAILBOX_OUT_ITEM,checkMark);
		return;
	}
	
	if ((*tocH)->which==IN)
		SetItemMark(CheckedMenu=GetMHandle(MAILBOX_MENU),
								CheckedItem=MAILBOX_IN_ITEM,checkMark);
	else if ((*tocH)->which==TRASH)
		SetItemMark(CheckedMenu=GetMHandle(MAILBOX_MENU),
								CheckedItem=MAILBOX_TRASH_ITEM,checkMark);
	else
	{
		for (n=0;;n++) if ((*BoxMap)[n]==(*tocH)->dirId) break;
		PCopy(name,(*tocH)->name);
		CheckedMenu=GetMHandle(n?n:MAILBOX_MENU);
		CheckedItem=FindItemByName(CheckedMenu,name);
		SetItemMark(CheckedMenu,CheckedItem,checkMark);
	}
}

/************************************************************************
 * EnableIf - enable a menu item based on an expression.
 * Returns True is the menu bar needs to be redrawn.
 ************************************************************************/
Boolean EnableIf(MenuHandle mh, short item, Boolean expr)
{ 
	Boolean result; 

	expr = expr ? 1 : 0;					/* normalize */
	result = item==0 && ((((*mh)->enableFlags&1)==1)!=(expr));
	if (expr)
		EnableItem(mh,item);
	else
		DisableItem(mh,item);
	return (result);
}

/************************************************************************
 * SetCheckItem - put the time of the next mail check in the Check Mail
 * item.
 ************************************************************************/
void SetCheckItem(void)
{
	Str63 item;
	unsigned ivalTicks;
	short n;
	
	GetRString(item,CHECK_MAIL);
	if (CheckTicks && (ivalTicks=TICKS2MINS*GetRLong(PREF_STRN+PREF_INTERVAL)))
	{
		PCat(item,"\p ");
		n = *item+1;
		IUTimeString(LocalDateTime()+(CheckTicks+ivalTicks-TickCount())/60,
								 False,item+n);
		item[0] += item[n]+1;
		item[n] = '(';
		PCatC(item,')');
	}
	SetItem(GetMHandle(FILE_MENU),FILE_CHECK_ITEM,item);
}

/************************************************************************
 * InsertRecipient - insert a given recipient in a window
 ************************************************************************/
void InsertRecipient(MyWindowPtr win, short which,Boolean all)
{
	UPtr spot;
	Str63 scratch;
	
	Undo.didClick=True;
	InsertCommaIfNeedBe(win);
	MyGetItem(GetMHandle(NEW_TO_HIER_MENU),which,scratch);
	for (spot=scratch+1;spot<scratch+*scratch+1;spot++)
		TESomething(win,TEKEY,*spot,0);
	if (all) FinishAlias(win,all,False);
}


/************************************************************************
 * BuildWindowsMenu
 ************************************************************************/
void BuildWindowsMenu(void)
{
	Str255 title;
	MyWindowPtr win;
	MenuHandle mh = GetMHandle(WINDOW_MENU);
	
	if (mh)
	{
		TrashMenu(mh,1);
		for (win=FrontWindow();win;win=win->qWindow.nextWindow)
		if (win->qWindow.visible)
			{
				GetWTitle(win,title);
				if (*title>31)
				{
					*title = 31;
					title[31] = 'É';
				}
				MyAppendMenu(mh,title);
				if (win->qWindow.windowKind==COMP_WIN)
					SetItemStyle(mh,CountMItems(mh),italic);
			}
	}
}


#pragma segment Ends
/************************************************************************
 *
 ************************************************************************/
Boolean SameSettingsFile(short vRef,long dirId,UPtr name)
{
#pragma unused(dirId)
	Str63 curName;
	short curVRef;
	long curDirId;
	
	if (GetFileByRef(SettingsRefN,&curVRef,&curDirId,curName)) return(False);
	
	return(MyVRef == vRef && EqualString(name,curName,False,True));
}
