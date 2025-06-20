/*
	DNR.c - DNR library for MPW

	� Copyright 1988 by Apple Computer.  All rights reserved
	
*/

/*>>>>>>>>>>>>>>>>>>>>

	This file have been updated to PPC by modifications made
	by John Norstadt, the NewsWatcher application.
	(Hey, everybody steals from eatchother, and it is really
	Apples fault not to realease a PPC compatible version.
	
	It is not the same as NWs, as I have kept all my mods 
	that make this compile nicely with MPW C (such as
	ANSI function headers).
	
	I have added an ResolveAliasFile.

<<<<<<<<<<<<<<<<<<<<<<*/

//#pragma require_prototypes off

#define MPW3.0

#include <OSUtils.h>
#include <Errors.h>
#include <Files.h>
#include <Resources.h>
#include <Memory.h>
#include <Traps.h>
#include <GestaltEqu.h>
#include <Folders.h>
#include <MixedMode.h>
#include <Aliases.h>

#include "AddressXlation.h"

#pragma segment MyComm

#define OPENRESOLVER	1
#define CLOSERESOLVER	2
#define STRTOADDR		3
#define	ADDRTOSTR		4
#define	ENUMCACHE		5
#define ADDRTONAME		6
#define	HINFO			7
#define MXINFO			8

Handle codeHndl = nil;
UniversalProcPtr	dnr = nil;


TrapType DNR_GetTrapType(unsigned long theTrap)
{
	if (BitAnd(theTrap, 0x0800) > 0)
		return(ToolTrap);
	else
		return(OSTrap);
	}
	
Boolean DNR_TrapAvailable(unsigned long trap)
{
	TrapType trapType = ToolTrap;
	unsigned long numToolBoxTraps;

	if (NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(0xAA6E, ToolTrap))
		numToolBoxTraps = 0x200;
	else
		numToolBoxTraps = 0x400;

	trapType = DNR_GetTrapType(trap);
	if (trapType == ToolTrap) {
		trap = BitAnd(trap, 0x07FF);
		if (trap >= numToolBoxTraps)
			trap = _Unimplemented;
		}
	return(NGetTrapAddress(trap, trapType) != NGetTrapAddress(_Unimplemented, ToolTrap));

}

void GetSystemFolder(short *vRefNumP, long *dirIDP)
{
	SysEnvRec info;
	long wdProcID;
	
	SysEnvirons(1, &info);
	if (GetWDInfo(info.sysVRefNum, vRefNumP, dirIDP, &wdProcID) != noErr) {
		*vRefNumP = 0;
		*dirIDP = 0;
	}
}

void GetCPanelFolder(short *vRefNumP, long *dirIDP)
{
	Boolean hasFolderMgr = false;
	long feature;
	
	if (Gestalt(gestaltFindFolderAttr, &feature) == noErr) 
		hasFolderMgr = true;
	if (!hasFolderMgr) {
		GetSystemFolder(vRefNumP, dirIDP);
		return;
	}
	else {
		if (FindFolder(kOnSystemDisk, kControlPanelFolderType, kDontCreateFolder, vRefNumP, dirIDP) != noErr) {
			*vRefNumP = 0;
			*dirIDP = 0;
		}
	}
}

/* SearchFolderForDNRP is called to search a folder for files that might 
	contain the 'dnrp' resource */
short SearchFolderForDNRP(long targetType, long targetCreator, short vRefNum, long dirID)
{
	HParamBlockRec fi;
	Str255 filename;
	short refnum;
	FSSpec spec;
	Boolean targetIsFolder, wasAliased;
	
	fi.fileParam.ioCompletion = nil;
	fi.fileParam.ioNamePtr = filename;
	fi.fileParam.ioVRefNum = vRefNum;
	fi.fileParam.ioDirID = dirID;
	fi.fileParam.ioFDirIndex = 1;
	
	while (PBHGetFInfo(&fi, false) == noErr) {
		/* scan system folder for driver resource files of specific type & creator */
		if (fi.fileParam.ioFlFndrInfo.fdType == targetType &&
			fi.fileParam.ioFlFndrInfo.fdCreator == targetCreator) {
			/* found the MacTCP driver file? */

			spec.vRefNum = vRefNum;
			spec.parID = dirID;
			BlockMove(&filename, &spec.name, 64);
			ResolveAliasFile(&spec, true, &targetIsFolder, &wasAliased);
			
			refnum = HOpenResFile(spec.vRefNum, spec.parID, spec.name, fsRdPerm);
			if (GetIndResource('dnrp', 1) == NULL)
				CloseResFile(refnum);
			else
				return refnum;
			}
		/* check next file in system folder */
		fi.fileParam.ioFDirIndex++;
		fi.fileParam.ioDirID = dirID;	/* PBHGetFInfo() clobbers ioDirID */
		}
	return(-1);
	}	



