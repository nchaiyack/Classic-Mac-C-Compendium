/*
 * ip.c
 *
 * High routines for dealing with MacTCP. This file has the freedom to
 * be application specific, unlike iplow.c. 
 *
 * These calls will vaguely approximate socket calls.
 *
 * This library was strongly patterned older releases of NewsWatcher,
 * with the blessing of John Norstad.
 *
 * Mike Trent 8/94
 */
/*
 * Since it's initial release in April 1995, I received some good feedback
 * on my code.  My thanks go out to the following people (in chronological order):
 *
 * 		Lim Wai Kong David <limwaiko@iscs.nus.sg>
 *		Matthias Ulrich Neeracher <neeri@iis.ee.ethz.ch>
 *		David M. Asbell <udasbell@mcs.drexel.edu>
 * 30 May 95
 */

/* ANSI Includes */
#include <stdio.h>  /* for num2dot's sprintf */
#include <string.h>

/* Additional Mac Includes */
#include <MacTCPCommonTypes.h>
#include <AddressXlation.h>
#include <MiscIPPB.h>
#include <UDPPB.h>
#include <TCPPB.h>
#include <GetMyIPAddr.h>


/* Local Includes */
#include "butil.h"
#include "ip.h"
#include "iplow.h"
#include "iperr.h"


/* Global to this module */

mac_socket streams[kNumSockets];
int	initedp = 0;

/* Private Functions */

int FindFreeSocket(void);
void ReleaseAll(void);


/**
 **
 ** MISC ROUTINES
 **
 **/


/* InitMacTCP
 * - Initializes the MacTCP driver via iplow.c , zero's out the module-global
 *	 streams array, and specifices an atexit routine to free memory. 
 * Returns OSErr: result of LowInitMacTCP
 */
 
OSErr InitMacTCP(void)
{
	OSErr err;
	
	if (initedp == 1) return noErr;	/*handle multiple calls gracefully */
	initedp = 1;
	
	bzero((char *)streams, (sizeof(mac_socket) * kNumSockets));
	
	/*
	if (atexit(ReleaseAll)) {
		return -1;
	}
	* - Removed December 11, 1994
	*/
	
	err = LowInitMacTCP();
	SetSpin(nil);
	return err;
}


/* DisposeMacTCP
 * - calls ReleaseAll, so that connections can be closed and memory
 *   can be disposed of.
 *   
 *   Originally this was handled with "atexit(ReleaseAll)" but this
 *   seems like a better way to do it.
 */
void DisposeMacTCP(void)
{
	ReleaseAll();
	
	initedp = 0;	/* Ok, I've provided this, but I'm not going to support it - yet */
}


/* FindFreeSocket
 * - Searches through streams array looking for a free 'socket'
 * Returns int: socket id 0-31, -1 if array is full.
 */
 
int FindFreeSocket(void)
{
	int x;
	
	for (x = 0; x < (kNumSockets -1); x++) {
		if (streams[x].stream == 0) {
			return x;
		}
	}
	return -1;
}

/* ReleaseAll
 * - Runs when program quits normally, and frees memory from MacTCP
 */
void ReleaseAll(void)
{
	OSErr err;
	int x;
	
	for (x=0; x<(kNumSockets -1); x++)
		if (streams[x].stream > 0)
			switch (streams[x].type) {
			case IPPROTO_TCP:
				err = LowTCPRelease(streams[x].stream);
				break;
			case IPPROTO_UDP:
				err = LowUDPRelease(streams[x].stream);
				break;
			}
	//error is ignored
	return;
}

/* SetSpin
 * - Sets the event routine so that the Macintosh can continue
 *   processing events while waiting for a MacTCP routine to return.
 *	 spinRoutine is a procedure pointer type. A routine passed
 *	 to it will be called during network access times. A nil value
 *	 resets the spin routine to the iplow default routine.
 */
 
 
void SetSpin(Spin spinRoutine)
{
	LowSetSpin(spinRoutine);
}


/**
 **
 ** DNS COMMANDS
 **
 **/


