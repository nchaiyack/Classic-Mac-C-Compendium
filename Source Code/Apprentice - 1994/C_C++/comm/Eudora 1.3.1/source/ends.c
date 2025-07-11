#define FILE_NUM 12
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * initialization and cleanup
 **********************************************************************/
#pragma load EUDORA_LOAD
#pragma segment Ends
	void SettingsInit(UPtr name);
	void OpenSettingsFile(UPtr name);
	void FindMyDirectory(UPtr name);
	void PutUpMenus(void);
	void CreateBoxes(void);
	void SetupInsurancePort(void);
	int GetAppFileVRef(UPtr name);
	void SetRunType(void);
	Boolean ModernSystem(void);
	void BoxFolder(long dirId,MenuHandle *menus, Str31 *badNames, short badCount,
								 UPtr newText, short *level);
	void RemoveBoxMenus(void);
	void MakeHmnusPurge(void);
	void TrashHmnu(void);
	void OpenPlugIns(void);
	Boolean UsingNewTables(void);
/**********************************************************************
 * Initialize - set up my stuff as well as the Mac managers
 **********************************************************************/
void Initialize(void)
{
	Str63 name;
	MacInitialize(16,20K); 			/* initialize mac managers */
	SetRunType(); 							/* figure out what kind of run this is */
	if (!ModernSystem()) WarnUser(OLD_SYSTEM,0);
	OpenPlugIns();
	FindMyDirectory(name);			/* find the mail folder */
	MakeGrow(SPARE_SIZE); 			/* reserve some memory for emergencies */
	SetGrowZone(GrowZone);			/* let the mac know it's there */
	SettingsInit(*name ? name:nil);
#ifdef PERF
	ThePGlobals = nil;
	if (!InitPerf(&ThePGlobals,10,8,True,True,"\pCODE",0,"",False,0,0,0))
		WarnUser(PERFORMANCE,1);
#endif
} 

/************************************************************************
 * SettingsInit - Initialize with a settings file
 ************************************************************************/
void SettingsInit(UPtr name)
{
	Str255 scratch;
	
	if (CurrentSize()+10K<EstimatePartitionSize()) MemoryWarning();
	POPSecure = False;
	OpenSettingsFile(name); 		/* open our settings file */
	AlertsTimeout = PrefIsSet(PREF_AUTO_DISMISS);
	UseCTB = PrefIsSet(PREF_TRANS_METHOD);
	CurTrans = UseCTB ? CTBTrans : TCPTrans;
	FigureOutFont();						/* figure out what font && size to use */
	CreateBoxes();							/* create In and Out boxes, for later use */
	PutUpMenus(); 							/* menus */
	PageSetup = nil;						/* remind me to go get that */
	TOCList = nil;							/* no open TOC's yet */
	MessList = nil; 						/* no open messages yet */
	SetSendQueue(); 						/* set SendQueue if there are queued messages */
	EnableMenus(nil); 					/* enable appropriate items */
	WrapWrong = CountResources('Rong') > 0;
	GetPref(scratch,PREF_POP); UUPCIn = *scratch && scratch[1]=='!';
	GetPref(scratch,PREF_SMTP); UUPCOut = *scratch && scratch[1]=='!';
	LogLevel = GetRLong(PREF_STRN+PREF_LOG);
	FakeTabs = PrefIsSet(PREF_TAB_IN_TEXT);
	NewTables = UsingNewTables();
	
	if (!(MouseRgn = NewRgn())) DieWithError(MEM_ERR,MemError());
	GetRString(NewLine,UseCTB ? CTB_NEWLINE : NEWLINE);

#ifndef KERBEROS
	/*
	 * saved password?
	 */
	if (PrefIsSet(PREF_SAVE_PASSWORD))
	{
		GetPref(Password,PREF_PASS_TEXT);
		GetPref(SecondPass,PREF_AUXPW);
	}
	else
		InvalidatePasswords(False,False);
#endif
	POPSecure = False;

	/*
	 * Bring Out Yer Dead!
	 */
	RecallOpenWindows();
	
	/*
	 * run initial check
	 */
	if (!EjectBuckaroo && GetRLong(PREF_STRN+PREF_INTERVAL) && !(CurrentModifiers()&optionKey))
		CheckForMail(True);
}

/************************************************************************
 * UsingNewTables - are we using the new translit table scheme?
 ************************************************************************/
