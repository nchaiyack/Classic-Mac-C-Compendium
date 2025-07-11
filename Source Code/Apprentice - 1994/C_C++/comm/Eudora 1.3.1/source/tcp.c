#define FILE_NUM 37
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * functions for i/o over a pseudo-telnet MacTcp stream
 * these functions are oriented toward CONVENIENCE, not performance
 ************************************************************************/
#pragma load EUDORA_LOAD
#pragma segment TcpTrans
static int TcpErr=0;
static int CharsAvail=0;
static UPtr TcpBuffer=nil;
static long TcpStream=nil;
static Boolean BeSilent;
#ifndef SLOW_CLOSE
pascal void FastCloseComplete();
void TcpFastClose(void);
#endif

/************************************************************************
 * private functions
 ************************************************************************/
int GetHostByName(UPtr name, struct hostInfo *hostInfoPtr);
int VerifyOpen(void);
pascal void BindDone(struct hostInfo *hostInfoPtr, char *userData);
pascal void TcpASR(StreamPtr tcpStream, unsigned short eventCode,
UPtr userDataPtr, unsigned short terminReason, struct ICMPReport *icmpMsg);
short WaitForChars(long timeout);
#define TcpTrouble(which,err) TT(which,err,FILE_NUM,__LINE__)
int TT(int which, int theErr, int file, int line);

/************************************************************************
 * TCPConnectTrans - connect to the remote host.	This version uses MacTCP.
 ************************************************************************/
int TCPConnectTrans(UPtr serverName, short port,Boolean silently)
{
	Str255 scratch;
	long bSize;
	struct hostInfo hi;

#ifdef DEBUG
	if (BUG12) port += 10000;
#endif
	BeSilent = silently;
	Progress(NoBar,GetRString(scratch,WHO_AM_I));
	if (TcpErr=VerifyOpen()) return (TcpErr);

	/*
	 * find the host
	 */
	PCat(GetRString(scratch,DNR_LOOKUP),serverName);
	Progress(NoBar,scratch);
	if (TcpErr=GetHostByName(serverName,&hi))
	{
		TcpTrouble(BIND_ERR,TcpErr);
		return(TcpErr);
	}
	
	/*
	 * allocate a buffer for tcp, and create the stream
	 */
	Progress(NoChange,GetRString(scratch,HOUSEKEEPING));
	bSize = GetRLong(RCV_BUFFER_SIZE);
	if ((RcvBuffer=NuHandle(bSize))==nil)
	{
		TcpErr=MemError();
		WarnUser(MEM_ERR,TcpErr);
		return(TcpErr);
	}
	bSize = GetRLong(TCP_BUFFER_SIZE);
#ifdef DEBUG
	if (BUG13) bSize *= 4;
	else if (BUG2) bSize *= 2;
#endif
	if ((TcpBuffer=NuPtr(bSize))==nil)
	{
		TcpErr=MemError();
		WarnUser(MEM_ERR,TcpErr);
		return(TcpErr);
	}
	if (TcpErr=MyTcpCreate(&TcpStream,TcpBuffer,bSize,TcpASR))
	{
		TcpTrouble(TCP_TROUBLE,TcpErr);
		return(TcpErr);
	}
	
	/*
	 * now, try to open the connection
	 */
	ComposeRString(scratch,CNXN_OPENING,serverName,*(unsigned long *)hi.addr);
	Progress(NoChange,scratch);
	TcpErr=MyTcpOpen(TcpStream,*hi.addr,port,1,GetRLong(OPEN_TIMEOUT));
	if (TcpErr)
		TcpTrouble(NO_SMTP_SERVER,TcpErr);
	RcvSpot = -1;
	
	return(TcpErr);
}

/************************************************************************
 * TCPSendTrans - send some text to the remote host.	This version uses MacTCP.
 ************************************************************************/
int TCPSendTrans(short count, UPtr text,long size, ...)
{
	va_list extra_buffers;
	if (!TcpStream) return(1);
	if (size==0) return(noErr); 	/* allow vacuous sends */
	CycleBalls();
	va_start(extra_buffers,size);
	if (TcpErr=MyTcpSend(TcpStream,count,text,size,extra_buffers))
		TcpTrouble(TCP_TROUBLE,TcpErr);
	va_end(extra_buffers);
	return(TcpErr);
}

