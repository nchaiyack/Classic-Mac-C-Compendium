#define FILE_NUM 3
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/* Copyright (c) 1992 by Qualcomm, Inc. */
/**********************************************************************
 * handling the alias panel
 **********************************************************************/
#pragma load EUDORA_LOAD
#pragma segment NickWin
Boolean VetoKey(short key);

/************************************************************************
 * the structure for the alias window
 ************************************************************************/
typedef struct
{
	STEHandle exTE;
	STEHandle noTE;
	Rect exRect;
	Rect noRect;
	Rect niRect;
	ControlHandle buttons[6];
	ListHandle lHand;
	Str31 partialSelect;
	short selected;
	short multiple;
	long aOffset;
	MyWindowPtr win;
	Boolean isDirty;
} AliasType, *AliasPtr, **AliasHandle;
AliasHandle AliasGlobals; 			/* for the alias window */
#define ExTE (*AliasGlobals)->exTE
#define NoTE (*AliasGlobals)->noTE
#define ExRect (*AliasGlobals)->exRect
#define NoRect (*AliasGlobals)->noRect
#define NiRect (*AliasGlobals)->niRect
#define Buttons (*AliasGlobals)->buttons
#define RenameButton (*AliasGlobals)->buttons[0]
#define NewButton (*AliasGlobals)->buttons[2]
#define RemoveButton (*AliasGlobals)->buttons[3]
#define ToButton (*AliasGlobals)->buttons[1]
#define CcButton (*AliasGlobals)->buttons[4]
#define BccButton (*AliasGlobals)->buttons[5]
#define LHand (*AliasGlobals)->lHand
#define PartialSelect (*AliasGlobals)->partialSelect
#define Selected (*AliasGlobals)->selected
#define Multiple (*AliasGlobals)->multiple
#define AOffset (*AliasGlobals)->aOffset
#define Win (*AliasGlobals)->win
#define IsDirty (*AliasGlobals)->isDirty
#define Scrolls (*AliasGlobals)->scrolls
#define ExHBar (*AliasGlobals)->scrolls[0]
#define ExVBar (*AliasGlobals)->scrolls[1]
#define NoBar (*AliasGlobals)->scrolls[2]
#define IsRecip(name) BinFindItemByName(GetMHandle(NEW_TO_HIER_MENU),name)

/************************************************************************
 * private functions
 ************************************************************************/
	void AddAliasNames(void);
	void AliasActivate(void);
	void AliasClick(MyWindowPtr win, EventRecord *event);
	void AliasKey(MyWindowPtr win, EventRecord *event);
	Boolean AliasMenu(MyWindowPtr win, int menu, int item, short modifiers);
	void AliasUpdate(MyWindowPtr win);
	void AliasHelp(MyWindowPtr win,Point mouse);
	void AliasCursor(Point mouse);
	void DisplaySelectedAlias(void);
	void SaveCurrentAlias(void);
	void FlipRecip(void);
	void RemoveNick(void);
	void RenameNick(void);
	void SetSTETo(STEHandle teh);
	void ActOnAliasSelect(void);
	void SelectAlias(int which);
	Boolean AliasScroll(MyWindowPtr win,int h,int v);
	void UnselectAlias(void);
	void InsertTheAlias(short txeIndex,Boolean wantExpansion);
	void AliasDidResize(MyWindowPtr win,Rect *oldContR);
	void InvalAliasLeftSide(void);
	void AliasSetGreys(void);
	Boolean CanInsertAlias(void);
	short ListPartial(ListHandle list,UPtr partial,short size,EventRecord*event);
	Boolean CellMatch(ListHandle list,Point c,UPtr text);
	void TETr(TEHandle teh,UPtr fromS, UPtr toS);
	Boolean Tr(Handle text,UPtr fromS, UPtr toS);
	void AliasButtons(MyWindowPtr win,ControlHandle button,long modifiers,short part);
	void AliasTextChanged(MyWindowPtr win, TEHandle teh, int oldNl, int newNl,Boolean scroll);
	Boolean BadNickname(UPtr candidate,UPtr veteran);
	UPtr GetCurrentName(UPtr putItHere,Boolean *isRecip);
	void PutCurrentName(UPtr what, Boolean isRecip);
	void RemoveFromTo(UPtr name);
	long FindAliasOffsetByIndex(short index);
	Boolean NeedMenuCursor(Point mouse);
	short GetDivLine(void);
	Boolean AskNickname(UPtr name,Boolean *makeRecip,Handle addresses);
	void AliasApp1(MyWindowPtr win,EventRecord *event);


/************************************************************************
 * OpenAliases - open the alias window
 ************************************************************************/
