/* 
	MacTCPCommonTypes.h  
	C type definitions used throughout MacTCP.
						
	Copyright Apple Computer, Inc. 1988-91 
	All rights reserved
	
	Modifications by Dave Nebinger (dnebing@andy.bgsu.edu), 01/30/94.
	
	06/12/95 - Added standard UPP's for pascal and C routines.
*/

#pragma once

#ifndef __H_MacTCPCommonTypes__
#define __H_MacTCPCommonTypes__

#ifndef __TYPES__
#include <Types.h>
#endif /* __TYPES__ */

#ifndef NEWBLOCK
#define NEWBLOCK

#define NEWPTR(a,b) ((a)NewPtr(b))
#define DISPOSEPTR(a) (DisposePtr((Ptr)a))

#define NEWHDL(a,b) ((a)NewHandle(b))
#define DISPOSEHDL(a) (DisposeHandle((Handle)a))
#endif /* NEWBLOCK */

/* MacTCP return Codes in the range -23000 through -23049 */
#define inProgress				1				/* I/O in progress */

#define ipBadLapErr				-23000			/* bad network configuration */
#define ipBadCnfgErr			-23001			/* bad IP configuration error */
#define ipNoCnfgErr				-23002			/* missing IP or LAP configuration error */
#define ipLoadErr				-23003			/* error in MacTCP load */
#define ipBadAddr				-23004			/* error in getting address */
#define connectionClosing			-23005			/* connection is closing */
#define invalidLength			-23006
#define connectionExists			-23007			/* request conflicts with existing connection */
#define connectionDoesntExist		-23008			/* connection does not exist */
#define insufficientResources		-23009			/* insufficient resources to perform request */
#define invalidStreamPtr			-23010
#define streamAlreadyOpen		-23011
#define connectionTerminated		-23012
#define invalidBufPtr			-23013
#define invalidRDS				-23014
#define invalidWDS				-23014
#define openFailed				-23015
#define commandTimeout			-23016
#define duplicateSocket			-23017

/* Error codes from internal IP functions */
#define ipDontFragErr			-23032			/* Packet too large to send w/o fragmenting */
#define ipDestDeadErr			-23033			/* destination not responding */
#define icmpEchoTimeoutErr 		-23035			/* ICMP echo timed-out */
#define ipNoFragMemErr			-23036			/* no memory to send fragmented pkt */
#define ipRouteErr				-23037			/* can't route packet off-net */

#define nameSyntaxErr 			-23041		
#define cacheFault				-23042
#define noResultProc			-23043
#define noNameServer			-23044
#define authNameErr				-23045
#define noAnsErr				-23046
#define dnrErr					-23047
#define outOfMemory			-23048

#define BYTES_16WORD   			2				/* bytes per 16 bit ip word */
#define BYTES_32WORD    		4				/* bytes per 32 bit ip word */
#define BYTES_64WORD    		8				/* bytes per 64 bit ip word */

enum ICMPMsgType {
	netUnreach, hostUnreach, protocolUnreach, portUnreach, fragReqd,
	sourceRouteFailed, timeExceeded, parmProblem, missingOption,
	lastICMPMsgType = 32767
};

typedef enum ICMPMsgType ICMPMsgType;
	
typedef unsigned long BufferPtr;
typedef unsigned long StreamPtr;
typedef unsigned char b_8;				/* 8-bit quantity */
typedef unsigned short b_16;				/* 16-bit quantity */
typedef unsigned long b_32;				/* 32-bit quantity */
typedef b_16 ip_port;
typedef b_32 ip_addr;					/* IP address is 32-bits */

typedef struct ip_addrbytes ip_addrbytes,IPAddr,* IPAddrPtr,** IPAddrHdl;
typedef struct wdsEntry wdsEntry,* wdsEntryPtr,** wdsEntryHdl;
typedef struct rdsEntry rdsEntry,* rdsEntryPtr,** rdsEntryHdl;
typedef struct ICMPReport ICMPReport,* ICMPReportPtr,** ICMPReportHdl;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct ip_addrbytes {
	union {
		b_32		addr;
		char		byte[4];
	} a;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif
	
#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct wdsEntry {
	ushort	length;							/* length of buffer */
	char*	ptr;								/* pointer to buffer */
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct rdsEntry {
	ushort	length;							/* length of buffer */
	char*	ptr;								/* pointer to buffer */
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct ICMPReport {
	StreamPtr		streamPtr;
	ip_addr			localHost;
	ip_port			localPort;
	ip_addr			remoteHost;
	ip_port			remotePort;
	enum ICMPMsgType	reportType;
	ushort			optionalAddlInfo;
	ulong				optionalAddlInfoPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

/* First comes the pascal definitions for UPP's */

typedef pascal OSErr (*PascalOSErrProcPtr)(void);
typedef pascal Ptr (*PascalPtrProcPtr)(void);
typedef pascal Boolean (*PascalBooleanProcPtr)(void);
typedef pascal void (*PascalVoidProcPtr)(void);

enum {
	uppPascalOSErrProcInfo=kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr))),
	uppPascalPtrProcInfo=kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(Ptr))),
	uppPascalBooleanProcInfo=kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(Boolean))),
	uppPascalVoidProcInfo=kPascalStackBased
};

