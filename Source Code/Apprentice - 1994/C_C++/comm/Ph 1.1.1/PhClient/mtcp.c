/*_____________________________________________________________________

  	mtcp.c - MacTCP Interface.
_____________________________________________________________________*/

/*_____________________________________________________________________

	Header Files.
_____________________________________________________________________*/

#pragma load "precompile"
#include <MacTCPCommonTypes.h>
#include <AddressXlation.h>
#include <TCPPB.h>
#include "mtcp.h"
#include "utl.h"
#include "oop.h"
#include "rslv.h"
#include "serv.h"
#include "glob.h"

#pragma segment mtcp

/*_____________________________________________________________________

	Constants.
_____________________________________________________________________*/

#define bufferSize		20000			/* size of MacTCP buffer */
#define rcvTimeout		120			/* TCPRcv timeout in seconds */
#define rcvGranularity	100			/* TCPRcv granularity */
#define rcvExpand			1000			/* Response buffer expansion increment */
#define responseLimit	50000			/* Response buffer size limit */

/*_____________________________________________________________________

	Types.
_____________________________________________________________________*/

typedef struct Wds {
	short				len;				/* length of buffer */
	char				*buf;				/* ptr to buffer */
	short				zero;				/* zero terminator */
} Wds;

typedef struct ResolverPBlock {
	Boolean						free;			/* true if block is free (not busy) */
	struct hostInfo			info;			/* MacTCP param block */
	struct ResolverPBlock	*next;		/* pointer to next block in list */
} ResolverPBlock;

/*_____________________________________________________________________

	Global Variables.
_____________________________________________________________________*/

static short				MacTCPDriverRefnum;	/* driver refnum */
static TCPiopb				MacTCPParmBlk;			/* PBControl param block */
static char					*MacTCPBuffer;			/* ptr to MacTCP buffer */
static StreamPtr			Stream;					/* ptr to stream, or nil */
static Handle				Response;				/* handle to response buffer */
static Boolean				Connected;				/* true if connected */
static Boolean				Canceled;				/* true if transaction canceled */
static ResolverPBlock	*ResolverPBlockList = nil;	/* ptr to linked list of resolver
																param blocks */

/*_____________________________________________________________________

	CheckCancel - Check for User Cancel.
	
	Exit:		function result = true if user cancel.
_____________________________________________________________________*/

static Boolean CheckCancel (void)

{
	oop_DoEvent(nil, everyEvent, 0, nil);
	return Canceled;
}

/*_____________________________________________________________________

	mtcp_BeginTransaction - Begin Transaction.
_____________________________________________________________________*/

void mtcp_BeginTransaction (void)

{
	Canceled = false;
}

/*_____________________________________________________________________

	mtcp_CancelTransaction - Cancel Transaction.
_____________________________________________________________________*/

void mtcp_CancelTransaction (void)

{
	Canceled = true;
}

/*_____________________________________________________________________

	DoPBControl - Make a PBControl Call.
	
	Exit:		function result = error code.
_____________________________________________________________________*/

static OSErr DoPBControl (void)

{
	PBControl((ParmBlkPtr)&MacTCPParmBlk, true);
	while (MacTCPParmBlk.ioResult == inProgress && !CheckCancel());
	return Canceled ? mtcpCancel : MacTCPParmBlk.ioResult;
}

/*_____________________________________________________________________

	ResolverResultProc - Resolver Result Proc for Call to StrToAddr
_____________________________________________________________________*/

static pascal void ResolverResultProc (struct hostInfo *hostInfoPtr, 
	char *userDataPtr)

{
#pragma unused (hostInfoPtr)
	
	*userDataPtr = true;
}

/*_____________________________________________________________________

	InitMacTCPParmBlk - Initialize MacTCP PBControl Parameter Block.

	Entry:	pBlock = pointer to param block.
				csCode = code for type of call.

	Exit:		MacTCPParmBlk initialized.	
_____________________________________________________________________*/

static void InitMacTCPParmBlk (TCPiopb *pBlock, short csCode)

{
	memset(pBlock, 0, sizeof(TCPiopb));
	pBlock->ioCRefNum = MacTCPDriverRefnum;
	pBlock->csCode = csCode;
	pBlock->tcpStream = Stream;
}

/*_____________________________________________________________________

	mtcp_Initialize - Initialize MacTCP.
_____________________________________________________________________*/

#pragma segment init

OSErr mtcp_Initialize (void)

{
	OSErr			rCode;			/* result code */

	MacTCPBuffer = NewPtr(bufferSize);
	if (!MacTCPBuffer) return MemError();
	if (rCode = OpenDriver("\p.IPP", &MacTCPDriverRefnum)) return rCode;
	if (rCode = OpenResolver(nil)) return rCode;
	/*return rslv_Init(MacTCPDriverRefnum);*/
	return noErr;
}

#pragma segment mtcp

