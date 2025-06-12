// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UMacTcp.cp

#include "UMacTcp.h"

#include "AddressXlation.h"
#include "GetMyIPAddr.h"
#include "UThread.h"
#include "UProgress.h"
#include "Tools.h"
#include "UFatalError.h"

#include <ErrorGlobals.h>

#ifndef __STDIO__
#include <stdio.h>
#endif

#include <Traps.h>
#include <SysEqu.h>
#include <OSUtils.h>

#pragma segment MyComm

const long kStreamBufferSize = 8 * 1024;

#if qDebug
#define macroCheck(x) x
#else
#define macroCheck(x)
#endif

#define qDebugActivity qDebug & 0

short gMacTCPID = 100;
//--------------------------------------------------------------------------------------------

short gMacTcpRefNum = 0;
long gMyIP = 0;
CStr255 gMyDotName("");
Boolean gResolverIsOpen = false;
TrapPatch gKillMacTCPPatch;
TLongintList *gMacTCPList = nil;
long gNuntiusA5 = nil;

#pragma push
#pragma trace off
#pragma segment A5Ref
char gResolverDone; 
// 'done' may not be on stack as it can be swapped out 
// and the variable is accessed from interrupt!
pascal void DNRResultProc(struct hostInfo* /* hInfoPtr */,char *userDataPtr)
{
	*userDataPtr = 0xff;
}
#pragma pop

#pragma segment A5Ref
void InitUMacTCP()
{
	// Placed in A5Ref to get the segment loaded so _LoadSeg called on that
	// segment. Otherwise it will be called at interrupt time :-(
	short errNo = OpenDriver(".IPP", gMacTcpRefNum);
	if (errNo != noErr)
		Failure(errNo, messageCannotOpenMacTcp);
	gNuntiusA5 = *(long*)CurrentA5; // low mem global
	TLongintList *lList = new TLongintList();
	lList->ILongintList();
	gMacTCPList = lList;
	FailOSErr(HeadPatch(gKillMacTCPPatch, _ExitToShell, (void*)StripLong(KillMacTCPStuff)));
}

#pragma segment MyComm
void CloseDownUMacTCP()
{
	if (gResolverIsOpen)
	{
		gResolverIsOpen = false;
		OSErr err = CloseResolver();
#if qDebug
		if (err)
		{
			fprintf(stderr, "Got error when closing resolver: %ld\n", long(err));
			ProgramBreak(gEmptyString);
		}
#endif
	}
	gMacTcpRefNum = 0;
}

long MyIP()
{
	if (!gMacTcpRefNum)
		return 0x01020304;
	if (gMyIP)
		return gMyIP;
	GetAddrParamBlock pb;
	BlockSet(Ptr(&pb), sizeof(pb), 0);
	pb.csCode = ipctlGetAddr;
	pb.ioCRefNum = gMacTcpRefNum;
	FailOSErr(PBControlSync(ParmBlkPtr(&pb)));
	gMyIP = pb.ourAddress;
#if qDebug
	fprintf(stderr, "My IP# is %ld.%ld.%ld.%ld\n", (gMyIP >> 24) & 255, (gMyIP >> 16) & 255, (gMyIP >> 8) & 255, gMyIP & 255);
#endif
	return gMyIP;
}

void GetMyDotName(CStr255 &myDotName)
{
	if (!gMacTcpRefNum)
	{
		myDotName = gMyDotName = "alone.in.world";
		return;
	}
	if (!gMyDotName.Length())
	{
		FailInfo fi;
		if (fi.Try())
		{
			IP2DotName(MyIP(), gMyDotName);
#if qDebug
			fprintf(stderr, "My dotname is %s\n", (char*)gMyDotName);
#endif
			fi.Success();
		}
		else // fail
		{
			if (fi.error == authNameErr)
				Failure(errMyMacHasNoDotName, fi.message);
			else
				fi.ReSignal();
		}
	}
	myDotName = gMyDotName;
}	

