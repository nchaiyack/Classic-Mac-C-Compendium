// tcp easy.c
// Darrell Andesron

#include "tcp easy.h"
#include "easy tcp structs.h"
#include "waitfor.h"

#include "MacTCPCommonTypes.h"
#include "TCPPB.h"
#include "CvtAddr.h"

// a reference to the MacTCP driver.
short TCPdriver;

// open the MacTCP driver.  MUST be done before other routines will work.
OSErr EasyTCPInit(void) {	
	OSErr err;

	// open the driver and remember it's reference number
	err = OpenDriver("\p.IPP",&TCPdriver);
	return(err);
}

OSErr EasyTCPCreate(StreamPtr *stream, int timeout, int buffersize) {
	TCPiopb *pBlock;
	OSErr err;
	Ptr buffer;

	// validate the timeout value
	if(timeout < 0) timeout = DEFAULT_TIMEOUT;
	if(buffersize < 4096) buffersize = DEFAULT_BUFFERSIZE;

	// allocate a parameter block
	pBlock = (TCPiopb *)NewPtr(sizeof(TCPiopb));	
	if (MemError() != noErr)
		return MemError();	
	pBlock->ioCompletion = 0L;
	pBlock->ioCRefNum = TCPdriver;

	// first create a buffer for the stream
	buffer = NewPtr(buffersize);
	if (MemError() != noErr)
		return MemError();		
	
	// get ready to create the stream
	pBlock->csCode = TCPCreate;
	pBlock->ioResult = 1;
	pBlock->csParam.create.rcvBuff = buffer;
	pBlock->csParam.create.rcvBuffLen = buffersize;
	pBlock->csParam.create.notifyProc = 0; // no notification procedure specified!
	// create the stream
	PBControl((ParmBlkPtr)pBlock,true);
	WaitFor(&(pBlock->ioResult));
	// remember the stream info		
	*stream = pBlock->tcpStream;
	err = pBlock->ioResult;

	DisposPtr((Ptr)pBlock);
	return(err);	
}

OSErr EasyTCPRelease(StreamPtr stream, int timeout) {
	TCPiopb *pBlock;
	Ptr streamBuffer;
	OSErr err;

	// allocate a parameter block
	pBlock = (TCPiopb *)NewPtr(sizeof(TCPiopb));	
	if (MemError() != noErr)
		return MemError();	
	pBlock->ioCompletion = 0L;
	pBlock->ioCRefNum = TCPdriver;
	
	// setup the parameter block for a release
	pBlock->csCode = TCPRelease;
	pBlock->ioResult = 1;
	pBlock->tcpStream = stream;
	// release the stream
	PBControl((ParmBlkPtr)pBlock,true);
	WaitFor(&(pBlock->ioResult));

	// something wrong?
	err = pBlock->ioResult;
	if(err!=noErr) return(err);
	
	// get and dispose of the stream's local buffer
	streamBuffer = pBlock->csParam.create.rcvBuff;
	DisposPtr(streamBuffer);
	
	// free up used memory
	DisposePtr((Ptr)pBlock);

	return(noErr);
}

OSErr EasyTCPOpen(StreamPtr stream, HostNPort *hostNport, int method, int timeout) {
	TCPiopb *pBlock;
	OSErr err;
	ip_addr ipAddr;
	
	// validate the timeout
	if(timeout < 0) timeout = DEFAULT_TIMEOUT;
	
	// allocate a parameter block
	pBlock = (TCPiopb *)NewPtr(sizeof(TCPiopb));	
	if (MemError() != noErr)
		return MemError();	
	pBlock->ioCompletion = 0L;
	pBlock->ioCRefNum = TCPdriver;
	
	// figure out the ip address of the host
	if(hostNport->hostname != '\0') {
		err = ConvertStringToAddr(hostNport->hostname,&ipAddr);
		if(err!=noErr) return(err);
	}
	
	// open the connection in the desired manner
	if(method == PASSIVE) {
		// setup the parameter block for passive open (ie listen/wait for a connection)
		pBlock->csCode = TCPPassiveOpen;
		pBlock->ioResult = 1;
		pBlock->ioCompletion = nil;
		pBlock->tcpStream = stream;
		pBlock->csParam.open.ulpTimeoutValue = timeout;
		pBlock->csParam.open.ulpTimeoutAction = 1;
		pBlock->csParam.open.validityFlags = 0xC0;
		pBlock->csParam.open.commandTimeoutValue = timeout;
		pBlock->csParam.open.remoteHost = 0; // accept from any machine
		pBlock->csParam.open.remotePort = 0; // from any remote port
		pBlock->csParam.open.localPort = hostNport->port;
		pBlock->csParam.open.tosFlags = 0;
		pBlock->csParam.open.precedence = 0;
		pBlock->csParam.open.dontFrag = 0;
		pBlock->csParam.open.timeToLive = 0;
		pBlock->csParam.open.security = 0;
		pBlock->csParam.open.optionCnt = 0;
		// wait and listen for the connection
		PBControl((ParmBlkPtr)pBlock,true);
		WaitFor(&(pBlock->ioResult));
		//remoteHost = pBlock->csParam.open.remoteHost;
		//remotePort = pBlock->csParam.open.remotePort;
		//localHost = pBlock->csParam.open.localHost;
		//localPort = pBlock->csParam.open.localPort;
		
		err = pBlock->ioResult;
		if(err!=noErr) return(err);
		
	} else if(method == ACTIVE) {
		// setup the paramater block for active open (ie go out and make the connection)
		pBlock->csCode = TCPActiveOpen;
		pBlock->ioResult = 1;
		pBlock->tcpStream = stream;
		pBlock->csParam.open.ulpTimeoutValue = timeout;
		pBlock->csParam.open.ulpTimeoutAction = 1;
		pBlock->csParam.open.validityFlags = 0xC0;
		pBlock->csParam.open.commandTimeoutValue = timeout;
		pBlock->csParam.open.remoteHost = ipAddr;
		pBlock->csParam.open.remotePort = hostNport->port;
		pBlock->csParam.open.localPort = 0; // pick an unused port
		pBlock->csParam.open.tosFlags = 0;
		pBlock->csParam.open.precedence = 0;
		pBlock->csParam.open.dontFrag = 0;
		pBlock->csParam.open.timeToLive = 0;
		pBlock->csParam.open.security = 0;
		pBlock->csParam.open.optionCnt = 0;
		// make the connection
		PBControl((ParmBlkPtr)pBlock,true);
		WaitFor(&(pBlock->ioResult));

		//localHost = pBlock->csParam.open.localHost;
		//localPort = pBlock->csParam.open.localPort;
			
		err = pBlock->ioResult;
		if(err!=noErr) return(err);
	} else return(1); // invalid method specified!
	// free up used memory
	DisposePtr((Ptr)pBlock);
	
	// at this point the stream is allocated and the connection is open
	return(noErr);
}

