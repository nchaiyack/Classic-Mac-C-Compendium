/****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1993,									*
*	Board of Trustees of the University of Illinois				*
*****************************************************************
*  Revisions:
*  10/87  Initial source release, Tim Krauskopf
*  2/88  typedef support for other compilers (TK)
*  8/88  Gaige Paulsen - support for MacTCP drivers
*  1/89  TK - conversion to new drivers, minor update for new calling convention
*  6/89  TK - update to MacTCP 1.0 include files and use of GetMyIPAddr()
*  7/92  Scott Bulmahn - add support for 2 global structs, put cursors into an array
*  11/92 Jim Browne - Fixed more bugs than I care to mention.					
*
*/

#ifdef MPW
#pragma segment Network
#endif

#include "TelnetHeader.h"
#include <TCPPB.h>
#include <stdio.h>
#include <String.h>
#include "telneterrors.h"
#include "InternalEvents.h"
#include "maclook.proto.h"
#include "menuseg.proto.h"
#include "mainseg.proto.h"
#include "getmyipaddr.h"	/* BYU LSC */
#include "MyMacTCPstructures.h"
#include "wind.h"
#include "network.proto.h"
#include "netevent.proto.h"
#include "encrypt.proto.h"
#include "DlogUtils.proto.h"	// For WriteZero
#include "debug.h"

#ifdef THINK_C
#pragma options(!profile)
#endif

#ifdef __MWERKS__
#pragma profile off
#endif

//#define NET_DEBUG_MUCHINFO
//#define	NET_DEBUG
#ifdef	NET_DEBUG
char	net_debug_string[256];
#define	net_debug_print(x)	putln(net_debug_string)
#else
#define	net_debug_print(x)	
#endif

pascal void Notify(StreamPtr streamPtr, unsigned short code, Ptr uptr, 
						unsigned short terminReason, struct ICMPReport *icmpMsg);
PROTO_UPP(Notify, TCPNotify);
static	TCPIOCompletionProc		OpenCompleteInit(TCPiopb *pbp);
PROTO_UPP(OpenCompleteInit, TCPIOCompletion);
static	TCPIOCompletionProc		CloseCompleteInit(TCPiopb *pbp);
PROTO_UPP(CloseCompleteInit, TCPIOCompletion);
static	TCPIOCompletionProc		SendCompleteInit(TCPiopb *pbp);
PROTO_UPP(SendCompleteInit, TCPIOCompletion);
static	void		OpenComplete(MyTCPpbPtr pbp);
static	void		SendComplete(MyTCPpbPtr pbp);
static	void		CloseComplete(MyTCPpbPtr pbp);

static	wdsEntry	*getSWDS(void);
static	void		returnSWDS(wdsEntry *wds);
static	void		MyPBreturn(MyTCPpbPtr pbp);
static	MyTCPpb		*getPB(short driver,short call, StreamPtr stream, short usernum);
static	void		clearPB(MyTCPpbPtr pbp, short driver, short call, StreamPtr stream);
static	OSErr		xPBControlSync(MyTCPpbPtr pbp);
static	OSErr		xPBControlAsync(MyTCPpbPtr pbp, TCPIOCompletionProc completion);
static	short		makestream(void);
static	void		reclaim(StreamRPtr p);
static	short		giveback(StreamRPtr p, wdsEntry *wds);
static	void		compressfds(wdsEntry *fds);

extern Cursor	*theCursors[];
extern WindRec	*screens;

long		MyA5;
short		TCPd = 0;					/* refnum of TCP drivers */
QHdr		gFreePBQueue;
QHdr		gFreeSWDSQueue;
short 		numSWDS=0;					/* Number of SWDS's ever alloc'd (PM Only) */
StreamRPtr	streams[NPORTS];
ip_port		nnfromport = 0;

/**************************************************************************/
wdsEntry	*getSWDS(void)
{
	short		n=0;
	wdsEntry *	wds;
	
	if (gFreeSWDSQueue.qHead == NULL)
		wds = (wdsEntry *) NewPtrClear( sizeof(wdsEntry) * MAX_SWDS_ELEMS);	
	else
		{
		wds = (wdsEntry *) gFreeSWDSQueue.qHead;
		Dequeue(gFreeSWDSQueue.qHead, &gFreeSWDSQueue);
		}
		
	if (wds == NULL) {	/* sorry, cant allocate WDS (BIG PROBLEM) */
		DoError(508 | NET_ERRORCLASS, LEVEL3, NULL); // <- Is 508 correct?
		forcequit();
		// return ((wdsEntry*) 0L); <- ?? perhaps
		}

	return(wds);
}

/**************************************************************************/
void	returnSWDS(wdsEntry *wds)
{
	Enqueue((QElemPtr)wds, &gFreeSWDSQueue);
}

