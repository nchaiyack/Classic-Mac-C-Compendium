/* 	DNR.c - DNR library for MPW

	(c) Copyright 1988 by Apple Computer.  All rights reserved
	
*/

#ifdef MPW
#pragma segment DNR
#endif

# include <Folders.h>			/* JMB */
# include <OSUtils.h>
# include <Errors.h>
# include <Files.h>
# include <Resources.h>
# include <Memory.h>

#define OPENRESOLVER	1
#define CLOSERESOLVER	2
#define STRTOADDR		3
#define	ADDRTOSTR		4
#define	ENUMCACHE		5
#define ADDRTONAME		6

Handle codeHndl = nil;

#ifdef MPW
typedef OSErr (*OSErrProcPtr)();
#else
typedef OSErr (*OSErrProcPtr)(...);
#endif
OSErrProcPtr dnr = nil;

#include "dnr.proto.h"

short AppleOpenOurRF(short VRefNum, long DirID);

/*	I've front end patched Apple's OpenOurRF.  This version looks in the Control Panels
	folder, then the system folder, and finally in the extensions folder for MacTCP.
	Newer versions of MacTCP creaty dummy files in the system folder to avoid this 
	problem.  However, I'd like to catch the case of the older MacTCP versions to avoid
	all calls to our tech support about the "Unable to Open Resolver" message.  The 
	performance hit is minimal. - JMB */

/* OpenOurRF is called to open the MacTCP driver resources */

short OpenOurRF(void)
{
	short VRefNum, retval;
	long DirID;
	
	FindFolder( kOnSystemDisk, kControlPanelFolderType, kCreateFolder, &VRefNum, &DirID);
	retval = AppleOpenOurRF(VRefNum, DirID);
	
	if (retval == -1)
		{
		FindFolder( kOnSystemDisk, kSystemFolderType, kCreateFolder, &VRefNum, &DirID);
		retval = AppleOpenOurRF(VRefNum, DirID);
		}
	
	if (retval == -1)
		{
		FindFolder( kOnSystemDisk, kExtensionFolderType, kCreateFolder, &VRefNum, &DirID);
		retval = AppleOpenOurRF(VRefNum, DirID);
		}
		
	return(retval);
}

short AppleOpenOurRF(short VRefNum, long DirID)
{
	HParamBlockRec fi;
	Str255 filename;
	short curVRefNum;
	WDPBRec pb;
	short status;

/*	SysEnvirons(1, &info); */

	fi.fileParam.ioCompletion = nil;
	fi.fileParam.ioNamePtr = (unsigned char *) &filename;	/* BYU LSC */
	fi.fileParam.ioVRefNum = VRefNum;
	fi.fileParam.ioDirID = DirID;
	fi.fileParam.ioFDirIndex = 1;

	
	while (PBHGetFInfo(&fi, false) == noErr) {
		/* scan system folder for driver resource files of specific type & creator */
		if (fi.fileParam.ioFlFndrInfo.fdType == 'cdev' &&
			(fi.fileParam.ioFlFndrInfo.fdCreator == 'mtcp' ||
			fi.fileParam.ioFlFndrInfo.fdCreator == 'ztcp')) {
			/* found the MacTCP driver file */
	
			/* save current volume (or working directory) */
			pb.ioNamePtr = 0L;
			if (PBHGetVol(&pb,false) != noErr)
				return(-1);
			curVRefNum = pb.ioVRefNum;
				
			/* move to system folder */
/*			if (SetVol(NULL,info.sysVRefNum) != noErr)
				return(-1);									*/
			status = HOpenResFile(VRefNum, DirID, filename, 0);
			/* back to orignal volume */
			SetVol(NULL,curVRefNum);
			
			return(status);
			}
		/* check next file in system folder */
		fi.fileParam.ioFDirIndex++;
		fi.fileParam.ioDirID = DirID;
		}
	return(-1);
	}	


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
		CloseResFile(refnum);
		}
		
	/* lock the DNR resource since it cannot be reloated while opened */
	HLockHi(codeHndl);								// JMB made HLock -> HLockHi
	dnr = (OSErrProcPtr) *codeHndl;
	
	/* call open resolver */
	rc = (*dnr)((long) OPENRESOLVER, fileName);		/* BYU LSC - needed (long) */
	if (rc != noErr) {
		/* problem with open resolver, flush it */
		HUnlock(codeHndl);
		DisposHandle(codeHndl);
		dnr = nil;
		}
	return(rc);
	}


OSErr CloseResolver(void)
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	/* call close resolver */
	(void) (*dnr)((long) CLOSERESOLVER);	/* BYU LSC - needed (long) */

	/* release the DNR resource package */
	HUnlock(codeHndl);
	DisposHandle(codeHndl);
	dnr = nil;
	return(noErr);
	}

OSErr StrToAddr(char *hostName, struct hostInfo *rtnStruct, long resultproc, char *userDataPtr)
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return((*dnr)((long) STRTOADDR, hostName, rtnStruct, resultproc, userDataPtr));		/* BYU LSC - needed (long) */
	}
	
OSErr AddrToStr(unsigned long addr, char *addrStr)
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	(*dnr)((long) ADDRTOSTR, addr, addrStr);	/* BYU LSC - needed (long) */
	return(noErr);
	}
	
OSErr EnumCache(long resultproc, char *userDataPtr)
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return((*dnr)((long) ENUMCACHE, resultproc, userDataPtr));	/* BYU LSC - needed (long) */
	}
	
	
OSErr AddrToName(unsigned long addr, struct hostInfo *rtnStruct, long resultproc, char *userDataPtr)
{
	if (dnr == nil)
		/* resolver not loaded error */
		return(notOpenErr);
		
	return((*dnr)((long) ADDRTONAME, addr, rtnStruct, resultproc, userDataPtr));	/* BYU LSC - needed (long) */
	}
	