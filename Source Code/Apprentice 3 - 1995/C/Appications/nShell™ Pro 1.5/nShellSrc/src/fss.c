/* ==========================================

	fss.c - FSSpec management
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

#include <aliases.h>

#include "multi.h"
#include "nsh.h"

#include "fss.proto.h"
#include "str_utl.proto.h"

extern	Boolean	gHasFSSpec;	// true if the calls are available

/* ========== Utilities. ========== */

OSErr fss_VRefNum(StringPtr pathname, short *realVRefNum)
{
	HParamBlockRec pb;
	Str255 tempPathname;
	OSErr error;

	pStrCopy(tempPathname,pathname);
	pb.volumeParam.ioNamePtr = (StringPtr)tempPathname;	/* use the copy so original isn't trashed */
	pb.volumeParam.ioVolIndex = -1;						/* use ioNamePtr/ioVRefNum combination */
	pb.volumeParam.ioVRefNum = 0;
	pb.volumeParam.ioCompletion = 0;

	error = PBHGetVInfoSync(&pb);
	*realVRefNum = pb.volumeParam.ioVRefNum;
	return (error);
}

OSErr fss_ParID(short vRefNum, StringPtr name, long *parID)
{
	CInfoPBRec pb;
	OSErr error;

	pb.hFileInfo.ioNamePtr = name;
	pb.hFileInfo.ioVRefNum = vRefNum;
	pb.hFileInfo.ioDirID = 0;
	pb.hFileInfo.ioFDirIndex = 0;				/* use ioNamePtr and ioDirID */
	error = PBGetCatInfoSync(&pb);
	*parID = pb.hFileInfo.ioFlParID;
	return (error);
}

OSErr fss_DirID(short vRefNum, StringPtr name, long *theDirID, Boolean *isDirectory)
{
	CInfoPBRec pb;
	OSErr error;

	pb.hFileInfo.ioNamePtr = name;
	pb.hFileInfo.ioVRefNum = vRefNum;
	pb.hFileInfo.ioDirID = 0;
	pb.hFileInfo.ioFDirIndex = 0;	/* use ioNamePtr and ioDirID */
	error = PBGetCatInfoSync(&pb);
	*theDirID = pb.hFileInfo.ioDirID;
	*isDirectory = (pb.hFileInfo.ioFlAttrib & 0x10) != 0;
	return (error);
}

OSErr fss_DirName(short vRefNum, long dirID, StringPtr name)
{
	CInfoPBRec pb;

	pb.hFileInfo.ioNamePtr = name;
	pb.hFileInfo.ioVRefNum = vRefNum;
	pb.hFileInfo.ioDirID = dirID;
	pb.hFileInfo.ioFDirIndex = -1;	/* get information about ioDirID */
	return (PBGetCatInfoSync(&pb));
}

OSErr fss_FilenameFromPathname(const Str255 pathname, Str255 filename)
{
	short	index;
	short	nameEnd;

	/* default to no filename */
	filename[0] = 0;

	/* check for no pathname */
	if (pathname == nil)
		return(notAFileErr);
	
	/* get string length */
	index = pathname[0];
	
	/* check for empty string */
	if (index == 0)
		return(notAFileErr);
	
	/* skip over last trailing colon (if any) */
	if (pathname[index] == ':')
		--index;

	/* save the end of the string */
	nameEnd = index;

	/* if pathname ends with multiple colons, then this pathname refers */
	/* to a directory, not a file */
	if (pathname[index] == ':')
		return (notAFileErr);
		
	
	/* parse backwards until we find a colon or hit the beginning of the pathname */
	while ((pathname[index] != ':') && (index != 0))
		--index;
	
	/* if we parsed to the beginning of the pathname and the pathname ended */
	/* with a colon, then pathname is a full pathname to a volume, not a file */
	if ((index == 0) && (pathname[pathname[0]]))
		return (notAFileErr);
	
	/* get the filename and return noErr */
	filename[0] = (char)(nameEnd - index);
	BlockMove(&pathname[index+1], &filename[1], nameEnd - index);
	return (noErr);
}

/* ========== version six work-arounds ========== */

OSErr fss_make_6( StringPtr pathname, FSSpec *spec)
{
	Boolean	isDir;
	long	theDirID;
	OSErr	error;
	Str255	parPathname;
	
	spec->vRefNum = 0;
	spec->parID = 0;
	spec->name[0] = 0;
	
	error = fss_VRefNum(pathname, &(spec->vRefNum));

	if (error == noErr) {
		error = fss_ParID(spec->vRefNum, pathname, &(spec->parID));
		if (error == noErr) {

			/* The object is present */			
			/* Find out if the object is a file or directory and */
			/* if object is a directory, get its dirID */
			error = fss_DirID(spec->vRefNum, pathname, &theDirID, &isDir);
			if (error == noErr) {
				if (isDir) {
					/* it's a directory, get its name */
					error = fss_DirName(spec->vRefNum, theDirID, spec->name);
					}
				else {
					/* it's a file - parse to get the file name */
					error = fss_FilenameFromPathname(pathname, spec->name);
					}
				}
			}
		else if (error == fnfErr)	{
			/* The object isn't present - see if its parent is present */
	
			/* parse to get the object name from end of pathname */
			error = fss_FilenameFromPathname(pathname, spec->name);
			/* if we can't get the object name from the end, we can't continue */
			if (error == noErr)
			{
				/* get a copy of the pathname */
				pStrCopy(parPathname,pathname);
				
				/* remove the object name */
				parPathname[0] -= spec->name[0];
				/* and the trailing colon (if any) */
				if (pathname[pathname[0]] == ':')
					--parPathname[0];
				
				/* OK, now get the parent's directory ID */
				error = fss_DirID(spec->vRefNum, parPathname, &(spec->parID), &isDir);
				
				isDir = false;	/* we don't know what the object is really going to be */
			}
			if (error != noErr)
				error = dirNFErr;	/* couldn't find parent directory */
			else
				error = fnfErr;		/* we found the parent, but not the file */
		}
	}
	
	/* if we get anything other than noErr or fnfErr, clear the results */
	if ((error != noErr) && (error != fnfErr))
	{
		spec->vRefNum = 0;
		spec->parID = 0;
		spec->name[0] = 0;
		isDir = false;
	}
	
	return (error);
}

/* ========== public access ========== */

OSErr fss_make( Str255 pathname, FSSpec *spec )
{
	int		myErr;
	Boolean	wasAlias;
	Boolean	wasFolder;
	
	if (gHasFSSpec) {
	
		myErr = FSMakeFSSpec(0, 0, pathname, spec);
		
		if (!myErr)
			myErr = ResolveAliasFile( spec, 1, &wasFolder, &wasAlias );
			
		}
	else
		myErr = fss_make_6(	pathname, spec );
	

	return( myErr );
}