void OpenAliases(void)
{
	STEHandle teh;
	Rect r,bounds;
	Point cSize;
	Handle grumble;
	Str31 scratch;
	short err,i;
	
	CycleBalls();
	/*
	 * allocate the globals
	 */
	if (AliasGlobals)
	{
		SelectWindow(Win);
		return;
	}
	
	if (!(AliasGlobals=NewZH(AliasType)))
		{WarnUser(MEM_ERR,MemError()); goto fail;}
	
	/*
	 * make sure the alias list is in memory
	 */
	if (RegenerateAliases(&Aliases,ALIAS_CMD) || RegenerateAliases(&Notes,NOTE_CMD))
	{
		return;
	}
	AliasRefCount++;
	
	/*
	 * get the labels
	 */
	LDRef(AliasGlobals);
	
	/*
	 * make the window
	 */
	Win = GetNewMyWindow(ALIAS_WIND,nil,InFront,False,False);
	UL(AliasGlobals);
	if (!Win) goto fail;
	Win->position = PositionPrefsTitle;
	SetPort(Win);
	r = Win->contR;
	

	/*
	 * the add and remove buttons
	 */
	for (i=0;i<sizeof(Buttons)/sizeof(Buttons[0]);i++)
	{
		if (!(grumble=GetNewControl(MAKE_TO_CNTL,Win)))
			{err=MemError(); goto fail;}
		else
		  {Buttons[i] = grumble;}
	}
	SetCTitle(RenameButton,GetRString(scratch,MBOX_RENAME));
	SetCTitle(RemoveButton,GetRString(scratch,MBOX_REMOVE));
	SetCTitle(NewButton,GetRString(scratch,MBOX_NEW));
	SetCTitle(ToButton,GetRString(scratch,HEADER_LABEL_STRN+TO_HEAD));
	SetCTitle(CcButton,GetRString(scratch,HEADER_LABEL_STRN+CC_HEAD));
	SetCTitle(BccButton,GetRString(scratch,HEADER_LABEL_STRN+BCC_HEAD));

	
	/*
	 * the alias list area
	 */
	cSize.v = FontLead; cSize.h = GetRLong(ALIAS_A_WIDTH)-GROW_SIZE;
	SetRect(&bounds,0,0,1,0);
	LHand = LNew(&r,&bounds,cSize,0,Win,True,False,False,True);
	if (!LHand) {WarnUser(MEM_ERR,MemError()); goto fail;}
	(*LHand)->indent.v = FontAscent;
	LDoDraw(False,LHand);
	
	
	/*
	 * the alias expansion area
	 */
	if (!(teh = NewSTE(Win,&r,True,True,True)))
		{WarnUser(MEM_ERR,MemError()); goto fail;}
	(*(*teh)->te)->crOnly = -1;
	ExTE = teh;
	
	
	/*
	 * TERec for notes
	 */
	if (!(teh = NewSTE(Win,&r,False,True,True)))
		{WarnUser(MEM_ERR,MemError()); goto fail;}
	NoTE = teh;
	
	
	/*
	 * now, stick in the alias names
	 */
	AddAliasNames();
	LDoDraw(True,LHand);
	
	/*
	 * fill out the window record, and draw it
	 */
	((WindowPeek)Win)->windowKind = ALIAS_WIN;
	Win->close = AliasClose;
	Win->update = AliasUpdate;
	Win->didResize = AliasDidResize;
  Win->click = AliasClick;
	Win->activate = AliasActivate;
	Win->cursor = AliasCursor;
	Win->dontControl = True;
	Win->vPitch = FontLead;
	Win->hPitch = FontWidth;
	Win->textChanged = AliasTextChanged;
	Win->button = AliasButtons;
	Win->key = AliasKey;
	Win->menu = AliasMenu;
	Win->minSize.h = 5*72;
	Win->minSize.v = 11*FontLead+4*GROW_SIZE;
	Win->help = AliasHelp;
	Win->app1 = AliasApp1;
	SetSTETo(nil);
	ShowMyWindow(Win);
	AliasSetGreys();
	UpdateMyWindow(Win);
	if ((*LHand)->dataBounds.bottom) SelectAlias(1);
	return;

fail:
	AliasClose(Win);
}

/************************************************************************
 * 
 ************************************************************************/
void AliasApp1(MyWindowPtr win,EventRecord *event)
{
	if (win->ste) STEApp1(win->ste,event);
	else
	{
		short page = RoundDiv((*LHand)->rView.bottom-(*LHand)->rView.top,(*LHand)->cellSize.v)-1;
		switch(event->message & charCodeMask)
		{
			case homeChar:
				page = -INFINITY/2;
				break;
			case endChar:
				page = INFINITY/2;
				break;
			case pageUpChar:
				page *= -1;
				break;
		}
		LScroll(0,page,LHand);
	}
}

#define ALIASLAYOUT \
	short aWidth=GetRLong(ALIAS_A_WIDTH);																			\
	short hi = (*CcButton)->contrlRect.bottom - (*CcButton)->contrlRect.top;	\
  short wi, wi2 ,advance;																										\
	short h1,h2,h3,h4,v1,v2,v3,v4,v5;																					\
	h1 = INSET+GROW_SIZE; h2 = h1+aWidth; h3 = h2+INSET;											\
	h4 = Win->contR.right-INSET;																							\
	v1 = 2*INSET+GROW_SIZE; v4 = Win->contR.bottom - 3*INSET - 2*hi;					\
	v2 = v1 + (GetRLong(ALIAS_ADDR_PC)*(v4-v1-3*INSET-GROW_SIZE))/100;				\
	v3 = v2 + 2*INSET + GROW_SIZE;																						\
	v5 = Win->contR.bottom - (Win->contR.bottom-v4-hi)/2-hi;									\
	wi = (h2 - h1 - INSET)/2;																									\
	wi2 = (h4 - h3 - 2*INSET)/3;																							\
	advance = wi2+INSET;

/************************************************************************
 * AliasDidResize - size the alias window
 ************************************************************************/
