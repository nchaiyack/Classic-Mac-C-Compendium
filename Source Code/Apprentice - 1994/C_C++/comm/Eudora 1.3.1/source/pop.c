#define FILE_NUM 30
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * functions for dealing with a pop 2 server
 ************************************************************************/
#pragma load EUDORA_LOAD
#ifdef	KERBEROS
#include				<krb.h>
#endif
#pragma segment POP

typedef enum
{
	user=1, pass, stat, retr, dele, quit, top, list, apop
} POPEnum;

#define CMD_BUFFER 514
#define PSIZE (UseCTB ? 256 : 4096)
/************************************************************************
 * private routines
 ************************************************************************/
#ifndef KERBEROS
int POPIntroductions(void);
#else
int POPIntroductions(UPtr hostname);
#endif
int POPByeBye(void);
int POPCmd(int cmd, UPtr args);
int POPCmdGetReply(int cmd, UPtr args, UPtr buffer, long *size);
int StartPOP(UPtr serverName, short port);
int EndPOP(void);
int POPGetMessage(long messageNumber,short *gotSome);
int POPCmdError(int cmd,UPtr args,UPtr message);
int PutOutFromLine(short refN,long *fromLen);
int DupHeader(short refN,UPtr buff,long bSize,long offset,long headerSize);
int SplitAndSave(short refN,long estSize);
int WritePartLine(short refN,short partNo);
short DoBody(short refN,long headerSize,long origOffset,char *buf,long bSize,Boolean abomination,long estSize);
int Prr;
Boolean PopConnected;
int FirstUnread(int count);
Boolean HasBeenRead(short msgNum,short count);
void StampPartNumber(MSumPtr sum,short part,short count);
UPtr ExtractStamp(UPtr stamp,UPtr banner);
Boolean GenDigest(UPtr banner,UPtr secret,UPtr digest);

/************************************************************************
 * public routines
 ************************************************************************/
/************************************************************************
 * GetMyMail - the biggie; transfers mail into In mailbox
 ************************************************************************/
short GetMyMail(Boolean quietly)
{
#pragma unused(quietly)
	int messageCount;
	int message,first;
	Str255 scratch;
	Str255 msgname;
	Str63 hostName;
	short port;
	short gotSome = 0;
	TOCHandle tocH;
	short err;
	
	GetPOPInfo(msgname,hostName);
	port = GetRLong(POP_PORT);
	if (StartPOP(hostName,port)==noErr)
	{
#ifdef	KERBEROS
		messageCount = POPIntroductions(hostName);
#else
		messageCount = POPIntroductions();
#endif
		if (messageCount>0 && !Prr)
		{
			ComposeLogR(LOG_RETR,nil,START_POP_LOG,hostName,port,messageCount);
			if (tocH=GetInTOC())
			{
				for (first=message=PrefIsSet(PREF_DONT_DELETE) ?
																						FirstUnread(messageCount) : 1;
							message<=messageCount;
							message++)
				{
					ComposeRString(msgname,POP_STATUS_FMT,
													message-first+1,messageCount-first+1);
					Progress(0,msgname);
					(*tocH)->dirty = True;
					if (POPGetMessage(message,&gotSome)) break;
				}
			}
			else
				Prr = 1;
		}
	}
	if (AttachedFiles) SetHandleBig(AttachedFiles,0);
	err = Prr;
	EndPOP();
	
#ifndef KERBEROS
	if (PrefIsSet(PREF_SAVE_PASSWORD) && *Password &&
	    (!EqualString(Password,GetPref(scratch,PREF_PASS_TEXT),True,True) ||
	     !EqualString(SecondPass,GetPref(scratch,PREF_AUXPW),True,True)))
	{
		ChangeStrn(PREF_STRN,PREF_PASS_TEXT,Password);
		ChangeStrn(PREF_STRN,PREF_AUXPW,SecondPass);
		UpdateResFile(SettingsRefN);
	}
#endif
	return(gotSome);
}

/************************************************************************
 * POPrror - see if there was a POP error
 ************************************************************************/
int POPrror(void)
{
	return(Prr);
}
	
/************************************************************************
 * private routines
 ************************************************************************/
/************************************************************************
 * StartPOP - get connected to the POP server
 ************************************************************************/
int StartPOP(UPtr serverName, short port)
{
	PopConnected=False;
	Prr=ConnectTrans(serverName,port,False);
	return(Prr);
}

