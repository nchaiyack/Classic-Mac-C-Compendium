#define FILE_NUM 21
#pragma load EUDORA_LOAD
#pragma segment Mailbox

	int OpenMailbox(long dirId,UPtr name, Boolean showIt);
	void SwapSum(MSumPtr sum1, MSumPtr sum2);
	void ZeroMailbox(TOCHandle tocH);
	void AddBox(short menuId,short function,UPtr name,Boolean folder,short level);
static Str31 Re;
	void AddBoxCountItem(short item,long dirId);
	void AddBoxCountMenu(MenuHandle mh, short item, long dirId);
	Boolean WantRebuildTOC(UPtr boxName);
	Boolean InsaneTOC(TOCHandle tocH);
	void NoteFreeSpace(TOCHandle tocH);

/************************************************************************
 * TOCByName - take a mailbox name, return a TOC
 ************************************************************************/
TOCHandle TOCByName(long dirId,UPtr name)
{
	if (!GetMailbox(dirId,name,False)) return(FindTOC(dirId,name));
	return(nil);
}

/**********************************************************************
 * GetMailbox - put a mailbox window frontmost; open if necessary
 **********************************************************************/
int GetMailbox(long dirId,UPtr name,Boolean showIt)
{
	TOCHandle toc;
	
	if (toc=FindTOC(dirId,name))
	{
		UsingWindow((*toc)->win);
		if (showIt)
		{
			if (!((WindowPeek)(*toc)->win)->visible)
				ShowMyWindow((*toc)->win);
			SelectWindow((*toc)->win);
		}
		return(0);
	}
	
	return(OpenMailbox(dirId,name,showIt));
}

/**********************************************************************
 * OpenMailbox - open the named mailbox
 **********************************************************************/
int OpenMailbox(long dirId,UPtr name, Boolean showIt)
{

	TOCHandle toc;
	MyWindow *win;
	
	/*
	 * create window
	 */
	if ((win=GetNewMyWindow(MAILBOX_WIND,nil,InFront,True,True))==nil)
	{
		WarnUser(COULDNT_WIN,MemError());
		return(MemError());
	}

	win->hPitch = FontWidth;
	win->vPitch = FontLead+FontDescent;
	
	/*
	 * read or build toc for window
	 */
	toc = CheckTOC(dirId,name);
	if (toc==nil)
	{
		DisposeWindow(win);
		return(1);
	}
	
	/*
	 * set it up properly
	 */
	((WindowPeek)win)->windowKind = (*toc)->which==OUT ? CBOX_WIN : MBOX_WIN;
	((WindowPeek)win)->refCon = (long) toc;
	win->close = BoxClose;
	win->click = BoxClick;
	win->activate = BoxActivate;
	win->menu = BoxMenu;
	win->key = BoxKey;
	win->help = BoxHelp;
	win->didResize = BoxDidResize;
	win->gonnaShow = BoxGonnaShow;
	win->position = BoxPosition;
	win->cursor = BoxCursor;
	win->zoomSize = MaxSizeZoom;
	win->minSize.h = 2*GetRLong(BOX_SIZE_SIZE);
	
	(*toc)->win = win;
	((WindowPeek)win)->windowKind = (*toc)->which==OUT ? CBOX_WIN : MBOX_WIN;
	SetWTitle(win,name);
	if (showIt)
		ShowMyWindow(win);
	
	/*
	 * push it onto list of open toc's
	 */
	LL_Push(TOCList,toc);
	
	return(0);
}

/**********************************************************************
 * SaveMessageSum - save a message summary into a TOC
 **********************************************************************/
Boolean SaveMessageSum(MSumPtr sum,TOCHandle tocH)
{
	if ((*tocH)->count)
	{
		SetHandleBig(tocH,GetHandleSize(tocH)+sizeof(MSumType));
		if (MemError()) return (False);
	}
	(*tocH)->needRedo = (*tocH)->count;
	BlockMove(sum,&(*tocH)->sums[(*tocH)->count++],sizeof(MSumType));
	CalcSumLengths(tocH,(*tocH)->count-1);
	InvalSum(tocH,(*tocH)->count-1);
	(*tocH)->dirty = True;
	return(True);
}

/**********************************************************************
 * CheckTOC - check a file for a table of contents, and build it if
 * necessary.
 **********************************************************************/
TOCHandle CheckTOC(long dirId,UPtr boxName)
{
	Str31 tocName;
	HFileInfo tocInfo,boxInfo;
	int err;
	Str15 suffix;
	Boolean makeDirty=False;
	TOCHandle ourBoy;
	short res;
	
	/*
	 * toc is kept in name.toc; get info on it
	 */
	BlockMove(boxName,tocName,*boxName+1L);
	PCat(tocName,GetRString(suffix,TOC_SUFFIX));
	if (err=HGetFileInfo(MyVRef,dirId,tocName,&tocInfo))
		return (BuildTOC(dirId,boxName));
	
	/*
	 * now, info on the box
	 */
	if (err=HGetFileInfo(MyVRef,dirId,boxName,&boxInfo))
		return (nil);
	
	/*
	 * is toc older than box?
	 * the -5 is for slop; the filesystem seems to be a bit arbitrary about
	 * modify times
	 */
	if (tocInfo.ioFlMdDat<boxInfo.ioFlMdDat-5 && !PrefIsSet(PREF_IGNORE_OUTDATE))
	{
		res = AlertStr(NEW_TOC_ALRT,Caution,boxName);
		if (res==TOC_CREATE_NEW) return(ReBuildTOC(dirId,boxName));
		else if (res==TOC_CANCEL) return(nil);
		else makeDirty = True;
	}
	
	/*
	 * try to read toc
	 */
	ourBoy = ReadTOC(dirId,boxName);
	if (ourBoy && makeDirty) (*ourBoy)->dirty = True;
	return(ourBoy);
}