void AliasDidResize(MyWindowPtr win,Rect *oldContR)
{
#define INSET 6
#pragma unused(win,oldContR)
	Rect r;
	Point cSize;
	ALIASLAYOUT
	
	SetPort(Win);

	/*
	 * buttons
	 */
	MoveMyCntl(Win,RenameButton,h1,v4+INSET,wi,hi);
	MoveMyCntl(Win,NewButton,h2-wi,v4+INSET,wi,hi);
	MoveMyCntl(Win,RemoveButton,h1+(h2-h1-wi)/2,v4+2*INSET+hi,wi,hi);
	MoveMyCntl(Win,ToButton,h3,v5,wi2,hi);
	MoveMyCntl(Win,CcButton,h3+advance,v5,wi2,hi);
	MoveMyCntl(Win,BccButton,h3+2*advance,v5,wi2,hi);

	/*
	 * the alias list area
	 */
	cSize.v = FontLead; cSize.h = aWidth-GROW_SIZE;
	SetRect(&r,h1,v1,h2,v4);
	NiRect = r;
	InsetRect(&r,1,1);
	r.right -= GROW_SIZE;
	LDoDraw(False,LHand);
	(*LHand)->rView = r;
	LSize(r.right-r.left,r.bottom-r.top,LHand);
	LDoDraw(True,LHand);
	
	
	/*
	 * the alias expansion area
	 */
	SetRect(&r,h3,v1,h4,v2);
	ExRect = r;
	ResizeSTE(ExTE,&r);
	
	/*
	 * TERec for editing notes
	 */
	SetRect(&r,h3,v3,h4,v4);
	NoRect = r;
	ResizeSTE(NoTE,&r);

	InvalRect(&Win->contR);
}

#pragma segment Main
/************************************************************************
 * AliasClose - close the alias window
 ************************************************************************/
Boolean AliasClose(MyWindowPtr win)
{
	int which=WANNA_SAVE_SAVE;
	if (!AliasGlobals) return(True);
	if (win && win->isDirty)
	{
		which = WannaSave(win);
		if (which==WANNA_SAVE_CANCEL || which==CANCEL_ITEM) return(False);
		else if (which == WANNA_SAVE_SAVE && !SaveAliases()) return(False);
	}

	if (LHand) LDispose(LHand);
	if (ExTE) STEDispose(ExTE);
	if (NoTE) STEDispose(NoTE);
	DisposHandle(AliasGlobals);
	AliasGlobals = nil;
	if (win) win->txe = nil;
	if (which==WANNA_SAVE_DISCARD)
	{
		AliasRefCount = 0;
		DisposHandle(Aliases);
		Aliases = nil;
	}
	else if (Aliases && AliasRefCount && !--AliasRefCount) HPurge(Aliases);
	HPurge(Notes);
	return(True);
}
#pragma segment AliasWin

/************************************************************************
 * AliasUpdate - draw the alias window
 ************************************************************************/
void AliasUpdate(MyWindowPtr win)
{
	Rect r;
	Str63 scratch;
	ALIASLAYOUT
	
	TextFont(0);
	TextSize(0);

	GetRString(scratch,ALIAS_A_LABEL);
	MoveTo(h1,v1-INSET);
	DrawString(scratch);

	GetRString(scratch,ALIAS_E_LABEL);
	MoveTo(h3,v1-INSET);
	DrawString(scratch);

	GetRString(scratch,ALIAS_N_LABEL);
	MoveTo(h3,v3-INSET);
	DrawString(scratch);

	FRAME_RECT(&NiRect);
	if (Win->isActive && !win->ste) {r = NiRect; HotRect(&r,True);}
	
	STEUpdate(ExTE);
	STEUpdate(NoTE);
	TextFont(FontID);
	TextSize(FontSize);
	LUpdate(((GrafPtr)win)->visRgn,LHand);
	OutlineControl(ToButton,win->isActive&&!InBG);
		
	if (!Win->isActive)
	{
		if (Selected&&!Multiple &&
			(*LHand)->visible.top <= Selected && Selected <= (*LHand)->visible.bottom)
		{
			Point pt;
			short v = (*LHand)->cellSize.v;
			PolyHandle pH;
			
			pt.h = h1 - 4;
			pt.v = v1 + ((2*(Selected-(*LHand)->visible.top)-1)*v)/2 + 1;
			MoveTo(pt.h,pt.v);
			if (pH=OpenPoly())
			{
				Line(4-pt.h,(1-v)/2);
				Line(0,v-1);
				LineTo(pt.h,pt.v);
				ClosePoly();
				FillPoly(pH,&qd.black);
				KillPoly(pH);
			}
		}
	}
}

/************************************************************************
 * AddAliasNames - put the names of the aliases into the list
 ************************************************************************/
void AddAliasNames(void)
{
	Str255 alias;
	long spot;
	short length;
	long max=GetHandleSize(Aliases);

	for (spot=Selected=0; spot<max; spot+=CountAliasTotal(Aliases,spot))
	{
		Selected++;
		length = CountAliasAlias(Aliases,spot);
		PCopy(alias,(*Aliases)+spot);
		(void) LAddRow(1,-1,LHand);
		PutCurrentName(alias,IsRecip(alias));
	}

	Selected = 0;
}

/************************************************************************
 *
 ************************************************************************/
void AliasesFixFont(void)
{
	TEFixup((*NoTE)->te); ResizeSTE(NoTE,nil);
	TEFixup((*ExTE)->te); ResizeSTE(ExTE,nil);
	TextFont(0);
	TextSize(0);
}

/************************************************************************
 * ChooseAlias - pick an alias
 ************************************************************************/
void ChooseAlias(short which)
{
	GrafPtr oldPort;
	
	if (Selected==which && !Multiple) return;
	
	GetPort(&oldPort);
	SetPort(Win);
	UnselectAlias();
	SelectAlias(which);
	LAutoScroll(LHand);
	SetPort(oldPort);
}


/************************************************************************
 * SetSTETo - make sure the alias window's STE is where it should be
 ************************************************************************/
void SetSTETo(STEHandle ste)
{
	Rect r;
	Point mouse;
	if (Win->ste != ste)
	{
		if (Win->ste)
			TEDeactivate((*(STEHandle)Win->ste)->te);
		Win->ste = ste;
		if (ste)
		{
		  TEActivate((*ste)->te);
			r=NiRect;HotRect(&r,False);
		}
		else
		{
		  r = NiRect; HotRect(&r,True);
		}
	}
	GetMouse(&mouse);
	AliasCursor(mouse);
	AliasSetGreys();
}

