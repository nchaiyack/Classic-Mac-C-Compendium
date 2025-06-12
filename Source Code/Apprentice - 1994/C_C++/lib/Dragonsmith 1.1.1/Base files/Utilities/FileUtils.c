/*
	FileUtils.c
	
	Created	05 May 1992	Extracted from Dragon.c
	Modified	28 May 1992	Changed parameters of FREFTypes
						Other very minor changes
			29 May 1992	Added RefNumToFSSpec function — I finally figured out how to get an FSSpec from a path refNum
			02 Aug 1992	Added FSpToPB, FSpToPBCatInfo, PBToFSp, and PBToFSpCatInfo — but they haven't been tested!
			05 Aug 1992	Extensive rewrites and some testing of the FSSpec <=> PBRecUnion conversion functions — what I
							tested seemed to work just fine, but then I didn't test directories…
			06 Aug 1992	Tuned up conversion functions
			09 Aug 1992	FREFTypes now takes a parameter — the refnum of the resource fork to look in
						Added FSpFindFolder
			16 Aug 1992	Fixed problems with ioDirID and ioFDirIndex in PBToFSpCatInfo
			04 Sep 1992	Added FSpRefreshFinderDisplay (and it works!)
			09 Sep 1992	Fixed bug in FREFTypes — it returned garbage if there were no 'FREF' resources
			11 Sep 1992	Extracted PBToFSp, FSpType, and FSpOpenableType — Dragonsmith doesn't need them
			18 Apr 1993	Rewrote PBToFSpCatInfo to overcome bug (?) in PBMakeFSSpec with directories on shared volumes
							— thanks to Paul van Mulbregt for pointing out the problem (which I should have spotted myself!)
			30 May 1993	Fixed FREFTypes — it was DisposHandle'ing a resource (oops)
			08 Aug 1993	Brought in DirectoryContentsLoop from its source file
						Added some safety measures to paramblock initializations
			14 Aug 1993	Fiddled with code to get it to compile with type-checked <MacHeaders>
			
	Copyright © 1992–1993 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (except as expressly permitted by the author)
		2.	This statement and the above copyright notice are left intact.

*/

#include	"FileUtils.h"
#include	"StringUtils.h"

void FSpRefreshFinderDisplay (FSSpec *fss)
{
	// Given an FSSpec to a file, tell the Finder to update its display of the file's icon — also works for directories
	// There will be a brief delay before the change is shown.  Experience indicates that the delay is noticeably longer
	//	if the only change is in the case (upper vs. lower) of the file's name
	
	CInfoPBRec		pb;
	OSErr			err;
	unsigned long		secs;
	
	// Fill in the parameter block so we get info on the parent directory rather than the file/folder itself
	pb.hFileInfo.ioCompletion = NULL;
	pb.hFileInfo.ioNamePtr = NULL;
	pb.hFileInfo.ioVRefNum = fss->vRefNum;
	pb.hFileInfo.ioFVersNum = 0;				// To be on the safe side
	pb.hFileInfo.ioFDirIndex = 0;
	pb.hFileInfo.ioDirID = fss->parID;
	
	// Get info on the parent directory
	if (PBGetCatInfoSync (&pb) == noErr) {
		// Now change the parent directory's modified date
		GetDateTime (&secs);
		pb.dirInfo.ioDrMdDat = (unsigned long) secs;
		(void) PBSetCatInfoSync (&pb);
	}
}

OSErr FSpOpenDataFork (FSSpec *fss, short *refNum, char *perm)
{
	// Open a file's data fork with fsRdPerm or (if possible) fsRdWrPerm
	
	OSErr	err;
	char		permWanted = *perm;
	
	err = FSpOpenDF (fss, *perm, refNum);
	if (err == noErr)
		return err;
	if ((err == opWrErr || err == permErr) && *perm == fsRdWrPerm) {
		err = FSpOpenDF (fss, fsRdPerm, refNum);
		*perm = fsRdPerm;
	}
	return err;
}

OSErr FSpOpenResFork (FSSpec *fss, short *refNum, char *perm)
{
	// Open a file's resource fork with fsRdPerm or (if possible) fsRdWrPerm
	
	OSErr	err;
	char		permWanted = *perm;
	
	*refNum = FSpOpenResFile (fss, *perm);
	err = ResError ();
	if (err == noErr)
		return err;
	if ((err == opWrErr || err == permErr) && *perm == fsRdWrPerm) {
		*refNum = FSpOpenResFile (fss, fsRdPerm);
		err = ResError ();
		*perm = fsRdPerm;
	}
	return err;
}

OSErr FSpFindFolder (OSType folderType, FSSpec *fss)
{
	// NOTE:	This function does NOT return an FSSpec to the folder in question; rather, it returns an FSSpec to an unnamed
	//		file IN THAT FOLDER.  In other words, just fill in the name field of the FSSpec returned to get at a file in the folder
	// If an error happens, *fss is not affected
	
	short	foundVRefNum;
	long		foundDirID;
	OSErr	err;
	
	err = FindFolder (kOnSystemDisk, folderType, kCreateFolder, &foundVRefNum, &foundDirID);
	if (err == noErr) {
		fss->vRefNum = foundVRefNum;
		fss->parID = foundDirID;
	}
	return err;
}

