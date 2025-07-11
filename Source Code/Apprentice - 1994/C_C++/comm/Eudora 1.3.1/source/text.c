#define FILE_NUM 39
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Text

	Boolean TextClose(MyWindowPtr win);
	MyWindowPtr FindText(short vRef,UPtr name);
	Boolean TextSave(MyWindowPtr win);
	Boolean TextSaveAs(MyWindowPtr win);
	Boolean TextMenu(MyWindowPtr win, int menu, int item, short modifiers);
	Boolean TextPosition(Boolean save,MyWindowPtr win);
	void TextChanged(MyWindowPtr win);

/************************************************************************
 * OpenText - open a text window
 ************************************************************************/
short OpenText(short vRef,UPtr name,MyWindowPtr win,Boolean showIt,UPtr alias)
{
	long bytes;
	UHandle textH=nil;
	short refN=0;
	short err=noErr;
	TextDHandle th=nil;
	MyWindowPtr oldWin;
	
	if (vRef)
	{
		if (oldWin=FindText(vRef,name))
		{
			ShowMyWindow(oldWin);
			SelectWindow(oldWin);
			return(noErr);
		}
			
		if (err=FSHOpen(name,vRef,0,&refN,fsRdPerm))
			FileSystemError(TEXT_READ,name,err);
		else if (err=GetEOF(refN,&bytes))
			FileSystemError(TEXT_READ,name,err);
		else if (bytes>INFINITY)
		{
			WarnUser(TEXT_TOO_BIG,bytes);
			err=INFINITY;
		}
	}
	else
		bytes = 0;
	if (err==noErr)
	{
		if (!(win = GetNewMyWindow(MESSAGE_WIND,win,InFront,False,False)))
			err=1;
		else if (!(th=NewZH(TextDesc)))
			WarnUser(MEM_ERR,err=MemError());
		else
		{
			((WindowPeek)win)->windowKind = TEXT_WIN;
			if (!(textH=NuHandle(bytes)))
				WarnUser(MEM_ERR,err=MemError());
			else if (vRef && (err=FSRead(refN,&bytes,LDRef(textH))))
				FileSystemError(TEXT_READ,name,err);
			else
			{
				UL(textH);
				FSClose(refN);
				refN = 0;
				if ((win->ste=NewSTE(win,nil,False,True,True))==nil)
					WarnUser(MESS_TE,err=MemError());
				else
				{
					(*(STEHandle)win->ste)->growBox = True;
					(*(STEHandle)win->ste)->dontFrame = True;
					STEInstallText(textH,win->ste); textH = nil;
					win->didResize = TextDidResize;
					win->zoomSize = CompZoomSize;
					win->close = TextClose;
					win->menu = TextMenu;
					SetWTitle(win,alias?alias:name);
					(*th)->vRef = vRef;
					PCopy((*th)->fileName,name);
					win->qWindow.refCon = (long)th; th=nil;
					win->position = alias ? PositionPrefsTitle : TextPosition;
					win->textChanged = TextChanged;
					if (showIt)
						ShowMyWindow(win);
					UpdateMyWindow(win);
					TEActivate(WinTEH(win));
				}
			}
		}
	}
	if (refN) FSClose(refN);
	if (textH) DisposHandle(textH);
	if (th) DisposHandle(th);
	return(err);
}

/************************************************************************
 * FindText - find a window containing the contents of a text file
 ************************************************************************/
MyWindowPtr FindText(short vRef,UPtr name)
{
	MyWindowPtr win;
	TextDHandle th;
	
	for (win=FrontWindow();win;win=win->qWindow.nextWindow)
		if (IsMyWindow(win) && win->qWindow.windowKind==TEXT_WIN)
		{
			th = (TextDHandle) win->qWindow.refCon;
			if ((*th)->vRef==vRef&& EqualString(name,LDRef(th)->fileName,False,True))
			{
				UL(th);
				return(win);
			}
			else
				UL(th);
		}
	return(nil);
}
	
/**********************************************************************
 * TextDidResize - handle the resizing of a message window
 **********************************************************************/
void TextDidResize(MyWindowPtr win,Rect *oldContR)
{
#pragma unused(oldContR)
	Rect r = win->contR;
	r.top--;
	r.right++;
	ResizeSTE(win->ste,&r);
}

#pragma segment Main
/************************************************************************
 * TextClose - close a text window
 ************************************************************************/