/************************************************************************
 * EndPOP - get rid of the POP server
 ************************************************************************/
int EndPOP()
{
	SilenceTrans(True);
	if (!Prr && !CommandPeriod)
	{
		(void) POPByeBye();
		Prr=DisTrans();
	}
	Prr = DestroyTrans();
	return(Prr);
}

#ifdef	KERBEROS
				/* callback fns kerberos uses to read/write on our connection */
static int			kread(char *buf, int len, void *uptr)
{
				int err;
				int ret_len = len;
				
				err = TCPRecvTrans((UPtr) buf, &ret_len);
				if (!err)
								return len;
				else
								return 0;
				}
static int			kwrite(char *buf, int len, void *uptr)
{
				int err;

				err = TCPSendTrans(1, (UPtr) buf, (int) len);
				if (!err)
								return len;
				else
								return 0;
				}
#endif

/************************************************************************
 * POPIntroductions - sniff the POP server's bottom, and vice-versa
 ************************************************************************/
#ifndef KERBEROS
int POPIntroductions(void)
#else
int POPIntroductions(UPtr hostname)
#endif
{
	Str255 buffer;
	Str63 userName;
	Str255 args;
	long size;
	int result = -1;
#ifdef	KERBEROS
				int status;
				KTEXT_ST ticket;
				long authopts;
				char name[ANAME_SZ];
				char instance[INST_SZ];
				char realm[REALM_SZ];
				char hname[256];
#else
	Boolean useAPOP = PrefIsSet(PREF_APOP);
	Str255 digest;	
#endif
				
#ifdef	KERBEROS
				Progress(NoChange,GetRString(buffer,LOGGING_IN));
				GetPOPInfo(userName,args);
				
				PtoCcpy(hname, hostname);
				PtoCcpy(name, userName);
				instance[0] = 0;
				realm[0] = 0;
				if(0 != (status = krb_get_lrealm(realm, 1))) {
								char		message[256];
								
								sprintf(message, "Login failed: %s", krb_err_txt[status]);
								c2pstr(message);
								AlertStr(OK_ALRT,Caution,message);
								Prr = '-';
								goto done;
				}
				
				authopts = 0L;
				status = krb_sendauth(authopts, &ticket, "pop", hname, NULL, NULL,
																												NULL, NULL, NULL, NULL, NULL, NULL, NULL, "VERSION9",
																												kread, kwrite, NULL, name, instance, realm);
				if (status != KSUCCESS) {
								char		message[256];
								
								sprintf(message, "Login failed: %s", krb_err_txt[status]);
								c2pstr(message);
								AlertStr(OK_ALRT,Caution,message);
								Prr = '-';
								goto done;
				}
#endif	

	do
	{
		size = sizeof(buffer)-1;
		Prr = RecvLine(buffer+1,&size);
		if (Prr) goto done;
		buffer[0] = MIN(size,127);
		Progress(NoChange,buffer);
	}
	while (buffer[1]!='+' && buffer[1]!='-');
	
	PopConnected = size && (buffer[1]=='+' || buffer[1]=='-');
	if (buffer[1] != '+')
	{
		Prr = buffer[1];
		POPCmdError(-1,nil,buffer);
		goto done;
	}
	
	if (useAPOP) {useAPOP = GenDigest(buffer,Password,digest);}
	
#ifndef KERBEROS
	Progress(NoChange,GetRString(buffer,LOGGING_IN));
	GetPOPInfo(userName,args);
#endif

	if (useAPOP)
	{
		size = sizeof(buffer)-1;
		PCopy(args,userName);
		PCatC(args,' ');
		PCat(args,digest);
		Prr = POPCmdGetReply(apop,args,buffer,&size);
	}
	else
	{
		size = sizeof(buffer)-1;
		Prr = POPCmdGetReply(user,userName,buffer,&size);
		if (Prr || *buffer != '+')
		{
			if (!Prr) POPCmdError(user,userName,buffer);
			Prr = '-';
			goto done;
		}
	
#ifndef KERBEROS
		PCopy(args,Password);
#else
		PCopy(args, "\parbitrary");
#endif
	
		size = sizeof(buffer)-1;
		Prr = POPCmdGetReply(pass,args,buffer,&size);
	}
	if (Prr || *buffer != '+')
	{
		if (!Prr)
		{
			POPSecure = False;
			POPCmdError(pass,nil,buffer);
#ifndef KERBEROS
			InvalidatePasswords(False,True);
#endif
		}
		Prr = '-';
		goto done;
	}
	POPSecure = True;
	
#ifndef POPSECURE
	Progress(NoChange,GetRString(buffer,LOOK_MAIL));
#endif
	size = sizeof(buffer)-1;
	Prr = POPCmdGetReply(stat,nil,buffer,&size);
	if (Prr || *buffer != '+')
	{
		if (!Prr)  POPCmdError(stat,nil,buffer);
		Prr = '-';
		goto done;
	}
	
	result = atoi(buffer+3);
	done:
	return(result);
}

