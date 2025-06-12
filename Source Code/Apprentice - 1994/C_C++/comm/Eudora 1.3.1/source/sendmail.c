#define FILE_NUM 34
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * functions for dealing with a sendmail (or other?) smtp server
 ************************************************************************/
#pragma load EUDORA_LOAD
#pragma segment SMTP
typedef enum
{
	helo=1, mail, rcpt, data, rset, send, soml, saml,
	vrfy, expn, help, noop, quit, turn
} SMTPEnum;

#define CMD_BUFFER 1024
#define IsAddressHead(head) (head==TO_HEAD || head==BCC_HEAD || head==CC_HEAD)
typedef struct wdsEntry *WDSPtr;
/************************************************************************
 * declarations for private routines
 ************************************************************************/
	int DoIntroductions(void);
	int SayByeBye(void);
	int SendCmd(int cmd, UPtr args);
	int SendCmdGetReply(int cmd, UPtr args, Boolean chatter);
	int SendHeaderLine(int header,TEHandle teh);
	int SendAttachments(TEHandle teh,Boolean doWrap,Boolean plainText);
	int SMTPCmdError(int cmd, UPtr args, UPtr message);
	void PrimeProgress(MessType **messH);
	int WannaSend(MyWindowPtr win);
	void GrabSignature(void);
	short SendPlain(short vRef,long dirId,UPtr name,Boolean doWrap);
	UPtr PriorityHeader(UPtr buffer,Byte priority);
int sErr;
UHandle Signature=nil;
short SendEm(WDSPtr entries,WDSPtr *current);
short AddWDS(WDSPtr entries, WDSPtr *current,Uptr bf,long len,short maxC);

/************************************************************************
 * Public routines
 ************************************************************************/
/************************************************************************
 * StartSMTP - initiate a connection with the specified SMTP server
 ************************************************************************/
int StartSMTP(UPtr serverName, short port)
{
	if (UUPCOut) sErr=UUPCPrime(serverName);
	else if (!(sErr=ConnectTrans(serverName,port,False)))
		sErr=DoIntroductions();
	GrabSignature();
	return(sErr);
}

/************************************************************************
 * SendMessage - send a message to the SMTP server
 ************************************************************************/
int SendMessage(TOCType **tocH,int sumNum)
{
	Str255 buffer;
	MessHandle messH;
	
	/*
	 * handle open, dirty windows
	 */
	if (!(messH=SaveB4Send(tocH,sumNum))) return(1);
	
	/*
	 * Log, if we must
	 */
	GetWTitle((*messH)->win,buffer);
	ComposeLogR(LOG_SEND,nil,SENDING,buffer);
	
	/*
	 * reset SMTP
	 */
	sErr = SendCmdGetReply(rset,nil,True);
	if (sErr/100 != 2) return(sErr);
	sErr = 0;
	
	/*
	 * envelope
	 */
	GetReturnAddr(buffer,False);
	sErr = SendCmdGetReply(mail,buffer,True);
	if (sErr/100 != 2) return(sErr);
	sErr = 0;
	
	if (WrapWrong) OffsetWindow((*messH)->win);
	
	if (sErr=DoRcptTos(messH,True)) goto done;
	
	/*
	 * and the body
	 */
	sErr = SendCmdGetReply(data,nil,True);
	if (sErr/100 != 3) goto done;
	sErr = 0;
	
	if (sErr=TransmitMessage(messH,True)) goto done;
	
	TimeStamp(tocH,sumNum,GMTDateTime(),ZoneSecs());
			
done:
	CloseMyWindow((*messH)->win);
	(void)ComposeLogR(LOG_SEND,nil,sErr?FAILED:SUCCEEDED,sErr);
	return(sErr);
}

/************************************************************************
 * EndSMTP - done talking to SMTP
 ************************************************************************/
int EndSMTP()
{
	if (UUPCOut) UUPCDry();
	else
	{
		if (sErr || CommandPeriod) SilenceTrans(True);
		if ((!sErr || (sErr<600 && sErr>=400)) && !(sErr=SayByeBye()))
			DisTrans();
		sErr = DestroyTrans();
	}
	if (Signature) ZapHandle(Signature);
	return(sErr);
}

/************************************************************************
 * SMTPError - return the last SMTP error
 ************************************************************************/
int SMTPError(void)
{
	return (sErr);
}

