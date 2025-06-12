/* 	DNR.c - DNR library for MPW

	(c) Copyright 1988 by Apple Computer.  All rights reserved
    Further modifications by Steve Falkenburg, Apple MacDTS 8/91
	
*/

#include <Types.h>
#include <OSUtils.h>
#include <Errors.h>
#include <Files.h>
#include <Resources.h>
#include <Memory.h>
#include <Traps.h>
#include <GestaltEqu.h>
#include <Folders.h>
#include <ToolUtils.h>
#include "AddressXlation.h"
#include <Aliases.h>


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

typedef OSErr (*OSErrProcPtr)(long,...);    /* added proto for Think C compatibility -->SJF<-- */
OSErrProcPtr dnr = nil;                     /* ... otherwise, first arg would be passed as short*/


TrapType GetTrapType(theTrap)
unsigned long theTrap;
{
	if (BitAnd(theTrap, 0x0800) > 0)
		return(ToolTrap);
	else
		return(OSTrap);
	}
	
Boolean TrapAvailable(trap)
unsigned long trap;
{
TrapType trapType = ToolTrap;
unsigned long numToolBoxTraps;

	if (NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(0xAA6E, ToolTrap))
		numToolBoxTraps = 0x200;
	else
		numToolBoxTraps = 0x400;

	trapType = GetTrapType(trap);
	if (trapType == ToolTrap) {
		trap = BitAnd(trap, 0x07FF);
		if (trap >= numToolBoxTraps)
			trap = _Unimplemented;
		}
	return(NGetTrapAddress(trap, trapType) != NGetTrapAddress(_Unimplemented, ToolTrap));

}

short GetCPanelFolder()
{
	short vRefNum = 0;
	long dirID = 0;
	short wdRef;
	
	if (FindFolder(kOnSystemDisk, kControlPanelFolderType, kDontCreateFolder, &vRefNum, &dirID) != noErr) return(0);
	if (OpenWD(vRefNum, dirID, 'dnrp', &wdRef) == noErr)
		return(wdRef);
	else
		return(0);
	}

/* OpenOurRF is called to open the MacTCP driver resources */

short OpenOurRF()
{
	HParamBlockRec fi;
	Str255 filename;
	short vRefNum;
	long dirID;
	FSSpec spec;
	Boolean targetIsFolder, wasAliased;
	
	FindFolder(kOnSystemDisk, kControlPanelFolderType, kCreateFolder, &vRefNum, &dirID);
	fi.fileParam.ioFDirIndex = 1;
	fi.fileParam.ioCompletion = nil;
	fi.fileParam.ioNamePtr = &filename;
	fi.fileParam.ioVRefNum = vRefNum;
	fi.fileParam.ioDirID = dirID;
	
	while (PBHGetFInfoSync(&fi) == noErr) {
		/* scan system folder for driver resource files of specific type & creator */
		if (fi.fileParam.ioFlFndrInfo.fdType == 'cdev' &&
/* HACK BY CPS */

//			fi.fileParam.ioFlFndrInfo.fdCreator == 'ztcp') {


			(fi.fileParam.ioFlFndrInfo.fdCreator == 'ztcp' ||
			fi.fileParam.ioFlFndrInfo.fdCreator == 'mtcp')) {
			spec.vRefNum = fi.fileParam.ioVRefNum;
			spec.parID = dirID;
			BlockMove(&filename, &spec.name, 63);
			ResolveAliasFile(&spec, true, &targetIsFolder, &wasAliased);
			/* found the MacTCP driver file */
			return(FSpOpenResFile(&spec, fsRdPerm));
/* END HACK */
			}
		/* check next file in system folder */
		fi.fileParam.ioDirID = dirID; // trashed
		fi.fileParam.ioCompletion = nil;
		fi.fileParam.ioNamePtr = &filename;
		fi.fileParam.ioVRefNum = vRefNum;
		fi.fileParam.ioFDirIndex++;
		}

	FindFolder(kOnSystemDisk, kSystemFolderType, kCreateFolder, &vRefNum, &dirID);
	fi.fileParam.ioFDirIndex = 1;
	fi.fileParam.ioCompletion = nil;
	fi.fileParam.ioNamePtr = &filename;
	fi.fileParam.ioVRefNum = vRefNum;
	fi.fileParam.ioDirID = dirID;
	
	while (PBHGetFInfoSync(&fi) == noErr) {
		/* scan system folder for driver resource files of specific type & creator */
		if (fi.fileParam.ioFlFndrInfo.fdType == 'cdev' &&
/* HACK BY CPS */

//			fi.fileParam.ioFlFndrInfo.fdCreator == 'ztcp') {


			(fi.fileParam.ioFlFndrInfo.fdCreator == 'ztcp' ||
			fi.fileParam.ioFlFndrInfo.fdCreator == 'mtcp')) {
			spec.vRefNum = fi.fileParam.ioVRefNum;
			spec.parID = dirID;
			BlockMove(&filename, &spec.name, 63);
			ResolveAliasFile(&spec, true, &targetIsFolder, &wasAliased);
			/* found the MacTCP driver file */
			return(FSpOpenResFile(&spec, fsRdPerm));
/* END HACK */
			}
		/* check next file in system folder */
		fi.fileParam.ioDirID = dirID; // trashed
		fi.fileParam.ioCompletion = nil;
		fi.fileParam.ioNamePtr = &filename;
		fi.fileParam.ioVRefNum = vRefNum;
		fi.fileParam.ioFDirIndex++;
		}

	return(-1);
	}	


