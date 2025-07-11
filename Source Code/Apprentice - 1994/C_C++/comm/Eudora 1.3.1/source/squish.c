#define FILE_NUM 44
#pragma load EUDORA_LOAD
#pragma segment Squish
	Boolean NeedsCompaction(long dirId, UPtr name);
/************************************************************************
 * DoCompact - compact all mailboxes
 ************************************************************************/
void DoCompact(long dirId,HFileInfo *hfi,short suffixLen)
{
	typedef struct
		{Str31 name; Boolean isDir; long dirId;} NameType, *NamePtr, **NameHandle;
	NameType thisOne;
	NameHandle names;
	short count=0,item;

	/*
	 * close all windows
	 */
	if (dirId==MyDirId && !CloseAll()) return;
	
	/*
	 * get started
	 */
	if ((names=NuHandle(0L))==nil)
		DieWithError(ALLO_MBOX_LIST,MemError());
	
	/*
	 * read names of mailbox files
	 */
	hfi->ioNamePtr = thisOne.name;
	hfi->ioFDirIndex = 0;
	for (thisOne.dirId=dirId;!DirIterate(MyVRef,dirId,hfi);thisOne.dirId=dirId)
	{
		if (thisOne.isDir = 0!=(hfi->ioFlAttrib&0x10))
			thisOne.dirId = hfi->ioDirID;
		else
		{
			if (hfi->ioFlFndrInfo.fdType!=TOC_TYPE) continue;
			*thisOne.name -= suffixLen;					/* remove suffix */
			thisOne.name[*thisOne.name+1] = 0;	/* null-terminate */
		}
		PtrAndHand (&thisOne,names,sizeof(thisOne));
		if (MemError()) DieWithError(ALLO_MBOX_LIST,MemError());
		count++;
		loop:;
	}
	
	if (dirId==MyDirId) OpenProgress();
	for (item=0;item<count;item++)
	{
		GiveTime();
		thisOne = (*names)[item];
		Progress(NoChange,thisOne.name);
		if (thisOne.isDir)
			DoCompact(thisOne.dirId,hfi,suffixLen);
		else if (NeedsCompaction(thisOne.dirId,thisOne.name))
			CompactMailbox(thisOne.dirId,thisOne.name);
	}
	if (dirId==MyDirId) CloseProgress();
	DisposHandle(names);
}

/************************************************************************
 * NeedsCompaction - see if a toc and a mailbox are in perfect agreement
 ************************************************************************/
Boolean NeedsCompaction(long dirId, UPtr name)
{
	TOCHandle tocH;
	short mNum;
	long offset = 0;
	Boolean need=False;
	long eof;
	HFileInfo info;
		
	if (tocH=TOCByName(dirId,name))
	{
		for (mNum=0; mNum<(*tocH)->count; mNum++)
			if ((*tocH)->sums[mNum].offset != offset)
			{
				need = True;
				break;
			}
			else
				offset += (*tocH)->sums[mNum].length;
		
		if (!need)
		{
			if (!HGetFileInfo((*tocH)->vRef,dirId,name,&info) &&
					info.ioFlLgLen!=offset && !BoxFOpen(tocH))
			{
				if (!GetEOF((*tocH)->refN,&eof))
				{
					SetEOF((*tocH)->refN,offset);
					(*tocH)->dirty = True;
				}
				BoxFClose(tocH);
			}
		}
		
		if (!need && !(*tocH)->win->qWindow.visible) CloseMyWindow((*tocH)->win);
	}
	return(need);
}

/************************************************************************
 * CompactMailbox - rewrite a mailbox to agree with a table of contents
 ************************************************************************/
