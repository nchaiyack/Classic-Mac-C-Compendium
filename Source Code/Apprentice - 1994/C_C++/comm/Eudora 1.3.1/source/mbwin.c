#define FILE_NUM 23
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * handling the mailbox window
 **********************************************************************/
#pragma load EUDORA_LOAD
#pragma segment MboxWin

/************************************************************************
 * the structure for the mailbox window
 ************************************************************************/
typedef enum {FromSideIndex,ToSideIndex} SideEnum;
typedef enum {FromRenameIndex,FromRemoveIndex,FromNewIndex,FromMoveIndex,
	ToRenameIndex,ToRemoveIndex,ToNewIndex,ToMoveIndex,
	FromMenuIndex,ToMenuIndex,ControlCount} ControlEnum;
typedef struct
{
	MenuHandle menu;
	ListHandle list;
	short id;
	long dirId;
} SideType, *SidePtr;
typedef struct
{
	SideType sides[2];
	MyWindowPtr win;
	ControlHandle controls[ControlCount];
} MBType, *MBPtr, **MBHandle;
#define Win (*MBG)->win
#define Controls (*MBG)->controls
#define FromRename (*MBG)->controls[FromRenameIndex]
#define FromRemove (*MBG)->controls[FromRemoveIndex]
#define FromNew (*MBG)->controls[FromNewIndex]
#define FromMenu (*MBG)->controls[FromMenuIndex]
#define ToRename (*MBG)->controls[ToRenameIndex]
#define ToRemove (*MBG)->controls[ToRemoveIndex]
#define ToNew (*MBG)->controls[ToNewIndex]
#define ToMenu (*MBG)->controls[ToMenuIndex]
#define FromMove (*MBG)->controls[FromMoveIndex]
#define ToMove (*MBG)->controls[ToMoveIndex]
#define Sides (*MBG)->sides
#define FromSide Sides[FromSideIndex]
#define ToSide Sides[ToSideIndex]
MBHandle MBG;
#define NSIDES (sizeof(Sides)/sizeof(SideType))
#define DIRID_OF(Side) ((Side).id==MAILBOX_MENU ? MyDirId : (*BoxMap)[(Side).id])

/************************************************************************
 * prototypes
 ************************************************************************/
	Boolean MBMenu(MyWindowPtr win, int menu, int item, short modifiers);
	void MBDidResize(MyWindowPtr win, Rect *oldContR);
	void MBUpdate(MyWindowPtr win);
	Boolean MBClose(MyWindowPtr win);
	short MBFill(void);
	short MBFillSide(short whichSide, MenuHandle mh);
	void MBClick(MyWindowPtr win,EventRecord *event);
	void MBCursor(Point mouse);
	void MBActivate(void);
	void MBSetControls(void);
	void MBHit(short which);
	void MBListOpen(short whichSide);
	void MBListClose(short whichSide);
	MenuHandle ParentMailboxMenu(MenuHandle mh,short *itemPtr);
	short FindItemBySubmenu(MenuHandle mh, short subID);
	short MBRefill(short whichSide,UPtr andSelect);
	void DoRenameBox(long inDirId,UPtr name,UPtr newName);
	Boolean DoRemoveBox(MenuHandle mh,UPtr name,Boolean *andShutUp);
	short DeleteMailFolder(MenuHandle mh,short item);
	void DoMoveMailbox(UPtr selectName,short si);
	Boolean SameSelected(void);
	UPtr MBGetData(UPtr data,Cell c,ListHandle list);
	Boolean DontMove(void);
	short CellMenuId(Cell c,short whichSide);
	void MBHelp(MyWindowPtr win,Point mouse);
	void MBFixSideIndices(void);
  void SetSideIndices(short side, MenuHandle mh);

/************************************************************************
 * OpenMBWin - open the mailbox window
 ************************************************************************/
