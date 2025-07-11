#define FILE_NUM 27
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * routines to make MacTCP palatable
 ************************************************************************/
#pragma load EUDORA_LOAD
#pragma segment TcpTrans

short TcpSpinOn(TCPiopb *pb,Boolean doAbort);

static short RefN=0;
/************************************************************************
 *
 ************************************************************************/
int MyTcpCreate(long *stream,UPtr buffer,int bSize,TCPNotifyProc asr)
{
	TCPiopb pb;
	int err;
	
	if (!RefN) return(badUnitErr);
	if (WhyTCPTerminated) return(WhyTCPTerminated);
	WriteZero(&pb,sizeof(pb));
	pb.ioCRefNum = RefN;
	pb.csCode = TCPCreate;

	pb.csParam.create.rcvBuff = buffer;
	pb.csParam.create.rcvBuffLen = bSize;
	pb.csParam.create.notifyProc = asr;
	
	err = PBControl(&pb,False);
	if (!err) *stream = pb.tcpStream;
	return(err ? err : pb.ioResult);
}

/************************************************************************
 *
 ************************************************************************/
int MyTcpOpen(long stream,long addr,short port,Byte toAction,Byte timeout)
{
	TCPiopb pb;
	int err;
	
	if (!RefN) return(badUnitErr);
	if (WhyTCPTerminated) return(WhyTCPTerminated);
	WriteZero(&pb,sizeof(pb));
	pb.ioCRefNum = RefN;
	pb.tcpStream = stream;
	pb.csCode = TCPActiveOpen;
	
	pb.csParam.open.commandTimeoutValue = timeout;
	pb.csParam.open.ulpTimeoutValue = timeout;
	pb.csParam.open.ulpTimeoutAction = toAction;
	pb.csParam.open.validityFlags = timeoutValue | timeoutAction;
	pb.csParam.open.remoteHost = addr;
	pb.csParam.open.remotePort = port;
	pb.csParam.open.localPort = GetRLong(LOCAL_PORT);
	err = PBControl(&pb,True);
	if (err==inProgress) err=0;
	if (err) return(err);
	return(TcpSpinOn(&pb,True));
}

/************************************************************************
 *
 ************************************************************************/
int MyTcpSend(long stream,short count,UPtr buffer,int bSize, va_list extra)
{
	wdsEntry myWDS[40];
	wdsEntry *exWDS;
		
	if (!RefN) return(badUnitErr);
	if (WhyTCPTerminated) return(WhyTCPTerminated);
	myWDS[0].length = bSize;
	myWDS[0].ptr = buffer;
	for (exWDS=myWDS+1;--count>0;exWDS++)
	{
		exWDS->ptr = va_arg(extra,UPtr);
		exWDS->length = va_arg(extra,unsigned);
		if (!exWDS->length) exWDS--;				/* skip zero-length sends */
	}
	exWDS->length = 0;
	return(MyTcpSendWDS(stream,myWDS));
}

/************************************************************************
 *
 ************************************************************************/
int MyTcpSendWDS(long stream,wdsEntry *theWDS)
{
	TCPiopb pb;
	int err;
	
	if (!RefN) return(badUnitErr);
	if (WhyTCPTerminated) return(WhyTCPTerminated);
	if (!theWDS->length)
		return(noErr);

	WriteZero(&pb,sizeof(pb));
	pb.ioCRefNum = RefN;
	pb.tcpStream = stream;
	pb.csCode = TCPSend;

	pb.csParam.send.pushFlag = 1;
	pb.csParam.send.wdsPtr = theWDS;
	
	if (!DontTranslate && TransOut)
	{
		wdsEntry *wds;
		UPtr cp;
		for (wds=theWDS;wds->length;wds++)
			for (cp=wds->ptr;cp<wds->ptr+wds->length;cp++) *cp = TransOut[*cp];
	}

	if (LogLevel&LOG_TRANS)
	{
		wdsEntry *wds;
		for (wds=theWDS;wds->length;wds++)
			CarefulLog(LOG_TRANS,LOG_SENT,wds->ptr,wds->length);
	}
	
	err = PBControl(&pb,True);
	if (err) return(err);
	return(TcpSpinOn(&pb,False));
}

/************************************************************************
 *
 ************************************************************************/