/* num2dot
 * - Quickly converts an IP number in u_long form to dot notation. The dot notation
 * is stored in *dot. Unless the programmer has special knowledge of the u_longs being
 * passed to num2dot, *dot should be at least 16 characters long (xxx.xxx.xxx.xxx\0).
 */

void num2dot(unsigned long ip, char *dot)
{
	unsigned char *tmp;
	
	tmp = (unsigned char *)&ip;
	(void) sprintf(dot, (const char *)"%d.%d.%d.%d", tmp[0], tmp[1], tmp[2], tmp[3]);
}
 

/* ConvertStrToAddr
 * - Executes a DNS look up. *name is a string containing either an IP Number in
 *   dot notation, or a DNS name
 * Returns: MacTCP OSErr.
 */
 
OSErr ConvertStrToAddr(char *name, unsigned long *ipNum)
{
	OSErr err;
	
	err = LowStringToAddr(name, ipNum);
	return err;
}


/* GetHostByName
 * - Emulates the UNIX routine by the same name.  When passed a DNS name, 
 *   GetHostByName will return the long int IP Number.
 * Returns: unsigned long IP Number; 0 = error
 */

unsigned long GetHostByName(char *name)
{
	OSErr err;
	unsigned long num;
	
	if ((err = LowStringToAddr(name, &num)) != noErr) {
		gMacErrno = err;
		return 0;
	}
	return num;
}


/* GetProtoByName
 * - returns the number associated with a named protocol. Only "tcp" and
 *   "udp" are supported via MacTCP. While this bears some resemblance to
 *	 the UNIX function of the same name, it really is different, returning
 *	 a simple int (for use in sockaddr_in.sin_port) rather than a complex
 *   record. This is mostly because MacTCP supports only UDP and TCP.
 * Returns int: protocol number, 0 = error.
 */
 
int GetProtoByName(char *name)
{
	if (!strcmp(name, "tcp")) {
		return IPPROTO_TCP;
	} else if (!strcmp(name, "udp")) {
		return IPPROTO_UDP;
	} else {
		return 0;
	}
}


/* GetHostName
 * Returns the DNS name of the current machine. If there is no DNS name, the
 * routine will return 0, and the machine's IP number in dot-notation will
 * be stored in *name.
 * Returns int: 0 noErr, -1 err.
 */

int GetHostName(char *name, int namelen)
{
//#pragma unused (namelen)
	OSErr err;
	unsigned long ip;
	
	if ((err = LowGetMyIP(&ip)) != noErr) {
		gMacErrno = err;
		return -1;
	}
	if ((err = LowAddrToName(ip, name)) != noErr) {
		num2dot(ip, name);
		//return -1;
	}
	return 0;
}

/* GetHostNameOnly
 * Returns the DNS name of the current machine. If there is no DNS name, the
 * routine will return -1.
 * Returns int: 0 noErr, -1 err.
 */

int GetHostNameOnly(char *name)
{
	OSErr err;
	unsigned long ip;
	
	if ((err = LowGetMyIP(&ip)) != noErr) {
		gMacErrno = err;
		return -1;
	}
	if ((err = LowAddrToName(ip, name)) != noErr) {
		gMacErrno = err;
		return -1;
	}
	return 0;
}


/* GetMyIPDot
 * - Returns the dot notation IP Address of the current machine in *name.
 * Returns int: 0 noError, -1 err
 */
 
int GetMyIPDot(char *num)
{
	OSErr err;
	unsigned long ip;
	
	if ((err = LowGetMyIP(&ip)) != noErr) {
		gMacErrno = err;
		return -1;
	}
	num2dot(ip, num);
	return 0;
}


/* GetMyIPNum
 * - As above, but it returns an unsigned long IP number, rather than a
 *   dot notation ip number.
 * Returns unsigned long IP Number; -1 = error
 */
 
unsigned long GetMyIPNum(void)
{
	OSErr err;
	unsigned long ip;
	
	if ((err = LowGetMyIP(&ip)) != noErr){
		gMacErrno = err;
		return -1;
	}
	return ip;
}