/************************************************************************
 * Private routines
 ************************************************************************/
/************************************************************************
 * DoIntroductions - take care of the beginning of the SMTP protocol
 ************************************************************************/
int DoIntroductions(void)
{
	Str255 buffer;
	
	/*
	 * get banner from the remote end
	 */
	sErr = GetReply(buffer,sizeof(buffer),True);
	if (sErr/100 != 2) return(sErr);
	sErr = 0;
	/*
	 * tell it who we are
	 */
	sErr = SendCmdGetReply(helo,WhoAmI(buffer),True);
	return((sErr/100 != 2) ? sErr :  (sErr=0));
}

/************************************************************************
 * SayByeBye - take care of the end of the SMTP protocol
 ************************************************************************/
int SayByeBye(void)
{ 
	sErr = SendCmdGetReply(quit,nil,False);
	return((sErr/100 != 2) ? sErr : (sErr=0));	
}

/************************************************************************
 * SendCmd - send an smtp command, with optional arguments
 ************************************************************************/
int SendCmd(int cmd, UPtr args)
{
	Byte buffer[CMD_BUFFER];

	GetRString(buffer,SMTP_STRN+cmd);
	if (args && *args)
		PCat(buffer,args);
	Progress(NoBar,buffer); 
	PCat(buffer,NewLine);
	
	if (sErr=SendTrans(1,buffer+1,*buffer)) return(sErr);
	
	return(noErr);
}

/************************************************************************
 * SMTPCmdError - report an error for an SMTP command
 ************************************************************************/
int SMTPCmdError(int cmd, UPtr args, UPtr message)
{
	Str255 theCmd;
	Str255 theError;
	int err;

	GetRString(theCmd,2800+cmd);
	if (args && *args)
		PCat(theCmd,args);
	strcpy(theError+1,message);
	*theError = strlen(theError+1);
	if (theError[*theError]=='\012') (*theError)--;
	if (theError[*theError]=='\015') (*theError)--;
	MyParamText(theCmd,theError,"\pSMTP","");
	err = ReallyDoAnAlert(PROTO_ERR_ALRT,Note);
	return(err);
}

/************************************************************************
 * SendCmdGetReply - send an smtp command, with optional arguments, and
 * wait for the reply.	Returns reply code.
 ************************************************************************/
int SendCmdGetReply(int cmd, UPtr args, Boolean chatter)
{
	char buffer[CMD_BUFFER];
	
	if (sErr=SendCmd(cmd,args)) return(601);			/* error in transmission */
	sErr = GetReply(buffer,sizeof(buffer),False);
	if (sErr>399 && sErr<=600 && sErr!=550 && chatter)
		SMTPCmdError(cmd,args,buffer);
	return(sErr);
}

/************************************************************************
 * GetReply - get a reply to an SMTP command
 ************************************************************************/
int GetReply(UPtr buffer, int size, Boolean verbose)
{
	long rSize;
	Str127 scratch;
	char *cp;
	short err;
	
	do
	{
		rSize = size;
		if (sErr=RecvLine(buffer,&rSize)) return(602);	/* error receiving */
		if (verbose)
		{
			*scratch = MIN(127,rSize);
			strncpy(scratch+1,buffer,*scratch);
			Progress(NoChange,scratch);
		}
		for (cp=buffer;cp<buffer+rSize && (*cp < ' ' || *cp>'~');cp++);
		rSize -= cp-buffer;
	}
	while (rSize<3 ||
				 !isdigit(cp[0])||!isdigit(cp[1])||!isdigit(cp[2]) ||
								 rSize>3 && cp[3]=='-');
	cp[rSize] = 0;
	err = atoi(cp);
	if (verbose && err>399 && err<600) SMTPCmdError(nil,nil,buffer);
	return(err);
} 

/************************************************************************
 * DoRcptTos - tell the remote sendmail who is getting the message
 ************************************************************************/
int DoRcptTos(MessType **messH, Boolean chatter)
{
	sErr=DoRcptTosFrom((*messH)->txes[TO_HEAD-1],chatter);
	if (sErr) return(sErr);
	sErr=DoRcptTosFrom((*messH)->txes[BCC_HEAD-1],chatter); 
	if (sErr) return(sErr);
	sErr=DoRcptTosFrom((*messH)->txes[CC_HEAD-1],chatter);
	return(sErr);
}

