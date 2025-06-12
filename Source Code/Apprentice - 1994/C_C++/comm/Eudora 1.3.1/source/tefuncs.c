#define FILE_NUM 38
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment TEFuncs

	short WhichLine(TEHandle teh,int offset);
	short TEFakeLotsaTabs(TEHandle teh,short selStart,short selEnd);
	void TEWrap(TEHandle teh,Boolean wrap);
	void DoTEsWorkForIt(TEHandle teh,short oldStart,short oldEnd,short key,short mods);
	Boolean IsWordChar(Byte c);
  short TabCount(TEHandle teh,short selStart,short stopDistance);
	short TELineDelta(TEHandle teh, Rect *view, short line);
	void ZapTEScrap(void);
/**********************************************************************
 * CountTeLines - figure out how many lines are in a TE rec
 **********************************************************************/
int CountTeLines(TEHandle teh)
{
	int count = (*teh)->nLines;
	
	if ((*teh)->teLength && (*(*teh)->hText)[(*teh)->teLength-1]=='\n')
		count++;
	return(count?count:1);
}

/**********************************************************************
 * ShowWinInsertion - show the insertion point
 **********************************************************************/
ShowInsertion(MyWindowPtr win,short whichEnd)
{
	TEHandle teh=WinTEH(win);
	int dv;
	
	if (!teh) return;
	
	if (win->showInsert) (*win->showInsert)(win,whichEnd);
	else if (win->ste)
		STEShowInsert(win->ste);
	else if (teh && (dv=TEInsertDelta(teh,&win->contR,whichEnd)))
	{
		HFC(win);
		ScrollIt(win,0,dv);
	}
}

/************************************************************************
 * TEInsertDelta - figure out how far a TE must scroll veritcally to show
 * insertion point
 ************************************************************************/
short TEInsertDelta(TEHandle teh,Rect *view,short whichEnd)
{
	short endDv = TELineDelta(teh,view,WhichLine(teh,(*teh)->selEnd));
	short startDv = TELineDelta(teh,view,WhichLine(teh,(*teh)->selStart));
	
	if (whichEnd==InsertStart) return(startDv);
	else if (whichEnd==InsertEnd) return(endDv);
	else if (endDv*startDv <= 0) return(0);	/* part of it is already visible */
	return(startDv);												/* prefer the top */
}

/************************************************************************
 * TELineDelta - how far do we need to scroll to show a given line?
 ************************************************************************/
short TELineDelta(TEHandle teh, Rect *view, short line)
{
	short top,bottom,dv=0;
	short hi = (*teh)->lineHeight;
	
	top = (*teh)->destRect.top+line*hi;
	bottom = top + hi;
	if (bottom > view->bottom)
		dv = -((bottom - (view->bottom-view->top)/2 - 1)/hi);
	else if (top < view->top)
		dv = ((view->bottom-view->top)/2 - top - 1)/hi;
	return(dv);
}


/************************************************************************
 * HFC - Hide Foolish Caret
 ************************************************************************/
void HFC(MyWindowPtr win)
{
	if (win && win->qWindow.windowKind>=userKind && win->isActive)
	{
		TEHandle teh=WinTEH(win);
		
		if (teh && !win->hasSelection && (*teh)->caretState==-1)
		{
			TEIdle(teh);
			(*teh)->caretState = 255;
			ERASE_RECT(&(*teh)->selRect);
			INVAL_RECT(&(*teh)->selRect);
		}
	}
}

/************************************************************************
 * TESomething - perform one of the TE functions
 ************************************************************************/