/************************************************************************
 * AliasClick - click in the alias window
 ************************************************************************/
void AliasClick(MyWindowPtr win, EventRecord *event)
{
	Rect exRect = ExRect;
	Rect noRect = NoRect;
	Rect niRect = NiRect;
	Point mouse = event->where;

	GlobalToLocal(&mouse);
	*PartialSelect = 0;
	if (PtInRect(mouse,&exRect))
	{
		exRect.top = exRect.bottom-GROW_SIZE;
		exRect.left = exRect.right-GROW_SIZE;
		if (!PtInRect(mouse,&exRect))
		{
			if (Selected && !Multiple)
			{
				SetSTETo(ExTE);
				STEClick(ExTE,event);
			}
		}
	}
	else if (PtInRect(mouse,&noRect))
	{
		if (Selected && !Multiple)
		{
			SetSTETo(NoTE);
			STEClick(NoTE,event);
		}
	}
	else
	{
		if (PtInRect(mouse,&niRect))
		{
			SetSTETo(nil);
			if (NeedMenuCursor(mouse))
				FlipRecip();
			else
			{
				Boolean dblClick=MyLClick(mouse,event->modifiers,LHand);
				ActOnAliasSelect();
				if (dblClick && Selected)
#ifdef I_HONESTLY_THINK_IT_COULD_BE_DONE_IN_ZINC
				{
					HiliteControl(ToButton,inButton);
					AliasButtons(Win,ToButton,event->modifiers,inButton);
					HiliteControl(ToButton,0);
				}
#else
					FlipRecip();
#endif
			}
		}
		else HandleControl(mouse,win);
	}
	Win->hasSelection = MyWinHasSelection(win);
	AliasSetGreys();
}


/************************************************************************
 * DisplaySelectedAlias - display the text of an alias in ExTE
 ************************************************************************/
void DisplaySelectedAlias(void)
{
	long offset;
	short length;
	Handle addresses,notes;
	Str63 name,isRecip;
	short err;
	
	STESetText("",0,ExTE);
	STESetText("",0,NoTE);
	if (Selected && !Multiple)
	{
		length = 255;
		AOffset = offset = FindAliasOffsetByIndex(Selected);
		length =	CountAliasExpansion(Aliases,offset);
		offset += CountAliasAlias(Aliases,offset)+1;
		LDRef(Aliases);
		addresses = SuckPtrAddresses((*Aliases)+offset+2,length,True);
		UL(Aliases);
		if (addresses)
		{
		  SetHandleBig((*(*ExTE)->te)->hText,0);
			FlattenListWith(addresses,'\n');
		  STEInstallText(addresses,ExTE);
		}
		GetCurrentName(name,&isRecip);
		if ((offset=FindAliasFor(Notes,name+1,*name))>=0)
		{
			if (err=PtrToHand(LDRef(Notes)+offset+*name+3,&notes,
												CountAliasExpansion(Notes,offset)))
				WarnUser(MEM_ERR,err);
			else
			{
				Tr(notes,"\003","\n");
				STEInstallText(notes,NoTE);
			}
			UL(Notes);
		}
	}
}

/************************************************************************
 * FindAliasOffsetByIndex - find the offset of the nth alias
 ************************************************************************/
long FindAliasOffsetByIndex(short index)
{
	long offset;
	offset = 0;
	while (--index)
	{
		offset += CountAliasTotal(Aliases,offset);
	}
	return(offset);
}


/************************************************************************
 * AliasActivate - handle things when the alias window is (de)activated
 ************************************************************************/
void AliasActivate(void)
{
  Rect r = NiRect;
	LActivate(Win->isActive,LHand);
	if (Selected && !Multiple) InvalAliasLeftSide();
	OutlineControl(ToButton,Win->isActive&&!InBG);
	if (Win->isActive)
	{
		AliasSetGreys();
		*PartialSelect = 0;
		if (!Win->ste) HotRect(&r,True);
	}
	else
	{
		SaveCurrentAlias();
		HotRect(&r,False);
	}
}

/************************************************************************
 * UnselectAlias - unselect the current alias
 ************************************************************************/
void UnselectAlias(void)
{
	Point c;
	c.h = c.v = 0;
	if (LGetSelect(True,&c,LHand))
	{
		InvalAliasLeftSide();
		SetSTETo(nil);
		do
		{
			LSetSelect(False,c,LHand);
		}
		while (LGetSelect(True,&c,LHand));
	}
}

/************************************************************************
 * SelectAlias - select a particular alias cell
 ************************************************************************/
void SelectAlias(int which)
{
	Point c;
	
	c.h = 0; c.v = which-1;
	LSetSelect(True,c,LHand);
	ActOnAliasSelect();
}

/************************************************************************
 * ActOnAliasSelect - react to the selection of an alias cell
 ************************************************************************/
void ActOnAliasSelect()
{
	short which, next;
	
	which = Next1Selected(0,LHand);
	next = Next1Selected(which,LHand);
	
	if (Multiple && next) return;	/* still a multiple selection */
	if (which==Selected && !next) return;	/* same single selection */
	SaveCurrentAlias();
	Selected = which;
	Multiple = next;
	DisplaySelectedAlias();
	SetSTETo(nil);
	InvalAliasLeftSide();
	AliasSetGreys();
}

/************************************************************************
 * InvalAliasLeftSide - invalidate the area to the left of the alias
 * selection rectangle
 ************************************************************************/
