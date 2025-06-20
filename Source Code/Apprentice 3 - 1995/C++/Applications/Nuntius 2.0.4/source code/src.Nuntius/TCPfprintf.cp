// Copyright � 1994-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// TCPfprintf.cp
 

#if qDebug

#include <MacTCP.h>

#include <Sound.h>
#include <Resources.h>
#include <GestaltEqu.h>

/*-------------- How To Use when running Nuntius: ----------------
To use:

1) Launch Telnet
2) Find the IP number of your mac (show network numbers in Telnet)
3) Launch Nuntius
    After some time, the splash screen will be hanging, and Nuntius will 
    emit strange sounds.
4) Switch to telnet by first clicking on the desktop
    (you can't use the menus while Nuntius is in front and
    waiting for the next thing to happend).
5) Open a connection to port 2000 on your mac, by choosing 
    "open connection" in Telnet, and write "127.0.0.16 2000"
    (substitute 127.0.0.16 by your IP number).

Nuntius should now be happy and write a bunch of stuff in the Telnet window. 
This window will not go away when Nuntius quit, you will have to close it
 and break the connection (don't worry, everything is safe).
*/

/*---------------- Interfacing to your code ----------------
Replace CWDebug.c and SIOX.Lib with this file.
*/

/*---------------- Operation ----------------
A MacTCP stream etc is created in the system heap
and registred with Gestalt, so:
1) it persist across multiple launches of the application
    (so you don't have to reopen the telnet window)
2) So it doesn't hurt when the application crashes and leaves
   the stream open (thanks for ZapTCP anyway).

The default setup creates a listener for socket 2000,
and accepts only connections from the mac running this
application. Fire the Firewalls!
*/

/*---------------- Control-key ----------------
If you hold it down:
Each time CWfprintf is called, the data is send, and 
the code sits waiting in a WaitNextEvent loop until all
of the data have been sent and acknowledged.
Well, the above is only true if the data contains
a '\n' (all those partial strings are plain uninteresting)
*/

/*---------------- MacTCP ----------------
How to set up MacTCP for a single mac which is
not connected to the Internet (v2.0.6):
	Select LocalTalk (or Ethernet)
	Obtain address: manual
	GateWay address: 0.0.0.0
	Class A
	Net: 127, Subnet: 0, Node: 16
	Domain name: delete all entries
*/

const long kTcpNubStreamBufferSize = 4096;

class CTcpNub // placed in system heap
{
	public:
		void SendData(void *bufferP, long len);

		void TCPCreateStream();
		void OpenConnection(long acceptIP, short port);
		void TCPAbortConnection();
		long MyIP();
		
		void *operator new(size_t size);
		void operator delete(void *p);
		
		CTcpNub(short macTcpRefNum);
	private:
		Boolean fConnectionIsOpen;
		Boolean fStreamIsOpen;
		char fStreamBuffer[kTcpNubStreamBufferSize];
		StreamPtr fStreamP;
		wdsEntry fWdsEntry[5];
		short fMacTcpRefNum;
		TCPiopb fOpenPB, fStatusPB, fSendPB;
		unsigned long fWaitStartTick;


		void TCPSendData(Boolean pushIt);
		void TCPGetStatus(TCPiopb &pb);

		void WaitForCompletion(TCPiopb &pb);
		void DoWait();
};

CTcpNub::CTcpNub(short macTcpRefNum)
	: fMacTcpRefNum(macTcpRefNum), fStreamIsOpen(false), fConnectionIsOpen(false)
{
}

void *CTcpNub::operator new(size_t size)
{
	Ptr aPtr = NewPtrSysClear(size);
	FailNIL(aPtr);
	return aPtr;
}

void CTcpNub::operator delete(void *p)
{
	if (p)
		DisposePtr(Ptr(p));
}

long CTcpNub::MyIP()
{
	if (!fMacTcpRefNum)
		return 0x01020304;
	GetAddrParamBlock pb;
	BlockSet(Ptr(&pb), sizeof(pb), 0);
	pb.csCode = ipctlGetAddr;
	pb.ioCRefNum = fMacTcpRefNum;
	FailOSErr(PBControlSync(ParmBlkPtr(&pb)));
	return pb.ourAddress;
}

void CTcpNub::DoWait()
{
	EventRecord event;
	WaitNextEvent(0, &event, 1, nil);
	if (TickCount() - fWaitStartTick > 180)
	{
		fWaitStartTick = TickCount();
		short oldRes = MAUseResFile(gCodeRefNum);
		long index = 1 + (TickCount() % Count1Resources('snd '));
		Handle h = Get1IndResource('snd ', index);
		MAUseResFile(oldRes);
		if (!h)
			return;
		HNoPurge(h);
		SndPlay(nil, SndListHandle(h), false);
		HPurge(h);
		fWaitStartTick = TickCount();
	}
}