void OpenMBWin(void)
{
	if (!MBG)
	{
		short err=0;
		short i;
		Point cSize;
		Rect r, bounds;
		Str31 scratch;
		
		if (!(MBG=NewZH(MBType))) {err=MemError(); goto fail;}
		LDRef(MBG);
		if (!(Win=GetNewMyWindow(MBWIN_WIND,nil,InFront,False,False)))
			{err=MemError(); goto fail;}
		Win->minSize.h = Win->contR.right-Win->contR.left;
		Win->minSize.v = Win->contR.bottom-Win->contR.top;
		SetPort(Win);
		TextFont(0);
		TextSize(0);
		/*
		 * controls
		 */
		for (i=0;i<=ToMoveIndex;i++)
		{
			if (!(Controls[i]=GetNewControl(MAKE_TO_CNTL,Win)))
				{err=MemError(); goto fail;}
		}
		SetCTitle(FromMove,GetRString(scratch,MBOX_MOVE));
		SetCTitle(FromRename,GetRString(scratch,MBOX_RENAME));
		SetCTitle(FromRemove,GetRString(scratch,MBOX_REMOVE));
		SetCTitle(FromNew,GetRString(scratch,MBOX_NEW));
		SetCTitle(ToMove,GetRString(scratch,MBOX_MOVE_BACK));
		SetCTitle(ToRename,GetRString(scratch,MBOX_RENAME));
		SetCTitle(ToRemove,GetRString(scratch,MBOX_REMOVE));
		SetCTitle(ToNew,GetRString(scratch,MBOX_NEW));
		
		/*
		 * the two popup menus
		 */
		if (!(FromMenu=GetNewControl(FROM_MB_CNTL,Win)))
			{err=MemError(); goto fail;}
		if (!(ToMenu=GetNewControl(TO_MB_CNTL,Win)))
			{err=MemError(); goto fail;}
		
		/*
		 * list areas
		 */
		SetRect(&r,0,0,20,20);
		SetRect(&bounds,0,0,1,0);
		cSize.v = GetLeading(0,0);	cSize.h = 20;
		for (i=0;i<NSIDES;i++)
		{
			if (!(Sides[i].list=
					LNew(&r,&bounds,cSize,MBWIN_LDEF,Win,False,False,False,True)))
				{err=MemError(); goto fail;}
			(*Sides[i].list)->indent.v = GetAscent(0,0);
			(*Sides[i].list)->indent.h = 2;
			(*Sides[i].list)->selFlags = lExtendDrag|lNoRect|lNoExtend|lUseSense;
		}
		
		if (MBFill()) goto fail;
		
		Win->didResize = MBDidResize;
		Win->close = MBClose;
		Win->update = MBUpdate;
		Win->position = PositionPrefsTitle;
		Win->click = MBClick;
		Win->qWindow.windowKind = MB_WIN;
		Win->dontControl = True;
		Win->cursor = MBCursor;
		Win->activate = MBActivate;
		Win->help = MBHelp;
		Win->menu = MBMenu;
		ShowMyWindow(Win);
		MyWindowDidResize(Win,&Win->contR);
		UL(MBG);
		return;
		
		fail:
			if (Win) CloseMyWindow(Win);
			else if (MBG) DisposHandle(MBG);
			if (err) WarnUser(COULDNT_MBOX,err);
			return;
	}
	SelectWindow(Win);
}

/************************************************************************
 * MBDidResize - resize the MB window
 ************************************************************************/
