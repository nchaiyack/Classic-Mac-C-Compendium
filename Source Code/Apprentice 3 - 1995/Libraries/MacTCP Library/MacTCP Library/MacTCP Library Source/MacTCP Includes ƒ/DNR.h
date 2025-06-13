/*
	DNR.h
	
	Header file for DNR.c
	
	01/31/94 dn - Created (some portions © Apple Computer)
	06/12/95 dn - Modified to work correctly with the universal headers.
*/

#pragma once

#ifndef __H_DNR__
#define __H_DNR__

#include <AddressXlation.h>

// procedure code numbers for DNR code segment

#define OPENRESOLVER	1
#define CLOSERESOLVER	2
#define STRTOADDR		3
#define ADDRTOSTR		4
#define ENUMCACHE		5
#define ADDRTONAME	6
#define HINFO			7
#define MXINFO			8

#define kMaxResolverCalls	8			// maximum allowable active resolver queries

#define kHostFileCreatorType 'SPM ' // if the host file doesn't exist, one will be created with this creator type...

typedef OSErr (*OpenResolverProcPtr) (long selector,char* filename);
typedef OSErr (*CloseResolverProcPtr) (long selector);
typedef OSErr (*StrToAddrProcPtr)(long selector, char* hostName,HostInfoPtr rtnStruct,HostInfoUPP resultProc, char* userData);
typedef OSErr (*AddrToStrProcPtr)(long selector,ip_addr address, char* hostName);
typedef OSErr (*EnumCacheProcPtr)(long selector, EnumUPP result, char* userData);
typedef OSErr (*AddrToNameProcPtr)(long selector,ip_addr addr,HostInfoPtr rtnStruct,HostInfoUPP resultProc, char* userData);
typedef OSErr (*HInfoProcPtr)(long selector, char* hostName,ReturnPtr returnRecPtr,ReturnRecUPP resultProc, char* userData);
typedef OSErr (*MXInfoProcPtr)(long selector, char* hostName,ReturnPtr returnRecPtr,ReturnRecUPP resultProc, char* userData);

#if USESROUTINEDESCRIPTORS
	enum {
		uppOpenResolverProcInfo=kCStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(long)))
			| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(short)))	// short?
			| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(char *))),
		uppCloseResolverProcInfo=kCStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(long)))
			| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(short))),
		uppStrToAddrProcInfo = kCStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(long)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(char *)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(HostInfoPtr)))
			 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(HostInfoUPP)))
			 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(char *))),
		uppAddrToStrProcInfo = kCStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(long)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(char *))),
		uppEnumCacheProcInfo = kCStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(long)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(EnumUPP)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(char *))),
		uppAddrToNameProcInfo = kCStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(long)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ip_addr)))
			| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(HostInfoPtr)))
			| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(HostInfoUPP)))
			| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(char *))),
		uppHInfoProcInfo = kCStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(long)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(char *)))
			| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(ReturnPtr)))
			| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(ReturnRecUPP)))
			| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(char *))),
		uppMXInfoProcInfo = kCStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(long)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(char *)))
			| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(ReturnPtr)))
			| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(ReturnRecUPP)))
			| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(char *)))
	};
	
	typedef UniversalProcPtr OpenResolverUPP;
	typedef UniversalProcPtr CloseResolverUPP;
	typedef UniversalProcPtr StrToAddrUPP;
	typedef UniversalProcPtr AddrToStrUPP;
	typedef UniversalProcPtr EnumCacheUPP;
	typedef UniversalProcPtr AddrToNameUPP;
	typedef UniversalProcPtr HInfoUPP;
	typedef UniversalProcPtr MXInfoUPP;
	
	#define CallOpenResolverProc(routine,selector,filename)\
		CallUniversalProc((UniversalProcPtr)(routine),uppOpenResolverProcInfo,(selector),(filename))
	#define NewOpenResolverProc(routine)\
		(OpenResolverUPP)NewRoutineDescriptor((ProcPtr)(routine),uppOpenResolverProcInfo,GetCurrentISA())
		
	#define CallCloseResolverProc(routine,selector)\
		CallUniversalProc((UniversalProcPtr)(routine),uppCloseResolverProcInfo,(selector))
	#define NewCloseResolverProc(routine)\
		(CloseResolverUPP)NewRoutineDescriptor((ProcPtr)(routine),uppCloseResolverProcInfo,GetCurrentISA())
		
	#define NewStrToAddrProc(userRoutine)\
	        (StrToAddrUPP) NewRoutineDescriptor(userRoutine, uppStrToAddrProcInfo, GetCurrentISA())
	#define CallStrToAddrProc(userRoutine, selector, hostName, rtnStruct, resultProc, userData)\
	        CallUniversalProc(userRoutine, uppStrToAddrProcInfo, selector, hostName, rtnStruct, resultProc, userData)

	#define NewAddrToStrProc(userRoutine)\
	        (AddrToStrUPP) NewRoutineDescriptor(userRoutine, uppAddrToStrProcInfo, GetCurrentISA())
	#define CallAddrToStrProc(userRoutine, selector, address, hostName)\
	        CallUniversalProc(userRoutine, uppAddrToStrProcInfo, selector, address, hostName)

	#define NewEnumCacheProc(userRoutine)                       \
	        (EnumCacheUPP) NewRoutineDescriptor(userRoutine, uppEnumCacheProcInfo, GetCurrentISA())
	#define CallEnumCacheProc(userRoutine, selector, result, userData)  \
	        CallUniversalProc(userRoutine, uppEnumCacheProcInfo, selector, result, userData)

	#define NewAddrToNameProc(userRoutine)                      \
	        (AddrToNameUPP) NewRoutineDescriptor(userRoutine, uppAddrToNameProcInfo, GetCurrentISA())
	#define CallAddrToNameProc(userRoutine, selector, addr, rtnStruct, resultProc, userData)    \
	        CallUniversalProc(userRoutine, uppAddrToNameProcInfo, selector, addr, rtnStruct, resultProc, userData)

	#define NewHInfoProc(userRoutine)                       \
	        (HInfoUPP) NewRoutineDescriptor(userRoutine, uppHInfoProcInfo, GetCurrentISA())
	#define CallHInfoProc(userRoutine, selector, hostName, returnRecPtr, resultProc, userData)  \
	        CallUniversalProc(userRoutine, uppHInfoProcInfo, selector, hostName, returnRecPtr, resultProc, userData)

	#define NewMXInfoProc(userRoutine)                      \
	        (MXInfoUPP) NewRoutineDescriptor(userRoutine, uppMXInfoProcInfo, GetCurrentISA())
	#define CallMXInfoProc(userRoutine, selector, hostName, returnRecPtr, resultProc, userData) \
	        CallUniversalProc(userRoutine, selector, hostName, returnRecPtr, resultProc, userData)
