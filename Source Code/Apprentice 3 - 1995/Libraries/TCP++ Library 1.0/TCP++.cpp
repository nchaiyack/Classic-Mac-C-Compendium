
#include "TCP++.h"
#include "Driver++.h"

#include "myUtils.h"

#include <Memory.h>

StreamPtr
tcpCreate(int bufSize, TCPNotifyUPP asr, Ptr usrData)
{
	OSErr	theErr;
	TCPiopb	pb;
	Ptr		buffer;
	
	buffer = NewPtr(bufSize);
	if (MemError() != noErr)
		return MemError();	

	pb.csCode		= TCPCreate;
	pb.ioCRefNum	= gRef.GetRef();
	pb.ioCompletion = nil;

	pb.csParam.create.rcvBuff		= buffer;
	pb.csParam.create.rcvBuffLen	= bufSize;
	pb.csParam.create.notifyProc 	= asr;
	pb.csParam.create.userDataPtr	= usrData;

	theErr = PBControlSync((ParmBlkPtr) &pb);
	
	if (theErr == noErr)
		return pb.tcpStream;
	else
		return (StreamPtr) theErr;
}

OSErr
tcpOpen(StreamPtr stream, ip_addr host, tcp_port port)
{
	TCPiopb	pb;

	BlockClear(&pb, sizeof(pb));

	pb.csCode		= TCPActiveOpen;
	pb.tcpStream	= stream;
	pb.ioCRefNum	= gRef.GetRef();
	
	pb.csParam.open.ulpTimeoutValue		= 5;
	pb.csParam.open.ulpTimeoutAction	= 0;
	pb.csParam.open.validityFlags		= 0xC0;
	pb.csParam.open.remoteHost			= host;
	pb.csParam.open.remotePort			= port;
	
	return PBControlSync((ParmBlkPtr) &pb);
}

OSErr
tcpOpenReserved(StreamPtr stream, ip_addr host, tcp_port port)
{
	TCPiopb		pb;
	tcp_port	local;
	OSErr		theErr;

	BlockClear(&pb, sizeof(pb));

	local = 1023;

	pb.csCode		= TCPActiveOpen;
	pb.tcpStream	= stream;
	pb.ioCRefNum	= gRef.GetRef();
	
	pb.csParam.open.ulpTimeoutValue		= 5;
	pb.csParam.open.ulpTimeoutAction	= 0;
	pb.csParam.open.validityFlags		= 0xC0;
	pb.csParam.open.remoteHost			= host;
	pb.csParam.open.remotePort			= port;
	
	do {
		pb.csParam.open.localPort		= local;
		
		theErr = PBControlSync((ParmBlkPtr) &pb);
		
		local--;
	} while ((theErr == duplicateSocket) && (local > 512));

	return theErr;
}

OSErr
tcpRead(StreamPtr stream, void* buffer, short* len)
{
	OSErr	theErr;
	TCPiopb	pb;
	
	BlockClear(&pb, sizeof(pb));
	
	pb.ioCRefNum	= gRef.GetRef();
	pb.tcpStream	= stream;
	pb.csCode		= TCPRcv;
	
	pb.csParam.receive.commandTimeoutValue	= 2;
	pb.csParam.receive.rcvBuff				= (Ptr) buffer;
	pb.csParam.receive.rcvBuffLen			= (unsigned short) *len;
	
	theErr = PBControlSync((ParmBlkPtr) &pb);
	
	if (theErr == noErr) 
		*len = pb.csParam.receive.rcvBuffLen;
	else
		*len = 0;
		
	return theErr;
}

OSErr
tcpWrite(StreamPtr stream, void* buffer, short len)
{
	typedef struct OneWDS {
		wdsEntry		wds;
		unsigned short	zero;
	} OneWDS;

	TCPiopb	pb;
	OneWDS	wds;
	
	BlockClear(&pb, sizeof(pb));

	wds.zero 		= 0;
	wds.wds.ptr 	= (char*)buffer;
	wds.wds.length	= len;

	pb.ioCRefNum	= gRef.GetRef();
	pb.tcpStream	= stream;
	pb.csCode 		= TCPSend;
	
	pb.csParam.send.ulpTimeoutValue		= 2;
	pb.csParam.send.ulpTimeoutAction	= 1;
	pb.csParam.send.validityFlags		= 0xC0;
	pb.csParam.send.wdsPtr				= (Ptr) &wds;
	
	return PBControlSync((ParmBlkPtr) &pb);
}

OSErr
tcpWriteWDS(StreamPtr stream, wdsEntry* wds)
{
	TCPiopb	pb;

	BlockClear(&pb, sizeof(pb));

	pb.ioCRefNum	= gRef.GetRef();
	pb.tcpStream	= stream;
	pb.csCode 		= TCPSend;
	
	pb.csParam.send.ulpTimeoutValue		= 2;
	pb.csParam.send.ulpTimeoutAction	= 1;
	pb.csParam.send.validityFlags		= 0xC0;
	pb.csParam.send.wdsPtr				= (Ptr) wds;
	
	return PBControlSync((ParmBlkPtr) &pb);
}

OSErr
tcpClose(StreamPtr stream)
{
	OSErr	theErr;
	TCPiopb	pb;

	pb.ioCompletion	= nil;
	pb.ioCRefNum	= gRef.GetRef();
	pb.tcpStream	= stream;
	pb.csCode 		= TCPClose;
	
	pb.csParam.close.ulpTimeoutValue 	= 5;
	pb.csParam.close.ulpTimeoutAction	= 1;
	pb.csParam.close.validityFlags		= 0xC0;
	pb.csParam.close.userDataPtr		= nil;
	
	theErr = PBControlSync((ParmBlkPtr) &pb);
	
	if (theErr == noErr) {
		pb.csCode = TCPRelease;
		theErr = PBControlSync((ParmBlkPtr) &pb);
	}
	
	if (theErr == noErr) {
		DisposePtr(pb.csParam.create.rcvBuff);
		theErr = MemError();
	}

	return theErr;
}

OSErr
tcpStatus(StreamPtr stream, TCPiopb* pb)
{
	pb->ioCRefNum	= gRef.GetRef();
	pb->tcpStream	= stream;
	pb->csCode		= TCPStatus;

	return PBControlSync((ParmBlkPtr) pb);
}

