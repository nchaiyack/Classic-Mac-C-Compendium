#define FILE_NUM 45
/* Copyright (c) 1991-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment BuildTOC

	void GleanFrom(UPtr line,MSumPtr sum);
	long BeautifyDate(UPtr dateStr,long *zoneSecs);
	short SalvageTOC(TOCHandle old, TOCHandle new);
	void TrimWrap(UPtr str,int openC,int closeC);
	long UnixDate2Secs(PStr date);
	short MonthNum(CStr cp);

/************************************************************************
 * RebuildTOC - rebuild a corrupt or out-of-date toc, salvaging what we
 * can.
 ************************************************************************/
TOCHandle ReBuildTOC(long dirId,UPtr name)
{
	short refN=0,err;
	long size;
	Str63 tocName;
	TOCHandle newTocH=nil;
	short oldCount, newCount, salvCount;
	
	/*
	 * read the old one into memory
	 */
	PCopy(tocName,name); PCat(tocName,"\p.toc");
	if (err=FSHOpen(tocName,MyVRef,dirId,&refN,fsRdPerm) ||
			(err=GetEOF(refN,&size)))
	{
		if (err!=fnfErr) FileSystemError(READ_TOC,tocName,err);
	}
	else
	{
		DamagedTOC = NuHandle(size);
		if (!DamagedTOC) WarnUser(READ_TOC,MemError());
		else
		{
			if (err=FSRead(refN,&size,LDRef(DamagedTOC)))
				FileSystemError(READ_TOC,tocName,err);
		}
		FSClose(refN);
	}
	if (err==fnfErr) err = noErr;
	
	if (!err || AlertStr(TOC_SALV_ALRT,Stop,name)==OK)
	{
		/*
			* build the new one
			*/
		if ((newTocH = BuildTOC(dirId,name)) && !err && DamagedTOC)
		{
			oldCount = (*DamagedTOC)->count;
			newCount = (*newTocH)->count;
			if (oldCount && newCount)
			{
				Str255 s;
				salvCount = SalvageTOC(DamagedTOC,newTocH); 		/* and salvage */
				ComposeRString(s,SALV_REPORT,salvCount,oldCount,newCount-salvCount);
				AlertStr(OK_ALRT,Note,s);
			}
		}
	}
	
	if (DamagedTOC) ZapHandle(DamagedTOC);
	return(newTocH);
}

/************************************************************************
 * SalvageTOC - reconcile and old a new TOC
 ************************************************************************/
short SalvageTOC(TOCHandle old, TOCHandle new)
{
	short first,last,mid;
	MSumPtr oldSum;
	long offset;
	short salvaged=0;
	
	LDRef(old); LDRef(new);
	(*old)->count = (GetHandleSize(old)-(sizeof(TOCType)-sizeof(MSumType)))/
									sizeof(MSumType);
	
	for (oldSum=(*old)->sums;oldSum<(*old)->sums+(*old)->count;oldSum++)
	{
		offset = oldSum->offset;
		first = 0; last=(*new)->count-1;
		for (mid=(first+last)/2;first<=last;mid=(first+last)/2)
			if (offset<(*new)->sums[mid].offset)
				last = mid -1;
			else if (offset==(*new)->sums[mid].offset)
				break;
			else
				first = mid+1;
		if (first<=last && (*new)->sums[mid].length==oldSum->length)
		{
			salvaged++;
			(*new)->sums[mid] = *oldSum;
		}
	}
	
	UL(old); UL(new);
	CleanseTOC(new);
	return(salvaged);
}

/**********************************************************************
 * BuildTOC - build a table of contents for a file.  The TOC is built
 * in memory.  This gets a little hairy in spots because the routine is
 * used both for received messages and messages under composition; but
 * the complication does not substantially affect the flow of the
 * function, so I let it stand.
 **********************************************************************/