/************************************************************************
 * DoRcptTosFrom - do the Rcpt to's from a particular TERec
 ************************************************************************/
int DoRcptTosFrom(TEHandle teh, Boolean chatter)
{
	Str255 toWhom;
	UHandle addresses,rawAddresses;
	UPtr address;

	sErr = 550;
	if (rawAddresses=SuckAddresses((*teh)->hText,(*teh)->teLength,False))
	{
		sErr=200;
		if (**rawAddresses)
		{
			addresses = ExpandAliases(rawAddresses,0,False);
			DisposHandle(rawAddresses);
			if (!addresses) return(sErr=500);
			for (address=LDRef(addresses); *address; address += *address + 2)
			{
				if (*address > MAX_ALIAS)
				{
					WarnUser(BAD_ADDRESS,0);
					sErr = 550;
					break;
				}
				if (UUPCOut)
				{
					if (sErr=UUPCWriteAddr(address)) break;
				}
				else
				{
					toWhom[0] = 1; toWhom[1] = '<';
					PCat(toWhom,address);
					PCatC(toWhom,'>');
					sErr = SendCmdGetReply(rcpt,toWhom,chatter);
					if (chatter && sErr==550)
						AlertStr(BAD_ADDR_ALRT,Stop,address);
					if (sErr/100 != 2) break;
				}
			}
			DisposHandle(addresses);
		}
		else
			DisposHandle(rawAddresses);
	}
	return(sErr/100!=2 ? sErr : (sErr=0));
}

/************************************************************************
 * TransmitMessage - send a message to the remote sendmail
 ************************************************************************/
int TransmitMessage(MessType **messH, Boolean chatter)
{
	int header;
	Str255 buffer;
	long flags=(*(*messH)->tocH)->sums[(*messH)->sumNum].flags;
	TEHandle body = (*messH)->txes[HEAD_LIMIT-1];
	Byte priority;
		
	PrimeProgress(messH);

	/*
	 * extra headers
	 */
	if (GetResource('STR#',EX_HEADERS_STRN))
	{
		for (header=1;;header++)
		{
			if (*GetRString(buffer,EX_HEADERS_STRN+header))
			{
				if (sErr=SendTrans(2,buffer+1,*buffer,NewLine+1,*NewLine))
					return(600);
			}
			else break;
		}
	}

	/*
	 * real headers
	 */
	priority = (*(*messH)->tocH)->sums[(*messH)->sumNum].priority;
	priority = Prior2Display(priority);
	if (priority!=3)
	{
		PriorityHeader(buffer,priority);
		if (sErr=SendTrans(2,buffer+1,*buffer,NewLine+1,*NewLine))
		  return(600);
	}
	
	BuildDateHeader(buffer,0);
	if (*buffer && (sErr=SendTrans(2,buffer+1,*buffer,NewLine+1,*NewLine)))
		return(600);
	
	for (header=1;header<HEAD_LIMIT;header++)
		if (header!=BCC_HEAD && (sErr=SendHeaderLine(header,(*messH)->txes[header-1])))
			return(600);
	GiveTime();
	
	/*
	 * body
	 */
	SendTrans(1,NewLine+1,*NewLine);
	
	if (WrapWrong)
	{
		LDRef(body);
		sErr=SendBodyLines((*body)->hText,(*body)->teLength,0,
											 (flags&FLAG_WRAP_OUT)!=0,True,(*body)->lineStarts,
											 (*body)->nLines,False);
		UL(body);
		if (sErr) return(600);
	}
	else if (sErr=SendBodyLines((*body)->hText,(*body)->teLength,0,
												 (flags&FLAG_WRAP_OUT)!=0,True,nil,0,False))
		return(600);

	if ((flags & FLAG_SIG) && Signature && (sErr=SendBodyLines(Signature,
			 GetHandleSize(Signature),0,(flags&FLAG_WRAP_OUT)!=0,True,nil,0,False)))
		return(600);
	
	if (sErr=SendAttachments((*messH)->txes[ATTACH_HEAD-1],
													 (flags&FLAG_WRAP_OUT)!=0,
													 (flags&FLAG_BX_TEXT)==0))
		return(600);
		
	if (!UUPCOut &&
			(sErr=SendTrans(3,NewLine+1,*NewLine,".",1,NewLine+1,*NewLine)))
		return(600);
	Progress(100,nil);
	
	if (!UUPCOut)
	{
		sErr = GetReply(buffer,sizeof(buffer),False);
		if (sErr > 399 && chatter)
			SMTPCmdError(data,"",buffer);
	}
	
	return(sErr/100 != 2 ? sErr : (sErr=0));
} 		

