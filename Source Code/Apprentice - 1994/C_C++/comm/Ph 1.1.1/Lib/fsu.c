/*______________________________________________________________________

	fsu.c - File System Utilities.
	
	Copyright © 1988-1991 Northwestern University.
	
	This module exports reusable file system utility routines.
	
	The primary purpose of this module is to permit the use of the new
	System 7 file system specification date type (FSSpec), even on 
	pre-System 7 systems.
	
	All of the code is placed in its own segment named "fsu", except for the
	fsu_Init routine, which is placed in segment fsui.
_____________________________________________________________________*/

#pragma load "precompile"
#include "fsu.h"
#include "utl.h"

#pragma segment fsu

/*______________________________________________________________________

	fsu_CheckRFSanity - Check a Resource File's Sanity.
	
	Entry:	fSpec = pointer to file spec.
	
	Exit:		*sane = true if resource fork is sane.
				function result = error code.
				
	This routine checks the sanity of a resource file.  The Resource Manager
	does not do error checking, and can bomb or hang if you use it to open
	a damaged resource file.  This routine can be called first to precheck
	the file.
	
	The routine checks the entire resource map.
_____________________________________________________________________*/

OSErr fsu_CheckRFSanity (FSSpecPtr fSpec, Boolean *sane)

{
	return fsu_HCheckRFSanity(fSpec->vRefNum, fSpec->parID, fSpec->name, 
		sane);
}

/*______________________________________________________________________

	fsu_FindFolder - Locate Finder Folder.
	
	Entry:	vRefNum = vol ref num.
				folderType = folder type (kPreferencesFolderType, etc.)
				createFolder = true to create folder if missing.
				
	Exit:		foundVRefNum = vol ref num of folder.
				foundDirID = dir id of folder.
				function result = error code.
				
	The vRefNum parameter is currently ignored - the boot disk is
	always used (kOnSystemDisk).
	
	For pre-7.0 systems the folderType and createFolder parameters are
	ignored.  The vol ref num of the boot disk and the directory id of
	the blessed folder are returned.
_____________________________________________________________________*/

OSErr fsu_FindFolder (short vRefNum, OSType folderType, Boolean createFolder,
	short *foundVRefNum, long *foundDirID)
	
{
#pragma unused (vRefNum)

	if (utl_GestaltFlag(gestaltFindFolderAttr, gestaltFindFolderPresent)) {
		return FindFolder(kOnSystemDisk, folderType, createFolder,
			foundVRefNum, foundDirID);
	} else {
		*foundVRefNum = utl_GetSysVol();
		*foundDirID = utl_GetSysDirID();
		return noErr;
	}
}

/*______________________________________________________________________

	fsu_FSpCreate - Create File.
	
	Entry:	spec = pointer to file system spec.
				creator = creator type.
				fileType = file type.
				
	Exit:		function result = error code.
	
	The poor man's search path is NOT used.
_____________________________________________________________________*/

OSErr fsu_FSpCreate (FSSpecPtr spec, OSType creator, OSType fileType)

{
	return fsu_HCreate(spec->vRefNum, spec->parID, spec->name, 
		creator, fileType);
}

/*______________________________________________________________________

	fsu_FSpCreateResFile - Create Resource File.
	
	Entry:	spec = pointer to file system spec.
				creator = creator type.
				fileType = file type.
				
	Exit:		function result = error code.
	
	The poor man's search path is NOT used.
_____________________________________________________________________*/

OSErr fsu_FSpCreateResFile (FSSpecPtr spec, OSType creator, OSType fileType)
	
{
	return fsu_HCreateResFile(spec->vRefNum, spec->parID, spec->name, 
		creator, fileType);
}

/*______________________________________________________________________

	fsu_FSpDelete - Delete a file.
	
	Entry:	spec = pointer to file system spec.
				
	Exit:		function result = error code.
_____________________________________________________________________*/

OSErr fsu_FSpDelete (FSSpecPtr spec)