/**********************************************************************
 * ReadTOC - read the toc file for a mailbox
 **********************************************************************/
TOCHandle ReadTOC(long dirId,UPtr boxName)
{
	Str31 tocName,scratch;
	long count;
	int err;
	short refN=0;
	TOCHandle tocH;
	Str15 suffix;
	
	/*
	 * toc is kept in name.toc; try to open it
	 */
	BlockMove(boxName,tocName,*boxName+1L);
	PCat(tocName,GetRString(suffix,TOC_SUFFIX));
	if (err=FSHOpen(tocName,MyVRef,dirId,&refN,fsRdPerm)) goto failure;
	
	/*
	 * allocate space for the toc
	 */
	err = GetEOF(refN, &count);
	if (err) goto failure;
	err = count < sizeof(TOCType);	/* change to support different toc versions */
	if (err) goto failure;
	tocH = NuHandle(count);
	if (!tocH)
	{
		FSClose(refN);
		WarnUser(MEM_ERR,MemError());
		return(nil);
	}
	
	/*
	 * try to read it
	 */
	LDRef(tocH);
	err = FSRead(refN,&count,*tocH);
	(void) FSClose(refN), refN = 0;
	if (err) goto failure;
	
	/*
	 * don't take these for granted...
	 */
	(*tocH)->dirty = False;
	(*tocH)->vRef = MyVRef;
	(*tocH)->dirId = dirId;
	(*tocH)->refN = 0;
	(*tocH)->win = nil;
	(*tocH)->volumeFree = 0;
	
	/*
	 * now, make sure the filename matches, for people who rename with the
	 * finder
	 */
	PCopy((*tocH)->name,boxName);
	
	/*
	 * and make sure it hasn't become special or unspecial
	 */
	(*tocH)->which = 0;
	if (dirId==MyDirId)
	{
		if (EqualString(GetRString(scratch,IN),boxName,False,True))
			(*tocH)->which = IN;
		else if (EqualString(GetRString(scratch,OUT),boxName,False,True))
			(*tocH)->which = OUT;
		else if (EqualString(GetRString(scratch,TRASH),boxName,False,True))
			(*tocH)->which = TRASH;
	}

	/*
	 * check toc for reasonableness
	 */
	if (InsaneTOC(tocH)) goto failure;
	
	/*
	 * make sure we don't have any leftovers
	 */
	CleanseTOC(tocH);
	
	/*
	 * just to make sure...
	 */
	if (dirId==MyDirId)
	{
		if (EqualString(boxName,GetRString(tocName,OUT),False,True))
			(*tocH)->which = OUT;
		else if (EqualString(boxName,GetRString(tocName,IN),False,True))
			(*tocH)->which = IN;
		else if (EqualString(boxName,GetRString(tocName,TRASH),False,True))
			(*tocH)->which = TRASH;
	}
	
	/*
	 * and sizes
	 */
	GetTOCK(tocH,&(*tocH)->usedK,&(*tocH)->totalK);
	
	/*
	 * hurray for our side!
	 */
	UL(tocH);
	return(tocH);

failure:
	if (refN) FSClose(refN);
	if (tocH) ZapHandle(tocH);
	if (err) FileSystemError(READ_TOC,boxName,err);
	return (WantRebuildTOC(boxName)?ReBuildTOC(dirId,boxName):nil);
}

/**********************************************************************
 * WriteTOC - write a toc to the proper file
 **********************************************************************/
