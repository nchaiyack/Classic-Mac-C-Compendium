/*_____________________________________________________________________

  	rslv.c - Domain name resolver.
	
	This is my own home-grown name resolver. It is not used in the current
	version of Ph. (All calls to it in mtcp.c have been commented out.)
	It doesn't work with MacTCP 1.1.1, presumably because the format of the
	dnsl 128 resource has changed.
	
	I'm leaving the source code here in case I need it some day.
_____________________________________________________________________*/

/*_____________________________________________________________________

	Header Files.
_____________________________________________________________________*/

#pragma load "precompile"
#include <MacTCPCommonTypes.h>
#include <UDPPB.h>
#include "rslv.h"
#include "utl.h"
#include "fsu.h"

#pragma segment rslv

/*_____________________________________________________________________

	Constants.
_____________________________________________________________________*/

#define MacTCPFileName		"\pMacTCP"		/* MacTCP file name */
#define initialTimeout		4					/* initial timeout delay in seconds */
#define maxQueries			4					/* max number of query attempts */
#define rcvBuffSize			2048				/* size of UDP receive buffer */
#define typeA					1					/* DNS "A" type code */
#define classIN				1					/* DNS "IN" class code */
#define ourTypeClass			((typeA<<16)|classIN)	/* "A" type code and "IN" class code */
#define dnsPort				53					/* DNS UDP port number */
#define dnsNameError			3					/* DNS "name error" response code */

/*_____________________________________________________________________

	Types.
_____________________________________________________________________*/

typedef struct DNSQuery {
	unsigned short			id;				/* identifier */
	unsigned short			flags;			/* header flags word */
	unsigned short			qdCount;			/* question count */
	unsigned short			anCount;			/* answer count */
	unsigned short			nsCount;			/* authority count */
	unsigned short			arCount;			/* additional count */
	unsigned char			question[260];	/* the question */
} DNSQuery;

typedef struct DNSReply {
	unsigned short			id;				/* identifier */
	unsigned short			flags;			/* header flags word */
	unsigned short			qdCount;			/* question count */
	unsigned short			anCount;			/* answer count */
	unsigned short			nsCount;			/* authority count */
	unsigned short			arCount;			/* additional count */
	unsigned char			sections[];		/* the sections */
} DNSReply;

typedef struct WDS {
	short						length;			/* buffer length */
	Ptr						buffer;			/* pointer to buffer */
	short						zero;				/* terminating zero word */
} WDS;

/*_____________________________________________________________________

	Global Variables.
_____________________________________________________________________*/

/* The server list is maintained in a single relocatable block containing
	a list of the server IP addresses. A server address may be set to 0
	to remove it from the list (e.g., if it returns a bogus reply). */
	
static Handle				ServerList;			/* handle to server list */
static short				ServerListSize;	/* size of server list */

/* The cache is maintained in a single relocatable block containing
	a list of entries. Each entry is a Pascal format domain name string
	followed by its longword IP address. */

static Handle				Cache;			/* handle to cache */
static short				CacheSize;		/* size of cache */

static Str255				Extension;		/* default domain name extension */
static short				RefNum;			/* MacTCP driver refnum */
static unsigned short	QueryId = 0;	/* DNS query identifier */

/*_____________________________________________________________________

	rslv_DottedDecimal - Convert IP Address to Dotted Decimal String.
	
	Entry:	addr = IP address.
				str = pointer to string buffer.
	
	Exit:		str = converted string, C format.
				function result = str.
_____________________________________________________________________*/

unsigned char *rslv_DottedDecimal (unsigned long addr, unsigned char *str)

{
	short		o1,o2,o3,o4;
	
	o1 = (addr >> 24) & 0xff;
	o2 = (addr >> 16) & 0xff;
	o3 = (addr >> 8) & 0xff;
	o4 = addr & 0xff;
	sprintf(str, "%d.%d.%d.%d", o1, o2, o3, o4);
	return str;
}

/*_____________________________________________________________________

	rslv_Init - Initialize.
	
	Entry:	refNum = MacTCP driver refnum.
_____________________________________________________________________*/