TOCHandle BuildTOC(long dirId,UPtr name)
{
	TOCHandle tocH=nil;
	MSumType sum;
	int err;
	Boolean isOut;
	Str255 scratch;
	
	/*
	 * first, try opening the file
	 */
	if (err=OpenLine(MyVRef,dirId,name))
	{
		FileSystemError(OPEN_MBOX,name,err);
		return(nil);
	}
	
	if ((tocH=NewZHandle(sizeof(TOCType)))==nil)
	{
		WarnUser(READ_MBOX,MemError());
		goto failure;
	}
	
	/*
	 * figure out for once and for all if we are an in or an out box
	 */
	isOut = False;
	if (dirId==MyDirId)
	{
		GetRString(scratch,IN);
		if (EqualString(scratch,name,False,True))
			(*tocH)->which = IN;
		else
		{
			GetRString(scratch,OUT);
			if (EqualString(scratch,name,False,True))
			{
				(*tocH)->which = OUT;
				isOut = True;
			}
			else
			{
				GetRString(scratch,TRASH);
				if (EqualString(scratch,name,False,True))
					(*tocH)->which = TRASH;
			}
		}
	}
	
	ReadSum(nil,False);
	while (ReadSum(&sum,isOut))
	{
		if (!SaveMessageSum(&sum,tocH))
		{
			WarnUser(SAVE_SUM,MemError());
			goto failure;
		}
	}
	ReadSum(nil,False);

	/*
	 * success
	 */
	CloseLine();
	(*tocH)->refN = 0;
	(*tocH)->vRef = MyVRef;
	(*tocH)->dirId = dirId;
	BlockMove(name,(*tocH)->name,*name+1L);
	(*tocH)->version = 0;
	(*tocH)->dirty = True;
	SetHandleBig(tocH,sizeof(TOCType) + 
		((*tocH)->count ? (*tocH)->count-1 : 0)*sizeof(MSumType));
	return (tocH);

failure:
	if (tocH != nil) DisposHandle(tocH);
	CloseLine();
	return(nil);
}

 /************************************************************************
 * ReadSum - read a summary, from the current position in the lineio
 * routines.
 ************************************************************************/
Boolean ReadSum(MSumPtr sum,Boolean isOut)
{ 
	static int type;
	Str255 line;
	static UHandle oldLine=nil;
	enum {BEGIN, IN_BODY, IN_HEADER} state;
	Str63 duck;
	long secs;
	Str31 prior;
	Boolean lookPrior;
	long origZone;
	
	if (!sum)
	{
		if (oldLine)
		{
			DisposHandle(oldLine);
			oldLine = nil;
		}
		return(True);
	}
	
	/*
	 * set up the priority stuff
	 */
	if (lookPrior = !PrefIsSet(PREF_NO_IN_PRIOR)) GetRString(prior,HEADER_STRN+PRIORITY_HEAD);

	/*
	 * now, read from it line by line
	 * break messages when sendmail-style From line is found
	 */
	state = BEGIN;
	WriteZero(sum,sizeof(MSumType));
	sum->state = UNREAD;
	sum->tableId = DEFAULT_TABLE;
	while (oldLine || (type=GetLine(line,sizeof(line))))
	{
		if (oldLine)
		{
			BlockMove(*oldLine,line,sizeof(line));
			DisposHandle(oldLine);
			oldLine = nil;
		}
		switch(type)
		{
			case LINE_START:
				if (IsFromLine(line))
				{
					if (state!=BEGIN)
					{
						if ((oldLine=NuHandle(sizeof(line)))==nil)
						{
							WarnUser(MEM_ERR,MemError());
							return(False);
						}
						BlockMove(line,*oldLine,sizeof(line));
						return(True);
					}
					WriteZero(sum,sizeof(MSumType));
					sum->tableId = DEFAULT_TABLE;
					if (!isOut) GleanFrom(line,sum);
					sum->offset = TellLine();
					sum->state = isOut ? UNSENDABLE : UNREAD;
					state = IN_HEADER;
				}
				else if (state==IN_HEADER)
				{ 											/* look for date, subj, from lines */
					if (*line=='\n')
					{
						state = IN_BODY;
						sum->bodyOffset = TellLine()-sum->offset;
					}
					else if (!striscmp(line,"date:"))
					{
						CopyHeaderLine(duck,sizeof(duck),line);
						if (secs=BeautifyDate(duck,&origZone)) PtrTimeStamp(sum,secs,origZone);
						else
						{
							if (*duck > sizeof(sum->date)-2) *duck=sizeof(sum->date)-2;
							BlockMove(duck,sum->date,sizeof(sum->date));
						}
					}
					else if (!isOut && !striscmp(line,"from:"))
					{
						CopyHeaderLine(sum->from,sizeof(sum->from),line);
						BeautifyFrom(sum->from);
					}
					else if (isOut && !striscmp(line,"to:"))
					{
						CopyHeaderLine(sum->from,sizeof(sum->from),line);
						if (*sum->from) sum->state = SENDABLE;
					}
					else if (!striscmp(line,"subject:"))
					{
						CopyHeaderLine(sum->subj,sizeof(sum->subj),line);
					}
					else if (!striscmp(line,"status: r"))
						sum->state=READ;
					else if (lookPrior && !striscmp(line,prior+1))
						sum->priority = sum->origPriority = Display2Prior(atoi(line+*prior));
				}
				break;
			case LINE_MIDDLE:
				break;
			default:
				state=BEGIN;
				goto done;					 /* error message already given */
				break;
		}
		sum->length += strlen(line);
		if (state==BEGIN) state=IN_HEADER;
	}
done:
	return(state!=BEGIN);
}