int WriteTOC(TOCHandle tocH)
{
	Str31 tocName;
	Str15 suffix;
	int err;
	short refN=0;
	long count;
	
	/*
	 * build the name
	 */
	PCopy(tocName,(*tocH)->name);
	PCat(tocName,GetRString(suffix,TOC_SUFFIX));
	
	/*
	 * open the file
	 */
	err = FSHOpen(tocName,(*tocH)->vRef,(*tocH)->dirId,&refN,fsRdWrPerm);
	if (err == fnfErr)
	{
		if (err=HCreate((*tocH)->vRef,(*tocH)->dirId,tocName,CREATOR,TOC_TYPE))
			goto failure;
		err = FSHOpen(tocName,(*tocH)->vRef,(*tocH)->dirId,&refN,fsRdWrPerm);
	}
	if (err) goto failure;
		
	
	/*
	 * write the file
	 */
	count = GetHandleSize(tocH);
	if (err = FSWrite(refN,&count,*tocH)) goto failure;

	/*
	 * and sizes
	 */
	GetTOCK(tocH,&(*tocH)->usedK,&(*tocH)->totalK);
	
	/*
	 * done
	 */
	GetFPos(refN,&count);
	SetEOF(refN,count);
	(void) FSClose(refN);
	(*tocH)->dirty = False;
	ComposeLogS(LOG_FLOW,nil,"\pWriteTOC %p\n",tocName);
	UL(tocH);
	
	/*
	 * the size area
	 */
	if ((*tocH)->win&&(*tocH)->win->qWindow.visible)
		InvalBoxSizeBox((*tocH)->win);
		
	return(noErr);
	
failure:
	FileSystemError(WRITE_TOC,tocName,err);
	UL(tocH);
	if (refN)
	{ /* get rid of partial file */
		(void) FSClose(refN);
		(void) HDelete((*tocH)->vRef,(*tocH)->dirId,tocName);
	}
	return(err);
}

/************************************************************************
 * CalcAllSumLengths - calculate all the lengths for all the sums in a toc
 ************************************************************************/
void CalcAllSumLengths(TOCHandle toc)
{
	int sumNum;

	for (sumNum=0; sumNum<(*toc)->count; sumNum++)
		CalcSumLengths(toc,sumNum);
}

/************************************************************************
 * CalcSumLengths - calculcate how long the strings in a sum can be
 ************************************************************************/
void CalcSumLengths(TOCHandle tocH,int sumNum)
{
	Str255 scratch;
	short trunc;
	short dWidth = (*BoxLines)[WID_DATE]-(*BoxLines)[WID_DATE-1];
	short fWidth = (*BoxLines)[WID_FROM]-(*BoxLines)[WID_FROM-1];
	
	if (FontIsFixed)
	{
		(*tocH)->sums[sumNum].dateTrunc = dWidth/FontWidth - 1;
		(*tocH)->sums[sumNum].fromTrunc = fWidth/FontWidth - 1;
	}
	else
	{
		PCopy(scratch,(*tocH)->sums[sumNum].date);
		trunc = CalcTrunc(scratch,dWidth,InsurancePort);
		if (trunc && trunc<*scratch) trunc--;
		(*tocH)->sums[sumNum].dateTrunc = trunc;

		PCopy(scratch,(*tocH)->sums[sumNum].from);
		trunc = CalcTrunc(scratch,fWidth,InsurancePort);
		if (trunc && trunc<*scratch) trunc--;
		(*tocH)->sums[sumNum].fromTrunc = trunc;
	}
}


/**********************************************************************
 * SetState - set a message's state in its summary.
 **********************************************************************/
void SetState(TOCHandle tocH,int sumNum,int state)
{
	int oldState = (*tocH)->sums[sumNum].state;
		
	if (oldState == state)
		return; 										/* nothing to do */

	InvalTocBox(tocH,sumNum,WID_STAT);
	
	if (oldState==QUEUED)
	  TimeStamp(tocH,sumNum,state==SENT ? GMTDateTime() : 0,ZoneSecs());
	(*tocH)->sums[sumNum].state = state;
	(*tocH)->dirty = True;
	
}

/**********************************************************************
 * FindTOC - find a TOC in the TOC window list
 **********************************************************************/
TOCHandle FindTOC(long dirId,UPtr name)
{
	TOCHandle tocH;
	
	for (tocH=TOCList; tocH; tocH = (*tocH)->next)
		if ((*tocH)->dirId==dirId && EqualString((*tocH)->name,name,False,True))
			break;
	return(tocH);
}

/**********************************************************************
 * BoxFOpen - open the mailbox file represented by a toc
 * may be called on open mailbox, and reports error to user
 **********************************************************************/
int BoxFOpen(TOCHandle tocH)
{
	short refN;
	int err=0;
	Str31 name;
	
	if ((*tocH)->refN==0)
	{
		PCopy(name,(*tocH)->name);
		err = FSHOpen(name,(*tocH)->vRef,(*tocH)->dirId,&refN,fsRdWrPerm);
		if (err)
			FileSystemError(OPEN_MBOX,(*tocH)->name,err);
		else
			(*tocH)->refN = refN;
		ComposeLogS(LOG_FLOW,nil,"\pBoxFOpen %p\n",name);
	}
	
	return(err);
}

/**********************************************************************
 * BoxFClose - close a mailbox file represented by a toc.  May be
 * called on open mailbox, reports any errors to user.
 **********************************************************************/
int BoxFClose(TOCHandle tocH)
{
	int err=0;
	
	if ((*tocH)->refN)
	{
		NoteFreeSpace(tocH);
		err = FSClose((*tocH)->refN);
		(*tocH)->refN = 0;
		if (err)
			FileSystemError(CLOSE_MBOX,LDRef(tocH)->name,err);
		ComposeLogS(LOG_FLOW,nil,"\pBoxFClose %p\n",LDRef(tocH)->name);
		UL(tocH);
	}
	
	return(err);
}

/************************************************************************
 * 
 ************************************************************************/