#pragma segment rslvi

OSErr rslv_Init (short refNum)

{
	FSSpec				fSpec;			/* MacTCP file spec */
	short					rsrcRefNum;		/* resource file refnum */
	OSErr					rCode;			/* error code */
	Handle				h;					/* handle to dnsl resource */
	unsigned char		*p;				/* pointer into dnsl resource */
	short					ct;				/* number of items in dnsl resource */
	unsigned char		*s;				/* pointer into server list */
	unsigned char		*sEnd;			/* pointer to end of server list */
	short					i;					/* loop index */
	Boolean				def;				/* true if default server */
	short					len;				/* length of string */
	unsigned long		addr;				/* IP address */
	HParamBlockRec		pBlock;			/* PBHGetFInfo param block */
	
	RefNum = refNum;
	
	/* Open the MacTCP resource file with read-only permission,
		read the dnsl id=128 resource, and initialize the server list.
		Look for the MacTCP file first in the Control panels folder, then in the
		System folder. If this doesn't work, look for any file with type
		'cdev' and creator 'ztcp' or 'mtcp' in the System folder, in case the
		user has renamed the MacTCP control panel. */
	
	utl_CopyPString(fSpec.name, MacTCPFileName);
	if (rCode = fsu_FindFolder(kOnSystemDisk, kControlPanelFolderType, kDontCreateFolder,
		&fSpec.vRefNum, &fSpec.parID)) return rCode;
	if (rCode = fsu_FSpOpenResFile(&fSpec, fsRdPerm, &rsrcRefNum)) {
		if (rCode = fsu_FindFolder(kOnSystemDisk, kSystemFolderType,
			kDontCreateFolder, &fSpec.vRefNum, &fSpec.parID)) return rCode;
		if (rCode = fsu_FSpOpenResFile(&fSpec, fsRdPerm, &rsrcRefNum)) {
			pBlock.fileParam.ioNamePtr = fSpec.name;
			pBlock.fileParam.ioVRefNum = fSpec.vRefNum;
			pBlock.fileParam.ioFDirIndex = 1;
			pBlock.fileParam.ioDirID = fSpec.parID;
			while (!(rCode = PBHGetFInfo(&pBlock, false))) {
				if (pBlock.fileParam.ioFlFndrInfo.fdType == 'cdev' &&
					(pBlock.fileParam.ioFlFndrInfo.fdCreator == 'ztcp' ||
					pBlock.fileParam.ioFlFndrInfo.fdCreator == 'mtcp')) break;
				pBlock.fileParam.ioFDirIndex++;
				pBlock.fileParam.ioDirID = fSpec.parID;
			}
			if (rCode) return rCode;
			if (rCode = fsu_FSpOpenResFile(&fSpec, fsRdPerm, &rsrcRefNum)) return rCode;
		}
	}
	h = GetResource('dnsl', 128);
	if (!h) return ResError();
	HLock(h);
	p = *h;
	ct = *(short*)p;
	p += 2;
	ServerList = NewHandle(0);
	*Extension = ServerListSize = 0;
	for (i = 0; i < ct; i++) {
		addr = *(long*)p;
		p += 4;
		def = *p++;
		len = strlen(p);
		if (len > 254) break;
		if (def) {
			*Extension = len+1;
			*(Extension+1) = '.';
			memcpy(Extension+2, p, len);
		}
		s = *ServerList;
		sEnd = s + ServerListSize;
		while (s < sEnd) {
			if (addr == *(unsigned long*)s) break;
			s += 4;
		}
		if (s >= sEnd) {
			SetHandleSize(ServerList, ServerListSize+4);
			*(unsigned long*)(*ServerList + ServerListSize) = addr;
			ServerListSize += 4;
		}
		p += len+1;
		if ((long)p & 1) p++;
	}
	HUnlock(h);
	CloseResFile(rsrcRefNum);
	if (!ServerListSize) return rslvNoServers;
	
	/* Initialize the cache. */
	
	Cache = NewHandle(0);
	CacheSize = 0;
	return noErr;
}