/************************************************************************
 * POPByeBye - tell the POP server we're leaving
 ************************************************************************/
int POPByeBye(void)
{
	char buffer[CMD_BUFFER];
	long size=sizeof(buffer);
	if (!PopConnected) return(noErr);
	Prr = POPCmdGetReply(quit,nil,buffer,&size);
	return (Prr || *buffer != '+');
}

/************************************************************************
 * POPCmd - Send a command to the POP server
 ************************************************************************/
int POPCmd(int cmd, UPtr args)
{
	char buffer[CMD_BUFFER];

	GetRString(buffer,POP_STRN+cmd);
	if (args && *args)
		PCat(buffer,args);
#ifdef DEBUG_COMM
	Progress(NoBar,buffer);
#endif DEBUG_COMM
	PCat(buffer,NewLine);
	
	Prr=SendTrans(1,buffer+1,*buffer);
	
	return(Prr);
}

/************************************************************************
 * POPCmdGetReply - send a POP command and get a reply
 ************************************************************************/
int POPCmdGetReply(int cmd, UPtr args, UPtr buffer, long *size)
{ 
	long rSize;
	if (Prr=POPCmd(cmd,args)) return(Prr);				/* error in transmission */
	do
	{
		rSize = *size;
		Prr = RecvLine(buffer,&rSize);
	}
	while (!Prr && *buffer!='+' && *buffer!='-');
	*size = rSize;
	return(Prr);
}

/************************************************************************
 * POPGetMessage - get a message from the POP server
 ************************************************************************/
int POPGetMessage(long messageNumber,short *gotSome)
{
	Str255 args;
	char buffer[CMD_BUFFER];
	long size = sizeof(buffer);
	long msgsize=0;
	char *spot;
	short count;
		
	NumToString((long)messageNumber,args);
	
	if (Prr=POPCmdGetReply(list,args,buffer,&size)) return(Prr);
	if (*buffer != '+')
	{
		Prr = *buffer;
		POPCmdError(list,args,buffer);
		return(Prr);
	}
	strtok(buffer," "); 																	/* skip ok */
	strtok(nil," ");																			/* skip message id */
	if (spot=strtok(nil," \n")) msgsize = atoi(spot); 		/* read message size */
	
  size = sizeof(buffer);
	if (PrefIsSet(PREF_NO_BIGGIES) && msgsize>GetRLong(BIG_MESSAGE))
	{
	  NumToString(GetRLong(BIG_MESSAGE_FRAGMENT),buffer);
		PCatC(args,' ');
		PCat(args,buffer);
		msgsize = -1;	/* let everyone down the line know what's going down */
		Prr = POPCmdGetReply(top,args,buffer,&size);
		NoAttachments = True;	/* don't do BinHex */
	}
	else
	{
	  Prr=POPCmdGetReply(retr,args,buffer,&size);
		NoAttachments = False;
	}
	if (Prr) return(Prr);
	if (*buffer!='+')
	{
		POPCmdError(retr,args,buffer);
		return(1);
	}
	BadBinHex = False;
	BeginHexBin();
	if (!AttachedFiles) AttachedFiles=NuHandle(0);
	SetHandleBig(AttachedFiles,0);
	count=POPMessageBody(msgsize);
	SetHandleBig(AttachedFiles,0);
	EndHexBin();
	SaveAbomination(nil,0);
	if (!Prr && !CommandPeriod && !PrefIsSet(PREF_DONT_DELETE) && !NoAttachments &&
	    (!BadBinHex || ReallyDoAnAlert(BAD_HEXBIN_ALRT,Stop)==BAD_HEXBIN_IGNORE))
	{
		NumToString(messageNumber,args);
		size = sizeof(buffer);
		Prr=POPCmdGetReply(dele,args,buffer,&size);
	}

	if (!Prr) (*gotSome)+=count;
	return(Prr);
}

