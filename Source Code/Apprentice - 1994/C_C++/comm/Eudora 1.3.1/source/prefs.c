#define FILE_NUM 31
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Prefs

void NewCreator(DialogPtr dl);
void FontHasChanged(void);
Boolean GetHexPath(UPtr folder,UPtr volpath);
int BadPrefItem(DialogPtr dialog);

#define PText(item,pref) SetDIText(pd,item,GetPref(scratch,pref))
#define PState(item,pref) SetDItemState(pd,item,PrefIsSet(pref))
#define SPText(item,pref) do {									\
	GetDIText(pd,item,scratch); 									\
	TrimWhite(scratch); 													\
	ChangeStrn(PREF_STRN,pref,scratch); 					\
	} while (0)
#define RPState(item,pref) SetDItemState(pd,item,!PrefIsSet(pref))
#define SPState(item,pref) \
	ChangeStrn(PREF_STRN,pref,GetDItemState(pd,item)?&y:&n)
#define SRPState(item,pref) \
	ChangeStrn(PREF_STRN,pref,GetDItemState(pd,item)?&n:&y)
#define PMenu(item,pref) SetDIPopup(pd,item,GetPref(scratch,pref))
#define SPMenu(item,pref) \
	ChangeStrn(PREF_STRN,pref,GetDIPopup(pd,item,scratch))
/**********************************************************************
 * DoNetPreferences - let the user diddle network preferences
 **********************************************************************/
