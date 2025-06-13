/* FSUtilities.c */
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */

// we pass FSSpecs by pointer instead of by value and
// use the Apple MoreFiles library [Fabrizio Oddone]

/* Routines to traverse files in folder listing */

#ifndef __FSPCOMPAT__
#include <FSpCompat.h>
#endif
#ifndef __MOREFILESEXTRAS__
#include <MoreFilesExtras.h>
#endif
#include "FSUtilities.h"
#include "AdaFileTypes.h"


/****
 *	CountFilesInFolder()
 *
 *	Given an FSSpec for a folder, this routine will count
 *	how many files and folders are stored in the parent
 *	folder.
 *
 *	Returns 0 on errors.
 *
 ****/

// fixed so that File Manager calls are kept to a minimum [Fabrizio Oddone]

long	CountItemsInFolder(const FSSpecPtr folder)
{
	CInfoPBRec	pb;				/* local pb */
	HFileInfo	*fpb = (HFileInfo *)&pb;	/* to pointers */
	DirInfo	*dpb = (DirInfo *) &pb;
//	long		dirID;
//	long		count, idx;
	OSErr		err;
//	Str255		name;

	fpb->ioFDirIndex = 0;		// this HAS to be 0!
	fpb->ioDirID   = folder->parID;
	fpb->ioVRefNum = folder->vRefNum;
	fpb->ioNamePtr = folder->name;
	err = PBGetCatInfoSync(&pb);
/* we are now checking for errors [Fabrizio Oddone] */

/* the following stuff only slows things down */
/* there is no need to index the directory */
/*
	dirID = fpb->ioDirID;

	count = 0;
	fpb->ioNamePtr = name;
	for (idx = 1; err == noErr; idx++) {
		fpb->ioDirID = dirID;
		fpb->ioFDirIndex = idx;
		err = PBGetCatInfo(&pb, false);

		//if ((err == noErr) && !(fpb->ioFlAttrib & 16))
		if (err == noErr)
			count++;
	}
*/
	return err ? 0 : dpb->ioDrNmFls;
}

/****
 *	GetItemInFolder()
 *
 *	Given a folder and an index to a file into the folder,
 *	return an FSSpec for the file/folder in the folder.
 *
 ****/

void	GetItemInFolder(long index, const FSSpecPtr folder, FSSpecPtr file)
{	// set file.name to a valid empty Pascal String if error
CInfoPBRec	pb;
HFileInfo	*fpb = (HFileInfo *)&pb;	/* to pointers */
//DirInfo		*dpb = (DirInfo *) &pb;
OSErr		err;
Str255		name;

fpb->ioFDirIndex = 0;		// this HAS to be 0!
fpb->ioDirID   = folder->parID;
fpb->ioVRefNum = folder->vRefNum;
fpb->ioNamePtr = folder->name;
err = PBGetCatInfoSync(&pb);
// check for errors [Fabrizio Oddone]
if (err == noErr) {
	fpb->ioNamePtr = name;
	fpb->ioFDirIndex = index;
	err = PBGetCatInfoSync(&pb);
	if (err == noErr) {
		err = FSMakeFSSpecCompat(fpb->ioVRefNum, fpb->ioFlParID, fpb->ioNamePtr, file);
		}
	}
if (err)
	StrLength(file->name) = 0;
}

/****
 *	ItemIsFolder
 *
 *	Given an FSSpec, this routine returns true if the
 *	FSSpec is for a folder; false otherwise.
 *
 ****/

Boolean	ItemIsFolder(const FSSpecPtr file)
{
long	theDirID;
OSErr	err;
Boolean	isDirectory;

err = GetDirID(file->vRefNum, file->parID, file->name, &theDirID, &isDirectory);
return isDirectory && err == noErr;
}