/************************************************************************
 * POPCmdError - report an error for an POP command
 ************************************************************************/
int POPCmdError(int cmd, UPtr args, UPtr message)
{
	Str255 theCmd;
	Str255 theError;
	int err;

	GetRString(theCmd,POP_STRN+cmd);
	if (args && *args)
		PCat(theCmd,args);
	strcpy(theError+1,message);
	*theError = strlen(theError+1);
	if (theError[*theError]=='\012') (*theError)--;
	if (theError[*theError]=='\015') (*theError)--;
	MyParamText(theCmd,theError,"\pPOP","");
	err = ReallyDoAnAlert(PROTO_ERR_ALRT,Note);
	return(err);
}

/************************************************************************
 * POPMessageBody - read in the body of a message
 ************************************************************************/
int POPMessageBody(long estSize)
{
	UPtr text=nil;
	TOCType **tocH;
	MSumType sum;
	long eof,chopHere;
	Str255 name;
	short count=0,part;
	
	/*
	 * make the message summary
	 */
	WriteZero(&sum,sizeof(MSumType));

	/*
	 * grab the "In" box
	 */
	if (!(tocH=GetInTOC()))
		return(1);
	PCopy(name,(*tocH)->name);
	
	Prr = BoxFOpen(tocH);
	if (Prr) {FileSystemError(OPEN_MBOX,name,Prr); goto done;}

	eof = FindTOCSpot(tocH,estSize);

	Prr = SetFPos((*tocH)->refN, fsFromStart, eof);
	if (Prr) {FileSystemError(WRITE_MBOX,name,Prr); goto done;}
		
	count = SplitAndSave((*tocH)->refN,estSize);
	
done:
	if (!Prr && !GetFPos((*tocH)->refN,&chopHere))
		SetEOF((*tocH)->refN,chopHere);
	
	Prr = BoxFClose(tocH) || Prr;
	if (Prr || !count) return(0);
	
	/*
	 * now, read it back from the file
	 */ 
	if (Prr=OpenLine((*tocH)->vRef,(*tocH)->dirId,name))
		 {FileSystemError(READ_MBOX,name,Prr);return(0);}
	if (Prr=SeekLine(eof)) return(FileSystemError(READ_MBOX,name,Prr));
	
	ReadSum(nil,False);
	for (part=1;part<=count;part++)
	{
		Prr = !ReadSum(&sum,False);
		TransLitString(sum.from);
		TransLitString(sum.subj);
		if (!sum.seconds) sum.seconds = GMTDateTime();
		if (count>1) StampPartNumber(&sum,part,count);
		if (Prr || (Prr=!SaveMessageSum(&sum,tocH))) break;
	}
	ReadSum(nil,False);
			
	CloseLine();
	if (Prr)
	{
		WarnUser(READ_MBOX,Prr);
		return(0);
	}

	InvalSum(tocH,(*tocH)->count-1);
	if (!PrefIsSet(PREF_CORVAIR))
	{
		Prr = WriteTOC(tocH);
		FlushVol(nil,MyVRef);
	}
	MakeMessTitle(name,tocH,(*tocH)->count-count);
	ComposeLogR(LOG_RETR,nil,MSG_GOT,name,count);
  return(count);
}

/************************************************************************
 * SplitAndSave - read a message, (possibly) splitting it into parts and
 * saving it.
 ************************************************************************/