void DoNetPreferences(void)
{
	DialogPtr pd;
	short dItem;
	int startAt;
	Str255 scratch;
	Str63 volpath,folder;
	Boolean newFont = False;
	Boolean done;
	Rect junk;
	Handle cntl;
	long dirId;
	short y='\01y'; 			/* used by the SPState macro */
	short n='\01n'; 			/* used by the SPState macro */
	short size = GetRLong(BOX_SIZE_FONT_SIZE);
	static short ptextItems[] =
		 {NPRDL_POP, NPRDL_SMTP, NPRDL_PH, NPRDL_RETURN, NPRDL_INTERVAL,
			NPRDL_CREATOR, NPRDL_FONT_SIZE, NPRDL_MWIDTH, NPRDL_MHEIGHT,
			NPRDL_PRINT_FONT_SIZE, NPRDL_AUTOHEX_VOLPATH,NPRDL_AUXUSR,NPRDL_REALNAME};
	static short ptextPrefs[] = 
		 {PREF_POP, PREF_SMTP, PREF_PH, PREF_RETURN, PREF_INTERVAL,
			PREF_CREATOR, PREF_FONT_SIZE, PREF_MWIDTH, PREF_MHEIGHT,
			PREF_PRINT_FONT_SIZE, PREF_AUTOHEX_VOLPATH,PREF_AUXUSR,PREF_REALNAME};
	
	SetDAFont(applFont);
	pd=GetNewMyDialog(NPREF_DLOG,nil,InFront,ThirdCenterDialog);
	SetDAFont(systemFont);
	if (pd==nil)
	{
		WarnUser(COULDNT_PREF,MemError());
		return;
	}
	((MyWindowPtr)pd)->update = DlgUpdate;
	SetPort(pd);
	TextSize(size);
	TextFont(applFont);
	{
		TEHandle teh = ((DialogPeek)pd)->textH;
		ControlHandle cntl = ((WindowPeek)pd)->controlList;
		(*teh)->txSize = size;
		(*teh)->lineHeight = GetLeading(applFont,size);
		(*teh)->fontAscent = GetAscent(applFont,size);
	}			
	
	/*
	 * load up preference values
	 */
	for (dItem=0;dItem<sizeof(ptextItems)/sizeof(short);dItem++)
		PText(ptextItems[dItem],ptextPrefs[dItem]);
	SetDItemState(pd,NPRDL_MACTCP,!PrefIsSet(PREF_TRANS_METHOD));
	SetDItemState(pd,NPRDL_CTB,PrefIsSet(PREF_TRANS_METHOD));
	PState(NPRDL_AUTOHEX,PREF_AUTOHEX);
	PMenu(NPRDL_FONT_NAME,PREF_FONT_NAME);
	GetPref(scratch,PREF_PRINT_FONT);
	if (!*scratch)
		if (FontIsFixed) GetRString(scratch,PRINT_FONT);
		else GetPref(scratch,PREF_FONT_NAME);
	SetDIPopup(pd,NPRDL_PRINT_FONT,scratch);
	
	GetPref(scratch,PREF_CREATOR_NAME);
	GetDItem(pd,NPRDL_CREATOR_NAME,&dItem,&cntl,&junk);
	SetCTitle(cntl,scratch);
	GetPref(scratch,PREF_AUTOHEX_NAME);
	GetDItem(pd,NPRDL_AUTOHEX_NAME,&dItem,&cntl,&junk);
	SetCTitle(cntl,scratch);
	if (!GetVolpath(GetPref(volpath,PREF_AUTOHEX_VOLPATH),scratch,&dirId))
		SetDItemState(pd,NPRDL_AUTOHEX,False);
	
	/*
	 * now, display the dialog
	 */
	ShowWindow(pd);
	HiliteButtonOne(pd);
	startAt = NPRDL_POP;
	do
	{
		SelIText(pd,startAt,0,32767);
		PushCursor(arrowCursor);
		done=False;
		do
		{
			ModalDialog(DlgFilter,&dItem);
			switch (dItem)
			{
				case NPRDL_POP:
#ifndef KERBEROS
					InvalidatePasswords(False,True);
#endif
					break;
				case NPRDL_AUXUSR:
#ifndef KERBEROS
					InvalidatePasswords(True,False);
#endif
					break;
				case NPRDL_MACTCP_LABEL:
				case NPRDL_CTB_LABEL:
					dItem = dItem==NPRDL_MACTCP_LABEL ? NPRDL_MACTCP : NPRDL_CTB;
				case NPRDL_MACTCP:
				case NPRDL_CTB:
					SetDItemState(pd,NPRDL_MACTCP,dItem==NPRDL_MACTCP);
					SetDItemState(pd,NPRDL_CTB,dItem==NPRDL_CTB);
					break;
				case NPRDL_AUTOHEX_LABEL:
				case NPRDL_AUTOHEX:
					if (GetDItemState(pd,NPRDL_AUTOHEX))
					{
						SetDItemState(pd,NPRDL_AUTOHEX,False);
						break;
					}
					else
					{
						GetDIText(pd,NPRDL_AUTOHEX_VOLPATH,volpath);
						if (GetVolpath(volpath,scratch,&dirId))
						{
							SetDItemState(pd,NPRDL_AUTOHEX,True);
							break;
						}
					}
					/* fall through here is deliberate */
				case NPRDL_AUTOHEX_NAME:
					GetDIText(pd,NPRDL_AUTOHEX_VOLPATH,volpath);
					if (GetHexPath(folder,volpath))
					{
						SetDItemState(pd,NPRDL_AUTOHEX,True);
						GetDItem(pd,NPRDL_AUTOHEX_NAME,&dItem,&cntl,&junk);
						SetCTitle(cntl,folder);
						SetDIText(pd,NPRDL_AUTOHEX_VOLPATH,volpath);
					}
					break;
				case NPRDL_FONT_NAME:
				case NPRDL_FONT_SIZE:
					newFont = True;
					break;
				case NPRDL_CREATOR_NAME:
					NewCreator(pd);
					break;
				default:
					done = True;
					break;
			}
		}
		while(!done);
		PopCursor();
	}
	while (dItem!=NPRDL_CANCEL&&dItem!=CANCEL_ITEM&&(startAt=BadPrefItem(pd)));
	
	if (dItem!=NPRDL_CANCEL && dItem!=CANCEL_ITEM)
	{
		/*
		 * suck out the values
		 */
		RecountStrn(PREF_STRN);
		for (dItem=0;dItem<sizeof(ptextItems)/sizeof(short);dItem++)
			SPText(ptextItems[dItem],ptextPrefs[dItem]);
		ChangeStrn(PREF_STRN,PREF_TRANS_METHOD,
			GetDItemState(pd,NPRDL_MACTCP) ? "\pn" : "\py");
		SPText(NPRDL_CREATOR,PREF_CREATOR);
		GetDIText(pd, NPRDL_CREATOR,scratch); 							/* don't TrimWhite */
		ChangeStrn(PREF_STRN, PREF_CREATOR,scratch);				/* don't TrimWhite */
		SPState(NPRDL_AUTOHEX,PREF_AUTOHEX);
		SPMenu(NPRDL_FONT_NAME,PREF_FONT_NAME);
		SPMenu(NPRDL_PRINT_FONT,PREF_PRINT_FONT);
		
		GetDItem(pd, NPRDL_AUTOHEX_NAME,&dItem,&cntl,&junk);
		GetCTitle(cntl,scratch);
		ChangeStrn(PREF_STRN,PREF_AUTOHEX_NAME,scratch);
		GetDItem(pd,NPRDL_CREATOR_NAME,&dItem,&cntl,&junk);
		GetCTitle(cntl,scratch);
		ChangeStrn(PREF_STRN,PREF_CREATOR_NAME,scratch);
#ifndef KERBEROS
		if (PrefIsSet(PREF_SAVE_PASSWORD))
		{
			ChangeStrn(PREF_STRN,PREF_PASS_TEXT,Password);
			ChangeStrn(PREF_STRN,PREF_AUXPW,SecondPass);
		}
		else
#endif
		{
			ChangeStrn(PREF_STRN,PREF_PASS_TEXT,"");
			ChangeStrn(PREF_STRN,PREF_AUXPW,"");
		}

		/*
		 * save the resource
		 */
		UpdateResFile(SettingsRefN);
		HPurge(GetResource('STR#',PREF_STRN));
		
		/*
		 * take necessary action
		 */
		if (newFont) FontHasChanged();
		GetPref(scratch,PREF_POP); UUPCIn = *scratch && scratch[1]=='!';
		GetPref(scratch,PREF_SMTP); UUPCOut = *scratch && scratch[1]=='!';
		NewPhHost();
		AlertsTimeout = PrefIsSet(PREF_AUTO_DISMISS);
		UseCTB = PrefIsSet(PREF_TRANS_METHOD);
		CurTrans = UseCTB ? CTBTrans : TCPTrans;
	
		GetRString(NewLine,UseCTB ? CTB_NEWLINE : NEWLINE);
		if (UseCTB) EnableItem(GetMHandle(SPECIAL_MENU),SPECIAL_CTB_ITEM);
		else DisableItem(GetMHandle(SPECIAL_MENU),SPECIAL_CTB_ITEM);
	}
	else
		ReleaseResource(GetResource('STR#',PREF_STRN));
	
	DisposDialog(pd);
}