void MBDidResize(MyWindowPtr win, Rect *oldContR)
{
#pragma unused(oldContR)
	Rect r;
	short hi,wi;
	short h1,h2,h3,h4;
	short v1,v2,v3,v4;
	Point cSize;
#define INSET 6

	/*
	 * setup some stuff
	 */
	hi = (*FromRename)->contrlRect.bottom - (*FromRename)->contrlRect.top;
	h1 = INSET;
	h4 = Win->contR.right-INSET;
	wi = MIN(108,3*(h4-h1-2*INSET)/10);
	h2 = h1 + (h4-h1-wi)/2;
	h3 = h4 - (h2-h1);
	v1 = INSET;
	v2 = 2*INSET+hi;
	v3 = Win->contR.bottom - 3*INSET - 2*hi;
	v4 = v3 + INSET;
	wi = (h2-h1-GROW_SIZE)/2;

		
	/*
	 * size the left-hand list area, including scroll bar
	 */
	SetRect(&r,h1,v2,h2-GROW_SIZE,v3);
	
	/*
	 * move the controls - left side
	 */
	MoveMyCntl(Win,FromRename,h1,v4,wi,hi);
	MoveMyCntl(Win,FromNew,h2-wi,v4,wi,hi);
	MoveMyCntl(Win,FromRemove,(h1+h2-wi)/2,v4+hi+INSET,wi,hi);
	MoveMyCntl(Win,FromMove,h2+INSET,v2+(v3-v2)/3-hi,h3-h2-2*INSET,hi);
	
	/*
	 * right side
	 */
	MoveMyCntl(Win,ToRename,h3,v4,wi,hi);
	MoveMyCntl(Win,ToNew,h4-wi,v4,wi,hi);
	MoveMyCntl(Win,ToRemove,(h4+h3-wi)/2,v4+hi+INSET,wi,hi);
	MoveMyCntl(Win,ToMove,h2+INSET,v2+2*(v3-v2)/3,h3-h2-2*INSET,hi);
	
	/*
	 * top
	 */
	MoveMyCntl(Win,FromMenu,h1,v1,h2-h1,hi);
	MoveMyCntl(Win,ToMenu,h3,v1,h4-h3,hi);
	
	/*
	 * left-side list
	 */
	LDoDraw(False,FromSide.list);
	(*FromSide.list)->rView = r;
	hi = r.bottom-r.top;
	wi = r.right-r.left;
	cSize.v = GetLeading(0,0);
	cSize.h = wi;
	LSize(wi,hi,FromSide.list);
	LCellSize(cSize,FromSide.list);
	LDoDraw(True,FromSide.list);
	
	/*
	 * right-side list
	 */
	LDoDraw(False,ToSide.list);
	OffsetRect(&r,h3-h1,0);
	(*ToSide.list)->rView = r;
	LSize(wi,hi,ToSide.list);
	LCellSize(cSize,ToSide.list);
	LDoDraw(True,ToSide.list);
	
	/*
	 * redraw
	 */
	MBSetControls();
	InvalContent(win);
}

#pragma segment Main
/************************************************************************
 * MBClose - close the window
 ************************************************************************/
Boolean MBClose(MyWindowPtr win)
{
#pragma unused(win)
	if (!MBG) return(True);
	if (FromSide.list) LDispose(FromSide.list);
	if (ToSide.list) LDispose(ToSide.list);
	DisposHandle(MBG);
	MBG = nil;
	return(True);
}
#pragma segment MBoxWin

/************************************************************************
 * MBUpdate - draw the window
 ************************************************************************/
void MBUpdate(MyWindowPtr win)
{
	Rect r;
	
	r = (*FromSide.list)->rView;
	InsetRect(&r,-1,-1);
	FrameRect(&r);
	r = (*ToSide.list)->rView;
	InsetRect(&r,-1,-1);
	FrameRect(&r);
	LUpdate(((GrafPtr)win)->visRgn,FromSide.list);
	LUpdate(((GrafPtr)win)->visRgn,ToSide.list);
}

/************************************************************************
 * MBActivate - activate the window
 ************************************************************************/
void MBActivate(void)
{
	LActivate(Win->isActive,FromSide.list);
	LActivate(Win->isActive,ToSide.list);
}

/************************************************************************
 * MBRefill - refill current menus
 ************************************************************************/
