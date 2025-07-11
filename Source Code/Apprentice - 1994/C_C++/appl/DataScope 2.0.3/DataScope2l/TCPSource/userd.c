/*
*  USER.C
*  Network library interface routines
*  Generally called by the session layer
*
*/
#define MASTERDEF 1
#include <stdio.h>
#include <memory.h>
#include "protocol.h"
#include "data.h"

#include <Devices.h>
#include "MacTCPCommonTypes.h"
#include "TCPPB.h"
#include "UDPPB.h"
#include "getmyipaddr.h"

#ifdef TRUSTING
#define initMyWorld()	/* noop */
#define myWorld()		/* noop */
#define theirWorld()	/* noop */
#endif  TRUSTING

int EtherNet = -99;		/* Signify Drivers */

#define LOWWATER 600

void initUDPstub(),UDPstub();

pascal void TCPNotify();
pascal void UDPNotify();
long openComplete();
long closeComplete();
long sendComplete();

/*
 * UDP Stuff
 */
 
#define	UDPBUFSIZ	(1024*8)
#define	getUPB(x,y,z,a)	(UDPiopb *)getPB(x,y,z,a)

typedef struct UDPRec {
	StreamPtr stream;				/* Apple's lovely Stream Pointer */
	char   *buffer;					/* Where the immovable UDP buffer is */
	uint	port;					/* Which UDP port to use */
	} UDPRec, *UDPRPtr;

UDPRPtr uport[ NPORTS];				/* our wonderful little thingies. */

/*
 * TCP Stuff
 */
#define	noError	0
#define TCPBUFSIZ	(1024*8)
#define	MAX_FDS_ELEMS	32
#define	MAX_SWDS_ELEMS	16
#define MAX_FREE_PB		128
#define MAX_FREE_SWDS	64

#define	Qcall	true
#define	noQcall	false

typedef	struct freeEntry {
	int inuse;						/* is this being used? */
	Ptr	ptr;						/* Pointer to the free entry */
	} freeEntry;
	
typedef struct exfds {
	int	inuse;						/* Is this being used */
	wdsEntry fds;					/* The real data */
	} exfds;
	
typedef struct StreamRec {
	StreamPtr stream;				/* Apple's lovely Stream Pointer */
	char   *buffer;					/* Where the immovable TCP buffer is */
	int		push;					/* TRUE if we should push next data block */
	char   *sbuffer;				/* Where the send buffer is */
	wdsEntry	fds[MAX_FDS_ELEMS];	/* Free Data Structure list */
	exfds	exFDS[MAX_FDS_ELEMS];	/* exFDS entries */
	int		maxFDSused;				/* Max of the FDS's that have been used */
	} StreamRec, *StreamRPtr;
	
short TCPd = 0;						/* refnum of TCP drivers */

StreamRPtr streams[NPORTS];

int numPB=0;						/* Number of PB's ever allocated  (Perf. mon. only ) */
int numSWDS=0;						/* Number of SWDS's ever alloc'd (PM Only) */

freeEntry	freePB[ MAX_FREE_PB];
freeEntry	freeSWDS[ MAX_FREE_SWDS];

/**************************************************************************/
wdsEntry *getSWDS()
{
	int n=0;
	
	while (freeSWDS[n].inuse &&  n<MAX_FREE_SWDS) n++;
	if (n >= MAX_FREE_SWDS)
		return(0L);
	
	freeSWDS[n].inuse=1;
	if (freeSWDS[n].ptr==0L) {
		char temp[50];
		freeSWDS[n].ptr = (char *)NewPtr ( sizeof(wdsEntry) *MAX_SWDS_ELEMS);
		numSWDS++;
		sprintf(temp,"New SWDS(%d)",numSWDS);
		putln(temp);
		}
	return( (wdsEntry *) freeSWDS[n].ptr);
}

/**************************************************************************/
returnSWDS( wds)
wdsEntry *wds;
{
	int n=0;
	
	while (freeSWDS[n].ptr != wds && n<MAX_FREE_SWDS) n++;
	if (n >= MAX_FREE_SWDS)
		return(-1);
	freeSWDS[n].inuse=0;
}

/**************************************************************************/
TCPiopb *getPB( driver, call, stream, usernum)
int driver, call, usernum;
unsigned long stream;
{
	TCPiopb *pbp;
	int n=0;
	
	while (freePB[n].inuse &&  n<MAX_FREE_PB) n++;
	if (n >= MAX_FREE_PB)
		return(0L);
	
	freePB[n].inuse=1;
	if (freePB[n].ptr==0L) {
		char temp[50];
		
		freePB[n].ptr =(char *)NewPtr ( sizeof(TCPiopb)+sizeof(int) );
		numPB++;
		sprintf(temp,"New PB(%d)",numPB);
		putln(temp);
		}
	pbp = (TCPiopb *)freePB[n].ptr;


	if (!pbp) {
		putln("GETPB failed! panic! ");
		quit();
		}
	
	memset( pbp, '\0', sizeof(TCPiopb)+sizeof(int));	/* Default to all zeros */
	
	pbp->ioCRefNum = driver;
	pbp->tcpStream=stream;
	pbp->csCode = call;
	
	return(pbp);
}