/************************************************************************
 * PriorityHeader: Build a priority header
 ************************************************************************/
UPtr PriorityHeader(UPtr buffer,Byte priority)
{
	return(ComposeRString(buffer,PRIORITY_FMT,HEADER_STRN+PRIORITY_HEAD,priority,PRIOR_STRN+priority));
}

/************************************************************************
 * These macros will help send bunches of lines at a time through MacTCP
 * Note that A/UX has VERY limited WDS capabilities.
 * There is also a problem with LC's and LC Ethernet cards, around 16
 * somewhere.
 ************************************************************************/
#define NENTRIES 32
#define ENTRYDECLARATIONS 																							\
	struct wdsEntry entries[NENTRIES];																		\
	WDSPtr *current=entries;																							\
	short lastC,lastLen;																									\
	int sendCount = IsAUX() ? 5 : MIN(GetRLong(WDS_LIMIT),NENTRIES)
#define ADD(bf,len) \
	do {lastLen = len;if ((lastC=AddWDS(entries,&current,bf,len,sendCount))<0) goto done;} while(0)
#define SEND() do {if (sErr=SendEm(entries,&current)) goto done;} while(0)

/************************************************************************
 * AddWDS - add a string to the current set of WDS buffers
 ************************************************************************/
short AddWDS(WDSPtr entries, WDSPtr *current,Uptr bf,long len,short maxC)
{
	short lastC;
	(*current)->ptr=bf; (*current)->length=len;
	lastC = (bf)[len-1];
	if (len && ++(*current)-entries == maxC-1)
		if (sErr=SendEm(entries,current)) return(-1);
	return(lastC);
}

/************************************************************************
 * SendEm - send the current set of WDS buffers
 ************************************************************************/
short SendEm(WDSPtr entries,WDSPtr *current)
{
	short err=0;
	WDSPtr this;
	long bytes;
	
	CycleBalls();
	for (this=entries;this<*current;this++) bytes+=this->length;
	if (bytes)
	{
		ByteProgress(nil,-bytes,0);
		(*current)->length = 0;
		err=SendWDS(entries);
		*current=entries;
	}
	GiveTime();
	return(err);
}

/************************************************************************
 * SendHeaderLine - send a line of header information to sendmail
 ************************************************************************/