void InvalAliasLeftSide(void)
{
	Rect r=NiRect;
	SAVE_PORT;
	SetPort(Win);
	r.right = r.left-4;
	r.left = 0;
	InvalRect(&r);
	REST_PORT;
}

/************************************************************************
 * AliasSetGreys - set the greyness of controls
 ************************************************************************/
void AliasSetGreys(void)
{
	Boolean oldTo=(*ToButton)->contrlRfCon=='GREY';
	SetGreyControl(RemoveButton,!Selected);
	SetGreyControl(RenameButton,!Selected || Multiple);
	SetGreyControl(ToButton,!Selected);
	SetGreyControl(CcButton,!Selected);
	SetGreyControl(BccButton,!Selected);
	if (oldTo!=((*ToButton)->contrlRfCon=='GREY'))
	{
		Rect r = (*ToButton)->contrlRect;
		InsetRect(&r,-4,-4);
		InvalRect(&r);
	}
}

/************************************************************************
 *
 ************************************************************************/
Boolean CanInsertAlias(void)
{
	MyWindowPtr win;
	return((win=(((WindowPeek)FrontWindow())->nextWindow)) && IsMyWindow(win) &&
					win->qWindow.windowKind==COMP_WIN);
}

/************************************************************************
 * AliasKey - handle a keystroke in the alias window
 ************************************************************************/
void AliasKey(MyWindowPtr win, EventRecord *event)
{
	short key = (event->message & 0xff);
	
	if (win->ste)
		switch (key)
		{
			default:
				if (!(event->modifiers&cmdKey)) TESomething(win,TEKEY,key,event->modifiers);
				break;
			case enterChar:
				SaveCurrentAlias();
				break;
			case tabChar:
				if (win->ste==nil)
					SetSTETo((event->modifiers&shiftKey)?NoTE:ExTE);
				if (win->ste==ExTE)
					SetSTETo((event->modifiers&shiftKey)?nil:NoTE);
				else
					SetSTETo((event->modifiers&shiftKey)?ExTE:nil);
				if (win->ste) NoScrollTESetSelect(0,INFINITY,(*(STEHandle)win->ste)->te);
				break;
		}
	else
		switch (key)
		{
			case upArrowChar:
				ChooseAlias(Selected>1 ? Selected-1 : (*LHand)->dataBounds.bottom);
				break;
			case downArrowChar:
				ChooseAlias(Selected<(*LHand)->dataBounds.bottom ? Selected+1 : 1);
				break;
			case returnChar:
			case enterChar:
				if (Selected)
				{
					SaveCurrentAlias();
					HiliteControl(ToButton,inButton);
					AliasButtons(win,ToButton,event->modifiers,inButton);
					HiliteControl(ToButton,0);
				}
				break;
			case tabChar:
				if (Selected && !Multiple)
				{
					SetSTETo(event->modifiers&shiftKey ? NoTE : ExTE);
					NoScrollTESetSelect(0,INFINITY,(*(STEHandle)win->ste)->te);
				}
				break;
			default:
				if (!(event->modifiers&cmdKey))
				{
					short which;
					LDRef(AliasGlobals);
					which =
						ListPartial(LHand,PartialSelect,sizeof(PartialSelect),event)+1;
					UL(AliasGlobals);
					if (which>0) ChooseAlias(which);
				}
		}
	Win->hasSelection = MyWinHasSelection(win);
}

/************************************************************************
 * ListPartial
 ************************************************************************/
short ListPartial(ListHandle list,UPtr partial,short size,EventRecord*event)
{
	Point c;
	short key = event->message&charCodeMask;
	static long oldTicks=0;
	
	c.h = c.v = 0;
	if (key==backSpace || key==escChar)
	{
		*partial = 0;
		return(-1);
	}
	
	if (event->when - oldTicks > GetRLong(PARTIAL_TICKS)) *partial = 0;
	
	oldTicks = event->when;
	if (*partial && LGetSelect(True,&c,list) && !CellMatch(list,c,partial))
	 c.h = c.v = 0;

	if (*partial<size-1) PCatC(partial,key);
	partial[*partial+1] = 0;
	for (;c.v<(*list)->dataBounds.bottom;c.v++)
		if (CellMatch(list,c,partial)) return(c.v);
		
	return(-1);
}

/************************************************************************
 *
 ************************************************************************/
Boolean CellMatch(ListHandle list,Point c,UPtr text)
{
	Str63 name;
	short len=sizeof(name)-1;
	
	LGetCell(name+1,&len,c,list);
	if (len-1<*text) return(0);
	name[len] = 0;
	return(!striscmp(name+2,text+1));
}

/************************************************************************
 * TETr - translate text in a TERec
 ************************************************************************/
void TETr(TEHandle teh,UPtr fromS, UPtr toS)
{
	if (Tr((*teh)->hText,fromS,toS))
	{
		SAVE_PORT;
		TECalText(teh);
		INVAL_RECT(&(*teh)->viewRect);
		TESelView(teh);
		REST_PORT;
	}
}


/************************************************************************
 * Tr - translate text in a handle
 ************************************************************************/
Boolean Tr(Handle text,Uptr fromS, Uptr toS)
{
	UPtr end,spot;
	Boolean did=False;
	short fromChar,toChar;
	long size = GetHandleSize(text);
	
	end = *text+size;
	for (;*fromS;fromS++,toS++)
	{
		fromChar = *fromS;
		toChar = *toS;
		for (spot= *text;spot<end;spot++)
			if (*spot==fromChar)
			{
				did = True;
				*spot = toChar;
			}
	}
	return(did);
}

/************************************************************************
 * AliasCursor - set the cursor properly for the alias window
 ************************************************************************/