/**************************************************************************/
clearPB( pbp, driver, call, stream, usernum)
TCPiopb *pbp;
int driver, call, usernum;
unsigned long stream;
{
	memset( pbp, '\0', sizeof(TCPiopb)+sizeof(int));	/* Default to all zeros */
	
	pbp->ioCRefNum = driver;
	pbp->tcpStream=stream;
	pbp->csCode = call;
	
}

returnPB( pbp)
TCPiopb *pbp;
{
	int n=0;
	
	while (freePB[n].ptr != pbp && n<MAX_FREE_PB) n++;
	if (n >= MAX_FREE_PB)
		return(-1);
	freePB[n].inuse=0;
	
}

/***************************************************************************/
/*  netread
*   Read from a connection buffer into a user buffer.  
*   Returns number of bytes read, < 0 on error
* NOTE:
*	current version very inefficient, but hopefully works.
*/
netread(pnum,buffer,n)
	int pnum,n;
	char *buffer;
	{
	StreamRPtr p;
	TCPiopb *pbp;
	int inQ, reqdamt;
	
	if (pnum < 0)			/* check validity */
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);
	
	
	pbp=getPB( TCPd, TCPStatus, p->stream, pnum);			/* Make status call */
	if (PBControl( (ParmBlkPtr) pbp, noQcall) != noError) {
		putln("TCPStatus failed(read)"); return(-1);
		}
#ifdef DEBUGHEADERS	
	{ char temp[100];
		sprintf(temp, "Stat: %x(%d)->%x(%d) <%d,%d,%d> [%d,%d] {%d,%d}",
			(int)pbp->csParam.status.remoteHost,
			(int)pbp->csParam.status.remotePort,
			(int)pbp->csParam.status.localHost,
			(int)pbp->csParam.status.localPort,
			(int)pbp->csParam.status.tosFlags,
			(int)pbp->csParam.status.precedence,
			(int)pbp->csParam.status.connectionState,
			(int)pbp->csParam.status.sendWindow,
			(int)pbp->csParam.status.rcvWindow,
			(int)pbp->csParam.status.amtUnackedData,
			(int)pbp->csParam.status.amtUnreadData);
		putln(temp);
	}
#endif DEBUGHEADERS
	
	if (pbp->csParam.status.connectionState !=8) {
		char temp[50];
		sprintf(temp,"CState: %d is %d",(int)pnum, (int)pbp->csParam.status.connectionState);
		putln(temp);
		return(-1);					  		/* Connection not established */
		}
	
	inQ = pbp->csParam.status.amtUnreadData;
	reqdamt = n >inQ ? inQ : n;
	
	clearPB( pbp, TCPd, TCPRcv, p->stream, pnum);
	pbp->csParam.receive.rcvBuff = buffer;
	pbp->csParam.receive.rcvBuffLen = reqdamt;
	
	if (reqdamt<1) {								/* Drop out if no data */
		returnPB(pbp);
		return(0);
		}
	
	if (PBControl( (ParmBlkPtr) pbp, noQcall) != noError) {
		putln( "TCPRcv failed ");
		return(-1);
		}
		
	reqdamt = pbp->csParam.receive.rcvBuffLen;
	if (reqdamt<inQ) {
		netputuev( CONCLASS, CONDATA, pnum);			/* more data to get */
		}
		
	returnPB(pbp);					/* Trash PB */
#ifdef TESTINGPARMS
	{
		char temp[100];
		sprintf(temp, "NETRead: %d from %d", reqdamt, pnum);
		putln(temp);
	}
#endif TESTINGPARMS
	return(reqdamt);
}

