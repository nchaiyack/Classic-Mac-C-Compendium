/*
	TCP.h
	
	Header file for TCP.c, wrappers for the low level TCP functions.
	
*/

#pragma once

#ifndef __H_TCP__
#define __H_TCP__

#include <TCPPB.h>

// special error codes returned by these routines
// (MacTCP uses -23xxx, so the library uses 23xxx.  This section uses 231xx.)

enum {
	tcpBaseErr=23100,					// the base for one of these errors
	tcpNilPB,							// the TCPiopb pointer was nil.
	tcpNilStream,						// the ioCRefNum field of the TCPiopb was nil.
	tcpNilRefNum						// the ioRefNum field of the TCPiopb was nil.
};

#define NewTCPCreatePtr() NEWPTR(TCPCreatePBPtr,sizeof(TCPCreatePB))
#define DisposeTCPCreatePtr(a) DISPOSEPTR(a)
#define NewTCPCreateHdl() NEWHDL(TCPCreatePBHdl,sizeof(TCPCreatePB))
#define DisposeTCPCreateHdl(a) DISPOSEHDL(a)

#define NewTCPOpenPtr() NEWPTR(TCPOpenPBPtr,sizeof(TCPOpenPB))
#define DisposeTCPOpenPtr(a) DISPOSEPTR(a)
#define NewTCPOpenHdl() NEWHDL(TCPOpenPBHdl,sizeof(TCPOpenPB))
#define DisposeTCPOpenHdl(a) DISPOSEHDL(a)

#define NewTCPSendPtr() NEWPTR(TCPSendPBPtr,sizeof(TCPSendPB))
#define DisposeTCPSendPtr(a) DISPOSEPTR(a)
#define NewTCPSendHdl() NEWHDL(TCPSendPBHdl,sizeof(TCPSendPB))
#define DisposeTCPSendHdl(a) DISPOSEHDL(a)

#define NewTCPReceivePtr() NEWPTR(TCPReceivePBPtr,sizeof(TCPReceivePB))
#define DisposeTCPReceivePtr(a) DISPOSEPTR(a)
#define NewTCPReceiveHdl() NEWHDL(TCPReceivePBHdl,sizeof(TCPReceivePB))
#define DisposeTCPReceiveHdl(a) DISPOSEHDL(a)

#define NewTCPClosePtr() NEWPTR(TCPClosePBPtr,sizeof(TCPClosePB))
#define DisposeTCPClosePtr(a) DISPOSEPTR(a)
#define NewTCPCloseHdl() NEWHDL(TCPClosePBHdl,sizeof(TCPClosePB))
#define DisposeTCPCloseHdl(a) DISPOSEHDL(a)

#define NewHistoBucketPtr() NEWPTR(HistoBucketPtr,sizeof(HistoBucket))
#define DisposeHistoBucketPtr(a) DISPOSEPTR(a)
#define NewHistoBucketHdl() NEWHDL(HistoBucketHdl,sizeof(HistoBucket))
#define DisposeHistoBucketHdl(a) DISPOSEHDL(a)

#define NewTCPConnectionStatsPtr() NEWPTR(TCPConnectionStatsPBPtr,sizeof(TCPConnectionStatsPB))
#define DisposeTCPConnectionStatsPtr(a) DISPOSEPTR(a)
#define NewTCPConnectionStatsHdl() NEWHDL(TCPConnectionStatsPBHdl,sizeof(TCPConnectionStatsPB))
#define DisposeTCPConnectionStatsHdl(a) DISPOSEHDL(a)

#define NewTCPStatusPtr() NEWPTR(TCPStatusPBPtr,sizeof(TCPStatusPB))
#define DisposeTCPStatusPtr(a) DISPOSEPTR(a)
#define NewTCPStatusHdl() NEWHDL(TCPStatusPBHdl,sizeof(TCPStatusPB))
#define DisposeTCPStatusHdl(a) DISPOSEHDL(a)

#define NewTCPAbortPtr() NEWPTR(TCPAbortPBPtr,sizeof(TCPAbortPB))
#define DisposeTCPAbortPtr(a) DISPOSEPTR(a)
#define NewTCPAbortHdl() NEWHDL(TCPAbortPBHdl,sizeof(TCPAbortPB))
#define DisposeTCPAbortHdl(a) DISPOSEHDL(a)

#define NewTCPParamPtr() NEWPTR(TCPParamPBPtr,sizeof(TCPParamPB))
#define DisposeTCPParamPtr(a) DISPOSEPTR(a)
#define NewTCPParamHdl() NEWHDL(TCPParamPBHdl,sizeof(TCPParamPB))
#define DisposeTCPParamHdl(a) DISPOSEHDL(a)

#define NewTCPStatsPtr() NEWPTR(TCPStatsPBPtr,sizeof(TCPStatsPB))
#define DisposeTCPStatsPtr(a) DISPOSEPTR(a)
#define NewTCPStatsHdl() NEWHDL(TCPStatsPBHdl,sizeof(TCPStatsPB))
#define DisposeTCPStatsHdl(a) DISPOSEHDL(a)

#define NewTCPGlobalInfoPtr() NEWPTR(TCPGlobalInfoPBPtr,sizeof(TCPGlobalInfoPB))
#define DisposeTCPGlobalInfoPtr(a) DISPOSEPTR(a)
#define NewTCPGlobalInfoHdl() NEWHDL(TCPGlobalInfoPBHdl,sizeof(TCPGlobalInfoPB))
#define DisposeTCPGlobalInfoHdl(a) DISPOSEHDL(a)

#define NewTCPiopbPtr() NEWPTR(TCPiopbPtr,sizeof(TCPiopb))
#define DisposeTCPiopbPtr(a) DISPOSEPTR(a);
#define NewTCPiopbHdl() NEWHDL(TCPiopbHdl,sizeof(TCPiopb))
#define DisposeTCPiopbHdl(a) DISPOSEHDL(a)

#ifdef __cplusplus
extern "C" {
#endif

OSErr TCPCreate(TCPiopbPtr pb,Boolean async);
OSErr TCPPassiveOpen(TCPiopbPtr pb,Boolean async);
OSErr TCPActiveOpen(TCPiopbPtr pb,Boolean async);
OSErr TCPSend(TCPiopbPtr pb,Boolean async);
OSErr TCPNoCopyRcv(TCPiopbPtr pb,Boolean async);
OSErr TCPRcvBfrReturn(TCPiopbPtr pb,Boolean async);
OSErr TCPRcv(TCPiopbPtr pb,Boolean async);
OSErr TCPClose(TCPiopbPtr pb,Boolean async);
OSErr TCPAbort(TCPiopbPtr pb,Boolean async);
OSErr TCPStatus(TCPiopbPtr pb,Boolean async);
OSErr TCPExtendedStat(TCPiopbPtr pb,Boolean async);
OSErr TCPRelease(TCPiopbPtr pb,Boolean async);
OSErr TCPGlobalInfo(TCPiopbPtr pb,Boolean async);

#ifdef __cplusplus
}
#endif

#endif /* __H_TCP__ */