int SplitAndSave(short refN,long estSize)
{
	long origOffset;
	long headerSize;
	long lastProgress=0;
	Str255 buf;
	long size;
	Boolean wasNl = False;
	UPtr writeHere, writeEnd;
	short count=0;
	Str127 abomination;
	Boolean abominable=False;
	Str31 status;
	Boolean foundStatus = False;
	
	if (estSize < 0) GetRString(status,STATUS);
	
	GetRString(abomination,ABOMINATION);
	if (estSize>0) ByteProgress(nil,0,estSize);
	GetFPos(refN,&origOffset);
	
	if (Prr=PutOutFromLine(refN,&headerSize)) return(Prr);
	/*
	 * read the header
	 */
	for (size=sizeof(buf)-1; !(Prr=RecvLine(buf+1,&size)); size=sizeof(buf)-1)
	{
		writeHere = buf+1;
		writeEnd = writeHere+size-1;

		/*
		 * check for embedded From lines
		 */
		if (IsFromLine(buf+1)){long one=1;(void)FSTabWrite(refN,&one,">");}
		
		/*
		 * check for abominations
		 */
		if (!striscmp(abomination+1,buf+1)) abominable=True;
		/*
		 * progress
		 */
		if ((lastProgress+=size) > PSIZE)
		{
			if (estSize) ByteProgress(nil,-lastProgress,0);
			lastProgress = 0;
		}
				
		/*
		 * check for end of message
		 */
		if (wasNl && size==2 && buf[1]=='.')
		{
			if (Prr=FSTabWrite(refN,&size,"\n\n")) FileSystemError(WRITE_MBOX,"",Prr);
			goto done;
		}

		/*
		 * check for escaped periods
		 */
		if (wasNl && buf[1]=='.')
			writeHere++;

		/*
		 * check for continuations, but only if the last "line" ended with
		 * a newline; if it didn't, we're just continuing anyway
		 */
		if (wasNl)
			if (size>1 && IsWhite(buf[1]))
			{
				while(IsWhite(*writeHere)) writeHere++; /* skip whitespace */
				writeHere--;										/* well, not ALL of it... */
			}
			else
				*--writeHere = '\n';		/* we need a newline for the previous line */
		
		/*
		 * remove a trailing newline, in case the next line is a continuation
		 * (and curse RFC 822)
		 */
		if (wasNl= *writeEnd=='\n') writeEnd--;
		
		/*
		 * is it a status header?  Do we care?
		 */
		if (estSize < 0 && wasNl && !foundStatus && writeHere < writeEnd)
		{
			writeEnd[1] = 0;
			foundStatus = !striscmp(writeHere+1,status+1);
		}
		
		/*
		 * now we can write it out...
		 */
		size = writeEnd-writeHere+1;
		if (Prr=FSTabWrite(refN,&size,writeHere))
		{
			FileSystemError(WRITE_MBOX,"",Prr);
			goto done;
		}
		
		/*
		 * was that the end of the header, by the way?
		 */
		if (size==1 && *writeHere=='\n' ||
				size==2 && *writeEnd=='\n' && *writeHere=='\n')
		{
			size = 1;
			if (Prr=FSTabWrite(refN,&size,"\n"))
			{
				FileSystemError(WRITE_MBOX,"",Prr);
				goto done;
			}
			break;
		}
	}
	
	/*
	 * at this point, we've saved the header (good for us, eh?)
	 * now, go save the body
	 */
	GetFPos(refN,&headerSize);
	count=DoBody(refN,headerSize-origOffset,origOffset,buf,sizeof(buf),abominable,estSize);
done:
	return(foundStatus ? 0 : count);
} 

/************************************************************************
 * DoBody - read the body of a message from a pop-3 server
 ************************************************************************/