/* getsockname
 * - returns the local address associated with a socket. 
 * Returns int: 0 noErr, -1 err
 */
 // Added on Wed 1/4/95. Not tested
 /* Contributed by Lim Wai Kong David <limwaiko@iscs.nus.sg> */
 
int getsockname (int sock, struct sockaddr_in *localaddr, int *addrlen)
{
	OSErr err = noErr;
	
	if ((sock < 0) || (sock >= kNumSockets)) {
		gErrno = EBADF;   					// bad socket
		return -1;
	}
	
	localaddr->sin_addr = streams[sock].localHost;
	localaddr->sin_port = streams[sock].localPort;
	localaddr->sin_family = AF_INET;
			
	*addrlen = sizeof (struct sockaddr_in);

	return 0;
}


/**
 **
 ** GENERAL IP ROUTINES
 **
 **/



/* socket
 * - Creates MacTCP stream connection.
 *		type - IPPROTO_TDP for tcp, IPPROTO_UDP for udp
 * Returns int: socket if 0-31, -1 if error.
 */
//int socket (int type)
/* socket
 * - Creates a MacTCP "stream" connection
 *		family   - Ignored.
 *		type	 - Ignored.
 *		protocol - Specifies either TCP/IP (6)  or UDP/IP (17).
 *	Returns 0-31 if successful, -1 if error.  If gErrno == EIO, additional information
 *				   is stored in gMacErrno
 *
 *	Revised 12/26/94
 */
/* Site for future revision: Dispatch on type, not protocol */
int socket(int family, int type, int protocol)
//#pragma unused (family)
//#pragma unused (type)
{
	OSErr err;
	int sock;
	
	sock = FindFreeSocket();
	if (sock < 0) {
		gErrno = EMFILE;
		return -1;
	}
	
	switch (protocol) {
	  case IPPROTO_TCP:
		err = LowTCPCreate(&(streams[sock].stream), streams);
		break;
	  case IPPROTO_UDP:
	  	err = LowUDPCreate(&(streams[sock].stream), 0, streams);
	  	break;
	  default:
	  	gErrno = ESOCKTNOSUPPORT;
	  	return -1;
	}
	    
	if (err != noErr) {
		gMacErrno = err; /* if mem err, gErrno = ENOBUFS */
		gErrno = EIO;
		return -1;
	}
	streams[sock].type = protocol;
	return sock;
}

/* connect
 * - creates an active TCP or UDP stream. 
 * Returns int: 0 noErr, -1 err
 */
int connect(int sock, struct sockaddr_in *raddr, int alen)
//#pragma unused (alen)
{
	OSErr err = noErr;
	
	if ((sock < 0) || (sock >= kNumSockets)) {
		gErrno = EBADF;   /* bad socket */
		return -1; 
	}
	/*if (streams[sock].stream == 0) {
		gErrno = ENOTSOCK;
		return -1;
	} Useless */
	if (streams[sock].connected) {
		gErrno = EISCONN;
		return -1;
	}
	streams[sock].remoteHost = raddr->sin_addr;
	streams[sock].remotePort = raddr->sin_port;
		
	switch (streams[sock].type) {
	  case IPPROTO_TCP:
	  	err = LowTCPActiveOpen (streams[sock].stream, (char)30, streams[sock].remoteHost,
							streams[sock].remotePort, &(streams[sock].localHost),
							&(streams[sock].localPort));
		break;
	  case IPPROTO_UDP:
	  	/* Do nothing. UDP merely records the destination for later use. Comer III p.70 
	  	   The important info was recorded in the two lines preceeding the switch */
	  	break;
	}
	  	
	if (err != noErr) {
		gMacErrno = err;
		gErrno = EIO;
		return -1;
	}
	
	streams[sock].connected = 1;
	return 0;
}



/* write
 * - sends data over the stream.
 *	  sock - socket descriptor
 *	  data - Ptr to data (also char *)
 *	  len  - length of mem pointed to in Ptr.
 * Returns int: len
 */