void CTcpNub::WaitForCompletion(TCPiopb &pb)
{
	fWaitStartTick = TickCount();
	while (pb.ioResult == inProgress || pb.ioResult == connectionClosing)
		DoWait();
}

void CTcpNub::TCPCreateStream()
{
	if (fStreamIsOpen)
		return;
	TCPiopb pb;
	BlockSet(Ptr(&pb), sizeof(pb), 0);
	pb.csCode = TCPCreate;
	pb.ioCRefNum = fMacTcpRefNum;
	pb.csParam.create.rcvBuff = fStreamBuffer;
	pb.csParam.create.rcvBuffLen = kTcpNubStreamBufferSize;
	pb.csParam.create.notifyProc = nil; //&DummyASR;
	PBControlSync(ParmBlkPtr(&pb));
	FailOSErr(pb.ioResult);
	fStreamP = pb.tcpStream;
	fStreamIsOpen = true;
}

void CTcpNub::TCPAbortConnection()
{
	TCPiopb pb;
	BlockSet(Ptr(&pb), sizeof(pb), 0);
	pb.csCode = TCPAbort;
	pb.ioCRefNum = fMacTcpRefNum;
	pb.tcpStream = fStreamP;
	PBControlSync(ParmBlkPtr(&pb));
	if (pb.ioResult == noErr || pb.ioResult == connectionDoesntExist || pb.ioResult == connectionTerminated) 
		return;
}

void CTcpNub::TCPGetStatus(TCPiopb &pb)
{
	BlockSet(Ptr(&pb), sizeof(pb), 0);
	pb.csCode = TCPStatus;
	pb.ioCRefNum = fMacTcpRefNum;
	pb.tcpStream = fStreamP;
	PBControlSync(ParmBlkPtr(&pb));
}

void CTcpNub::OpenConnection(long acceptIP, short port)
{
	TCPGetStatus(fStatusPB);
	if (fStatusPB.csParam.status.connectionState == 8) // connection Established
		return;
	do
	{
		TCPGetStatus(fStatusPB);
		if (fStatusPB.csParam.status.connectionState != 0) // no connection exists
			TCPAbortConnection();
		BlockSet(Ptr(&fOpenPB), sizeof(fOpenPB), 0);
		fOpenPB.csCode = TCPPassiveOpen;
		fOpenPB.ioCRefNum = fMacTcpRefNum;
		fOpenPB.tcpStream = fStreamP;
		fOpenPB.csParam.open.ulpTimeoutValue = 30;
		fOpenPB.csParam.open.localPort = port;
		OSErr err = PBControlAsync(ParmBlkPtr(&fOpenPB));
		if (err)
		{
			DebugStr("\pGot error from TCPPassiveOpen");
			return;
		}
		WaitForCompletion(fOpenPB);
		err = fOpenPB.ioResult;
		if (err)
		{
			DebugStr("\pGot error from TCPPassiveOpen");
			return;
		}
	} while (acceptIP && fOpenPB.csParam.open.remoteHost != acceptIP);
}

void CTcpNub::TCPSendData(Boolean pushIt)
{	
	BlockSet(Ptr(&fSendPB), sizeof(fSendPB), 0);
	fSendPB.csCode = TCPSend;
	fSendPB.ioCRefNum = fMacTcpRefNum;
	fSendPB.tcpStream = fStreamP;
	fSendPB.csParam.send.ulpTimeoutValue = 20;
	fSendPB.csParam.send.ulpTimeoutAction = 1;
	fSendPB.csParam.send.validityFlags = 0xC0;
	fSendPB.csParam.send.pushFlag = pushIt;
	fSendPB.csParam.send.urgentFlag = false;
	fSendPB.csParam.send.wdsPtr = Ptr(fWdsEntry);
	OSErr err = PBControlAsync(ParmBlkPtr(&fSendPB));
	if (err)
		DebugStr("\pGot error from TCPSend");
	WaitForCompletion(fSendPB);
	err = fSendPB.ioResult;
	if (err)
		DebugStr("\pGot error from TCPSend");
}