/**************************************************************************/
void MyPBreturn(MyTCPpbPtr pbp)
{
	Enqueue((QElemPtr)pbp, &gFreePBQueue);
}

/**************************************************************************/
MyTCPpb *getPB(short driver,short call, StreamPtr stream, short usernum)
{
	#pragma unused (usernum)
	MyTCPpb *pbp;

	if (gFreePBQueue.qHead == NULL)
		pbp = (MyTCPpbPtr) NewPtrClear( sizeof(MyTCPpb) );
	else
		{
		pbp = (MyTCPpbPtr) gFreePBQueue.qHead;
		Dequeue(gFreePBQueue.qHead, &gFreePBQueue);
		}
		
	if (pbp == NULL) {	/* sorry, cant allocate TCP buffer (BIG PROBLEM) */
		DoError(508 | NET_ERRORCLASS, LEVEL3, NULL);
		forcequit();
		}
	
	memset((char *) pbp, '\0', sizeof(MyTCPpb));	
	pbp->pb.ioCRefNum = driver;
	pbp->pb.tcpStream = stream;
	pbp->pb.csCode = call;
	
	return(pbp);
}

/**************************************************************************/
void clearPB(MyTCPpbPtr pbp, short driver, short call, StreamPtr stream)
{
	WriteZero((Ptr)pbp, sizeof(MyTCPpb));	/* BYU LSC - Default to all zeros */	
	pbp->pb.ioCRefNum = driver;
	pbp->pb.tcpStream = stream;
	pbp->pb.csCode = call;
}

/**************************************************************************/
OSErr xPBControlSync(MyTCPpbPtr pbp) 
{ 
	pbp->pb.ioCompletion = 0L; 						/* Charlie will puke if he ever sees */
	return PBControl((ParmBlkPtr)(pbp),false); 		/* his funtions here. -- JMB */
}

/**************************************************************************/
OSErr xPBControlAsync(MyTCPpbPtr pbp, TCPIOCompletionProc completion) 
{ 
#ifndef __powerpc__
	pbp->SavedA5 = MyA5;
#endif
	pbp->pb.ioCompletion = completion; 
	return(PBControl((ParmBlkPtr)(pbp),true));		/* async */
}

void	SetPortType(short port, short type)
{
	if (port >= 0 && port < NPORTS && (streams[port] != NULL))
		streams[port]->portType = type;
}

short	GetPortType(short port)
{
	if (port >= 0 && port < NPORTS && (streams[port] != NULL))
		return(streams[port]->portType);
	else return(-1);
}

/**************************************************************************/
/*	Returns an empty stream */ 
short makestream(void)
{
	short		pnum, i;
	StreamRPtr	p;
	MyTCPpbPtr	pbp;

	for ( pnum=0; (streams[pnum]!= NULL) && pnum<NPORTS; pnum++);	/* BYU 2.4.16 */
	
	if (pnum >= NPORTS)
		return(-2);

 	if (!(p = streams[pnum] = (StreamRPtr) NewPtrClear(sizeof(StreamRec))))
 		return(-1);
  
 	if ((p->buffer = (char *) NewPtrClear( TCPBUFSIZ)) == NULL) {
 		DisposePtr((Ptr)p);
 		streams[pnum] = 0;
  		return(-1);
 	}
 	if ((p->sbuffer = (char *) NewPtrClear( TCPBUFSIZ)) == NULL) {
 		DisposePtr((Ptr)p->buffer);
 		DisposePtr((Ptr)p);
 		streams[pnum] = 0;
  		return(-1);
 	}	
 
	
	for (i=0; i<MAX_FDS_ELEMS; i++) 
		{
		p->fds[ i].length =0; p->fds[ i].ptr = 0L;
		p->exFDS[ i].inuse=0; p->exFDS[ i].fds.length=0;p->exFDS[ i].fds.ptr=0L;
		}
	p->fds[0].length = TCPBUFSIZ;
	p->fds[0].ptr = p->sbuffer;
	p->maxFDSused=0;
	p->edata = NULL;
	
	pbp=getPB( TCPd, TCPCreate, (long) 0, 7);	/* BYU LSC important - Make create call */
	pbp->pb.csParam.create.rcvBuff = p->buffer;
	pbp->pb.csParam.create.rcvBuffLen = TCPBUFSIZ;
	pbp->pb.csParam.create.notifyProc = (TCPNotifyProc)NotifyUPP;

	if (xPBControlSync(pbp) != noError) 
		DoError (512 | NET_ERRORCLASS, LEVEL2, NULL);

	p->stream = pbp->pb.tcpStream;
	
	net_debug_print("Made new stream");
	MyPBreturn(pbp);
	return(pnum);
}