/*_____________________________________________________________________

	mtcp_Terminate - Terminate MacTCP.
_____________________________________________________________________*/

OSErr mtcp_Terminate (void)

{
	ResolverPBlock		*pBlock;			/* pointer to resolver param block */
	
	pBlock = ResolverPBlockList;
	while (pBlock) {
		while (!pBlock->free);
		pBlock = pBlock->next;
	}
	return CloseResolver();
}

/*_____________________________________________________________________

	mtcp_OpenConnection - Open Connection to Ph Server.
	
	Entry:	host = Ph server host domain name.
	
	Exit:		function result = error code.
				if no errors:
					Stream = pointer to created TCP stream.
					Connected = true.
_____________________________________________________________________*/

OSErr mtcp_OpenConnection (char *host)

{
	OSErr					rCode;			/* result code */
	unsigned long		ipAddr;			/* ip address of host */
	Str255				server;			/* server domain name */
	ResolverPBlock		*pBlock;			/* ptr to resolver param block */
	short					nTry;				/* number of times to try MacTCP resolver */
	
	Stream = nil;
	Connected = false;
	utl_CopyPString(server, host);
	p2cstr(server);
	nTry = 2;
	while (true) {
		pBlock = ResolverPBlockList;
		while (pBlock && !pBlock->free) pBlock = pBlock->next;
		if (!pBlock) {
			pBlock = (ResolverPBlock*)NewPtr(sizeof(ResolverPBlock));
			pBlock->next = ResolverPBlockList;
			ResolverPBlockList = pBlock;
		}
		pBlock->free = false;
		rCode = StrToAddr(server, &pBlock->info, ResolverResultProc, &pBlock->free);
		if (rCode == cacheFault) {
			while (!pBlock->free && !CheckCancel());
		} else {
			pBlock->free = true;
			if (rCode) return rCode;
		}
		if (Canceled) return mtcpCancel;
		rCode = pBlock->info.rtnCode;
		if (rCode == cacheFault || rCode == outOfMemory) {
			/* handle problem with MacTCP resolver - error code is
				still set to cacheFault (MacTCP v1.0.1) or outOfMemory
				(MacTCP v1.1) after result proc has been called! In this 
				case, first try closing and reopening the MacTCP resolver 
				and try it again. If it fails again, give up and call my 
				own resolver. */
			if (--nTry) {
				if (rCode = CloseResolver()) return rCode;
				if (rCode = OpenResolver(nil)) return rCode;
				continue;
			}
			/*if (rCode = rslv_Resolve(host, CheckCancel, &ipAddr))
				return Canceled ? mtcpCancel : rCode;*/
			return rCode;
			break;
		} else if (rCode) {
			return rCode;
		} else {
			ipAddr = pBlock->info.addr[0];
			break;
		}
	}
	InitMacTCPParmBlk(&MacTCPParmBlk, TCPCreate);
	MacTCPParmBlk.csParam.create.rcvBuff = MacTCPBuffer;
	MacTCPParmBlk.csParam.create.rcvBuffLen = bufferSize;
	if (rCode = DoPBControl()) return rCode;
	Stream = MacTCPParmBlk.tcpStream;
	InitMacTCPParmBlk(&MacTCPParmBlk, TCPActiveOpen);
	MacTCPParmBlk.csParam.open.remoteHost = ipAddr;
	MacTCPParmBlk.csParam.open.remotePort = PhPort;
	if (rCode = DoPBControl()) return rCode;
	Connected = true;
	return noErr;
}

/*_____________________________________________________________________

	mtcp_CloseConnection - Close Ph Server Connection.
	
	Exit:		function result = error code.
				Connection closed and stream released.
				Connected = false.
				Stream = nil.
_____________________________________________________________________*/

OSErr mtcp_CloseConnection (void)

{
	OSErr			rCodeClose;			/* TCPClose result code */
	OSErr			rCodeRelease;		/* Release result code */
	TCPiopb		releaseBlock;		/* Release param block */

	rCodeClose = rCodeRelease = noErr;
	if (Connected) {
		InitMacTCPParmBlk(&MacTCPParmBlk, TCPClose);
		rCodeClose = DoPBControl();
		Connected = false;
	}
	if (Stream) {
		InitMacTCPParmBlk(&releaseBlock, TCPRelease);
		rCodeRelease = PBControl((ParmBlkPtr)&releaseBlock, false);
		Stream = nil;
	}
	if (rCodeClose) return rCodeClose;
	return rCodeRelease;
}

/*_____________________________________________________________________

	mtcp_AbortConnection - Abort Ph Server Connection.
	
	Exit:		function result = error code.
				Connection aborted and stream released.
				Connected = false.
				Stream = nil.
_____________________________________________________________________*/

OSErr mtcp_AbortConnection (void)