/************************************************************************/
/* netwrite
*  write something into the output queue, netsleep routine will come
*  around and send the data, etc.
*
*/
netwrite(pnum,buffer,nsend)
	int pnum,nsend;
	char *buffer;
	{
	StreamRPtr p;
	wdsEntry *swds;
	int remaining, queued, n,m;
	TCPiopb *pbp;
	
	if (pnum < 0)
		return(-2);

	if ( (p = streams[pnum]) == NULL)
		return(-2);
	
	if ( !nsend )
		return(0);
		
	swds = getSWDS();
	
	reclaim( p);
	compressfds( p->fds);

	n=0; remaining = nsend;
	while (p->fds[n].ptr !=0 && remaining>0 ) {
		swds[n].ptr = p->fds[n].ptr;
		if ( p->fds[n].length > remaining) {
			swds[n].length = remaining;
			p->fds[n].length -= remaining;
			p->fds[n].ptr += remaining;
			remaining=0;
			}
		else {
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
	
	for (m=0; m<n; m++) {
		memcpy( swds[m].ptr, buffer, swds[m].length);	/* Put data in WDS */
		buffer +=swds[m].length;
		}
	swds[m].ptr =0L;
	swds[m].length=0;
	
	pbp=getPB( TCPd, TCPSend, p->stream, pnum);			/* Make send call */
	pbp->csParam.send.wdsPtr = (Ptr) swds;
	pbp->csParam.send.pushFlag = p->push;
	
	pbp->ioCompletion = (TCPIOCompletionProc) sendComplete;			/* Completion routine */

	p->push=0;

	if (PBControl( (ParmBlkPtr) pbp, Qcall) != noError) {
		putln("TCPSend failed to Q"); return(-1);
		}
	
#ifdef TESTINGPARMS
	putln("TCP Sent");
	{ char temp[100];
		sprintf(temp, "TCP Sent: %d of %d on %d [%d/%d]", queued, nsend, pnum,n,p->maxFDSused);
		putln(temp);
	}
#endif TESTINGPARMS
	return(queued);
}

/**************************************************************************/
/*  netpush
*   attempt to push the rest of the data from the queue
*   and then return whether the queue is empty or not (0 = empty)
*   returns the number of bytes in the queue.
*/
netpush(pnum)
	int pnum;
	{
	StreamRPtr p;
	TCPiopb *pbp;
	int inQ;
	
	if (pnum < 0)
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);

	pbp=getPB( TCPd, TCPStatus, p->stream, pnum);			/* Make status call */
	if (PBControl((ParmBlkPtr) pbp, noQcall) != noError) {
		putln("TCPStatus failed(push)"); return(-1);
		}
	inQ = pbp->csParam.status.amtUnackedData;
	returnPB( pbp);
	
	p->push=1;
	
	return(inQ);

}	

/**************************************************************************/
/*  netqlen
*   return the number of bytes waiting to be read from the incoming queue.
*/
netqlen(pnum)
	int pnum;
	{
	StreamRPtr p;
	TCPiopb *pbp;
	int inQ;
	
	if (pnum < 0)
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);

	pbp=getPB( TCPd, TCPStatus, p->stream, pnum);			/* Make status call */
	if (PBControl( (ParmBlkPtr) pbp, noQcall) != noError) {
		putln("TCPStatus failed(qlen)"); return(-1);
		}
	inQ = pbp->csParam.status.amtUnreadData;
	returnPB( pbp);
	
	p->push = 1;
	return(inQ);
}

/**************************************************************************/
/*  netroom()
*	return how much room is available in output buffer for a connection
*/
netroom(pnum)
	int pnum;
	{
	StreamRPtr p;
	int inQ,n;
	
	if (pnum < 0)
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);

	reclaim( p);
	compressfds( p->fds);

	inQ = n = 0;
	while (p->fds[n].ptr) {
	
		inQ += p->fds[n].length;				/* add up free list space */
		n++;
		}

	return(inQ);
}

/**************************************************************************/
/* netsegsize and neterrchange and netsetip and netgetip
*
*  set operating parameters to change them from the default values used.
*/

netsegsize(newsize)
	int newsize;
	{
	int i;

	i = nnsegsize;
	nnsegsize = newsize;

	return(i);
}

/**************************************************************************/
netquench(newcredit)
	int newcredit;
	{
	int i;

	i = nncredit;
	nncredit = newcredit;

	return(i);
}

/**************************************************************************/
netarptime(t)					/* dlayer timeout in secs */
	int t;
	{
	nndto = t;
}

/**************************************************************************/
netsetip(st)
	unsigned char *st;
	{
/*
*  this is a no-op with the MacTCP driver
*/
}

/**************************************************************************/
netgetip(st)
unsigned char *st;
{
	struct IPParamBlock mypb;

	putln("Attempting getmyipaddr");
	
	memset( &mypb, '\0', sizeof(struct IPParamBlock));	/* Default to all zeros */
	
	mypb.ioCRefNum = TCPd;			/* TCP driver has to be open by now */
	mypb.csCode = ipctlGetAddr;

	if (PBControl( (ParmBlkPtr) &mypb, noQcall) != noError) {
		putln("Getting my address failed"); 
		return(-1);
		}
	
	memcpy(st, &mypb.ourAddress, 4);	/* copy the address */
	
	/* netmask is here if we want it, too */
	
	return(0);

}


/**************************************************************************/
netsetmask(st)
unsigned char *st;
{
	movebytes(nnmask,st,4);
}

/**************************************************************************/
netgetmask(st)
unsigned char *st;
{
	movebytes(st,nnmask,4);
}

netfromport(port)			/* next "open" will use this port */
int16 port;
{
	nnfromport = port;

}

/**************************************************************************/
/*  netest?
*  is a particular session established yet?
*  Returns 0 if the connection is in the established state.
*/
netest(pnum)
int pnum;
{
	StreamRPtr p;
	TCPiopb *pbp;
	int inQ;
	
	if (pnum < 0)
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);

	pbp=getPB( TCPd, TCPStatus, p->stream, pnum);			/* Make status call */
	if (PBControl( (ParmBlkPtr) pbp, noQcall) != noError) {
		putln("TCPStatus failed(est)");
		inQ = -1;
		}
	else 
		inQ = pbp->csParam.status.connectionState !=8;
	returnPB( pbp);
	
	return(inQ);

}