Boolean TESomething(MyWindowPtr win,TEEnum what,short key,short mods)
{
	int oldNl;						/* number of lines before we did bad things */
	int newNl;						/* ditto, but after */
	TEHandle teh=WinTEH(win);
	int oldBytes, newStart, newEnd;
	MessType **messH = (MessType **)((WindowPeek)win)->refCon; /* maybe */
	short err,beginLine;
	Boolean dirty=False;
	short oldStart = (*teh)->selStart;
	short oldEnd = (*teh)->selEnd;
	short len;
	long junk;
	short whichEnd = InsertAny;
	
	if (what!=TECUT && what!=TECLEAR && what!=TECOPY && what!=TEPASTE &&
			what!=TEQPASTE && what!=TEKEY && what!=TEWRAP && what!=TEUNWRAP) return(False);
	oldNl = CountTeLines(teh);
	if (win->isActive)
		if (what!=TEKEY || DirtyKey(key))
			TEPrepareUndo(win,what);
		else
			Undo.didClick = True;
	switch (what)
	{
		case TECUT:
			dirty = (*teh)->selStart != (*teh)->selEnd;
			TECut(teh);
			if ((err=ZeroScrap()) || (err=TEToScrap()))
				WarnUser(CUT_FAILED,err);
				ZapTEScrap();
			break;
		case TECLEAR:
			dirty = (*teh)->selStart != (*teh)->selEnd;
			if (win->ro)
			{
				if (SumOf((MessHandle)win->qWindow.refCon)->state!=SENT)
				{
					TEDelete(teh);
					AttachSelect(teh);
				}
			}
			else
				TEDelete(teh);
			break;
		case TEWRAP:
		case TEUNWRAP:
			TEWrap(teh,what==TEWRAP);
			dirty = True;
			break;
		case TECOPY:
			TECopy(teh);
			if ((err=ZeroScrap()) || (err=TEToScrap()))
				WarnUser(COPY_FAILED,err);
			ZapTEScrap();
			break;
		case TEPASTE:
			len = GetScrap(nil,'TEXT',&junk);
			if (len>0 && (*teh)->teLength+len-oldEnd+oldStart>INFINITY)
				WarnUser(TE_TOO_MUCH,0);
			else if (len<=0 || (err=TEFromScrap()))
				WarnUser(PASTE_FAILED,len<0?len:err);
			else
			{
				TEPaste(teh); ZapTEScrap();
				newEnd = (*teh)->selEnd;
				if (FakeTabs && (win->qWindow.windowKind!=COMP_WIN||
					(*(*messH)->tocH)->sums[(*messH)->sumNum].flags&FLAG_TABS))
				{
					newEnd += TEFakeLotsaTabs(teh,oldStart,newEnd);
					NoScrollTESetSelect(newEnd,newEnd,teh);
				}
				dirty = True;
			}
			break;
		case TEQPASTE:
			len = GetScrap(nil,'TEXT',&junk);
			if (len>0 && (*teh)->teLength+len-oldEnd+oldStart>INFINITY)
				WarnUser(TE_TOO_MUCH,0);
			else if (len<=0 || (err=TEFromScrap()))
				WarnUser(PASTE_FAILED,err);
			else
			{
				newStart = (*teh)->selStart;
				TEPaste(teh); ZapTEScrap();
				newEnd = (*teh)->selEnd;
				oldBytes = (*teh)->teLength;
				beginLine = WhichLine(teh,newStart);
				if (newStart && (*(*teh)->hText)[newStart-1]!='\n') beginLine++;
				QuoteLines(messH,FindCompTx(messH,teh),
					beginLine,WhichLine(teh,newEnd-1),QUOTE_PREFIX);
				newEnd += (*teh)->teLength - oldBytes;
				if (FakeTabs && (win->qWindow.windowKind!=COMP_WIN||
					(*(*messH)->tocH)->sums[(*messH)->sumNum].flags&FLAG_TABS))
					newEnd += TEFakeLotsaTabs(teh,oldStart,newEnd);
				NoScrollTESetSelect(newEnd,newEnd,teh);
				dirty = true;
			}
			break;
		case TEKEY:
			if (key==enterChar) key = returnChar;
			if (key==delChar)
			{
				dirty = True;
				if ((*teh)->selStart!=(*teh)->selEnd)
					TEKey(backSpace,teh);
				else if ((*teh)->selStart<(*teh)->teLength)
				{
					Byte undoChar = (*(*teh)->hText)[(*teh)->selStart];
					TEDeactivate(teh);
					NoScrollTESetSelect((*teh)->selStart,(*teh)->selStart+1,teh);
					TEKey(backSpace,teh);
					TEActivate(teh);
					/*
					 * append the deleted char to the undo buffer
					 */
					if (!Undo.text) Undo.text = NewHandle(0);
					if (!Undo.text || PtrAndHand(&undoChar,Undo.text,1))
						WarnUser(WONT_UNDO,MemError());
				}
				else
					dirty = False;
			}
			else
			{
				if (key==backSpace &&
						(*teh)->selStart==(*teh)->selEnd && (*teh)->selEnd &&
						(*teh)->selStart == Undo.startFrom)
				{
					/*
					 * prepend undoChar to Undo buffer
					 */
					Byte undoChar = (*(*teh)->hText)[(*teh)->selStart-1];
					if (!Undo.text) Undo.text = NewHandle(0);
					if (!Undo.text || PtrAndHand(&undoChar,Undo.text,1))
						WarnUser(WONT_UNDO,MemError());
					BlockMove(*Undo.text,(*Undo.text)+1,GetHandleSize(Undo.text)-1);
					**Undo.text = undoChar;
					Undo.startFrom--;
				}
				
				/* make TE down arrow do the right thing */
				if (!(mods&optionKey) && key==downArrowChar) TESetSelect(oldEnd,oldEnd,teh);
				
				/*
				 * pass key to TE
				 */
				TEKey(key,teh);
				
				/* post-processing */
				dirty = DirtyKey(key);
				if (!dirty) Undo.didClick = True;
				if (key>=leftArrowChar && key<=downArrowChar)
				{
					if (mods&(shiftKey|optionKey))
						DoTEsWorkForIt(teh,oldStart,oldEnd,key,mods);
					whichEnd = (key==leftArrowChar||key==upArrowChar) ?
											InsertStart:InsertEnd;
				}
			}
			break;
	}
	if (win->isActive && Undo.teh && dirty) Undo.goTil = (*Undo.teh)->selEnd;
	if (dirty)
	{
		newNl = CountTeLines(teh);
		win->isDirty = True;
		if (win->textChanged) (*win->textChanged)(win,teh,oldNl,newNl);
	}
	ShowInsertion(win,whichEnd);
	return(True);
}