short MBRefill(short whichSide,UPtr andSelect)
{
	Point oldCell={0,0};
	short err;
	Boolean selected;
	
	selected = LGetSelect(True,&oldCell,Sides[whichSide].list);
	err = MBFillSide(whichSide,GetMHandle(Sides[whichSide].id));
	if (andSelect)
	{
		Point newCell={0,0};
		if (LSearch(andSelect,2+andSelect[1],nil,&newCell,Sides[whichSide].list))
		{
			oldCell = newCell;
			selected = True;
		}
	}
	if (selected)
	{
		if (oldCell.v>=(*Sides[whichSide].list)->dataBounds.bottom)
			oldCell.v = (*Sides[whichSide].list)->dataBounds.bottom;
		LSetSelect(True,oldCell,Sides[whichSide].list);
		LAutoScroll(Sides[whichSide].list);
	}
	return(err);
}

/************************************************************************
 * MBFill - fill the windows
 ************************************************************************/
short MBFill(void)
{
	short err;
	MenuHandle mh=GetMHandle(MAILBOX_MENU);
	if (err=MBFillSide(FromSideIndex,mh)) return(err);
	if (err=MBFillSide(ToSideIndex,mh)) return(err);
	return(noErr);
}

/************************************************************************
 * MBFillSide - fill a list from a menu
 ************************************************************************/
short MBFillSide(short whichSide, MenuHandle mh)
{
	Str63 name;
	short i,n;
	Boolean hasSub;
	short base=0;
	Point c;
	short err;
	Rect scrap;
	MenuHandle topMenu;
	ControlHandle topControl = (whichSide?ToMenu : FromMenu);
	
	topMenu = (MenuHandle)*(long *)*(*topControl)->contrlData;
	
	LDoDraw(False,Sides[whichSide].list);
	LDelRow(INFINITY,0,Sides[whichSide].list);
	n = CountMItems(mh);
	c.h = 0;
	SetPort(Win);
	i = (mh==GetMHandle(MAILBOX_MENU)) ? 5 : 1;
	for (;i<=n;i++)
	{
		hasSub = HasSubmenu(mh,i);
		MyGetItem(mh,i,name+1);
		name[0] = hasSub;
		if (!base)
		{
			if (err=LAddRow(n-i+1,INFINITY,Sides[whichSide].list))
			return(WarnUser(COULDNT_MBOX,err));
			base = i;
		}
		c.v = i-base;
		LSetCell(name,name[1]+2,c,Sides[whichSide].list);
	}
	LDoDraw(True,Sides[whichSide].list);
	scrap = (*Sides[whichSide].list)->rView;
	scrap.right += GROW_SIZE;
	InsetRect(&scrap,-2,-2);
	InvalRect(&scrap);
	Sides[whichSide].menu = mh;
	Sides[whichSide].id = (*mh)->menuID;
	Sides[whichSide].dirId = DIRID_OF(Sides[whichSide]);
	
	TrashMenu(topMenu,1);
	while(1)
	{
		if (mh==GetMHandle(MAILBOX_MENU))
		{
			GetDirName(nil,MyVRef,MyDirId,name);
			MyAppendMenu(topMenu,name);
			break;
		}
		else
		{
			mh=ParentMailboxMenu(mh,&i);
			MyGetItem(mh,i,name);
			MyAppendMenu(topMenu,name);
		}
	}
	SetCtlMin(topControl,1);
	SetCtlMax(topControl,CountMItems(topMenu));
	SetCtlValue(topControl,1);
	return(noErr);
}

/************************************************************************
 * MBClick - click in mailboxes window
 ************************************************************************/
void MBClick(MyWindowPtr win,EventRecord *event)
{
#pragma unused(win);
	Rect r;
	short i;
	Point pt,c;
	
	pt = event->where;
	GlobalToLocal(&pt);
	for (i=0;i<NSIDES;i++)
	{
		r = (*Sides[i].list)->rView;
		r.right += GROW_SIZE;
		if (PtInRect(pt,&r))
		{
			if (MyLClick(pt,event->modifiers,Sides[i].list))
				MBListOpen(i);
			while (FirstSelected(&c,Sides[1-i].list))
				LSetSelect(False,c,Sides[1-i].list);
			break;
		}
	}
	
	if (i>=NSIDES)
		for (i=0;i<ControlCount;i++)
		{
			r = (*Controls[i])->contrlRect;
			if (PtInRect(pt,&r))
			{
				if ((*Controls[i])->contrlRfCon!='GREY' &&
						TrackControl(Controls[i],pt,(void *)(-1)))
					MBHit(i);
				break;
			}
		}
	
	MBSetControls();
}

