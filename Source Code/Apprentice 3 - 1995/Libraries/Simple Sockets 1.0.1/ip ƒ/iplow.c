/*
 * iplow.c
 *
 * Low routines for MacTCP.  This file contains the code that talks
 * directly to the MacTCP driver.  I hope that when Apple changes
 * the IP interface around in system 8, this will be the only file
 * that requires modification.
 *
 * This should vaguely approximate socket_like calls. Only asynch-
 * ronous calls will be used.
 *
 * This library was strongly patterned after NewsWatcher, with the
 * blessing of John Norstad.
 *
 * Mike Trent 8/94
 *
 */

/* ANSI Includes */
#include <string.h> 

/* Additional Mac Includes */

#include <MacTCPCommonTypes.h>
#include <AddressXlation.h>
#include <MiscIPPB.h>
#include <UDPPB.h>
#include <TCPPB.h>
#include <GetMyIPAddr.h>

/* Local Includes */

#include "ip.h"
#include "iplow.h"
#include "butil.h"

/* constants for this module */

#define kBufferLength 32767 			/*  32 k  */
//#define kBufferLength 65535			/* unsigned short */

/* Global to this Module */

static short gRefNum;   /* refnum for talking to MacTCP */
Spin gSpin;				/* chosen spin routine */

/* Private Function headers */

pascal void myResultProc (hostInfo *hi, char *data);	/*private*/
pascal void MyTCPNotifyProc(StreamPtr stream, unsigned short eventCode, Ptr data, 
							unsigned short termr, struct ICMPReport *icmpMsg);
pascal void MyUDPNotifyProc(StreamPtr stream, unsigned short eventCode, Ptr data, 
							struct ICMPReport *icmpMsg);
OSErr SpinDefault (void);
void InitUDPPB (UDPiopb *pb);



/**
 **
 **  GENERIC MacTCP ROUTINES
 **
 **/


/* LowInitMacTCP 
 * - Inits the MacTCP driver. It should be called once near the 
 *	 beginning of the program. 
 * Returns OSErr: from OpenDriver
 */

OSErr LowInitMacTCP (void)
{
	OSErr	err;
	
	err = OpenDriver("\p.IPP",&gRefNum);
	return(err);
}

void LowSetSpin(Spin spinRoutine)
{
	if (spinRoutine == nil)
		gSpin = SpinDefault;
	else
		gSpin = spinRoutine;
}

/* LowSpin
 * - Invokes Spin Routine
 */
void LowSpin(void)
{
	(void)(*gSpin)();
}

/* SpinDefault
 * - a routine that handles events while waiting for a connection or
 *   somesuch.  Currently, it should just call some event routine
 *   so that other processes can function properly.
 *
 *	 Strongly patterned after GiveTime() in NewsWatcher.
 */
 
OSErr SpinDefault (void)
{
	EventRecord ev;
	Boolean gotEvt;
	short part;
	WindowPtr theWindow;

	gotEvt = WaitNextEvent(everyEvent,&ev,0,nil);
	if ( gotEvt )
		switch (ev.what) {
			case mouseDown:
				part = FindWindow(ev.where, &theWindow);
				if (part == inSysWindow) 
					SystemClick(&ev, theWindow);
				break;
			case activateEvt:
				break;
			case updateEvt:
				break;
			case app4Evt:
				break;
			case keyDown:
			case autoKey:
				if ((ev.message & charCodeMask) == 0x1B) {
					return -1;
				}
				break;
		}
	return 0;
}


/* LowStringToAddr
 * - Performs a DNS lookup for the host 'name'.
 *		*name  - the name of the host. This can be in DNS name form
 *				 or in "dot notation". The "official" name of the
 *				 host will be returned.
 *		*ipNum - returned ip number.
 * Returns OSErr: StrToAddr call.
 */
 
OSErr LowStringToAddr(char *name, unsigned long *ipNum)
{
	hostInfo hi;
	char done = 0x00;
	ResultUPP myResultUPP;
	OSErr err;
	
	if ((err = OpenResolver(nil)) != noErr)	
		return err;

	myResultUPP = NewResultProc(myResultProc);

	err = StrToAddr(name, &hi, myResultUPP, &done) ;
	if (err == cacheFault) {
		while (!done) 
			if ((*gSpin)()){
				err = 1;
				goto foo;
			}
	} else if (err != noErr) goto foo;
	
	if ((hi.rtnCode == noErr) || (hi.rtnCode == cacheFault)) {
		*ipNum = hi.addr[0];
		strcpy(name, hi.cname);
		name[strlen(name) -1] = '\0';
	}
	err = hi.rtnCode;
		
foo:
	(void) CloseResolver();
	return err;
}

