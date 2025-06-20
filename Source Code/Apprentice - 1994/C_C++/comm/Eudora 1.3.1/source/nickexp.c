#define FILE_NUM 6
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * expansion of nicknames
 **********************************************************************/
#pragma load EUDORA_LOAD
#pragma segment NickExp

Boolean AliasRecursion;
/************************************************************************
 * ExpandAliases - take an address list (as from SuckAddresses), and
 * expand it using the alias list
 ************************************************************************/
UHandle  ExpandAliases(UHandle	fromH,short depth,Boolean wantComments)
{
	int err=0;
	UHandle toH,spewHandle;
	long offset, newOffset;
	
	if (++depth > MAX_DEPTH) return(nil);
	AliasRecursion = 0;
	if (!(err=RegenerateAliases(&Aliases,ALIAS_CMD)))
	{
		AliasRefCount++;
		toH = NuHandle(0L);
		if (!toH)
			WarnUser(ALLO_EXPAND,err=MemError());
		else
		{
			for (offset=0;(*fromH)[offset]; offset += (*fromH)[offset]+2)
			{
				/*
				 * get rid of the extraneous stuff
				 */
				spewHandle = SuckPtrAddresses(LDRef(fromH)+offset+1,(*fromH)[offset],False);
				UL(fromH);
				if (!spewHandle) {err=1;break;}
				newOffset=FindExpansionFor(Aliases,LDRef(spewHandle)+1,**spewHandle);
				DisposHandle(spewHandle);
				if (newOffset)
				{
					UHandle lookup,result;
					lookup = SuckPtrAddresses(LDRef(Aliases)+newOffset+2,
										 (*Aliases)[newOffset]*256+(*Aliases)[newOffset+1],wantComments);
					UL(Aliases);
					if (lookup==nil) err=1;
					else
					{
						/*
						* now, expand the addresses in the expansion
						*/
						result=ExpandAliases(lookup,depth,wantComments);
						if (!result)
							if (!(err = MemError()))
							{
								err = 1;
								if (!AliasRecursion)
								{
									Str255 msg;
									Str255 alias;
									GetRString(msg,ALIA_LOOP);
									BlockMove(&(*fromH)[offset],alias,(*fromH)[offset]+1);
									MyParamText(msg,alias,"","");
									(void) ReallyDoAnAlert(OK_ALRT,Stop);
									AliasRecursion = 1;
								}
							}
						DisposHandle(lookup);
						
						/*
						 * add the expanded aliases to the new list
						 */
						if (!err && result)
						{
							SetHandleBig(result,GetHandleSize(result)-1);
							HLock(result);
							HandAndHand(result,toH);
							err = MemError();
							DisposHandle(result);
						}
						else if (!err)
							err = MemError();
					}
				}
				else
				{
					Str63 temp;
					/*
					 * original was NOT an alias; just copy it
					 */
					/* if it is "me" then the user has not defined me as a nickname.
						 just put in the return address. */
					GetRString(temp, ME);
					LDRef(fromH);
					if (EqualString(temp,(*fromH)+offset,True,True))
					{
						GetReturnAddr(temp, True);
						err = PtrAndHand(temp, toH, temp[0]+2);
					}
					else
						err=PtrAndHand((*fromH)+offset,toH,(*fromH)[offset]+2);
					HUnlock(fromH);
				}
				if (err && err!=1) {WarnUser(ALLO_EXPAND,err); break;}
			}
		}
		if (Aliases && AliasRefCount && !--AliasRefCount) HPurge(Aliases);
	}
	if (!err)
	{
		SetHandleBig(toH,GetHandleSize(toH)+1);
		if (!(err=MemError())) (*toH)[GetHandleSize(toH)-1] = 0;
	}
	if (err)
	{
		DisposHandle(toH);
		AliasRecursion = 1;
	}
	return(err ? nil : toH);
}

/************************************************************************
 * FindAliasFor - find an alias.	Returns the offset into the Aliases block.
 *	returns -1 if none found.
 ************************************************************************/