OSErr FSpToPB (PBRecUnion *pb, FSSpec *fss, Boolean resolveAFs, Boolean followAFChain, Boolean *wasAF)
{
	OSErr	err = noErr;
	Boolean	isFolder;
	
	// Check for a NULL name pointer — prob. the most common mistake I make when using PB's
	if (pb->h.fileParam.ioNamePtr == NULL)
		return bdNamErr;		// This seems like a reasonable error code to use…
		
	if (resolveAFs)
		err = ResolveAliasFile (fss, followAFChain, &isFolder, wasAF);
	if (err == noErr) {
		pb->h.fileParam.ioVRefNum = fss->vRefNum;
		pb->h.fileParam.ioDirID = fss->parID;
		pb->h.fileParam.ioFVersNum = 0;						// To be on the safe side
		SmartCopyPStr (fss->name, pb->h.fileParam.ioNamePtr);	// This ensures that .ioNamePtr points to a string exactly
														//	equivalent to fss->name
	}
	return err;
}

OSErr FSpToPBCatInfo (PBRecUnion *pb, FSSpec *fss, Boolean resolveAFs, Boolean followAFChain, Boolean *wasAF)
{
	OSErr	err = noErr;
	Boolean	isFolder;
	
	err = FSpToPB (pb, fss, resolveAFs, followAFChain, wasAF);
	if (err == noErr) {
		pb->c.hFileInfo.ioFDirIndex = 0;	// No indexing
		err = PBGetCatInfoSync (&pb->c);
	}
	return err;
}

OSErr PBToFSpCatInfo (PBRecUnion *pb, FSSpec *fss, Boolean resolveAFs, Boolean followAFChain, Boolean *wasAF)
{	
	// NOTE:	*wasAF will contain garbage if the error returned != noErr

	OSErr	err;
	long		dirIDOrFileNum, ioDirIDWas;
	Ptr		ioMiscWas;
	short	ioFDirIndexWas;
	Boolean	isFolder;
	
	// ASSERT — the ioNamePtr, ioVRefNum, ioFDirIndex, and ioDirID fields in *pb are valid
	
	// Check for a NULL name pointer — prob. the most common mistake I make when using PB's
	if (pb->h.fileParam.ioNamePtr == NULL)
		return bdNamErr;		// This seems like a reasonable error code to use…
		
	err = PBGetCatInfoSync (&pb->c);
	if (err == noErr) {

		// ASSERT —	Most of the fields in *pb are now valid (unless the doc is an alias file and resolveAFs == TRUE,
		//			in which case we've got the wrong file — calling ResolveAliasFile below will fix this, though, so don't worry!)
	
		// Now we're ready to make an FSSpec out of the paramblock
		
		// Calling PBMakeFSSpec here leads to problems on other shared volumes, so we do it by hand
		
		fss->parID = pb->c.hFileInfo.ioFlParID;
		
		// If resolveAFs == TRUE, then we need to check to see if this is an alias file — we can't
		//	eliminate any file types, since in the future there may exist aliases to things (like maybe
		//	mailboxes, in OCE) that didn't exist at the time this code was written.  We can, however,
		//	be sure that only files can be alias files (kinda obvious, ain't it?)
		
		*wasAF = FALSE;

		if (err == noErr && resolveAFs && PBIsAliasFile (pb)) {
			err = ResolveAliasFile (fss, followAFChain, &isFolder, wasAF);
			if (err == noErr && *wasAF) {
				// If this was an alias file, we need to call PBGetCatInfo once more for the actual file
				pb->c.hFileInfo.ioVRefNum = fss->vRefNum;
				pb->c.hFileInfo.ioDirID = fss->parID;
				
				ioFDirIndexWas = pb->c.hFileInfo.ioFDirIndex;	// Save the value in ioFDirIndex
				pb->c.hFileInfo.ioFDirIndex = 0;				// Zero ioFDirIndex so PBGetCatInfo won't use indexing
				
				// You don't need to copy fss->name to .ioNamePtr here — PBGetCatInfo will do it for us
				err = PBGetCatInfoSync (&pb->c);
				
				pb->c.hFileInfo.ioFDirIndex = ioFDirIndexWas;	// Restore ioFDirIndex
			}
		}
	}
	return err;
}