{
	return fsu_HDelete(spec->vRefNum, spec->parID, spec->name);
}

/*______________________________________________________________________

	fsu_FSpGetFInfo - Get Finder Info.
	
	Entry:	spec = pointer to file system spec.
				
	Exit:		fndrInfo = Finder info.
				function result = error code.
_____________________________________________________________________*/

OSErr fsu_FSpGetFInfo (FSSpecPtr spec, FInfo *fndrInfo)

{
	return fsu_HGetFInfo(spec->vRefNum, spec->parID, spec->name, fndrInfo);
}	

/*______________________________________________________________________

	fsu_FSpOpenDF - Open Data Fork.
	
	Entry:	spec = pointer to file system spec.
				permission = permission.
				
	Exit:		refNum = opened file refnum.
				function result = error code.
	
	The poor man's search path is NOT used.
_____________________________________________________________________*/

OSErr fsu_FSpOpenDF (FSSpecPtr spec, char permission, short *refNum)

{
	return fsu_HOpenDF(spec->vRefNum, spec->parID, spec->name,
		permission, refNum);
}

/*______________________________________________________________________

	fsu_FSpOpenResFile - Open Resource File.
	
	Entry:	spec = pointer to file system spec.
				permission = permission.
				
	Exit:		refNum = opened file refnum.
				function result = error code.
				
	The poor man's search path is NOT used.
_____________________________________________________________________*/

OSErr fsu_FSpOpenResFile (FSSpecPtr spec, char permission, short *refNum)

{
	return fsu_HOpenResFile(spec->vRefNum, spec->parID, spec->name,
		permission, refNum);
}

/*______________________________________________________________________

	fsu_FSpOpenRF - Open Resource Fork.
	
	Entry:	spec = pointer to file system spec.
				permission = permission.
				
	Exit:		refNum = opened file refnum.
				function result = error code.
	
	The poor man's search path is NOT used.
_____________________________________________________________________*/

OSErr fsu_FSpOpenRF (FSSpecPtr spec, char permission, short *refNum)

{
	return fsu_HOpenRF(spec->vRefNum, spec->parID, spec->name,
		permission, refNum);
}

/*______________________________________________________________________

	fsu_FSpRstFLock - Unlock a file.
	
	Entry:	spec = pointer to file system spec.
				
	Exit:		function result = error code.
_____________________________________________________________________*/

OSErr fsu_FSpRstFLock (FSSpecPtr spec)

{
	return fsu_HRstFLock(spec->vRefNum, spec->parID, spec->name);
}	

/*______________________________________________________________________

	fsu_FSpSetFInfo - Set Finder Info.
	
	Entry:	spec = pointer to file system spec.
				fndrInfo = pointer to Finder info.
				
	Exit:		function result = error code.
_____________________________________________________________________*/

OSErr fsu_FSpSetFInfo (FSSpecPtr spec, FInfo *fndrInfo)

{
	return fsu_HSetFInfo(spec->vRefNum, spec->parID, spec->name, fndrInfo);
}	

/*______________________________________________________________________

	fsu_FSpSetFLock - Lock a file.
	
	Entry:	spec = pointer to file system spec.
				
	Exit:		function result = error code.
_____________________________________________________________________*/

OSErr fsu_FSpSetFLock (FSSpecPtr spec)

{
	return fsu_HSetFLock(spec->vRefNum, spec->parID, spec->name);
}	

/*______________________________________________________________________

	fsu_HCheckRFSanity - Check a Resource File's Sanity.
	
	Entry:	vRefNum = vol ref num.
				dirID = directory id.
				fileName = file name.
	
	Exit:		*sane = true if resource fork is sane.
				function result = error code.
				
	This routine checks the sanity of a resource file.  The Resource Manager
	does not do error checking, and can bomb or hang if you use it to open
	a damaged resource file.  This routine can be called first to precheck
	the file.
	
	The routine checks the entire resource map.
_____________________________________________________________________*/

