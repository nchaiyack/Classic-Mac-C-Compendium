/* 
	TCPPB.h	
	C definitions of parameter block entries needed for TCP calls

	Copyright Apple Computer, Inc. 1988-91
	All rights reserved
	
	Modifications by Dave Nebinger (dnebing@andy.bgsu.edu), 01/30/94.
*/

#pragma once

#ifndef __H_TCPPB__
#define __H_TCPPB__

#ifndef __TYPES__
#include <Types.h>
/*	#include <ConditionalMacros.h>									*/
/*	#include <MixedMode.h>										*/
/*		#include <Traps.h>										*/
#endif

/* Command codes */
#define kTCPCreate				30
#define kTCPPassiveOpen			31
#define kTCPActiveOpen			32
#define kTCPSend				34
#define kTCPNoCopyRcv			35
#define kTCPRcvBfrReturn		36
#define kTCPRcv				37
#define kTCPClose				38
#define kTCPAbort				39
#define kTCPStatus				40
#define kTCPExtendedStat			41
#define kTCPRelease				42
#define kTCPGlobalInfo			43
#define kTCPCtlMax				49

enum TCPEventCode {
	TCPClosing = 1,
	TCPULPTimeout,
	TCPTerminate,
	TCPDataArrival,
	TCPUrgent,
	TCPICMPReceived,
	lastEvent = 32767
};

enum TCPTerminationReason {
	TCPRemoteAbort = 2,
	TCPNetworkFailure,
	TCPSecPrecMismatch,
	TCPULPTimeoutTerminate,
	TCPULPAbort,
	TCPULPClose,
	TCPServiceError,
	lastReason = 32767
};

enum {					/* ValidityFlags */
	timeoutValue		= 0x80,
	timeoutAction		= 0x40,
	typeOfService		= 0x20,
	precedence		= 0x10
};

enum {					/* TOSFlags */
	lowDelay			= 0x01,
	throughPut		= 0x02,
	reliability			= 0x04
};

typedef enum TCPEventCode TCPEventCode;
typedef enum TCPTerminationReason TCPTerminationReason;

typedef unsigned short tcp_port;
typedef unsigned char byte;

typedef struct TCPCreatePB TCPCreatePB,* TCPCreatePBPtr,** TCPCreatePBHdl;
typedef struct TCPOpenPB TCPOpenPB,* TCPOpenPBPtr,** TCPOpenPBHdl;
typedef struct TCPSendPB TCPSendPB,* TCPSendPBPtr,** TCPSendPBHdl;
typedef struct TCPReceivePB TCPReceivePB,* TCPReceivePBPtr,** TCPReceivePBHdl;
typedef struct TCPClosePB TCPClosePB,* TCPClosePBPtr,** TCPClosePBHdl;
typedef struct HistoBucket HistoBucket,* HistoBucketPtr,** HistoBucketHdl;
typedef struct TCPConnectionStats TCPConnectionStats,* TCPConnectionStatsPtr,** TCPConnectionStatsHdl;
typedef struct TCPStatusPB TCPStatusPB,* TCPStatusPBPtr,** TCPStatusPBHdl;
typedef struct TCPAbortPB TCPAbortPB,* TCPAbortPBPtr,** TCPAbortPBHdl;
typedef struct TCPParam TCPParam,* TCPParamPtr,** TCPParamHdl;
typedef struct TCPStats TCPStats,* TCPStatsPtr,** TCPStatsHdl;
typedef struct TCPGlobalInfoPB TCPGlobalInfoPB,* TCPGlobalInfoPBPtr,** TCPGlobalInfoPBHdl;
typedef struct TCPiopb TCPiopb,* TCPiopbPtr,** TCPiopbHdl;

typedef pascal void (*TCPNotifyProcPtr) (StreamPtr tcpStream,ushort eventCode,Ptr userDataPtr,ushort terminReason,
			ICMPReportPtr icmpMsg);
typedef void (*TCPIOCompletionProcPtr)(TCPiopbPtr iopb);

#if USESROUTINEDESCRIPTORS
	enum {
		uppTCPNotifyProcInfo=kPascalStackBased
			| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(StreamPtr)))
			| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(ushort)))
			| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(Ptr)))
			| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(ushort)))
			| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(ICMPReportPtr))),
		uppTCPIOCompletionProcInfo=kCStackBased
			| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(TCPiopbPtr)))
	};
	
	typedef UniversalProcPtr TCPNotifyUPP;
	typedef UniversalProcPtr TCPIOCompletionUPP;
	
	#define CallTCPNotifyProc(routine,stream,event,data,term,mesg)\
		CallUniversalProc((UniversalProcPtr)routine,uppTCPNotifyProcInfo,(stream),(event),(data),(term),(mesg))
	#define NewTCPNotifyProc(routine)\
		(TCPNotifyUPP)NewRoutineDescriptor((ProcPtr)(routine),uppTCPNotifyProcInfo,GetCurrentISA())
	
	#define CallTCPIOCompletionProc(routine,iopb)\
		CallUniversalProc((UniversalProcPtr)routine,uppTCPIOCompletionProcInfo,(iopb))
	#define NewTCPIOCompletionProc(routine)\
		(TCPIOCompletionUPP)NewRoutineDescriptor((ProcPtr)(routine),uppTCPIOCompletionProcInfo,GetCurrentISA())