/**************************************************************************/
/*  netlisten
*   Listen to a TCP port number and make the connection automatically when
*   the SYN packet comes in.  The TCP layer will notify the higher layers
*   with a CONOPEN event.  Save the port number returned to refer to this
*   connection.
*
*   usage:   portnum = netlisten(service);
*            int service;
*
*/
netlisten(serv)
uint serv;
{
	int	pnum;
	StreamRPtr p;
	uint16 nn;
	TCPiopb *pbp;

	pnum = makestream();

	if (pnum < 0)
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);

	pbp=getPB( TCPd, TCPPassiveOpen, p->stream, pnum);			/* Make Listen call */
	
	pbp->csParam.open.localPort = serv;
	pbp->ioCompletion = (TCPIOCompletionProc) openComplete;		/* IO Completion for open */
	
	if (PBControl( (ParmBlkPtr) pbp, Qcall) != noError) {
		putln("TCPListen failed"); return(-1);
		}
		
	return(pnum);
}

/***********************************************************************/
/*  netgetftp
*  Provides the information that ftp needs to open a stream back to the
*  originator of the command connection.  The other side's IP number
*  and the port numbers to be used to calculate the default data connection
*  number.  Returns values in an integer array for convenient use in 
*  PORT commands.
*/
netgetftp(a,pnum)
int a[];
int pnum;
{
	StreamRPtr p;
	TCPiopb *pbp;
	long temp;
	
	if (pnum < 0)
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);

	pbp=getPB( TCPd, TCPStatus, p->stream, pnum);			/* Make status call */
	if (PBControl( (ParmBlkPtr) pbp, noQcall) != noError) {
		putln("TCPStatus failed(getftp)"); return(-1);
		}

	temp  	= pbp->csParam.status.remoteHost;
	a[0]= (temp>>24) & 0xff;
	a[1]= (temp>>16) & 0xff;
	a[2]= (temp>> 8) & 0xff;
	a[3]= (temp    ) & 0xff;
	temp	= pbp->csParam.status.localPort;
	a[4]= (temp>> 8) & 0xff;
	a[5]= (temp    ) & 0xff;
	temp	= pbp->csParam.status.remotePort;
	a[6]= (temp>> 8) & 0xff;
	a[7]= (temp    ) & 0xff;

	returnPB( pbp);
}

/**************************************************************************/
/*  netopen
*   Netopen is a cheap way to open a connection without looking up any
*   machine information.  Uses suitable default values for everything.
*/
netopen(s,tport)
unsigned char *s;
uint tport;
{

	return(netxopen(s,tport,MINRTO,TSENDSIZE,DEFSEG,DEFWINDOW));
}


/**************************************************************************/
/*  netxopen
*   Open a network socket for the user.
*
*/
netxopen(machine,service,rto,mtu,mseg,mwin)
uint32 *machine;
uint service,rto,mtu,mseg,mwin;		/* unix service port number */
{
	int	pnum;
	StreamRPtr p;
	TCPiopb *pbp;
	char temp[100];
	
	pnum = makestream();

	if (pnum < 0)
		return(-2);

	if (NULL == (p = streams[pnum]))
		return(-2);

	pbp=getPB( TCPd, TCPActiveOpen, p->stream, pnum);			/* Make Listen call */
	
	pbp->csParam.open.remoteHost = *machine;			/* IP # */
	pbp->csParam.open.remotePort = service;				/* Port */
	pbp->csParam.open.localPort = nnfromport;			/* My Port */
	nnfromport=0;											/* Next one is random */
	
	pbp->ioCompletion = (TCPIOCompletionProc) openComplete;	/* IO Completion for open */
	
	if (PBControl( (ParmBlkPtr) pbp, Qcall) != noError) {
		putln("TCPOpen failed(Active)"); return(-1);
		}
	sprintf(temp,"TCPOpen on %d",pnum);
	putln(temp);
	return(pnum);
}


/**************************************************************************/
/* netclose
*  Do appropriate actions to return connection state to SCLOSED which
*  enables the memory for that port to be reused.
*
*	Specifically:
*		o If status is closed, then release the data structures
*		o If status is not close, perform bkgrd close which generates CLOSEDONE,
*			which should make the session layer call us again
*/
netclose(pnum)
int pnum;
{
	StreamRPtr p;
	TCPiopb *pbp;
	int errorCode=0;
	int status;

	if (pnum < 0 || pnum > NPORTS)			/* is a valid port? */
		return(-1);

	if ((p = streams[pnum]) == NULL) 			/* nothing there */
		return (1);

	pbp=getPB( TCPd, TCPStatus, p->stream, pnum);			/* Make status call */
	if ((errorCode = PBControl( (ParmBlkPtr) pbp, noQcall)) != noError) {
		if ( errorCode == invalidStreamPtr) {
			putln("TCPStatus failed because of bad stream pointer (close)");
			return(-1);
			}
		else
			status=0;
		}
	else 
		status = pbp->csParam.status.connectionState;			/* The connection Status */

	if (status < 18 && status >2 ) {							/* We aren't closed yet ! */
		char temp[50];
		sprintf(temp, "TCPClose being attempted state ...[%d]",status);	/* Prolly because outstanding close */
		putln(temp);
		clearPB( pbp, TCPd, TCPClose, p->stream, pnum);			/* Make Close call */
		pbp->ioCompletion = (TCPIOCompletionProc) closeComplete;						/* IO Completion for close */
		if ((errorCode=PBControl( (ParmBlkPtr) pbp, Qcall)) != noError) {
			char temp[50];
			sprintf(temp, "TCPClose failed...[%d]",errorCode);	/* Prolly because outstanding close */
			putln(temp);
			}
		return (0);											/* Return with OK */
		}

	/* IF we got here, we must be at closed state, so free memory */

	putln("TCP Being Released...... ");
	clearPB( pbp,TCPd, TCPRelease, p->stream, pnum);			/* Make Release call */
	if (PBControl( (ParmBlkPtr) pbp, noQcall) != noError) {
		putln("TCPRelease failed"); return(-1);
		}
	
	DisposPtr( (Ptr)  p->buffer);				/* Free Receive Buffer */
	DisposPtr( (Ptr)  p->sbuffer);				/* Free Send Buffer */
	DisposPtr( (Ptr)  p);						/* Free Stream Structure */
	streams[pnum]=0L;

	returnPB(pbp);
	return(0);
}