/* OpenOurRF is called to open the MacTCP driver resources */

short OpenOurRF()
{
	short refnum;
	short vRefNum;
	long dirID;
	
	/* first search Control Panels for MacTCP 1.1 */
	GetCPanelFolder(&vRefNum, &dirID);
	refnum = SearchFolderForDNRP('cdev', 'ztcp', vRefNum, dirID);
	if (refnum != -1) return(refnum);
		
	/* next search System Folder for MacTCP 1.0.x */
	GetSystemFolder(&vRefNum, &dirID);
	refnum = SearchFolderForDNRP('cdev', 'mtcp', vRefNum, dirID);
	if (refnum != -1) return(refnum);
		
	/* finally, search Control Panels for MacTCP 1.0.x */
	GetCPanelFolder(&vRefNum, &dirID);
	refnum = SearchFolderForDNRP('cdev', 'mtcp', vRefNum, dirID);
	if (refnum != -1) return(refnum);
		
	return -1;
	}	




typedef OSErr (*OpenResolverProcPtr)(long selector, char* fileName);

enum {
	uppOpenResolverProcInfo = kCStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(char *)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr OpenResolverUPP;

#define	NewOpenResolverProc(userRoutine)						\
		(OpenResolverUPP) NewRoutineDescriptor(userRoutine, uppOpenResolverProcInfo, GetCurrentISA())
#define	CallOpenResolverProc(userRoutine, selector, filename)	\
		CallUniversalProc(userRoutine, uppOpenResolverProcInfo, selector, filename)
#else
typedef OpenResolverProcPtr OpenResolverUPP;

#define	NewOpenResolverProc(userRoutine)					\
		(OpenResolverUPP)(userRoutine)
#define	CallOpenResolverProc(userRoutine, selector, filename)	\
		(*(OpenResolverProcPtr)userRoutine)(selector, filename)
#endif



OSErr OpenResolver(char *fileName)
{
	short refnum;
	OSErr rc;
	
	if (dnr != nil)
		/* resolver already loaded in */
		return(noErr);
		
	/* open the MacTCP driver to get DNR resources. Search for it based on
	   creator & type rather than simply file name */	
	refnum = OpenOurRF();

	/* ignore failures since the resource may have been installed in the 
	   System file if running on a Mac 512Ke */
	   
	/* load in the DNR resource package */
	codeHndl = GetIndResource('dnrp', 1);
	if (codeHndl == nil) {
		/* can't open DNR */
		return(ResError());
	}
	
	DetachResource(codeHndl);
	if (refnum != -1) {
		CloseWD(refnum);
		CloseResFile(refnum);
	}
		
	/* lock the DNR resource since it cannot be reloated while opened */
	HLockHi(codeHndl);
	dnr = (UniversalProcPtr) *codeHndl;
	
	/* call open resolver */
	rc = CallOpenResolverProc(dnr, OPENRESOLVER, fileName);
	if (rc != noErr) {
		/* problem with open resolver, flush it */
		HUnlock(codeHndl);
		DisposeHandle(codeHndl);
		dnr = nil;
	}
	return(rc);
}



typedef OSErr (*CloseResolverProcPtr)(long selector);

enum {
	uppCloseResolverProcInfo = kCStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr CloseResolverUPP;

#define	NewCloseResolverProc(userRoutine)						\
		(CloseResolverUPP) NewRoutineDescriptor(userRoutine, uppCloseResolverProcInfo, GetCurrentISA())
#define	CallCloseResolverProc(userRoutine, selector)	\
		CallUniversalProc(userRoutine, uppCloseResolverProcInfo, selector)
#else
typedef CloseResolverProcPtr CloseResolverUPP;

#define	NewCloseResolverProc(userRoutine) 						\
		(CloseResolverUPP)(userRoutine)
#define	CallCloseResolverProc(userRoutine, selector)	\
		(*(CloseResolverProcPtr)userRoutine)(selector)
#endif



OSErr CloseResolver()
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	/* call close resolver */
	CallCloseResolverProc(dnr, CLOSERESOLVER);