void IP2DotName(long ip, CStr255 &name)
{
#if qDebug
	char IP[50];
	sprintf(IP, "%ld.%ld.%ld.%ld", (ip >> 24) & 255, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);
	fprintf(stderr, "ip -> name of %s\n", IP);
#endif
	static IP2DotNameBusy = false;
	if (IP2DotNameBusy)
	{
#if qDebug
		fprintf(stderr, "Waits for previous IP2DotName to complete...\n");
#endif
		while (IP2DotNameBusy)
			gCurThread->YieldTime(); 
	}
	IP2DotNameBusy = true;
	Boolean wasDisabled = gCurProgress->SetAbortDisable(true);
	VOLATILE(wasDisabled);
	FailInfo fi;
	if (fi.Try())
	{
		static hostInfo theHostInfo; // may not be on stack, like gResolverDone!
		if (!gResolverIsOpen)
		{
#if qDebug
			fprintf(stderr, "Opens Resolver...\n");
#endif
			FailOSErr(OpenResolver(nil));
			gResolverIsOpen = true;
		}
		gResolverDone = false;
#if qDebug
		fprintf(stderr, "Executes ip -> name of %s\n", IP);
#endif
		OSErr err = AddrToName(ip, &theHostInfo, DNRResultProc, &gResolverDone);
		if (err == cacheFault)
		{
#if qDebug
			fprintf(stderr, "MacTCP asks nameserver about name for %s (cacheFault)\n", IP);
#endif
			while (!gResolverDone)
				gCurThread->YieldTime(); 
		}
		else 
			FailOSErr(err);
		err = short(theHostInfo.rtnCode);
#if qDebug
		if (err != noErr)
			fprintf(stderr, "Error from theHostInfo.rtnCode = %ld\n", long(err));
#endif
		if (err != cacheFault)
			FailOSErr(err);
		gCurProgress->SetAbortDisable(wasDisabled);
		theHostInfo.cname[254] = 0;
		name.Length() = strlen(theHostInfo.cname);
		BytesMove(theHostInfo.cname, &name[1], strlen(theHostInfo.cname));
		if (name.Length() && name[name.Length()] == '.')
			name.Length()--;
#if qDebug
		fprintf(stderr, "IP2DotName: %s -> %s\n", IP, (char*)name);
#endif
		long ll;
		if (sscanf(theHostInfo.cname, "%ld.%ld.%ld.%ld", &ll, &ll, &ll, &ll) == 4)
		{
#if qDebug
			fprintf(stderr, "Just got dotnumber as response, not accepted!\n");
#endif
			FailOSErr(authNameErr); // got dotnumber and not name...
		}
		IP2DotNameBusy = false;
		fi.Success();
	}
	else // fail
	{
		gCurProgress->SetAbortDisable(wasDisabled);
		IP2DotNameBusy = false;
		fi.ReSignal();
	}
}

long DotName2IP(const CStr255 &name)
{
	char cname[300];
	cname[name.Length()] = 0;
	BytesMove((char*)name, cname, name.Length());
	// can't abort DNR calls...
	Boolean wasDisabled = gCurProgress->SetAbortDisable(true);
	VOLATILE(wasDisabled);
	FailInfo fi;
	if (fi.Try())
	{
		if (!gResolverIsOpen)
		{
			FailOSErr(OpenResolver(nil));
			gResolverIsOpen = true;
		}
		gResolverDone = false;
		static hostInfo theHostInfo; // may not be on the stack, like gResolverDone!
		short err = StrToAddr(name, &theHostInfo, &DNRResultProc, &gResolverDone);
		if (err == cacheFault)
		{
#if qDebug
			fprintf(stderr, "MacTCP asks nameserver about IP for %s\n", (char*)name);
#endif
			err = noErr;
			while (!gResolverDone)
				gCurThread->YieldTime(); 
		}
		FailOSErr(err);
		err = short(theHostInfo.rtnCode);
		if (err != cacheFault)
			FailOSErr(err);
		long ip = theHostInfo.addr[0];
		if (ip == 0 || ip == 0xFFFFFFFF)
			FailOSErr(authNameErr);
		gCurProgress->SetAbortDisable(wasDisabled);
#if qDebug
		fprintf(stderr, "DotName2IP: %s -> %ld.%ld.%ld.%ld\n", (char*)name, (ip >> 24) & 255, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);
#endif
		fi.Success();
		return ip;
	}
	else // fail
	{
		gCurProgress->SetAbortDisable(wasDisabled);
		fi.ReSignal();
	}
}
	

//=================================== PanicKill ============================================
#pragma push
#pragma trace off
#pragma segment A5Ref