/************************************************************************
 * MBCursor - set the cursor properly for the mailboxes window
 ************************************************************************/
void MBCursor(Point mouse)
{
	short i;
	
	for (i=0;i<NSIDES;i++)
	{
		if (CursorInRect(mouse,(*Sides[i].list)->rView,MouseRgn))
		{
			SetMyCursor(plusCursor);
			break;
		}
	}
	if (i==NSIDES) SetMyCursor(arrowCursor);
}

/************************************************************************
 * MBSetControls - enable or disable the controls, based on current situation
 ************************************************************************/
void MBSetControls(void)
{
	Cell c,nc;
	Boolean fromSelect, fromMultiple, toSelect, toMultiple;
	
	c.h = c.v=0;
	nc = c; toSelect = LGetSelect(True,&nc,ToSide.list);
	nc.v++; toMultiple = toSelect && LGetSelect(True,&nc,ToSide.list);
	nc = c; fromSelect = LGetSelect(True,&nc,FromSide.list);
	nc.v++; fromMultiple = fromSelect && LGetSelect(True,&nc,FromSide.list);
	
	SetGreyControl(FromMove,!fromSelect||toSelect||DontMove());
	SetGreyControl(FromRename,!fromSelect||fromMultiple);
	SetGreyControl(FromRemove,!fromSelect);
	SetGreyControl(ToMove,!toSelect||fromSelect||DontMove());
	SetGreyControl(ToRename,!toSelect||toMultiple);
	SetGreyControl(ToRemove,!toSelect);
}

/************************************************************************
 * MBHit - a control item was hit
 ************************************************************************/
void MBHit(short which)
{
	Point c;
	Str63 data,otherData,newData;
	Boolean andShutUp=False;
	Boolean needRefill=False;
	UPtr fromSelect = nil;
	UPtr toSelect = nil;
	short si,lev;
	MenuHandle mh;
	long moreJunk;
	Boolean folder;
	long newDirId;
	
	si = which<=FromMoveIndex || which==FromMenuIndex ? FromSideIndex : ToSideIndex;
	switch (which)
	{
		case FromRenameIndex:
		case ToRenameIndex:
			FirstSelected(&c,Sides[si].list);
			MBGetData(data,c,Sides[si].list);
			DoRenameBox(DIRID_OF(Sides[si]),data+1,newData+1);
			newData[0] = data[0];
			fromSelect = newData;
			if (FirstSelected(&c,Sides[1-si].list))
			{
				MBGetData(otherData,c,Sides[1-si].list);
				toSelect = FromSide.id==ToSide.id && EqualString(data+1,otherData+1,False,True) ?
					newData:otherData;
			}
			needRefill=True;
			break;
		case FromRemoveIndex:
		case ToRemoveIndex:
			for (c.h = c.v = 0;LGetSelect(True,&c, Sides[si].list);c.v++)
			{
				MBGetData(data,c,Sides[si].list);
				if (!DoRemoveBox(Sides[si].menu,data+1,&andShutUp)) break;
			}
			needRefill=True;
			break;
		case FromMoveIndex:
		case ToMoveIndex:
			DoMoveMailbox(otherData,si);
			if (FirstSelected(&c,Sides[1-si].list)) toSelect = otherData;
			needRefill = True;
			break;
		case FromNewIndex:
		case ToNewIndex:
			if (GetNewMailbox(DIRID_OF(Sides[si]),data,&folder,&newDirId,nil))
			{
				needRefill = True;
				fromSelect = data;
			}
			break;
		case FromMenuIndex:
		case ToMenuIndex:
			lev = GetCtlValue(Controls[which]);
			if (lev>1)
			{
				mh = Sides[si].menu;
				while (--lev) mh = ParentMailboxMenu(mh,&moreJunk);
				MBFillSide(si,mh);
			}
			break;
	}
	if (needRefill)
	{
		BuildBoxMenus();
		if (si==FromSideIndex) MBTickle(fromSelect,toSelect);
		else MBTickle(toSelect,fromSelect);
	}
}