/************************************************************************
 * TCPSendWDS - send a lot of text to the remote host.	This version uses MacTCP.
 ************************************************************************/
int TCPSendWDS(wdsEntry *theWDS)
{
	CycleBalls();
	if (!TcpStream) return(1);
	if (TcpErr=MyTcpSendWDS(TcpStream,theWDS))
		TcpTrouble(TCP_TROUBLE,TcpErr);
	return(TcpErr);
}

/************************************************************************
 * TCPRecvTrans - get some text from the remote host.  This version uses MacTCP.
 ************************************************************************/
int TCPRecvTrans(UPtr line,long *size)
{
	Str31 scratch;
	
	if (!TcpStream) return(1);
	do
	{
		TcpErr = WaitForChars(GetRLong(RECV_TIMEOUT));
		if (!TcpErr || TcpErr == connectionDoesntExist)
			TcpErr=MyTcpRecv(TcpStream,line,size,0);
	}
	while (TcpErr==commandTimeout &&
				 AlertStr(TIMEOUT_ALRT,Caution,GetRString(scratch,RECV_TIMEOUT))==1);

	if (TcpErr)
	{
		if (TcpErr!=commandTimeout && TcpErr!=userCancelled)
			TcpTrouble(TCP_TROUBLE,TcpErr);
	}
	return(TcpErr);
}

/************************************************************************
 * TCPDisTrans - disconnect from the remote host.  This version uses MacTCP.
 * You MUST wait for other side's close under MacOS; if you get a packet
 * for a released, stream, the Mac will crash.	This should be avoided
 * under A/UX, however, since it's unnecessary and takes so long.
 ************************************************************************/
int TCPDisTrans(void)
{
	Str63 buffer;
#ifdef OLDCLOSE
	int length;
#endif

#ifndef SLOW_CLOSE
	TcpFastClose();
#endif
	if (TcpStream)
	{
		Progress(NoBar,GetRString(buffer,CTB_CLOSING));
		if (!TcpErr && !(TcpErr=MyTcpClose(TcpStream)))
		{
#ifdef OLDCLOSE
		  if (!IsAUX())
			{
				for (length=sizeof(buffer);!TcpErr;length=sizeof(buffer))
				{
					if (!(TcpErr=WaitForChars(GetRLong(RECV_TIMEOUT))))
							TcpErr=MyTcpRecv(TcpStream,buffer,&length,0);
				}
			}
#endif
		}
	}
	if (TcpErr==connectionDoesntExist)
		TcpErr = noErr;
	return(TcpErr);
}

/************************************************************************
 * TCPDestroyTrans - Be sure the connection is destroyed
 ************************************************************************/
int TCPDestroyTrans(void)
{
	if (TcpStream)
	{
		TCPiopb pb;
		if (!MyTcpStatus(TcpStream,&pb) && pb.csParam.status.connectionState)
		{
			MyTcpAbort(TcpStream);
			Pause(60);				/* believe me, this is the safe thing to do */
		}
		MyTcpRelease(TcpStream);
		TcpStream = nil;
	}
	if (TcpBuffer)
	{
		DisposPtr(TcpBuffer);
		TcpBuffer = nil;
	}
	if (RcvBuffer) ZapHandle(RcvBuffer);
	return(TcpErr);
}


/************************************************************************
 * VerifyOpen - make sure the MacTCP driver is going
 ************************************************************************/
int VerifyOpen(void)
{
	static short refN = 0;
	WhyTCPTerminated = 0;
	if (!refN)
	{
		Str255 driverName;
		GetRString(driverName,TCP_DRIVER);
		if (TcpErr = OpenDriver(driverName,&refN))
		{
			TcpTrouble(TCP_TROUBLE,TcpErr);
			return(TcpErr);
		}
		MyTcpRefN(refN);
	}
	return(noErr);
}