pascal void KillMacTCPStuff() // must be pascal due to patch code
{
	if (!gMacTCPList) // before patching A5!
		return; // ups
#if !qModelFarData
#error "This code only works as model far data"
#endif
	long oldA5 = SetA5(gNuntiusA5); // it works as gNuntiusA5 is referenced as 32 bit imm. addr
	FailInfoPtr oldTopHandler = gTopHandler;
	gTopHandler = nil;
	for (ArrayIndex index = gMacTCPList->fSize; index > 0; index--)
	{
		PTcpStream *mtcp = *(PTcpStream**)gMacTCPList->TLongintList::ComputeAddress(index);
		if (IsPtrObject(mtcp))
			mtcp->CloseConnection(false);
	}
	gTopHandler = oldTopHandler;
	SetA5(oldA5);
}
#pragma pop
//=================================== PTcpStream ============================================


#pragma segment MyComm
PTcpStream::PTcpStream()
{
	fConnectionIsOpen = false;
	fStreamIsOpen = false;
	fStreamP = nil; // not mine
	fStreamBufferP = nil;
	fHostIP = 0;
	fHostPort = 0;
}

void PTcpStream::ITcpStream(long timeoutSecs)
{
	FailInfo fi;
	if (fi.Try())
	{
		fMacTCPID = ++gMacTCPID;
		gMacTCPList->InsertLast(long(this));
		fTimeoutTicks = timeoutSecs * 60;
		fTimeoutSecs = timeoutSecs;

		BlockSet(Ptr(&fStreamPB), sizeof(fStreamPB), 0);
		BlockSet(Ptr(&fOpenPB), sizeof(fOpenPB), 0);
		BlockSet(Ptr(&fClosePB), sizeof(fClosePB), 0);
		BlockSet(Ptr(&fAbortPB), sizeof(fAbortPB), 0);
		BlockSet(Ptr(&fReceivePB), sizeof(fReceivePB), 0);
		BlockSet(Ptr(&fSendPB), sizeof(fSendPB), 0);
		BlockSet(Ptr(&fStatusPB), sizeof(fStatusPB), 0);

		fReceivePB.ioResult = noErr; // signal that previous command has ended
		fReceivePB.csParam.receive.rcvBuffLen = 0; // nothing received

		fWdsEntry[0].ptr = nil;
		fWdsEntry[0].length = 0;
		fWdsEntry[1].ptr = nil;
		fWdsEntry[1].length = 0;
	
		fStreamBufferP = NewPermPtr(kStreamBufferSize);
		if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, created at $%lx\n", fMacTCPID, this);
		fi.Success();
	}
	else // fail
	{
		FreeIfPtrObject(this);
		fi.ReSignal();
	}
}

PTcpStream::~PTcpStream()
{
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, Going disposing\n", fMacTCPID);
	gMacTCPList->Delete(long(this));
	if (fStreamBufferP)
		CloseConnection(false);
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, Disposed.\n", fMacTCPID);
	fStreamBufferP = DisposeIfPtr(fStreamBufferP);
}

//=================================== Tools ============================================
void PTcpStream::WaitForCompletion(TCPiopb &pb)
{
	FailInfo fi;
	if (fi.Try())
	{
		if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, WaitForCompl\n", fMacTCPID);
		while (pb.ioResult == inProgress || pb.ioResult == connectionClosing)
			gCurThread->YieldTime();
		if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, WaitForCompl done\n", fMacTCPID);
		fi.Success();
	}
	else // fail
	{
#if qDebug
		fprintf(stderr, "Aborts MacTCP operation\n");
#endif
		fi.ReSignal();
	}
}
//================= STREAM ==========================================
#pragma push
#pragma trace off
#pragma segment A5Ref
void pascal DummyASR(StreamPtr, unsigned short,
		Ptr, unsigned short, struct ICMPReport *)
{
}
#pragma pop