#pragma segment rslv

/*_____________________________________________________________________

	CrackOctet - Crack an Octet from a String.
	
	Entry:	p = pointer into string.
	
	Exit:		function result = pointer to char following cracked octet,
					or nil if no octet found.
				*o = cracked octet.
_____________________________________________________________________*/

static unsigned char *CrackOctet (unsigned char *p, short *o)

{
	short			octet;			/* cracked octet */

	if (isdigit(*p) && (octet = atoi(p)) <= 255) {
		p += strspn(p, "0123456789");
		*o = octet;
		return p;
	}
	return nil;
}

/*_____________________________________________________________________

	SkipName - Skip Domain Name in Server Reply.
	
	Entry:	p = pointer into reply.
				pEnd = pointer to end of reply.
	
	Exit:		function result = updated pointer into reply, or 
					nil if error.
_____________________________________________________________________*/

static unsigned char *SkipName (unsigned char *p, unsigned char *pEnd)

{
	while (true) {
		if (!*p) {
			return p+1;
		} else if (*p >= 0xc0) {
			return p+2;
		} else {
			p += *p+1;
			if (p > pEnd) return nil;
		}
	}
}

/*_____________________________________________________________________

	ParseReply - Parse DNS Server Reply.
	
	Entry:	reply = pointer to reply.
				replyLen = length of reply.
				
	Exit:		function result = error code:
				*addr = IP address from reply.
_____________________________________________________________________*/

static OSErr ParseReply (DNSReply *reply, unsigned short replyLen, 
	unsigned long *addr)
	
{
	unsigned char	*replyEnd;			/* pointer to end of server reply */
	short				servRCode;			/* server reply code */
	unsigned char	*z;					/* pointer into reply */
	short				i;						/* loop index */
	short				rdLength;			/* reply rdata length */
	unsigned long	typeClass;			/* type (A) and class (IN) codes */
	
	if (replyLen < 12) return rslvServError;
	if (reply->id != QueryId) return rslvNotOurQuery;
	if (!(reply->flags & 0x0080)) return rslvServError; /* recursion not available */
	servRCode = reply->flags & 0xf;
	if (servRCode == dnsNameError) {
		return rslvNoSuchDomain;
	} else if (servRCode) {
		return rslvServError;
	}
	replyEnd = (unsigned char*)reply + replyLen;
	z = &reply->sections;
	for (i = 0; i < reply->qdCount; i++) {
		if (!(z = SkipName(z, replyEnd))) return rslvServError;
		z += 4;
	}
	for (i = 0; i < reply->anCount; i++) {
		if (!(z = SkipName(z, replyEnd))) return rslvServError;
		memcpy(&typeClass, z, 4);
		z += 8;
		memcpy(&rdLength, z, 2);
		z += 2;
		if (z + rdLength > replyEnd) return rslvServError;
		if (typeClass == ourTypeClass && rdLength == 4) {
			memcpy(addr, z, 4);
			return noErr;
		}
		z += rdLength;
	}
}

/*_____________________________________________________________________

	ResolveOne - Resolve a Domain Name Via a Single DNS Server Query.
	
	Entry:	iopb = pointer to UDP io param block.
				serverAddress = IP address of DNS server.
				name = domain name (Pascal string).
				timeOut = timeout delay in seconds.
				checkCancel = pointer to check cancel function.
	
	Exit:		function result = error code.
				*addr = IP address.
_____________________________________________________________________*/

static OSErr ResolveOne (UDPiopb *iopb, unsigned long serverAddress, 
	Str255 name, short timeOut, 
	rslv_CheckCancelPtr checkCancel, unsigned long *addr)