int SendHeaderLine(int header,TEHandle teh)
{
	Str63 label, note;
	UPtr start, stop, end, space, limit;
	int lineLimit = GetRLong(WRAP_LIMIT);
	UHandle safe=nil;
	ENTRYDECLARATIONS;

	if (!(*teh)->teLength && header!=TO_HEAD) return(noErr);/* nothing here */
	
	/*
	 * label the header block
	 */
	GetRString(label,HEADER_STRN+header);
	ADD(label+1,*label);
	
	/*
	 * is it an address header?
	 */
	if (header==TO_HEAD && !(*teh)->teLength)
	{
		GetRString(note,BCC_ONLY);
		ADD(note+1,*note);
		ADD(NewLine+1,*NewLine);
	}
	else if (IsAddressHead(header))
	{
		UHandle addresses;
		UHandle rawAddresses = SuckAddresses((*teh)->hText,(*teh)->teLength,True);
		int charsOnLine=0;
		if (rawAddresses)
		{
			if (**rawAddresses)
			{
				addresses = ExpandAliases(rawAddresses,0,True);
				DisposHandle(rawAddresses);
				if (addresses)
				{
					for (start=LDRef(addresses); *start; start += *start+2)
					{
						if (lineLimit && charsOnLine && *start+charsOnLine>lineLimit)
						{
							ADD(",",1);
							ADD(NewLine+1,*NewLine);
							charsOnLine = 0;
						}
						if (charsOnLine)
							ADD(", ",2);
						else
							ADD(" ",1);
						ADD(start+1,*start);
						charsOnLine += 1+*start;
					}
					ADD(NewLine+1,*NewLine);
					SEND();
				}
				DisposHandle(addresses);
			}
			else
				DisposHandle(rawAddresses);
		}
	}
	else if (header==FROM_HEAD)
	{
		Handle popCanon=nil,returnCanon=nil;
		Str31 sender;
		Str255 buffer;
		static UPtr unverified = " (Unverified)";	/* deliberately code-embedded */
		
		/* send the return address */
		ADD(" ",1);
		ADD(LDRef((*teh)->hText),(*teh)->teLength);
		
		if (*Password)
		{
			/* figure out what the return addr means */
			returnCanon = SuckPtrAddresses(*(*teh)->hText,(*teh)->teLength,False);
			
			/* grab the POP account, and figure out what it means */		
			GetPref(buffer,PREF_POP);
			popCanon = SuckPtrAddresses(buffer+1,*buffer,False);
		}
		
		/* if different or no password, send Sender field */
		if (!UUPCOut && (!POPSecure || !EqualString(LDRef(popCanon),LDRef(returnCanon),False,True)))
		{
			ADD(NewLine+1,*NewLine);	/* finish From: header */
			GetPref(buffer,PREF_POP);
			GetRString(sender,SENDER);
			ADD(sender+1,*sender);		/* send sender line, but not newline */
			if (!UUPCIn) ADD(buffer+1,*buffer);
			if (!POPSecure) ADD(unverified,13);
		}
		ADD(NewLine+1,*NewLine);
		SEND();

		DisposHandle(popCanon);
		DisposHandle(returnCanon);
	}
	else
	{
		if (header==ATTACH_HEAD && (safe=NewHandle((*teh)->teLength)))
			BlockMove(*(*teh)->hText,*safe,(*teh)->teLength);
		/*
		 * send it, a line at a time
		 * prepend a ' ' to each line, ala RFC 822
		 */
		start = LDRef((*teh)->hText);
		stop = start + (*teh)->teLength;
		for (;start<stop;start=end+1)
		{
			limit = start + lineLimit;
			if (stop<limit) limit = stop;
			for (space=end=start;end<limit && *end!='\n'; end++)
				if (*end==' ') space=end;
			if (space>start && end >= limit && limit<stop) end = space;
			ByteProgress(nil,-1,0);
			ADD(" ",1); ADD(start,end-start); ADD(NewLine+1,*NewLine);
		}
	}
	if (current!=entries) SEND();
	if (safe)
	{
		BlockMove(*safe,*(*teh)->hText,(*teh)->teLength);
		DisposHandle(safe);
	}
done:
	UL((*teh)->hText);
	return(sErr); 
}

/************************************************************************
 * SendBodyLines - send the actual body of the message
 *	Don't look at this; it's a mess.
 *	text				Handle to the text to send
 *	length			length of same
 *  offset			offset at which to begin
 *	doWrap			should the text be wrapped?
 *	forceLines	should I force a newline at the end of the text?
 *	lineStarts	pointer to array for determining wrap (may be nil)
 *	nLines			length of same
 *	partial			should I listen to the partial information?
 ************************************************************************/