OSErr fsu_HCheckRFSanity (short vRefNum, long dirID, Str255 fileName, 
	Boolean *sane)

{
	short					refNum;			/* file refnum */
	long					count;			/* number of bytes to read */
	long					logEOF;			/* logical EOF */
	unsigned char		*map;				/* pointer to resource map */
	unsigned long		dataLWA;			/* offset in file of data end */
	unsigned long		mapLWA;			/* offset in file of map end */
	unsigned short		typeFWA;			/* offset from map begin to type list */
	unsigned short		nameFWA;			/* offset from map begin to name list */
	unsigned char		*pType;			/* pointer into type list */
	unsigned char		*pName;			/* pointer to start of name list */
	unsigned char		*pMapEnd;		/* pointer to end of map */
	short					nType;			/* number of resource types in map */
	unsigned char		*pTypeEnd;		/* pointer to end of type list */
	short					nRes;				/* number of resources of given type */
	unsigned short		refFWA;			/* offset from type list to ref list */
	unsigned char		*pRef;			/* pointer into reference list */
	unsigned char		*pRefEnd;		/* pointer to end of reference list */
	unsigned short		resNameFWA;		/* offset from name list to resource name */
	unsigned char		*pResName;		/* pointer to resource name */
	unsigned long		resDataFWA;		/* offset from data begin to resource data */
	Boolean				mapOK;			/* true if map is sane */
	OSErr					rCode;			/* error code */
	
	struct {
		unsigned long		dataFWA;		/* offset in file of data */
		unsigned long		mapFWA;		/* offset in file of map */
		unsigned long		dataLen;		/* data area length */
		unsigned long		mapLen;		/* map area length */
	} header;
	
	/* Open the resource file. */
	
	if (rCode = fsu_HOpenRF(vRefNum, dirID, fileName, fsRdPerm, &refNum)) {
		if (rCode == fnfErr) {
			*sane = true;
			return noErr;
		} else {
			return rCode;
		}
	}
	
	/* Get the logical eof of the file. */
	
	if (rCode = GetEOF(refNum, &logEOF)) return rCode;
	if (!logEOF) {
		*sane = true;
		if (rCode = FSClose(refNum)) return rCode;
		return noErr;
	}
	
	/* Read and validate the resource header. */
	
	count = 16;
	if (rCode = FSRead(refNum, &count, (Ptr)&header)) {
		FSClose(refNum);
		return rCode;
	}
	dataLWA = header.dataFWA + header.dataLen;
	mapLWA = header.mapFWA + header.mapLen;
	mapOK = count == 16 && header.mapLen > 28 &&
		header.dataFWA < 0x01000000 && header.mapFWA < 0x01000000 &&
		dataLWA <= logEOF && mapLWA <= logEOF &&
		(dataLWA <= header.mapFWA || mapLWA <= header.dataFWA);
		
	/* Read the resource map. */
	
	map = nil;
	if (mapOK) {
		map = (unsigned char*)NewPtr(header.mapLen);
		if (!(rCode = SetFPos(refNum, fsFromStart, header.mapFWA))) {
			count = header.mapLen;
			rCode = FSRead(refNum, &count, map);
		}
	}
	
	/* Verify the type list and name list offsets. */
	
	if (!rCode) {
		typeFWA = *(unsigned short*)(map+24);
		nameFWA = *(unsigned short*)(map+26);
		mapOK = typeFWA == 28 && nameFWA >= typeFWA && nameFWA <= header.mapLen &&
			!(typeFWA & 1) && !(nameFWA & 1);
	}
	
	/* Verify the type list, reference lists, and name list. */
	
	if (mapOK) {
		pType = map + typeFWA;
		pName = map + nameFWA;
		pMapEnd = map + header.mapLen;
		nType = *(short*)pType + 1;
		pType += 2;
		pTypeEnd = pType + (nType<<3);
		if (mapOK = pTypeEnd <= pMapEnd) {
			while (pType < pTypeEnd) {
				nRes = *(short*)(pType+4) + 1;
				refFWA = *(unsigned short*)(pType+6);
				pRef = map + typeFWA + refFWA;
				pRefEnd = pRef + 12*nRes;
				if (!(mapOK = pRef >= pTypeEnd && pRef < pName && 
					!(refFWA & 1))) break;
				while (pRef < pRefEnd) {
					resNameFWA = *(unsigned short*)(pRef+2);
					if (resNameFWA != 0xFFFF) {
						pResName = pName + resNameFWA;
						if (!(mapOK = pResName + *pResName < pMapEnd)) break;
					}
					resDataFWA = *(unsigned long*)(pRef+4) & 0x00FFFFFF;
					if (!(mapOK = header.dataFWA + resDataFWA < dataLWA)) break;
					pRef += 12;
				}
				if (!mapOK) break;
				pType += 8;
			}
		}
	}
	
	/* Dispose of the resource map, close the file and return. */
	
	if (map) DisposPtr((Ptr)map);
	if (!rCode) {
		rCode = FSClose(refNum);
	} else {
		(void) FSClose(refNum);
	}
	*sane = mapOK;
	return rCode;
}