/**************************************************************************/
/*	reclaim( p) -
		reclaims buffer space to stream (from pointer p) into the FDS list  */
 
void reclaim(StreamRPtr p)
{
	short n=0, offset=0;
	
	while (offset < MAX_FDS_ELEMS && p->fds[offset].ptr != 0L) offset++;
	
	if (offset >= MAX_FDS_ELEMS) /* offset too large -- cant reclaim */		
		return;
		
	for (n=0 ; n<MAX_FDS_ELEMS && offset< MAX_FDS_ELEMS; n++) 
		{
		if (p->exFDS[ n].inuse) 
			{
			p->fds[ offset++]=p->exFDS[ n].fds;
			p->exFDS[ n].inuse = 0;
			}
		}
}

/**************************************************************************
	giveback( p, wds) - gives WDS entries back to the stream by putting them in the 
 		mutually exclusive buffer.
	p -> stream
	wds -> wds array			*/
short giveback(StreamRPtr p, wdsEntry *wds)
{
	short n=0, m=0;
	
	while ( n< MAX_SWDS_ELEMS && wds[n].ptr !=0L) 
		{
		while (m< MAX_FDS_ELEMS && p->exFDS[ m].inuse) m++;
		if (m >= MAX_FDS_ELEMS)									// This was off by one - JMB 2.6
			return(-1);				/* No room in the RECLAIMation center */
		else 
			{
			p->exFDS[ m].inuse =1;
			p->exFDS[ m].fds = wds[n];
			m++;
			}
		n++;
		}
}

/**************************************************************************/
/*	compressfds( fds)
		compress an fds data structure to make everyone happy */
void compressfds(wdsEntry *fds)
{
	short n,m,compressed;
	
	compressed = 0;
	
	while ( !compressed) {
		compressed=1;
		for (n=0; n< MAX_FDS_ELEMS; n++) 
			{				/* Slow Forwards */
			if (fds[n].ptr) 
				{								/* Do if N exists */
				for ( m = MAX_FDS_ELEMS -1; m>=0; m--) 
					{	/* Fast Backwards */
					if (fds[m].ptr && (fds[m].ptr+fds[m].length == fds[n].ptr)) {
						fds[n].length+=fds[m].length;
						fds[n].ptr = fds[m].ptr;
						fds[m].ptr=0L;
						fds[m].length=0;
						compressed=0;
						}
#ifdef CHECKBOTHWAYZ
					else 
					if (fds[n].ptr+fds[n].length == fds[m].ptr) 
						{
						fds[m].length+=fds[n].length;
						fds[n].ptr=0L;
						fds[n].length=0;
						compressed=0;
						}
#endif CHECKBOTHWAYZ
					}
				}
			}
		}
	m=0;n=0;
	
	/* Close the gaps */
	
	while (n+m < MAX_FDS_ELEMS) 
		{
		while (fds[n+m].ptr ==0L && n+m< MAX_FDS_ELEMS)
			m++;			/* increase gap to valid entry */
		if (n+m<MAX_FDS_ELEMS)
			fds[n]=fds[n+m];
		n++;
		}
	
	/* Get rid of the empty spaces */
	
	n--;		/* for the next loop */
	while (n < MAX_FDS_ELEMS)
		{
		fds[n].ptr=0;
		fds[n++].length=0;
		}
}

/***************************************************************************/
/*  Mnetread
*   Read from a connection buffer into a user buffer.  
*   Returns number of bytes read, < 0 on error
* NOTE:
*	current version very inefficient, but hopefully works.
*/
short	netread(short pnum, void *buffer, short n)
{
	short		i;
	StreamRPtr	p;
	MyTCPpbPtr	pbp;
	short		inQ, reqdamt;

	if (pnum < 0 || pnum >= NPORTS)			/* BYU 2.4.15 */
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);
		
	pbp=getPB( TCPd, TCPStatus, p->stream, 1);				/* Make status call */
	if (xPBControlSync(pbp) != noError) 					/* status call bombed */
		DoError (509 | NET_ERRORCLASS, LEVEL1, "Mnetread");

	if (pbp->pb.csParam.status.connectionState !=8) {
#ifdef	NET_DEBUG
		sprintf(net_debug_string, "CState: %d is %d",(int)pnum, (int)pbp->pb.csParam.status.connectionState);
		net_debug_print(net_debug_string);
#endif
		MyPBreturn(pbp);
		return(-1);					  		/* Connection not established */
		}
	
	inQ = pbp->pb.csParam.status.amtUnreadData;
	reqdamt = n >inQ ? inQ : n;
	
	clearPB( pbp, TCPd, TCPRcv, p->stream);
	pbp->pb.csParam.receive.rcvBuff = buffer;
	pbp->pb.csParam.receive.rcvBuffLen = reqdamt;
	
	if (reqdamt<1) 
		{								/* Drop out if no data */
		MyPBreturn(pbp);
		return(0);
		}
	
	if ((i = xPBControlSync(pbp)) != noError) {
		DoError (524 | NET_ERRORCLASS, LEVEL1, NULL);
		return(-1);
		}
		
	reqdamt = pbp->pb.csParam.receive.rcvBuffLen;
	if (reqdamt<inQ) 
		netputuev( CONCLASS, CONDATA, pnum,0);			/* more data to get */

 	/* Decrypt data */
 	if (p->edata && (((CDATA *)(p->edata))->decrypt_input)) {
 		unsigned char *cp = (unsigned char *)buffer;
 		short len = reqdamt;
 		
 		while (len-- > 0) {
 			*cp = (*((CDATA *)(p->edata))->decrypt_input)((CDATA *)p->edata, (int)(*cp));
 			cp++;
 		}		
 	}
 
	MyPBreturn(pbp);
	return(reqdamt);
}

