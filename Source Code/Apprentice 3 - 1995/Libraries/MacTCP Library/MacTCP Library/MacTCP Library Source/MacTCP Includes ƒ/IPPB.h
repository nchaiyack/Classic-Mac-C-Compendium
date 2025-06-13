/* 
	IPPB.h	
	C definitions of parameter block entries needed for IP calls

	Copyright Apple Computer, Inc. 1988-91 
	All rights reserved
	
	Modifications by Dave Nebinger (dnebing@andy.bgsu.edu), 01/30/94.
*/

#pragma once

#ifndef __H_IPPB__
#define __H_IPPB__

#ifndef __TYPES__
#include <Types.h>
/*	#include <ConditionalMacros.h>									*/
/*	#include <MixedMode.h>										*/
/*		#include <Traps.h>										*/
#endif

#include <AppleTalk.h>

/* control codes */
#define ipctlEchoICMP		17			/* send icmp echo */
#define ipctlLAPStats		19			/* get lap stats */

// error codes
#define icmpEchoTimeoutErr 		-23035			/* ICMP echo timed-out */

#define NBP_TABLE_SIZE			20				/* number of NBP table entries */
#define NBP_MAX_NAME_SIZE		16+10+2
#define ARP_TABLE_SIZE			20				/* number of ARP table entries */

typedef struct nbp_entry nbp_entry,* nbp_entryPtr,** nbp_entryHdl;
typedef struct Enet_addr Enet_addr,* Enet_addrPtr,** Enet_addrHdl;
typedef struct arp_entry arp_entry,* arp_entryPtr,** arp_entryHdl;
typedef struct LAPStats LAPStats,* LAPStatsPtr,** LAPStatsHdl;
typedef struct IPEchoPB IPEchoPB,* IPEchoPBPtr,** IPEchoPBHdl;
typedef struct LAPStatsPB LAPStatsPB,* LAPStatsPBPtr,** LAPStatsPBHdl;
typedef struct IPParamBlock IPParamBlock,* IPParmBlkPtr,** IPParmBlkHdl,IPiopb,* IPiopbPtr,** IPiopbHdl;
typedef struct icmpEchoInfo ICMPEchoInfo,* ICMPEchoInfoPtr,** ICMPEchoInfoHdl;
typedef struct ICMPParamBlock ICMPParamBlock,* ICMPParmBlkPtr,** ICMPParmBlkHdl;

typedef void (*ICMPIOCompletionProcPtr)(ICMPParmBlkPtr iopb);

#if USESROUTINEDESCRIPTORS
	enum {
		uppICMPIOCompletionProcInfo=kCStackBased
			| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(ICMPParmBlkPtr)))
	};
	
	typedef UniversalProcPtr ICMPIOCompletionUPP;
	
	#define CallICMPIOCompletionProc(routine,iopb)\
		CallUniversalProc((UniversalProcPtr)routine,uppICMPIOCompletionProcInfo,(iopb))
	#define NewICMPIOCompletionProc(routine)\
		(ICMPIOCompletionUPP)NewRoutineDescriptor((ProcPtr)(routine),uppICMPIOCompletionProcInfo,GetCurrentISA())
#else
	typedef ICMPIOCompletionProcPtr ICMPIOCompletionUPP;
	
	#define CallICMPIOCompletionProc(routine,iopb)\
		(*(routine))((iopb))
	#define NewICMPIOCompletionProc(routine)\
		(ICMPIOCompletionUPP)(routine)
#endif

// this macro will expand correctly for either case
#define DisposeICMPIOCompletionProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct nbp_entry {
	ip_addr 		ip_address;				/* IP address */
	AddrBlock		at_address;				/* matching AppleTalk address */
	Boolean		gateway;					/* TRUE if entry for a gateway */
	Boolean		valid;					/* TRUE if LAP address is valid */
	Boolean		probing;					/* TRUE if NBP lookup pending */
	int			age;						/* ticks since cache entry verified */
	int			access;					/* ticks since last access */
	char			filler[116];				/* for internal use only !!! */
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct Enet_addr {
	b_16		en_hi;
	b_32		en_lo;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct arp_entry {
	short		age;				/* cache aging field */
	b_16			protocol;			/* Protocol type */
	ip_addr		ip_address;		/* IP address */
	Enet_addr		en_address;		/* matching Ethernet address */
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct LAPStats {
	short	ifType;
	char*	ifString;
	short	ifMaxMTU;
	long		ifSpeed;
	short	ifPhyAddrLength;
	char*	ifPhysicalAddress;
	
	union {
		arp_entry*	arp_table;	
		nbp_entry*	nbp_table;
	} AddrXlation;
	
	short	slotNumber;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct IPEchoPB{
	ip_addr				dest;				/* echo to IP address */
	wdsEntry				data;
	short				timeout;
	Ptr					options;
	ushort				optLength;
	ICMPIOCompletionUPP	icmpCompletion;
	ulong					userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct LAPStatsPB{
	LAPStatsPtr	lapStatsPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct IPParamBlock {
	char 					fill12[12];
	ICMPIOCompletionUPP	ioCompletion;
	short 				ioResult;
	char 					*ioNamePtr;		
	short 				ioVRefNum;		
	short				ioCRefNum;			
	short 				csCode;
	union {
		IPEchoPB			IPEchoPB;
		LAPStatsPB		LAPStatsPB;
	} csParam;
	
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct icmpEchoInfo{
	ulong		echoRequestOut;	/* time in ticks of when the echo request went out */
	ulong		echoReplyIn;		/* time in ticks of when the reply was received */
	rdsEntry	echoedData;		/* data received in responce */
	Ptr		options;
	ulong		userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

// returned by a call to IPEchoICMP
#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct ICMPParamBlock {
	char 					fill12[12];
	ICMPIOCompletionUPP	ioCompletion;
	short 				ioResult;
	char 					*ioNamePtr;		
	short 				ioVRefNum;		
	short				ioCRefNum;			
	short 				csCode;
	short				params[11];
	ICMPEchoInfo			icmpEchoInfo;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#endif /* __H_IPPB__ */