/**************************************************************************/
/* netabort
*	Nuke the connection, NOW!
*/
netabort(pnum)
int pnum;
{
	StreamRPtr p;
	TCPiopb *pbp;
	int errorCode=0;

	if (pnum < 0 || pnum > NPORTS)			/* is a valid port? */
		return(-1);

	if ((p = streams[pnum]) != NULL) {			/* something there */
		pbp=getPB( TCPd, TCPAbort, p->stream, pnum);			/* Make Close call */
		if ((errorCode=PBControl( (ParmBlkPtr) pbp, noQcall)) != noError) {
			char temp[50];
			sprintf(temp, "TCPAbort failed...[%d]",errorCode);
			putln(temp);
			}
		clearPB( pbp,TCPd, TCPRelease, p->stream, pnum);			/* Make Close call */
		if (PBControl( (ParmBlkPtr) pbp, noQcall) != noError) {
			putln("TCPRelease failed"); return(-1);
			}
		}
	else
		return(1);
	
	DisposPtr( (Ptr)  p->buffer);				/* Free Receive Buffer */
	DisposPtr( (Ptr)  p->sbuffer);				/* Free Send Buffer */
	DisposPtr( (Ptr)  p);						/* Free Stream Structure */
	streams[pnum]=0L;

	returnPB(pbp);
	return(0);
}

/**************************************************************************/
/*  netinit
*   Calls all of the various initialization routines that set up queueing
*   variables, static values, reads configuration files, etc.
*/

netinit()
{
	int i;
	
	for (i=0; i<NPORTS;i++)
		streams[i]= (StreamRPtr) 0;
		
	for (i=0; i<NPORTS;i++)
		uport[i]= (UDPRPtr) 0;
	
	if (opendriver(".IPP",&TCPd) != noError) {
		putln( "Couldn't open IP driver ");
		quit();
		}
		
	initMyWorld();
	initUDPstub( UDPNotify);
	
	return(0);				/* set up empty packets */
}

/*************************************************************************/
/*  netshut
*   Close all the connections and turn off the hardware.
*/
netshut()
	{
	int i;

	for (i=0; i < NPORTS ; i++) 
		if (streams[i] != (StreamRPtr) NULL)
			netabort(i);						/* Prolly should abort */
	for (i=0; i < NPORTS ; i++) 
		if (uport[i] != (UDPRPtr) NULL)
			netuclose(uport[i]->port);			/* Shut down UDP too... */
#ifdef SAFE
	CloseDriver( TCPd);
#endif SAFE
}

/**************************************************************************/
/*
 * Returns an empty stream
 */ 
makestream()
{
	int	pnum;
	StreamRPtr p;
	TCPiopb *pbp;
	int i;
	
	for ( pnum=0; streams[pnum]!= NULL && pnum<NPORTS; pnum++);
	
	if (pnum >= NPORTS)
		return(-2);

	p= streams[pnum] = (StreamRPtr) NewPtr(sizeof(StreamRec));

	if ((p->buffer = (char *)NewPtr( TCPBUFSIZ)) == (char *)NULL)
		return(-1);
	if ((p->sbuffer = (char *)NewPtr( TCPBUFSIZ)) == (char *)NULL)
		return(-1);
	
	for (i=0; i<MAX_FDS_ELEMS; i++) {
		p->fds[ i].length =0; p->fds[ i].ptr = 0L;
		p->exFDS[ i].inuse=0; p->exFDS[ i].fds.length=0;p->exFDS[ i].fds.ptr=0L;
		}
	p->fds[0].length = TCPBUFSIZ;
	p->fds[0].ptr = p->sbuffer;
	p->maxFDSused=0;
	
	pbp=getPB( TCPd, TCPCreate, 0, pnum);			/* Make create call */
	pbp->csParam.create.rcvBuff = p->buffer;
	pbp->csParam.create.rcvBuffLen = TCPBUFSIZ;
	pbp->csParam.create.notifyProc = TCPNotify;
	if (PBControl( (ParmBlkPtr) pbp, noQcall) != noError) {
		putln("TCPCreate failed"); return(-1);
		}
	
	p->stream = pbp->tcpStream;
	
	putln("Made a new stream");
	returnPB(pbp);
	return(pnum);
}