/************************************************************************
 * OffsetTE - move the terecs around in a window
 ************************************************************************/
void OffsetTE(MyWindowPtr win,TEHandle teh,short offset)
{
	MessType **messH = (MessType **)((WindowPeek)win)->refCon;
	short tx = FindCompTx(messH,teh);
	OFFSET_RECT(&(*teh)->destRect,0,offset);
	OFFSET_RECT(&(*teh)->selRect,0,offset);
	
	(*teh)->viewRect.top = MAX((*teh)->destRect.top,win->contR.top);
	(*teh)->viewRect.bottom = tx==HEAD_LIMIT-1 ? win->contR.bottom :
														 MIN((*teh)->destRect.bottom,win->contR.bottom);
}

/************************************************************************
 * WhichLine - which line is an offset on?
 ************************************************************************/
short WhichLine(TEHandle teh,int offset)
{
	short which;
	
	for (which=0;which<(*teh)->nLines;which++)
		if ((*teh)->lineStarts[which]>offset) break;
	return(which-1);
}

/************************************************************************
 * TEWordSelect - force the selection to consist of integral words
 ************************************************************************/
void TEWordSelect(TEHandle teh)
{
	UPtr start, end, selStart, selEnd;
	
	start = *(*teh)->hText;
	selStart = start + (*teh)->selStart;
	selEnd = start + (*teh)->selEnd;
	end = start + (*teh)->teLength;
	
	/*
	 * extend until the char before it is a space
	 */
	while (selStart>start && selStart[-1]!=' ') selStart--;
	
	/*
	 * extend until char after is a space
	 */
	while (selEnd < end && *selEnd!=' ') selEnd++;
	
	/*
	 * set it
	 */
	NoScrollTESetSelect(selStart-start,selEnd-start,teh);
}