/**********************************************************************
 * BeautifySum - beautify a message summary before saving it.  This
 * involves beautifying the from address and the date.
 **********************************************************************/
void BeautifySum(MSumPtr sum)
{
	if (sum->seconds) PtrTimeStamp(sum,sum->seconds,ZoneSecs());
	BeautifyFrom(sum->from);
}

/**********************************************************************
 * IsFromLine - determine whether or not a given line is a sendmail From
 * line.
 **********************************************************************/
Boolean IsFromLine(UPtr line)
{
	int num,len;
	int quote=0;
	Str255 scratch;
	UPtr cp;
	short weekDay,year,tym,day,month,other,remote,from;

#define isdig(c) ('0'<=(c) && (c)<='9') 
	if (line[0]!='F' || line[1]!='r' || line[2]!='o' || line[3]!='m')
		return (False);  /* quick and dirty */
	
	/*
	 * it passed the first test.	Now, we have to be more rigorous.
	 * this is a sample sendmail From line:
	 * >From paul@uxc.cso.uiuc.edu Wed Jun 14 12:36:18 1989<
	 * However, various systems do various bad things with the From line.
	 * therefore, I've changed it to look for:
	 */
	
	/* check for the space after the from */
	line += 4;
	if (*line++!=' ') return (False);
	
	/* skip the return address */
	while (*line && (quote || *line!=' '))
	{
		if (*line=='"') quote = !quote;
		line++;
	}
	if (!*line++) return (False);
	while (*line==' ') line++;
	
	remote=from=weekDay=day=from=year=tym=month=other=0;
	len = strlen(line);
	if (len>sizeof(scratch)-1) return(False);
	strcpy(scratch,line);
	for (cp=strtok(scratch," \t\n,");cp;cp=strtok(nil," \t\n,"))
	{
		len = strlen(cp);
		num = atoi(cp);
		if (num<24 && (len>=5 && cp[2]==':') && (len==5 || len==8 && cp[5]==':'))
		{
			if (tym++) return(False);
		}
		else if (!year && day && len==2 && isdig(cp[len-1]))
		{
			if (year++) return(False);
		}
		else if (len<=2 && num && num<32)
		{
			if (day++) return(False);
		}
		else if (len==4&&num>1900)
		{
			if (year++) return(False);
		}
		else if (len==6 && !striscmp(cp,"remote"))
		{
			if (remote++ || from) return(False);
		}
		else if (len==4 && !striscmp(cp,"from"))
		{
			if (!remote || from++) return(False);
		}
		else if (len==3 &&
			!(striscmp(cp,"mon") && striscmp(cp,"tue") && striscmp(cp,"wed")
			 && striscmp(cp,"thu") && striscmp(cp,"fri")
			 && striscmp(cp,"sat") && striscmp(cp,"sun")))
		{
			if (weekDay++) return(False);
		}
		else if (len==3 && !(striscmp(cp,"jan") && striscmp(cp,"feb") &&
			striscmp(cp,"mar") && striscmp(cp,"apr") &&
			striscmp(cp,"may") && striscmp(cp,"jun") &&
			striscmp(cp,"jul") && striscmp(cp,"aug") &&
			striscmp(cp,"sep") && striscmp(cp,"oct") &&
			striscmp(cp,"nov") && striscmp(cp,"dec")))
		{
			if (month++) return(False);
		}
		else
		{
			other++;
		}
	}
	return (day && year && month && tym && other<=2);
}
/**********************************************************************
 * GleanFrom - grab relevant info from sendmail From line
 * input line is in C format, fields in summary are in Pascal form
 **********************************************************************/
void GleanFrom(UPtr line,MSumPtr sum)
{
	Str255 copy;
	register char *cp=copy;
	register char *ep;
	long seconds;
	long offset = ZoneSecs();
	
	strcpy(copy,line);
	/*
	 * from address
	 */
	while (*cp++ != ' ');
	for (ep=cp; *ep!= ' '; ep++);
	*ep = '\0';
	MakePStr(sum->from,cp,ep-cp);
	
	/*
	 * date
	 */
	for (cp=++ep;*ep!='\n' && *ep; ep++);
	*ep = '\0';
	MakePStr(sum->date,cp,ep-cp);
	
	/*
	 * TimeStamp
	 */
	seconds = UnixDate2Secs(sum->date)-offset;
	PtrTimeStamp(sum,seconds,offset);
}