/************************************************************************/
/* netwrite
*  write something into the output queue, netsleep routine will come
*  around and send the data, etc.
*
*/
short	netwrite(short pnum, void *buffer, short nsend)
{	
	StreamRPtr	p;
	wdsEntry	*swds;
	short		remaining, queued, n,m;
	MyTCPpbPtr	pbp;
	OSErr		err;

#ifdef	NET_DEBUG_MUCHINFO
	sprintf(net_debug_string, "port: %d\tbuffer: %lx\tnsend %d", pnum,
		(unsigned int)buffer, (int)nsend);
	net_debug_print(net_debug_string);
#endif

	if (pnum < 0 || pnum >= NPORTS)		/* BYU 2.4.15 */
		return(-2);

	if ( (p = streams[pnum]) == NULL)
		return(-2);
	
	if ( !nsend )
		return(0);

	swds = getSWDS();
	
	if (swds == NULL) return(nsend);		// Duh, Mr. Bulmahn. - JMB 2.6
	
	reclaim( p);

	compressfds( p->fds);

	n=0; remaining = nsend;

//	if (p->fds[0].ptr == 0) DebugStr("\pArgh! fds[0] is NULL!");
	
	while (p->fds[n].ptr !=0 && remaining>0 ) {
//		if (n >= MAX_SWDS_ELEMS) DebugStr("\pOverflowing SWDS in while loop!");
		
		swds[n].ptr = p->fds[n].ptr;
		if ( p->fds[n].length > remaining) {
			swds[n].length = remaining;
			p->fds[n].length -= remaining;
			p->fds[n].ptr += remaining;
			remaining=0;
			}
		else 
			{
			swds[n].length =  p->fds[n].length;
			remaining -= p->fds[n].length;
			p->fds[n].length = 0;
			p->fds[n].ptr = 0;
			}
		n++;
		}

	if (n>p->maxFDSused) p->maxFDSused=n;
	
	compressfds( p->fds);

	queued = nsend-remaining;
	
	for(m=0; m<n; m++) {
#ifdef	NET_DEBUG_MUCHINFO
	sprintf(net_debug_string, "swds[m].ptr: %lx\tswds[m].length: %d", (unsigned int)swds[m].ptr, (int)swds[m].length);
	net_debug_print(net_debug_string);
#endif
		}
		
	for (m=0; m<n; m++) 
		{
		BlockMove(buffer, swds[m].ptr, swds[m].length);	/* BYU LSC - Put data in WDS */
	
		/*
		 * encrypt if encrypting the session
		 */
 		if (p->edata && ((CDATA *)p->edata)->encrypt_output)
 			(*((CDATA *)p->edata)->encrypt_output)((CDATA *)p->edata, (unsigned char *)swds[m].ptr, swds[m].length);

		buffer=(void *)((char *)buffer + swds[m].length);
		}
	
	swds[m].ptr = 0L;
	swds[m].length=0;
	
	pbp=getPB( TCPd, TCPSend, p->stream, 2);			/* Make send call */
	pbp->pb.csParam.send.wdsPtr = (Ptr) swds;
	pbp->pb.csParam.send.pushFlag = p->push;
	p->push=0;

	if ((err = xPBControlAsync(pbp, (TCPIOCompletionProc)SendCompleteInitUPP)) != noError) 
		{
		char temp_xvvyz[256];
		
		sprintf(temp_xvvyz, "netwrite: %d", err);
		DoError(510 | NET_ERRORCLASS, LEVEL2, temp_xvvyz);
		return(-1);
		}

	return(queued);
}