/* LowAddrToName
 * - Provided an ipNum, LowAddrToName will look up the DNS name for
 *   that address; if said name exists.
 *		ipNum - ip address.
 *		*name - returned DNS name.
 * Returns OSErr: AddrToName call
 */

OSErr LowAddrToName(unsigned long ipNum, char *name)
{	
	hostInfo hi;
	char done = 0x00;
	ResultUPP myResultUPP;
	OSErr err;
	
	if ((err = OpenResolver(nil)) != noErr)	
		return err;

	myResultUPP = NewResultProc(myResultProc);

	err = AddrToName(ipNum, &hi, myResultUPP, &done);
	if (err == cacheFault) {
		while(!done) 
			if ((*gSpin)()) {
				err = 1;
				goto foo;
			}

	} else if (err != noErr) goto foo;
	
	if ((hi.rtnCode == noErr) || (hi.rtnCode == cacheFault)) {
		strcpy(name, hi.cname);
		name[strlen(name)-1] = '\0';
		err = noErr;
	} else  err = hi.rtnCode;
	
foo:
	(void) CloseResolver();
	return err;
}


/* myResultProc
 * - ProcPtr (UPP) called when DNS routines cacheFault.
 *   See MacTCP Dev Kit.
 * 		*hi   - hostInfo record returned by driver.
 *		*data - user pointer ... used to flag when data is
 *				valid.
 */ 

pascal void myResultProc(hostInfo *hi, char *data)
{
#pragma unused (hi)
	*data = 0xff;
}


/* LowGetMyIP
 * - Returns local machines IP Number.
 *		*ipNum - returned IP Number
 * Returns OSErr: PBControl
 */

OSErr LowGetMyIP (unsigned long *ipNum)
{
	OSErr err;
	struct GetAddrParamBlock ippb;
	
	bzero ((char *)&ippb, sizeof(struct GetAddrParamBlock));

	ippb.csCode = ipctlGetAddr;
	ippb.ioCRefNum = gRefNum;
	
	err = PBControl((ParmBlkPtr)&ippb, true);
	if (err != noErr) return err;
	
	while (ippb.ioResult > 0) 
		if ((*gSpin)())
			return 1;

	if ((err = ippb.ioResult) != noErr) return err;
	
	*ipNum = ippb.ourAddress;
	
	return err;
}


/**
 **
 **  UDP/IP MacTCP ROUTINES
 **
 **/

/* As of 4/10/95, these routines have not been rigorously tested. /*

/* InitUDPPB
 * - A utility routine to zero a UDPiopb structure, and set some
 *	 global initializations.
 *		*pb - UDPiopb to be initialized
 */

void InitUDPPB (UDPiopb *pb)
{
	bzero ((char *)pb, sizeof(UDPiopb));
	pb->ioResult = 1;
	pb->ioCRefNum = gRefNum;
}

/* LowUDPCreate
 * - Creates UDP stream asynchronously.
 *     *stream - returns the UDP stream pointer.
 *	   *port   - port number requested, 0 = any. If *port = 0, the port assigned
 *				 will be returned in *port.  If port = nil, any port will be used,
 *				 but nothing will be returned (read: no bus error).
 *	   *sockets- this is ip.c specific - an unfortunate kludge.
 * Returns OSErr: MemErr or PBControl
 */

OSErr LowUDPCreate (StreamPtr *stream, unsigned short *port, mac_socket *sockets)
{
	UDPiopb pb;
	OSErr err = noErr;
	Ptr buffer;
		
	buffer = NewPtr(kBufferLength);
	if ((err = MemError()) != noErr) {
		return (err);
	}
	
	InitUDPPB(&pb);
	
	pb.csCode = UDPCreate;
	pb.csParam.create.rcvBuff = buffer;
	pb.csParam.create.rcvBuffLen = kBufferLength;
	pb.csParam.create.notifyProc = (UDPNotifyUPP)NewUDPNotifyProc(MyUDPNotifyProc);
	pb.csParam.create.localPort = (port != nil) ? *port: 0;
	pb.csParam.create.userDataPtr = (Ptr)sockets;
	
	err = PBControl((ParmBlkPtr)&pb,true);
	while (pb.ioResult > 0) (*gSpin)();

	*stream = pb.udpStream;
	if ((port != nil) && (*port == 0)) *port = pb.csParam.create.localPort;
	err = pb.ioResult;
	
	return err;
}