void PTcpStream::TCPCreateStream()
{
	if (fStreamIsOpen)
	{
		ReportFatalError("PTcpStream::TCPCreateStream, has already opened a stream");
		Failure(minErr, 0);
	}
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, Creates Stream\n", fMacTCPID);
	BlockSet(Ptr(&fStreamPB), sizeof(fStreamPB), 0);
	fStreamPB.csCode = TCPCreate;
	fStreamPB.ioCRefNum = gMacTcpRefNum;
	fStreamPB.csParam.create.rcvBuff = fStreamBufferP;
	fStreamPB.csParam.create.rcvBuffLen = kStreamBufferSize;
	fStreamPB.csParam.create.notifyProc = &DummyASR;
	PBControlSync(ParmBlkPtr(&fStreamPB));
	FailOSErr(fStreamPB.ioResult);
	fStreamP = fStreamPB.tcpStream;
	fStreamIsOpen = true;
}

void PTcpStream::TCPReleaseStream()
{
	if (!fStreamIsOpen)
		return;
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, Releases Stream\n", fMacTCPID);
	fStreamIsOpen = false;
	BlockSet(Ptr(&fStreamPB), sizeof(fStreamPB), 0);
	fStreamPB.csCode = TCPRelease;
	fStreamPB.ioCRefNum = gMacTcpRefNum;
	fStreamPB.tcpStream = fStreamP;
	fStreamP = nil; // not mine
	PBControlSync(ParmBlkPtr(&fStreamPB));
	FailOSErr(fStreamPB.ioResult);
}

//================= OPEN/CLOSE ==========================================
void PTcpStream::TCPOpenConnection()
{
	if (fOpenPB.ioResult != noErr)
	{
		ReportFatalError("PTcpStream::TCPOpenConnection, fOpenPB is already busy");
		Failure(minErr, 0);
	}
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, Opens connection\n", fMacTCPID);
	BlockSet(Ptr(&fOpenPB), sizeof(fOpenPB), 0);
	fOpenPB.csCode = TCPActiveOpen;
	fOpenPB.ioResult = 1;
	fOpenPB.ioCRefNum = gMacTcpRefNum;
	fOpenPB.tcpStream = fStreamP;
	fOpenPB.csParam.open.ulpTimeoutValue = (unsigned char)Min(200, fTimeoutSecs);
	fOpenPB.csParam.open.ulpTimeoutAction = 1;
	fOpenPB.csParam.open.validityFlags = 0xC0;
	fOpenPB.csParam.open.commandTimeoutValue = (unsigned char)Min(200, fTimeoutSecs);
	fOpenPB.csParam.open.remoteHost = fHostIP;
	fOpenPB.csParam.open.remotePort = fHostPort;
	fOpenPB.csParam.open.localPort = 0;
	fOpenPB.csParam.open.tosFlags = 0;
	fOpenPB.csParam.open.precedence = 0;
	fOpenPB.csParam.open.dontFrag = 0;
	fOpenPB.csParam.open.timeToLive = 0;
	fOpenPB.csParam.open.security = 0;
	fOpenPB.csParam.open.optionCnt = 0;
	long startTick = TickCount();
	FailOSErr(PBControlAsync(ParmBlkPtr(&fOpenPB)));
	WaitForCompletion(fOpenPB);
	long ticks = TickCount() - startTick; // made because MacTCP returns timeOut as openFailed
	if (ticks >= fTimeoutTicks - 30)
		FailOSErr(errOpenCommandTimeout);
	else
		FailOSErr(fOpenPB.ioResult);
	fConnectionIsOpen = true;
}

// CloseConnection aborts the connection if it could not be closed
void PTcpStream::TCPCloseConnection(Boolean async)
{
	fConnectionIsOpen = false;
	if (gCurThread->IsAborted()) // don't yield if we get aborted in first second
		async = false;
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, Closes connection, async = %ld\n", fMacTCPID, long(async));
	BlockSet(Ptr(&fClosePB), sizeof(fClosePB), 0);
	fClosePB.csCode = TCPClose;
	fClosePB.ioCRefNum = gMacTcpRefNum;
	fClosePB.tcpStream = fStreamP;
	if (async)
		fClosePB.csParam.close.ulpTimeoutValue = (unsigned char)Max(20, fTimeoutSecs);
	else
		fClosePB.csParam.close.ulpTimeoutValue = 2;
	fClosePB.csParam.close.validityFlags = 0xC0;
	fClosePB.csParam.close.ulpTimeoutAction = 1; // 0 == report, 1 == abort
	OSErr err = PBControl(ParmBlkPtr(&fClosePB), async);
	if (err == noErr && async)
	{
		WaitForCompletion(fClosePB);
		err = fClosePB.ioResult;
	}
	if (err != connectionDoesntExist && err != connectionTerminated) 
		FailOSErr(err);
}

