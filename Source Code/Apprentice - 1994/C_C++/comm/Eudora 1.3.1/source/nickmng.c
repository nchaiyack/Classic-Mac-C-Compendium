#define FILE_NUM 2
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * routines to manage the nickname list
 ************************************************************************/
#pragma load EUDORA_LOAD
#pragma segment NickMng

/************************************************************************
 * The aliases list has the following structure:
 * <length byte>name-of-alias<2 length bytes>expansion-of-alias...
 * The aliases file contains lines of the form:
 * "alias" name-of-alias expansion of alias<newline>
 * Newlines may be escaped with "\".
 * Lines not beginning with "alias" will be ignored
 ************************************************************************/

/************************************************************************
 * private functions
 ************************************************************************/
typedef struct
{
	Str31 name;
	long offset;
} AliasSortType, *AliasSortPtr, **AliasSortHandle;

	int ReadAliases(UHandle intoH,short cmdName);
	Boolean NeatenLine(UPtr line, long *len);
	short GatherCompAddresses(MyWindowPtr win,Handle biglist);
	int BuildAliasInversion(AliasSortHandle *tableHP, long *count);
	void AliasSortSwap(AliasSortPtr as1, AliasSortPtr as2);
#define issep(c) (isspace(c) || (c)==',')

/************************************************************************
 * RegenerateAliases - make sure the alias list is in memory
 ************************************************************************/
int RegenerateAliases(UHandle *intoH,short cmdName)
{
	int err=0;
	
	if (*intoH==nil)
	{
		*intoH=NuHandle(0L);
		if (!*intoH) return(WarnUser(ALLO_ALIAS,MemError()));
		err = ReadAliases(*intoH,cmdName);
	}
	else
	{
		HNoPurge(*intoH);
		if (!**intoH)
		{
			ReallocHandle(*intoH,0);
			if (!*intoH) return(WarnUser(ALLO_ALIAS,MemError()));
			HNoPurge(*intoH);
			err=ReadAliases(*intoH,cmdName);
		}
	}
	if (err) ZapHandle(*intoH);
#ifdef DEBUG
	else ASSERT(GetHandleSize(*intoH));
#endif
	return(err);
}

/************************************************************************
 * ReadAliases - read the list of aliases
 ************************************************************************/
int ReadAliases(UHandle intoH,short cmdName)
{
	Str31 aliasFile;
	Str31 aliasCmd;
	int err;
	Str255 line;
	short expBytes;
	UPtr spot,anchor,expSpot;
	short type;
	Boolean exLine=False;
	long expOffset=0;
	long len;
	
	GetRString(aliasFile,ALIAS_FILE);
	GetRString(aliasCmd,cmdName);
	if (err=OpenLine(MyVRef,MyDirId,aliasFile))
		return(err==fnfErr ? noErr : FileSystemError(OPEN_ALIAS,aliasFile,err));
	while ((type=GetLine(line,sizeof(line)))>0)
	{
		len = strlen(line);
		if (exLine || type==LINE_MIDDLE)
		{
			if (expOffset)
			{
				len = strlen(line);
				exLine = NeatenLine(line,&len);
				if (exLine && !issep(*line))
				{
					if (err=PtrAndHand(" ",intoH,1)) break;
					expBytes++;
				}
				if (err=PtrAndHand(line,intoH,len)) break;
				expBytes += len;
			}
		}
		else
		{
			exLine = NeatenLine(line,&len);
			if (expOffset)
			{
				(*intoH)[expOffset] = expBytes >> 8;
				(*intoH)[expOffset+1] = expBytes & 0xff;
				expOffset = expBytes = 0;
			}
			if (len > *aliasCmd && !striscmp(line,aliasCmd+1))
			{
				for (spot=line+*aliasCmd+1;issep(*spot);spot++);				/* skip ws */
				for (anchor=spot;*spot && !issep(*spot);spot++);				/* find name */
				for (expSpot=spot;issep(*expSpot);expSpot++); 					/* skip ws */
				if (*expSpot) 																					/* valid */
				{
					anchor[-1] = MIN(MAX_NICKNAME,spot-anchor);
					if (err=PtrAndHand(anchor-1,intoH,anchor[-1]+3)) break;
					expOffset = GetHandleSize(intoH)-2;
					if (err=PtrAndHand(expSpot,intoH,len-(expSpot-line))) break;
					expBytes = len-(expSpot-line);
				}
			}
		}
	}
	CloseLine();
	if (type	|| err)
		return(err ? WarnUser(ALLO_ALIAS,err) :
											FileSystemError(READ_ALIAS,aliasFile,type));
	if (expOffset)
	{
		(*intoH)[expOffset] = expBytes >> 8;
		(*intoH)[expOffset+1] = expBytes & 0xff;
		expOffset = 0;
	}
	return(noErr);
} 