{
	DNSQuery			query;				/* DNS query */
	short				queryLen;			/* length of query */
	short				len;					/* length of string */
	unsigned char	*p;					/* pointer into domain name */
	unsigned char	*q, *r;				/* pointers into query */
	unsigned long	typeClass;			/* type (A) and class (IN) codes */
	WDS				wds;					/* write data structure */
	OSErr				rCode;				/* result code */
	DNSReply			*reply;				/* pointer to server reply */
	unsigned short	replyLen;			/* length of server reply */
	OSErr				bfrReturnCode;		/* result code from buffer return call */

	/* Format DNS query. */
	
	query.id = QueryId;
	query.flags = 0x0100;	/* recursion desired */
	query.qdCount = 1;
	query.anCount = 0;
	query.nsCount = 0;
	query.arCount = 0;
	len = *name;
	p = name+1;
	r = query.question;
	q = query.question+1;
	while(len--) {
		if (*p == '.') {
			*r = q-r-1;
			if (!*r || *r > 0x3f) return rslvNameSyntaxErr;
			r = q;
			q++;
		} else {
			*q++ = *p;
		}
		p++;
	}
	*r = q-r-1;
	if (!*r || *r > 0x3f) return rslvNameSyntaxErr;
	*q++ = 0;
	typeClass = ourTypeClass;
	memcpy(q, &typeClass, 4);
	queryLen = q + 4 - &query;
	
	/* Send query to server. */
	
	wds.length = queryLen;
	wds.buffer = (Ptr)&query;
	wds.zero = 0;
	iopb->csCode = UDPWrite;
	iopb->csParam.send.reserved = 0;
	iopb->csParam.send.remoteHost = serverAddress;
	iopb->csParam.send.remotePort = dnsPort;
	iopb->csParam.send.wdsPtr = (Ptr)&wds;
	iopb->csParam.send.checkSum = 0;
	iopb->csParam.send.sendLength = 0;
	if (rCode = PBControl((ParmBlkPtr)iopb, false)) return rCode;
	
	/* Wait for and process reply from server. */
	
	while (true) {
		
		/* Issue read and wait for reply */
	
		iopb->ioCompletion = nil;
		iopb->csCode = UDPRead;
		iopb->csParam.receive.timeOut = timeOut;
		iopb->csParam.receive.secondTimeStamp = 0;
		if (rCode = PBControl((ParmBlkPtr)iopb, true)) return rCode;
		while (iopb->ioResult == 1) {
			if (checkCancel && (*checkCancel)()) return rslvCancel;
		}
		if (rCode = iopb->ioResult) return rCode;
		
		/* Parse reply. */
		
		reply = (DNSReply*)iopb->csParam.receive.rcvBuff;
		replyLen = iopb->csParam.receive.rcvBuffLen;
		rCode = ParseReply(reply, replyLen, addr);
		if (replyLen) {
			iopb->csCode = UDPBfrReturn;
			iopb->csParam.receive.rcvBuff = (Ptr)reply;
			if (bfrReturnCode = PBControl((ParmBlkPtr)iopb, false)) return bfrReturnCode;
		}
		if (rCode != rslvNotOurQuery) return rCode;
	}
}

/*_____________________________________________________________________

	Resolve - Resolve a Domain Name Via DNS Server Queries.
	
	Entry:	name = domain name (Pascal string).
				checkCancel = pointer to check cancel function.
	
	Exit:		function result = error code.
				*addr = IP address.
_____________________________________________________________________*/

static OSErr Resolve (Str255 name, 
	rslv_CheckCancelPtr checkCancel, unsigned long *addr)
	