/*______________________________________________________________________

	fsu_HCreate - Create File.
	
	Entry:	vRefNum = vol ref num.
				dirID = directory id.
				fileName = file name.
				creator = creator type.
				fileType = file type.
				
	Exit:		function result = error code.
	
	The poor man's search path is NOT used.
_____________________________________________________________________*/

OSErr fsu_HCreate (short vRefNum, long dirID, Str255 fileName, 
	OSType creator, OSType fileType)

{
	OSErr					rCode;			/* result code */
	HParamBlockRec		pBlock;			/* param block */
	FInfo					fndrInfo;		/* Finder info */
	
	pBlock.fileParam.ioNamePtr = fileName;
	pBlock.fileParam.ioVRefNum = vRefNum;
	pBlock.fileParam.ioDirID = dirID;
	pBlock.fileParam.ioFVersNum = 0;
	if (rCode = PBHCreate(&pBlock, false)) return rCode;
	if (rCode = fsu_HGetFInfo(vRefNum, dirID, fileName, &fndrInfo)) return rCode;
	fndrInfo.fdCreator = creator;
	fndrInfo.fdType = fileType;
	return fsu_HSetFInfo(vRefNum, dirID, fileName, &fndrInfo);
}

/*______________________________________________________________________

	fsu_HCreateResFile - Create Resource File.
	
	Entry:	vRefNum = vol ref num.
				dirID = directory id.
				fileName = file name.
				creator = creator type.
				fileType = file type.
				
	Exit:		function result = error code.
	
	The poor man's search path is NOT used.
_____________________________________________________________________*/

OSErr fsu_HCreateResFile (short vRefNum, long dirID, Str255 fileName, 
	OSType creator, OSType fileType)
	
{
	OSErr					rCode;			/* result code */
	HParamBlockRec		pBlock;			/* PBHCreate param block */
	FInfo					fndrInfo;		/* Finder info */
	
	/* Note: the following call to PBHCreate is supposed to be
		unnecessary.  See TN 214.  But the 7.0b1 HCreateResFile trap
		doesn't work right - it returns -48 = dupFNErr if a resource
		file with the same name already exists in the system folder.
		See Outside Bug Report JLN8. */
	pBlock.fileParam.ioNamePtr = fileName;
	pBlock.fileParam.ioVRefNum = vRefNum;
	pBlock.fileParam.ioDirID = dirID;
	pBlock.fileParam.ioFVersNum = 0;
	PBHCreate(&pBlock, false);
	HCreateResFile(vRefNum, dirID, fileName);
	if (rCode = ResError()) return rCode;
	if (rCode = fsu_HGetFInfo(vRefNum, dirID, fileName, &fndrInfo)) return rCode;
	fndrInfo.fdCreator = creator;
	fndrInfo.fdType = fileType;
	return fsu_HSetFInfo(vRefNum, dirID, fileName, &fndrInfo);
}