int SendBodyLines(UHandle  text,long length,long offset,Boolean doWrap,Boolean forceLines,short *lineStarts,short nLines,Boolean partial)
{
	UPtr start;			/* the beginning of the text left to be sent */
	UPtr stop;			/* the end of the entire text block */
	UPtr end;				/* one past the last character of a line of text to be sent
										 for complete lines, this will be a return */
	UPtr space;			/* the last space before end */
	UPtr limit;			/* the point at which the line should be wrapped (if any) */
	int lineLimit;	/* # of chars at which to wrap */
	int quoteLimit;	/* ditto for quoted lines */
	static short quoteLevel;		/* the # of quote chars at start of line */
	Byte suspendChar;						/* the quote character */
	static short partialSize;		/* the size of the last line output, if it
																 was an incomplete line */
	static Boolean softNewline;	/* was the last newline added by us? */
	Str31 scratch;
	short i;
	ENTRYDECLARATIONS;
	
	if (!partial)
	{
		softNewline=False;	/* the caller has told us not to */
		partialSize = 0;		/* bother with partial processing */
	}
	start = LDRef(text)+offset;
	stop = *text + length;

	/*
	 * gather up important info for wrap calculations
	 */
	if (doWrap)
	{
		suspendChar = (GetRString(scratch,QUOTE_PREFIX))[1];
		lineLimit = GetRLong(WRAP_LIMIT);
		quoteLimit = GetRLong(QUOTE_LIMIT);
		
		/*
		 * if this is a new line, count the quote level
		 */
		if (!partialSize)
		{
			for (end = start;*end==suspendChar && end<stop;end++);
			quoteLevel = end-start;
		}
	}
	
	/*
	 * main loop; loop through the buffer, sending one line at a time
	 */
	for (; start<stop; start = end+1)
	{
		/*
		 * Rong wrapping; based on the textedit linestarts array
		 */
		if (doWrap && lineStarts)
		{
			if (nLines<=0) end=stop;		/* a single line */
			else
			{
				nLines--, lineStarts++;			/* point us at the next line */
				end = *text+*lineStarts-1;
			}
		}
		
		/*
		 * normal wrapping; based on line length
		 */
		else
		{
			/* calculate the spot before which we should wrap */
			if (doWrap)
				limit = start + (quoteLevel ? quoteLimit : lineLimit) - partialSize;
			
			/* if we don't want wrapping, or there is less text than the wrap limit */
			if (!doWrap || stop<limit) limit = stop;	/* no need to wrap */
			
			/*
			 * look through the buffer, from start to the calculated line limit
			 * keep track of the last space we see, since it's a potential wrap point
			 * if we find a return, we have a whole line, and can send it
			 */
			if (doWrap)
			{
				for (space=end=start;end<limit && *end!='\n'; end++)
					if (*end==' ' && doWrap) space=end;
				if (end >= limit &&		/* we went over the wrap limit */
					  limit<stop &&			/* it wasn't a bogus wrap limit */
						space>start)			/* and we found a space */
					end = space;				/* Wrap it! */
			}
			else
			{
				for (end=start;end<limit && *end!='\n'; end++);	/* just look for newlines */
			}
			
			/*
			 * make special allowance for lines >wrap limit but < 80
			 */
			if (!softNewline && limit<stop && end-start+partialSize<quoteLimit)
			{
				UPtr nl;
				for (nl=end;nl<start+partialSize+quoteLimit && *nl!='\n';nl++);
				if (*nl=='\n' && nl<stop) end=nl;		/* extend to hard return */
			}
		 
			/* are we adding the newline?  We'll want to know for the next line. */
			if (end<limit) softNewline = *end!='\n';
		}
		
		/*
		 * at this point, start points at the beginning of the line to send,
		 * and end points one character past the end of the line to send
		 */
		
		/* escape initial periods, if need be */ 
		if (!partialSize && !UUPCOut && end>start && *start=='.') ADD(".",1);
		
		/* if there is data to send, send it */
		if (end>start) ADD(start,end-start);
		
		/*
		 * send the newline, unless we've run out of characters and so don't know
		 * if this should be a complete line or not
		 */
		if (forceLines || end<stop) ADD(NewLine+1,*NewLine);
		
		/*
		 * We just put out a line, so we know we're starting fresh for
		 * the next one, if there is a next one
		 */
		partialSize = 0;

		/*
		 * quoted line processing, if there are any chars left
		 */
		if (end<stop)
			/*
			 * if we sent out a complete line, peek at the next line to see how
			 * many quote characters it has
			 */
			if (*end=='\n')
			{
				UPtr p;
				for (p=end+1;*p==suspendChar && p<stop;p++);
				quoteLevel = p-end-1;
			}
			else	/* if we wrapped it, prequote the next line */
			{
				for (i=0;i<quoteLevel;i++) ADD(&suspendChar,1);
				partialSize = quoteLevel;	/* guess we have a partial line after all */
			}
		
		/*
		 * normally, end points at a newline (for complete lines) or space
		 * (for wrapped ones).  So, we normally skip the character end points
		 * to.  However, long solid lines or Rong-wrapped lines might not obey
		 * this behavior; adjust end back by one to make up for the increment
		 * we'll do in just a few cycles...
		 */
		if (end<stop && *end!=' ' && *end!='\n') end--;
	}
	
	/*
	 * all done with that buffer.  If the last character is a newline,
	 * we don't have much to do.  Otherwise, we may (forceLines) wish to
	 * newline-terminate, else we want to remember how long the line
	 * fragment we sent was
	 */
	if (lastC!=NewLine[*NewLine])
	  if (forceLines) ADD(NewLine+1,*NewLine);
		else partialSize = lastLen;
		
	/*
	 * send the last buffer(s)
	 */
	if (current != entries) SEND();
done:
	UL(text);
	return(sErr); 
}