Boolean UsingNewTables(void)
{
	Boolean found=False;
	Handle table;
	short ind=0;
	Str255 name;
	short id;
	ResType type;
	
	SetResLoad(False);
	while (table=GetIndResource('taBL',++ind))
	{
		GetResInfo(table,&id,&type,&name);
		if (found=(id<1001||id>1003)&&*name) break;
	}
	SetResLoad(True);
	return(found);
}

/************************************************************************
 * OpenNewSettings - open a new settings file, closing things out
 * properly first
 ************************************************************************/
void OpenNewSettings(short vRef,long dirId,UPtr name)
{
	Str31 folder;
	short menu;
	MenuHandle mHandle;
	
	/*
	 * a simulated Quit
	 */
	DoQuit();
	if (!AmQuitting) return;
	Done = False;
	
	/*
	 * let's get rid of some stuff
	 */
	RemoveBoxMenus();
	for (menu=APPLE_MENU;menu<MENU_LIMIT;menu++)
	{
		if (mHandle=GetMHandle(menu)) DisposeMenu(mHandle);
	}
	ClearMenuBar();
		
	ZapHandle(Aliases);
	
	/*
	 * Now, point us at the right directory
	 */
	MyVRef = vRef;
	MyDirId = dirId;
	HSetVol(nil,MyVRef,MyDirId);
	HGetVol(folder,&MyVRef,&MyDirId);
	MyVRef = GetMyWD(0,MyDirId);
	
	/*
	 * keep yer fingers crossed...
	 */
	SettingsInit(name);
}

/************************************************************************
 * SetRunType - figure out from the name of the app whether we're prod-
 * uction, testing, or my private version.	This is a HACK.
 ************************************************************************/
void SetRunType(void)
{
#ifdef DEBUG
	Str255 appName;
	short appRefNum;
	Handle apParam;
	
	GetAppParms(appName,&appRefNum,&apParam);
	
	switch (appName[1])
	{
		case 'P': RunType=Steve;break;
		case 'e': RunType=Debugging;break;
		default: RunType=Production;break;
	}
#else
	RunType=Production;
#endif DEBUG
}

/**********************************************************************
 * do what we need before exiting functions
 **********************************************************************/
void Cleanup()
{
#ifdef	KERBEROS
				extern void 		cplus_cleanup();
#endif
	TrashHmnu();
	if (SettingsRefN) CloseResFile(SettingsRefN);
	(void) CloseResolver();
	if (DestroyTrans) (void) DestroyTrans();
#ifndef SLOW_CLOSE
	TcpFastFlush(True);
#endif
	CloseLog();
	FlushVol(nil,MyVRef);
#ifdef	KERBEROS
				cplus_cleanup();
#endif
#ifdef PERF
	{
		int err;
		if (err=PerfDump(ThePGlobals,"\pPerform.Out",False,0))
			WarnUser(PERFORMANCE,err);
		TermPerf(ThePGlobals);
	}
#endif
}


/**********************************************************************
 * figure out what font and size to use
 **********************************************************************/
void FigureOutFont()
{
	Str255 fontName;
	Str255 applFontName;
	Str255 fontSizeStr;
	long aLong;
	
	/*
	 * which font?
	 */
	if (IUEqualString(GetPref(fontName,PREF_FONT_NAME),
						GetRString(applFontName,APPL_FONT)) ==0)
		FontID = 1;
	else
		FontID = GetFontID(fontName);
		
	/*
	 * how big?
	 */
	StringToNum(GetPref(fontSizeStr,PREF_FONT_SIZE),&aLong);
	FontSize = aLong;
	
	/*
	 * and how big is big?
	 */
	FontWidth = GetWidth(FontID,FontSize);
	FontLead = GetLeading(FontID,FontSize);
	FontDescent = GetDescent(FontID,FontSize);
	FontAscent = GetAscent(FontID,FontSize);
	FontIsFixed = IsFixed(FontID,FontSize);
	
	/*
	 * and get the widths of boxes
	 */
	GetBoxLines();
	
	/*
	 * create a spare port for necessities
	 */
	SetupInsurancePort();
}

/**********************************************************************
 * open (or create) the settings file
 **********************************************************************/