void AliasCursor(Point mouse)
{
	Rect r;
	short cell, divLine;
	short cursor;
	
	r = NiRect; r.right -= GROW_SIZE;
	if (CursorInRect(mouse,r,MouseRgn))
	{
		cursor = plusCursor;	/* probably */
		cell=InWhich1Cell(mouse,LHand);
		r.top += Win->vPitch * (cell-(*LHand)->visible.top-1);
		r.bottom = r.top+Win->vPitch;
		divLine = GetDivLine();
		if (mouse.h<divLine)
		{
			r.right = divLine;
			if (Cell1Selected(cell,LHand)) cursor = MENU_CURS;
		}
		else
			r.left = divLine;
		SetMyCursor(cursor);
		RectRgn(MouseRgn,&r);
	}
	else if (CursorInRect(mouse,ExRect,MouseRgn))
	{
		if (Selected && !Multiple) STECursor(ExTE); else SetMyCursor(arrowCursor);
	}
	else if (CursorInRect(mouse,NoRect,MouseRgn))
	{
		if (Selected && !Multiple) STECursor(NoTE); else SetMyCursor(arrowCursor);
	}
	else
		SetMyCursor(arrowCursor);
}

/************************************************************************
 * AliasButtons - handle a click on a button
 ************************************************************************/
void AliasButtons(MyWindowPtr win,ControlHandle button,long modifiers,short part)
{
#pragma unused(win)
	if (part!=inButton) return;
	if (button==NewButton)
		NewNick(nil);
	else if (button==RenameButton)
		RenameNick();
	else if (button==RemoveButton)
		RemoveNick();
	else
	{
		if (Selected)
		{
			SaveCurrentAlias();
			if (button==ToButton)
			{
				InsertTheAlias(TO_HEAD-1,(modifiers&optionKey)!=0);
				if (FrontWindow()==Win) SelectWindow(Win->qWindow.nextWindow);
			}
			else if (button==CcButton)
				InsertTheAlias(CC_HEAD-1,(modifiers&optionKey)!=0);
			else if (button==BccButton)
				InsertTheAlias(BCC_HEAD-1,(modifiers&optionKey)!=0);
		}
	}
}

/************************************************************************
 * RemoveNick - get rid of a nickname
 ************************************************************************/
void RemoveNick(void)
{
	short s;
	Str63 name;
	Boolean isRecip;
	
	for (s=Next1Selected(0,LHand);s;s=Next1Selected(s-1,LHand))
	{
		Selected = s;
		GetCurrentName(name,&isRecip);
		LDelRow(1,s-1,LHand);
		if (isRecip) RemoveFromTo(name);
		RemoveAlias(Aliases,name);
		RemoveAlias(Notes,name);
	}
	Selected = -1;
	IsDirty = False;
	Win->isDirty = True;
	SelectAlias(0);
}

/************************************************************************
 * RemoveFromTo - remove something from the recipient menus
 ************************************************************************/
void RemoveFromTo(UPtr name)
{
	short menu,item;
	
	if (item=BinFindItemByName(GetMHandle(NEW_TO_HIER_MENU),name))
	{
		for (menu=NEW_TO_HIER_MENU;menu<=INSERT_TO_HIER_MENU;menu++)
			DelMenuItem(GetMHandle(menu),item);
		ToMenusChanged();
	}
}

/************************************************************************
 * RenameNick - rename a nickname
 ************************************************************************/
void RenameNick(void)
{
	Str63 name;
	Boolean makeRecip,wasRecip;
	
	GetCurrentName(name,&wasRecip);
	makeRecip = wasRecip;
	if (AskNickname(name,&makeRecip,nil))
	{
		UpdateMyWindow(Win);
		PutCurrentName(name,makeRecip);
		IsDirty = Win->isDirty = True;
		SaveCurrentAlias();
	}
}

/************************************************************************
 * GetCurrentName - get the currently selected nickname
 ************************************************************************/
UPtr GetCurrentName(UPtr putItHere,Boolean *isRecip)
{
	Point c;
	short len=31;
	
	if (!Selected)
		*putItHere = 0;
	else
	{
		c.h = 0; c.v = Selected-1;
		LGetCell(putItHere,&len,c,LHand);
		*isRecip = (*putItHere==bulletChar);
		*putItHere = len-1;
		putItHere[len] = 0;	/* null terminate, in case anybody cares */
	}
	return(putItHere);
}

/************************************************************************
 * PutCurrentName - put something into the currently selected nickname
 ************************************************************************/
void PutCurrentName(UPtr what,Boolean isRecip)
{
	Point c;
	Str31 scratch;
	
	PCopy(scratch,what);
	c.h = 0; c.v = Selected-1;
	*scratch = isRecip ? bulletChar : nbSpaceChar;
	LSetCell(scratch,*what+1,c,LHand);
}

/************************************************************************
 * NewNick - create a new alias for the user
 ************************************************************************/
void NewNick(Handle addresses)
{
	Point c;
	Str63 name;
	Boolean makeRecip=False;
	
	*name = 0;
	if (!AliasGlobals && RegenerateAliases(&Aliases,ALIAS_CMD)) return;
	AliasRefCount++;
	if (AskNickname(name,&makeRecip,addresses))
	{
		long addrSize = addresses ? GetHandleSize(addresses) : 0;
		if (addrSize) FlattenListWith(addresses,',');
		if (AliasGlobals) {UpdateMyWindow(Win);SaveCurrentAlias();}
		AddAlias(Aliases,name,addresses);
		if (makeRecip) AddStringAsTo(name);
		
		if (AliasGlobals)
		{
			UnselectAlias();
			c.v = LAddRow(1,INFINITY,LHand);
			c.h = 0;
			LSetSelect(True,c,LHand);
			LAutoScroll(LHand);
			SelectAlias(c.v+1);
			PutCurrentName(name,makeRecip);
			Win->isDirty = IsDirty = True;
			if (FrontWindow()==Win && !addrSize)
				SetSTETo(ExTE);
		}
		else
			SaveAliases();
	}
	AliasRefCount--;
}