void CompactMailbox(long dirId, UPtr name)
{
	TOCHandle tocH=nil;
	short oldRefN=0, newRefN=0;
	Str31 tmpName;
	Str31 tmpSuffix;
	long size=0;
	short mNum;
	int err=0;
	int rnErr=0;
	FInfo info;
	short newVRef;
	long newDirId;
	HFileInfo hfi;
		
	CycleBalls();
	GetRString(tmpSuffix,TEMP_SUFFIX);
	PCopy(tmpName,name);
	PCat(tmpName,tmpSuffix);
	
	if ((tocH=TOCByName(dirId,name))==nil) goto done;
	
	if (err=BoxFOpen(tocH)) goto done;
	oldRefN = (*tocH)->refN;
	newVRef=(*tocH)->vRef;newDirId=(*tocH)->dirId;
	if (err=MyResolveAlias(&newVRef,&newDirId,name,nil)) goto done;
	if (!HGetFileInfo(newVRef,newDirId,tmpName,&hfi) && hfi.ioFlLgLen)
		{FileSystemError(SQUISH_LEFTOVERS,tmpName,dupFNErr); goto done;}
	if (err=MakeResFile(tmpName,newVRef,newDirId,CREATOR,MAILBOX_TYPE))
		{FileSystemError(COULDNT_SQUEEZE,name,err); goto done;}
	if (err=FSHOpen(tmpName,newVRef,newDirId,&newRefN,fsRdWrPerm))
		{FileSystemError(COULDNT_SQUEEZE,name,err); goto done;}

	/*
	 * figure out how big it should be
	 */
	for (mNum=0;mNum<(*tocH)->count;mNum++)
		size += (*tocH)->sums[mNum].length;
	if (err=MyAllocate(newRefN,size))
		{FileSystemError(COULDNT_SQUEEZE,name,err); goto done;}

	size = 0;
	for (mNum=0;mNum<(*tocH)->count;mNum++)
	{
	  CycleBalls();
		if ((err=CopyFBytes(oldRefN,(*tocH)->sums[mNum].offset,
				(*tocH)->sums[mNum].length,newRefN,size)))
			{FileSystemError(COULDNT_SQUEEZE,name,err); goto done;}
		(*tocH)->sums[mNum].offset = size;
		size += (*tocH)->sums[mNum].length;
	}
	if (mNum>=(*tocH)->count)
	{
		/* success! */
		FSClose(newRefN); newRefN = 0;
		FSClose(oldRefN); oldRefN = 0;
		(*tocH)->refN = 0;
		
		/* now, copy the resource fork */
		(void) CopyRFork(newVRef,newDirId,tmpName,
							(*tocH)->vRef,(*tocH)->dirId,name);
		HGetFInfo(newVRef,newDirId,name,&info);
		
		/* do the deed */
		if (err=HDelete(newVRef,newDirId,name))
			{FileSystemError(COULDNT_SQUEEZE,name,err); goto done;}
		if (rnErr=err=HRename(newVRef,newDirId,tmpName,name))
			FileSystemError(BAD_COMP_RENAME,tmpName,err);
		HSetFInfo(newVRef,newDirId,name,&info);
	}
	
done:
	if (newRefN) FSClose(newRefN);
	if (oldRefN) {FSClose(oldRefN); (*tocH)->refN = 0;}
	if (tocH)
	{
		(*tocH)->dirty = err==0;
		(*tocH)->totalK = (*tocH)->usedK;	/* let's not do this again, shall we? */
		CloseMyWindow((*tocH)->win);
	}
	if (err && !rnErr) HDelete(newVRef,newDirId,tmpName);
}


/************************************************************************
 * NeedAutoCompact - should we compact this mailbox?
 ************************************************************************/
Boolean NeedAutoCompact(TOCHandle tocH)
{
	long waste = ((*tocH)->totalK-(*tocH)->usedK)K;
	
	/*
	 * is there room to do it?
	 */
	if ((*tocH)->volumeFree < ((*tocH)->usedK+10)K) return (False);
	
	/*
	 * is there too much waste space?
	 */
	if ((100*waste)/((*tocH)->totalK K + 1) > GetRLong(COMPACT_WASTE_PER)) return(True);
	
	/*
	 * are we too close to the disk limit?
	 */
	if ((100*waste)/(*tocH)->volumeFree > GetRLong(COMPACT_FREE_PER)) return(True);
		
	return(False);
}