/**************************************************************************/
/*  Mnetpush
*   attempt to push the rest of the data from the queue
*   and then return whether the queue is empty or not (0 = empty)
*   returns the number of bytes in the queue.
*/
short netpush(short pnum)
{
	StreamRPtr	p;
	MyTCPpbPtr	pbp;
	short		inQ;
	OSErr		err;

	if (pnum < 0 || pnum >= NPORTS)		/* BYU 2.4.15 */
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);

	pbp=getPB( TCPd, TCPStatus, p->stream, 3);			/* Make status call */
//#ifdef	NET_DEBUG_MUCHINFO
//	sprintf(net_debug_string, "TCPd == %d", TCPd);
//	net_debug_print(net_debug_string);
//#endif
	if ((err = xPBControlSync(pbp)) != noError) {		/* status call bombed */
		char temp_xvvyz[256];

		sprintf(temp_xvvyz, "netpush: %d", err);
		DoError(509 | NET_ERRORCLASS, LEVEL2, temp_xvvyz);
		}
		
	inQ = pbp->pb.csParam.status.amtUnackedData;
	MyPBreturn( pbp);
	
	p->push=1;
	return(inQ);
}	

/**************************************************************************/
/*  Mnetqlen
*   return the number of bytes waiting to be read from the incoming queue.
*/
short netqlen(short pnum)
{
	StreamRPtr	p;
	MyTCPpbPtr	pbp;
	short		inQ;

	if (pnum < 0 || pnum >= NPORTS)		/* BYU 2.4.15 */
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);

	pbp=getPB( TCPd, TCPStatus, p->stream, 4);				/* Make status call */
	if (xPBControlSync(pbp) != noError) 					/* status call failed */
		DoError(509 | NET_ERRORCLASS, LEVEL2, "Mnetqlen");

	inQ = pbp->pb.csParam.status.amtUnreadData;
	MyPBreturn( pbp);
	
	p->push = 1;
	return(inQ);
}

/**************************************************************************/
/*  Mnetroom()
*	return how much room is available in output buffer for a connection
*/
short netroom(short pnum)
{
	StreamRPtr	p;
	short		inQ, n;

	if (pnum < 0 || pnum >= NPORTS)		/* BYU 2.4.15 */
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);

	reclaim( p);
	compressfds( p->fds);

#ifdef OLDM
	pbp=getPB( TCPd, TCPStatus, p->stream, 5);			/* Make status call */
	if (xPBControlSync(pbp) != noError) 
		DoError(509 | NET_ERRORCLASS, LEVEL1, "Mnetroom");

	inQ = pbp->csParam.status.sendWindow -
				pbp->csParam.status.amtUnackedData;
	MyPBreturn( pbp);
#else
/*#pragma unused(pbp)	/* BYU LSC */
#endif

	inQ = n = 0;
	while (p->fds[n].ptr) {
	
		inQ += p->fds[n].length;				/* add up free list space */
		n++;
		}

	return(inQ);
}

/**************************************************************************/
void netgetip(unsigned char *st)
{
	struct GetAddrParamBlock mypb;
	/* long netmask; */

	net_debug_print("Attempting getmyipaddr");
	
	WriteZero((Ptr)&mypb, sizeof(struct GetAddrParamBlock));
	
	mypb.ioCRefNum = TCPd;			/* BYU LSC - TCP driver has to be open by now */
	mypb.csCode = ipctlGetAddr;

	if (PBControl((ParmBlkPtr)&mypb, false) != noError) 
		{
		DoError(511 | NET_ERRORCLASS, LEVEL2, NULL);
		return;
		}
	
	BlockMove(&mypb.ourAddress, st, 4);	/* BYU LSC - copy the address */
	/* netmask is here if we want it, too */
}

void netfromport			/* next "open" will use this port */
  ( short port)
{
	nnfromport = (ip_port)port;
}

/**************************************************************************/
/*  Mnetest?
*  is a particular session established yet?
*  Returns 0 if the connection is in the established state.
*/
short netest(short pnum)
{
	StreamRPtr	p;
	MyTCPpbPtr	pbp;
	short		inQ;

	if (pnum < 0 || pnum >= NPORTS)		/* BYU 2.4.15 */
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);

	pbp=getPB( TCPd, TCPStatus, p->stream, 6);			/* Make status call */
	if (xPBControlSync(pbp) != noError)
		{
		DoError(509 | NET_ERRORCLASS, LEVEL2, "Mnetest");
		inQ = -1;
		}
	else 
		inQ = pbp->pb.csParam.status.connectionState !=8;
	MyPBreturn( pbp);
	
	return(inQ);
}

/**************************************************************************/
/*  Mnetlisten
*   Listen to a TCP port number and make the connection automatically when
*   the SYN packet comes in.  The TCP layer will notify the higher layers
*   with a CONOPEN event.  Save the port number returned to refer to this
*   connection.
*
*   usage:   portnum = netlisten(service);
*            short service;
*
*/