long FindAliasFor(UHandle aliases,UPtr name,short size)
{
	long max = GetHandleSize(aliases);
	long offset;
	Str255 pName;
	
	BlockMove(name,pName+1,size);
	*pName = size;
	/*RemoveParens(pName);*/
	
	for (offset=0;
				offset<max;
				offset+=CountAliasTotal(aliases,offset))
		if (EqualString(pName,(*aliases)+offset,False,False)) break;
	if (offset<max) return(offset);
	else return(-1);
}

/************************************************************************
 * FindExpansionFor - find the expansion for an alias.	Returns an
 * offset into the Aliases block.  returns 0 if none found.
 ************************************************************************/
long FindExpansionFor(UHandle aliases,UPtr name,short size)
{
	long offset = FindAliasFor(aliases,name,size);

	if (offset>=0) return(offset+CountAliasAlias(aliases,offset)+1);
	else return(0);
}
/************************************************************************
 * FinishAlias - finish a partially completed alias
 ************************************************************************/
void FinishAlias(MyWindowPtr win, Boolean wantExpansion, Boolean findOnly)
{
	Str63 word;
	UPtr spot, begin, end;
	TEHandle teh=WinTEH(win);
	long offset,max;
	short found=0;
	short which,current;
	long foundOffset=0;
	uShort save;
	
	if (!teh) return;
	
	begin = *(*teh)->hText;
	if ((*teh)->selStart != (*teh)->selEnd)
	{
		spot = begin+(*teh)->selStart;
		end = begin+(*teh)->selEnd-1;
	}
	else
	{
		spot = end = *(*teh)->hText + (*teh)->selStart - 1;
		while(spot>=begin && *spot>' ' && *spot!=',' && *spot!='(') spot--;
		spot++;
		if (spot>end) return;
	}
	*word = MIN(sizeof(word)-1,end-spot+1);
	BlockMove(spot,word+1,*word);
	
	if (RegenerateAliases(&Aliases,ALIAS_CMD)) return;
	
	max = GetHandleSize(Aliases);
	for (offset=0,current=0;
				offset<max;
				offset+=CountAliasTotal(Aliases,offset),current++)
	{
		save = (*Aliases)[offset];
		if (save<*word) continue;
		(*Aliases)[offset] = *word;
		if (EqualString(word,(*Aliases)+offset,False,True))
		{
			if (!found)
			{
				foundOffset=offset;
				which=current;
			}
			else if (found == 1 && !findOnly)
				SysBeep(20);
			found++;
		}
		(*Aliases)[offset] = save;
	}
	
	if (found == 1 && !findOnly)
	{
		TESetSelect(spot-begin,end-begin+1,teh);
		InsertAlias(win,foundOffset,wantExpansion);
	}
	if (found && AliasWinIsOpen() && win->qWindow.windowKind!=ALIAS_WIN)
		ChooseAlias(which+1);
	HPurge(Aliases);
}

/************************************************************************
 * InsertAlias - insert an alias in a window
 ************************************************************************/
void InsertAlias(MyWindowPtr win, long foundOffset,Boolean wantExpansion)
{
	short i,n;
	UHandle wordH,list;
	UPtr spot,begin;
	
	Undo.didClick = True;
	if (!wantExpansion)
	{
		n = CountAliasAlias(Aliases,foundOffset);
		for (i=1;i<=n;i++)
			TESomething(win,TEKEY,(*Aliases)[foundOffset+i],0);
	}
	else
	{
		if (!(wordH=NuHandle((*Aliases)[foundOffset]+3)))
			WarnUser(MEM_ERR,MemError());
		else
		{
			PCopy(*wordH,(*Aliases)+foundOffset);
			(*wordH)[**wordH+1] = (*wordH)[**wordH+2] = 0;
			list = ExpandAliases(wordH,0,True);
			DisposHandle(wordH);
			if (list)
			{
				Undo.didClick = True;
				begin=LDRef(list);
				for (spot=begin; *spot; spot++)
				{
					if (spot!=begin)
					{
						TESomething(win,TEKEY,',',0);
						TESomething(win,TEKEY,' ',0);
					}
					spot++;
					while (*spot) TESomething(win,TEKEY,*spot++,0);
				}
				DisposHandle(list);
			}
		}
	}
}