#else
	typedef OpenResolverProcPtr OpenResolverUPP;
	typedef CloseResolverProcPtr CloseResolverUPP;
	typedef StrToAddrProcPtr StrToAddrUPP;
	typedef AddrToStrProcPtr AddrToStrUPP;
	typedef EnumCacheProcPtr EnumCacheUPP;
	typedef AddrToNameProcPtr AddrToNameUPP;
	typedef HInfoProcPtr HInfoUPP;
	typedef MXInfoProcPtr MXInfoUPP;
	
	#define CallOpenResolverProc(routine,selector,filename)\
		(*routine)((selector),(filename))
	#define NewOpenResolverProc(routine)\
		(OpenResolverUPP)(routine)

	#define CallCloseResolverProc(routine,selector)\
		(*routine)(selector)
	#define NewCloseResolverProc(routine)\
		(CloseResolverUPP)(routine)

	#define NewStrToAddrProc(userRoutine)\
	        (StrToAddrUPP)(userRoutine)
	#define CallStrToAddrProc(userRoutine, selector, hostName, rtnStruct, resultProc, userData)\
	        (*userRoutine)(selector, hostName, rtnStruct, resultProc, userData)

	#define NewAddrToStrProc(userRoutine)\
	        (AddrToStrUPP)(userRoutine)
	#define CallAddrToStrProc(userRoutine, selector, address, hostName)\
	        (*userRoutine)(selector, address, hostName)

	#define NewEnumCacheProc(userRoutine)                       \
	        (EnumCacheUPP)(userRoutine)
	#define CallEnumCacheProc(userRoutine, selector, result, userData)  \
	        (*userRoutine)(selector, result, userData)

	#define NewAddrToNameProc(userRoutine)                      \
	        (AddrToNameUPP)(userRoutine)
	#define CallAddrToNameProc(userRoutine, selector, addr, rtnStruct, resultProc, userData)    \
	        (*userRoutine)(selector, addr, rtnStruct, resultProc, userData)

	#define NewHInfoProc(userRoutine)                       \
	        (HInfoUPP)(userRoutine)
	#define CallHInfoProc(userRoutine, selector, hostName, returnRecPtr, resultProc, userData)  \
	        (*userRoutine)(selector, hostName, returnRecPtr, resultProc, userData)

	#define NewMXInfoProc(userRoutine)                      \
	        (MXInfoUPP)(userRoutine)
	#define CallMXInfoProc(userRoutine, selector, hostName, returnRecPtr, resultProc, userData) \
	        (*userRoutine)(selector, hostName, returnRecPtr, resultProc, userData)
#endif

// macros will expand correctly for either case
#define DisposeOpenResolverProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)(routine))
#define DisposeCloseResolverProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)(routine))
#define DisposeStrToAddrProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)(routine))
#define DisposeAddrToStrProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)(routine))
#define DisposeEnumCacheProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)(routine))
#define DisposeAddrToNameProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)(routine))
#define DisposeHInfoProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)(routine))
#define DisposeMXInfoProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)(routine))


extern Handle				gMacDNRcode;	// the DNR code resource’s handle
extern UniversalProcPtr		gMacDNRentry;	// the DNR code entry point

// error codes
enum {
	dnrBaseErr=23050,					// base error number for this section.
	dnrNoResolver						// The resolver is not open.
	
};

#ifdef __cplusplus
extern "C" {
#endif

// opening/closing the resolver
OSErr OpenResolver(StringPtr fileName);
OSErr CloseResolver(void);

// resolver routines
OSErr StrToAddr(char* hostName,HostInfoPtr host,HostInfoUPP hiproc,char* userDataPtr);
OSErr AddrToStr(ip_addr addr,char* addrStr);
OSErr EnumCache(EnumUPP eproc,char* userDataPtr);
OSErr AddrToName(ip_addr addr,HostInfoPtr host,HostInfoUPP hiproc,char* userDataPtr);
OSErr HInfo(char* hostName,ReturnPtr ret,ReturnRecUPP rproc,char* userDataPtr);
OSErr MXInfo(char* hostName,ReturnPtr ret,ReturnRecUPP rproc,char* userDataPtr);

// internal routines.
OSErr SearchFolderForDNRP(OSType targetType,OSType targetCreator,short vRefNum, long dirID);
OSErr OpenTheDNR(void);

#ifdef __cplusplus
}
#endif

#endif /* __H_DNR__ */