/************************************************************************
 * NeatenLine - strip the newline from a line; if it was escaped, strip
 * the backslash, and return True
 ************************************************************************/
Boolean NeatenLine(UPtr line, long *len)
{
	if (line[*len-1]=='\n') line[--*len] = 0;
	if (line[*len-1]=='\\')
	{
		line[--*len] = 0;
		return(True);
	}
	return(False);
}


/************************************************************************
 * CountAliasTotal - how long is an alias (altogether)
 ************************************************************************/
long CountAliasTotal(UHandle aliases,long offset)
{
	return(CountAliasAlias(aliases,offset)+1+CountAliasExpansion(aliases,offset)+2);
}

/************************************************************************
 * CountAliasAlias - how long is the alias part of an alias (length byte
 * not counted)
 ************************************************************************/
long CountAliasAlias(UHandle aliases,long offset)
{
	return ((unsigned) (*aliases)[offset]);
}

/************************************************************************
 * CountAliasExpansion - how long is an alias expansion (length word
 * not counted)
 ************************************************************************/
long CountAliasExpansion(UHandle aliases,long offset)
{
	long base=offset+CountAliasAlias(aliases,offset)+1;
	return (256*(unsigned)(*aliases)[base] + (unsigned)(*aliases)[base+1]);
}


/************************************************************************
 * AddAlias - add an alias
 ************************************************************************/
short AddAlias(UHandle aliases,UPtr name,Handle addresses)
{
	long addrSize = addresses ? GetHandleSize(addresses) : 0;
	long size = GetHandleSize(aliases);

	SetHandleBig(aliases,size+3+*name+addrSize);
	if (MemError()) return(WarnUser(MEM_ERR,MemError()));
	PCopy((*aliases)+size,name);
	(*aliases)[size+*name+1] = addrSize>>8 & 0xff;
	(*aliases)[size+*name+2] = addrSize & 0xff;
	if (addrSize) BlockMove(*addresses,(*aliases+size+*name+3),addrSize);
	return(0);
}

/************************************************************************
 * RemoveAlias - remove the named alias
 ************************************************************************/
void RemoveAlias(UHandle aliases,UPtr name)
{
	long oldSize=GetHandleSize(aliases);
	long offset = FindAliasFor(aliases,name+1,*name);
	long length;
	if (offset >= 0)
	{
		length = CountAliasTotal(aliases,offset);
		BlockMove((*aliases)+offset+length,(*aliases)+offset,
																									oldSize-offset-length);
		SetHandleBig(aliases,oldSize-length);
	}
}

/************************************************************************
 * ReplaceAlias - replace one nickname definition with another
 ************************************************************************/