/************************************************************************
 * PrimeProgress - get the progress window started.
 ************************************************************************/
void PrimeProgress(MessType **messH)
{
	Str255 buff;
	
	*buff = 0;
	PCat(buff,*((WindowPeek)(*messH)->win)->titleHandle);
	ByteProgress(buff,0,CountCompBytes(messH));
}


/************************************************************************
 * WannaSend - find out of the user wants to send a dirty window
 ************************************************************************/
int WannaSend(MyWindowPtr win)
{
	Str255 title;
	
	PCopy(title,*win->qWindow.titleHandle);
	return(AlertStr(WANNA_SEND_ALRT,Stop,title));
}

/************************************************************************
 * SendAttachments - send the files the user has attached to his message.
 ************************************************************************/
int SendAttachments(TEHandle teh,Boolean doWrap,Boolean plainText)
{
	short colons[4];
	short onColon;
	Str31 name;
	Str31 volName;
	long id;
	int err;
	int onChar;
	UHandle text = (*teh)->hText;
	
	onColon = 0;
	for (onChar=0;onChar<(*teh)->teLength;onChar++)
		if ((*text)[onChar] == ':')
		{
			colons[onColon] = onChar;
			if (++onColon==sizeof(colons)/sizeof(short))
			{
				BlockMove((*text)+colons[0]+1,volName+1,colons[1]-colons[0]);
				*volName = colons[1]-colons[0];
				id = atoi(LDRef(text)+colons[1]+1);
				BlockMove((*text)+colons[2]+1,name+1,colons[3]-colons[2]-1);
				*name = colons[3]-colons[2]-1;
				UL(text);
				if (plainText && IsText(volName,id,name))
					err = SendPlain(GetMyVR(volName),id,name,doWrap);
				else
					err = SendBinHex(GetMyVR(volName),id,name);
				if (err) return(err);
				onColon = 0;
			}
		}
	return(noErr);
}

/************************************************************************
 * GrabSignature - read the signature file
 ************************************************************************/
void GrabSignature(void)
{
	Str31 name;
	short refN,err,vRef;
	long dirId;
	long bytes;
	

	if (err=GetFileByRef(SettingsRefN,&vRef,&dirId,name))
		FileSystemError(READ_SETTINGS,name,err);
	else if (err=FSHOpen(name,MyVRef,MyDirId,&refN,fsRdPerm))
		FileSystemError(READ_SETTINGS,name,err);
	else
	{
		if (err=GetEOF(refN,&bytes))
			FileSystemError(READ_SETTINGS,name,err);
		else if (bytes)
		{
			if (!(Signature = NuHandle(bytes)))
				WarnUser(MEM_ERR,err=MemError());
			else
			{
				if (err=FSRead(refN,&bytes,LDRef(Signature)))
				{
					FileSystemError(READ_SETTINGS,name,err);
					ZapHandle(Signature);
				}
			}
		}
		FSClose(refN);
	}
}
/************************************************************************
 * SendPlain - send a plain text file
 ************************************************************************/
short SendPlain(short vRef,long dirId,UPtr name,Boolean doWrap)
{
	short refN=0;
	UHandle dataBuffer=nil;
	short dataSize;
	int err;
	short oldVol;
	short newVol;
	long fileSize,sendSize,readSize;
	Str63 scratch;
	Boolean partial = False;
	
	GetVol(scratch,&oldVol);
	if (err=HSetVol(nil,vRef,dirId))
		{FileSystemError(BINHEX_OPEN,name,err); goto done;}
	GetVol(scratch,&newVol);
	
	/*
	 * allocate the buffers
	 */
	dataSize = GetRLong(BUFFER_SIZE);
	if (!(dataBuffer=NuHandle(dataSize)))
		{WarnUser(MEM_ERR,MemError()); goto done;}

	ComposeRString(scratch,PLAIN_PROG_FMT,name);
	PushProgress();
	Progress(0,scratch);
	
	/*
	 * open it
	 */
	if (err = FSHOpen(name,vRef,dirId,&refN,fsRdPerm))
		{FileSystemError(BINHEX_OPEN,name,err); goto done;}
	if (err = GetEOF(refN,&fileSize))
		{FileSystemError(BINHEX_OPEN,name,err); goto done;}
	
	/*
	 * send it
	 */
	ByteProgress(nil,0,fileSize);
	for (;fileSize;fileSize-=readSize)
	{
		readSize=MIN(dataSize,fileSize);
		sendSize = readSize;
		if (err=FSRead(refN,&sendSize,LDRef(dataBuffer)))
			{FileSystemError(BINHEX_READ,name,err); goto done;}
		UL(dataBuffer);
		if (SendBodyLines(dataBuffer,sendSize,0,doWrap,False,nil,0,partial))
			goto done;
		partial = (*dataBuffer)[sendSize-1] != '\n';
	}
	SendTrans(1,NewLine+1,*NewLine);
	
	
done:
	if (refN) FSClose(refN);
	if (dataBuffer) DisposHandle(dataBuffer);
	PopProgress(False);
	SetVol(nil,oldVol);
	return(err);	
} 			