void NoteFreeSpace(TOCHandle tocH)
{
	short vRef;
	long dirId;
	Str31 name;
	
	if (GetFileByRef((*tocH)->refN,&vRef,&dirId,name))
		(*tocH)->volumeFree = 0;
	else
		(*tocH)->volumeFree = VolumeFree(vRef);
}

#pragma segment Main
/**********************************************************************
 * DeleteSum - remove a sum from a toc
 **********************************************************************/
void DeleteSum(TOCHandle tocH,int sumNum)
{
	MSumPtr sum;
	int mNum;

	(*tocH)->maxValid = MIN((*tocH)->maxValid,sumNum-1);
	if ((*tocH)->sums[sumNum].state==QUEUED) ForceSend = 0;
	if (sumNum < (*tocH)->count-1)		/* is this not the last sum? */
	{
		LDRef(tocH);
		sum = (*tocH)->sums + sumNum;
		BlockMove(sum+1,sum,((*tocH)->count-1-sumNum)*sizeof(MSumType));
		UL(tocH);
		for (; mNum < (*tocH)->count-1; mNum++)
			if ((MessType **)(*tocH)->sums[mNum].messH)
				(*(MessType **)(*tocH)->sums[mNum].messH)->sumNum--;
	}
	SetHandleBig(tocH,
						MAX(sizeof(TOCType),GetHandleSize(tocH)-sizeof(MSumType)));
	if (--(*tocH)->count == 0) ZeroMailbox(tocH);

	(*tocH)->dirty = True;
}
#pragma segment Mailbox

/**********************************************************************
 * InvalSum - invalidate an entire message summary line
 **********************************************************************/
void InvalSum(TOCHandle tocH,int sum)
{
	Rect r;
	MyWindowPtr win = (*tocH)->win;
	GrafPtr oldPort;
	
	if (!win) return;
	GetPort(&oldPort);
	SetPort(win);
	
	r.top = win->vPitch * (sum-GetCtlValue(win->vBar));
	r.bottom = r.top + win->vPitch + 1;
	r.left = win->contR.left;
	r.right = win->contR.right;
	InvalRect(&r);
	SetPort(oldPort);
}

/************************************************************************
 *
 ************************************************************************/
void AddBox(short menuId,short function,UPtr name,Boolean folder,short level)
{
	MenuHandle mh=GetMHandle(menuId);
	short base=function*MAX_BOX_LEVELS;
	short item, lastItem;
	Style theStyle;
	Str31 scratch;
	Boolean hasSub;
	
	lastItem=CountMItems(GetMHandle(menuId));
	for (item=lastItem;item>0;item--)
	{
		hasSub=HasSubmenu(mh,item);
		if (hasSub && !folder) continue;		/* add real mailboxes above folders */
		GetItemStyle(mh,item,&theStyle);
		if (theStyle) break;								/* "new" is italicized */
		MyGetItem(mh,item,scratch);
		if (scratch[1]=='-') break; 				/* menu separator (transfer) */
		if (IUCompString(scratch,name)<0) break;
	}
	MyInsMenuItem(GetMHandle(menuId),name,item);
	if (folder)
	{
		AttachHierMenu(menuId,item+1,base+level);
		InsertMenu(NewMenu(base+level,""),-1);
		if (function==MAILBOX)
			AppendMenu(GetMHandle(base+level),GetRString(scratch,NEW_ITEM_TEXT));
	}
}

/**********************************************************************
 * GetNewMailbox - get the name of and create a new mailbox
 * returns 1 for normal mb's, or else dirId
 **********************************************************************/
Boolean GetNewMailbox(long inDirId, UPtr name, Boolean *folder,long *newDirId,Boolean *xfer)
{
	DialogPtr dgPtr;
	short item;
	
	ThirdCenterDialog(NEW_MAILBOX_DLOG);
	if ((dgPtr = GetNewDialog(NEW_MAILBOX_DLOG,nil,InFront))==nil)
	{
		WarnUser(GENERAL,MemError());
		return(False);
	}
	
	if (!xfer) HideDItem(dgPtr,NEW_MAILBOX_NOXF);
	ShowWindow(dgPtr);
	HiliteButtonOne(dgPtr);
	do
	{
		SelIText(dgPtr,NEW_MAILBOX_NAME,0,INFINITY);
		PushCursor(iBeamCursor);
		do
		{
			ModalDialog(DlgFilter,&item);
			if (item==NEW_MAILBOX_FOLDER)
				SetDItemState(dgPtr,item,!GetDItemState(dgPtr,item));
			else if (item==NEW_MAILBOX_NOXF)
				SetDItemState(dgPtr,item,!GetDItemState(dgPtr,item));
		} while (item==NEW_MAILBOX_FOLDER || item==NEW_MAILBOX_NOXF);
		PopCursor();
		GetDIText(dgPtr,NEW_MAILBOX_NAME,name);
		*folder = GetDItemState(dgPtr,NEW_MAILBOX_FOLDER);
		if (xfer) *xfer = GetDItemState(dgPtr,NEW_MAILBOX_NOXF);
	}
	while (item==NEW_MAILBOX_OK && BadMailboxName(inDirId,name,*folder,newDirId));
	
	DisposDialog(dgPtr);
	
	return(item==NEW_MAILBOX_OK);
}