short ReplaceAlias(UHandle aliases,UPtr oldName,UPtr newName,Handle text)
{
	short oldLength;
	short newLength;
	short eLen;
	long size, newSize;
	short maxAlias=GetRLong(BUFFER_SIZE)-40;
	long offset= -1;
	
	if (oldName && *oldName) offset = FindAliasFor(aliases,oldName+1,*oldName);
	if (offset<0) return(AddAlias(aliases,newName,text));
	
	/*
	 * grab the expansion
	 */
	eLen = GetHandleSize(text);
	
	/*
	 * figure the sizes
	 */
	oldLength = CountAliasTotal(aliases,offset);
	newLength = *newName + eLen + 3;
	if (newLength > maxAlias)
	{
		WarnUser(ALIAS_TOO_LONG, maxAlias);
		eLen -= newLength-maxAlias;
		newLength = maxAlias;
	}
			
	/*
	 * do we need to expand?
	 */
	if (newLength > oldLength)
	{
		size = GetHandleSize(aliases);
		newSize = size+newLength-oldLength;
		SetHandleBig(aliases,newSize);
		if (MemError()) return(WarnUser(COULDNT_MOD_ALIAS,MemError()));
		BlockMove((*aliases)+offset+oldLength,(*aliases)+offset+newLength,
											size-offset-oldLength);
	}

	/*
	 * put the bytes in
	 */
	PCopy((*aliases)+offset,newName);
	(*aliases)[offset+*newName+1] = (eLen>>8) & 0xff;
	(*aliases)[offset+*newName+2] = eLen & 0xff;
	BlockMove(*text,(*aliases)+offset+*newName+3,eLen);
	
	/*
	 * do we get to contract?
	 */
	if (newLength<oldLength)
	{
		size = GetHandleSize(aliases);
		newSize = size+newLength-oldLength;
		BlockMove((*aliases)+offset+oldLength,(*aliases)+offset+newLength,
											size-offset-oldLength);
		SetHandleBig(aliases,newSize);
	}
	return(0);
}

/************************************************************************
 * MakeCompNick - make a nickname out of a comp window
 ************************************************************************/
void MakeCompNick(MyWindowPtr win)
{
	Handle biglist;
	
	biglist = NuHandle(0);
	(void) GatherCompAddresses(win,biglist);

	/*
	 * and make the nickname...
	 */
	if (**biglist) NewNick(biglist);
	else WarnUser(NO_ADDRESSES,0);
	
	DisposeHandle(biglist);
}

/************************************************************************
 * GatherCompAddresses - gather the addresses from a window
 ************************************************************************/
short GatherCompAddresses(MyWindowPtr win,Handle biglist)
{
	Handle littlelist;
	MessHandle messH;
	static short heads[] = {TO_HEAD-1,CC_HEAD-1,BCC_HEAD-1};
	short err=0;
	short h;
	
	/*
	 * I vaant to suck your addresses...
	 */
	messH = Win2MessH(win);
	for (h=0;h<sizeof(heads)/sizeof(short);h++)
	{
		littlelist = SuckAddresses((*(*messH)->txes[heads[h]])->hText,
															 (*(*messH)->txes[heads[h]])->teLength,True);
		if (littlelist)
		{
		  if (**littlelist)
			{
				long size=GetHandleSize(biglist);
				if (size) SetHandleBig(biglist,size-1);	/* strip final terminator */
				HLock(littlelist);
				if (err=HandAndHand(littlelist,biglist)) break;
			}
			DisposeHandle(littlelist);
		}
	}
	return(err);
}

/************************************************************************
 * MakeMessNick - make a nickname out of a message window
 ************************************************************************/
void MakeMessNick(MyWindowPtr win,short modifiers)
{
	TOCHandle out=GetOutTOC();
	if (out)
	{
		Boolean wasDirty = (*out)->win->isDirty;
		win = DoReplyMessage(win,modifiers | shiftKey,0,False);
		if (!win) return;
		MakeCompNick(win);
		CloseMyWindow(win);
		(*out)->win->isDirty = wasDirty;
	}
}

/************************************************************************
 * MakeMboxNick - make a nickname out of the selected messages in an mbox
 ************************************************************************/