/************************************************************************
 * UnixDate2Secs - convert a UNIX date into seconds
 ************************************************************************/
long UnixDate2Secs(PStr date)
{
	Str63 copy;
	UPtr end;
	DateTimeRec dtr;
	long secs = 0;
	
	PCopy(copy,date);
	end = copy+*copy;
	/* let's null-terminate this, shall we? */
	end[1] = 0;
	
	/* back up and grab the year */
	while (*end!=' ' && end>copy) end--;
	dtr.year = atoi(end);
	if (dtr.year>0 && dtr.year<1900) dtr.year+=1900;
	
	/* seconds */
	*end = 0;
	while (*end!=':'&&end>copy) end--;
	dtr.second = atoi(end+1);
	
	/* minutes */
	*end = 0;
	while (*end!=':'&&end>copy) end--;
	dtr.minute = atoi(end+1);
	
	/* hours */
	*end = 0;
	while (*end!=' '&&end>copy) end--;
	dtr.hour = atoi(end+1);
	
	/* date */
	*end = 0;
	while (*end!=' '&&end>copy) end--;
	dtr.day = atoi(end+1);

	/* Month */
	*end = 0;
	while (*--end==' ' && end>=copy);
	while (*end!=' '&&end>copy) end--;
	dtr.month = MonthNum(end+1);
	
	if (dtr.year && dtr.month) Date2Secs(&dtr,&secs);
	return(secs);
}

/************************************************************************
 * MonthNum - get the month number from a month name
 ************************************************************************/
short MonthNum(CStr cp)
{
	char monthStr[4];
	short month=0;
	
	/*
	 * copy and lowercase
	 */
	BlockMove(cp,monthStr,3);
	monthStr[3] = 0;
	for (cp=monthStr;*cp;cp++) if (isupper(*cp)) *cp = tolower(*cp);
	cp = monthStr;
	
	switch(cp[0])
	{
		case 'j': month = cp[1]=='a' ? 1 : (cp[2]=='n' ? 6 : 7); break;
		case 'f': month = 2; break;
		case 'm': month = cp[2]=='r' ? 3 : 5; break;
		case 'a': month = cp[1]=='p' ? 4 : 8; break;
		case 's': month = 9; break;
		case 'o': month = 10; break;
		case 'n': month = 11; break;
		case 'd': month = 12; break;
	}
	return(month);
}

/************************************************************************
 * BeautifyDate - make a date look better
 * assumes the date is in smtp date format
 ************************************************************************/
long BeautifyDate(UPtr dateStr,long *origZone)
{
	UPtr cp, cp2;
	DateTimeRec dtr;
	long secs;
	long offset;
	*origZone = 0;
	
	if (*dateStr < 10) return(0);  /* ignore really short dates */
	dateStr[*dateStr+1] = 0;
	WriteZero(&dtr,sizeof(dtr));
	
	/*
	 * delete day of the week
	 */
	if (cp=strchr(dateStr+1,','))
	{
		for (cp++;isspace(*cp);cp++);
		strcpy(dateStr+1,cp);
		*dateStr = strlen(dateStr+1);
	}
	
	/*
	 * make sure that single-digit dates begin with a space, not a '0' and
	 * not nothing
	 */
	dtr.day = atoi(dateStr+1);
	if (dateStr[1]=='0')
		dateStr[1] = ' ';
	else if (dateStr[2]==' ')
	{
		Str255 temp;
		*temp = 1;
		PCat(temp,dateStr);
		PCopy(dateStr,temp);
		dateStr[1] = ' ';
	}
		
	/*
	 * delete year, if present
	 */
	/* pointing at day */
	for (cp=dateStr+2;*cp && !isspace(*cp);cp++);
	for (;isspace(*cp);cp++);
	/* pointing at month */
	dtr.month = MonthNum(cp);
	for (;*cp && !isspace(*cp);cp++);
	for (cp2=cp;isspace(*cp2);cp2++);
	
	if (*cp && isdigit(*cp2))
	{
		dtr.year = atoi(cp2);
		if (dtr.year<20) dtr.year += 2000;
		else if (dtr.year<1900) dtr.year += 1900;
		while (isdigit(*cp2)) cp2++;
		strcpy(cp,cp2);
		*dateStr -= cp2-cp;
		dateStr[*dateStr+1] = 0;
	}
	
	/*
	 * delete the seconds, if present
	 */
	dtr.hour = atoi(cp);
	if (cp=strchr(cp,':'))
	{
		dtr.minute = atoi(cp+1);
		if (cp[3]==':')
		{
			dtr.second = atoi(cp+4);
			strcpy(cp+3,cp+6);
			*dateStr -= 3;
			dateStr[*dateStr+1] = 0;
		}
	}
	
	if (dtr.year)
	{
		Date2Secs(&dtr,&secs);
		cp = strchr(dateStr+1,':');
		cp2 = dateStr+*dateStr+1;
		if (cp&&cp+4<cp2)
		{
			cp+=4;
			if (offset = atoi(cp))
			{
				if (*cp == '-') offset *= -1;
				offset = (3600 * offset)/100 + 60*(offset%100);
				if (*cp == '-') offset *= -1;
			}
			else
			{
				offset = TZName2Offset(cp);
			}
		}
		else
			offset = ZoneSecs();
		secs -= offset;
		*origZone = offset;
	}
	else secs = 0;
	return(secs);
}