int write (int sock, Ptr data, int len)
{
	OSErr err;
	struct wdsEntry myWDS[2];	/* global write data structure */

	if ((sock < 0) || (sock >= kNumSockets)) {
		gErrno = EBADF;   /* bad socket */
		return -1; 
	}
	if (streams[sock].stream == 0) {
		gErrno = ENOTSOCK;
		return -1;
	}
	if (!streams[sock].connected) {
		gErrno = EPIPE; 
		return -1;
	}
	if (len == 0) return 0;
	
	if (len > 32767) {
		gErrno = EFBIG;
		return -1;
	}

	myWDS[1].length = 0;
	myWDS[1].ptr = nil;
	
	switch(streams[sock].type) {
	  case IPPROTO_TCP:	
		myWDS[0].length = len;
		myWDS[0].ptr = data;
	
		err = LowTCPSend(streams[sock].stream, (char)30, (Ptr)myWDS);
		
		if (err != noErr) {
			gMacErrno = err;
			gErrno = EIO;
			return -1;
		}
		break;
	  case IPPROTO_UDP:
	 	{
	 		unsigned short mtu;
	 		int lenidx=0;
	 		
	 		if (streams[sock].mtu == 0) {
	 			err = LowUDPMTU(streams[sock].stream, streams[sock].remoteHost, &streams[sock].mtu);
	 			if (err != noErr) {
	 				streams[sock].mtu = 0;
		 			gErrno = EIO;
		 			gMacErrno = err;
		 			return -1;
		 		}
		 	}
	 		mtu = streams[sock].mtu - kHeaderSize;
	 		
	 		/* Site for future revision: Don't split up the UDP call! */
	 		while (lenidx < len) {
	 			myWDS[0].length = (mtu < (len - lenidx) ? mtu : (len - lenidx));
	 			myWDS[0].ptr = &data[lenidx];
	 			lenidx += myWDS[0].length;
	 			
	 			err = LowUDPWrite(streams[sock].stream, (Ptr)myWDS, streams[sock].remoteHost,
	 							  streams[sock].remotePort);
	 			if (err != noErr) {
	 				gErrno = EIO;
	 				gMacErrno = err;
	 				return -1;
	 			}
	 		}
		}
		break;
	}
	return len;
}


/* read
 * - reads data from a socket. It will block if there is no data.
 *    sock - socket descriptor
 *	  buf  - memory buffer to be written to
 *	  len  - length of memory buffer
 * Returns int: number of bytes read, or -1
 */

int read (int sock, Ptr buf, int len)
{
	OSErr err;
	int tmplen = len;
	unsigned long dmy1=0;
	unsigned short dmy2=0;

	if ((sock < 0) || (sock >= kNumSockets)) {
		gErrno = EBADF;   /* bad socket */
		return -1; 
	}
	if (streams[sock].stream == 0) {
		gErrno = ENOTSOCK;
		return -1;
	}
	if (!streams[sock].connected) {
		gErrno = EPIPE; 
		return -1;
	}
	if (len == 0) return 0;
	if (len > 32767) {
		gErrno = EFBIG;
		return -1;
	}
	
	switch(streams[sock].type){
	case IPPROTO_TCP:
		err = LowTCPRcv (streams[sock].stream, (char)30, buf, &tmplen);
		break;
	case IPPROTO_UDP:
	    err = LowUDPRead (streams[sock].stream, (char)30, buf, &tmplen, &dmy1, &dmy2); 
		break;
	}	    

	if (err != noErr) {
		gMacErrno = err;
		gErrno = EIO;
		return -1;
	}
	streams[sock].hasData = 0;		/* Assume the user read all the data */
	return tmplen;
}


/* close
 * - closes a stream and releases it's memory
 *    sock - socket descriptors
 * Returns int: 0 noErr, -1 err
 */