/**********************************************************************
 * RenameMailbox - rename a mailbox
 **********************************************************************/
int RenameMailbox(long inDirId,UPtr oldName, UPtr newName, Boolean folder)
{
	int err;
	Str31 oldTOCName, newTOCName;
	Str15 suffix;
	
	err = HRename(MyVRef,inDirId,oldName,newName);
	if (err) return(FileSystemError(RENAMING_BOX,oldName,err));
	
	if (!folder)
	{
		PCopy(oldTOCName,oldName);
		PCopy(newTOCName,newName);
		PCat(oldTOCName,GetRString(suffix,TOC_SUFFIX));
		PCat(newTOCName,GetRString(suffix,TOC_SUFFIX));
		
		err = HRename(MyVRef,inDirId,oldTOCName,newTOCName);
		if (err==fnfErr) err = 0;
		if (err)
		{
			FileSystemError(RENAMING_BOX,oldTOCName,err);
			(void) HRename(MyVRef,inDirId,newName,oldName);
		}
	}
	
	return(err);
}

/**********************************************************************
 * BadMailboxName - figure out if a mailbox name is ok by trying to
 * create the mailbox.
 **********************************************************************/
Boolean BadMailboxName(long inDirId,UPtr name,Boolean folder,long *newDirId)
{
	int err;
	char *cp;
	Str15 suffix;
	
	if (*name>31-*GetRString(suffix,TOC_SUFFIX))
	{
		TooLong(name);
		return(True);
	}
	
	if (name[1]=='.')
	{
		WarnUser(LEADING_PERIOD,0);
		return(True);
	}
	
	for (cp=name+*name;cp>name;cp--)
		if (*cp==':')
		{
			WarnUser(NO_COLONS_HERE,0);
			return(True);
		}
	
	if (folder)
	{
		if (GetHandleSize(BoxMap)/sizeof(long)>MAX_BOX_LEVELS)
		{
			WarnUser(TOO_MANY_LEVELS,MAX_BOX_LEVELS);
			return(True);
		}
		if (err=DirCreate(MyVRef,inDirId,name,newDirId))
		{
			FileSystemError(CREATING_MAILBOX,name,err);
			return(True);
		}
		PtrAndHand(newDirId,BoxMap,sizeof(long));
	}
	else
	{
		err = HCreate(MyVRef,inDirId,name,CREATOR,MAILBOX_TYPE);
		if (err)
		{
			FileSystemError(CREATING_MAILBOX,name,err);
			return(True);
		}
	}
	return(False);
}

/************************************************************************
 * ZeroMailbox - set a mailbox's size to zero.  Assumes box is empty
 ************************************************************************/
void ZeroMailbox(TOCHandle tocH)
{
	if (!BoxFOpen(tocH))
	{
		SetEOF((*tocH)->refN,0L);
		BoxFClose(tocH);
	}
}

/************************************************************************
 * FlushTOCs - make sure all toc's are quiescent
 ************************************************************************/
int FlushTOCs(Boolean andClose,Boolean canSkip)
{
	TOCHandle tocH, nextTocH;
	int shdBe;
	short err;
	static long lastTime;
	static short delay;
	
	if (canSkip && lastTime && TickCount()-lastTime < delay)
		return(noErr);
	
	err = 0;
	
	for (tocH=TOCList; tocH; tocH = nextTocH)
	{
		if ((long)tocH < *(long *)ApplZone || (long)tocH > *(long *)HeapEnd ||
			 (long)*tocH < *(long *)ApplZone || (long)*tocH> *(long *)HeapEnd ||
			 (long)*tocH & 1)
		{
#ifdef DEBUG
			if (RunType != Production) DebugStr("\pbad tocH!");
#endif DEBUG
			if (tocH==TOCList)
				TOCList = nil;
			else
			{
				for (nextTocH=TOCList;
						 (*nextTocH)->next!=tocH;nextTocH=(*nextTocH)->next);
				(*nextTocH)->next = nil;
			}
			break;
		}
		
		shdBe = sizeof(TOCType)+MAX(0,(*tocH)->count-1)*sizeof(MSumType);
		if (GetHandleSize(tocH)!=shdBe)
		{
#ifdef DEBUG
			if (RunType != Production)
			{
				Str127 debug;
				Str32 name;
				PCopy(name,(*tocH)->name);
				ComposeString(debug,"\p%p: size %d, should be %d (%d + (%d-1)*%d)",
					name,GetHandleSize(tocH),shdBe,sizeof(TOCType),(*tocH)->count,
					sizeof(MSumType));
				SysBreakStr(debug);
			}
#endif DEBUG
			SetHandleBig(tocH,shdBe);
		}
		nextTocH = (*tocH)->next;

		if (err=BoxFClose(tocH)) break;
		if ((*tocH)->dirty)
			if (err=WriteTOC(tocH))
				break;
		if (andClose && !(*tocH)->win->qWindow.visible
				&& !(MyNMRec && (*tocH)->which==IN))
		{
			int sNum;
			for (sNum=0;sNum<(*tocH)->count;sNum++)
				if ((*tocH)->sums[sNum].messH) break;
			if (sNum==(*tocH)->count)
				CloseMyWindow((*tocH)->win);
		}
		else RedoTOC(tocH);
	}
	
	if (err)
	{
		lastTime = TickCount();
		if (!delay)
			delay = 60*30;
		else
			delay = MIN(60*60*5,(delay*3)/2);
	}
	else
		lastTime = delay = 0;
	return(err);
} 