	/* release the DNR resource package */
	HUnlock(codeHndl);
	DisposeHandle(codeHndl);
	dnr = nil;
	return(noErr);
}




typedef OSErr (*StrToAddrProcPtr)(long selector, char* hostName, struct hostInfo* rtnStruct,
									ResultUPP resultProc, char* userData);
									
enum {
	uppStrToAddrProcInfo = kCStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(char *)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(struct hostInfo *)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(char *)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr StrToAddrUPP;

#define	NewStrToAddrProc(userRoutine)						\
		(StrToAddrUPP) NewRoutineDescriptor(userRoutine, uppStrToAddrProcInfo, GetCurrentISA())
#define	CallStrToAddrProc(userRoutine, selector, hostName, rtnStruct, resultProc, userData)	\
		CallUniversalProc(userRoutine, uppStrToAddrProcInfo, selector, hostName, rtnStruct, resultProc, userData)
#else
typedef StrToAddrProcPtr StrToAddrUPP;

#define	NewStrToAddrProc(userRoutine)						\
		(StrToAddrUPP)(userRoutine)
#define	CallStrToAddrProc(userRoutine, selector, hostName, rtnStruct, resultProc, userData)	\
		(*(StrToAddrProcPtr)userRoutine)(selector, hostName, rtnStruct, resultProc, userData)
#endif



OSErr StrToAddr(char *hostName, struct hostInfo *rtnStruct, 
			ResultUPP resultProc, char *userDataPtr)
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return(CallStrToAddrProc(dnr, STRTOADDR, hostName, rtnStruct, resultProc, userDataPtr));
}
	

typedef OSErr (*AddrToStrProcPtr)(long selector, long address, char* hostName);

enum {
	uppAddrToStrProcInfo = kCStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(unsigned long)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(char *)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr AddrToStrUPP;

#define	NewAddrToStrProc(userRoutine)						\
		(AddrToStrUPP) NewRoutineDescriptor(userRoutine, uppAddrToStrProcInfo, GetCurrentISA())
#define	CallAddrToStrProc(userRoutine, selector, address, hostName)	\
		CallUniversalProc(userRoutine, uppAddrToStrProcInfo, selector, address, hostName)
#else
typedef AddrToStrProcPtr AddrToStrUPP;

#define	NewAddrToStrProc(userRoutine)						\
		(AddrToStrUPP)(userRoutine)
#define	CallAddrToStrProc(userRoutine, selector, address, hostName)	\
		(*(AddrToStrProcPtr)userRoutine)(selector, address, hostName)
#endif

OSErr AddrToStr(unsigned long addr, char *addrStr)
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	CallAddrToStrProc(dnr, ADDRTOSTR, addr, addrStr);
	return(noErr);
}
	
typedef OSErr (*EnumCacheProcPtr)(long selector, EnumResultUPP enumResultProc, char* userData);

enum {
	uppEnumCacheProcInfo = kCStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(char *)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr EnumCacheUPP;

#define	NewEnumCacheProc(userRoutine)						\
		(EnumCacheUPP) NewRoutineDescriptor(userRoutine, uppEnumCacheProcInfo, GetCurrentISA())
#define	CallEnumCacheProc(userRoutine, selector, result, userData)	\
		CallUniversalProc(userRoutine, uppEnumCacheProcInfo, selector, result, userData)
#else
typedef EnumCacheProcPtr EnumCacheUPP;

#define	NewEnumCacheProc(userRoutine)						\
		(EnumCacheUPP)(userRoutine)
#define	CallEnumCacheProc(userRoutine, selector, result, userData)	\
		(*(EnumCacheProcPtr)userRoutine)(selector, result, userData)
#endif



OSErr EnumCache(EnumResultUPP enumResultProc, char *userDataPtr)
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return(CallEnumCacheProc(dnr, ENUMCACHE, enumResultProc, userDataPtr));
}
	

typedef OSErr (*AddrToNameProcPtr)(long selector, unsigned long addr, struct hostInfo* rtnStruct,
									ResultUPP ResultProc, char* userData);

enum {
	uppAddrToNameProcInfo = kCStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(unsigned long)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(struct hostInfo *)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(char *)))

};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr AddrToNameUPP;