/**************************************************************************/
pascal void TCPNotify( streamPtr, code, uptr, terminReason, icmpMsg)
StreamPtr streamPtr;
unsigned short code;
unsigned short terminReason;
struct ICMPReport *icmpMsg;
Ptr uptr;   /* user data pointer */
{
	StreamRPtr p;
	int pnum;
	
	myWorld();
	pnum= findbystream(streamPtr);
	
	if (pnum<0 || (p=streams[pnum])==0L) {
		theirWorld();
		return;
		}
	
	switch( code) {
		case TCPTerminate:
		case TCPClosing:
			netputevent(CONCLASS, CONCLOSE, pnum);
			break;
		case TCPULPTimeout:
			netputevent(CONCLASS, CONFAIL, pnum);
			break;
		case TCPDataArrival:
		case TCPUrgent:
			netputuev(CONCLASS, CONDATA, pnum);
			break;
		case TCPICMPReceived:
		default:
			break;
		}
	theirWorld();
	return;
}

/**************************************************************************/
int findbystream( streamPtr)
StreamPtr streamPtr;
{
	int pnum=0;
	
	while (pnum<NPORTS &&
			( (streams[pnum] ==(StreamRPtr)0L) || (streamPtr !=streams[pnum]->stream)))
		pnum++;
	if (pnum >=NPORTS)
		return(-1);
	else
		return(pnum);
}

/*************************************************************************/
/*  netopen2
*   Send out repeat SYN on a connection which is not open yet
*   Checks, and only sends one if needed.
*   Returns 1 if the state is still SYNS and 0 if the connection has proceeded.
*   The timing is all handled at a higher layer.
*/
netopen2(pnum)
int pnum;
{
	return( netest(pnum));
}

/**************************************************************************/
long openComplete( pbp)
TCPiopb *pbp;
{
	StreamRPtr p;
	int pnum;
	
	myWorld();
	pnum= findbystream(pbp->tcpStream);
	
	if (pnum<0 || (p=streams[pnum])==0L) {
		theirWorld();
		return(-1);
		}
		
	if (pbp->ioResult !=noError) 
		netputevent(CONCLASS, CONFAIL, pnum);			/* Failure ... */
	else 
		netputevent(CONCLASS, CONOPEN, pnum);			/* Success ! */

	returnPB( pbp);

	theirWorld();
	return(0);
	
}

/**************************************************************************/
long sendComplete( pbp)
TCPiopb *pbp;
{
	StreamRPtr p;
	int pnum;
	wdsEntry *swds;
	int i=0,j=0;
	
	myWorld();
	swds = (wdsEntry *) pbp->csParam.send.wdsPtr;
	
	pnum= findbystream(pbp->tcpStream);
	if (pnum<0 || (p=streams[pnum])==0L) {
		theirWorld();
		return(-1);
		}

	returnSWDS( swds);
	returnPB( pbp);

	giveback( p, pbp->csParam.send.wdsPtr);			/* Give this back.... NOW */
	
	theirWorld();
	return(0);
}


/**************************************************************************/
long closeComplete( pbp)
TCPiopb *pbp;
{
	StreamRPtr p;
	int pnum;
	
	myWorld();
	pnum= findbystream(pbp->tcpStream);
	
	if (pnum<0 || (p=streams[pnum])==0L) {
		theirWorld();
		netputevent(SCLASS, CLOSEDONE+1, pnum);
		return(-1);
		}
		
	if (pbp->ioResult !=noError) 
		netputevent(SCLASS, CLOSEDONE+1, pnum);
	else 
		netputevent(SCLASS, CLOSEDONE, pnum);			/* Success ! */

	returnPB( pbp);

	theirWorld();
	return(0);
	
}


/**************************************************************************/
/*
 * 	compressfds( fds)
 *		compress an fds data structure to make everyone happy
 */
 
