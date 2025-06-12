#define FILE_NUM 47
#pragma load EUDORA_LOAD
Boolean SkipToFromLine(void);

/************************************************************************
 * Incoming from UUPC
 *	- We put the pathname in the POP account, preceeded by a !.
 *	- Then, it's just a matter of sicking POPMessageBody on each message.
 *	--- Well, we make a new RecvLine that returns ".\n" for EOF or
 *			a uucp envelope; that helps.
 *	--- We also have the luxury of being able to back up and suck in raw
 *			binhex/uudecode if the conversion fails
 ************************************************************************/
#pragma segment UUPCIn
extern int Prr;

/************************************************************************
 * GetUUPCMail - read mail from the named mailbox
 ************************************************************************/
short GetUUPCMail(Boolean quietly)
{
#pragma unused(quietly)
	Str255 mailpath;
	short err;
	short anyErr = 0;
	short gotSome = 0;
	long oldPos;
	TOCHandle inTocH;
	Boolean foundOne;
	short count;
	
	/*
	 * get the pathname of the maildrop
	 */
	GetPref(mailpath,PREF_POP);
	BlockMove(mailpath+2,mailpath+1,*mailpath-1);
	(*mailpath)--;
	Prr = 0;
	
	CurTrans = UUPCTrans;
	
	/*
	 * open it
	 */
	if (err=FSOpen(mailpath,0,&DropRefN))
		{FileSystemError(READ_MBOX,mailpath,err);goto done;}
	
	/*
	 * now, grab messages
	 */
	for (foundOne=SkipToFromLine();foundOne;)
	{
		GetFPos(DropRefN,&oldPos);
		Progress(NoChange,ComposeRString(mailpath,UUPC_COPY,gotSome+1));
		BadBinHex = False;
		BeginHexBin();
		if (!AttachedFiles) AttachedFiles=NuHandle(0);
		SetHandleBig(AttachedFiles,0);
		count=POPMessageBody(0);
		{extern int Prr; err = Prr;}
		SetHandleBig(AttachedFiles,0);
		EndHexBin();
		SaveAbomination(nil,0);
		if (!err && !BadBinHex) gotSome++;
		anyErr |= err;
		if (BadBinHex)
		{
			SetFPos(DropRefN,fsFromStart,oldPos);
			NoAttachments = True;
			inTocH = GetInTOC();
			DeleteMessage(inTocH,(*inTocH)->count-1);
			continue;
		}
		else
		{
			GetFPos(DropRefN,&oldPos);
			NoAttachments = False;
		}
		foundOne = SkipToFromLine();
	}
	anyErr |= Prr;
done:
	if (DropRefN)
	{
		if (!PrefIsSet(PREF_DONT_DELETE) && 
				(!anyErr || ReallyDoAnAlert(CLEAR_DROP_ALRT,Normal)==1))
			SetEOF(DropRefN,0);
		FSClose(DropRefN);
		DropRefN=0;
	}
	NoAttachments = False;
	CurTrans = UseCTB ? CTBTrans : TCPTrans;
	return(gotSome);
}

/************************************************************************
 * UUPCRecvLine - read a line at a time from the maildrop.	Returns ".\n"
 * at the ends of messages.
 ************************************************************************/