/************************************************************************
 * BeautifyFrom - make a from line look better
 ************************************************************************/
void BeautifyFrom(UPtr fromStr)
{
	UPtr cp1,cp2;
	int len;
	
	fromStr[*fromStr+1] = 0;
	
	/*
	 * elide <>'ed text, unless it's all there is
	 */
	TrimWhite(fromStr);
	if (cp1=strchr(fromStr+1,'<'))
	{
		if (cp2=strchr(cp1+1,'>'))
		{
			while (cp2[1]==' ') cp2++;
			len = cp2 - cp1 - 1;
			if (len>0 && len < *fromStr-2)
			{
				strcpy(cp1,cp2+1);
				*fromStr -= len+2;
			}
		}
	}
	/*
	 * prefer parenthesized text
	 */
	else if (cp1=strchr(fromStr+1,'('))
		if (cp2=strchr(cp1+1,')'))
		{
			len = cp2 - cp1 - 1;
			if (len>0)
			{
				*fromStr = len;
				strncpy(fromStr+1,cp1+1,len);
				fromStr[*fromStr+1] = 0;
			}
		}
	TrimWhite(fromStr);
	TrimWrap(fromStr,'(',')');
	TrimWrap(fromStr,'"','"');
}

void TrimWrap(UPtr str,int openC,int closeC)
{
  if (*str>1 && str[1] == openC && str[*str] == closeC)
	{
	  BlockMove(str+2,str+1,*str-2);
		*str -= 2;
		str[*str+1] = 0;
	}
}
	
/************************************************************************
 * SumToFrom - build a sendmail-style from line from a message summary
 ************************************************************************/
int SumToFrom(MSumPtr sum, UPtr fromLine)
{
	char *start, *end;
	Str31 delims;
	Str63 fromWhom;
	short n;
	
	GetRString(delims,DELIMITERS);
	if (sum && Tokenize(sum->from+1,*sum->from,&start,&end,delims))
	{
		strncpy(fromWhom+1,start,end-start);
		*fromWhom = end-start;
	}
	else
		GetRString(fromWhom,UNKNOWN_SENDER);

	PCopy(fromLine,"\pFrom ");
	PCat(fromLine,fromWhom);
	n = fromLine[0]+1;
	LocalDateTimeStr(fromLine+n);
	fromLine[0] += fromLine[n]+1;
	fromLine[n] = ' ';
	p2cstr(fromLine);
	return(strlen(fromLine));
}

/**********************************************************************
 * CopyHeaderLine - copy the contents of a header line (C format)
 * into a Pascal string.
 **********************************************************************/
void CopyHeaderLine(UPtr to,int size,UPtr from)
{
	while (*from++ != ':');
	for (from++; *from == ' '; from++);
	strncpy(to,from,size-2);
	to[size-2] = 0;
	c2pstr(to);
	TrimWhite(to);
} 

/************************************************************************
 * FindTOCSpot - find a spot in a file that's big enough to hold a
 * message.
 ************************************************************************/
long FindTOCSpot(TOCHandle tocH, long length)
{
#pragma unused(length)
	long end=0;
	MSumPtr sum;
	MSumPtr limit;
	
	/*
	 * for now, just return the end
	 */
	sum = (*tocH)->sums;
	limit = sum + (*tocH)->count;
	for (; sum<limit ; sum++)
		if (end < sum->offset+sum->length)
			end = sum->offset + sum->length;
	
	return(end);
}