short netlisten(ip_port serv)
{
	short		pnum;
	StreamRPtr	p;
	MyTCPpbPtr	pbp;

	pnum = makestream();

	if (pnum < 0 || pnum >= NPORTS)		/* BYU 2.4.15 */
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);

	pbp=getPB( TCPd, TCPPassiveOpen, p->stream, 8);			/* Make Listen call */
	
	pbp->pb.csParam.open.localPort = serv;
	
	if (xPBControlAsync(pbp, (TCPIOCompletionProc)OpenCompleteInitUPP) != noError) 
		DoError(513 | NET_ERRORCLASS, LEVEL2, NULL);

	return(pnum);						/* BYU 2.4.16 */
}

/***********************************************************************/
/*  Mnetgetftp
*  Provides the information that ftp needs to open a stream back to the
*  originator of the command connection.  The other side's IP number
*  and the port numbers to be used to calculate the default data connection
*  number.  Returns values in an integer array for convenient use in 
*  PORT commands.
*/
short	netgetftp(short pnum, ip_addr *addr, ip_port *localPort, ip_port *remotePort)
{
	StreamRPtr p;
	MyTCPpbPtr pbp;
	
	if (pnum < 0)
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);

	pbp=getPB( TCPd, TCPStatus, p->stream, 9);			/* Make status call */
	if (xPBControlSync(pbp) != noError) 
		DoError(514 | NET_ERRORCLASS, LEVEL2, NULL);

	*addr = pbp->pb.csParam.status.remoteHost;
	*localPort = pbp->pb.csParam.status.localPort;
	*remotePort = pbp->pb.csParam.status.remotePort;
	MyPBreturn( pbp);
}

/**************************************************************************/
/*  Open a network socket for the user. */
short netxopen( ip_addr machine, ip_port service,
				short	timeout	)		// in seconds
{
	short		pnum;
	StreamRPtr	p;
	MyTCPpbPtr	pbp;

	pnum = makestream();

	if (pnum < 0)
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);

	pbp=getPB( TCPd, TCPActiveOpen, p->stream, 10);			/* Make Listen call */
	
	pbp->pb.csParam.open.remoteHost = machine;			/* IP # */
	pbp->pb.csParam.open.remotePort = service;				/* Port */
	pbp->pb.csParam.open.localPort = nnfromport;			/* My Port */
	pbp->pb.csParam.open.ulpTimeoutValue = (byte)timeout;	// timeout value
	pbp->pb.csParam.open.ulpTimeoutAction = 1;				// Abort
	pbp->pb.csParam.open.validityFlags = 0xC0;				// Timeout and action are valid
	nnfromport=0;											/* Next one is random */
	
	if (xPBControlAsync(pbp, (TCPIOCompletionProc)OpenCompleteInitUPP) != noError) {
		DoError(515 | NET_ERRORCLASS, LEVEL2, NULL);
		return(-1);
		}
	
	SetPortType(pnum, NO_TYPE);				// Is allocated to the user
	
#ifdef	NET_DEBUG
	sprintf(net_debug_string, "TCPOpen on port #%d", pnum);
	net_debug_print(net_debug_string);
#endif
	return(pnum);					/* BYU 2.4.16 */
}