void MakeMboxNick(MyWindowPtr win,short modifiers)
{
	Handle addresses=NuHandle(0);
	TOCHandle tocH = (TOCHandle)win->qWindow.refCon;
	MyWindowPtr messWin,compWin;
  short sumNum;
	short err = 0;
	Boolean isOut = (*tocH)->which==OUT;
	
	if (!addresses) return;
	for (sumNum=0;!err && sumNum<(*tocH)->count;sumNum++)
	{
		if ((*tocH)->sums[sumNum].selected && ((*tocH)->sums[sumNum].messH || GetAMessage(tocH,sumNum,nil,False)))
		{
			messWin = (*(MessType **)(*tocH)->sums[sumNum].messH)->win;
			compWin = isOut ? messWin : DoReplyMessage(messWin,modifiers,0,False);
			if (compWin)
			{
				err=GatherCompAddresses(compWin,addresses);
				if (compWin != messWin) CloseMyWindow(compWin);
			}
			if (!messWin->qWindow.visible) CloseMyWindow(messWin);
		}
	}
	
	if (!err)
	{
		if (**addresses) NewNick(addresses);
		else WarnUser(NO_ADDRESSES,0);
	}
	
	DisposeHandle(addresses);
}

/************************************************************************
 * MakeCboxNick - make a nickname out of the selected messages in Out
 ************************************************************************/
void MakeCboxNick(MyWindowPtr win)
{
	Handle addresses=NuHandle(0);
	TOCHandle tocH = (TOCHandle)win->qWindow.refCon;
	MyWindowPtr compWin;
  short sumNum;
	short err = 0;
	
	if (!addresses) return;
	for (sumNum=0;!err && sumNum<(*tocH)->count;sumNum++)
	{
		if ((*tocH)->sums[sumNum].selected && 
		    ((*tocH)->sums[sumNum].messH || GetAMessage(tocH,sumNum,nil,False)))
		{
			compWin = (*(MessType **)(*tocH)->sums[sumNum].messH)->win;
			if (compWin)
			{
				err=GatherCompAddresses(compWin,addresses);
				if (!compWin->qWindow.visible) CloseMyWindow(compWin);
			}
		}
	}
	
	if (!err)
	{
		if (**addresses) NewNick(addresses);
		else WarnUser(NO_ADDRESSES,0);
	}
	
	DisposeHandle(addresses);
}

/************************************************************************
 * FlattenListWith - make an address list one to a line
 ************************************************************************/
void FlattenListWith(Handle h,Byte c)
{
  UPtr from, to;
	
	from = to = *h;
	while (*from)
	{
	  while (*++from) *to++ = *from;	/* skip length byte, copy string */
		from++;													/* skip terminator */
		*to++ = c;										/* and add a separator */
	}
	if (to > *h) to--;
	SetHandleBig(h,to-*h);
}

/************************************************************************
 * SaveAliases - save the edited aliases (if necessary)
 * returns False if the operation failed
 ************************************************************************/