/************************************************************************
 * MBListOpen - open some stuff from a list handle
 ************************************************************************/
void MBListOpen(short whichSide)
{
	Cell c;
	Str63 data;
	
	for (c.h = c.v = 0;LGetSelect(True,&c,Sides[whichSide].list);c.v++)
	{
		MBGetData(data,c,Sides[whichSide].list);
		if (!data[0])
			(void) GetMailbox(DIRID_OF(Sides[whichSide]),data+1,True);
		else
		{
			MBFillSide(whichSide,GetMHandle(CellMenuId(c,whichSide)));
			break;
		}
	}
}

/************************************************************************
 * ParentMailboxMenu - find the menu enclosing this one
 ************************************************************************/
MenuHandle ParentMailboxMenu(MenuHandle mh,short *itemPtr)
{
	short myID, curID;
	
	curID = myID = (*mh)->menuID;
	while (--curID)
	{
		mh = GetMHandle(curID);
		if (*itemPtr = FindItemBySubmenu(mh,myID)) return(mh);
	}
	
	mh = GetMHandle(MAILBOX_MENU);
	if (*itemPtr = FindItemBySubmenu(mh,myID)) return(mh);
	return(nil);
}

/************************************************************************
 * FindItemBySubmenu - find a menu item with a given submenu
 ************************************************************************/
short FindItemBySubmenu(MenuHandle mh, short subID)
{
	short item, newID;
	for (item=CountMItems(mh);HasSubmenu(mh,item);item--)
	{
		GetItemMark(mh,item,&newID);
		if (newID==subID) return(item);
	}
	return(0);
}


/************************************************************************
 * DoRenameBox - handle interaction with the user for renaming a mailbox
 ************************************************************************/
void DoRenameBox(long inDirId,UPtr name,UPtr newName)
{
	DialogPtr dgPtr;
	short item;
	TOCHandle tocH;
	Boolean isFolder=IsItAFolder(MyVRef,inDirId,name);
	long newDirId;
	
	if (!isFolder && (!(tocH=TOCByName(inDirId,name)) || (*tocH)->which))
	{
		WarnUser(MAYNT_RENAME_BOX,0);
		return;
	}
	
	MyParamText(name,"","","");
	
	ThirdCenterDialog(RENAME_BOX_DLOG);
	if ((dgPtr = GetNewDialog(RENAME_BOX_DLOG,nil,InFront))==nil)
	{
		WarnUser(GENERAL,MemError());
		return;
	}
	
	SetDIText(dgPtr,NEW_MAILBOX_NAME,name);
	ShowWindow(dgPtr);
	HiliteButtonOne(dgPtr);
	do
	{
		MyParamText(name,"","","");
		SelIText(dgPtr,NEW_MAILBOX_NAME,0,INFINITY);
		PushCursor(iBeamCursor);
		ModalDialog(DlgFilter,&item);
		PopCursor();
		GetDIText(dgPtr,NEW_MAILBOX_NAME,newName);
	}
	while (item==1 && !EqualString(name,newName,False,True) &&
				 BadMailboxName(inDirId,newName,isFolder,&newDirId));
	
	DisposDialog(dgPtr);
	
	if (item==1)
	{
		if (isFolder)
		{
			if (!EqualString(name,newName,False,True))
				HDelete(MyVRef,inDirId,newName);
			if (HRename(MyVRef,inDirId,name,newName)) return;
		}
		else
		{
			if (!EqualString(name,newName,False,True))
				HDelete(MyVRef,inDirId,newName);
			if (RenameMailbox(inDirId,name,newName,isFolder)) return;
			(*tocH)->dirty = True;
			PCopy((*tocH)->name,newName);
			SetWTitle((*tocH)->win,newName);
		}
	}
}