int UUPCRecvLine(UPtr buffer, long *size)
{
	static Boolean wasFrom;
	static Boolean wasNl=True;
	short err;
	IOParam iop;
	
	if (MiniEvents()) return(userCancelled);
	if (!buffer) return(wasFrom);
	wasFrom=False;
	(*size)--;
	
	iop.ioCompletion=nil;
	iop.ioRefNum=DropRefN;
	iop.ioBuffer = buffer;
	iop.ioReqCount = *size;
	iop.ioPosMode = 1<<7 | ('\n'<<8);
	iop.ioPosOffset = 0;
	iop.ioActCount = 0;
	err=PBRead(&iop,False);
	if (iop.ioActCount) err=0;
	*size = iop.ioActCount;
	buffer[*size] = 0;
	if (!DontTranslate && TransIn)
		{Uptr cp; for(cp=buffer;cp<buffer+*size;cp++) *cp = TransIn[*cp];}
	if (err && err!=eofErr)
		{FileSystemError(READ_MBOX,"",err);}
	if (!err || err==eofErr)
	{
		if (err || wasNl&&(wasFrom=IsFromLine(buffer)))
		{
			*size = 2;
			buffer[0]='.'; buffer[1]='\n';
		}
		else if (wasNl && *buffer=='.')
		{
			BlockMove(buffer,buffer+1,*size);
			(*size)++;
			*buffer = '.';
		}
		buffer[*size] = 0;
		err = noErr;
	}
	wasNl = err || buffer[*size-1]=='\n';
	return(err);
}

/************************************************************************
 * SkipToFromLine - skip to the next envelope.	Also gives time to others,
 * and keeps track of whether or not we last read a From line.
 ************************************************************************/
Boolean SkipToFromLine(void)
{
	Str255 buffer;
	long eof, atNow, size;
	short err;
	EventRecord event;
	
	if (WNE(everyEvent,&event,0))
	{
		(void) MiniMainLoop(&event);
		if (CommandPeriod) {Prr = userCancelled; return(False);}
	}
	if ((err=GetEOF(DropRefN,&eof)) || (err=GetFPos(DropRefN,&atNow)))
	{
		FileSystemError(READ_MBOX,GetPref(buffer,PREF_POP),err);
		Prr = err;
		return(False);
	}
	if (eof) Progress((atNow*100)/eof,nil);
	if (!eof || atNow==eof) return(False);
	if (RecvLine(nil,nil)) return(True);
	do
	{
		size = sizeof(buffer);
		if (err=RecvLine(buffer,&size))
		{
			FileSystemError(READ_MBOX,GetPref(buffer,PREF_POP),err);
			Prr = err;
			return(False);
		}
	}
	while (size!=2 || *buffer!='.');
	if (eof) Progress((atNow*100)/eof,nil);
	
	return(True);
}

#pragma segment UUPCOut
/************************************************************************
 * Ok; sending mail is more complicated, but it's easier to do
 * SMTP server item has to have the following info, ! separated:
 *	- the name of our mac
 *	- the name of the remote system (optional, ignored if present)
 *	- the path to the spool directory
 *	- our user name
 *	- our current sequence number
 *	ie, !mymac!relay!spoolvol:spooldir:!username!0000
 *	- We generate a sequence number, 4 digits (####)
 *	- We deposit the mail in D.relay0####, newline-separated
 *	- In D.mymac0####, we put commands for the UUCP system, newline-separated
 *	--- U username mymac				; this is us
 *	--- F D.mymac0####					; this is the mail file, on the remote system
 *	--- I D.mymac0####					; and we're using it for input
 *	--- C rmail <recip-list>		; the command; rmail and recipients as args
 *
 *	By declaring our own SendTrans, we actually can use TransmitMessage
 *	from sendmail.c, which is tres nice.
 *
 *	There are some race conditions in creating the files.  So what?
 ************************************************************************/

typedef struct
{
	Str255 spoolPath;
	Str15 myMac;
	unsigned char relay[9];
	Str15 userName;
	short sequence;
	short refNs[2];
	Str255 files[2];
	Str31 fileNames[2];
	short oldSeq;
	MessHandle messH;
} UUXBlock,*UUXPtr,**UUXHandle;
UUXHandle UUX;
#define SpoolPath (*UUX)->spoolPath
#define MyMac (*UUX)->myMac
#define Relay (*UUX)->relay
#define UserName (*UUX)->userName
#define Sequence (*UUX)->sequence
#define MailRefN (*UUX)->refNs[0]
#define XRefN (*UUX)->refNs[1]
#define MailFile (*UUX)->files[0]
#define XFile (*UUX)->files[1]
#define Dmymac (*UUX)->fileNames[0]
#define Xmymac (*UUX)->fileNames[1]
#define OldSeq (*UUX)->oldSeq
#define MessH (*UUX)->messH