short DoBody(short refN,long headerSize,long origOffset,char *buf,long bSize,Boolean abomination,long estSize)
{
	long writeCount;
	Str63 beginHex;
	UPtr writeHere,writeEnd;
	HexBinStates amHexing = HexDone;
	AbStates snowMan = AbDone;
	Boolean wasNl = True;
	long messageSize=headerSize;
	long lastProgress=0;
	long fragSize=GetRLong(FRAGMENT_SIZE);
	long splitThresh=GetRLong(SPLIT_THRESH);
	long hexOffset;
	short partNo = 0;
	long size;
	long oTicks = TickCount();

	GetRString(beginHex,BINHEX);	
	for (size=bSize; !(Prr=RecvLine(buf,&size)); size=bSize)
	{
		writeHere = buf;
		writeEnd = buf + size-1;
		
		/*
		 * check for embedded From lines
		 */
		if (IsFromLine(buf)){long one=1;(void)FSTabWrite(refN,&one,">");}
		
		/*
		 * progress
		 */
		if ((lastProgress+=size) > PSIZE)
		{
			if (estSize>0 || amHexing!=HexDone || snowMan!=AbDone)
				ByteProgress(nil,-lastProgress,0);
			lastProgress = 0;
		}
		
		/*
		 * check for end of message
		 */
		if (wasNl && size==2 && writeHere[0]=='.') break;

		/*
		 * check for periods
		 */
		if (wasNl && writeHere[0]=='.')
			writeHere++;
			
		/*
		 * does this line end in a newline?
		 */
		wasNl = *writeEnd=='\n';
		
		/*
		 * do some autohexing
		 */
		if (!NoAttachments) switch (amHexing)
		{
			case HexDone:
				if (size>=*beginHex && !strncmp(beginHex+1,writeHere,*beginHex))
				{
					amHexing = NotHex;
					GetFPos(refN,&hexOffset);  /* save binhex start */
					DontTranslate = True;
				}
				break;
			case NotHex:
			case CollectName:
			case CollectInfo:
				amHexing = SaveHexBin(writeHere,writeEnd-writeHere+1,estSize);
				if (amHexing>CollectInfo && amHexing!=HexDone)
				{
					SetFPos(refN,fsFromStart,hexOffset);	/* toss the saved bits */
					SetEOF(refN,hexOffset);
					goto loop;	/* do NOT save the line into the message proper */
				}
				break;
			default:
				amHexing = SaveHexBin(writeHere,writeEnd-writeHere+1,estSize);
				goto loop;	/* do NOT save the line into the message proper */
		}
		
		if (!NoAttachments && abomination)
			switch(snowMan)
					
			{
				case AbDone:
					if (IsAbLine(writeHere,size))
					{
						snowMan = NotAb;
						GetFPos(refN,&hexOffset);  /* save binhex start */
						DontTranslate = True;
					}
					break;
				case AbHeader:
				case AbName:
					snowMan = SaveAbomination(writeHere,writeEnd-writeHere+1);
					if (snowMan>AbName && snowMan!=AbDone)
					{
						SetFPos(refN,fsFromStart,hexOffset);	/* toss the saved bits */
						SetEOF(refN,hexOffset);
						goto loop;	/* do NOT save the line into the message proper */
					}
					break;
				default:
					snowMan = SaveAbomination(writeHere,writeEnd-writeHere+1);
					goto loop;	/* do NOT save the line into the message proper */
			}
		
		/*
		 * are we over the limit?
		 */
		if ((estSize>splitThresh || estSize<messageSize) && messageSize > fragSize)
		{
			Str255 buf2; short b2size=sizeof(buf2);
			writeCount = 1; 	/* make sure we have newline */
			if (Prr=FSTabWrite(refN,&writeCount,"\n")) return(Prr);
			if (Prr=DupHeader(refN,buf2,b2size,origOffset,headerSize)) return(Prr);
			/*if (Prr=WritePartLine(refN,++partNo)) return(Prr);*/
			++partNo;
			messageSize = headerSize;
		}
		
		/*
		 * save the data
		 */
		writeCount = writeEnd-writeHere+1;
		if (Prr=FSTabWrite(refN,&writeCount,writeHere))
			return(FileSystemError(WRITE_MBOX,"",Prr));
		messageSize += writeCount;
		loop:
			if (amHexing==HexDone && snowMan==AbDone) DontTranslate = False;
	}
	if (!Prr && (writeCount=GetHandleSize(AttachedFiles)))
	{
		Prr=FSTabWrite(refN,&writeCount,LDRef(AttachedFiles));
		UL(AttachedFiles);
	}
	if (!Prr && estSize < 0)
	{
	  Str255 msg;
		GetRString(msg,BIG_MESSAGE_MSG);
		writeCount = *msg;
		Prr=FSTabWrite(refN,&writeCount,msg+1);
	}
  if (Prr) return(FileSystemError(WRITE_MBOX,"",Prr));
	Progress(100,nil);
	return(Prr?partNo : partNo+1);
}

/************************************************************************
 *
 ************************************************************************/
int WritePartLine(short refN,short partNo)
{
	Str63 partLine;
	long partLen;
	
	ComposeRString(partLine,PART_FMT,partNo);
	partLen = *partLine;
	if (Prr=FSTabWrite(refN,&partLen,partLine+1))
		return(FileSystemError(WRITE_MBOX,"",Prr));
	return(noErr);
}

/************************************************************************
 *
 ************************************************************************/