void PTcpStream::TCPAbortConnection()
{
#if qDebug
	fprintf(stderr, "Aborts TCP/IP connection\n");
#endif
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, Aborts connection\n", fMacTCPID);
	BlockSet(Ptr(&fAbortPB), sizeof(fAbortPB), 0);
	fAbortPB.csCode = TCPAbort;
	fAbortPB.ioCRefNum = gMacTcpRefNum;
	fAbortPB.tcpStream = fStreamP;
	PBControlSync(ParmBlkPtr(&fAbortPB));
	if (fAbortPB.ioResult == noErr || fAbortPB.ioResult == connectionDoesntExist || fAbortPB.ioResult == connectionTerminated) 
		return;
#if qDebug
	fprintf(stderr, "Got error when aborting connection: %ld\n", long(fAbortPB.ioResult));
#endif
	FailOSErr(fAbortPB.ioResult);
}

void PTcpStream::TCPCloseOrAbortConnection(Boolean async)
{
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, Close || Aborts connection\n", fMacTCPID);
	FailInfo fi;
	if (fi.Try())
	{
		TCPCloseConnection(async);
		fi.Success();
		return;
	}
	else // fail
	{
#if qDebug
		fprintf(stderr, "PTcpStream::TCPCloseOrAbortConnection, error at close = %ld\n", long(fi.error));
#endif
	}
	// now try with abort
	if (fi.Try())
	{
		TCPAbortConnection();
		fi.Success();
		return;
	}
	else // fail
	{
#if qDebug
		fprintf(stderr, "PTcpStream::TCPCloseOrAbortConnection, error at abort = %ld\n", long(fi.error));
#endif
	}
}
//=========== LOW LEVEL SEND/RECEIVE ===================================================
void PTcpStream::TCPSendData()
{	
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, TCPSendData\n", fMacTCPID);
	BlockSet(Ptr(&fSendPB), sizeof(fSendPB), 0);
	fSendPB.csCode = TCPSend;
	fSendPB.ioCRefNum = gMacTcpRefNum;
	fSendPB.tcpStream = fStreamP;
	fSendPB.csParam.send.ulpTimeoutValue = (unsigned char)Min(200, fTimeoutSecs);
	fSendPB.csParam.send.ulpTimeoutAction = 1; // 0 == report, 1 == abort
	fSendPB.csParam.send.validityFlags = 0xC0;
	fSendPB.csParam.send.pushFlag = true; // send it at once
	fSendPB.csParam.send.urgentFlag = false;
	fSendPB.csParam.send.wdsPtr = Ptr(fWdsEntry);
	FailOSErr(PBControlAsync(ParmBlkPtr(&fSendPB)));
	WaitForCompletion(fSendPB);
	FailOSErr(fSendPB.ioResult);
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, TCPSendData done\n", fMacTCPID);
}

void PTcpStream::TCPReceiveData(void *bufferP, unsigned short bufferSize,
									short &bytesReceived)
{
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, TCPReceiveData\n", fMacTCPID);
	bytesReceived = 0;
	BlockSet(Ptr(&fReceivePB), sizeof(fReceivePB), 0);
	fReceivePB.csCode = TCPRcv;
	fReceivePB.ioCRefNum = gMacTcpRefNum;
	fReceivePB.tcpStream = fStreamP;
	fReceivePB.csParam.receive.commandTimeoutValue = 10;
	fReceivePB.csParam.receive.rcvBuff = Ptr(bufferP);
	fReceivePB.csParam.receive.rcvBuffLen = bufferSize;
	FailOSErr(PBControlSync(ParmBlkPtr(&fReceivePB)));
	FailOSErr(fReceivePB.ioResult);
	bytesReceived = fReceivePB.csParam.receive.rcvBuffLen;
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, TCPReceiveData done, got %ld\n", fMacTCPID, long(bytesReceived));
}