void OpenSettingsFile(UPtr name)
{
	Str255 settingsName;
	Handle appVersion;
	Handle prefVersion;
	short appRefN;
	int err;
	FInfo info;
	
	/*
	 * close it if it's open
	 */
	if (SettingsRefN) CloseResFile(SettingsRefN);
	
	/*
	 * we'll create it before we open it, to avoid the PMSP
	 */
	if (name && *name && !HGetFInfo(MyVRef,MyDirId,name,&info) && 
			info.fdType==SETTINGS_TYPE && info.fdCreator==CREATOR)
		PCopy(settingsName,name);
	else GetRString(settingsName,SETTINGS_FILE);
	if (err=MakeResFile(settingsName,MyVRef,MyDirId,CREATOR,SETTINGS_TYPE))
		DieWithError(CREATE_SETTINGS,err);
	
	/*
	 * grab some stuff from the application resource file first
	 */
	if ((appVersion = GetResource(CREATOR,1))==nil)
		DieWithError(READ_SETTINGS,ResError());
	appRefN = HomeResFile(appVersion);
	
	/*
	 * now, open the preferences file
	 */
	if ((SettingsRefN=OpenResFile(settingsName))==nil)
		DieWithError(OPEN_SETTINGS,ResError());
		
	/*
	 * copy the preferences into it if need be
	 * "need be" means either the resources aren't now there,
	 * or the program version has changed since last time the
	 * preferences file was written.
	 */
	if ((prefVersion = GetResource(CREATOR,1))==nil)
		DieWithError(READ_SETTINGS,ResError());
		
	if (HomeResFile(prefVersion)!=SettingsRefN ||
			!EqualString(*prefVersion,*appVersion,False,False))
	{
		ReleaseResource(prefVersion);
		if (err=ResourceCpy(SettingsRefN,appRefN,CREATOR,1))
			DieWithError(WRITE_SETTINGS,err);
		if (err=ResourceCpy(SettingsRefN,appRefN,'STR#',PREF_STRN))
			DieWithError(WRITE_SETTINGS,err);
		UpdateResFile(SettingsRefN);
		if (ResError()) DieWithError(WRITE_SETTINGS,ResError());
	}
	else
	{
		ReleaseResource(prefVersion);
		ReleaseResource(appVersion);
	}
}

/**********************************************************************
 * FindMyDirectory - find the directory for us in the system folder.
 * If it doesn't exist, create it.
 **********************************************************************/
void FindMyDirectory(UPtr name)
{
	DirInfo myDI;
	Str31 folder;
	int err=0;
	SysEnvRec env;
	
	MyDirId = 0;
	*folder = 0;
	if (!(MyVRef = GetAppFileVRef(name)))
	{
		SysEnvirons(ENVIRONS_VERSION,&env);
		
		/*
		 * now, look for our directory
		 */
#ifdef DEBUG
		GetRString(folder,RunType==Steve ? STEVE_FOLDER : FOLDER_NAME);
#else
		GetRString(folder,FOLDER_NAME);
#endif DEBUG
		myDI.ioNamePtr = folder;
		myDI.ioVRefNum = MyVRef = env.sysVRefNum;
		myDI.ioFDirIndex = 0;
		myDI.ioDrDirID = 0;
		err=PBGetCatInfo(&myDI,False);
		if (err==fnfErr)								/* it doesn't exist */
		{ 	
			if (err=DirCreate(MyVRef,0,folder,&MyDirId))
				DieWithError(CREATE_FOLDER,err);
		}
		else if (myDI.ioFlAttrib&0x10)
			MyDirId = myDI.ioDrDirID;
		else
			DieWithError(NO_PLAIN_OPEN,0);
	}
	if (err) DieWithError(MAIL_FOLDER,err);
	if (HSetVol(folder,MyVRef,MyDirId))
		HSetVol(nil,MyVRef,MyDirId);
	HGetVol(folder,&MyVRef,&MyDirId);
	MyVRef = GetMyWD(0,MyDirId);
}
	
/**********************************************************************
 * PutUpMenus - set up the menu bar
 **********************************************************************/
