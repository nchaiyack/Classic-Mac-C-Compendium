/*  MACUTIL.C
*****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1993,									*
*	Board of Trustees of the University of Illinois				*
*****************************************************************
*	April, 1993	Entirely rewritten by Jim Browne to use HFS.
*/

#ifdef MPW
#pragma segment FTPServer
#endif

#include <stdio.h>
#include <string.h>

#include "TelnetHeader.h"		
#include "maclook.proto.h"
#include "telneterrors.h"
#include "DlogUtils.proto.h"
#include "macutil.proto.h"
#include "debug.h"

/* Some globals for file lookup */
char FileName[256];

//	Get the name of the directory identified by vRefNum & dirID
//	(works for volumes too... [dirID = 2])
void GetDirectoryName(short vRefNum, long dirID, Str32 name)
{
	CInfoPBRec	theInfo;
	
	WriteZero((Ptr)&theInfo, sizeof(CInfoPBRec));
	theInfo.dirInfo.ioVRefNum = vRefNum;
	theInfo.dirInfo.ioDrDirID = dirID;
	theInfo.dirInfo.ioNamePtr = name;
	theInfo.dirInfo.ioFDirIndex = -1;		// Only give me the Directory Info
	PBGetCatInfoSync(&theInfo);
}

//	Convert the given volume name to a reference number.  Returns the default vRefNum
//	if the volume specified is not found.
short VolumeNameToRefNum(Str32 volumeName)
{
	short	retval;
	
	if (SetVol(volumeName, 0) != noErr) return(-1);
	GetVol(NULL, &retval);
	
	return(retval);
}
	
void PathNameFromDirID(long dirID, short vRefNum, StringPtr fullPathName)
{
	CInfoPBRec	block;
	Str255		directoryName;
	OSErr		err;

	fullPathName[0] = '\0';

	block.dirInfo.ioDrParID = dirID;
	block.dirInfo.ioNamePtr = directoryName;
	do {
			block.dirInfo.ioVRefNum = vRefNum;
			block.dirInfo.ioFDirIndex = -1;
			block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
			if (err = PBGetCatInfoSync(&block)) return;
			pstrcat(directoryName, (StringPtr)"\p/");
			pstrinsert(fullPathName, directoryName);
	} while (block.dirInfo.ioDrDirID != 2);

	pstrinsert(fullPathName, "\p/");
}

//	ChangeDirectory will change the directory relative from the provided dirID and 
//		vRefNum.  It handles preceding ".." modifiers as well as absolute pathnames.
short ChangeDirectory(long *dirID, short *vRefNum,char *pathname)
{
	CInfoPBRec		theDirInfo;
	char		tempst[256], *nSlash, *start;
	short 		NumberOfLevelsToGoUp = 0, temp, localvRefNum;
	long		junk1, junk2, localdirID;
	OSErr		err;
	
	start = pathname;
	localvRefNum = *vRefNum;
	localdirID = *dirID;
	
	while ( (nSlash = strchr(pathname,'/') ) !=0L) {
		if (( (nSlash - pathname) == 2) && (*pathname=='.') && (*(pathname+1) =='.') ) {
			pathname += 3L;
			start = pathname;
			NumberOfLevelsToGoUp++;
			}
		else {
			*nSlash=':';
			pathname=nSlash + 1L;
			}
		}
	
	if ( strcmp("..",pathname)==0) {
		start[0]=0;
		NumberOfLevelsToGoUp++;
		}

	// Now start points to the beginning of a relative pathname devoid of ".."'s, and colons
	//		for seperators rather than slashes.
	
	if ( *start==':') {							// This is a fully qualified pathname
		if (NumberOfLevelsToGoUp) return(1);	// Someone did a "../..//blah"...
		nSlash = strchr(start+1L, ':');			// Seperate out volume name
		if (strlen(start) == 1)					// Someone did a "cd /"
			localdirID = 2;						//		Use top level directory
		else {
			if (nSlash != NULL) *nSlash = 0;
			strcpy(tempst, start+1);			// Remove leading colon
			strcat(tempst, ":");				// Add trailing colon
			CtoPstr(tempst);
			SetVol((StringPtr)tempst, 0);
			GetVol(NULL, &temp);
			GetWDInfo(temp, &localvRefNum, &junk1, &junk2);
			localdirID = 2;

			if (nSlash != NULL) {				// There's more than a volume name
				*nSlash = ':';					// Restore the leading slash
				strcpy( tempst, nSlash);
				CtoPstr(tempst);				// tempst now contains relative pathname
				
				WriteZero((Ptr)&theDirInfo, (long)sizeof(CInfoPBRec));
				theDirInfo.dirInfo.ioVRefNum = localvRefNum;		// Now find the directory on the volume
				theDirInfo.dirInfo.ioDrDirID = localdirID;
				theDirInfo.dirInfo.ioNamePtr = (StringPtr)tempst;
				theDirInfo.dirInfo.ioFDirIndex = 0;
				if (err = PBGetCatInfo(&theDirInfo, FALSE) != noErr) return(1);
				localdirID = theDirInfo.dirInfo.ioDrDirID;
				}		
			}
		}
	else
		{	// This is a relative pathname, start by taking care of any ".."s...
			WriteZero((Ptr)&theDirInfo, (long)sizeof(CInfoPBRec));
			while (NumberOfLevelsToGoUp && (localdirID != 2)) {
				theDirInfo.dirInfo.ioVRefNum = localvRefNum;
				theDirInfo.dirInfo.ioDrDirID = localdirID;
				theDirInfo.dirInfo.ioNamePtr = 0;
				theDirInfo.dirInfo.ioFDirIndex = -1;
				if (err = PBGetCatInfo(&theDirInfo, FALSE) != noErr) return(1);
				localdirID = theDirInfo.dirInfo.ioDrParID;
				NumberOfLevelsToGoUp--;
				}
		
			// Now use the relative pathname to find out the actual directory ID
			// Relative pathnames must begin with a colon, so put on on the beginning
			
			if(start[0]) {
				strcpy(tempst+1, start);
				tempst[0] = ':';
				CtoPstr(tempst);
				WriteZero((Ptr)&theDirInfo, (long)sizeof(CInfoPBRec));
				theDirInfo.dirInfo.ioVRefNum = localvRefNum;
				theDirInfo.dirInfo.ioDrDirID = localdirID;
				theDirInfo.dirInfo.ioNamePtr = (StringPtr)tempst;
				theDirInfo.dirInfo.ioFDirIndex = 0;
				if (err = PBGetCatInfo(&theDirInfo, FALSE) != noErr) return(1);
				localdirID = theDirInfo.dirInfo.ioDrDirID;
				}
		}
	
	*dirID = localdirID;		// Everything went ok, change the dirID and vRefNum
	*vRefNum = localvRefNum;		
	return(0);
}