int PutOutFromLine(short refN,long *fromLen)
{
	Str255 fromLine;
	long len;
	
	*fromLen = len = SumToFrom(nil,fromLine);
	if (Prr=FSTabWrite(refN,&len,fromLine))
		return(FileSystemError(WRITE_MBOX,"",Prr));
	return(noErr);
}

/************************************************************************
 *
 ************************************************************************/
int DupHeader(short refN,UPtr buff,long bSize,long offset,long headerSize)
{
	long currentOffset;
	long readBytes,writeBytes;
	long copied;
	
	if (Prr=GetFPos(refN,&currentOffset))
		return(FileSystemError(READ_MBOX,"",Prr));
	for (copied=0; copied<headerSize; copied += readBytes)
	{
		if (Prr=SetFPos(refN,fsFromStart,offset+copied))
			return(FileSystemError(READ_MBOX,"",Prr));
		readBytes = bSize < headerSize-copied ? bSize : headerSize-copied;
		if (Prr=FSRead(refN,&readBytes,buff))
			return(FileSystemError(READ_MBOX,"",Prr));
		if (Prr=SetFPos(refN,fsFromStart,currentOffset))
			return(FileSystemError(WRITE_MBOX,"",Prr));
		writeBytes = readBytes;
		if (Prr=FSZWrite(refN,&writeBytes,buff))
			return(FileSystemError(WRITE_MBOX,"",Prr));
		currentOffset += writeBytes;
	}
	return(noErr);
}

/************************************************************************
 * FirstUnread - find the first unread message
 *	 We do try to be clever about it.  If 
 ************************************************************************/
int FirstUnread(int count)
{
	short first, last, on;
	static short lastCount=0;
	Boolean hasBeen;
	
	on = lastCount;
	lastCount = count;

#define SETHASBEEN(o,c)\
	do {hasBeen=HasBeenRead(o,c);if (CommandPeriod) return(c+1);} while (0) 
	if (PrefIsSet(PREF_NO_BIGGIES))
	{
		/* Heuristics */
		if (on && on<=count)
		{
			SETHASBEEN(on,count);
			if (hasBeen)
			{
				SETHASBEEN(on+1,count);
				if (!hasBeen) return(on+1);
			}
		}
		SETHASBEEN(count,count);
		if (hasBeen) return(count+1);
		if (count==1) return(1);
		
		/* search... */
		for (on=count-1;on;on--)
		{
			SETHASBEEN(on,count);
			if (hasBeen) break;
		}
		return(on+1);
	}
	else
	{
		first = 1;
		last = count;
		/*
		 * try to cut the search short via heuristics
		 */
		if (on && on<=count)
		{
			SETHASBEEN(on,count);
			if (hasBeen)
			{
				if (on<count)
				{
					SETHASBEEN(++on,count);
					if (!hasBeen) return(on);
				}
				first = on+1;
			}
			else
				last = on-1;
		}
		else
		{
			SETHASBEEN(count,count);
			if (hasBeen) return(count+1);
			SETHASBEEN(1,count);
			if (count==1 || !hasBeen) return(1);
			last=count-1;
			first = 2;
			on = count;
			hasBeen = False;
		}
			
		/*
		 * hi ho, hi ho, it's off to search we go
		 */
		while (first<=last)
		{
			on = (first+last)/2;
			SETHASBEEN(on,count);
			if (hasBeen) first=on+1;
			else last=on-1;
		}
		if (!hasBeen)
			return(on);
		else
			return(on+1);
	}
}

/************************************************************************
 * HasBeenRead - has a particular message been read?
 * look for a "Status:" header; if it's "Status: R<something>", message
 * has been read
 ************************************************************************/
Boolean HasBeenRead(short msgNum,short count)
{
	Str127 scratch;
	Boolean unread=False, statFound=False;
	Str31 terminate;
	Str31 status;
	UPtr cp;
	long size;

	if (msgNum>count) return(0);
	Progress((msgNum*100)/count,GetRString(scratch,FIRST_UNREAD));
	Pause(60);
	GetRString(terminate,ALREADY_READ);
	GetRString(status,STATUS);
	NumToString(msgNum,scratch);
	PLCat(scratch,1);
	POPCmd(top,scratch);
	for (size=sizeof(scratch);
				!(Prr=RecvLine(scratch,&size)) &&
				!(scratch[0]=='.' && scratch[1]=='\n');
				size=sizeof(scratch))
		if (!unread && !statFound && !striscmp(scratch,status+1))
		{
			statFound = True;
			for (cp=scratch;cp<scratch+size;cp++)
			{
				if (*cp==':')
				{
					for (cp++;cp<=scratch+size-*terminate;cp++)
						if (!striscmp(cp,terminate+1)) break;
					unread = cp> scratch+size-*terminate;
					break;
				}
			}
		}
	return(statFound && !unread);
}