/************************************************************************
 * BuildDateHeader - build an RFC 822 date header
 ************************************************************************/
void BuildDateHeader(UPtr buffer,long seconds)
{
	DateTimeRec dtr;
	long delta = ZoneSecs();
	Boolean negative;
	
	if (delta==-1) {*buffer=0;return;}
	if (seconds)
		Secs2Date(seconds+delta,&dtr);
	else
		GetTime(&dtr);
	if (negative=delta<0) delta *= -1;
	delta /= 60; /* we want minutes */
	ComposeRString(buffer,DATE_HEADER,
											WEEKDAY_STRN+dtr.dayOfWeek,
											dtr.day,
											MONTH_STRN+dtr.month,
											dtr.year,
											dtr.hour/10, dtr.hour%10,
											dtr.minute/10, dtr.minute%10,
											dtr.second/10, dtr.second%10,
											negative ? '-' : '+',
											delta/600,(delta%600)/60,(delta%60)/10,delta%10);
	return;
} 			

/************************************************************************
 * SaveB4Send - grab an outgoing message, saving if necessary
 ************************************************************************/
MessHandle SaveB4Send(TOCHandle tocH,short sumNum)
{
	short which;
	MessHandle messH = (MessHandle)(*tocH)->sums[sumNum].messH;
	
	if (messH && (*messH)->win->isDirty)
	{
		which = WannaSend((*messH)->win);
		if (which == WANNA_SAVE_CANCEL || which==CANCEL_ITEM)
			return (nil);
		else if (which == WANNA_SAVE_SAVE && !SaveComp((*messH)->win))
			return(nil);
		else if (which == WANNA_SAVE_DISCARD)
		{
			(*messH)->win->isDirty = False;
			CloseMyWindow((*messH)->win);
			messH = nil;
		} 	
	}
	if (!messH)
	{
		if (!OpenComp(tocH,sumNum,nil,False)) return(nil);
		messH = (MessHandle)(*tocH)->sums[sumNum].messH;
	}
	return(messH);
}

/************************************************************************
 * TimeStamp - put a time stamp on a message
 ************************************************************************/
void TimeStamp(TOCHandle tocH,short sumNum,uLong when,long delta)
{
	PtrTimeStamp(LDRef(tocH)->sums+sumNum,when,delta);
	UL(tocH);
	CalcSumLengths(tocH,sumNum);
	InvalSum(tocH,sumNum);
	(*tocH)->dirty = True;
}

/************************************************************************
 * PtrTimeStamp - timestamp, but into a sum directly
 ************************************************************************/
void PtrTimeStamp(MSumPtr sum,uLong when,long delta)
{
	sum->seconds = when;
	if (when)
	{
		/* put the date into the summary */
		Str31 d1, d2, d3;
		Str63 fmt;
		long secs = when + delta;
		Boolean neg = delta < 0;
		
		IUTimeString(secs,False,d1);
		IUDateString(secs,shortDate,d2);
		
		if (neg) delta *= -1;
		delta /= 60;	/* minutes*/
		ComposeString(d3,"\p%c%d%d%d%d",neg?'-':'+',
											delta/600,(delta%600)/60,(delta%60)/10,delta%10);

		GetRString(fmt,DATE_SUM_FMT);
		if (d1[2]<'0' || d1[2]>'9') {BlockMove(d1+1,d1+2,*d1+1);++*d1;d1[1]=optSpace;}
		utl_PlugParams(fmt,sum->date,d1,d2,d3,nil);
	}
	else
	  sum->date[0] = 0;
}