/************************************************************************
 * SwapSum
 ************************************************************************/
void SwapSum(MSumPtr sum1, MSumPtr sum2)
{
	MSumType tempSum;
	tempSum = *sum1;
	*sum1 = *sum2;
	*sum2 = tempSum;
}

/************************************************************************
 * SumTimeCompare - compare the arrival times of two sums
 ************************************************************************/
int SumTimeCompare(MSumPtr sum1, MSumPtr sum2)
{
	long res = (unsigned)sum1->seconds > (unsigned)sum2->seconds ? 1 :
						 (sum1->seconds==sum2->seconds ? 0 : -1);
	return(res ? res : sum1->spare[0]-sum2->spare[0]);
}

/************************************************************************
 * SumSubjCompare - compare the subjects of two sums
 ************************************************************************/
int SumSubjCompare(MSumPtr sum1, MSumPtr sum2)
{
	Str63 s1,s2;
	Boolean p1,p2;
	short result;
	
	PCopy(s1,sum1->subj);
	PCopy(s2,sum2->subj);
	p1 = TrimPrefix(s1,Re);
	p2 = TrimPrefix(s2,Re);
	TrimInitialWhite(s1);
	TrimInitialWhite(s2);
	result = IUCompString(s1,s2);
	if (result) return(result);
	if (p1)
		return(p2?(sum1->spare[0]-sum2->spare[0]):1);
	else
		return((!p2)?(sum1->spare[0]-sum2->spare[0]):-1);
}

/************************************************************************
 * SumFromCompare - compare the senders of two sums
 ************************************************************************/
int SumFromCompare(MSumPtr sum1, MSumPtr sum2)
{
	short res=IUCompString(sum1->from,sum2->from);
	return (res ? res : sum1->spare[0]-sum2->spare[0]);
}


/************************************************************************
 * 
 ************************************************************************/
int SumStatCompare(MSumPtr sum1, MSumPtr sum2)
{
	short res = sum1->state-sum2->state;
	return (res ? res : sum1->spare[0]-sum2->spare[0]);
}
int RevSumStatCompare(MSumPtr sum1, MSumPtr sum2)
{
	return(-SumStatCompare(sum1,sum2));
}
int SumPriorCompare(MSumPtr sum1, MSumPtr sum2)
{
	short p1,p2,res;
	p1 = sum1->priority;
	p2 = sum2->priority;
	if (!p1) p1=Display2Prior(3);
	if (!p2) p2=Display2Prior(3);
	res = p1-p2;
	return (res ? res : sum1->spare[0]-sum2->spare[0]);
}
int RevSumPriorCompare(MSumPtr sum1, MSumPtr sum2)
{
	return(-SumPriorCompare(sum1,sum2));
}

int RevSumFromCompare(MSumPtr sum1, MSumPtr sum2)
{return(-SumFromCompare(sum1,sum2));}
int RevSumSubjCompare(MSumPtr sum1, MSumPtr sum2)
{return(-SumSubjCompare(sum1,sum2));}
int RevSumTimeCompare(MSumPtr sum1, MSumPtr sum2)
{return(-SumTimeCompare(sum1,sum2));}
/************************************************************************
 * SortTOC - sort a toc, given a comparison function
 ************************************************************************/
void SortTOC(TOCHandle tocH, int (*compare)())
{
	MSumPtr sums, sPtr;
	int count = (*tocH)->count;
	Str63 infinity;
	
	SetHandleBig(tocH,GetHandleSize(tocH)+sizeof(MSumType));
	if (MemError()) {WarnUser(MEM_ERR,MemError()); return;}
	sums=LDRef(tocH)->sums;
	GetRString(infinity,INFINITE_STRING);
	PCopy(sums[count].from,infinity);
	PCopy(sums[count].subj,infinity);
	sums[count].seconds = 0xffffffff;
	GetRString(Re,REPLY_INTRO);
	
	/* tag with original positions */
	for (sPtr=sums;sPtr<sums+(*tocH)->count;sPtr++) sPtr->spare[0]=sPtr-sums;
	
	QuickSort(sums,sizeof(MSumType),0,count-1,compare,SwapSum);
	for (sPtr=sums;sPtr<sums+(*tocH)->count;sPtr++)
		if (sPtr->messH) (*(MessHandle)sPtr->messH)->sumNum = sPtr-sums;

	UL(tocH);
	SetHandleBig(tocH,GetHandleSize(tocH)-sizeof(MSumType));
	(*tocH)->dirty = True;
	
	if ((*tocH)->win)
	{
		GrafPtr oldPort;
		GetPort(&oldPort); SetPort((*tocH)->win);
		INVAL_RECT(&(*tocH)->win->contR);
		SetPort(oldPort);
		BoxCenterSelection((*tocH)->win);
	}
}