/************************************************************************
 * DoRemoveBox - interact with the user before removing a mailbox
 ************************************************************************/
Boolean DoRemoveBox(MenuHandle mh,UPtr name,Boolean *andShutUp)
{
	TOCHandle tocH;
	Boolean isFolder;
	Boolean isEmpty;
	short clicked;
	long inDirId;
	
	inDirId = (mh==GetMHandle(MAILBOX_MENU))? MyDirId : (*BoxMap)[(*mh)->menuID];
	isFolder = IsItAFolder(MyVRef,inDirId,name);	
	if (isFolder)
		isEmpty = FolderFileCount(inDirId,name)<=0;
	else
	{
		if (!(tocH=TOCByName(inDirId,name))) return(False);
		if ((*tocH)->which)
		{
			WarnUser(MAYNT_DELETE_BOX,0);
			return(False);
		}
		isEmpty = (*tocH)->count==0;
	}
	
	if (!*andShutUp)
	{
		clicked = AlertStr(isEmpty?DELETE_EMPTY_ALRT:DELETE_NON_EMPTY_ALRT,
											 Caution,name);
		if (clicked==4) *andShutUp = True;
		else if (clicked!=2) return(False);
	}
	
	if (isFolder)
	{
		short item = FindItemByName(mh,name); 
		if (DeleteMailFolder(mh,item)) return(False);
	}
	else
		RemoveMailbox(inDirId,name);
	return(True);
}

/************************************************************************
 * DeleteMailFolder - delete a folder, including contents
 ************************************************************************/
short DeleteMailFolder(MenuHandle mh,short item)
{
	short err=0;
	Str31 name;
	Str31 boxName;
	MenuHandle subMh;
	long dirId = mh==GetMHandle(MAILBOX_MENU) ?
											 MyDirId :(*BoxMap)[(*mh)->menuID];
	
	MyGetItem(mh,item,name);
	GetItemMark(mh,item,&item);
	if (!item) return(FileSystemError(DELETING_BOX,name,fnfErr));
	subMh = GetMHandle(item);
	
	for (item=CountMItems(subMh);item;item--)
		if (HasSubmenu(subMh,item))
		{
			if (err=DeleteMailFolder(subMh,item)) return(err);
		}
		else
		{
			MyGetItem(subMh,item,boxName);
			RemoveMailbox((*BoxMap)[(*subMh)->menuID],boxName);
		}
	
	/*
	 * are we deleting or changing the index of a folder we're displaying?
	 */
	{
		short side;
		for (side=0;side<=1;side++)
			if (Sides[side].id==(*subMh)->menuID)
				SetSideIndices(side,mh);
	}

	/*
	 * do the deletion
	 */
	HSetVol(nil,MyVRef,MyDirId);
	if (err=HDelete(MyVRef,dirId,name))
		FileSystemError(DELETING_BOX,name,err);
	if (err) return(err);
}

/************************************************************************
 * SetSideIndices - set the side information based on a menu
 ************************************************************************/
void SetSideIndices(short side, MenuHandle mh)
{
	Sides[side].menu = mh;
	Sides[side].id = (*mh)->menuID;
	Sides[side].dirId = DIRID_OF(Sides[side]);
}

/************************************************************************
 * 
 ************************************************************************/

void DoMoveMailbox(UPtr selectName,short si)
{
	Str63 fromName;
	Str15 suffix;
	UPtr toName=nil;
	long fromDirId, toDirId, finalDirId;
	Cell c;
	short err;
	TOCHandle tocH;
	
	c.h = c.v = 0;
	toDirId = finalDirId = DIRID_OF(Sides[1-si]);
	fromDirId = DIRID_OF(Sides[si]);
	for (c.h=c.v=0;LGetSelect(True,&c,Sides[si].list);c.v++)
	{
		MBGetData(fromName,c,Sides[si].list);
		if (err=HMove(MyVRef,fromDirId,fromName+1,toDirId,toName))
			{FileSystemError(MOVE_MAILBOX,fromName,err); return;}
		PCat(fromName+1,GetRString(suffix,TOC_SUFFIX));
		if ((err=HMove(MyVRef,fromDirId,fromName+1,toDirId,toName)) &&
				err!=fnfErr)
			{FileSystemError(MOVE_MAILBOX,fromName,err); return;}
		fromName[1] -= *suffix;
		if (tocH=FindTOC(fromDirId,fromName+1))
			(*tocH)->dirId = finalDirId;
		PCopy(selectName,fromName);
	}
}