void UUPCGenFilenames(void);
Boolean UUPCBadFilenames(void);
int UUPCMakeFiles(void);
void UUPCKillFiles(void);
void UUPCCloseFiles(void);
int UUPCOpenFiles(void);
int UUPCWriteXFile(void);
int UUPCWriteMailFile(void);
void UUPCGetNewline(UPtr newline,UPtr buffer);
/************************************************************************
 * UUPCPrime - get ready
 ************************************************************************/
int UUPCPrime(UPtr server)
{
	UPtr bangs[6];	/* these will point to exclams in the server text */
	UPtr *bang;
	short err;
	short seq;
	
	/*
	 * count the bangs, shall we?
	 */
	WriteZero(bangs,sizeof(bangs));
	bangs[0] = server+1;	/* first one */
	for (bang=bangs+1;bang<bangs+sizeof(bangs)/sizeof(UPtr);bang++)
	{
		*bang = strchr(bang[-1]+1,'!');	/* look for next exclam */
		if (!*bang) break;
	}
	
	if (!bangs[3]) {WarnUser(UUPC_WRONG_SMTP,0);return(1);}	/* not enough */
	if (!bangs[4])
	{
		/* relay not included; insert a pretend relay */
		BlockMove(bangs+1,bangs+2,3*sizeof(UPtr));
	}
	bangs[5] = server+*server+1;	/* end of the string */

	/*
	 * allocate some space
	 */
	if (!UUX) UUX=NewZH(UUXBlock);
	if (!UUX) {WarnUser(MEM_ERR,err=MemError());return(err);}
	
	/*
	 * copy values into it
	 */
#define MOVE(to,b1,b2) do{ \
	*to=MIN(sizeof(to)-2,bangs[b2]-bangs[b1]-1); to[*to+1]=0; \
	BlockMove(bangs[b1]+1,to+1,*to); } while (0)
	
	MOVE(MyMac,0,1);
	MOVE(SpoolPath,2,3);
	MOVE(UserName,3,4);
	seq = atoi(bangs[4]+1);
	Sequence = seq;
	
	CurTrans = UUPCTrans;
	
	return(noErr);
}

/************************************************************************
 * UUPCDry - put away the toys
 ************************************************************************/
void UUPCDry(void)
{
	if (UUX)
	{
		if (*UUX)
		{
			if (Sequence!=OldSeq)
			{
				Str255 smtp;
				GetPref(smtp,PREF_SMTP);
				if (*smtp>4)
				{
					UPtr cp = smtp+*smtp;
					short n=4;
					while (n--)
					{
						*cp-- = Sequence%10+'0';
						Sequence /= 10;
					}
				}
				ChangeStrn(PREF_STRN,PREF_SMTP,smtp);
			}
		}
		ZapHandle(UUX);
	}
	CurTrans = UseCTB ? CTBTrans : TCPTrans;
}

/************************************************************************
 * UUPCSendMessage - the hard work goes here
 ************************************************************************/
int UUPCSendMessage(TOCHandle tocH,short sumNum)
{
	short err;
	MessHandle messH;
	
	/*
	 * grab the message
	 */
	if (!(messH=SaveB4Send(tocH,sumNum))) return(1);
	MessH = messH;

	/*
	 * generate some unique filenames in the spool directory
	 */
	do {UUPCGenFilenames();} while (UUPCBadFilenames());
	err = UUPCMakeFiles();
	if (!err)
	{
		err = UUPCOpenFiles();
		if (!err)
		{
			err = UUPCWriteXFile();
			if (!err) err = UUPCWriteMailFile();
		}
	}

	UUPCCloseFiles();
	if (err) UUPCKillFiles();
	else TimeStamp(tocH,sumNum,GMTDateTime(),ZoneSecs());
	return(err);
}