/************************************************************************
 * TCPTransError - report our most recent error
 ************************************************************************/
int TCPTransError(void)
{
	return(TcpErr);
}

#pragma segment Main
/************************************************************************
 * BindDone - report that the resolver has done its duty
 ************************************************************************/
pascal void BindDone(struct hostInfo *hostInfoPtr, char *userData)
{
#pragma unused(hostInfoPtr)
	if (userData) *(int *)userData = 1;
	return;
}
#pragma segment TcpTrans

/************************************************************************
 * WaitForChars - spin, giving everybody else time, until chars available
 ************************************************************************/
short WaitForChars(long timeout)
{
	EventRecord event;
	long ticks=TickCount();
	static long waitTicks=0;
	long tookTicks;
	Boolean result;
	long timeoutTicks = ticks + 60*timeout;
	TCPiopb pb;
	short err;

	if (!InBG) waitTicks = 0;
	do
	{
		if (TickCount()-ticks  > 10)
		{
			CyclePendulum();
			ticks=TickCount();
			if (ticks >timeoutTicks) return(commandTimeout);
		}
		tookTicks = TickCount();
		result = WNE(everyEvent,&event,waitTicks);
		tookTicks = TickCount()-tookTicks;
		if (InBG)
			if (tookTicks > waitTicks+1)
				waitTicks = MIN(120,tookTicks);
			else
				waitTicks = MAX(0,waitTicks>>1);
		if (result)
		{
			(void) MiniMainLoop(&event);
			if (CommandPeriod) return(userCancelled);
		}
		ClearICMP();
		if (err=MyTcpStatus(TcpStream,&pb)) return(err);
		if (pb.csParam.status.connectionState != 8) return(connectionDoesntExist);
	}
	while (!pb.csParam.status.amtUnreadData);
	CharsAvail = 0;
	return(0);
}

/************************************************************************
 * GetHostByName - get host information, given a hostname
 * this routine maintains a small, unflushable cache.
 ************************************************************************/
int GetHostByName(UPtr name, struct hostInfo *hostInfoPtr)
{
	Str255 cName;
	static struct hostInfo cache[4];
	static short nextInfo;
	short info;
	HostInfoQHandle hiqh;
#define HI (*hiqh)->hi
	
	strncpy(cName,name+1,*name);
	cName[*name] = 0;
	for (info=0;info<sizeof(cache)/sizeof(struct hostInfo);info++)
		if (!strcmp(cName,cache[info].cname))
		{
			*hostInfoPtr = cache[info];
			return (noErr);
		}
	if ((hiqh=NewZH(HostInfoQ))==nil) return(MemError());
	LL_Push(HIQ,hiqh);
	MoveHHi(hiqh);
	HLock(hiqh);
	if (TcpErr = OpenResolver(nil))
	{
		TcpTrouble(BIND_ERR,TcpErr);
		return(TcpErr);
	}
	strncpy(cName,name+1,*name);
	cName[*name] = 0;
	HI.rtnCode = inProgress;
	TcpErr = StrToAddr(cName,&HI,BindDone,nil);
	if (TcpErr==cacheFault || !TcpErr) TcpErr=SpinOn(((short *)&HI.rtnCode)+1,0);
	if (!CommandPeriod)
	{
		if (!TcpErr)
		{
			cache[nextInfo] = *hostInfoPtr = HI;
			strcpy(cache[nextInfo].cname,cName);	/* remember the name we're given,
																							 not the real cname */
			nextInfo = (nextInfo + 1) % (sizeof(cache)/sizeof(struct hostInfo));
		}
		LL_Remove(HIQ,hiqh,(HostInfoQHandle));
		if (!HIQ) CloseResolver();
		DisposHandle(hiqh);
	}
	return(TcpErr);
}

/************************************************************************
 * GetHostByAddr - get host information, given an address
 * this routine maintains a small, unflushable cache.
 ************************************************************************/
