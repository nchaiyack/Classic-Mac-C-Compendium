/* 	DNR.c - DNR library for MPW

	(c) Copyright 1988 by Apple Computer.  All rights reserved
	
*/

#ifndef THINK_C
#include <OSUtils.h>
#include <Errors.h>
#include <Files.h>
#include <Resources.h>
#include <Memory.h>
#include <CursorCtl.h>
#endif

#include "MacTCPCommonTypes.h"
#include "AddressXLation.h"

#define OPENRESOLVER	1L
#define CLOSERESOLVER	2L
#define STRTOADDR		3L
#define	ADDRTOSTR		4L
#define	ENUMCACHE		5L
#define ADDRTONAME		6L

Handle codeHndl = 0L;

OSErrProcPtr dnr = 0L;

short OpenOurRF(void);


/* OpenOurRF is called to open the MacTCP driver resources */

short OpenOurRF(void)
{
	SysEnvRec info;
	HParamBlockRec fi;
	Str255 filename;

	SysEnvirons(1, &info);

	fi.fileParam.ioCompletion = 0L;
	fi.fileParam.ioNamePtr = filename;
	fi.fileParam.ioVRefNum = info.sysVRefNum;
	fi.fileParam.ioDirID = 0;
	fi.fileParam.ioFDirIndex = 1;
	
	while (PBHGetFInfo(&fi, false) == noErr) {
		/* scan system folder for driver resource files of specific type & creator */
		if (fi.fileParam.ioFlFndrInfo.fdType == 'cdev' &&
			fi.fileParam.ioFlFndrInfo.fdCreator == 'mtcp') {
			/* found the MacTCP driver file */
			SetVol(0,info.sysVRefNum);
			return(OpenResFile(filename)); // CHANGED!! was "&filename" DCA
			}
		/* check next file in system folder */
		fi.fileParam.ioFDirIndex++;
		fi.fileParam.ioDirID = 0;
		}
	return(-1);
	}	


OSErr OpenResolver(fileName)
char *fileName;
{
	short refnum;
	OSErr rc;
	
	if (dnr != 0L)
		/* resolver already loaded in */
		return(noErr);
		
	/* open the MacTCP driver to get DNR resources. Search for it based on
	   creator & type rather than simply file name */	
	refnum = OpenOurRF();

	/* ignore failures since the resource may have been installed in the 
	   System file if running on a Mac 512Ke */
	   
	/* load in the DNR resource package */
	codeHndl = GetIndResource('dnrp', 1);
	if (codeHndl == 0L) {
		/* can't open DNR */
		return(ResError());
		}
	
	DetachResource(codeHndl);
	if (refnum != -1) {
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
		dnr = 0L;
		}
	return(rc);
	}


OSErr CloseResolver(void)
{
	if (dnr == 0L)
		/* resolver not loaded error */
		return(notOpenErr);
		
	/* call close resolver */
	(void) (*dnr)(CLOSERESOLVER);

	/* release the DNR resource package */
	HUnlock(codeHndl);
	DisposHandle(codeHndl);
	dnr = 0L;
	return(noErr);
	}

OSErr StrToAddr(hostName, rtnStruct, resultproc, userDataPtr)
char *hostName;
struct hostInfo *rtnStruct;
ResultProcPtr resultproc;
char *userDataPtr;
{
	if (dnr == 0L)
		/* resolver not loaded error */
		return(notOpenErr);
	return((*dnr)(STRTOADDR, hostName, rtnStruct, resultproc, userDataPtr));
	}
	
OSErr AddrToStr(addr, addrStr)
unsigned long addr;
char *addrStr;									
{
	if (dnr == 0L)
		/* resolver not loaded error */
		return(notOpenErr);
		
	(*dnr)(ADDRTOSTR, addr, addrStr);
	return(noErr);
	}
	
OSErr EnumCache(resultproc, userDataPtr)
ResultProcPtr resultproc;
char *userDataPtr;
{
	if (dnr == 0L)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return((*dnr)(ENUMCACHE, resultproc, userDataPtr));
	}
	
	
OSErr AddrToName(addr, rtnStruct, resultproc, userDataPtr)
unsigned long addr;
struct hostInfo *rtnStruct;
ResultProcPtr resultproc;
char *userDataPtr;									
{
	if (dnr == 0L)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return((*dnr)(ADDRTONAME, addr, rtnStruct, resultproc, userDataPtr));
	}
	