/************************************************************************
 * UUPCGenFilenames - make the filenames
 ************************************************************************/
void UUPCGenFilenames(void)
{
	Str15 seq;
	int n;
	Str255 scratch;
	Str15 mymacShort;
	
	PCopy(mymacShort,MyMac);
	*mymacShort=MIN(7,*mymacShort);
	
	n = ++Sequence;
	seq[0] = 4;
	seq[4] = n%10+'0'; n/=10;
	seq[3] = n%10+'0'; n/=10;
	seq[2] = n%10+'0'; n/=10;
	seq[1] = n%10+'0';
	ComposeRString(scratch,UUPC_DMYMAC,mymacShort,seq); PCopy(Dmymac,scratch);
	ComposeRString(scratch,UUPC_XMYMAC,mymacShort,seq); PCopy(Xmymac,scratch);
	PCopy(MailFile,SpoolPath); PCat(MailFile,Dmymac);
	PCopy(XFile,SpoolPath); PCat(XFile,Xmymac);
}

/************************************************************************
 * UUPCBadFilenames - check that our files don't already exist
 ************************************************************************/
Boolean UUPCBadFilenames(void)
{
	Str255 scratch;
	short i;
	HFileInfo hfi;
	
	for (i=0;i<sizeof((*UUX)->refNs)/sizeof(short);i++)
	{
		PCopy(scratch,(*UUX)->files[i]);
		if (!HGetFileInfo(0,0,scratch,&hfi)) return(True);
	}
	return(False);
}

/************************************************************************
 * UUPCMakeFiles - create our files
 ************************************************************************/
int UUPCMakeFiles(void)
{
	Str255 scratch;
	short i,err;
	long creator;
	
	GetPref(scratch,PREF_CREATOR);
	if (*scratch!=4) GetRString(scratch,TEXT_CREATOR);
	BlockMove(scratch+1,&creator,4);
	
	for (i=0;i<sizeof((*UUX)->refNs)/sizeof(short);i++)
	{
		PCopy(scratch,(*UUX)->files[i]);
		if (err=HCreate(0,0,scratch,creator,'TEXT'))
			return(FileSystemError(TEXT_WRITE,scratch,err));
	}
	return(False);
}

/************************************************************************
 * UUPCCloseFiles - close our files
 ************************************************************************/
void UUPCCloseFiles(void)
{
	short i;
	for (i=0;i<sizeof((*UUX)->refNs)/sizeof(short);i++)
		if ((*UUX)->refNs[i]) (void) FSClose((*UUX)->refNs[i]);
}

/************************************************************************
 * UUPCKillFiles - destroy our files
 ************************************************************************/
void UUPCKillFiles(void)
{
	Str255 scratch;
	short i;
	for (i=0;i<sizeof((*UUX)->refNs)/sizeof(short);i++)
	{
		PCopy(scratch,(*UUX)->files[i]);
		(void) HDelete(0,0,scratch);
	}
	Sequence--;
}

/************************************************************************
 * UUPCOpenFiles - open our files
 ************************************************************************/
int UUPCOpenFiles(void)
{
	Str255 scratch;
	short i,err,refN;
	for (i=0;i<sizeof((*UUX)->refNs)/sizeof(short);i++)
	{
		PCopy(scratch,(*UUX)->files[i]);
		if (err=FSOpen(scratch,0,&refN))
			return(FileSystemError(TEXT_WRITE,scratch,err));
		(*UUX)->refNs[i] = refN;
	}
	return(0);
}

/************************************************************************
 * UUPCWriteXFile - write the file to send to the other side for execution
 ************************************************************************/
