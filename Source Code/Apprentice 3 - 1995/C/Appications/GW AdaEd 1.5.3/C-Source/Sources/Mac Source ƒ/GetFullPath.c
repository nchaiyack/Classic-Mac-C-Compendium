#include "GetFullPath.h"
#include "PStrUtilities.h"

/* get full path */
Boolean AdaGetFullPath(const FSSpecPtr fSpec, StringPtr fullPath)
{
CInfoPBRec pb;
CInfoPBRec	myPB;					/* parameter block for PBGetCatInfo */
Str255		dirName;				/* a directory name */
OSErr		myErr;
short		len;

#define isFolder(X) ((X.hFileInfo.ioFlAttrib & 0x10) ? true : false)
// another way of doing this macro is 	(pb.hFileInfo.ioFlAttrib & (1<<4))



	PCopyString(fSpec->name, fullPath);		/* initialize full pathname */

	pb.hFileInfo.ioFDirIndex = 0;			/* this HAS to be 0! */
	pb.hFileInfo.ioDirID = fSpec->parID;
	pb.hFileInfo.ioVRefNum = fSpec->vRefNum;
	pb.hFileInfo.ioNamePtr = fSpec->name;
	myErr = PBGetCatInfoSync(&pb);

	if (isFolder(pb))						/* file is a directory */
		PConcatStrings(fullPath, "\p:");

	/* What if we already have what we need...? */
	if (pb.dirInfo.ioDrDirID == fsRtDirID) {

		return true;		/* then we are done */
	}

	else {

		myPB.dirInfo.ioNamePtr = dirName;
		myPB.dirInfo.ioVRefNum = fSpec->vRefNum;	/* indicate target volume */
		myPB.dirInfo.ioDrParID = fSpec->parID;	/* initialize parent directory ID */
		myPB.dirInfo.ioFDirIndex = -1;			/* get info about a directory */
	
		/* Get name of each parent directory, up to root directory. */
		len = StrLength(fullPath);
	
		do {
	
			myPB.dirInfo.ioDrDirID = myPB.dirInfo.ioDrParID;
			myErr = PBGetCatInfoSync(&myPB);
			if (myErr == noErr) {
				PConcatStrings(dirName, "\p:");
				len += dirName[0];
				PConcatStrings(dirName, fullPath);
				PCopyString(dirName, fullPath);
			}
	
		} while  ((myErr == noErr) && (myPB.dirInfo.ioDrDirID != fsRtDirID));
	
		return len == StrLength(fullPath);
		/* True/False depending on whether the whole path was built or not */
	}
}