/************************************************************************
 * StampPartNumber - put the part number on a mail message
 ************************************************************************/
void StampPartNumber(MSumPtr sum,short part,short count)
{
	char *spot;
	short i, digits, len;
	
	for (i=count,digits=0;i;i/=10,digits++);
	len = 2*digits+2;
	spot=sum->subj+MIN(*sum->subj+len,sizeof(sum->subj)-1);
	*sum->subj = spot-sum->subj;
	for (i=digits;i;i--,count/=10) *spot-- = '0'+count%10;
	*spot-- = '/';
	for (i=digits;i;i--,part/=10) *spot-- = '0'+part%10;
	*spot = ' ';
}
	
/************************************************************************
 * AddAttachNote - note that we've attached a file
 ************************************************************************/
void AddAttachNote(short wdRef, UPtr name, long creator, long type)
{
	Str255 fileAndFolder;
	Str31 folderName;
	Str15 typeString, creatorString;
	
	BlockMove(&creator,creatorString+1,4);
	BlockMove(&type,typeString+1,4);
	*creatorString = *typeString = 4;
	GetDirName(nil,wdRef,GetMyDirID(wdRef),folderName);
	ComposeRString(fileAndFolder,FILE_FOLDER_FMT,
								 folderName,name,typeString,creatorString); 		
	PtrAndHand(fileAndFolder+1,AttachedFiles,*fileAndFolder);
}

/************************************************************************
 * AddAttachInfo - attach a note about problems with the enclosure
 ************************************************************************/
void AddAttachInfo(int theIndex, int result)
{
	Str255 theMessage;
	
	ComposeString(theMessage,"\p%r%d\n", theIndex, result);
	PtrAndHand(theMessage+1,AttachedFiles,*theMessage);
}


#ifdef POPSECURE
/************************************************************************
 * VetPOP - make sure the user's POP account is ok.
 ************************************************************************/
short VetPOP(void)
{
	Str255 user, host;
	short port;
	short err;
	
	if (UUPCIn && !UUPCOut) {WarnUser(UUPC_SECURE,0);return(1);}
	GetPOPInfo(user,host);
	port = GetRLong(POP_PORT);
#ifdef DEBUG
	if (BUG12) port=40110;
#endif DEBUG
	if ((err=StartPOP(host,port))==noErr)
	{
		(void) POPIntroductions();
		if (Prr) err=Prr;
	}
	EndPOP();
	if (UseCTB && !err) CTBNavigateSTRN(NAVMID);
	return(err);
}
#endif

#ifndef KERBEROS
#pragma segment MD5
/************************************************************************
 * GenDigest - generate a digest for APOP
 ************************************************************************/
Boolean GenDigest(UPtr banner,UPtr secret,UPtr digest)
{
	Str255 stamp;
	MD5_CTX md5;
	static char hex[]="0123456789abcdef";
	short i;
	
	if (!*ExtractStamp(stamp,banner)) return(False);
	
	MD5Init(&md5);
	MD5Update(&md5,stamp+1,*stamp);
	MD5Update(&md5,secret+1,*secret);
	MD5Final(&md5);
	
	for (i=0;i<sizeof(md5.digest);i++)
	{
		digest[2*i+1] = hex[(md5.digest[i]>>4)&0xf];
		digest[2*i+2] = hex[md5.digest[i]&0xf];
	}
	digest[0] = 2*sizeof(md5.digest);
	return(True);
}

/************************************************************************
 * ExtractStamp - grab the timestamp out of a POP banner
 ************************************************************************/
UPtr ExtractStamp(UPtr stamp,UPtr banner)
{
	UPtr cp1,cp2;
	
	*stamp = 0;
	banner[*banner+1] = 0;
	if (cp1=strchr(banner+1,'<'))
		if (cp2=strchr(cp1+1,'>'))
		{
			*stamp = cp2-cp1+1;
			strncpy(stamp+1,cp1,*stamp);
		}
	return(stamp);
}
#endif