Boolean SaveAliases(void)
{
	Str31 aliasFile,tmpFile;
	Str31 aliasCmd;
	Str63 scratch;
	int err;
	long max, offset;
	long eLen;
	long bytes;
	short refN=0;
	int i, count;
	AliasSortHandle invers=nil;
	
	AliasWinGonnaSave();
	GetRString(aliasFile,ALIAS_FILE);
	PCopy(tmpFile,aliasFile);
	GetRString(scratch,TEMP_SUFFIX);
	PCat(tmpFile,scratch);
	if (!Aliases || !*Aliases) {WarnUser(SAVE_ALIAS,0);goto done;}
	if (RegenerateAliases(&Notes,NOTE_CMD)) goto done;
	if (err=MakeResFile(tmpFile,MyVRef,MyDirId,CREATOR,MAILBOX_TYPE))
		{FileSystemError(SAVE_ALIAS,tmpFile,err); goto done;}
	if (err=FSHOpen(tmpFile,MyVRef,MyDirId,&refN,fsRdWrPerm))
		{FileSystemError(OPEN_ALIAS,tmpFile,err); goto done;}
		
	max = GetHandleSize(Aliases);
	if (BuildAliasInversion(&invers,&count)) count=max; /* hack */

	GetRString(aliasCmd,ALIAS_CMD);
	PCatC(aliasCmd,' ');
	LDRef(Aliases);
	for (i=0;i<count;i++)
	{
		if (invers) offset = (*invers)[i].offset;
		if (offset>max) break; 										/* hack */
		if (!(err = FSWriteP(refN,aliasCmd)))
		{
			PCopy(scratch,*Aliases+offset);
			PCatC(scratch,' ');
			if (!(err=FSWriteP(refN,scratch)))
			{
				bytes = CountAliasExpansion(Aliases,offset);
				if (!(err = FSWrite(refN,&bytes,*Aliases+offset+*scratch+2)))
				{
					bytes = 1;
					err = FSWrite(refN,&bytes,"\n");
				}
			}
		}
	}
	if (invers) DisposHandle(invers);
	UL(Aliases);
	
	GetRString(aliasCmd,NOTE_CMD);
	PCatC(aliasCmd,' ');
	LDRef(Notes);
	eLen = GetHandleSize(Notes);
	for (offset=0;offset<eLen;offset+=CountAliasTotal(Notes,offset))
	{
		if (!(err = FSWriteP(refN,aliasCmd)))
		{
			PCopy(scratch,*Notes+offset);
			PCatC(scratch,' ');
			if (!(err=FSWriteP(refN,scratch)))
			{
				bytes = CountAliasExpansion(Notes,offset);
				if (!(err = FSWrite(refN,&bytes,*Notes+offset+*scratch+2)))
				{
					bytes = 1;
					err = FSWrite(refN,&bytes,"\n");
				}
			}
		}
		if (err) {FileSystemError(SAVE_ALIAS,aliasFile,err); goto done;}
	}
	GetFPos(refN,&bytes);
	SetEOF(refN,bytes);
	AliasWinDidSave();

	/* do the deed */
	if (err=HDelete(MyVRef,MyDirId,aliasFile))
		{FileSystemError(SAVE_ALIAS,aliasFile,err); goto done;}
	if (err=HRename(MyVRef,MyDirId,tmpFile,aliasFile))
		FileSystemError(SAVE_ALIAS,aliasFile,err);

done:
	if (Notes) UL(Notes);
	if (Aliases) UL(Aliases);
	if (refN) FSClose(refN);
	return(err==noErr);
}

/************************************************************************
 * BuildAliasInversion - create a sort table for the aliases
 ************************************************************************/
int BuildAliasInversion(AliasSortHandle *tableHP, long *count)
{
	AliasSortHandle asH=nil;
	int n=0;
	int err=0;
	long offset,max;
	AliasSortType as;
	int aLen;
	
	if (!(asH=NuHandle(0)))
		err = MemError();
	else
	{
		max = GetHandleSize(Aliases);
		for (offset = 0; offset<max; offset += CountAliasTotal(Aliases,offset))
		{
			n++;
			aLen = CountAliasAlias(Aliases,offset);
			if (aLen >= sizeof(as.name)) aLen = sizeof(as.name)-1;
			BlockMove((*Aliases)+offset,as.name,aLen+1);
			as.offset = offset;
			if (err=PtrAndHand(&as,asH,sizeof(as))) break;
		}
	}
	
	if (!err)
	{
		QuickSort(LDRef(asH),sizeof(as),0,n-1,StrCompar,AliasSortSwap);
		UL(asH);
		*tableHP = asH;
		*count = n;
	}
	else
		if (asH) DisposHandle(asH);
	
	return(err);
}

/************************************************************************
 * AliasSortSwap - swap two AliasSortType's
 ************************************************************************/
void AliasSortSwap(AliasSortPtr as1, AliasSortPtr as2)
{
	AliasSortType asTemp;
	
	asTemp = *as1;
	*as1 = *as2;
	*as2 = asTemp;
}
