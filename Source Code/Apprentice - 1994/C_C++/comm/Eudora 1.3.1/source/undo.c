#define FILE_NUM 40
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * routines for undoing editing changes
 ************************************************************************/
#pragma load EUDORA_LOAD
#pragma segment Lib

/************************************************************************
 * DoUndo - undo the last undoable operation
 ************************************************************************/
void DoUndo(void)
{
	long size;
	UHandle redoText=nil;
	short oldNl;
	MyWindowPtr win;
	short selStart,selEnd;

	if (!Undo.teh) return;
	win = (*Undo.teh)->inPort;
	
	selStart = (*Undo.teh)->selStart;
	selEnd = (*Undo.teh)->selEnd;
	if (win->qWindow.windowKind==COMP_WIN) oldNl = CountTeLines(Undo.teh);
	
	if (size = Undo.goTil - Undo.startFrom)
	{
		if (!(redoText = NuHandle(size)))
			{WarnUser(WONT_UNDO,MemError());return;}
		BlockMove(*(*Undo.teh)->hText+Undo.startFrom,
																								*redoText,size);
		NoScrollTESetSelect(Undo.startFrom,Undo.goTil,Undo.teh);
		TEDelete(Undo.teh);
		Undo.goTil = Undo.startFrom;
	}
	
	if (Undo.text)
	{
		NoScrollTESetSelect(Undo.startFrom,Undo.startFrom,Undo.teh);
		TEInsert(LDRef(Undo.text),GetHandleSize(Undo.text),Undo.teh);
		DisposHandle(Undo.text);
		Undo.goTil = (*Undo.teh)->selEnd;
	}
	
	NoScrollTESetSelect(Undo.oldSelStart,Undo.oldSelEnd,Undo.teh);
	
	Undo.didUndo = !Undo.didUndo;
	Undo.text = redoText;
	Undo.oldSelStart = selStart;
	Undo.oldSelEnd = selEnd;
	
	win->isDirty = True;
	if (win->textChanged) (*win->textChanged)(win,Undo.teh,oldNl,CountTeLines(Undo.teh));
	ShowInsertion(win,InsertAny);
	Undo.didClick=True;
	
	SetUndoMenu();	
}

/************************************************************************
 * NukeUndo - remove the undo, if it pertains to this window
 ************************************************************************/
void NukeUndo(MyWindowPtr win)
{
	if (win && win==Undo.win)
	{
		if (Undo.text) ZapHandle(Undo.text);
		Undo.startFrom = Undo.goTil = 0;
		Undo.win = nil;
		SetUndoMenu();
	}
}

/************************************************************************
 * TEPrepareUndo - stash information for undo
 ************************************************************************/
void TEPrepareUndo(MyWindowPtr win,TEEnum what)
{
	long size;
	
	if (what==TECOPY || Undo.win==win && Undo.teh==WinTEH(win) && what==TEKEY &&
			Undo.wasWhat==TEKEY && !Undo.didClick)
		return;
	
	if (Undo.text) ZapHandle(Undo.text);	/* out with the old */
	Undo.startFrom = Undo.goTil = 0;
	
	Undo.win = win;
	Undo.teh = WinTEH(win);
	Undo.oldSelStart = (*Undo.teh)->selStart;
	Undo.oldSelEnd = (*Undo.teh)->selEnd;
	if (size = (*Undo.teh)->selEnd - (*Undo.teh)->selStart)
	{
		if (!(Undo.text = NuHandle(size)))
			{WarnUser(WONT_UNDO,MemError());return;}
		BlockMove(*(*Undo.teh)->hText+(*Undo.teh)->selStart,
																								*Undo.text,size);
	}
	Undo.startFrom = Undo.goTil = (*Undo.teh)->selStart;
	Undo.oldSelStart = (*Undo.teh)->selStart;
	Undo.oldSelEnd = (*Undo.teh)->selEnd;
	Undo.wasWhat = what;
	Undo.didUndo = Undo.didClick = False;
	SetUndoMenu();
}

/************************************************************************
 * SetUndoMenu - set the Undo menu properly
 ************************************************************************/
void SetUndoMenu(void)
{
	Str63 scratch;
	if (!Undo.text && Undo.startFrom==Undo.goTil || !Undo.teh)
	{
		DisableItem(GetMHandle(EDIT_MENU),EDIT_UNDO_ITEM);
		GetRString(scratch,UNDO);
	} 
	else
	{
		EnableItem(GetMHandle(EDIT_MENU),EDIT_UNDO_ITEM);
		GetRString(scratch,UNDO_STRN+Undo.wasWhat+(Undo.didUndo?TEENUM_LIMIT-1:0));
	}
	ASSERT(*scratch!=0);
	SetItem(GetMHandle(EDIT_MENU),EDIT_UNDO_ITEM,scratch);
}