/**********************************************************************
 * DouserPreferences - let the user diddle private preferences
 **********************************************************************/
void DoUserPreferences(void)
{
	DialogPtr pd;
	short dItem;
	Boolean done;
	short y='\01y'; 			/* used by the SPState macro */
	short n='\01n'; 			/* used by the SPState macro */
	Boolean redraw=False;
	static short pstateItems[] =
	 {UPRDL_AUTO_COMPACT, UPRDL_AUTO_DISMISS, UPRDL_AUTO_EMPTY,
		UPRDL_AUTO_SEND, UPRDL_BX_TEXT, UPRDL_DONT_DELETE, UPRDL_EASY_DELETE,
		UPRDL_ICON_BAR, UPRDL_KEEP_OUTGOING, UPRDL_NEW_ALERT, UPRDL_NEW_SOUND,
		UPRDL_NICK_FOLLOW, UPRDL_NO_BIGGIES, UPRDL_PARAGRAPHS, UPRDL_PLAIN_ARROW,
		UPRDL_SAVE_PASSWORD, UPRDL_SEND_CHECK, UPRDL_SHOW_ALL, UPRDL_SHOW_SIZE,
		UPRDL_SIG, UPRDL_SUPERCLOSE, UPRDL_TAB_IN_TEXT, UPRDL_WRAP_OUT,
		UPRDL_ZOOM_OPEN, -UPRDL_EXCLUDE_HEADERS, -UPRDL_NO_APPLE_FLASH,
		-UPRDL_NO_CMD_ARROW, -UPRDL_NO_PROGRESS, -UPRDL_NO_OPEN_IN,
		UPRDL_REPLY_ALL, -UPRDL_NOT_ME, -UPRDL_NO_AUTO_OPEN};
	static short pstatePrefs[] =
	 {PREF_AUTO_COMPACT, PREF_AUTO_DISMISS, PREF_AUTO_EMPTY,
		PREF_AUTO_SEND, PREF_BX_TEXT, PREF_DONT_DELETE, PREF_EASY_DELETE,
		PREF_ICON_BAR, PREF_KEEP_OUTGOING, PREF_NEW_ALERT, PREF_NEW_SOUND,
		PREF_NICK_FOLLOW, PREF_NO_BIGGIES, PREF_PARAGRAPHS, PREF_PLAIN_ARROW,
		PREF_SAVE_PASSWORD, PREF_SEND_CHECK, PREF_SHOW_ALL, PREF_SHOW_SIZE,
		PREF_SIG, PREF_SUPERCLOSE, PREF_TAB_IN_TEXT, PREF_WRAP_OUT,
		PREF_ZOOM_OPEN, -PREF_EXCLUDE_HEADERS, -PREF_NO_APPLE_FLASH,
		-PREF_NO_CMD_ARROW, -PREF_NO_PROGRESS, -PREF_NO_OPEN_IN,
		PREF_REPLY_ALL, -PREF_NOT_ME, -PREF_NO_AUTO_OPEN};
	if ((pd=GetNewMyDialog(UPREF_DLOG,nil,InFront,ThirdCenterDialog))==nil)
	{
		WarnUser(COULDNT_PREF,MemError());
		return;
	}
	((MyWindowPtr)pd)->update = DlgUpdate;
	
	/*
	 * load up preference values
	 */
	for (dItem=0;dItem<sizeof(pstateItems)/sizeof(short);dItem++)
		if (pstateItems[dItem]>0)
			PState(pstateItems[dItem],pstatePrefs[dItem]);
		else
			RPState(-pstateItems[dItem],-pstatePrefs[dItem]);

	/*
	 * now, display the dialog
	 */
	ShowWindow(pd);
	HiliteButtonOne(pd);
	PushCursor(arrowCursor);
	done=False;
	do
	{
		ModalDialog(DlgFilter,&dItem);
		switch (dItem)
		{
			case UPRDL_SHOW_SIZE:
				redraw = !redraw;
				/* fall through is deliberate */
			default:
				SetDItemState(pd,dItem,!GetDItemState(pd,dItem));
				break;
			case UPRDL_OK:
			case UPRDL_CANCEL:
			case CANCEL_ITEM:
				done = True;
				break;
		}
	}
	while(!done);
	PopCursor();
	
	if (dItem!=UPRDL_CANCEL && dItem!=CANCEL_ITEM)
	{
		/*
		 * suck out the values
		 */
		RecountStrn(PREF_STRN);
		for (dItem=0;dItem<sizeof(pstateItems)/sizeof(short);dItem++)
			if (pstateItems[dItem]>0)
				SPState(pstateItems[dItem],pstatePrefs[dItem]);
			else
				SRPState(-pstateItems[dItem],-pstatePrefs[dItem]);

		/*
		 * save the resource
		 */
		UpdateResFile(SettingsRefN);
		HPurge(GetResource('STR#',PREF_STRN));
		FakeTabs = PrefIsSet(PREF_TAB_IN_TEXT);
	}
	else
		ReleaseResource(GetResource('STR#',PREF_STRN));

	if (redraw)
	{
		TOCHandle tocH;
		GrafPtr oldPort;
		Rect r;
		GetPort(&oldPort);
		r.left = r.top = 0;
		r.right = r.bottom = 20000;
		for (tocH=TOCList;tocH;tocH=(*tocH)->next)
		{
			if ((*tocH)->win->qWindow.visible)
			{
				SetPort((*tocH)->win);
				MyWindowDidResize((*tocH)->win,nil);
				RectRgn((*tocH)->win->qWindow.updateRgn,&r);
			}
		}
		SetPort(oldPort);
	}
	DisposDialog(pd);

}