void CTcpNub::SendData(void *bufferP, long len)
{
	Boolean hasCR = false;
	char *p = (char*)bufferP + len;
	while (true)
	{
		if (p == bufferP)
			break; // no '\n'
		if (*--p == '\n')
		{
			hasCR = true;
			break;
		}
	}
	p = (char*)bufferP;
	const long kMaxPartLen = 1 * 1024;
	while (len > 0) 
	{
		short partLen = short(Min(len, kMaxPartLen));
		fWdsEntry[0].ptr = Ptr(p);
		fWdsEntry[0].length = partLen;
		fWdsEntry[1].ptr = 0;
		fWdsEntry[1].length = 0;
		TCPSendData(hasCR && partLen == len);
		p += partLen;
		len -= partLen;
	}
	if (!hasCR)
		return;
	KeyMap km;
	GetKeys(km);
	if ((km[1] & 8) == 0)
		return; // control key not down
	fWaitStartTick = TickCount();
	while (TickCount() - fWaitStartTick < 60)
	{
		DoWait();
		TCPGetStatus(fStatusPB);
		if (fStatusPB.csParam.status.amtUnreadData == 0)
			break;
	}
}

//==================================================================================

CTcpNub *GetNub()
{
	static CTcpNub *sNub = nil;
	if (sNub == nil)
	{
		short refnum;
		OSErr errNo = OpenDriver("\p.IPP", &refnum);
		if (errNo != noErr)
		{
			DebugStr("\pCan't open MacTcp");
			return nil;
		}
		long response;
		errNo = Gestalt('dTCP', &response);
		if (errNo == noErr && response)
			sNub = (CTcpNub*)response;
		else
		{
			sNub = new CTcpNub(refnum);
			Ptr p = NewPtrSysClear(6 * 4);
			FailNIL(p);
			long *pc = (long*)p;
			*pc++ = 0x206F0004;
			*pc++ = 0x4e71203C;
			*pc++ = long(sNub);
			*pc++ = 0x2080426F;
			*pc++ = 0x000C205F;
			*pc++ = 0x508F4ED0;
			FlushCache();
			errNo = NewGestalt('dTCP', SelectorFunctionUPP(p));
			if (errNo != noErr)
			{
				DebugStr("\pCan't create gestalt");
				return nil;
			}
			sNub->TCPCreateStream();
		}
	}
	sNub->OpenConnection(sNub->MyIP(), 2000);	
	return sNub;
}

//==================================================================================

void CWInstallWriteLnHook()
{
	setvbuf(stderr, NULL, _IOLBF, 128);
}

Boolean CW_SIOUXHandleOneEvent(EventRecord * /* anEvent */)
{
	return 0;
}
	
short InstallConsole(short /* fd */)
{
	return noErr;
}
	
void RemoveConsole()
{
	return;
}
	
long ReadCharsFromConsole(char */* buffer */, long /* n */)
{
	return 0;
}

const long kGlobalBufferSize = 4096;
char *gBuffer = nil;
long gBufferChars = 0;
void FlushGlobalBuffer(CTcpNub *nub)
{
	if (gBuffer)
		nub->SendData(gBuffer, gBufferChars);
	gBufferChars = 0;
}
	
void PutCharInGlobalBuffer(CTcpNub *nub, char ch)
{
	if (gBuffer == nil && (gBuffer = new char[kGlobalBufferSize + 10]) == nil)
		return;
	gBuffer[gBufferChars++] = ch;
	if (gBufferChars == kGlobalBufferSize || ch == 10)
		FlushGlobalBuffer(nub);
}

long WriteCharsToConsole(char *buffer, long n)
{
	static Boolean skipOutput = false;
	if (skipOutput)
		return false;
	CTcpNub *nub = GetNub();
	if (!nub)
		return n;
	for (long i = 1; i <= n; i++)
	{
		PutCharInGlobalBuffer(nub, *buffer);
		if (*buffer == 13)
			PutCharInGlobalBuffer(nub, 10);
		buffer++;
	}
	return n;	
}

/*
#define TCPfprintf CWfprintf
int TCPfprintf(FILE *file, const char * format, ...)
{
	if (file == nil)
		return 0;
	va_list	ap = NULL;
	int result;
	if (file != stderr)
	{
	    va_start(ap, format );    
		result = vfprintf(file, format, ap);
		va_end(ap);
		return result;
	}
    char formatted[512];
    va_start(ap, format );    
	result = vsprintf(formatted, format, ap);
	va_end(ap);
	if ( result < 0 )
	{
		DebugStr("\pvsprintf failed\n");
		return 0;
	}
	static Boolean skipOutput = false;
	if (skipOutput)
		return false;
	CTcpNub *nub = GetNub();
	if (!nub)
		return 0;
	char *s = formatted;
	char tmp[200];
	char *d = tmp;
	while (*s)
	{
		if ((*d++ = *s++) == 13)
			*d++ = 10;
		if (d > tmp + 190)
		{
			nub->SendData(tmp, d - tmp);
			d = tmp;
		}
	}
	if (d > tmp)
		nub->SendData(tmp, d - tmp);
	return (result);	
}
*/
#endif