{
	OSErr			rCodeAbort;			/* TCPAbort result code */
	OSErr			rCodeRelease;		/* Release result code */
	TCPiopb		abortBlock;			/* Abort param block */
	TCPiopb		releaseBlock;		/* Release param block */

	rCodeAbort = rCodeRelease = noErr;
	if (Connected) {
		InitMacTCPParmBlk(&abortBlock, TCPAbort);
		rCodeAbort = PBControl((ParmBlkPtr)&abortBlock, false);
		Connected = false;
	}
	if (Stream) {
		InitMacTCPParmBlk(&releaseBlock, TCPRelease);
		rCodeRelease = PBControl((ParmBlkPtr)&releaseBlock, false);
		Stream = nil;
	}
	if (rCodeAbort) return rCodeAbort;
	return rCodeRelease;
}

/*_____________________________________________________________________

	mtcp_PhCommand - Execute a Ph Server Command.
	
	Entry:	command = handle to command string, terminated by a line feed.
				response = handle to allocated response buffer.
				
	Exit:		function result = error code.
				response = handle to response.
				truncated = true if reponse truncated.
					
	The response memory block size in increased if necessary to hold the
	full response, up to a limit of responseLimit bytes.
	
	The response block contains a leading line feed character, followed
	by the response text exactly as received from the server, followed by
	a zero byte.
	
	Any carriage return characters in the response block are stripped out.
_____________________________________________________________________*/

OSErr mtcp_PhCommand (Handle command, Handle response,
	Boolean *truncated)

{
	Wds			wds;							/* MacTCP wds */
	long			responseSize;				/* size of response buffer */
	long			next;							/* index in response of next char to read */
	short			ct;							/* timeout counter */
	char			*p;							/* pointer into string */
	char			*q;							/* pointer into string */
	char			*r;							/* pointer into string */
	long			n;								/* number of chars read */
	Boolean		done;							/* true when entire response received */
	short			code;							/* server response code */
	short			len;							/* length of database off message */
	OSErr			rCodeSend;					/* send result code */
	OSErr			rCodeRcv;					/* receive result code */
	
	*DbOffMsg = 0;
	HLock(command);
	p = *command;
	q = strchr(p, '\r');
	wds.len = q-p+1;
	wds.buf = p;
	wds.zero = 0;
	InitMacTCPParmBlk(&MacTCPParmBlk, TCPSend);
	MacTCPParmBlk.csParam.send.wdsPtr = (Ptr)&wds;
	rCodeSend = DoPBControl();
	HUnlock(command);
	if (rCodeSend && rCodeSend != connectionTerminated && 
		rCodeSend != connectionClosing) return rCodeSend;
	responseSize = GetHandleSize(response);
	next = 1;
	**response = '\r';
	HLock(response);
	done = false;
	ct = rcvTimeout;
	*truncated = false;
	rCodeRcv = noErr;
	while (!done && ct--) {
		if (next + rcvGranularity + 1 > responseSize) {
			if (responseSize < responseLimit) {
				responseSize += rcvExpand;
			} else {
				responseSize = responseLimit + rcvGranularity;
				next = responseLimit;
			}
			HUnlock(response);
			SetHandleSize(response, responseSize);
			HLock(response);
		}
		p = *response + next;
		InitMacTCPParmBlk(&MacTCPParmBlk, TCPRcv);
		MacTCPParmBlk.csParam.receive.commandTimeoutValue = 1;
		MacTCPParmBlk.csParam.receive.rcvBuff = p;
		MacTCPParmBlk.csParam.receive.rcvBuffLen = rcvGranularity;
		rCodeRcv = DoPBControl();
		if (rCodeRcv == commandTimeout) continue;
		if (rCodeRcv) break;
		ct = rcvTimeout;
		n = MacTCPParmBlk.csParam.receive.rcvBuffLen;
		q = p + n - 1;
		r = q;
		if (*r == '\r') {
			while (--r >= *response) {
				if (*r == '\r') {
					r++;
					code = atoi(r);
					done = code >= 200;
					if (code == phDatabaseOff) {
						r = strpbrk(r, ":\r");
						if (*r == ':') {
							r++;
							r += strspn(r, " \t");
						}
						len = q-r;
						if (len > 255) len = 255;
						memcpy(DbOffMsg+1, r, len);
						*DbOffMsg = len;
						rCodeRcv = connectionTerminated;
					}
					break;
				}
			}
		}
		next += n;
	}
	if (!done) {
		if (!rCodeRcv) rCodeRcv = commandTimeout;
		HUnlock(response);
		SetHandleSize(response, 1);
		**response = 0;
		return rCodeRcv;
	}
	if (*truncated = next >= responseLimit) {
		next = responseLimit - 1;
		*(*response + next - 1) = '\r';
	}
	*(*response + next) = 0;
	p = *response;
	q = *response;
	while (*p) {
		if (*p != '\n') *q++ = *p;
		p++;
	}
	*q = 0;
	HUnlock(response);
	return rCodeSend ? rCodeSend : rCodeRcv;
}