TypeListHndl FREFTypes (short resFork)
{
	short		numFREFs, frefNum, numTypes = 0, saveResFork;
	OSType		type, **fref;
	TypeListHndl	typesHndl = NULL;
	
	if (resFork == kInvalidRefNum)
		return NULL;
		
	saveResFork = CurResFile ();		// Save the current order of open resource forks
	UseResFile (resFork);			// We'll read 'FREF' resources just from this file, since there may be any number of
								//	open allication files and we don't want to end up with 'FREF's from all of them!
	numFREFs = Count1Resources ('FREF');
	if (numFREFs != 0) {
		typesHndl = (TypeListHndl) NewHandle (numFREFs * sizeof (OSType) + sizeof (short));
		if (typesHndl != NULL) {
			for (numTypes = 0, frefNum = 1; frefNum <= numFREFs; frefNum++) {
				fref = (OSType **) Get1IndResource ('FREF', frefNum);
				if (fref != NULL) {
					type = **fref;
					ReleaseResource ((Handle) fref);
					if (type != 'APPL')							// Don't count the 'APPL' type — nearly all
						(*typesHndl)->type[numTypes++] = type;	//	applications have an 'FREF' for this
				}
			}
			if (numTypes < numFREFs)
				SetHandleSize ((Handle) typesHndl, numTypes * sizeof (OSType) + sizeof (short));
		}
	}	
	if (typesHndl != NULL)
		(*typesHndl)->numTypes = numTypes;
	
	UseResFile (saveResFork);		// Restore the original order of open resource forks
	return typesHndl;
}

Boolean OpenableType (OSType fileType, TypeListHndl openableTypesHndl)
{
	short	i;
	OSType	t, *tp;
	
	if (openableTypesHndl == NULL)
		return FALSE;
		
	tp = &(*openableTypesHndl)->type[0];
	for (i = (*openableTypesHndl)->numTypes; i > 0; i--, tp++) {
		t = *tp;
		if (fileType == 'fold' || fileType == 'disk') {
			if (t == fileType)
				return TRUE;
		} else if (t == '****' || t == fileType)
			return TRUE;
	}
	return FALSE;		// If we got here, there's no match
}

OSErr RefNumToFSSpec (short refNum, FSSpec *fss, Boolean *isResFork)
{
	FCBPBRec	pb;
	OSErr		err;
	
	// See Inside Macintosh IV pp.178–180 for documentation of PBGetFCBInfo
	// NOTE:	There is an apparent typo there — at the bottom of page 180, it says:
	//
	//			"FCBMdRByt (which corresponds to ioFCBFlags in the parameter
	//			block for PBGetFCBInfo) contains flags that describe the status
	//			of the file, as follows:…"
	//
	//		But experience shows that these flags are returned in the HIGH BYTE
	//		of the ioFCBFlags field, not the whole word (in effect, in its low byte)
	//		
	//		Then again, maybe it's just a Pascal bit thing…
	
	pb.ioCompletion = NULL;
	pb.ioVRefNum = 0;			// 0 == check all volumes
	pb.ioNamePtr = fss->name;	// Yes, we need the name of the file — don't worry,
							//	HFSDispatch (in its guise of PBGetFCBInfo)
							//	doesn't move or purge memory (whew!)
	pb.ioRefNum = refNum;
	pb.ioFCBIndx = 0L;			// Get info on just this one file
	
	err = PBGetFCBInfoSync (&pb);	// NOTE:	For some strange reason, calling
								//		PBGetFCBInfo (&pb, FALSE) here
								//		instead gives us a major hang — I
								//		have a feeling this is the fault of
								//		THINK C's glue routines but I'm not
								//		going to bother finding out for sure…
	if (err != noErr)
		return err;
	
	// Fill in the FSSpec — note that fss->name has already been filled in
	fss->vRefNum = pb.ioFCBVRefNum;
	fss->parID = pb.ioFCBParID;	// Inside Macintosh IV implies that pb.ioVRefNum
							//	returns the volume refnum, but in fact the call
							//	to PBGetFCBInfo doesn't affect it.  This makes
							//	sense when you think about it — if it DID return
							//	the volume refnum in ioVRefNum, you'd have
							//	to reset it to 0 in each of a sequence of indexed
							//	calls (with ioFCBIndx going from 1 to n == the
							//	number of open paths)
	if (isResFork != NULL)
		*isResFork = (pb.ioFCBFlags & 0x0200);
		// 0x0200 == binary 00000010_00000000 — as I mentioned above, it's the
		//	HIGH byte that counts.  Bit 9 of the word == bit 1 of the high byte — Inside
		//	Macintosh IV (bottom of p.180) tells us "[bit] 1 … [is set] if the entry
		//	describes a resource fork"
	
}

OSErr DirectoryContentsLoop (short vRefNum, long dirID, DCLActionProc actionProc, Boolean skipOnError, long refCon)
{
	// Loop through the directory designated by vRefNum and dirID, calling the function actionProc with the
	//	parameter refcon for each thing in the directory.  This function does not recurse through subdirectories.
	
	CInfoPBRec	pb;
	short		i;
	OSErr		err = noErr;
	Str63		name;
	Boolean		keepGoing;
	
	pb.hFileInfo.ioCompletion = NULL;
	pb.hFileInfo.ioNamePtr = &name[0];
	
	for (i = 1, keepGoing = TRUE; keepGoing; i++) {
		pb.hFileInfo.ioVRefNum = vRefNum;
		pb.hFileInfo.ioDirID = dirID;
		pb.hFileInfo.ioFDirIndex = i;
		err = PBGetCatInfoSync (&pb);
		if (err == noErr)
			keepGoing = (*actionProc) (&pb, refCon);
		else if (err == fnfErr || !skipOnError)
			keepGoing = FALSE;
	}

	if (err == fnfErr)
		err = noErr;

	return err;
}

