/* 
	AddressXlation.h		
	MacTCP name to address translation routines.

	Copyright Apple Computer, Inc. 1988-91
	All rights reserved
	
	Modifications by Dave Nebinger (dnebing@andy.bgsu.edu), 01/30/94.
*/	

#pragma once

#ifndef __H_AddressXlation__
#define __H_AddressXlation__

#define NUM_ALT_ADDRS	4

enum AddrClasses {
	A = 1,
	NS,
	CNAME = 5,
	HINFO = 13,
	MX = 15,
	lastClass = 32767
};

typedef enum AddrClasses AddrClasses; 

typedef struct HostInfo HostInfo,* HostInfoPtr,** HostInfoHdl;
typedef struct HInfoRec HInfoRec,* HInfoPtr,** HInfoHdl;
typedef struct MXRec MXRec,* MXPtr,** MXHdl;
typedef struct ReturnRec ReturnRec,* ReturnPtr,** ReturnHdl;
typedef struct CacheEntryRec CacheEntryRec,* CacheEntryPtr,** CacheEntryHdl;

// macros for allocating new blocks...
#define NewHostInfoPtr() NEWPTR(HostInfoPtr,sizeof(HostInfo))
#define DisposeHostInfoPtr(a) DISPOSEPTR(a)
#define NewHostInfoHdl() NEWHDL(HostInfoHdl,sizeof(HostInfo))
#define DisposeHostInfoHdl(a) DISPOSEHDL(a)

#define NewHInfoPtr() NEWPTR(HInfoPtr,sizeof(HInfoRec))
#define DisposeHInfoPtr(a) DISPOSEPTR(a)
#define NewHInfoHdl() NEWHDL(HInfoHdl,sizeof(HInfoRec))
#define DisposeHInfoHdl(a) DISPOSEHDL(a)

#define NewMXPtr() NEWPTR(MXPtr,sizeof(MXRec))
#define DisposeMXPtr(a) DISPOSEPTR(a)
#define NewMXHdl() NEWHDL(MXHdl,sizeof(MXRec))
#define DisposeMXHdl(a) DISPOSEHDL(a)

#define NewReturnPtr() NEWPTR(ReturnPtr,sizeof(ReturnRec))
#define DisposeReturnPtr(a) DISPOSEPTR(a)
#define NewReturnHdl() NEWHDL(ReturnHdl,sizeof(ReturnRec))
#define DisposeReturnHdl(a) DISPOSEHDL(a)

#define NewCacheEntryPtr() NEWPTR(CacheEntryPtr,sizeof(CacheEntryRec))
#define DisposeCacheEntryPtr(a) DISPOSEPTR(a)
#define NewCacheEntryHdl() NEWHDL(CacheEntryHdl,sizeof(CacheEntryRec))
#define DisposeCacheEntryHdl(a) DISPOSEHDL(a)

typedef pascal void (*EnumProcPtr)(CacheEntryPtr cacheEntryRecordPtr, char* userDataPtr);
typedef pascal void (*HostInfoProcPtr)(HostInfoPtr hostInfoPtr, char* userDataPtr);
typedef pascal void (*ReturnRecProcPtr)(ReturnPtr returnRecPtr, char* userDataPtr);

#if USESROUTINEDESCRIPTORS
	enum {
		uppEnumProcInfo=kPascalStackBased
			| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(CacheEntryPtr)))
			| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(char*))),
		uppHostInfoProcInfo=kPascalStackBased
			| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(HostInfoPtr)))
			| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(char*))),
		uppReturnRecProcInfo=kPascalStackBased
			| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(ReturnPtr)))
			| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(char*)))
	};
	
	typedef UniversalProcPtr EnumUPP;
	typedef UniversalProcPtr HostInfoUPP;
	typedef UniversalProcPtr ReturnRecUPP;
	
	#define CallEnumProc(routine,cep,data)\
		CallUniversalProc((UniversalProcPtr)routine,uppEnumProcInfo,(cep),(data))
	#define NewEnumProc(routine)\
		(EnumUPP)NewRoutineDescriptor((ProcPtr)(routine),uppEnumProcInfo,GetCurrentISA())
	#define CallHostInfoProc(routine,cep,data)\
		CallUniversalProc((UniversalProcPtr)routine,uppHostInfoProcInfo,(cep),(data))
	#define NewHostInfoProc(routine)\
		(HostInfoUPP)NewRoutineDescriptor((ProcPtr)(routine),uppHostInfoProcInfo,GetCurrentISA())
	#define CallReturnRecProc(routine,cep,data)\
		CallUniversalProc((UniversalProcPtr)routine,uppReturnRecProcInfo,(cep),(data))
	#define NewReturnRecProc(routine)\
		(ReturnRecUPP)NewRoutineDescriptor((ProcPtr)(routine),uppReturnRecProcInfo,GetCurrentISA())
#else
	typedef EnumProcPtr EnumUPP;
	typedef HostInfoProcPtr HostInfoUPP;
	typedef ReturnRecProcPtr ReturnRecUPP;
	
	#define CallEnumProc(routine,cep,data)\
		(*(routine))((cep),(data))
	#define NewEnumProc(routine)\
		(EnumUPP)(routine)
	#define CallHostInfoProc(routine,cep,data)\
		(*(routine))((cep),(data))
	#define NewHostInfoProc(routine)\
		(HostInfoUPP)(routine)
	#define CallReturnRecProc(routine,cep,data)\
		(*(routine))((cep),(data))
	#define NewReturnRecProc(routine)\
		(ReturnRecUPP)(routine)
#endif

// this macro will expand correctly for either case
#define DisposeEnumProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)
#define DisposeHostInfoProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)
#define DisposeReturnRecProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct HostInfo {
	long			rtnCode;
	char			cname[255];
	ip_addr		addr[NUM_ALT_ADDRS];
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct HInfoRec {
	char			cpuType[30];
	char			osType[30];
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct MXRec {
	ushort		preference;
	char			exchange[255];
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif
	
#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct ReturnRec {
	long			rtnCode;
	char			cname[255];
	union {
		ip_addr	addr[NUM_ALT_ADDRS];
		HInfoRec	hinfo;
		MXRec	mx;
	} rdata;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct CacheEntryRecord {
	char*		cname;
	ushort		type;
	ushort		cclass;
	ulong			ttl;
	union {
		char*	name;
		ip_addr	addr;
	} rdata;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern OSErr OpenResolver(StringPtr fileName);
extern OSErr CloseResolver(void);

extern OSErr StrToAddr(char* hostName,HostInfoPtr host,HostInfoUPP hiproc,char* userDataPtr);
extern OSErr AddrToStr(ip_addr addr,char* addrStr);
extern OSErr EnumCache(EnumUPP eproc,char* userDataPtr);
extern OSErr AddrToName(ip_addr addr,HostInfoPtr host,HostInfoUPP hiproc,char* userDataPtr);
extern OSErr HInfo(char* hostName,ReturnPtr ret,ReturnRecUPP rproc,char* userDataPtr);
extern OSErr MXInfo(char* hostName,ReturnPtr ret,ReturnRecUPP rproc,char* userDataPtr);

#ifdef __cplusplus
}
#endif

#endif