int close(int sock)
{
	OSErr err;

	if ((sock < 0) || (sock >= kNumSockets)) {
		gErrno = EBADF;   /* bad socket */
		return -1; 
	}
	if (!streams[sock].connected) {
		gErrno = EBADF; 
		return -1;
	}
	
	switch (streams[sock].type) {
	  case IPPROTO_TCP:
		err = LowTCPClose (streams[sock].stream,(char) 30);
		(void) LowTCPRelease(streams[sock].stream);
		break;
	  case IPPROTO_UDP:
	  	err = LowUDPRelease(streams[sock].stream);
	  	break;
	}
	
	if ((err != noErr) && (err != connectionDoesntExist)) {
		gMacErrno = err;
		gErrno = EIO;
		
		/* Might as well just call the stream closed */
		bzero((char *)&(streams[sock]), sizeof(mac_socket));
		return -1;
	}

	/* zero out entire structure */
	bzero((char *)&(streams[sock]), sizeof(mac_socket));
	
	return 0;
}



/**
 **
 **  TCP SECTION
 **
 **/


/* select
 * - Reports which streams has unread data queued in them. See UNIX man page.
 *		nfds    - should always be equal to kNumSockets
 *		*readfs - bitmap of sockets; Specifies streams to check on IN, and
 *				  returns the streams w/ pending data on OUT.
 * Returns int : number of sockets w/ pending data.
 */
/* 30 May 95 - Modified routine so that if timeout != nil, data will be polled
 * at least once, even if tv_sec = tv_usec = 0.  This was suggested by
 * David M. Asbell (udasbell@mcs.drexel.edu); he has my thanks.
 */
 int select (int nfds, unsigned long *readfs, struct timeval *timeout)
{
	int sock, total = 0;
	unsigned long bitmap = 0;
	long waittil;
	int debug = -1;
	
	if (timeout) waittil = TickCount() + (60 * timeout->tv_sec);
	
	for (;;) {
				
		for (sock=0; sock<nfds; sock++) {
			if (*readfs & (1 << sock)) {  /* if socket selected */
				if (streams[sock].stream == 0) {	/* if selected socket isn't allocated */
					gErrno = EBADF;
					gMacErrno = 0;
					return -1;
				}
				if ((streams[sock].hasData == 1) || (sock == debug)) { /*the last bit added for debugging MDT */
					bitmap |= (1 << sock);  // select bit 
					total++;
				}
			} /* if */
		} /* for (sock) */
		
		if (total) break;	/* if we have unread data, break */

		if ((timeout) && (waittil < TickCount()) ) /* if we have timed out, break. Note that
													  total still = 0 */
			break;

		LowSpin();  /* give a hoot - call the spin routine! */
		
	}/* for (;;) */
	*readfs = bitmap;
	return (total);
}

/* OBSOLETE */

int old_select (int nfds, unsigned long *readfs){
	OSErr err;
	int sock, total = 0;
	int ans = 0;
	
	for (sock=0; sock<nfds; sock++) {
		if (*readfs & (1 << sock)) {  /* if socket selected */
			if (streams[sock].stream == 0) {	/* if selected socket isn't allocated */
				gErrno = EBADF;
				gMacErrno = 0;
				return -1;
			}
			if (streams[sock].type == IPPROTO_UDP) 
				continue; 						/* skip UDP sockets */
			if ((err = LowTCPSelect(streams[sock].stream, &ans)) != noErr) {
				gErrno = EIO;
				gMacErrno = err;
				return -1;
			}
			if (ans == 0) {
				*readfs &= ~(1 << sock);  // unselect bit 
			} else {
				total++;
			}
		}
	}
	return (total);
}


/* bind
 * - Specifies port for server to listen to.
 * 		sock  - Socket to bind
 *		*name - address to bind socket to.  Traditionally, name->sin_family == AF_INET,
 *				though name->sin_family is ignored in this implementation.  name->sin_addr
 *				must equal INADDR_ANY ((u_long)0x00000000) or the local system address,
 *				though, again, in this case, name->sin_addr is ignored.  name->sin_port
 *				specifies the port number the socket is to be bound to. Traditionally,
 *				ports between 1 and 1023 (inclusive) are reserved for the high and mighty;
 *				this implementation does not preserve the notion of reserved ports.
 *		alen  - the length of the *name field.  Ignored.
 * Returns 0 if successful, -1 if error.  The specific error code is stored in gErrno. If
 *				gErrno == EIO, additional information is stored in gMacErrno.
 *
 * Revised 12/26/94
 * Revised 3/14/95 - see below.
 */