/**************************************************************************/
/* Mnetclose
*  Do appropriate actions to return connection state to SCLOSED which
*  enables the memory for that port to be reused.
*
*	Specifically:
*		o If status is closed, then release the data structures
*		o If status is not close, perform bkgrd close which generates CLOSEDONE,
*			which should make the session layer call us again
*/
short netclose(short pnum)
{
	StreamRPtr	p;
	MyTCPpbPtr	pbp;
	OSErr		errorCode;
	short		status;
	static		short count=0;

	errorCode = 0;
	
	if (pnum < 0 || pnum >= NPORTS)			/* is a valid port? */
		return(-1);

	if ((p = streams[pnum]) == NULL) 			/* nothing there */
		return (1);

	pbp=getPB( TCPd, TCPStatus, p->stream, 11);			/* Make status call */
	if ((errorCode = xPBControlSync(pbp)) != noError) 
		{
		if ( errorCode == invalidStreamPtr)
			{
			DoError (516 | NET_ERRORCLASS, LEVEL2, "Mnetclose");
			return(-1);
			}
		else
			{
			status=0;
			count =0;
			}
		}
	else 
		{
		status = pbp->pb.csParam.status.connectionState;			/* The connection Status */
		if (count++ ==10) 
			{
			status= 0;
			count =0;
			}
		}

/* */

#ifdef	NET_DEBUG
	sprintf(net_debug_string, "Mnetclose:error code=%i status=%d",(int)errorCode, status);
	net_debug_print(net_debug_string);
#endif

	if (status < 18 && status >2 ) {	/* We aren't closed yet ! */
#ifdef	NET_DEBUG
		sprintf(net_debug_string, "TCPClose being attempted state ...[%d]",status); /* Prolly because outstanding close */
		net_debug_print(net_debug_string);
#endif
		clearPB( pbp, TCPd, TCPClose, p->stream);			/* Make Close call */
		if ((errorCode=xPBControlAsync(pbp, (TCPIOCompletionProc)CloseCompleteInitUPP)) != noError) 
			{			/* TCP close failed */
			DoError(517 | NET_ERRORCLASS, LEVEL2, "Mnetclose");
			return (errorCode);
			}

		/* Go ahead and return... PB is in use by Async Close call. */
		return (0);											/* Return with OK */
		}

	/* IF we got here, we must be at closed state, so free memory */

#ifdef	NET_DEBUG
	sprintf(net_debug_string, "TCP being released, status = %d", status);
	net_debug_print(net_debug_string);
#endif

	clearPB( pbp,TCPd, TCPRelease, p->stream);			/* Make Release call */
	if (xPBControlSync(pbp) != noError) 
		{				/* TCP failed to be released.  Let us know... */
		DoError (518 | NET_ERRORCLASS, LEVEL2, "Mnetclose");
		return (-1);
		}
	
	DisposPtr( p->buffer);				/* Free Receive Buffer */
	DisposPtr( p->sbuffer);				/* Free Send Buffer */
	DisposPtr((Ptr) p);					/* Free Stream Structure */
	streams[pnum]=0L;
	netportencryptstate(pnum, 0);

	MyPBreturn(pbp);
	return(0);
}

/**************************************************************************/
/*	Nuke the connection, NOW! */
short	netabort(short pnum)
{
	StreamRPtr	p;
	MyTCPpbPtr	pbp;
	OSErr		errorCode = noErr;

	if (pnum < 0 || pnum >= NPORTS)			/* is a valid port? */
		return(-1);

	if ((p = streams[pnum]) != NULL) 	/* something there */
		{			
		pbp=getPB( TCPd, TCPAbort, p->stream, 12);			/* Make Close call */
		if ((errorCode=xPBControlSync(pbp)) != noError) 		/* TCP abort failed.  Bad news */
			DoError (519 | NET_ERRORCLASS, LEVEL1, NULL);
		clearPB( pbp,TCPd, TCPRelease, p->stream);			/* Make Close call */
		if (xPBControlSync(pbp) != noError) 
			{
			DoError (518 | NET_ERRORCLASS, LEVEL2, NULL);
			return(-1);
			}
		}
	else
		return(1);
	
	DisposPtr( p->buffer);				/* Free Receive Buffer */
	DisposPtr( p->sbuffer);				/* Free Send Buffer */
	DisposPtr((Ptr) p);					/* Free Stream Structure */
	streams[pnum]=0L;

	MyPBreturn(pbp);
	return(0);
}


/**************************************************************************/
/*  Mnetinit
*   Calls all of the various initialization routines that set up queueing
*   variables, static values, reads configuration files, etc.
*/

void Mnetinit( void)
{
	short	i;
	OSErr	err;

#ifndef __powerpc__	
	MyA5 = SetCurrentA5();
#endif
	
	for (i=0; i<NPORTS;i++) {
		streams[i]= (StreamRPtr) 0;
		SetPortType(i, NO_TYPE);
		} 
		
	gFreePBQueue.qHead = NULL;
	gFreePBQueue.qTail = NULL;
	gFreePBQueue.qFlags = 0;
	gFreeSWDSQueue.qHead = NULL;
	gFreeSWDSQueue.qTail = NULL;
	gFreeSWDSQueue.qFlags = 0;
		
	if ((err = OpenDriver("\p.IPP",&TCPd)) != noError) 		/* cant open the IP driver */
		{	
		SetCursor(theCursors[normcurs]);
		FatalAlert(CANT_OPEN_MACTCP, 300, err);		// Doesn't return!
		}
}



/*************************************************************************/
/*  Mnetshut
*   Close all the connections and turn off the hardware.
*/
void netshut(void)
{
	short	i;

	for (i=0; i < NPORTS ; i++) 
		if (streams[i] != (StreamRPtr) NULL)
			netabort(i);						/* Prolly should abort */
}

/**************************************************************************/
short	findbystream(StreamPtr streamPtr)
{
	short	pnum=0;
	
	while (pnum<NPORTS &&
			( (streams[pnum] ==(StreamRPtr)0L) || (streamPtr !=streams[pnum]->stream)))
		pnum++;
	if (pnum >=NPORTS)
		return(-1);
	else
		return(pnum);
}