/* LowUDPRead
 * - reads from a UDP stream.
 *     stream      - the UDP stream ptr;
 *     timeout     - the number of seconds MacTCP should wait before timing out; 
 *				     0 = no timeout;
 *	   buf         - a pointer to memory for output to be written into;
 *	   *buflen     - the size of the memory pointed to by buf.  When LowUDPRead
 *				     returns, *buflen stores the number of bytes actually read;
 *	   *remoteHost - IP number of the host to read from.  If *remoteHost = 0,
 *				     then any host will be accepted, and their ip number will
 *				     be stored in *remoteHost upon completion;
 *	   *remotePort - like *remoteHost above.
 * Returns OSErr:  PBControl or ioResult.
 */
 
OSErr LowUDPRead (StreamPtr stream, char timeout, Ptr buf, int *buflen, 
					unsigned long *remoteHost, unsigned short *remotePort)
{
	UDPiopb pb;
	OSErr err = noErr;

	InitUDPPB(&pb);
	
	pb.csCode = UDPRead;
	pb.udpStream = stream;
	pb.csParam.receive.timeOut = timeout;
	
	if ((err = PBControl((ParmBlkPtr)&pb,true)) != noErr)
		return err;
	
	while (pb.ioResult > 0) 
		if ((*gSpin)())
			return 1;

	
	*buflen = pb.csParam.receive.rcvBuffLen;
	bcopy ((char *)pb.csParam.receive.rcvBuff, (char *)buf, (long)*buflen);
	*remoteHost = pb.csParam.receive.remoteHost;
	*remotePort = pb.csParam.receive.remotePort;
	
	err = pb.ioResult;
	
	pb.csCode = UDPBfrReturn;				// Instead of returning right away,
	pb.ioResult = 1;						// we need to return the buffer ptr
	(void)PBControl((ParmBlkPtr)&pb, true);	// to the driver.
	while (pb.ioResult>0)
		if ((*gSpin)())
			return 1;
	
	return err;
}

/* LowUDPWrite
 * Writes Data to a UDP stream.
 *    stream     - the UDP stream ptr;
 *	  wdsPtr     - a wdsPtr: see MacTCP Developer's Kit;
 *	  remoteHost - IP number of remote host to write to;
 *	  remotePort - port number to write to.
 * Returns OSErr: PBControl or ioResult.
 */
 
OSErr LowUDPWrite (StreamPtr stream, Ptr wdsPtr, unsigned long remoteHost, 
					unsigned short remotePort)
{
	UDPiopb pb;
	OSErr err = noErr;

	InitUDPPB(&pb);
	
	pb.csCode = UDPWrite;
	pb.udpStream = stream;
	pb.csParam.send.remoteHost = remoteHost;
	pb.csParam.send.remotePort = remotePort;
	pb.csParam.send.wdsPtr = wdsPtr;
	pb.csParam.send.checkSum = 0; /* MDT FIX */
	
	if ((err = PBControl((ParmBlkPtr)&pb,true)) != noErr)
		return err;
	
	while (pb.ioResult > 0) 
		if ((*gSpin)())
			return 1;
	return pb.ioResult;
}

/* LowUDPRelease
 * Releases memory for a UDP stream. This must be called when a stream is 
 * no longer needed.
 *    stream - the UDP stream ptr.
 * Returns OSErr: PBControl or ioResult.
 */
OSErr LowUDPRelease (StreamPtr stream)
{
	UDPiopb pb;
	OSErr err = noErr;

	InitUDPPB(&pb);
	
	pb.csCode = UDPRelease;
	pb.udpStream = stream;

	if ((err = PBControl((ParmBlkPtr)&pb, true)) != noErr)
		return err;
	while (pb.ioResult >0) (*gSpin)();

	if (pb.ioResult != noErr) return pb.ioResult;
	
	DisposPtr(pb.csParam.create.rcvBuff);
	return MemError();
}