/* Site for future revision: Currently, I don't check to see if other processes
   have bound to the desired port. */
int bind (int sock, struct sockaddr_in *name, int alen)
//#pragma unused (alen)
{
	int x;
	
	if ((sock < 0) || (sock >= kNumSockets)) {
		gErrno = EBADF;   /* unix bad socket */
		return -1; 
	}
/* Apparently, one can bind UDP sockets.  The RPC portmapper infact binds a UDP
 * and a TCP socket to port 111 at the same time.  I didn't read Comer carefully.
 * See below ...
 */
/*	if (streams[sock].type == IPPROTO_UDP) {
		gErrno = EBADF;
		gMacErrno = 0;
		return -1;
	}*/
	if ((name->sin_port < 1) || (name->sin_port > 32767)) {
		gErrno = EADDRNOTAVAIL; /* bad port num */
		return -1;
	}
	if (streams[sock].bound) {	
		gErrno = EINVAL;
		return -1;
	}
	for (x=0; x< (kNumSockets -1); x++) {
		if ((name->sin_port == streams[x].localPort) && (streams[x].bound) && 
							(streams[x].type == streams[sock].type)) {
		/* if someone else is bound to this port ... return error.  */
		/* this if was modified to make sure a UDP socket and a TCP socket CAN connect to
		 * the same port, but two or more of like-kinds cannot bind to the port.
		 * 14 Mar 95 - MDT
		 */
			gErrno= EADDRINUSE;
			return -1;
		}
	}
	streams[sock].bound = 1;
	streams[sock].localPort = name->sin_port;
	return 0;
}

/* there is no listen */
int listen(int socket, int queuelen)
//#pragma unused (socket)
//#pragma unused (queuelen)
{
	/* Welcome to Macintosh */
	return 0;
}

/* accept
 * - Passively waits for a TCP connection. 
 *		sock - specifies socket to wait on
 *		*sin - returns address of calling service.
 *		alen - Ignored.
 * Returns 0 if successful, -1 if not. If gErrno == EIO, additional information is stored
 *				in gMacErrno.
 *
 * Revised 12/26/94
 */
int accept (int sock, struct sockaddr_in *sin, int *alen)
{
//#pragma unused (alen)
	OSErr err;
	unsigned long remoteHost;
	unsigned short remotePort;
	int tmp;
	
	if ((sock < 0) || (sock >= kNumSockets)) {
		gErrno = EBADF;   /* unix bad socket */
		return -1; 
	}
	if (streams[sock].type == IPPROTO_UDP) {
		gErrno = EBADF;
		return -1;
	}
	if ((streams[sock].stream == 0) || (streams[sock].localPort == 0)) {
		gErrno = ENOTSOCK;
		return -1;
	}
	if (streams[sock].connected) {
		gErrno = EOPNOTSUPP; /* or something - the port is busy already */
		return -1;
	}
	if (!streams[sock].bound) {
		/* socket is not bound to port*/
		return -1;
	}
	if ((tmp = FindFreeSocket()) == -1) {	
		/* out of sockets */
		return -1;
	}
		
	remotePort = remoteHost = 0;
	
	err = LowTCPPassiveOpen(streams[sock].stream, (char)0, &remoteHost, &remotePort,
							&(streams[sock].localHost),&(streams[sock].localPort));
	if (err != noErr) {
		gMacErrno = err;
		gErrno = EIO;
		return -1;
	}
	
	bcopy((char *)&streams[sock], (char *)&streams[tmp], sizeof(mac_socket)); /* copy current socket into tmp */
	
	/* accept call was successful */
	/* create new stream for original socket */
	err = LowTCPCreate(&(streams[sock].stream), streams);
	if (err != noErr) {
		/* something bad happened */
		gErrno = EIO;
		gMacErrno = err;
		/* restore everything back to normal */
		streams[sock].stream = streams[tmp].stream;
		bzero((char *)&streams[tmp], sizeof(mac_socket));
		return -1;
	}

	streams[sock].connected = 0;	/* unset connected flag for sock */
	streams[tmp].connected = 1;
	streams[tmp].bound = 0;			/* unset bound flag for tmp 	 */
	sin->sin_port = remotePort;
	sin->sin_addr = remoteHost;
	return (tmp);
}