/************************************************************************
 * TEFakeTab - fake a tab stop by inserting an appropriate number of spaces
 ************************************************************************/
short TEFakeTab(MyWindowPtr win,short stopDistance)
{
	short i,count;
	TEHandle teh=WinTEH(win);
	
	count = i = TabCount(teh,(*teh)->selStart,stopDistance);
	while (i--)
		TESomething(win,TEKEY,' ',0);
	return(count);
}


/************************************************************************
 * TabCount - how many spaces do we need here for a tab?
 ************************************************************************/
short TabCount(TEHandle teh,short selStart,short stopDistance)
{
	UPtr spot;
	short count;

	spot = *(*teh)->hText + selStart - 1;
	while (*spot != '\n' && spot>=*(*teh)->hText) spot--;
	count = selStart - (spot-*(*teh)->hText) - 1;
	count = stopDistance - count%stopDistance;
	return(count);
}

/************************************************************************
 * TEFakeLotsaTabs - take char of tabs during paste
 *  (but make the user pay <evil laugh>)
 ************************************************************************/
short TEFakeLotsaTabs(TEHandle teh,short selStart,short selEnd)
{
	short newEnd = selEnd;
	short stopDistance = GetRLong(TAB_DISTANCE);
	short count;
	Str31 spaces;
	long len = (*teh)->teLength;
	
	memset(spaces,' ',stopDistance);
	
	for (;selStart<newEnd;selStart++)
		if ((*(*teh)->hText)[selStart]==tabChar)
		{
			count = TabCount(teh,selStart,stopDistance);
			if (len+count-1>INFINITY) {WarnUser(TE_TOO_MUCH,0); break;}
			if (0>Munger((*teh)->hText,selStart,nil,1,spaces,count))
				{WarnUser(PASTE_FAILED,0);break;}
			newEnd += count-1;
			selStart += count-1;
			len += count-1;
		}
	if (newEnd!=selEnd)
	{
		len = GetHandleSize((*teh)->hText);
		(*teh)->teLength = len;
		INVAL_RECT(&(*teh)->viewRect);
		NoScrollTECalText(teh);
		NoScrollTESetSelect(newEnd,newEnd,teh);
		UpdateMyWindow((*teh)->inPort);
	}
	return(newEnd-selEnd);
}

/************************************************************************
 * InsertCommaIfNeedBe - put a comma after previous address, if one is
 * not there
 ************************************************************************/
void InsertCommaIfNeedBe(MyWindowPtr win)
{
	UPtr spot, end, begin;
	TEHandle teh=WinTEH(win);
	
	begin=*(*teh)->hText;
	end = begin + (*teh)->teLength;
	spot = begin + (*teh)->selStart - 1;
	
	while (spot>=begin && *spot==' ') spot--;
	if (spot>=begin && *spot!=',')
	{
		TESomething(win,TEKEY,',',0);
		TESomething(win,TEKEY,' ',0);
	}
}

/************************************************************************
 * TEFixup - fix up a terec with the right fonts, etc.
 ************************************************************************/
void TEFixup(TEHandle teh)
{
	if (teh)
	{
		(*teh)->txFont = FontID;
		(*teh)->txSize = FontSize;
		(*teh)->lineHeight = FontLead;
		(*teh)->fontAscent = FontAscent;
		
		TECalText(teh);
	}
}

/************************************************************************
 * TEMaxLine - fine the longest line in a TERec
 ************************************************************************/
short TEMaxLine(TEHandle teh)
{
	short *this,*last,max;
	
	max = 0;
	this=(*teh)->lineStarts;
	last = this+(*teh)->nLines;
	for (this++;this<last;this++) if (this[0]-this[-1]>max)
		max = this[0]-this[-1];
	if ((*teh)->nLines && (*teh)->teLength-last[-1]>max)
		max = (*teh)->teLength-last[-1];
	return(max);
}