/************************************************************************
 * AskNickname - ask the user for a new nickname.  Returns True if successful
 ************************************************************************/
Boolean AskNickname(UPtr name,Boolean *makeRecip,Handle addresses)
{
#pragma unused(addresses)
	Str255 workingName;
	DialogPtr dgPtr;
	short item;
	SAVE_PORT;
	
	ThirdCenterDialog(NEW_NICK_DLOG);
	if ((dgPtr = GetNewDialog(NEW_NICK_DLOG,nil,InFront))==nil)
		return(False);
		
	PCopy(workingName,name);
	SetDItemState(dgPtr,NEW_NICK_RECIP,*makeRecip);
	SetDIText(dgPtr,NEW_NICK_NAME,workingName);
	ShowWindow(dgPtr);
	HiliteButtonOne(dgPtr);
	do
	{
		SelIText(dgPtr,NEW_NICK_NAME,0,INFINITY);
		ModalDialog(DlgFilter,&item);
		if (item==NEW_NICK_RECIP)
			SetDItemState(dgPtr,NEW_NICK_RECIP,!GetDItemState(dgPtr,NEW_NICK_RECIP));
		else
			GetDIText(dgPtr,NEW_NICK_NAME,workingName);
	}
	while (item==NEW_NICK_RECIP || item==1 && BadNickname(workingName,name));
	
	if (item==1)
	{
		*makeRecip = GetDItemState(dgPtr,NEW_NICK_RECIP);
		PCopy(name,workingName);
	}
	DisposDialog(dgPtr);
	REST_PORT;
	return(item==1);
}

/************************************************************************
 * BadNickname - is the nickname defective?
 ************************************************************************/
Boolean BadNickname(UPtr candidate,UPtr veteran)
{
	Str31 verboten;
	UPtr spot,end,key;

	if (!*candidate) return(True);
	if (*candidate>MAX_NICKNAME)
	{
		WarnUser(ALIAS_TOO_LONG,MAX_NICKNAME);
		return(True);
	}
	if (*veteran && EqualString(candidate,veteran,False,True)) return(False);
	GetRString(verboten,ALIAS_VERBOTEN);
	end = verboten+*verboten+1;
	for (key=candidate+1;key<=candidate+*candidate;key++)
		for (spot=verboten+1;spot<end;spot++)
			if (*spot==*key)
			{
				WarnUser(WARN_VERBOTEN,0);
				return(True);
			}
	if (FindExpansionFor(Aliases,candidate+1,*candidate))
	{
		WarnUser(NICK_IN_USE,0);
		return(True);
	}
	return(False);
}

/************************************************************************
 * SaveCurrentAlias - save the expansion
 ************************************************************************/
void SaveCurrentAlias(void)
{
	Str63 newName,oldName;
	short maxAlias=GetRLong(BUFFER_SIZE)-40;
	UHandle addresses,text;
	Boolean isRecip;
	
	SetSTETo(nil);
	if (IsDirty)
	{
		GetCurrentName(newName,&isRecip);
		PCopy(oldName,(*Aliases)+AOffset);
		
		/*
		 * fix the expansion
		 */
		text = (*(*ExTE)->te)->hText;
		if (HandToHand(&text)) {WarnUser(COULDNT_MOD_ALIAS,MemError());return;}
		Tr(text,"\n",",");
		if (addresses=SuckAddresses(text,GetHandleSize(text),False)) DisposHandle(addresses);
		ReplaceAlias(Aliases,oldName,newName,text);
		DisposHandle(text);
		
		/*
		 * fix the notes
		 */
		text = (*(*NoTE)->te)->hText;
		if (HandToHand(&text)) {WarnUser(COULDNT_MOD_ALIAS,MemError());return;}
		if (GetHandleSize(text))
		{
			Tr(text,"\n","\003");
			if (FindAliasFor(Notes,oldName+1,*oldName)>=0)
				ReplaceAlias(Notes,oldName,newName,text);
			else
				AddAlias(Notes,newName,text);
		}
		else
			RemoveAlias(Notes,oldName);
		
		/*
		 * change recip menu
		 */
		RemoveFromTo(oldName);
		if (isRecip)
			AddStringAsTo(newName);
		IsDirty = False;
		NukeUndo(Win);
	}
}

/************************************************************************
 * 
 ************************************************************************/
void AliasTextChanged(MyWindowPtr win, TEHandle teh, int oldNl, int newNl, Boolean scroll)
{
#pragma unused(oldNl,newNl,scroll)
	IsDirty = True;
	if ((*ExTE)->te==teh) SetSTETo(ExTE);
	else if ((*NoTE)->te==teh) SetSTETo(NoTE);
	else return;
	STETextChanged(win->ste);
}

/************************************************************************
 * InsertTheAlias - insert the given alias in the window below us
 ************************************************************************/