/**
 **
 ** UDP/IP ROUTINES
 **
 **/
 
int recvfrom (int sock, char *buf, int len, int flags, struct sockaddr_in *sin, int *alen)
{
//#pragma unused (alen)
	OSErr err;
	int tmplen = len;
	unsigned long address=0;
	unsigned short port=0;

	if ((sock < 0) || (sock >= kNumSockets)) {
		gErrno = EBADF;   /* bad socket */
		return -1; 
	}
	if (streams[sock].stream == 0) {
		gErrno = ENOTSOCK;
		return -1;
	}
	/*if (!streams[sock].connected) {
		gErrno = EISNCONN;
		return -1;
	} DUH !! Don't be stupid! */
	if (streams[sock].type == IPPROTO_TCP) {
		gErrno = EBADF;   /* bad socket */
		return -1; 
	}

	if ((len == 0) || (len > 32767)) 
		return 0; /* this is rough, but for debug */
	
	/* This bit is thanks to Matthias Ulrich Neeracher <neeri@iis.ee.ethz.ch>, author
	 * of GUSI, who took the time to read my code (even with all it's crappy commenting)
	 * and point out mistakes. Thanks!
	 */	
	if (sin) {
		address = sin->sin_addr;
		port  = sin->sin_port;
	}
	
    err = LowUDPRead (streams[sock].stream, (char)0, buf, &tmplen, &address, 
    					&port); 
	if (err != noErr) {
		gMacErrno = err;
		gErrno = EIO;
		return -1;
	}
	streams[sock].hasData = 0; /* assume all data has been read */
	
	if (sin) {
		sin->sin_addr = address;
		sin->sin_port = port;
	}
	
	return tmplen;
}




int sendto (int sock, char *data, int len, int flags, struct sockaddr_in *sin, int alen)
{
//#pragma unused (alen)
	OSErr err;
	struct wdsEntry myWDS[2];	/* global write data structure */
	unsigned short mtu;
	int lenidx=0;


	if ((sock < 0) || (sock >= kNumSockets)) {
		gErrno = EBADF;   /* bad socket */
		return -1; 
	}
	if (streams[sock].stream == 0) {
		gErrno = ENOTSOCK;
		return -1;
	}
/*	if (!streams[sock].connected) {
		gErrno = EISNCONN; 
		return -1;
	}*/
	if (streams[sock].type != IPPROTO_UDP) {
		gErrno = EBADF;   /* UDP ONLY PLEASE! */
		return -1; 
	}
	if ((len == 0) || (len > 32767)) return 0;

	myWDS[1].length = 0;
	myWDS[1].ptr = nil;
	
	if (streams[sock].mtu == 0) {
		err = LowUDPMTU(streams[sock].stream, sin->sin_addr, &streams[sock].mtu);
		if (err	!= noErr) {
			streams[sock].mtu = 0;
			gErrno = EIO;
			gMacErrno = err;
			return -1;
		}
	}
	mtu = streams[sock].mtu - kHeaderSize;
	
	while (lenidx < len) {
		myWDS[0].length = (mtu < (len - lenidx) ? mtu : (len - lenidx));
		myWDS[0].ptr = &data[lenidx];
		lenidx += myWDS[0].length;
		
		err = LowUDPWrite(streams[sock].stream, (Ptr)myWDS, sin->sin_addr, sin->sin_port);
		if (err != noErr) {
			gErrno = EIO;
			gMacErrno = err;
			return -1;
		}
	}
	return len;
}