/*______________________________________________________________________

	fsu_HDelete - Delete a file.
	
	Entry:	vRefNum = vol ref num.
				dirID = directory id.
				fileName = file name.
				
	Exit:		function result = error code.
_____________________________________________________________________*/

OSErr fsu_HDelete (short vRefNum, long dirID, Str255 fileName)

{
	HParamBlockRec		pBlock;		/* HFS param block */

	pBlock.fileParam.ioNamePtr = fileName;
	pBlock.fileParam.ioVRefNum = vRefNum;
	pBlock.fileParam.ioDirID = dirID;
	pBlock.fileParam.ioFVersNum = 0;
	return PBHDelete(&pBlock, false);
}

/*______________________________________________________________________

	fsu_HGetFInfo - Get Finder Info.
	
	Entry:	vRefNum = vol ref num.
				dirID = directory id.
				fileName = file name.
				
	Exit:		fndrInfo = Finder info.
				function result = error code.
_____________________________________________________________________*/

OSErr fsu_HGetFInfo (short vRefNum, long dirID, Str255 fileName, 
	FInfo *fndrInfo)

{
	HParamBlockRec		pBlock;		/* HFS param block */
	OSErr					rCode;		/* result code */

	pBlock.fileParam.ioNamePtr = fileName;
	pBlock.fileParam.ioVRefNum = vRefNum;
	pBlock.fileParam.ioFDirIndex = 0;
	pBlock.fileParam.ioDirID = dirID;
	pBlock.fileParam.ioFVersNum = 0;
	if (rCode = PBHGetFInfo(&pBlock, false)) return rCode;
	*fndrInfo = pBlock.fileParam.ioFlFndrInfo;
	return noErr;
}	

/*______________________________________________________________________

	fsu_HOpenDF - Open Data Fork.
	
	Entry:	vRefNum = vol ref num.
				dirID = directory id.
				fileName = file name.
				permission = permission.
				
	Exit:		refNum = opened file refnum.
				function result = error code.
	
	The poor man's search path is NOT used.
_____________________________________________________________________*/

OSErr fsu_HOpenDF (short vRefNum, long dirID, Str255 fileName, 
	char permission, short *refNum)

{
	OSErr					rCode;		/* result code */
	HParamBlockRec		pBlock;		/* HFS param block */
	
	pBlock.ioParam.ioNamePtr = fileName;
	pBlock.ioParam.ioVRefNum = vRefNum;
	pBlock.ioParam.ioPermssn = permission;
	pBlock.ioParam.ioMisc = nil;
	pBlock.fileParam.ioDirID = dirID;
	pBlock.fileParam.ioFVersNum = 0;
	if (rCode = PBHOpen(&pBlock, false)) return rCode;
	*refNum = pBlock.ioParam.ioRefNum;
	return noErr;
}

/*______________________________________________________________________

	fsu_HOpenResFile - Open Resource File.
	
	Entry:	vRefNum = vol ref num.
				dirID = directory id.
				fileName = file name.
				permission = permission.
				
	Exit:		refNum = opened file refnum.
				function result = error code.
				
	The poor man's search path is NOT used.
_____________________________________________________________________*/

OSErr fsu_HOpenResFile (short vRefNum, long dirID, Str255 fileName, 
	char permission, short *refNum)

{
	OSErr			rCode;			/* result code */
	FCBPBRec		pBlock;			/* PBGetFCBInfo param block */

	*refNum = HOpenResFile(vRefNum, dirID, fileName, permission);
	rCode = ResError();
	/* Note: the following shouldn't be necessary.  But the 7.0b1 HOpenResFile 
		trap doesn't work right - if a resource file with the same name already 
		exists in the system folder it is opened.  See Outside Bug Report 
		JLN8. */
	if (!rCode && !utl_VolIsMFS(vRefNum)) {
		pBlock.ioNamePtr = nil;
		pBlock.ioRefNum = *refNum;
		pBlock.ioFCBIndx = 0;
		rCode = PBGetFCBInfo(&pBlock, false);
		if (!rCode && pBlock.ioFCBParID != dirID) rCode = fnfErr;
		if (rCode) CloseResFile(*refNum);
	}
	return rCode;
}