/************************************************************************
 * GetInTOC - find the toc of the In mailbox
 ************************************************************************/
TOCHandle GetInTOC(void)
{
	Str31 inName;
	
	GetRString(inName,IN);
	return(TOCByName(MyDirId,inName));
}

/************************************************************************
 * GetOutTOC - find the toc of the Out mailbox
 ************************************************************************/
TOCHandle GetOutTOC(void)
{
	Str31 outName;
	
	GetRString(outName,OUT);
	return(TOCByName(MyDirId,outName));
}

/************************************************************************
 * RemoveMailbox - remove a mailbox
 ************************************************************************/
int RemoveMailbox(long inDirId,UPtr name)
{
	TOCHandle tocH;
	int err;
	Str31 delName;
	Str15 suffix;
	
	/*
	 * open windows
	 */
	if (tocH = FindTOC(inDirId,name))
	{
		(*tocH)->dirty = False;
		if ((*tocH)->win) CloseMyWindow((*tocH)->win);
	}

	/*
	 * files
	 */
	PCopy(delName,name);
	if (err = HDelete(MyVRef,inDirId,delName))
		return(FileSystemError(DELETING_BOX,name,err));
	PCat(delName,GetRString(suffix,TOC_SUFFIX));
	err = HDelete(MyVRef,inDirId,delName);
	if (err==fnfErr) err = 0;
	if (err) return(FileSystemError(DELETING_BOX,delName,err));
		
	return(noErr);
}

/************************************************************************
 * MessagePosition - save/restore position for a new message window
 ************************************************************************/
Boolean MessagePosition(Boolean save,MyWindowPtr win)
{
	TOCHandle tocH = (*(MessHandle)win->qWindow.refCon)->tocH;
	short sumNum = (*(MessHandle)win->qWindow.refCon)->sumNum;
	Rect r;
	Boolean zoomed;
	Boolean res=True;
	
	if (save)
	{
		utl_SaveWindowPos(win,&r,&zoomed);
		(*tocH)->sums[sumNum].savedPos = r;
		if (zoomed)
			(*tocH)->sums[sumNum].flags |= FLAG_ZOOMED;
		else
			(*tocH)->sums[sumNum].flags &= ~FLAG_ZOOMED;
		(*tocH)->dirty = True;
	}
	else
	{
		r = (*tocH)->sums[sumNum].savedPos;
				zoomed = false;
		if (r.right && r.right>r.left && r.bottom && r.bottom>r.top)
			zoomed = ((*tocH)->sums[sumNum].flags & FLAG_ZOOMED)!=0;
		else
		{
			short offset = GetMBarHeight();
			Point corner;
			short val;
			r = ((GrafPtr)win)->portRect;
			if (!(val=GetRLong(PREF_STRN+PREF_MWIDTH))) val=GetRLong(DEF_MWIDTH);
			r.right = r.left + win->hPitch*(val+1) + GROW_SIZE;
			if (val=GetRLong(PREF_STRN+PREF_MHEIGHT))
				r.bottom = r.top + win->vPitch*val;
			utl_StaggerWindow(&r,1,offset,&corner,GetRLong(PREF_STRN+PREF_NW_DEV));
			OffsetRect(&r,corner.h-r.left,corner.v-r.top);
			res = zoomed = False;
		}
		SanitizeSize(&r);
		utl_RestoreWindowPos(win,&r,zoomed,1,FigureZoom,DefPosition);
	}
	return(res);
}

/************************************************************************
 * TooLong - complain about a name that's too long
 ************************************************************************/
void TooLong(UPtr name)
{
	Str63 toolong1,toolong2;
	MyParamText(GetRString(toolong1,BOX_TOO_LONG1),name,
						GetRString(toolong2,BOX_TOO_LONG2),"");
	ReallyDoAnAlert(OK_ALRT,Note);
}

/************************************************************************
 *
 ************************************************************************/
short FindDirLevel(long dirId)
{
	short level;
	short n=GetHandleSize(BoxMap)/sizeof(long);
	
	for (level=0;level<n;level++)
		if ((*BoxMap)[level]==dirId) return(level);
	
	return(-1);
}

/************************************************************************
 * BuildBoxCount - build the numbered list of mailboxes (for Find)
 ************************************************************************/
void BuildBoxCount(void)
{
	if (BoxCount) DisposHandle(BoxCount);
	BoxCount = NuHandle(0);
	if (!BoxCount) {WarnUser(MEM_ERR,MemError()); return;}
	
	AddBoxCountMenu(GetMHandle(MAILBOX_MENU),MAILBOX_BAR1_ITEM+1,MyDirId);
	AddBoxCountItem(MAILBOX_IN_ITEM,MyDirId);
	AddBoxCountItem(MAILBOX_OUT_ITEM,MyDirId);
	AddBoxCountItem(MAILBOX_TRASH_ITEM,MyDirId);
}