void PutUpMenus()
{
	int menu;
	MenuHandle mHandle,savedMHandle;
	Boolean touchedTo = False;
	Str63 scratch;
	
	/*
	 * add the standard ones
	 */
	for (menu=APPLE_MENU;menu<MENU_LIMIT;menu++)
	{
		if (mHandle=GetMHandle(menu)) DisposeMenu(mHandle);
		mHandle = GetMenu(menu+1000);
		if (mHandle==nil)
			DieWithError(GET_MENU,MemError());
		InsertMenu(mHandle,0);
	}
	if (mHandle=GetMHandle(WINDOW_MENU)) DisposeMenu(mHandle);
	mHandle = GetMenu(WINDOW_MENU+1000);
	if (mHandle==nil)
		DieWithError(GET_MENU,MemError());
	InsertMenu(mHandle,0);
#ifdef DEBUG
	if (RunType!=Production) InsertMenu(GetMenu(DEBUG_MENU+1000),0);
#endif
	
	/*
	 * take care of special menus
	 */
	mHandle = GetMHandle(APPLE_MENU);
	AddResMenu(mHandle,'DRVR');
	BuildBoxMenus();						/* build the menus that refer to Mailboxes */

	/*
	 * the user menus
	 */
	AttachHierMenu(MESSAGE_MENU,MESSAGE_NEW_TO_ITEM,NEW_TO_HIER_MENU);
	AttachHierMenu(MESSAGE_MENU,MESSAGE_REPLY_TO_ITEM,REPLY_TO_HIER_MENU);
	AttachHierMenu(MESSAGE_MENU,MESSAGE_FORWARD_TO_ITEM,FORWARD_TO_HIER_MENU);
	AttachHierMenu(MESSAGE_MENU,MESSAGE_REDIST_TO_ITEM,REDIST_TO_HIER_MENU);
	AttachHierMenu(EDIT_MENU,EDIT_INSERT_TO_ITEM,INSERT_TO_HIER_MENU);
	AttachHierMenu(SPECIAL_MENU,SPECIAL_REMOVE_TO_ITEM,REMOVE_TO_HIER_MENU);
	if (!(savedMHandle = GetMenu(NEW_TO_HIER_MENU+1000)))
	{
		if (savedMHandle = GetMenu(NEW_TO_HIER_MENU+900))
		{
			 SetResInfo(savedMHandle,NEW_TO_HIER_MENU+1000,"");
			 touchedTo = True;
		}
		else
		{
			if (savedMHandle = NewMenu(NEW_TO_HIER_MENU,""))
			{
				MyAppendMenu(savedMHandle,GetRString(scratch,S_DORNER));
				AddResource(savedMHandle,'MENU',NEW_TO_HIER_MENU+1000,"");
				touchedTo = True;
			}
			else
				DieWithError(GET_MENU,MemError());
		}
	}
	for (menu=NEW_TO_HIER_MENU;menu<=INSERT_TO_HIER_MENU;menu++)
	{
		mHandle = savedMHandle;
		if (menu>NEW_TO_HIER_MENU) HandToHand(&mHandle);
		if (mHandle!=nil)
		{
			(*mHandle)->menuID=menu;
			InsertMenu(mHandle,-1);
		}
		else
			DieWithError(GET_MENU,MemError());
	}
	if (touchedTo) ToMenusChanged();
	
	/*
	 * hier menus
	 */
	mHandle = GetMenu(FIND_HIER_MENU+1000);
	if (mHandle==nil) DieWithError(GET_MENU,menu);
	InsertMenu(mHandle,-1);
	mHandle = GetMenu(SORT_HIER_MENU+1000);
	if (mHandle==nil) DieWithError(GET_MENU,menu);
	InsertMenu(mHandle,-1);
	
	/*
	 * draw the menu bar
	 */
	DrawMenuBar();
	
	/*
	 * install the menuhook
	 */
#ifdef DEBUG
	if (RunType != Production)
		*(long *) 0xa30 = (long)Hook;
#endif
}

/**********************************************************************
 * BuildBoxMenus - build the mailbox menus according to the files in
 * our mailbox folder.
 **********************************************************************/