/*______________________________________________________________________

	fsu_HOpenRF - Open Resource Fork.
	
	Entry:	vRefNum = vol ref num.
				dirID = directory id.
				fileName = file name.
				permission = permission.
				
	Exit:		refNum = opened file refnum.
				function result = error code.
	
	The poor man's search path is NOT used.
_____________________________________________________________________*/

OSErr fsu_HOpenRF (short vRefNum, long dirID, Str255 fileName, 
	char permission, short *refNum)

{
	OSErr					rCode;		/* result code */
	HParamBlockRec		pBlock;		/* HFS param block */
	
	pBlock.ioParam.ioNamePtr = fileName;
	pBlock.ioParam.ioVRefNum = vRefNum;
	pBlock.ioParam.ioPermssn = permission;
	pBlock.ioParam.ioMisc = nil;
	pBlock.fileParam.ioDirID = dirID;
	pBlock.ioParam.ioVersNum = 0;
	if (rCode = PBHOpenRF(&pBlock, false)) return rCode;
	*refNum = pBlock.ioParam.ioRefNum;
	return noErr;
}

/*______________________________________________________________________

	fsu_HRstFLock - Unlock a file.
	
	Entry:	vRefNum = vol ref num.
				dirID = directory id.
				fileName = file name.
				
	Exit:		function result = error code.
_____________________________________________________________________*/

OSErr fsu_HRstFLock (short vRefNum, long dirID, Str255 fileName)

{
	HParamBlockRec		pBlock;		/* HFS param block */

	pBlock.fileParam.ioNamePtr = fileName;
	pBlock.fileParam.ioVRefNum = vRefNum;
	pBlock.fileParam.ioDirID = dirID;
	pBlock.fileParam.ioFVersNum = 0;
	return PBHRstFLock(&pBlock, false);
}	

/*______________________________________________________________________

	fsu_HSetFInfo - Set Finder Info.
	
	Entry:	vRefNum = vol ref num.
				dirID = directory id.
				fileName = file name.
				fndrInfo = pointer to Finder info.
				
	Exit:		function result = error code.
_____________________________________________________________________*/

OSErr fsu_HSetFInfo (short vRefNum, long dirID, Str255 fileName, 
	FInfo *fndrInfo)

{
	HParamBlockRec		pBlock;		/* HFS param block */
	OSErr					rCode;		/* result code */

	pBlock.fileParam.ioNamePtr = fileName;
	pBlock.fileParam.ioVRefNum = vRefNum;
	pBlock.fileParam.ioFDirIndex = 0;
	pBlock.fileParam.ioDirID = dirID;
	pBlock.fileParam.ioFVersNum = 0;
	if (rCode = PBHGetFInfo(&pBlock, false)) return rCode;
	pBlock.fileParam.ioDirID = dirID;
	pBlock.fileParam.ioFlFndrInfo = *fndrInfo;
	pBlock.fileParam.ioFVersNum = 0;
	return PBHSetFInfo(&pBlock, false);
}	

/*______________________________________________________________________

	fsu_HSetFLock - Lock a file.
	
	Entry:	vRefNum = vol ref num.
				dirID = directory id.
				fileName = file name.
				
	Exit:		function result = error code.
_____________________________________________________________________*/

OSErr fsu_HSetFLock (short vRefNum, long dirID, Str255 fileName)

{
	HParamBlockRec		pBlock;		/* HFS param block */

	pBlock.fileParam.ioNamePtr = fileName;
	pBlock.fileParam.ioVRefNum = vRefNum;
	pBlock.fileParam.ioDirID = dirID;
	pBlock.fileParam.ioFVersNum = 0;
	return PBHSetFLock(&pBlock, false);
}	