/* LowUDPMTU
 * Divines the MTU for a given UDP stream.
 *    stream     - the UDP stream ptr;
 *    remoteHost - IP Number of the host one wants to send to;
 *	  *MTU	     - returns the size of MTU in bytes.
 * Returns OSErr: PBControl or ioResult.
 */
OSErr LowUDPMTU (StreamPtr stream, unsigned long remoteHost, unsigned short *MTU)
{
	UDPiopb pb;
	OSErr err = noErr;

	InitUDPPB(&pb);
	
	pb.csCode = UDPMaxMTUSize;
	pb.udpStream = stream;
	pb.csParam.mtu.remoteHost = remoteHost;

	if ((err = PBControl((ParmBlkPtr)&pb, true)) != noErr)
		return err;
	while (pb.ioResult >0) 
		if ((*gSpin)())
			return 1;

	*MTU = pb.csParam.mtu.mtuSize;
		
	return (pb.ioResult);

}

/* multiport UDP commands have been ignored */




/**
 **
 **  TCP/IP MacTCP ROUTINES
 **
 **/


/* InitTCPPB
 * - A utility routine to zero a TCPiopb structure, and set some
 *	 global initializations.
 *		*pb - TCPiopb to be initialized
 */

void InitTCPPB (TCPiopb *pb)
{
	bzero ((char *)pb, sizeof(TCPiopb));
	pb->ioResult = 1;
	pb->ioCRefNum = gRefNum;
}
	
	

/* LowTCPCreate
 * - Creates TCP stream asynchronously.
 *     *stream - returns the TCP stream pointer.
 *	   *sockets- this is ip.c specific - an unfortunate kludge.
 * Returns OSErr: MemErr and PBControl
 */

OSErr LowTCPCreate (StreamPtr *stream, mac_socket *sockets)
{	
	TCPiopb pb;
	OSErr err = noErr;
	Ptr buffer;
		
	buffer = NewPtr(kBufferLength);
	if ((err = MemError()) != noErr) {
		return (err);
	}
	
	InitTCPPB(&pb);
	
	pb.csCode = TCPCreate;
	pb.csParam.create.rcvBuff = buffer;
	pb.csParam.create.rcvBuffLen = kBufferLength;
	pb.csParam.create.notifyProc = (TCPNotifyUPP) NewTCPNotifyProc (MyTCPNotifyProc); 
	pb.csParam.create.userDataPtr = (Ptr)sockets;
	
	err = PBControl((ParmBlkPtr)&pb,true);
	if (err != noErr ) return err;
	
	while (pb.ioResult > 0) (*gSpin)();
	*stream = pb.tcpStream;
	
	err = pb.ioResult;
	return err;
}


/* LowTCPPassiveOpen
 * - waits on a stream for an incomming connection. This procedure
 *   will block until a connection is received.
 *	  stream 	  - socket descriptor
 *	  char	 	  - time out value in seconds
 *	  *remoteHost - registers the address connections may be received
 *					from. Passing 0 allows any address to connect. 
 *					Returns remoteHost IPNumber if 0 was passed to it.
 *	  *remotePort - as above for *remoteHost
 *	  *localhost  - returns local host IP
 *	  *localPort  - port to listen to; 0 specifies any port. Returns the
 *					port connected to.
 * Returns OSErr: PBControl
 */
OSErr LowTCPPassiveOpen (StreamPtr stream, char timeout,
	unsigned long *remoteHost, unsigned short *remotePort, unsigned long *localHost,
	unsigned short *localPort)
{
	OSErr err = noErr;
	TCPiopb pb;
	
	InitTCPPB(&pb);

	pb.csCode = TCPPassiveOpen;
	pb.tcpStream = stream;
	pb.csParam.open.ulpTimeoutValue = timeout;
	pb.csParam.open.ulpTimeoutAction = 1;
	pb.csParam.open.validityFlags = 0xC0;		/* timeout flags */
	pb.csParam.open.commandTimeoutValue = timeout;
	pb.csParam.open.remoteHost = *remoteHost;
	pb.csParam.open.remotePort = *remotePort;
	pb.csParam.open.localPort = *localPort;
	
	err = PBControl((ParmBlkPtr)&pb,true);
	if (err != noErr) return err;
	
	while (pb.ioResult > 0 ) 
		if ((*gSpin)())
			return 1;

	err = pb.ioResult;
	
	*remoteHost = pb.csParam.open.remoteHost;
	*remotePort = pb.csParam.open.remotePort;
	return err;
}