long PTcpStream::TCPNumBytesAvailable()
{
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, TCPNumBytesAvailable\n", fMacTCPID);
	BlockSet(Ptr(&fStatusPB), sizeof(fStatusPB), 0);
	fStatusPB.csCode = TCPStatus;
	fStatusPB.ioCRefNum = gMacTcpRefNum;
	fStatusPB.tcpStream = fStreamP;
	FailOSErr(PBControlSync(ParmBlkPtr(&fStatusPB)));
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, TCPNumBytesAvailable done = %ld\n", fMacTCPID, long(fStatusPB.csParam.status.amtUnreadData));
	return fStatusPB.csParam.status.amtUnreadData;
}

void PTcpStream::DebugCheckWds()
{
#if qDebug
	Boolean badWDS = false;
	if (fWdsEntry[1].ptr || fWdsEntry[1].length)
	{
		fprintf(stderr, "fWdsEntry[1].ptr at $%lx is invalid\n", fWdsEntry);
		badWDS = true;
	}
	if (fWdsEntry[0].ptr == 0 || fWdsEntry[0].length <= 0 || fWdsEntry[0].length > 32000)
	{
		fprintf(stderr, "fWdsEntry[0] at $%lx is invalid\n", fWdsEntry);
		badWDS = true;
	}
	if (badWDS)
	{
		fprintf(stderr, "WDS: \n");
		fprintf(stderr, " 0: ptr = $%lx, len = %ld\n", fWdsEntry[0].ptr, fWdsEntry[0].length);
		fprintf(stderr, " 1: ptr = $%lx, len = %ld\n", fWdsEntry[1].ptr, fWdsEntry[1].length);
		ProgramBreak(gEmptyString);
	}
#endif
}

//========= HighLevel Open/Close ============================================================

void PTcpStream::OpenConnection(long hostIP, short port)
{
	if (fConnectionIsOpen)
		return;
	if (!fStreamIsOpen)
		TCPCreateStream();
	fHostIP = hostIP;
	fHostPort = port;
	TCPOpenConnection();
}

void PTcpStream::CloseConnection(Boolean async)
{
	FailInfo fi;
	if (fConnectionIsOpen) 
	{
		if (fi.Try())
		{
			TCPCloseOrAbortConnection(async);
			fi.Success();
		}
		else // fail
		{
			switch (fi.error) 
			{
				case connectionTerminated:
				case connectionDoesntExist:
					break; // ignore them
				default:
#if qDebug
					fprintf(stderr, "PTcpStream::CloseConnection, very unexpected: got signal when closing connection, err = %ld\n", fi.error);
#endif
					break;
			}
		}
	}
	fHostIP = 0;
	fHostPort = 0;
	if (fStreamIsOpen)
	{
		if (fi.Try())
		{
			TCPReleaseStream();
			fi.Success();
		}
		else // fail
		{
#if qDebug
			fprintf(stderr, "PTcpStream::CloseConnection, very unexpected: got signal when releasing stream, err = %ld\n", fi.error);
#endif
		}
	}
}

//=========== HIGH LEVEL SEND/RECEIVE ==================================================

void PTcpStream::SendData(void *bufferP, long len)
{
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, SendData of %ld\n", fMacTCPID, len);
	const char *p = (const char*)bufferP;
	const long kMaxPartLen = 8 * 1024;
	while (len > 0) 
	{
		short partLen = short(Min(len, kMaxPartLen));
		fWdsEntry[0].ptr = p;
		fWdsEntry[0].length = partLen;
		fWdsEntry[1].ptr = 0;
		fWdsEntry[1].length = 0;
		macroCheck(DebugCheckWds());
		TCPSendData();
		macroCheck(DebugCheckWds());
		p += partLen;
		len -= partLen;
	}
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, SendData done\n", fMacTCPID);
}

void PTcpStream::ReceiveSomeData(void *bufferP, long bufferSize, long &bytesReceived)
{
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, ReceiveSomeData\n", fMacTCPID);
	bufferSize = Min(30000, bufferSize); // is short in MacTCP
	while (true)
	{
		long toRead = Min(bufferSize, TCPNumBytesAvailable());
		if (toRead > 0)
		{
			short noBytes;
			TCPReceiveData(bufferP, short(toRead), noBytes);
			bytesReceived = noBytes;
			if (bytesReceived)
				break;
		}
		gCurThread->YieldTime();
	}
	if (qDebugActivity) fprintf(stderr, "MacTCP id %ld, ReceiveSomeData done = %ld\n", fMacTCPID, bytesReceived);
}