OSErr OpenResolver(fileName)
char *fileName;
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
//		CloseWD(refnum);
		CloseResFile(refnum);
		}
		
	/* lock the DNR resource since it cannot be reloated while opened */
	HLock(codeHndl);
	dnr = (OSErrProcPtr) *codeHndl;
	
	/* call open resolver */
	rc = (*dnr)(OPENRESOLVER, fileName);
	if (rc != noErr) {
		/* problem with open resolver, flush it */
		HUnlock(codeHndl);
		DisposHandle(codeHndl);
		dnr = nil;
		}
	return(rc);
	}


OSErr CloseResolver()
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	/* call close resolver */
	(void) (*dnr)(CLOSERESOLVER);

	/* release the DNR resource package */
	HUnlock(codeHndl);
	DisposHandle(codeHndl);
	dnr = nil;
	return(noErr);
	}

OSErr StrToAddr(hostName, rtnStruct, resultproc, userDataPtr)
char *hostName;
struct hostInfo *rtnStruct;
long resultproc;
char *userDataPtr;
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return((*dnr)(STRTOADDR, hostName, rtnStruct, resultproc, userDataPtr));
	}
	
OSErr AddrToStr(addr, addrStr)
unsigned long addr;
char *addrStr;									
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	(*dnr)(ADDRTOSTR, addr, addrStr);
	return(noErr);
	}
	
OSErr EnumCache(resultproc, userDataPtr)
long resultproc;
char *userDataPtr;
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return((*dnr)(ENUMCACHE, resultproc, userDataPtr));
	}
	
	
OSErr AddrToName(addr, rtnStruct, resultproc, userDataPtr)
unsigned long addr;
struct hostInfo *rtnStruct;
long resultproc;
char *userDataPtr;									
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return((*dnr)(ADDRTONAME, addr, rtnStruct, resultproc, userDataPtr));
	}


extern OSErr HInfo(hostName, returnRecPtr, resultProc, userDataPtr)
char *hostName;
struct returnRec *returnRecPtr;
long resultProc;
char *userDataPtr;
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return((*dnr)(HINFO, hostName, returnRecPtr, resultProc, userDataPtr));

	}
	
extern OSErr MXInfo(hostName, returnRecPtr, resultProc, userDataPtr)
char *hostName;
struct returnRec *returnRecPtr;
long resultProc;
char *userDataPtr;
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return((*dnr)(MXINFO, hostName, returnRecPtr, resultProc, userDataPtr));

	}