int UUPCWriteXFile(void)
{
	short err;
	Str255 buffer;
	Str15 newline;
	
	LDRef(UUX);
	
	ComposeRString(buffer,UUPC_U_CMD,UserName,MyMac);
	err = FSWriteP(XRefN,buffer);
	
	if (!err)
	{
		UUPCGetNewline(newline,buffer);
		
		ComposeRString(buffer,UUPC_F_CMD,Dmymac);
		err = FSWriteP(XRefN,buffer);
		
		if (!err)
		{
			ComposeRString(buffer,UUPC_I_CMD,Dmymac);
			err = FSWriteP(XRefN,buffer);
			
			if (!err)
			{
				UL(UUX);
				GetRString(buffer,UUPC_C_CMD);
				err = FSWriteP(XRefN,buffer);
				
				if (!err) err=DoRcptTos(MessH,False);
				if (!err) err=FSWriteP(XRefN,newline);
			}
		}
	}
	
	if (err) FileSystemError(TEXT_WRITE,XFile,err);
	UL(UUX);
	return(err);
}

/************************************************************************
 * UUPCWriteMailFile - get the message into the file
 ************************************************************************/
int UUPCWriteMailFile(void)
{
	MSumType sum;
	Str255 envelope;
	short len;
	Str15 oldNewLine;
	short err;
	
	PCopy(sum.from,UserName);
	*envelope = SumToFrom(&sum,envelope+1);
	
	LDRef(UUX);
	ComposeRString(envelope+*envelope,UUPC_REMOTE,MyMac);
	UL(UUX);
	
	len = *envelope;
	*envelope += envelope[len];
	envelope[len] = ' ';
	if (err=FSWriteP(MailRefN,envelope))
	{
		PCopy(envelope,MailFile);
		FileSystemError(TEXT_WRITE,envelope,err);
		return(err);
	}

	PCopy(oldNewLine,NewLine);
	UUPCGetNewline(NewLine,envelope);
	err = TransmitMessage(MessH,False);
	PCopy(NewLine,oldNewLine);
	return(err);
}

/************************************************************************
 * UUCPWriteAddr - write an address to the X file
 ************************************************************************/
int UUPCWriteAddr(UPtr addr)
{
	Str255 buffer;
	short err;
	
	*buffer = 1;
	buffer[1] = ' ';
	PCat(buffer,addr);
	if (err=FSWriteP(XRefN,buffer))
	{
		PCopy(buffer,XFile);
		FileSystemError(TEXT_WRITE,buffer,err);
	}
	return(err ? 500 : 0);
}

/************************************************************************
 * UUPCGetNewline - what's this guy using for a newline?
 ************************************************************************/
void UUPCGetNewline(UPtr newline,UPtr buffer)
{
	UPtr bp;
	for (bp=buffer+*buffer;bp[-1]<' ';bp--);
	*newline = (buffer+*buffer)-bp+1;
	BlockMove(bp,newline+1,*newline);
}

/************************************************************************
 * UUPCSendTrans - sendtrans to a file
 ************************************************************************/
int UUPCSendTrans(short count, UPtr text,long size, ...)
{
	long bSize;
	short err;
	
	if (size==0) return(noErr); 	/* allow vacuous sends */
	if (MiniEvents()) return(userCancelled);
	
	if (!DontTranslate && TransOut)
		{Uptr cp; for(cp=text;cp<text+size;cp++) *cp = TransOut[*cp];}
	bSize = size;
	if (err=FSWrite(MailRefN,&bSize,text))
		return(FileSystemError(TEXT_WRITE,MailFile,err));
	if (--count>0)
	{
		Uptr buffer;
		va_list extra_buffers;
		va_start(extra_buffers,size);
		while (count--)
		{
			buffer = va_arg(extra_buffers,UPtr);
			bSize = va_arg(extra_buffers,int);
			if (UUPCSendTrans(1,buffer,bSize)) break;
		}
		va_end(extra_buffers);
	}
	return(err);
}