/**************************************************************************/
SIMPLE_UPP(Notify, TCPNotify);
pascal void Notify(StreamPtr streamPtr, unsigned short code, Ptr uptr, 
						unsigned short terminReason, struct ICMPReport *icmpMsg)
{
	#pragma unused(uptr, terminReason, icmpMsg)
	StreamRPtr	p;
	short		pnum;
	
	pnum = findbystream(streamPtr);
	
	if (pnum < 0 || (p = streams[pnum]) == 0L)
		return;
	
	switch( code) {
		case TCPTerminate:
		case TCPClosing:
			netputevent(CONCLASS, CONCLOSE, pnum,0);
			break;
		case TCPULPTimeout:
			netputevent(CONCLASS, CONFAIL, pnum,0);
			break;
		case TCPDataArrival:
		case TCPUrgent:
			netputuev(CONCLASS, CONDATA, pnum,0);
			break;
		case TCPICMPReceived:
		default:
			break;
		}
	return;
}

/**************************************************************************/
SIMPLE_UPP(OpenCompleteInit, TCPIOCompletion);
TCPIOCompletionProc OpenCompleteInit(TCPiopb *pbp)
{
	long		saveA5;
	MyTCPpbPtr	myptr;
	
	myptr = (MyTCPpbPtr)pbp;

#ifndef _powerpc_
	saveA5 = SetA5(myptr->SavedA5);
#endif

	OpenComplete(myptr);

#ifndef _powerpc_
	SetA5(saveA5);
#endif
}

/**************************************************************************/
void	OpenComplete(MyTCPpbPtr pbp)
{
	StreamRPtr	p;
	short		pnum;

	pnum= findbystream(pbp->pb.tcpStream);
	
	if (pnum < 0 || (p = streams[pnum]) == 0L) 
		MyPBreturn(pbp);
		
	if (pbp->pb.ioResult !=noError) 
		netputevent(CONCLASS, CONFAIL, pnum,0);			/* Failure ... */
	else 
		netputevent(CONCLASS, CONOPEN, pnum,0);			/* Success ! */

	MyPBreturn( pbp);
}

/**************************************************************************/
void	SendComplete(MyTCPpbPtr pbp)
{
	StreamRPtr	p;
	short		pnum;
	wdsEntry	*swds;

	swds = (wdsEntry *) pbp->pb.csParam.send.wdsPtr;
	
	pnum = findbystream(pbp->pb.tcpStream);
	
	if (pnum < 0 || (p = streams[pnum]) == 0L) {
		MyPBreturn(pbp);
		return;
		}
		
	MyPBreturn( pbp);
	giveback( p, (wdsEntry *) pbp->pb.csParam.send.wdsPtr);	/* BYU LSC - Give this back.... NOW */
	returnSWDS( swds);
}


/**************************************************************************/
SIMPLE_UPP(SendCompleteInit, TCPIOCompletion);
TCPIOCompletionProc SendCompleteInit(TCPiopb *pbp)
{
	long		saveA5;
	MyTCPpbPtr	myptr;
	
	myptr = (MyTCPpbPtr)pbp;

#ifndef _powerpc_
	saveA5 = SetA5(myptr->SavedA5);
#endif
	
	SendComplete(myptr);
	
#ifndef _powerpc_
	SetA5(saveA5);
#endif
}

/**************************************************************************/
void	CloseComplete(MyTCPpbPtr pbp)
{
	StreamRPtr	p;
	short		pnum;

	pnum= findbystream(pbp->pb.tcpStream);
	
	if (pnum < 0 || (p = streams[pnum]) == 0L)
	  {
		netputevent(SCLASS, CLOSEDONE+1, pnum,0);
		MyPBreturn(pbp);
		return;
	  }
		
	if (pbp->pb.ioResult !=noError) 
		netputevent(SCLASS, CLOSEDONE+1, pnum,0);
	else 
		netputevent(SCLASS, CLOSEDONE, pnum,0);			/* Success ! */

	MyPBreturn( pbp);
}

/**************************************************************************/
SIMPLE_UPP(CloseCompleteInit, TCPIOCompletion);
TCPIOCompletionProc CloseCompleteInit(TCPiopb *pbp)
{
	long		saveA5;
	MyTCPpbPtr	myptr;
	
	myptr = (MyTCPpbPtr)pbp;

#ifndef _powerpc_
	saveA5 = SetA5(myptr->SavedA5);
#endif
	
	CloseComplete(myptr);
	
#ifndef _powerpc_
	SetA5(saveA5);
#endif
}
 
short netportencryptstate (short port, Ptr edata)
{
	StreamRPtr	p;
 
	if (port < 0 || port >= NPORTS)
 		return(-2);
 
 	if ((p = streams[port]) == NULL)
 		return(-2);
 
 	p->edata = edata;
 
 	return 0;
}