/************************************************************************
 * TopOffset - what's the offset at the top of the TE?
 ************************************************************************/
short TopOffset(TEHandle teh)
{
	short linesOffTop = RoundDiv((*teh)->viewRect.top-(*teh)->destRect.top,(*teh)->lineHeight);
	return((*teh)->lineStarts[linesOffTop]);
}

/************************************************************************
 * MakeTopOffset - put the given offset at the top of the window, if possible
 ************************************************************************/
void MakeTopOffset(TEHandle teh,short offset)
{
	short linesOffTop = RoundDiv((*teh)->viewRect.top-(*teh)->destRect.top,(*teh)->lineHeight);
	short lineIs = WhichLine(teh,offset);
	
	TEPinScroll(0,(*teh)->lineHeight*(linesOffTop-lineIs),teh);
}

/************************************************************************
 * TEWrap - wrap the current selection
 ************************************************************************/
void TEWrap(TEHandle teh,Boolean wrap)
{
	void *oldSendWDS;
	short oldSize, newSize;
	if (WrapHandle = NuHandle(0))
	{
		oldSendWDS = SendWDS;
		SendWDS = WrapSendWDS;
		if (wrap)
			SendBodyLines((*teh)->hText,(*teh)->selEnd,(*teh)->selStart,True,True,nil,0,False);
		else
		{
			UnwrapSave(LDRef((*teh)->hText),(*teh)->selEnd,(*teh)->selStart,0);
			UL((*teh)->hText);
		}
		if (WrapHandle)
		{
			oldSize = (*teh)->selEnd-(*teh)->selStart;
			newSize = GetHandleSize(WrapHandle);
			Munger((*teh)->hText,(*teh)->selStart,nil,oldSize,LDRef(WrapHandle),newSize);
			(*teh)->teLength += GetHandleSize((*teh)->hText);
			(*teh)->selEnd += newSize - oldSize;
			TECalText(teh);
			InvalContent((*teh)->inPort);
		}
		SendWDS = oldSendWDS;
		ZapHandle(WrapHandle);	
	}
}

/************************************************************************
 * WrapSendWDS - accumulate "sent" text.
 ************************************************************************/
int WrapSendWDS(wdsEntry *theWDS)
{
	short err=0;

	if (WrapHandle)
	{
		for (;!err && theWDS->length;theWDS++)
		{
			if (theWDS->length == *NewLine && !strncmp(theWDS->ptr,NewLine+1,*NewLine))
				err = PtrAndHand("\n",WrapHandle,1);
			else
				err = PtrAndHand(theWDS->ptr,WrapHandle,theWDS->length);
		}
	}
	if (err) ZapHandle(WrapHandle);
	return(err);
}

/************************************************************************
 * TESelPara - extend the current selection to include whole paragraphs
 ************************************************************************/
void TESelPara(TEHandle teh)
{
  UPtr begin,end,selStart,selEnd;
	
	begin = *(*teh)->hText;
	end = begin+(*teh)->teLength;
	selStart = begin+(*teh)->selStart;
	selEnd = begin+MAX(1,(*teh)->selEnd);
	
	for (;selStart>begin;selStart--) {if (selStart[-1]=='\n') break;}
	for (;selEnd<end;selEnd++) if (selEnd[-1]=='\n') break;
	if (selStart!=begin+(*teh)->selStart || selEnd!=begin+(*teh)->selEnd)
		NoScrollTESetSelect(selStart-begin,selEnd-begin,teh);
}

/************************************************************************
 * DoTEsWorkForIt - handle standard text selection stuff.  TE is the only
 * place this can be done easily.  Hack, hack, kludge, kludge, O what
 * fun maccing is.  Hack, hack, kludge, kludge...
 ************************************************************************/