/************************************************************************
 * MBFixSideIndices - make sure the id and dirId match for both sides
 ************************************************************************/
void MBFixSideIndices(void)
{
  short i,j,newid;
  for (i=0;i<=1;i++)
	  if (DIRID_OF(Sides[i])!=Sides[i].dirId)
		{
			newid = MAILBOX_MENU;
			for (j=0;j<GetHandleSize(BoxMap)/sizeof(short);j++)
			  if ((*BoxMap)[j]==Sides[i].dirId) {newid=j;break;}
			SetSideIndices(i,GetMHandle(newid));
		}
}

/************************************************************************
 * CellMenuId - find the menu id for a given cell in a list
 ************************************************************************/
short CellMenuId(Cell c,short whichSide)
{
	short item,id;
	Str63 data;
	
	MBGetData(data,c,Sides[whichSide].list);
	if (!data[0]) return(0);
	item = FindItemByName(Sides[whichSide].menu,data+1);
	GetItemMark(Sides[whichSide].menu,item,&id);
	return(id);
}

UPtr MBGetData(UPtr data,Cell c,ListHandle list)
{
	short junk=64;
	LGetCell(data,&junk,c,list);
	return(data);
}

Boolean DontMove(void)
{
	short unSelId;
	Cell c;
	short si;
	
	if (FromSide.id==ToSide.id) return(True);
	c.h = c.v = 0;
	if (FirstSelected(&c,ToSide.list)) si = ToSideIndex;
	else if (FirstSelected(&c,FromSide.list)) si = FromSideIndex;
	else return(False);
	
	unSelId = Sides[1-si].id;
	
	do
	{
		if (CellMenuId(c,si)==unSelId) return(True);
		c.v++;
	}
	while(LGetSelect(True,&c,Sides[si].list));

	return(False);
}

void MBTickle(UPtr fromSelect,UPtr toSelect)
{
	if (MBG)
	{
		MBFixSideIndices();
		(void) MBRefill(FromSideIndex,fromSelect);
		(void) MBRefill(ToSideIndex,toSelect);
	}
}

#pragma segment Balloon
/************************************************************************
 * MBHelp - provide help for the mailbox window
 ************************************************************************/
void MBHelp(MyWindowPtr win,Point mouse)
{
#pragma unused(win);
	Rect r;
	short i;
	short hnum=1;
	
	for (i=0;i<NSIDES;i++)
	{
		r = (*Sides[i].list)->rView;
		r.right += GROW_SIZE;
		if (PtInRect(mouse,&r))
		{
			HelpRect(&r,MBWIN_HELP_STRN+hnum,100);
			return;
		}
	}
	hnum++;
	
	for (i=0;i<ControlCount;i++,hnum+=2)
	{
		r = (*Controls[i])->contrlRect;
		if (PtInRect(mouse,&r))
		{
			if ((*Controls[i])->contrlRfCon!='GREY') hnum++;
			HelpRect(&r,MBWIN_HELP_STRN+hnum,100);
			return;
		}
	}
}

#pragma segment MboxWin
/************************************************************************
 * MBMenu - menu choice in the mailbox window
 ************************************************************************/
Boolean MBMenu(MyWindowPtr win, int menu, int item, short modifiers)
{
#pragma unused(win,modifiers)
	
	switch (menu)
	{
		case FILE_MENU:
			switch(item)
			{
				case FILE_OPEN_ITEM:
					MBListOpen(FromSideIndex);
					MBListOpen(ToSideIndex);
					return(True);
					break;
			}
			break;
	}
	return(False);
}