void BoxFolder(long dirId,MenuHandle *menus,Str31 *badNames, short badCount,
							 UPtr newText, short *level)
{
	typedef struct
		{Str31 name; Boolean isDir; long dirId;} NameType, *NamePtr, **NameHandle;
	HFileInfo hfi;
	NameType thisOne;
	NameHandle names;
	short count=0,i,item,dirItem;
	MenuHandle newMenus[MENU_ARRAY_LIMIT];
	Str31 namePtrFodder;
	Str15 suffix;
	Str15 tmpsuffix;
	
	/*
	 * make sure we're kosher
	 */
	if (*level>MAX_BOX_LEVELS)
		DieWithError(TOO_MANY_LEVELS,*level);
	PtrAndHand(&dirId,BoxMap,sizeof(long));

	/*
	 * get started
	 */
	if ((names=NuHandle(0L))==nil)
		DieWithError(ALLO_MBOX_LIST,MemError());
	hfi.ioNamePtr = namePtrFodder;
	hfi.ioFDirIndex = 0;
	GetRString(suffix,TOC_SUFFIX);
	GetRString(tmpsuffix,TEMP_SUFFIX);
	/*
	 * read names of mailbox files
	 */
	while (!DirIterate(MyVRef,dirId,&hfi))
	{
		if (dirId==MyDirId)
			for (i=0;i<badCount;i++)
				if (EqualString(badNames[i],hfi.ioNamePtr,False,True)) goto nextfile;
		if (!(hfi.ioFlAttrib&0x10))
		{
			if (hfi.ioFlFndrInfo.fdType!=MAILBOX_TYPE) continue;
			if (*hfi.ioNamePtr>*tmpsuffix && EndsWith(hfi.ioNamePtr,tmpsuffix))
				TempWarning(hfi.ioNamePtr);
			if (*hfi.ioNamePtr>31-*suffix)
			{
				TooLong(hfi.ioNamePtr);
				continue;
			}
		}
		PCopy(thisOne.name,hfi.ioNamePtr);
		thisOne.isDir = hfi.ioFlAttrib&0x10;
		thisOne.dirId = hfi.ioDirID;
		PtrAndHand (&thisOne,names,sizeof(thisOne));
		if (MemError()) DieWithError(ALLO_MBOX_LIST,MemError());
		count++;
		nextfile:;
	}
	
	/*
	 * add the New item
	 */
 AppendMenu(menus[TRANSFER],newText);

	/*
	 * stick them into the proper menus
	 */
	for (item=0;item<count;item++)
	{
		thisOne = (*names)[item];
		if (thisOne.isDir) continue;
		MyAppendMenu(menus[MAILBOX],thisOne.name);
		GetRString(thisOne.name,TRANSFER_PREFIX);
		PCat(thisOne.name,(*names)[item].name);
		MyAppendMenu(menus[TRANSFER],thisOne.name);
	}
	for (item=0;item<count;item++)
	{
		thisOne = (*names)[item];
		if (!thisOne.isDir) continue;
		MyAppendMenu(menus[MAILBOX],thisOne.name);
		GetRString(thisOne.name,TRANSFER_PREFIX);
		PCat(thisOne.name,(*names)[item].name);
		MyAppendMenu(menus[TRANSFER],thisOne.name);
		thisOne = (*names)[item];
		for (i=0;i<MENU_ARRAY_LIMIT;i++)
		{
			dirItem = i*MAX_BOX_LEVELS+*level+1;
			if (!(newMenus[i] = NewMenu(dirItem,thisOne.name)))
				DieWithError(ALLO_MBOX_LIST,MemError());
			AttachHierMenu((*menus[i])->menuID,CountMItems(menus[i]),dirItem);
			InsertMenu(newMenus[i],-1);
			if (HasHelp && *level && !Get1Resource('hmnu',dirItem))
			{
				Handle mHand=GetResource('hmnu',i*MAX_BOX_LEVELS+1);
				Handle newHand=mHand;
				if (mHand && !HandToHand(&newHand))
				{
					AddResource(newHand,'hmnu',dirItem,"");
					SetResAttrs(newHand,resChanged|resPurgeable);
				}
			}
		}
		(*level)++;
		BoxFolder(thisOne.dirId,newMenus,badNames,badCount,newText,level);
		if (MemError()) DieWithError(ALLO_MBOX_LIST,MemError());
	}
	DisposHandle(names);
}

/************************************************************************
 * TempWarning - warn the user about a temp file in his mailbox structure
 ************************************************************************/
void TempWarning(UPtr filename)
{
	Str255 msg;
	GetRString(msg,TEMP_WARNING);
	MyParamText(msg,filename,"","");
	ReallyDoAnAlert(OK_ALRT,Caution);
}

/**********************************************************************
 * BuildBoxMenus - build the mailbox menus according to the files in
 * our mailbox folder.
 **********************************************************************/