/************************************************************************
 * NewCreator - allow the user to pick a new application
 ************************************************************************/
void NewCreator(DialogPtr dl)
{
	SFReply sfr;
	Str63 junk;
	Point where;
	SFTypeList tl;
	FInfo fi;
	int err;
	Rect r;
	ControlHandle cntl;
	
	OpenProgress();
	GetRString(junk,PICK_CREATOR);
	Progress(NoChange,junk);
	
	StdFileSpot(&where,SFGETFILE_ID);
	*junk = 0;
	tl[0] = 'APPL';
	SFPGetFile(where,junk,nil,1,tl,nil,&sfr,SFGETFILE_ID,DlgFilter);
	if (sfr.good)
	{
		if (err=GetFInfo(sfr.fName,sfr.vRefNum,&fi))
			FileSystemError(NO_FINFO,sfr.fName,err);
		else
		{
			GetDItem(dl,NPRDL_CREATOR_NAME,&err,&cntl,&r);
			SetCTitle(cntl,sfr.fName);
			BlockMove(&fi.fdCreator,junk+1,4);
			*junk = 4;
			SetDIText(dl,NPRDL_CREATOR,junk);
		}
	}
	CloseProgress();
}
	
/************************************************************************
 * FontHasChanged - the user changed the font on us on the fly.  Fix up
 * windows.
 ************************************************************************/