/* LowTCPActiveOpen
 * - Actively connects with a server on the specified remoteHost and
 *	 remotePort
 *	  stream 	  - socket descriptor
 *	  char	 	  - time out value in seconds
 *	  remoteHost  - IPAddress unsigned long notation. (can't be 0)
 *	  remotePort  - port to connect to. (can't be 0)
 *	  *localhost  - returns local host IP
 *	  *localPort  - port to use; 0 specifies any port. Returns the
 *					port connected to.
 * Returns OSErr: StrToAddr or PBControl
 */

OSErr LowTCPActiveOpen (StreamPtr stream, char timeout,
	unsigned long remoteHost, unsigned short remotePort, unsigned long *localHost,
	unsigned short *localPort)
{
	OSErr err = noErr;
	TCPiopb pb;
				
	InitTCPPB(&pb);

	pb.csCode = TCPActiveOpen;
	pb.tcpStream = stream;
	pb.csParam.open.ulpTimeoutValue = timeout;
	pb.csParam.open.ulpTimeoutAction = 1;
	pb.csParam.open.validityFlags = 0xC0;		/* timeout flags */
	pb.csParam.open.commandTimeoutValue = timeout;
	pb.csParam.open.remoteHost = remoteHost;
	pb.csParam.open.remotePort = remotePort;
	pb.csParam.open.localPort = *localPort;
	
	err = PBControl((ParmBlkPtr)&pb,true);
	if (err != noErr) return err;

	while (pb.ioResult > 0 )
		if ((*gSpin)())
			return 1;

	err = pb.ioResult;
	*localPort = pb.csParam.open.localPort;
	*localHost = pb.csParam.open.localHost;
	return err;
}


/* LowTCPSend
 * - Sends data over the stream
 *    stream - stream descriptor
 *    char   - time out in seconds
 *    wdsPtr - write structure
 * Returns OSErr: PBControl
 */
OSErr LowTCPSend (StreamPtr stream, char timeout, Ptr wdsPtr)
{
	OSErr err = noErr;
	TCPiopb pb;
	
	InitTCPPB(&pb);

	pb.csCode = TCPSend;
	pb.tcpStream = stream;
	pb.csParam.send.ulpTimeoutValue = timeout;
	pb.csParam.send.ulpTimeoutAction = 1;
	pb.csParam.send.validityFlags = 0xC0;
	pb.csParam.send.wdsPtr = wdsPtr;
	
	err = PBControl((ParmBlkPtr)&pb,true);
	if (err != noErr) return err;
	
	while (pb.ioResult>0) 
		if ((*gSpin)())
			return 1;

	err = pb.ioResult;
	return err;
}

/* I'm going to ignore the NoCopy routines, because I don't understand them */


/* LowTCPRcv
 * - receives data over the stream
 *    stream  - stream descriptor
 *    char    - time out in seconds
 *    rbuf	  - memory buffer
 *	  *buflen - length of memory buffer. Returns # of bytes received.
 * Returns OSErr: PBControl
 */
OSErr LowTCPRcv (StreamPtr stream, char timeout, Ptr rbuf, int *buflen)
{
	OSErr err = noErr;
	TCPiopb pb;
	
	InitTCPPB(&pb);

	pb.csCode = TCPRcv;
	pb.tcpStream = stream;
	pb.csParam.receive.commandTimeoutValue = timeout;
	pb.csParam.receive.rcvBuff = rbuf;
	pb.csParam.receive.rcvBuffLen = *buflen;
	
	err = PBControl((ParmBlkPtr)&pb,true);
	if (err != noErr) return err;
	
	while (pb.ioResult>0)
		if ((*gSpin)())
			return 1;

	*buflen = pb.csParam.receive.rcvBuffLen;
	err = pb.ioResult;
	
	return err;
}


/* LowTCPClose
 * - Closes a stream.
 *	  stream - stream descriptor
 *	  char	 - timeout in seconds
 * Returns OSErr: PBControl
 */
 