// close a connection
OSErr EasyTCPClose(StreamPtr stream, int timeout) {
	TCPiopb *pBlock;
	OSErr err;
		
	// validate the timeout
	if(timeout < 0) timeout = DEFAULT_TIMEOUT;

	// allocate a parameter block
	pBlock = (TCPiopb *)NewPtr(sizeof(TCPiopb));	
	if (MemError() != noErr)
		return MemError();	
	pBlock->ioCompletion = 0L;
	pBlock->ioCRefNum = TCPdriver;
		
	// setup the parameter block for a close
	pBlock->csCode = TCPClose;
	pBlock->ioResult = 1;
	pBlock->tcpStream = stream;
	pBlock->csParam.close.ulpTimeoutValue = timeout;
	pBlock->csParam.close.validityFlags = 0xC0;
	pBlock->csParam.close.ulpTimeoutAction = 1;
	// close the connection
	PBControl((ParmBlkPtr)pBlock,true);
	WaitFor(&(pBlock->ioResult));

	err = pBlock->ioResult;
	if(err!=noErr) {
		// no more mr. nice guy
		pBlock->csCode = TCPAbort;
		pBlock->ioResult = 1;
		pBlock->tcpStream = stream;
		// beat the connection to a pulp
		PBControl((ParmBlkPtr)pBlock,true);
	WaitFor(&(pBlock->ioResult));

		err = pBlock->ioResult;
		if(err!=noErr) return(err);
	}
	
	DisposPtr((Ptr)pBlock);
	
	// at this point the connection is closed (though we may have had to use force)
	return(noErr);
}

OSErr EasyTCPSend(StreamPtr stream, Ptr data, int dataLength, int timeout) {
	TCPiopb *pBlock;
	OSErr err;
	struct wdsEntry wds[2];

	// validate the timeout
	if(timeout < 0) timeout = DEFAULT_TIMEOUT;

	// make the Write Data Structure
	wds[0].length = dataLength;
	wds[0].ptr = data;
	wds[1].length = 0;
	wds[1].ptr = nil;
		
	// allocate a parameter block
	pBlock = (TCPiopb *)NewPtr(sizeof(TCPiopb));	
	if (MemError() != noErr)
		return MemError();	
	pBlock->ioCompletion = 0L;
	pBlock->ioCRefNum = TCPdriver;

	// prepare the parameter block for a send
	pBlock->csCode = TCPSend;
	pBlock->ioResult = 1;
	pBlock->tcpStream = stream;
	pBlock->ioCompletion = nil;
	pBlock->csParam.send.ulpTimeoutValue = timeout;
	pBlock->csParam.send.ulpTimeoutAction = 1;
	pBlock->csParam.send.validityFlags = 0xC0;
	pBlock->csParam.send.pushFlag = 0;
	pBlock->csParam.send.urgentFlag = 0;
	pBlock->csParam.send.wdsPtr = (Ptr)wds;
	// send the data
	PBControl((ParmBlkPtr)pBlock,true);
	WaitFor(&(pBlock->ioResult));

	err = pBlock->ioResult;

	DisposPtr((Ptr)pBlock);
	return(err);
}

OSErr EasyTCPReceive(StreamPtr stream, Ptr data, int bytesToRead, int *bytesRead, int timeout) {
	TCPiopb *pBlock;
	OSErr err;

	// validate the timeout
	if(timeout < 0) timeout = DEFAULT_TIMEOUT;

	// allocate a parameter block
	pBlock = (TCPiopb *)NewPtr(sizeof(TCPiopb));	
	if (MemError() != noErr)
		return MemError();	
	pBlock->ioCompletion = 0L;
	pBlock->ioCRefNum = TCPdriver;
	
	pBlock->csCode = TCPRcv;
	pBlock->ioResult = 1;
	pBlock->ioCompletion = nil;
	pBlock->tcpStream = stream;
	pBlock->csParam.receive.commandTimeoutValue = timeout;
	pBlock->csParam.receive.rcvBuff = data;
	pBlock->csParam.receive.rcvBuffLen = bytesToRead;
	// do the read
	PBControl((ParmBlkPtr)pBlock,true);
	WaitFor(&(pBlock->ioResult));

	// figure out how much we read
	*bytesRead = pBlock->csParam.receive.rcvBuffLen;
	//urgent = pBlock->csParam.receive.urgentFlag;
	//mark = pBlock->csParam.receive.markFlag;
	err = pBlock->ioResult;

	DisposPtr((Ptr)pBlock);
	return(err);
}