{
	Ptr				rcvBuff;				/* pointer to receive buffer */
	UDPiopb			iopb;					/* UDP io param block */
	UDPiopb			iopbRel;				/* UDP io param block for UDPRelease call */
	short				timeOut;				/* timeout delay */
	short				serverInx;			/* index in ServerList */
	short				i;						/* loop index */
	unsigned long	serverAddress;		/* IP address of server */
	OSErr				rCode;				/* result code */
	OSErr				releaseCode;		/* UDPRelease call result code */

	/* Create UDP stream. */
	
	rcvBuff = NewPtr(rcvBuffSize);
	iopb.ioCRefNum = RefNum;
	iopb.csCode = UDPCreate;
	iopb.csParam.create.rcvBuff = rcvBuff;
	iopb.csParam.create.rcvBuffLen = rcvBuffSize;
	iopb.csParam.create.notifyProc = nil;
	iopb.csParam.create.localPort = 0;
	if (rCode = PBControl((ParmBlkPtr)&iopb, false)) return rCode;
	
	/* Query the servers in round-robin order, doubling the timeout
		delay each time (exponential backoff). */
	
	timeOut = initialTimeout;
	serverInx = 0;
	QueryId++;
	for (i = 1; i <= maxQueries; i++) {
		serverAddress = *(unsigned long*)(*ServerList + serverInx);
		if (serverAddress) {
			rCode = ResolveOne(&iopb, serverAddress, name, timeOut, checkCancel, addr);
			if (rCode == rslvServError) {
				*(unsigned long*)(*ServerList + serverInx) = 0;
			} else if (rCode != commandTimeout) {
				break;
			} else if (i == maxQueries) {
				rCode = rslvNoResponse;
				break;
			} else {
				timeOut <<= 1;
			}
		}
		serverInx += 4;
		if (serverInx >= ServerListSize) serverInx = 0;
	}
	
	/* Release UDP stream. */
	
	iopbRel.ioCRefNum = RefNum;
	iopbRel.udpStream = iopb.udpStream;
	iopbRel.csCode = UDPRelease;
	releaseCode = PBControl((ParmBlkPtr)&iopbRel, false);
	DisposPtr(rcvBuff);
	
	return rCode ? rCode : releaseCode;
}

/*_____________________________________________________________________

	rslv_Resolve - Resolve a Domain Name.
	
	Entry:	name = domain name (Pascal string).
				checkCancel = pointer to check cancel function.
	
	Exit:		function result = error code.
				*addr = IP address.
_____________________________________________________________________*/

OSErr rslv_Resolve (Str255 name, rslv_CheckCancelPtr checkCancel,
	unsigned long *addr)

{
	Str255				nam;				/* copy of name */
	unsigned char		*p;				/* pointer into domain name */
	short					o1,o2,o3,o4;	/* dotted decimal octets */
	unsigned char		*c;				/* pointer into cache */
	unsigned char		*cEnd;			/* pointer to end of cache */
	short					len;				/* length of string */
	OSErr					rCode;			/* error code */
	
	/* Check for dotted decimal notation. */
	
	utl_CopyPString(nam, name);
	p2cstr(nam);
	p = nam;
	if ((p = CrackOctet(p, &o1)) && *p++ == '.' &&
		(p = CrackOctet(p, &o2)) && *p++ == '.' &&
		(p = CrackOctet(p, &o3)) && *p++ == '.' &&
		(p = CrackOctet(p, &o4)) && !*p) {
		*addr = (o1<<24) | (o2<<16) | (o3<<8) | o4;
		return noErr;
	}
	
	/* Append the default domain name extension if no dot in name. */
	
	p = nam;
	if (!strchr(p, '.')) {
		len = strlen(nam);
		if (len + *Extension <= 255) {
			memcpy(nam + len, Extension+1, *Extension);
			*(nam + len + *Extension) = 0;
		} else {
			return rslvNameSyntaxErr;
		}
	}
	c2pstr(nam);
	
	/* Check the cache. */
	
	c = *Cache;
	cEnd = c + CacheSize;
	while (c < cEnd) {
		if (EqualString(c, nam, true, true)) {
			c += *c + 1;
			memcpy(addr, c, 4);
			return noErr;
		}
		c += *c + 5;
	}
	
	/* Resolve using the domain name servers. */
	
	if (rCode = Resolve(nam, checkCancel, addr)) return rCode;
	
	/* Add the new name/address pair to the cache. */
	
	len = *nam;
	SetHandleSize(Cache, CacheSize + len + 5);
	utl_CopyPString(*Cache + CacheSize, nam);
	memcpy(*Cache + CacheSize + len + 1, addr, 4);
	CacheSize += len + 5;
	return noErr;
}