Boolean TextClose(MyWindowPtr win)
{
	short which;
	if (win->isDirty)
	{
		which = WannaSave(win);
		if (which==WANNA_SAVE_CANCEL || which==CANCEL_ITEM) return(False);
		if (which==WANNA_SAVE_SAVE && !TextSave(win)) return(False);
	}
	if (win->ste) STEDispose(win->ste);
	return(True);
}
#pragma segment Text

/************************************************************************
 * TextSave - save the contents of a text window
 ************************************************************************/
Boolean TextSave(MyWindowPtr win)
{
	TextDHandle th=(TextDHandle)win->qWindow.refCon;
	TEHandle teh = WinTEH(win);
	short refN,err;
	long bytes;
	
	if (!(*th)->vRef) return(TextSaveAs(win));
	LDRef(th);
	if (err=FSOpen((*th)->fileName,(*th)->vRef,&refN))
		FileSystemError(TEXT_WRITE,(*th)->fileName,err);
	else
	{
		bytes = (*teh)->teLength;
		if (err=FSWrite(refN,&bytes,LDRef((*teh)->hText)))
			FileSystemError(TEXT_WRITE,(*th)->fileName,err);
		else if (err=SetEOF(refN,bytes))
			FileSystemError(TEXT_WRITE,(*th)->fileName,err);
		UL((*teh)->hText);
		FSClose(refN);
		win->isDirty = err!=noErr;
	}
	UL(th);
	return(err==noErr);
}

/************************************************************************
 * TextChanged - notice when text has changed
 ************************************************************************/
void TextChanged(MyWindowPtr win)
{
	STETextChanged(win->ste);
}

/************************************************************************
 * TextSaveAs - save the contents of a text window, giving a new name
 ************************************************************************/
Boolean TextSaveAs(MyWindowPtr win)
{
	short vRef,refN,err;
	Str31 name;
	Str31 scratch;
	long creator,bytes;
	TextDHandle th;
	TEHandle teh = WinTEH(win);
	
	/*
	 * standard file stuff
	 */
	GetPref(scratch,PREF_CREATOR);
	if (*scratch!=4) GetRString(scratch,TEXT_CREATOR);
	BlockMove(scratch+1,&creator,4);
	th = (TextDHandle)win->qWindow.refCon;
	PCopy(name,(*th)->fileName);
	if (err=SFPutOpen(name,&vRef,creator,'TEXT',&refN,DlgFilter,SFPUTFILE_ID))
		return(False);
	bytes = (*teh)->teLength;
	if (err=FSWrite(refN,&bytes,LDRef((*teh)->hText)))
		FileSystemError(TEXT_WRITE,name,err);
	else if (err=SetEOF(refN,bytes))
		FileSystemError(TEXT_WRITE,name,err);
	else
	{
		(*th)->vRef = vRef;
		PCopy((*th)->fileName,name);
		SetWTitle(win,name);
		win->isDirty = False;
	}
	UL((*teh)->hText);
	FSClose(refN);
	return(err==noErr);
}

/************************************************************************
 * TextMenu - handle menu choices peculiar to Text windows
 ************************************************************************/
Boolean TextMenu(MyWindowPtr win, int menu, int item, short modifiers)
{
#pragma unused(modifiers)
	switch (menu)
	{
		case FILE_MENU:
			switch (item)
			{
				case FILE_SAVE_ITEM:
					(void) TextSave(win);
					break;
				case FILE_SAVE_AS_ITEM:
					TextSaveAs(win);
					break;
				case FILE_PRINT_ITEM:
				case FILE_PRINT_SELECT_ITEM:
					PrintOneMessage(win,item==FILE_PRINT_SELECT_ITEM);
					break;
				default:
					return(False);
			}
			break;
		case EDIT_MENU:
			if (item==TEWRAP && modifiers&optionKey) item=TEUNWRAP;
			return(TESomething(win,item,0,modifiers));
			break;
		default:
			return(False);
	}
	return(True);
}

/************************************************************************
 * TextPosition - remember the position of a text window
 ************************************************************************/
Boolean TextPosition(Boolean save,MyWindowPtr win)
{
	Rect r;
	Boolean zoomed;
	TextDHandle textH = (TextDPtr) win->qWindow.refCon;
	Str31 name;
	
	PCopy(name,(*textH)->fileName);
	if (!*name) return(False);
	if (save)
	{
		utl_SaveWindowPos(win,&r,&zoomed);
		SavePosFork((*textH)->vRef,0,name,&r,zoomed);
	}
	else
	{
		if (!RestorePosFork((*textH)->vRef,0,name,&r,&zoomed))
			return(False);
		utl_RestoreWindowPos(win,&r,zoomed,1,FigureZoom,DefPosition);
	}
	return(True);
}