#define	NewAddrToNameProc(userRoutine)						\
		(AddrToNameUPP) NewRoutineDescriptor(userRoutine, uppAddrToNameProcInfo, GetCurrentISA())
#define	CallAddrToNameProc(userRoutine, selector, addr, rtnStruct, resultProc, userData)	\
		CallUniversalProc(userRoutine, uppAddrToNameProcInfo, selector, addr, rtnStruct, resultProc, userData)
#else
typedef AddrToNameProcPtr AddrToNameUPP;

#define	NewAddrToNameProc(userRoutine)						\
		(AddrToNameUPP)(userRoutine)
#define	CallAddrToNameProc(userRoutine, selector, addr, rtnStruct, resultProc, userData)	\
		(*(AddrToNameProcPtr)userRoutine)(selector, addr, rtnStruct, resultProc, userData)
#endif

	

OSErr AddrToName(unsigned long addr, struct hostInfo *rtnStruct, 
				ResultUPP resultProc, char *userDataPtr)
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return(CallAddrToNameProc(dnr, ADDRTONAME, addr, rtnStruct, resultProc, userDataPtr));
}


typedef OSErr (*HInfoProcPtr)(long selector, char* hostName, struct returnRec* returnRecPtr,
								ResultProc2Ptr resultProc, char* userData);

enum {
	uppHInfoProcInfo = kCStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(char *)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(struct returnRec *)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(char *)))

};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr HInfoUPP;

#define	NewHInfoProc(userRoutine)						\
		(HInfoUPP) NewRoutineDescriptor(userRoutine, uppHInfoProcInfo, GetCurrentISA())
#define	CallHInfoProc(userRoutine, selector, hostName, returnRecPtr, resultProc, userData)	\
		CallUniversalProc(userRoutine, uppHInfoProcInfo, selector, hostName, returnRecPtr, resultProc, userData)
#else
typedef HInfoProcPtr HInfoUPP;

#define	NewHInfoProc(userRoutine)						\
		(HInfoUPP)(userRoutine)
#define	CallHInfoProc(userRoutine, selector, hostName, returnRecPtr, resultProc, userData)	\
		(*(HInfoProcPtr)userRoutine)(selector, hostName, returnRecPtr, resultProc, userData)
#endif


extern OSErr HInfo(char *hostName, struct returnRec *returnRecPtr, 
		ResultProc2Ptr resultProc, char *userDataPtr)
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return(CallHInfoProc(dnr, HINFO, hostName, returnRecPtr, resultProc, userDataPtr));
}


typedef OSErr (*MXInfoProcPtr)(long selector, char* hostName, struct returnRec* returnRecPtr,
								ResultProc2Ptr resultProc, char* userData);

enum {
	uppMXInfoProcInfo = kCStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(char *)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(struct returnRec *)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(char *)))

};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr MXInfoUPP;

#define	NewMXInfoProc(userRoutine)						\
		(MXInfoUPP) NewRoutineDescriptor(userRoutine, uppMXInfoProcInfo, GetCurrentISA())
#define	CallMXInfoProc(userRoutine, selector, hostName, returnRecPtr, resultProc, userData)	\
		CallUniversalProc(userRoutine, selector, hostName, returnRecPtr, resultProc, userData)
#else
typedef MXInfoProcPtr MXInfoUPP;

#define	NewMXInfoProc(userRoutine)						\
		(MXInfoUPP)(userRoutine)
#define	CallMXInfoProc(userRoutine, selector, hostName, returnRecPtr, resultProc, userData)	\
		(*(MXInfoProcPtr)userRoutine)(selector, hostName, returnRecPtr, resultProc, userData)
#endif
	
	
extern OSErr MXInfo(char *hostName, struct returnRec *returnRecPtr, 
					ResultProc2Ptr resultProc, char *userDataPtr)
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return(CallMXInfoProc(dnr, MXINFO, hostName, returnRecPtr, resultProc, userDataPtr));
}