void FontHasChanged(void)
{
	MyWindowPtr win;
	short i;
	GrafPtr oldPort;
	
	GetPort(&oldPort);
	
	FigureOutFont();
	
	for (win=FrontWindow();win;win=win->qWindow.nextWindow)
		if (IsMyWindow(win))
		{
			SetPort(win);
			TextFont(FontID);
			TextSize(FontSize);
			win->vPitch = FontLead;
			win->hPitch = FontWidth;
			switch (win->qWindow.windowKind)
			{
				case COMP_WIN:
					for (i=0;i<HEAD_LIMIT;i++)
						TEFixup((*(MessHandle)win->qWindow.refCon)->txes[i]);
					TextFont(0);
					TextSize(0);
					break;
				case TEXT_WIN:
					TEFixup(WinTEH(win));
					break;
				case MESS_WIN:
					MyWindowDidResize(win,nil);
					TEFixup(Win2Body(win));
					TEFixup((*Win2MessH(win))->txes[SUBJ_HEAD-1]);
					break;
				case MBOX_WIN:
				case CBOX_WIN:
					win->vPitch = FontLead+FontDescent;
					CalcAllSumLengths((TOCHandle)win->qWindow.refCon);
					break;
				case PH_WIN:
					PhFixFont();
					break;
				case ALIAS_WIN:
					AliasesFixFont();
					break;
			}
			SetPort(win);
			MyWindowDidResize(win,&win->contR);
		}
	RedrawAllWindows();
	SetPort(oldPort);
}

#pragma segment Main
/************************************************************************
 * SettingsPtr - put some data into the Settings file
 ************************************************************************/
short SettingsPtr(ResType type,UPtr name,short id,UPtr dataPtr, long dataSize)
{
	Handle rH = NuHandle(dataSize);
	short err;
	
	if (!rH) return(MemError());
	BlockMove(dataPtr,*rH,dataSize);
	if (err=SettingsHandle(type,name,id,rH))
		DisposHandle(rH);
	return(err);
}

/************************************************************************
 * SettingsHandle - put some data into the Settings file
 ************************************************************************/