int GetHostByAddr(struct hostInfo *hostInfoPtr,long addr)
{
	static struct hostInfo lastInfo;
	static long lastAddr;
	HostInfoQHandle hiqh;
#define HI (*hiqh)->hi
	
	if (addr==lastAddr)
	{
		*hostInfoPtr = lastInfo;
		return(noErr);
	}

	if ((hiqh=NewZH(HostInfoQ))==nil) return(MemError());
	LL_Push(HIQ,hiqh);
	MoveHHi(hiqh);
	HLock(hiqh);
	if (TcpErr = OpenResolver(nil))
	{
		TcpTrouble(BIND_ERR,TcpErr);
		return(TcpErr);
	}

	HI.rtnCode = inProgress;
	TcpErr = AddrToName(addr,&HI,BindDone,nil);
	if (TcpErr==cacheFault || !TcpErr) TcpErr=SpinOn(((short *)&HI.rtnCode)+1,0);
	if (!CommandPeriod)
	{
		LL_Remove(HIQ,hiqh,(HostInfoQHandle));
		if (!HIQ) CloseResolver(); else ASSERT(0);
		if (!TcpErr)
		{
			short l = strlen(HI.cname);
			if (HI.cname[l-1]=='.') HI.cname[l-1]=0;	/* for benefit of broken sendmails */
			lastInfo = *hostInfoPtr = HI;
			lastAddr = addr;
		}
		DisposHandle(hiqh);
	}

	return(TcpErr);
}

/************************************************************************
 * TcpTrouble - report an error with TCP and break the connection
 ************************************************************************/
int TT(int which, int theErr, int file, int line)
{
	if (!BeSilent && !CommandPeriod)
	{
		Str255 message;
		Str255 tcpMessage;
		Str63 debugStr;
		Str31 rawNumber;
		
		NumToString(theErr,rawNumber);
		GetRString(message, which);
		if (-23000>=theErr && theErr >=-23048)
			GetRString(tcpMessage,MACTCP_ERR_STRN-22999-theErr);
		else if (2<=theErr && theErr<=9)
			GetRString(tcpMessage,MACTCP_ERR_STRN+theErr+(23048-23000));
		else
			*tcpMessage = 0;

		ComposeRString(debugStr,FILE_LINE_FMT,file,line);
		MyParamText(message,rawNumber,tcpMessage,debugStr);
		(void) ReallyDoAnAlert(BIG_OK_ALRT,Caution);
	}
	return(TcpErr = theErr);
}

/************************************************************************
 * TCPSilenceTrans - turn off error reports from tcp routines
 ************************************************************************/
void TCPSilenceTrans(Boolean silence)
{
	BeSilent = silence;
}

/************************************************************************
 * ClearICMP - warn about an ICMP error
 ************************************************************************/
short ClearICMP(void)
{
	if (ICMPAvail && GetRLong(ICMP_SECONDS))
	{
		Str255 scratch;
		Boolean oldAlertsTimeout = AlertsTimeout;
		short report = ICMPMessage.reportType;

		if (report<0 || report>8) report = 9;
		AlertsTimeout = True;
		GetRString(scratch,ICMP_STRN+report+1);
		AlertTicks = GetRLong(ICMP_SECONDS)*60 + TickCount();
		InBG = 0; /* Don't ask, please */
		AlertStr(ICMP_ALRT,Caution,scratch);
		AlertTicks = 0;
		AlertsTimeout = oldAlertsTimeout;
		ICMPAvail = 0;											/* resetting the flag here
																					 means we may lose ICMP messages
																					 while the alert is up.  That
																					 is GOOD. */
		return(ICMPMessage.reportType+1);
	}
	return(0);
}

/************************************************************************
 * TCPWhoAmI - return the mac's tcp name
 ************************************************************************/
UPtr TCPWhoAmI(Uptr who)
{
	uLong addr, mask;
	MyHostid((ip_addr *)&addr,&mask);
	return(ComposeRString(who,TCP_ME,addr));
}

/************************************************************************
 * GetTCPStatus - return connection info
 ************************************************************************/
int GetTCPStatus(TCPiopb *pb)
{
	return(MyTcpStatus(TcpStream,pb));
}