int MyTcpRecv(long stream,UPtr buffer,int *bSize,Byte timeout)
{
	TCPiopb pb;
	int err;
	
	if (!RefN) return(badUnitErr);
	if (WhyTCPTerminated) return(WhyTCPTerminated);
	WriteZero(&pb,sizeof(pb));
	pb.ioCRefNum = RefN;
	pb.tcpStream = stream;
	pb.csCode = TCPRcv;

	pb.csParam.receive.commandTimeoutValue = timeout;
	pb.csParam.receive.rcvBuff = buffer;
	pb.csParam.receive.rcvBuffLen = *bSize;
	*bSize = 0;
	
	err = PBControl(&pb,True);
	if (err) return(err);
	err = TcpSpinOn(&pb,False);
	if (!err)
		*bSize = pb.csParam.receive.rcvBuffLen;
	if (*bSize && LogLevel&LOG_TRANS) CarefulLog(LOG_TRANS,LOG_GOT,buffer,*bSize);
	return(err);
}

/************************************************************************
 *
 ************************************************************************/
int MyTcpStatus(long stream,TCPiopb *pb)
{
	int err;
	
	if (!RefN) return(badUnitErr);
	if (WhyTCPTerminated) return(WhyTCPTerminated);
	WriteZero(pb,sizeof(*pb));
	pb->ioCRefNum = RefN;
	pb->tcpStream = stream;
	pb->csCode = TCPStatus;

	return(err = PBControl(pb,False));
}

/************************************************************************
 *
 ************************************************************************/
int MyTcpClose(long stream)
{
	TCPiopb pb;
	int err;
	
	if (!RefN) return(badUnitErr);
	if (WhyTCPTerminated) return(WhyTCPTerminated);
	WriteZero(&pb,sizeof(pb));
	pb.ioCRefNum = RefN;
	pb.tcpStream = stream;
	pb.csCode = TCPClose;
	
	err = PBControl(&pb,True);
	if (err) return(err);
	return(TcpSpinOn(&pb,True));
}

/************************************************************************
 *
 ************************************************************************/
int MyTcpAbort(long stream)
{
	TCPiopb pb;
	int err;
	
	if (!RefN) return(badUnitErr);
	if (WhyTCPTerminated) return(WhyTCPTerminated);
	WriteZero(&pb,sizeof(pb));
	pb.ioCRefNum = RefN;
	pb.tcpStream = stream;
	pb.csCode = TCPAbort;
	
	err = PBControl(&pb,False);
	return(err ? err : pb.ioResult);
}

/************************************************************************
 *
 ************************************************************************/
short MyTcpRefN(short refN)
{
	if (refN) RefN = refN;
	return(RefN);
}

/************************************************************************
 *
 ************************************************************************/
int MyTcpRelease(long stream)
{
	TCPiopb pb;
	int err;
	
	if (!RefN) return(badUnitErr);
	WriteZero(&pb,sizeof(pb));
	pb.ioCRefNum = RefN;
	pb.tcpStream = stream;
	pb.csCode = TCPRelease;

	err = PBControl(&pb,False);
	return(err ? err : pb.ioResult);
}

/************************************************************************
 * MyHostid - figure out my address and net mask
 ************************************************************************/
int MyHostid(ip_addr *addr, long *mask)
{
	struct IPParamBlock pb;
	int err;
	
	if (!RefN) return(badUnitErr);
	pb.ioCompletion = nil;
	pb.ioNamePtr = nil;
	pb.ioCRefNum = RefN;
	pb.ioVRefNum = 0;
	pb.csCode = ipctlGetAddr;

	err = PBControl(&pb,False);
	if (!err)
	{
		*addr = pb.ourAddress;
		*mask = pb.ourNetMask;
	}
	return(err ? err : pb.ioResult);
}

/************************************************************************
 * TcpSpinOn - spin, allowing cancellation, but only AFTER the operation
 * actually completes (I'm cutting this part; dunno why it was here)
 ************************************************************************/
short TcpSpinOn(TCPiopb *pb,Boolean doAbort)
{
	short err;

#ifndef OLDCODE
	doAbort = True;
#endif

	err = SpinOn(&pb->ioResult,0);
	if (err==userCancelled)
	{
		if (doAbort)
		{
			MyTcpAbort(pb->tcpStream);
		}
		else
			while (SpinOn(&pb->ioResult,0)==userCancelled);
		return(userCancelled);
	}
	if (WhyTCPTerminated) return(WhyTCPTerminated);
	return(err);
}