short SettingsHandle(ResType type,UPtr name,short id,UHandle	dataHandle)
{
	short oldRefN = CurResFile();
	short err;
	Handle oldDataHandle;
	
	UseResFile(SettingsRefN);
	oldDataHandle = Get1Resource(type,id);
	if (oldDataHandle!=nil)
	{
		RmveResource(oldDataHandle);
		DisposHandle(oldDataHandle);
	}
	
	AddResource(dataHandle,type,id,name?name:"");
	err = ResError();
	UseResFile(oldRefN);
	return(err);
}

/************************************************************************
 * GetHexPath - get the auto-binhex path
 ************************************************************************/
Boolean GetHexPath(UPtr folder,UPtr volpath)
{
	Str31 volName;
	long dirId;
	short vRef;
	Boolean result;
	UPtr rev;
	
	if (!GetVolpath(volpath,volName,&dirId))
	{
		GetVol(volName,&vRef);
		dirId = 2;
	}
	if (result=GetFolder(volName,&vRef,&dirId,True,True,True,True))
	{
		PCopy(volpath,volName);
		PLCat(volpath,dirId);
		for (rev=volpath+*volpath;*rev!=' ';rev--);
		*rev = ':';
		(void) GetDirName(volName,vRef,dirId,folder);
	}
	return(result);
}

#pragma segment Util
/************************************************************************
 * PrefIsSet - is an on/off preference on?
 ************************************************************************/
Boolean PrefIsSet(short pref)
{
	Str255 scratch;
	
	GetPref(scratch,pref);
	return(*scratch && scratch[1]=='y');
}


/************************************************************************
 * GetPOPInfo - get pop user and account
 ************************************************************************/
void GetPOPInfo(UPtr user, UPtr host)
{
	Str255 scratch;
	UPtr atSign;
	
	GetPref(scratch,PREF_POP);
	if (*scratch && (atSign=strchr(scratch+1,'@')))
	{
		*atSign = *scratch - (atSign-scratch);
		*scratch = atSign-scratch-1;
		PCopy(user,scratch);
		PCopy(host,atSign);
		user[*user+1] = host[*host+1] = 0;
	}
	else
		*user = *host = 0;
}

/************************************************************************
 * GetSMTPInfo - get SMTP host (fallback to POP host if necessary)
 ************************************************************************/
UPtr GetSMTPInfo(UPtr host)
{
	Str255 user;
	GetPref(host,PREF_SMTP);
	if (!*host)
		GetPOPInfo(user,host);
	return(host);
}

/************************************************************************
 * GetReturnAddr - get return address (fallback to POP account if necessary)
 ************************************************************************/
UPtr GetReturnAddr(UPtr addr,Boolean comments)
{
	UHandle canon;
	Str255 realname;
	
	GetPref(addr,PREF_RETURN);
	if (!*addr)
		GetPref(addr,PREF_POP);
	if (canon=SuckPtrAddresses(addr+1,*addr,comments))
	{
		*addr = 0;
		if (!comments && **canon!='<') PCatC(addr,'<');
		PCat(addr,*canon);
		if (!comments && addr[*addr]!='>') PCatC(addr,'>');
		addr[*addr+1] = 0;
		DisposHandle(canon);
	}
	if (comments)
	{
		GetPref(realname,PREF_REALNAME);
		if (*realname)
		{
			Str255 fmt,buffer;
			GetRString(fmt,ADD_REALNAME);
			utl_PlugParams(fmt,buffer,addr,realname,nil,nil);
			PCopy(addr,buffer);
		}
	}
	return(addr);
}

/************************************************************************
 * GetVolpath - check the auto-binhex path
 ************************************************************************/