compressfds( fds)
wdsEntry *fds;
{
	int n,m,compressed;
	
	compressed = 0;
	
	while ( !compressed) {
		compressed=1;
		for (n=0; n< MAX_FDS_ELEMS; n++) {				/* Slow Forwards */
			if (fds[n].ptr) {								/* Do if N exists */
				for ( m = MAX_FDS_ELEMS -1; m>=0; m--) {	/* Fast Backwards */
					if (fds[m].ptr && (fds[m].ptr+fds[m].length == fds[n].ptr)) {
						fds[n].length+=fds[m].length;
						fds[n].ptr = fds[m].ptr;
						fds[m].ptr=0L;
						fds[m].length=0;
						compressed=0;
						}
#ifdef CHECKBOTHWAYZ
					else 
					if (fds[n].ptr+fds[n].length == fds[m].ptr) {
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
	
	while (n+m < MAX_FDS_ELEMS) {
		while (fds[n+m].ptr ==0L && n+m< MAX_FDS_ELEMS) {
			m++;			/* increase gap to valid entry */
			}
		if (n+m<MAX_FDS_ELEMS)
			fds[n]=fds[n+m];
		n++;
		}
	
	/* Get rid of the empty spaces */
	
	n--;		/* for the next loop */
	while (n < MAX_FDS_ELEMS) {
		fds[n].ptr=0; fds[n++].length=0;
		}
}

/**************************************************************************/
/* 
 *	reclaim( p) -
 *		reclaims buffer space to stream (from pointer p) into the FDS list 
 */
 
reclaim(p)
StreamRPtr p;
{
	int n=0, offset=0;
	
	while (offset < MAX_FDS_ELEMS && p->fds[offset].ptr != 0L) offset++;
	
	if (offset >= MAX_FDS_ELEMS) {
		putln("Couldn't reclaim because offset was too large ");
		return(0);
		}
	for (n=0 ; n<MAX_FDS_ELEMS && offset< MAX_FDS_ELEMS; n++) {
		if (p->exFDS[ n].inuse) {
			p->fds[ offset++]=p->exFDS[ n].fds;
			p->exFDS[ n].inuse = 0;
			}
		}
}

/**************************************************************************/
/*
 *	giveback( p, wds) -
 *		gives WDS entries back to the stream by putting them in the 
 *		mutually exclusive buffer.
 *	p -> stream
 *	wds -> wds array
 */
giveback( p, wds)
StreamRPtr p;
wdsEntry *wds;
{
	int n=0, m=0;
	
	while ( n< MAX_SWDS_ELEMS && wds[n].ptr !=0L) {
		while (m< MAX_FDS_ELEMS && p->exFDS[ m].inuse) m++;
		if (m> MAX_FDS_ELEMS)
			return(-1);				/* No room in the RECLAIMation center */
		else {
			p->exFDS[ m].inuse =1;
			p->exFDS[ m].fds = wds[n];
			m++;
			}
		n++;
		}
}



/*****************************************************************************
 *
 * Here lie the awful UDP routines, I put them here for the drivers from Apple.
 *
 */
 
 int UDPlisten =0;				/* what port the old routines listen for */
 

/****************************************************************************/
/*					New UDP routines....									*/
/****************************************************************************/

/**************************************************************************/
/*
 * netuopen (port) 			- open the udp port "Port"
 */

netuopen(port)
int port;
{

	return(makeuport( port));
}

/**************************************************************************/
/*
 * netuclose( port) 			- close the udp port 
 */

netuclose( port)
int port;
{
	UDPRPtr p;
	UDPiopb *pbp;
	int pnum;
	
	pnum= UDPfindport( port);
	
	if (pnum<0)
		return(-1);
	
	p=uport[pnum];
	
	pbp = getUPB( TCPd, UDPRelease, p->stream, 0);
	if (PBControl( (ParmBlkPtr) pbp, noQcall) != noError) {
		putln("UDPClose failed"); return(-1);
		}
	DisposPtr( (Ptr)  p->buffer);
	DisposPtr( (Ptr)  uport[pnum]);
	uport[pnum]=0;				/* use me again */
	
	returnPB( pbp);
}

/**************************************************************************/
/*
 *	netuget( port, buffer,len, who,where)
 *					- read up to len bytes from port port into buffer buffer, noting
 *						who it was from and where....
 */

netuget( port, buffer, len, who, where)
int port, len;
int *who, *where;
char *buffer;
{
	int pnum, length;
	UDPRPtr p;
	UDPiopb *pbp;
	
	pnum= UDPfindport( port);
	
	if (pnum<0)
		return(-1);
	
	p=uport[pnum];
	
	pbp= getUPB( TCPd, UDPRead, p->stream, 0);
	pbp->csParam.receive.timeOut = 1;							/* time out at one sec. */
	
	if (PBControl( (ParmBlkPtr) pbp, noQcall) != noError) {
		putln("UDPRead failed"); return(-1);
		}
	
	length = pbp->csParam.receive.rcvBuffLen;				/* look how BIG it is */
	length = length > len ? len:length;
	
	memcpy( buffer, pbp->csParam.receive.rcvBuff,length);
	
	pbp->csCode = UDPBfrReturn;								/* Let my buffer go.. */
	if (PBControl( (ParmBlkPtr) pbp, noQcall) != noError) {
		putln("UDPReturn failed"); return(-1);
		}
		
	returnPB( pbp);
	
	return(length);
}

/**************************************************************************/
netuput( machine, port, myport, buffer, n)
long *machine;
char *buffer;
int port, myport;
int n;
{
	wdsEntry wds[2];
	UDPRPtr p;
	UDPiopb *pbp; int pnum;
	
	pnum= UDPfindport( myport);
	
	if (pnum<0)
		return(-1);
	
	p=uport[pnum];
	
	pbp= getUPB( TCPd, UDPWrite, p->stream, 0);
	pbp->csParam.send.remoteHost = *machine;
	pbp->csParam.send.remotePort = port;
	pbp->csParam.send.checkSum   = 1;					/* Do do that checksum that you do so well */
	pbp->csParam.send.wdsPtr = (Ptr) wds;
	pbp->csParam.send.remoteHost = *machine;
	
	wds[0].ptr = buffer;
	wds[0].length=n;
	wds[1].ptr = (char *) 0L; wds[1].length=0;
	
	if (PBControl( (ParmBlkPtr) pbp, noQcall) != noError) {
		putln("UDPReturn failed"); return(-1);
		}
		
	returnPB( pbp);
	return(0);
}


/****************************************************************************/
/*  neturead
*   get the data from the UDP buffer
*   Returns the number of bytes transferred into your buffer, -1 if none here
*   This needs work.
*/
neturead(buffer)
char *buffer;
{
	int who, where;
	
	if (!UDPlisten)
		return(-1);
		
	return( netuget( UDPlisten, buffer, 512, &who, &where));
}

/***************************************************************************/
/*  netulisten
*   Specify which UDP port number to listen to.
*   Can only listen to one at a time.
*/
netulisten(port)
int port;
{
	char temp[50];
	int pnum;

	
	sprintf( temp, "UDP listening on ....%d", port);
	putln(temp);
	
	UDPlisten = port;
	
	if ( (pnum=UDPfindport( port))<0)
		pnum= netuopen(port);
	
	return (pnum);
}

/***************************************************************************/
/*  netusend
*   send some data out in a UDP packet
*   uses the preinitialized data in the port packet ulist.udpout
*   
*   returns 0 on okay send, nonzero on error
*/
netusend(machine,port,retport,buffer,n)
unsigned char *machine,*buffer;
unsigned int port,retport;
int n;
{
	/* find if port is open */
	if ( UDPfindport( retport)<0)
		netuopen(retport);
	
	/* Send data */
	netuput( machine, port, retport, buffer,n);
}

/**************************************************************************/
makeuport( port)
int port;
{
	int	pnum;
	UDPRPtr p;
	UDPiopb *pbp;
	int i;
	
	for ( pnum=0; uport[pnum]!= NULL && pnum<NPORTS; pnum++);
	
	if (pnum >= NPORTS)
		return(-2);

	p= uport[pnum] = (UDPRPtr)NewPtr(sizeof(UDPRec));

	if ((p->buffer = (char *)NewPtr( UDPBUFSIZ)) == (char *)NULL)
		return(-1);
	
	
	pbp=getUPB( TCPd, UDPCreate, 0, pnum);			/* Make create call */
	
	pbp->csParam.create.rcvBuff  = p->buffer;
	pbp->csParam.create.rcvBuffLen= UDPBUFSIZ;
	pbp->csParam.create.notifyProc	 = UDPNotify;
	pbp->csParam.create.localPort = port;
	
	if ((i=PBControl( (ParmBlkPtr) pbp, noQcall)) != noError) {
		char temp[50];
		sprintf(temp, "UDPCreate failed (%d)",i);
		putln(temp);
		return(-1);
		}
	else {
		char temp[50];
		sprintf(temp, "UDPCreate successfull on %d(%d) [%x]",port,pnum,pbp->udpStream);
		putln(temp);
		}
	
	p->stream = pbp->udpStream;
	p->port = port;
	
	putln("Made a new UPORT");
	returnPB(pbp);
	return(pnum);
}

/**************************************************************************/
int ufindbystream( streamPtr)
StreamPtr streamPtr;
{
	int pnum=0;
	
	while (pnum<NPORTS &&
			( uport[pnum] ==(UDPRPtr)0L || streamPtr !=uport[pnum]->stream))
		pnum++;
	if (pnum >=NPORTS)
		return(-1);
	else
		return(pnum);
}

/**************************************************************************/
int UDPfindport( port)
int port;
{
	int pnum=0;
	
	while (pnum<NPORTS &&
			( uport[pnum] ==(UDPRPtr)0L || port !=uport[pnum]->port))
		pnum++;
	if (pnum >=NPORTS)
		return(-1);
	else
		return(pnum);
}

/**************************************************************************/
pascal void UDPNotify( streamPtr, code, uptr, icmpMsg)
StreamPtr streamPtr;
unsigned short code;
struct ICMPReport *icmpMsg;
Ptr uptr;   /* user data */
{
	UDPRPtr p;
	int pnum;
	
	myWorld();
	pnum= ufindbystream(streamPtr);
	
	if (pnum<0 || (p=uport[pnum])==0L) {
		theirWorld();
		return;
		}
	
	switch( code) {
		case UDPDataArrival:
			netputuev(USERCLASS,UDPDATA,p->port);		/* post that it is here */
		default:
			break;
		}
	theirWorld();
	return;
}

netconfig( hardware)
char *hardware;
{
	putln("I'm a driver TCP, I don't need hardware.....");
	initipnum(0);
}

netarpme(s)
char *s;
{
	putln("Drivers don't need arps, either.");
}

netsetgate(s)
char *s;
{
	putln("Yeah, right....");
}

netgetrarp()
{
	putln("RARP handled above me....");
}

uint8 *getdlayer()
{
	putln("This shouldn't be called...");
	return(0L);
}

tcpsend()
{
}

demux()
{
	return(0);
}

/*************************************************************************/
/* neteventinit
*  load up the pointers for the event queue
*  makes a circular list to follow, required for error messages
*/
neteventinit()
	{
	int i;

	for (i=0; i < NEVENTS; i++)
		nnq[i].next = i+1;

	nnq[NEVENTS-1].next = -1;

	nnefirst = 0;
	nnelast = 0;
	nnefree = 1;
}

getATaddress()
{
}

KIPfindgate()
{
}

KIPgetns()
{
}

KIPgetdynam()
{
}

KIPregister()
{
}