void BuildBoxMenus()
{
	short badIndices[] = {IN,OUT,ALIAS_FILE,TRASH,LOG_NAME,OLD_LOG};
	Str31 badNames[sizeof(badIndices)/sizeof(short)];
	Str31 newText;
	MenuHandle menus[MENU_ARRAY_LIMIT];
	short level=0,i;
	
	if (BoxMap!=nil) RemoveBoxMenus();
	if ((BoxMap=NuHandle(0L))==nil)
		DieWithError(ALLO_MBOX_LIST,MemError());
	for (i=0;i<sizeof(badIndices)/sizeof(short);i++)
	  GetRString(badNames[i],badIndices[i]);
	GetRString(newText,NEW_ITEM_TEXT);
			
	/*
	 * stick them into the proper menus
	 */
	menus[MAILBOX] = GetMHandle(MAILBOX_MENU);
	menus[TRANSFER] = GetMHandle(TRANSFER_MENU);
	
	/*
	 * now, do the recursive thing
	 */
	BoxFolder(MyDirId,menus,badNames,sizeof(badNames)/sizeof(Str31),
						newText,&level);
	if (HasHelp) MakeHmnusPurge();
	BuildBoxCount();
	DrawMenuBar();
}

/************************************************************************
 * RemoveBoxMenus - get a clean slate
 ************************************************************************/
void RemoveBoxMenus(void)
{
	DisposHandle(BoxMap); BoxMap = nil;
	TrashHmnu();
	CheckBox(nil);
	TrashMenu(GetMHandle(MAILBOX_MENU),MAILBOX_BAR1_ITEM+1);
	TrashMenu(GetMHandle(TRANSFER_MENU),TRANSFER_NEW_ITEM);
}

/************************************************************************
 * TrashHmnu - get rid of the help resources for subfolders
 ************************************************************************/
void TrashHmnu(void)
{
	short i,n;
	Handle hHand;
	
	n = GetHandleSize(BoxMap)/sizeof(long);
	SetResLoad(False);
	for (i=2;i<=n;i++)
	{
		if (hHand=Get1Resource('hmnu',i))
		{
			RmveResource(hHand);
			DisposHandle(hHand);
		}
		if (hHand=Get1Resource('hmnu',i+MAX_BOX_LEVELS))
		{
			RmveResource(hHand);
			DisposHandle(hHand);
		}
	}
	SetResLoad(True);
	UpdateResFile(CurResFile());
}

/************************************************************************
 * MakeHmnusPurge - make sure we can purge the Hmnus
 ************************************************************************/
void MakeHmnusPurge(void)
{
	short i,n;
	Handle hHand;
	
	/*
	 * first off, get them written out
	 */
	UpdateResFile(CurResFile());
	
	/*
	 * trash 'em
	 */
	n = GetHandleSize(BoxMap)/sizeof(long);
	SetResLoad(False);
	for (i=1;i<=n;i++)
	{
		if (hHand=Get1Resource('hmnu',i))
			ReleaseResource(hHand);
		if (hHand=Get1Resource('hmnu',i+MAX_BOX_LEVELS))
			ReleaseResource(hHand);
	}
	SetResLoad(True);
}

/************************************************************************
 * TrashMenu - get rid of a menu, beginning at a particular item
 ************************************************************************/
void TrashMenu(MenuHandle mh, short beginAtItem)
{
	short count,id,item;
	MenuHandle subMh;
	
	if (!mh) return;
	for (count=CountMItems(mh),item=beginAtItem?beginAtItem:1;item<=count;item++)
		if (HasSubmenu(mh,item))
		{
			GetItemMark(mh,item,&id);
			subMh = GetMHandle(id);
			TrashMenu(subMh,0);
			SetItemCmd(mh,item,0);
			DeleteMenu(id);
			DisposeMenu(subMh);
		}
	if (beginAtItem>0)
		while (count>=beginAtItem) DelMenuItem(mh,count--);
}

/**********************************************************************
 * CreateBoxes - Create the "In" and "Out" mailboxes if need be
 **********************************************************************/
void CreateBoxes()
{
	Str63 name;
	int err;
	
	GetRString(name,IN);
	if (err=MakeResFile(name,MyVRef,MyDirId,CREATOR,MAILBOX_TYPE))
		DieWithError(CREATING_MAILBOX,err);
	GetRString(name,OUT);
	if (err=MakeResFile(name,MyVRef,MyDirId,CREATOR,MAILBOX_TYPE))
		DieWithError(CREATING_MAILBOX,err);
	GetRString(name,TRASH);
	if (err=MakeResFile(name,MyVRef,MyDirId,CREATOR,MAILBOX_TYPE))
		DieWithError(CREATING_MAILBOX,err);
	GetRString(name,ALIAS_FILE);
	if (err=MakeResFile(name,MyVRef,MyDirId,CREATOR,'TEXT'))
		DieWithError(CREATING_ALIAS,err);
}