Boolean GetVolpath(UPtr volpath,UPtr volName,long *dirId)
{
	Str31 maybeVol,maybeFolder;
	UPtr colon;
	long maybeId;
	short oldVol,newVol;
	short err;
	
	/*
	 * volpath is of the form "volname:dirId"; find that colon.
	 */
	if (!*volpath) return(False);
	for (colon=volpath+1;colon<volpath+*volpath;colon++) if (*colon==':') break;
	if (*colon!=':') return(False);
	
	/*
	 * volume name
	 */
	BlockMove(volpath,maybeVol,colon-volpath+1);
	*maybeVol = colon-volpath;
	
	/*
	 * dirId
	 */
	maybeId=0;
	for (colon++;colon<=volpath+*volpath;colon++)
		{maybeId *= 10; maybeId += (*colon)-'0';}

	/*
	 * now check it
	 */
	GetVol(nil,&oldVol);
	if (!(err=SetVol(maybeVol,0)))
	{
		GetVol(nil,&newVol);
		if (!(err = GetDirName(maybeVol,0,maybeId,maybeFolder)))
		{
			*dirId = maybeId;
			PCopy(volName,maybeVol);
		}
	}
	SetVol(nil,oldVol);
	return(!err);
}

/************************************************************************
 * SaveBoxLines - save the current setting of BoxLines
 ************************************************************************/
void SaveBoxLines(void)
{
	Handle bLinesH = Get1Resource('STR#',BOX_LINES_STRN);
	Str31 num;
	short i,spot,last=0,offset = 0;
	
	if (!bLinesH)
	{
		if (!(bLinesH=NewZHandle(2))) return;
		*num = 0;
		AddResource(bLinesH,'STR#',BOX_LINES_STRN,num);
	}
	
	for (i=0;i<WID_LIMIT;i++)
	{
		spot = (*BoxLines)[i];
		if (spot-last==3) {offset += 3; spot -= offset;}
		spot /= FontWidth;
		NumToString(spot,num);
		ChangeStrn(BOX_LINES_STRN,i+1,num);
		last = (*BoxLines)[i];
	}
	UpdateResFile(SettingsRefN);
	GetBoxLines();
}

/**********************************************************************
 * BadPrefItem - vet a preferences dialog.	Saves preferences if
 * ok, otherwise scolds the user.  Returns index of offending value,
 * or 0 if everything is ok.
 **********************************************************************/
int BadPrefItem(DialogPtr dialog)
{
	Str255 newValue,scratch;
	int dItem;
	char *where;
	
	/*
	 * check each value for reasonableness
	 */
	for (dItem=NPRDL_POP; dItem <= NPRDL_PRINT_FONT_SIZE; dItem++)
	{
		GetDIText(dialog,dItem,newValue);
		if (*newValue==sizeof(Str255)-1) (*newValue)--;
		newValue[*newValue+1] = 0;
		switch (dItem)
		{
			case NPRDL_POP:
#ifndef POPSECURE
				if (*newValue && newValue[1]=='!') break;
#endif
				where=strchr(newValue+1,'@');
				if (where==nil || where==newValue+1 || where==newValue+*newValue)
				{
					GetRString(newValue,EXPL_POP);
					DoABigAlert(Note,newValue);
					return(dItem);
				}
				break;
			case NPRDL_SMTP:
				where=strchr(newValue+1,'@');
				if (where!=nil)
				{
					GetRString(newValue,EXPL_SMTP);
					DoABigAlert(Note,newValue);
					return(dItem);
				}
				break;
			case NPRDL_PH:
				where=strchr(newValue+1,'@');
				if (where!=nil)
				{
					GetRString(newValue,PH_EXPL);
					DoABigAlert(Note,newValue);
					return(dItem);
				}
				break;
			case NPRDL_RETURN:
				break;
			case NPRDL_INTERVAL:
				for (where=newValue+1;where<newValue+*newValue+1;where++)
					if (!isdigit(*where))
					{
						GetRString(newValue,EXPL_INTERVAL);
						DoABigAlert(Note,newValue);
						return(dItem);
					}
				break;
			case NPRDL_MHEIGHT:
			case NPRDL_MWIDTH:
			case NPRDL_PRINT_FONT_SIZE:
				if (!*newValue) break;
			case NPRDL_FONT_SIZE:
				for (where=newValue+1;where<newValue+*newValue+1;where++)
					if (!isdigit(*where))
					{
						ComposeRString(scratch,FONTSIZE_EXPL,newValue);
						DoAnAlert(Note,scratch);
						return(dItem);
					}
				break;
		}
	}
	return(0);
}