void DoTEsWorkForIt(TEHandle teh,short oldStart,short oldEnd,short key,short mods)
{
	Boolean shift = (mods&shiftKey)!=0;
	Boolean option = (mods&optionKey)!=0;
	Boolean wasWord;
	short newStart=(*teh)->selStart,newEnd=(*teh)->selEnd;
	
	switch(key)
	{
		case downArrowChar:
			/*
			 * TextEdit does something really stupid here; fix it up
			if (WhichLine(teh,oldStart)!=WhichLine(teh,oldEnd))
			{
				NoScrollTESetSelect(oldEnd,oldEnd,teh);
				TEKey(downArrowChar,teh);
				newEnd = (*teh)->selEnd;
			}
			 */
			/* Well, we've explained life to TextEdit now. */
			if (shift) newStart = oldStart;
			break;
			
		case upArrowChar:
			if (shift) newEnd = oldEnd;
			break;
			
		case leftArrowChar:
			if (option)
			{
				wasWord = IsWordChar((*(*teh)->hText)[oldStart-1]);
				newStart = oldStart;
				while (newStart && wasWord==IsWordChar((*(*teh)->hText)[newStart-1]))
					newStart--;
				if (newStart && !wasWord)
					while (newStart && IsWordChar((*(*teh)->hText)[newStart-1]))
						newStart--;
				if (!shift) newEnd = newStart;
			}
			if (shift) newEnd = oldEnd;
			break;
			
		case rightArrowChar:
			if (option)
			{
				wasWord = IsWordChar((*(*teh)->hText)[oldEnd]);
				newEnd = oldEnd;
				while (newEnd<(*teh)->teLength && wasWord==IsWordChar((*(*teh)->hText)[newEnd]))
					newEnd++;
				if (oldStart && !wasWord)
					while (newEnd<(*teh)->teLength && IsWordChar((*(*teh)->hText)[newEnd]))
						newEnd++;
				if (!shift) newStart = newEnd;
			}
			if (shift) newStart = oldStart;
			break;
	}
	
	if (newStart==newEnd)
	{
		TEDeactivate(teh);
		(*teh)->clikStuff = key==rightArrowChar ? 0 : 255;
	}
	((MyWindowPtr)(*teh)->inPort)->hasSelection = newStart!=newEnd;
	NoScrollTESetSelect(newStart,newEnd,teh);
	TEActivate(teh);
}

/************************************************************************
 * IsWordChar - is the selected character part of a word?
 ************************************************************************/
Boolean IsWordChar(Byte c)
{
	return(c>' ');	/* this isn't internationally correct, but it's
													 what TE does.  Sort of. */
}

/************************************************************************
 * NoScrollTECalText - TECalText, but without the autoscroll
 ************************************************************************/
void NoScrollTECalText(TEHandle teh)
{
	MyWindowPtr win = (*teh)->inPort;
	TEAutoView(False,teh);
	TECalText(teh);
	if (win->qWindow.windowKind != COMP_WIN) TEAutoView(True,teh);
}

/************************************************************************
 * NoScrollTESetSelect - TESetSelect, but without the autoscroll
 ************************************************************************/
void NoScrollTESetSelect(short start,short end,TEHandle teh)
{
	MyWindowPtr win = (*teh)->inPort;
	TEAutoView(False,teh);
	TESetSelect(start,end,teh);
	if (win->qWindow.windowKind != COMP_WIN) TEAutoView(True,teh);
}

/************************************************************************
 * TELineOf - figure out which line a selection falls on
 ************************************************************************/
short TELineOf(short position, TEHandle teh)
{
	short line;
	
	for (line=0;line<(*teh)->nLines;line++)
		if ((*teh)->lineStarts[line] > position) break;
	if (line==(*teh)->nLines) line--;
	
	return(line);
}

/************************************************************************
 * ZapTEScrap - get rid of the horrid monster te scrap
 ************************************************************************/
void ZapTEScrap(void)
{
	Handle h = TEScrapHandle();
	if (h)
	{
		SetHandleSize(h,0);
		TESetScrapLen(0);
	}
}
	