#if USESROUTINEDESCRIPTORS
	typedef UniversalProcPtr PascalOSErrUPP;
	typedef UniversalProcPtr PascalPtrUPP;
	typedef UniversalProcPtr PascalBooleanUPP;
	typedef UniversalProcPtr PascalVoidUPP;
	
	#define CallPascalOSErrProc(routine)\
		CallUniversalProc((UniversalProcPtr)routine,uppPascalOSErrProcInfo)
	#define NewPascalOSErrProc(routine)\
		(PascalOSErrUPP)NewRoutineDescriptor((ProcPtr)(routine),uppPascalOSErrProcInfo,GetCurrentISA())
	
	#define CallPascalPtrProc(routine)\
		CallUniversalProc((UniversalProcPtr)routine,uppPascalPtrProcInfo)
	#define NewPascalPtrProc(routine)\
		(PascalPtrUPP)NewRoutineDescriptor((ProcPtr)(routine),uppPascalPtrProcInfo,GetCurrentISA())
	
	#define CallPascalBooleanProc(routine)\
		CallUniversalProc((UniversalProcPtr)routine,uppPascalBooleanProcInfo)
	#define NewPascalBooleanProc(routine)\
		(PascalBooleanUPP)NewRoutineDescriptor((ProcPtr)(routine),uppPascalBooleanProcInfo,GetCurrentISA())
	
	#define CallPascalVoidProc(routine)\
		CallUniversalProc((UniversalProcPtr)routine,uppPascalVoidProcInfo)
	#define NewPascalVoidProc(routine)\
		(PascalVoidUPP)NewRoutineDescriptor((ProcPtr)(routine),uppPascalVoidProcInfo,GetCurrentISA())
	
#else
	typedef PascalOSErrProcPtr PascalOSErrUPP;
	typedef PascalPtrProcPtr PascalPtrUPP;
	typedef PascalBooleanProcPtr PascalBooleanUPP;
	typedef PascalVoidProcPtr PascalVoidUPP;
	
	#define CallPascalOSErrProc(routine)\
		(*(routine))()
	#define NewPascalOSErrProc(routine)\
		(PascalOSErrUPP)(routine)
	
	#define CallPascalPtrProc(routine)\
		(*(routine))()
	#define NewPascalPtrProc(routine)\
		(PascalPtrUPP)(routine)
	
	#define CallPascalBooleanProc(routine)\
		(*(routine))()
	#define NewPascalBooleanProc(routine)\
		(PascalBooleanUPP)(routine)
	
	#define CallPascalVoidProc(routine)\
		(*(routine))()
	#define NewPascalVoidProc(routine)\
		(PascalVoidUPP)(routine)
	
#endif

// these four macros will expand correctly for either case
#define DisposePascalOSErrProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)
#define DisposePascalPtrProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)
#define DisposePascalBooleanProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)
#define DisposePascalVoidProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)

/* Now comes the C definitions for the UPP's */

typedef OSErr (*COSErrProcPtr)(void);
typedef Ptr (*CPtrProcPtr)(void);
typedef Boolean (*CBooleanProcPtr)(void);
typedef void (*CVoidProcPtr)(void);

enum {
	uppCOSErrProcInfo=kCStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr))),
	uppCPtrProcInfo=kCStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(Ptr))),
	uppCBooleanProcInfo=kCStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(Boolean))),
	uppCVoidProcInfo=kCStackBased
};

#if USESROUTINEDESCRIPTORS
	typedef UniversalProcPtr COSErrUPP;
	typedef UniversalProcPtr CPtrUPP;
	typedef UniversalProcPtr CBooleanUPP;
	typedef UniversalProcPtr CVoidUPP;
	
	#define CallCOSErrProc(routine)\
		CallUniversalProc((UniversalProcPtr)routine,uppCOSErrProcInfo)
	#define NewCOSErrProc(routine)\
		(COSErrUPP)NewRoutineDescriptor((ProcPtr)(routine),uppCOSErrProcInfo,GetCurrentISA())
	
	#define CallCPtrProc(routine)\
		CallUniversalProc((UniversalProcPtr)routine,uppCPtrProcInfo)
	#define NewCPtrProc(routine)\
		(CPtrUPP)NewRoutineDescriptor((ProcPtr)(routine),uppCPtrProcInfo,GetCurrentISA())
	
	#define CallCBooleanProc(routine)\
		CallUniversalProc((UniversalProcPtr)routine,uppCBooleanProcInfo)
	#define NewCBooleanProc(routine)\
		(CBooleanUPP)NewRoutineDescriptor((ProcPtr)(routine),uppCBooleanProcInfo,GetCurrentISA())
	
	#define CallCVoidProc(routine)\
		CallUniversalProc((UniversalProcPtr)routine,uppCVoidProcInfo)
	#define NewCVoidProc(routine)\
		(CVoidUPP)NewRoutineDescriptor((ProcPtr)(routine),uppCVoidProcInfo,GetCurrentISA())
	
#else
	typedef COSErrProcPtr COSErrUPP;
	typedef CPtrProcPtr CPtrUPP;
	typedef CBooleanProcPtr CBooleanUPP;
	typedef CVoidProcPtr CVoidUPP;
	
	#define CallCOSErrProc(routine)\
		(*(routine))()
	#define NewCOSErrProc(routine)\
		(COSErrUPP)(routine)
	
	#define CallCPtrProc(routine)\
		(*(routine))()
	#define NewCPtrProc(routine)\
		(CPtrUPP)(routine)
	
	#define CallCBooleanProc(routine)\
		(*(routine))()
	#define NewCBooleanProc(routine)\
		(CBooleanUPP)(routine)
	
	#define CallCVoidProc(routine)\
		(*(routine))()
	#define NewCVoidProc(routine)\
		(CVoidUPP)(routine)
	
#endif

// these two macros will expand correctly for either case
#define DisposeCOSErrProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)
#define DisposeCPtrProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)
#define DisposeCBooleanProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)
#define DisposeCVoidProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)

#endif /* __H_MacTCPCommonTypes__ */