/************************************************************************
 * AddBoxCountMenu - add the contents of a menu to the BoxCount thingy
 ************************************************************************/
void AddBoxCountMenu(MenuHandle mh, short item, long dirId)
{
	short n=CountMItems(mh);
	short it;

	
	for (;item<=n;item++)
	{
		if (HasSubmenu(mh,item))
		{
			GetItemMark(mh,item,&it);
			AddBoxCountMenu(GetMHandle(it),1,(*BoxMap)[it%MAX_BOX_LEVELS]);
		}
		else
			AddBoxCountItem(item,dirId);
	}
}

/************************************************************************
 * AddBoxCountItem - add a single item to the BoxCount list
 ************************************************************************/
void AddBoxCountItem(short item,long dirId)
{
	BoxCountElem bce;
	
	bce.item = item;
	bce.dirId = dirId;
	if (PtrAndHand(&bce,BoxCount,sizeof(bce))) WarnUser(MEM_ERR,MemError());
}

/************************************************************************
 * InsaneTOC - see if a TOC is nuts
 ************************************************************************/
Boolean InsaneTOC(TOCHandle tocH)
{
	long boxSize;
	MSumPtr start,stop;
	short err;
	Str31 name;
	
	PCopy(name,(*tocH)->name);

	/*
	 * is the toc the right size?
	 */
	if ((*tocH)->count<0) return(True);
	if (sizeof(TOCType)+((*tocH)->count?(*tocH)->count-1:0)*sizeof(MSumType)!=
			GetHandleSize(tocH))
	{
		ComposeLogS(LOG_ALRT,nil,"\p%p size mismatch; #%d != %d",name,
									(*tocH)->count,GetHandleSize(tocH));
		return(True);
	}
			
	/*
	 * figure out how big the mailbox is
	 */
	if (BoxFOpen(tocH)) return(True);
	err = GetEOF((*tocH)->refN,&boxSize);
	BoxFClose(tocH);
	if (err)
	{
		FileSystemError(READ_TOC,name,err);
		return(True);
	}
	
	/*
	 * check for out of range pointers
	 */
	for (start=(*tocH)->sums,stop=start+(*tocH)->count;start<stop;start++)
	{
		if (start->offset<0 ||start->length<0 ||start->bodyOffset<0 ||
		    start->bodyOffset>start->length ||
				start->offset+start->length > boxSize)
		{
			ComposeLogS(LOG_ALRT,nil,"\p%p bad sum; o%d b%d l%d s%d",name,
				start->offset,start->bodyOffset,start->length,boxSize);
			return(True);
		}
	}
	
	/*
	 * everything looks ok
	 */
	return(False);
}

/************************************************************************
 * WantRebuildTOC - see if the user wants us to remake the toc
 ************************************************************************/
Boolean WantRebuildTOC(UPtr boxName)
{
	return (AlertStr(REB_TOC_ALRT,Caution,boxName)==TOC_CREATE_NEW);
}

/************************************************************************
 * GetTOCK - grab the K counts for a mailbox
 ************************************************************************/
short GetTOCK(TOCHandle tocH,short *usedK, short *totalK)
{
	MSumPtr sum;
	long used=0;
	short err;
	HFileInfo info;
	Str31 name;
	
	for (sum=(*tocH)->sums;sum<(*tocH)->sums+(*tocH)->count;sum++)
		used += sum->length;
	*usedK = used/1024;
	PCopy(name,(*tocH)->name);
	*totalK = (err=HGetFileInfo(MyVRef,(*tocH)->dirId,name,&info)) ?
										 0:info.ioFlLgLen/1024;
	ASSERT(*totalK<4096K);
	return(err);
}

/************************************************************************
 * CleanseTOC - free a newly-read toc of vestiges of its past life
 ************************************************************************/
void CleanseTOC(TOCHandle tocH)
{
	MSumPtr sum,limit;
	 /*
		* unset the selected flags; they might have been saved from last time
		* unset the messH; it may have been filled the last time
		*/
	for (sum=(*tocH)->sums,limit=sum+(*tocH)->count; sum<limit; sum++)
	{
		sum->messH = 0;
		sum->selected = False;
	}
}

/************************************************************************
 * GetTransferParams - get the name & dirid of a mailbox we want to move to
 ************************************************************************/
Boolean GetTransferParams(short menu,short item,long *dirId,UPtr name)
{
	Boolean folder=False, noxfer=False;
	long newDirId;
	Str31 fix;
	
	if (*dirId==MyDirId && item==TRANSFER_NEW_ITEM || *dirId!=MyDirId && item==1)
	{
		do
		{
			if (GetNewMailbox(*dirId,name,&folder,&newDirId,&noxfer))
			{
				BuildBoxMenus();
				MBTickle(nil,nil);
			}
			else return(False);
			if (folder) *dirId = newDirId;
		}
		while (folder);
	}
	else
	{
		MyGetItem(GetMHandle(menu),item,name);
		TrimPrefix(name,GetRString(fix,TRANSFER_PREFIX));
	}
	return(!noxfer);
}