/**********************************************************************
 * GetBoxLines - read the horizontal widths of the various mailbox
 * data areas.
 **********************************************************************/
void GetBoxLines(void)
{
	Handle strn;
	int count;
	Str255 scratch;
	long rightSide;
	short last=0;
	short offset=0;
	short i;
	
	strn=GetResource('STR#',BOX_LINES_STRN);
	if (strn==nil) DieWithError(GENERAL,ResError());
	
	count = *((short *)*strn);
	
	if (!BoxLines) BoxLines = NuHandle(count*sizeof(short));
	if (BoxLines==nil) DieWithError(MEM_ERR,MemError());
	
	for(i=0;i<count;i++)
	{
		StringToNum(GetRString(scratch,BOX_LINES_STRN+i+1),&rightSide);
		if (rightSide<=last) {rightSide=last; offset+=3;}	/* for zero, make 3 pixels */
		last = rightSide;
		(*BoxLines)[i] = FontWidth*rightSide+offset;
	}
}

/************************************************************************
 * SetSendQueue - preload the SendQueue global with the number of queued
 * messages.
 ************************************************************************/
void SetSendQueue(void)
{
	TOCType **tocH;
	int sumNum;
	uLong gmtSecs = GMTDateTime();
	
	SendQueue = 0;
	ForceSend = 0xffffffff;
	if (!(tocH=GetOutTOC())) return;
	for (sumNum=0; sumNum<(*tocH)->count; sumNum++)
		if ((*tocH)->sums[sumNum].state == QUEUED)
		{
		  if ((*tocH)->sums[sumNum].seconds &&
			    (*tocH)->sums[sumNum].seconds < ForceSend)
				ForceSend = (*tocH)->sums[sumNum].seconds;
		  if ((*tocH)->sums[sumNum].seconds <= gmtSecs) SendQueue++;
		}
}

/************************************************************************
 * SetupInsurancePort - create a grafport for use when others are unavailable
 ************************************************************************/
void SetupInsurancePort(void)
{
	if (!InsurancePort)
	{
		InsurancePort = New(GrafPort);
		if (!InsurancePort) DieWithError(MEM_ERR,MemError());
		OpenPort(InsurancePort);
	}
	SetPort(InsurancePort);
	TextFont(FontID);
	TextSize(FontSize);
}

/************************************************************************
 * GetAppFileVRef - get the vrefn of the file that launched us
 ************************************************************************/
int GetAppFileVRef(UPtr name)
{
	short message,count;
	AppFile aFile;
	
	*name = 0;
	CountAppFiles(&message,&count);
	if (!count) return(0);
	
	GetAppFiles(1,&aFile);
	PCopy(name,aFile.fName);
	return(aFile.vRefNum);
}

/************************************************************************
 * ModernSystem - is the system 6.0.4 or better?
 ************************************************************************/
Boolean ModernSystem(void)
{
	SysEnvRec env;
	long resp;

	HasHelp = !Gestalt(gestaltHelpMgrAttr,&resp) &&
										resp&(1L<<gestaltHelpMgrPresent);
	HasPM = !Gestalt(gestaltOSAttr,&resp) && resp&(1L<<gestaltLaunchControl);
	if (HasPM && !Gestalt(gestaltAUXVersion, &resp))
		HasPM = (resp>>8)>=3;
	SysEnvirons(ENVIRONS_VERSION,&env);
	return(env.systemVersion >= 0x0604);
}

typedef struct
{
	short wType;					/* window type */
	long dirId; 					/* if any */
	short index;					/* if any */
	Str31 volName;				/* if any */
	Str31 name; 					/* if any */
	Str31 alias;					/* if any */
} DejaVu, **DejaVuPtr, **DejaVuHandle;
#define DEJA_VU_TYPE 'dJvU'