#else
	typedef TCPNotifyProcPtr TCPNotifyUPP;
	typedef TCPIOCompletionProcPtr TCPIOCompletionUPP;
	
	#define CallTCPNotifyProc(routine,stream,event,data,term,mesg)\
		(*(routine))((stream),(event),(data),(term),(mesg))
	#define NewTCPNotifyProc(routine)\
		(TCPNotifyUPP)(routine)
	
	#define CallTCPIOCompletionProc(routine,iopb)\
		(*(routine))((iopb))
	#define NewTCPIOCompletionProc(routine)\
		(TCPIOCompletionUPP)(routine)
#endif

// these two macros will expand correctly for either case
#define DisposeTCPNotifyProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)
#define DisposeTCPIOCompletionProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPCreatePB {
	Ptr			rcvBuff;
	ulong			rcvBuffLen;
	TCPNotifyUPP	notifyProc;
	Ptr			userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPOpenPB {
	byte		ulpTimeoutValue;
	byte		ulpTimeoutAction;
	byte		validityFlags;
	byte		commandTimeoutValue;
	ip_addr	remoteHost;
	tcp_port	remotePort;
	ip_addr	localHost;
	tcp_port	localPort;
	byte		tosFlags;
	byte		precedence;
	Boolean	dontFrag;
	byte		timeToLive;
	byte		security;
	byte		optionCnt;
	byte		options[40];
	Ptr		userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif
	
#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPSendPB {
	byte			ulpTimeoutValue;
	byte			ulpTimeoutAction;
	byte			validityFlags;
	Boolean		pushFlag;
	Boolean		urgentFlag;
	wdsEntryPtr	wdsPtr;
	ulong			sendFree;
	ushort		sendLength;
	Ptr			userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif
	
#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPReceivePB {	/* for TCPRcv, TCPNoCopyRcv, & TCPRcvBfrReturn */
	byte 			commandTimeoutValue;
	byte 			filler;
	Boolean 		markFlag;
	Boolean 		urgentFlag;
	Ptr 			rcvBuff;
	ushort 		rcvBuffLen;
	rdsEntryPtr 	rdsPtr;
	ushort 		rdsLength;
	ushort 		secondTimeStamp;
	Ptr 			userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif
	
#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPClosePB {
	byte 		ulpTimeoutValue;
	byte 		ulpTimeoutAction;
	byte 		validityFlags;
	Ptr 		userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif
	
#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct HistoBucket {
	unsigned short 	value;
	unsigned long 	counter;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif
	
#define NumOfHistoBuckets	7

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPConnectionStats {
	ulong 		dataPktsRcvd;
	ulong 		dataPktsSent;
	ulong 		dataPktsResent;
	ulong 		bytesRcvd;
	ulong 		bytesRcvdDup;
	ulong 		bytesRcvdPastWindow;
	ulong  		bytesSent;
	ulong 		bytesResent;
	ushort 		numHistoBuckets;
	HistoBucket 	sentSizeHisto[NumOfHistoBuckets];
	ushort 		lastRTT;
	ushort 		tmrSRTT;
	ushort 		rttVariance;
	ushort 		tmrRTO;
	byte 			sendTries;
	byte 			sourchQuenchRcvd;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif
	
#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPStatusPB {
	byte 					ulpTimeoutValue;
	byte 					ulpTimeoutAction;
	long 					unused;
	ip_addr 				remoteHost;
	tcp_port 				remotePort;
	ip_addr 				localHost;
	tcp_port 				localPort;
	byte 					tosFlags;
	byte 					precedence;
	byte 					connectionState;
	ushort 				sendWindow;
	ushort 				rcvWindow;
	ushort 				amtUnackedData;
	ushort 				amtUnreadData;
	Ptr 					securityLevelPtr;
	ulong 				sendUnacked;
	ulong 				sendNext;
	ulong 				congestionWindow;
	ulong 				rcvNext;
	ulong 				srtt;
	ulong 				lastRTT;
	ulong 				sendMaxSegSize;
	TCPConnectionStatsPtr	connStatPtr;
	Ptr					userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif
	
#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPAbortPB {
	Ptr					userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPParam {
	ulong		tcpRtoA;
	ulong		tcpRtoMin;
	ulong		tcpRtoMax;
	ulong		tcpMaxSegSize;
	ulong		tcpMaxConn;
	ulong		tcpMaxWindow;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPStats {
	ulong		tcpConnAttempts;
	ulong		tcpConnOpened;
	ulong		tcpConnAccepted;
	ulong		tcpConnClosed;
	ulong		tcpConnAborted;
	ulong		tcpOctetsIn;
	ulong		tcpOctetsOut;
	ulong		tcpOctetsInDup;
	ulong		tcpOctetsRetrans;
	ulong		tcpInputPkts;
	ulong		tcpOutputPkts;
	ulong		tcpDupPkts;
	ulong		tcpRetransPkts;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif
	
#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPGlobalInfoPB {
	TCPParamPtr		tcpParamPtr;
	TCPStatsPtr		tcpStatsPtr;
#ifdef RUBBISH
	StreamPtr		*tcpCDBTable[];
#else
	StreamPtr		*tcpCDBTable;
#endif
	Ptr 				userDataPtr;
	ushort			maxTCPConnections;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif
	
#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPiopb {
	char 					fill12[12];
	TCPIOCompletionUPP		ioCompletion;
	short 				ioResult;
	char 					*ioNamePtr;		
	short 				ioVRefNum;		
	short				ioCRefNum;			
	short 				csCode;
	StreamPtr		 	tcpStream;				
	union {
		TCPCreatePB		create;
		TCPOpenPB		open;
		TCPSendPB		send;
		TCPReceivePB		receive;
		TCPClosePB		close;
		TCPAbortPB		abort;
		TCPStatusPB		status;
		TCPGlobalInfoPB	globalInfo;
	} csParam;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#endif /* __H_TCPPB__ */