short wccheck(char *file, char *template)	/* BYU - routine now returns (short) for recursion. */
{
	while(*template) {
		if (*template=='*') {
			template++;
			if (*template) {
				while((*file) && !wccheck(file,template)) file++;	/* BYU */
				if ((*file)==0) return(0);
				}
			else return(1);
			continue;
			}
		else
			if ((*template!='?') && (*template!=*file)) return(0);
		template++;file++;
		}
	if (*file)			/* BYU */
		return(0);		/* BYU */
	else				/* BYU */
		return(1);		/* BYU */
}

/* firstname
*  find the first name in the given directory which matches the wildcard
*  specification	*/
char *firstname(char *spec, long dirID, short vRefNum, CInfoPBRec *finfo)
{
	CInfoPBRec *localfinfo;
	
	localfinfo = (CInfoPBRec *) NewPtrClear(sizeof(CInfoPBRec));	
	
	finfo->hFileInfo.ioNamePtr=(StringPtr) FileName;
	finfo->hFileInfo.ioFDirIndex=1;
	finfo->hFileInfo.ioVRefNum = vRefNum;
	finfo->hFileInfo.ioDirID = dirID;
	
	BlockMove(finfo, localfinfo, sizeof(CInfoPBRec));
	if (PBGetCatInfo(localfinfo,FALSE)!=0) {
		DisposePtr((Ptr)localfinfo);
		return(0L);
		}

	FileName[FileName[0]+1]=0;
	
	while(!wccheck(&FileName[1],spec)) {
		finfo->hFileInfo.ioFDirIndex++;
		BlockMove(finfo, localfinfo, sizeof(CInfoPBRec));
		if (PBGetCatInfo(localfinfo,FALSE)!=0) {
			DisposePtr((Ptr)localfinfo);
			return(0L);
			}
		FileName[FileName[0]+1]=0;
		}

	if (localfinfo->hFileInfo.ioFlAttrib & 16)  {
		FileName[++FileName[0]]='/';
		FileName[FileName[0]+1]=0;
		}

	DisposePtr((Ptr)localfinfo);
	return(&FileName[1]);
}

/**********************************************************************/
/* nextname
*  modify the path spec to contain the next file name in the
*  sequence as given by DOS
*
*  if at the end of the sequence, return NULL
*/
char *nextname(char *spec, CInfoPBRec *finfo)
{
	CInfoPBRec *localfinfo;
	
	localfinfo = (CInfoPBRec *) NewPtrClear(sizeof(CInfoPBRec));	
	finfo->hFileInfo.ioFDirIndex++;

	BlockMove(finfo, localfinfo, sizeof(CInfoPBRec));
	if (PBGetCatInfo(localfinfo,FALSE)!=0) {
		DisposePtr((Ptr)localfinfo);
		return(0L);
		}

	FileName[FileName[0]+1]=0;
	while(!wccheck(&FileName[1], spec)) {
		putln(&FileName[1]);
		finfo->hFileInfo.ioFDirIndex++;
		BlockMove(finfo, localfinfo, sizeof(CInfoPBRec));
		if (PBGetCatInfo(localfinfo,FALSE)!=0) {
			DisposePtr((Ptr) localfinfo);
			return(0L);
			}
		FileName[FileName[0]+1]=0;
		}
	
	if (localfinfo->hFileInfo.ioFlAttrib & 16)  {
		FileName[++FileName[0]]='/';
		FileName[FileName[0]+1]=0;
		}

	DisposePtr((Ptr)localfinfo);
	return(&FileName[1]);
}