void RememberOpenWindows(void)
{
	DejaVuHandle dvh;
	DejaVu dv;
	short ind;
	MyWindowPtr win;
	TOCHandle tocH;
	MessHandle messH;
	TextDHandle textH;
	extern FindClose();
	
	/*
	 * out with the old
	 */
	SetResLoad(False);
	while (dvh=Get1IndResource(DEJA_VU_TYPE,1))
	{
		RmveResource(dvh);
				DisposHandle(dvh);
	}
	SetResLoad(True);
	
	/*
	 * save each window
	 */
	ind = 1000;
	for (win=FrontWindow();win;win=win->qWindow.nextWindow)
		if (IsMyWindow(win) && win->qWindow.visible)
				{
					dv.wType = win->qWindow.windowKind;
					if (win->close==FindClose) dv.wType = FIND_WIN;
					switch (dv.wType)
					{
						case MBOX_WIN:
						case CBOX_WIN:
							tocH = (TOCHandle) win->qWindow.refCon;
							PCopy(dv.name,(*tocH)->name);
							dv.dirId = (*tocH)->dirId;
							break;
						case MESS_WIN:
						case COMP_WIN:
							messH = (MessHandle) win->qWindow.refCon;
							tocH = (TOCHandle) (*messH)->tocH;
							PCopy(dv.name,(*tocH)->name);
							dv.index = (*messH)->sumNum;
							dv.dirId = (*tocH)->dirId;
							break;
						case TEXT_WIN:
							textH = (TextDHandle) win->qWindow.refCon;
							PCopy(dv.name,(*textH)->fileName);
							PCopy(dv.alias,*win->qWindow.titleHandle);
							GetMyVolName((*textH)->vRef,dv.volName);
							PCatC(dv.volName,':');
							dv.dirId = GetMyDirID((*textH)->vRef);
							break;
						case ALIAS_WIN:
						case MB_WIN:
						case FIND_WIN:
						case PH_WIN:
							break;
					}
					if (dvh=NuHandle(sizeof(dv)))
					{
						BlockMove(&dv,*dvh,sizeof(dv));
								AddResource(dvh,DEJA_VU_TYPE,++ind,"");
								ReleaseResource(dvh);
					}
				}
}

/************************************************************************
 * RecallOpenWindows - reopen windows there when we quit
 ************************************************************************/
void RecallOpenWindows(void)
{
	short ind;
	DejaVuHandle dvh;
	DejaVu dv;
	TOCHandle tocH;
	short vRef;
	
	for (ind=1000+Count1Resources(DEJA_VU_TYPE);!EjectBuckaroo && ind>1000;ind--)
	{
		if (dvh=Get1Resource(DEJA_VU_TYPE,ind))
		{
			ActivateMyWindow(FrontWindow(),False);
			dv = **dvh;
			ReleaseResource(dvh);
			switch (dv.wType)
			{
				case MBOX_WIN:
				case CBOX_WIN:
					if (FindDirLevel(dv.dirId)>=0)	/* don't open boxes not in tree */
						(void) GetMailbox(dv.dirId,dv.name,True);
					break;
				case MESS_WIN:
				case COMP_WIN:
					if (FindDirLevel(dv.dirId)>=0 &&	/* don't open boxes not in tree */
						  (tocH = TOCByName(dv.dirId,dv.name)) && dv.index<(*tocH)->count)
						GetAMessage(tocH,dv.index,nil,True);
					break;
				case TEXT_WIN:
					vRef = GetMyVR(dv.volName);
					if (!OpenWD(vRef,dv.dirId,nil,&vRef))
						OpenText(vRef,dv.name,nil,True,dv.alias);
					break;
				case ALIAS_WIN:
					OpenAliases();
					break;
				case MB_WIN:
					OpenMBWin();
					break;
				case FIND_WIN:
					DoFind(FIND_FIND_ITEM);
					break;
				case PH_WIN:
					OpenPh();
					break;
			}
		}
		MonitorGrow();
	}
}

/************************************************************************
 * OpenPlugIns - open extra Eudora resource files
 ************************************************************************/
void OpenPlugIns(void)
{
  short vRef;
	long dirId;
	HFileInfo hfi;
	Str31 name;
  if (!FindFolder(kOnSystemDisk,kPreferencesFolderType,False,&vRef,&dirId))
	{
	  hfi.ioFDirIndex = 0;
		hfi.ioNamePtr = name;
		while(!DirIterate(vRef,dirId,&hfi))
		if (hfi.ioFlFndrInfo.fdType==PLUG_TYPE &&
		    hfi.ioFlFndrInfo.fdCreator==CREATOR)
		{
			if (!HOpenResFile(vRef, dirId, name, fsRdPerm))
			  FileSystemError(OPEN_SETTINGS,name,ResError());
		}
	}
}