void InsertTheAlias(short txeIndex,Boolean wantExpansion)
{
	MyWindowPtr win;
	GrafPtr oldPort;
	short s=0;
	
	win = ((WindowPeek)FrontWindow())->nextWindow;
	if (!IsMyWindow(win) || win->qWindow.windowKind!=COMP_WIN ||
	  SumOf(Win2MessH(win))->state==SENT)
	{
		win = DoComposeNew(0);
	}
	
	if (IsMyWindow(win) && win->qWindow.windowKind==COMP_WIN &&
	  SumOf(Win2MessH(win))->state!=SENT)
	{
		TEHandle oldTXE=win->txe;
		MessHandle messH = Win2MessH(win);
		GetPort(&oldPort);
		SetPort(win);
		win->txe = (*messH)->txes[txeIndex];
		TESetSelect(INFINITY,INFINITY,win->txe);
		while (s=Next1Selected(s,LHand))
		{
			InsertCommaIfNeedBe(win);
			InsertAlias(win,FindAliasOffsetByIndex(s),wantExpansion);
	  }
		win->txe = oldTXE;
		ShowMyWindow(win);
		SetPort(oldPort);
	}
}

/************************************************************************
 * AliasMenu - menu choice in the alias window
 ************************************************************************/
Boolean AliasMenu(MyWindowPtr win, int menu, int item, short modifiers)
{
#pragma unused(modifiers)
	Boolean res;
	short oldSel,s,err;
	Handle addresses;
	Str63 name;
	Boolean isRecip;
	Point pt;
	
	switch (menu)
	{
		case FILE_MENU:
			switch(item)
			{
				case FILE_SAVE_ITEM:
					SaveAliases();
					return(True);
					break;
			}
			break;
		case SPECIAL_MENU:
			switch(item)
			{
				case SPECIAL_MAKE_NICK_ITEM:
					oldSel = Selected;
					addresses = NuHandle(0);
					for (s=Next1Selected(0,LHand);s;s=Next1Selected(s,LHand))
					{
						Selected = s;
						GetCurrentName(name,&isRecip);
						if (err = PtrAndHand(name,addresses,*name+2))
							{WarnUser(MEM_ERR,err); break;}
					}
					Selected = oldSel;
					if (!err)
					{
						PtrAndHand("",addresses,1);	/* if we can't get one byte, to hell with it */
						NewNick(addresses);
					}
					if (addresses) DisposHandle(addresses);
					break;
			}
			break;
		case EDIT_MENU:
			if (win->ste)
			{
				res = TESomething(win,item,0,modifiers);
				Win->hasSelection = MyWinHasSelection(win);
				return(res);
			}
			else if (item==EDIT_SELECT_ITEM)
			{
				s = (*LHand)->dataBounds.bottom;
				for (pt.h=pt.v=0;pt.v<s;pt.v++) LSetSelect(True,pt,LHand);
			}
			break;
	}
	return(False);
}

/************************************************************************
 * FlipRecip - change the recipient state of some nicknames
 ************************************************************************/
void FlipRecip(void)
{
	Str63 name;
	Boolean isRecip,makeRecip;
	short oldSel = Selected;
	short s=0;
	
	GetCurrentName(name,&isRecip);
	makeRecip = !isRecip;
	while(s=Next1Selected(s,LHand))
	{
		Selected = s;
		GetCurrentName(name,&isRecip);
		if (isRecip != makeRecip)
		{
			PutCurrentName(name,makeRecip);
			if (makeRecip) AddStringAsTo(name);
			else RemoveFromTo(name);
		}
	}
}

/************************************************************************
 * NeedMenuCursor - do we need the menu-change cursor?
 ************************************************************************/
Boolean NeedMenuCursor(Point mouse)
{
	return (mouse.h<GetDivLine() &&
		Cell1Selected(InWhich1Cell(mouse,LHand),LHand));
}

/************************************************************************
 * GetDivLine - calculate the dividing line between the recip and nickname
 *	areas.
 ************************************************************************/
short GetDivLine(void)
{
	return((*LHand)->rView.left+Win->hPitch+(*LHand)->indent.h);
}

#pragma segment Main
/************************************************************************
 * AliasWinIsOpen - is the alias window open?
 ************************************************************************/
Boolean AliasWinIsOpen(void)
{
	return(AliasGlobals!=nil);
}

/************************************************************************
 * AliasWinGonnaSave - we're going to save the nickname list
 ************************************************************************/
void AliasWinGonnaSave(void)
{
	if (AliasGlobals) SaveCurrentAlias();
}

/************************************************************************
 * AliasWinDidSave - we just saved the nickname list
 ************************************************************************/
void AliasWinDidSave(void)
{
	if (AliasGlobals) Win->isDirty = False;
}

/************************************************************************
 * CanMakeNick - should we enable the "make nickname" item?
 ************************************************************************/
Boolean CanMakeNick(void)
{
  return(AliasGlobals&&Selected&&Multiple);
}

#pragma segment Balloon
/************************************************************************
 * AliasHelp - help for the alias window
 ************************************************************************/
void AliasHelp(MyWindowPtr win,Point mouse)
{
#pragma unused(win)
	Rect r;
	short c;
	short hnum=1;
	Boolean canInsert = CanInsertAlias();
	

	r = NoRect;
	if (PtInRect(mouse,&r))
	{
		if (Selected && !Multiple) hnum++;
		goto display;
	}
	hnum += 2;
	
	r = ExRect;
	if (PtInRect(mouse,&r))
	{
		if (Selected && !Multiple) hnum++;
		goto display;
	}
	hnum += 2;
	
	r = NiRect;
	if (PtInRect(mouse,&r))
	{
		if (!canInsert) hnum++;
		goto display;
	}
	hnum+=2;
	
	for (c=0;c<sizeof(Buttons)/sizeof(ControlHandle);c++,hnum+=3)
		if (PtInRect(mouse,&(*Buttons[c])->contrlRect))
		{
			r = (*Buttons[c])->contrlRect;
			if ((*Buttons[c])->contrlRfCon=='GREY') goto display;
			hnum++;
			if (!canInsert) hnum++;
			goto display;
		}

	return;
	
	display:
		HelpRect(&r,NICK_HELP_STRN+hnum,100);
}