#ifndef SLOW_CLOSE
/************************************************************************
 * TcpFastClose - close the current stream, but don't wait for it
 *
 * We allocate a parameter block on the heap, and stuff the current stream
 * info into it.  Then, we start a receive.  The ioCompletion routine for the
 * receive will start another one, until there is nothing more to receive.
 * then the ioCompletion routine starts a close.  When the close completes
 * or if there is an error anywhere along the way, the csCode of the block
 * is set to TCPRelease, which signals TcpFastFlush (called from the main
 * loop and the Cleanup routine) to release the stream (and the pb).
 *
 * if we can't allocate the pb, we just return, and let the stream go through
 * the normal close process
 ************************************************************************/
void TcpFastClose(void)
{
	FastPBPtr mypb = NewPtrClear(sizeof(FastPB));
	if (mypb)
	{
		/*
		 * link it onto the list
		 */
		mypb->next = FastList;
		FastList = mypb;
		
		/*
		 * fill in invariant values
		 */
		mypb->pb.ioCRefNum = MyTcpRefN(0);
		mypb->pb.tcpStream = TcpStream;
		mypb->pb.ioCompletion = FastCloseComplete;
		mypb->streamBuffer = TcpBuffer;
		
		/*
		 * start a receive
		 */
		mypb->pb.csCode = TCPRcv;
		mypb->pb.csParam.receive.commandTimeoutValue = 60;	/* let it go for a minute */
		mypb->pb.csParam.receive.rcvBuff = mypb->buffer;
		mypb->pb.csParam.receive.rcvBuffLen = sizeof(mypb->buffer);
		if (PBControl(mypb,True)) mypb->pb.csCode = TCPRelease;	/* kill it on error */
		
		/*
		 * don't have to worry about the stream anymore
		 */
		TcpStream = nil;
		TcpBuffer = nil;
	}
}

#pragma segment Main

/************************************************************************
 * TcpFastFlush - run through the queue, killing off defunct streams
 ************************************************************************/
void TcpFastFlush(Boolean destroy)
{
	FastPBPtr this,next,last=nil;
	
	for (this=FastList;this;this=next)
	{
		next = this->next;
		if (destroy || this->pb.csCode==TCPRelease)
		{
			/*
			 * cut it out of the herd
			 */
			if (FastList==this) FastList=next;
			if (last) last->next = next;
			
			/*
			 * and finish it off
			 */
			MyTcpRelease(this->pb.tcpStream);
			DisposPtr(this->streamBuffer);
			DisposPtr(this);
		}
		else
			last = this;
	}
}

/************************************************************************
 * FastCloseComplete - ioCompletion routine for fast close
 ************************************************************************/
FastPBPtr		*GetParmBlockPtr()		= { 0x2008 }; 	// move.l a0, d0 ;move the pointer to where MPW C places function results
pascal void FastCloseComplete()
{
	FastPBPtr pb = GetParmBlockPtr();
	
	if (pb->pb.csCode==TCPClose)
	{
		pb->pb.csCode = TCPRelease;					/* tell TcpFastFlush to kill it */
		return;
	}
	else if (pb->pb.ioResult==noErr)	/* still going */
		pb->pb.csParam.receive.rcvBuffLen = sizeof(pb->buffer);
	else  /* receive failed */
	{
		pb->pb.csCode = TCPClose;
		pb->pb.csParam.close.validityFlags = 0;
	}
	if (PBControl(pb,True)) pb->pb.csCode = TCPRelease;	/* kill it on error */
}
#endif

/************************************************************************
 * TcpASR - asynchronous notification routine
 ************************************************************************/
pascal void TcpASR(StreamPtr tcpStream, unsigned short eventCode,
	UPtr userDataPtr, unsigned short terminReason, struct ICMPReport *icmpMsg)
{
#pragma unused(userDataPtr)
	if (tcpStream==TcpStream)
	{
		if (eventCode==TCPDataArrival) CharsAvail = 1;
		else if (eventCode==TCPICMPReceived)
		{
			ICMPAvail = 1;
			ICMPMessage = *icmpMsg;
		}
		else if (eventCode==TCPTerminate)
		{
			WhyTCPTerminated = terminReason;
		}
	}
}
