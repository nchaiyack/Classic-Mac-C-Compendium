/*
	UDP.h
	
	Header file for UDP.c, wrappers for the low level UDP functions.
	
*/

#pragma once

#ifndef __H_UDP__
#define __H_UDP__

#include <UDPPB.h>

// error values.
enum{
	udpBaseErr=23150,				// the base error for this section
	udpNilPB,						// the UDPiopb pointer was nil.
	udpNilRefNum,					// the ioCRefNum field of the UDPiopb was nil
	udpNilStream,					// the udpStream field of the UDPiopb was nil
	udpPortNil						// the starting port for UDPMultiCreate was 0 (not allowed).
	
};

// macros for allocating new blocks...
#define NewUDPCreatePtr() NEWPTR(UDPCreatePBPtr,sizeof(UDPCreatePB))
#define DisposeUDPCreatePtr(a) DISPOSEPTR(a)
#define NewUDPCreateHdl() NEWHDL(UDPCreatePBHdl,sizeof(UDPCreatePB))
#define DisposeUDPCreateHdl(a) DISPOSEHDL(a)

#define NewUDPSendPtr() NEWPTR(UDPSendPBPtr,sizeof(UDPSendPB))
#define DisposeUDPSendPtr(a) DISPOSEPTR(a)
#define NewUDPSendHdl() NEWHDL(UDPSendPBHdl,sizeof(UDPSendPB))
#define DisposeUDPSendHdl(a) DISPOSEHDL(a)

#define NewUDPReceivePtr() NEWPTR(UDPReceivePBPtr,sizeof(UDPReceivePB))
#define DisposeUDPReceivePtr(a) DISPOSEPTR(a)
#define NewUDPReceiveHdl() NEWHDL(UDPReceivePBHdl,sizeof(UDPReceivePB))
#define DisposeUDPReceiveHdl(a) DISPOSEHDL(a)

#define NewUDPMTUPtr() NEWPTR(UDPMTUPBPtr,sizeof(UDPMTUPB))
#define DisposeUDPMTUPtr(a) DISPOSEPTR(a)
#define NewUDPMTUHdl() NEWHDL(UDPMTUPBHdl,sizeof(UDPMTUPB))
#define DisposeUDPMTUHdl(a) DISPOSEHDL(a)

#define NewUDPiopbPtr() NEWPTR(UDPiopbPtr,sizeof(UDPiopb))
#define DisposeUDPiopbPtr(a) DISPOSEPTR(a);
#define NewUDPiopbHdl() NEWHDL(UDPiopbHdl,sizeof(UDPiopb))
#define DisposeUDPiopbHdl(a) DISPOSEHDL(a)

#ifdef __cplusplus
extern "C" {
#endif

OSErr UDPCreate(UDPiopbPtr pb,Boolean async);
OSErr UDPRead(UDPiopbPtr pb,Boolean async);
OSErr UDPBfrReturn(UDPiopbPtr pb,Boolean async);
OSErr UDPWrite(UDPiopbPtr pb,Boolean async);
OSErr UDPRelease(UDPiopbPtr pb,Boolean async);
OSErr UDPMaxMTUSize(UDPiopbPtr pb,Boolean async);
OSErr UDPStatus(UDPiopbPtr pb,Boolean async);
OSErr UDPMultiCreate(UDPiopbPtr pb,Boolean async);
OSErr UDPMultiSend(UDPiopbPtr pb,Boolean async);
OSErr UDPMultiRead(UDPiopbPtr pb,Boolean async);

#ifdef __cplusplus
}
#endif

#endif /* __H_UDP__ */