OSErr LowTCPClose (StreamPtr stream, char timeout)
{
	OSErr err = noErr;
	TCPiopb pb;
	
	InitTCPPB(&pb);

	pb.csCode = TCPClose;
	pb.tcpStream = stream;
	pb.csParam.close.ulpTimeoutValue = timeout;
	pb.csParam.close.ulpTimeoutAction = 1;
	pb.csParam.close.validityFlags = 0xC0;
	
	err = PBControl((ParmBlkPtr)&pb,true);
	if (err != noErr) return err;
	
	while (pb.ioResult>0) (*gSpin)();

	err = pb.ioResult;
	
	return err;
}


/* LowTCPAbort
 * - Abruptly closes a stream.
 *	  stream - stream descriptor
 * Returns OSErr: PBControl
 */
 
OSErr LowTCPAbort(StreamPtr stream)
{
	OSErr err = noErr;
	TCPiopb pb;
	
	InitTCPPB(&pb);

	pb.csCode = TCPAbort;
	pb.tcpStream = stream;
	
	err = PBControl((ParmBlkPtr)&pb,true);
	if (err != noErr) return err;
	
	while (pb.ioResult>0) (*gSpin)();

	err = pb.ioResult;
	
	return err;
}

/* TCP Status - skipped */

/* LowTCPRelease
 * - Releases memory reserved for a stream. If the stream is still
 * 	 connected, the stream will be aborted.
 *	  stream - stream descriptor
 * Returns OSErr: PBControl and MemError
 */

OSErr LowTCPRelease(StreamPtr stream)
{
	OSErr err = noErr;
	TCPiopb pb;
	Ptr buffer;
	
	InitTCPPB(&pb);

	pb.csCode = TCPRelease;
	pb.tcpStream = stream;
	
	err = PBControl((ParmBlkPtr)&pb,true);
	if (err != noErr) return err;
	
	while (pb.ioResult>0) (*gSpin)();

	if ((err = pb.ioResult) != noErr) return err;
	buffer = pb.csParam.create.rcvBuff;

	DisposPtr(buffer);
	err = MemError();
	return err;
}

/* LowGlobalInfo - skip */




/* LowTCPSelect
 * - Reports if there is unread data on a stream.
 *		stream - stream descriptor
 *		*ans   - 0 if no data waiting , 1 if data waiting.
 * Returns OSErr: PBControl
 */

/* OBSOLETE - Moved to a higher level because of ASR */

OSErr LowTCPSelect (StreamPtr stream, int *ans)
{
	OSErr err;
	TCPiopb pb;
	
	InitTCPPB(&pb);

	pb.csCode = TCPStatus;
	pb.tcpStream = stream;
	
	err = PBControl((ParmBlkPtr)&pb,true);
	if (err != noErr) return err;
	
	while (pb.ioResult>0)
		if ((*gSpin)())
			return 1;

	err = pb.ioResult;

	if (pb.csParam.status.amtUnreadData > 0) {
		*ans = 1;
	} else {
		*ans = 0;
	}
	
	return err;
}

/* The following code is specifically for my socket ip.c file.  They are two 
 * MacTCP call back routines necessary to provide select() support.  I'm still
 * finding bugs in these routines - I think I was sleepy when I wrote them.
 * Handle with care - MDT.
 */

pascal void MyUDPNotifyProc(StreamPtr stream, unsigned short eventCode, Ptr data, 
							struct ICMPReport *icmpMsg)
{
#pragma unused (icmpMsg)
	short i,s = -1;
	
	for (i=0; i<kNumSockets; i++){
		if (((mac_socket *) data)[i].stream == stream){
			s = i;
			break;
		}
	}
	if (s < 0) return;

	switch (eventCode) {
	case UDPDataArrival:
		((mac_socket *)data)[s].hasData = 1;
		break;
	default:
		;
	}
}

pascal void MyTCPNotifyProc(StreamPtr stream, unsigned short eventCode, Ptr data, 
							unsigned short termr, struct ICMPReport *icmpMsg)
{
#pragma unused (icmpMsg)
#pragma unused (termr)
	short i,s = -1;
	
	for (i=0; i<kNumSockets; i++){
		if (((mac_socket *) data)[i].stream == stream) {
			s = i;
			break;
		}
	}
	if (s < 0) return;

	switch (eventCode) {
	case TCPDataArrival:
		((mac_socket *)data)[s].hasData = 1;
		break;
	default:
		;
	}
}