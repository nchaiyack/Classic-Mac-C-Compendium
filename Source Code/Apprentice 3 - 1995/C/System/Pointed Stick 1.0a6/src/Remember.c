#include "Global.h"
#include "Remember.h"
#include "Conversions.h"		// SelectionToFSSpec
#include "GetItemList.h"		// gList
#include "PLStringFuncs.h"		// PStringCopy
#include "Utilities.h"			// ReportError

UniversalProcPtr	gOldMountVol;
UniversalProcPtr	gOldUnmountVol;

DynamicArrayHdl		gFileList;
DynamicArrayHdl		gDirectoryList;
DynamicArrayHdl		gVolumeList;
DynamicArrayHdl		gReboundList;
short				gSortType;
short				gMaxFileObjectsToRemember = 30;

static short		pMountOccured = true;
static short		pLastVRefNum;
static short		pLastIndex;

void				UpdateAllVRefNums(void);
void				UpdateVRefNum(VolumeEntryHdl volume);
short				GetVRefNumFromIndex(short index);
short				GetIndexFromVolName(Str255 name, unsigned long crDate);
short				GetIndexFromVRefNum(short vRefNum);

FileObjectHdl		SearchListForFileObject(DynamicArrayHdl array, FSSpecPtr);

//--------------------------------------------------------------------------------
/*
	Given an FSSpec, remember the file in the spec as well as its parent.
*/

void	RememberFileAndParent(FSSpecPtr filePtr)
{
	if (StrLength(filePtr->name) != 0)
		RememberFileObject(filePtr, gFileList);
	RememberParentDirectory(filePtr);
}

//--------------------------------------------------------------------------------
/*
	Given an FSSpec, remember the parent directory of the file in that spec.
	This function calls GetCatInfo to get information on the parent. If that
	works, it calls the common function RememberFileObject to remember that
	parent.
*/

void	RememberParentDirectory(FSSpecPtr filePtr)
{
	CInfoPBRec		pb;
	OSErr			err;
	FSSpec			newSpec;
	Str255			name;

	pb.dirInfo.ioNamePtr = name;
	pb.dirInfo.ioVRefNum = filePtr->vRefNum;
	pb.dirInfo.ioFDirIndex = -1;				// Get info on directory
	pb.dirInfo.ioDrDirID = filePtr->parID;
	err = PBGetCatInfoSync(&pb);

	if (err == noErr) {
		FSMakeFSSpec(pb.dirInfo.ioVRefNum,  pb.dirInfo.ioDrParID, name, &newSpec);
		RememberFileObject(&newSpec, gDirectoryList);
	} else {
		ReportError(kGetDirInfo, err);
	}
}


//--------------------------------------------------------------------------------
/*
*/

void	RememberSelectionForRebound(void)
{
	FileObjectHdl		reboundObject;
	FSSpec				selectedItem;
	
	SelectionToFSSpec(gList, &selectedItem);
	reboundObject = GetReboundItem(selectedItem.vRefNum, selectedItem.parID);
	if (reboundObject != NIL) {
		SetHandleSize((Handle) (**reboundObject).fName, StrLength(selectedItem.name) + 1);
		PStringCopy(*(**reboundObject).fName, selectedItem.name);
		ReadDateTime(&(**reboundObject).timeAdded);
	} else {
		RememberFileObject(&selectedItem, gReboundList);
	}
}


//--------------------------------------------------------------------------------

FileObjectHdl	GetReboundItem(short vRefNum, long parID)
{
	short			index;
	FileObjectHdl	entry;
	
	for (index = 0; index < (**gReboundList).numRecords; index++) {
		entry = (FileObjectHdl) (**gReboundList).records[index];
		if (GetVolRefNum(entry) == vRefNum && (**entry).parID == parID)
			return entry;
	}
	
	return NIL;
}


//--------------------------------------------------------------------------------
/*
	Remember a generic file object: either a folder or file (but not volumes).
	We are passed a pointer to the FSSpec holding the file object, and the
	handle to the list we are supposed to remember it in. If the object
	already exists in the list, we don�t do anything except update the �last
	touched� date. Otherwise, we add the file to the list, and kick off the
	oldest item in the list if we�ve already caught our limit.
*/

void	RememberFileObject(FSSpecPtr filePtr, DynamicArrayHdl list)
{
	CInfoPBRec		pb;
	OSErr			err;
	FileObjectHdl	entry;
	FileObjectPtr	entryPtr;
	Boolean			isFile;

	if (StrLength(filePtr->name) != 0) {

		pb.hFileInfo.ioNamePtr = filePtr->name;
		pb.hFileInfo.ioVRefNum = filePtr->vRefNum;
		pb.hFileInfo.ioFDirIndex = 0;
		pb.hFileInfo.ioDirID = filePtr->parID;
		err = PBGetCatInfoSync(&pb);

		if (err == noErr) {
			isFile = (pb.hFileInfo.ioFlAttrib & ioDirMask) == 0;
		} else {
			isFile = TRUE;		// punt
			pb.hFileInfo.ioFlFndrInfo.fdType = kUnknownSig;
		}

		entry = SearchListForFileObject(list, filePtr);
		if (entry == NIL) {

			entry = (FileObjectHdl) NewHandleSysClear(sizeof(FileObject));
			if (entry != NIL) {
				FSSpecToHandle(filePtr, entry);
				entryPtr = *entry;
				if (isFile) {
					entryPtr->u.type = pb.hFileInfo.ioFlFndrInfo.fdType;
					entryPtr->finderFlags = pb.hFileInfo.ioFlFndrInfo.fdFlags;
					entryPtr->openableByCurrentApp = FALSE;
				} else {
					entryPtr->u.dirID = pb.dirInfo.ioDrDirID;
					entryPtr->finderFlags = pb.dirInfo.ioDrUsrWds.frFlags;
					entryPtr->openableByCurrentApp = TRUE;
				}

				AppendRecord(list, (Handle) entry);

			}
		}

		if (entry != NIL) {
			ReadDateTime(&(**entry).timeAdded);
		}

		CullOldest(list);
		SortList(list);

	} else {
		ReportError(kAddingEmptyName, 0);
	}
}


//--------------------------------------------------------------------------------
/*
	Return the vRefNum for volume that the given file is on.
*/

short	GetVolRefNum(FileObjectHdl entry)
{
	if ((**entry).volIndex >= 0)
		return GetVRefNumFromIndex((**entry).volIndex);
	else
		return 0;
}


//--------------------------------------------------------------------------------
/*
	Given a vRefNum, remember the corresponding volume in our volumes list. If
	a volume with the same name, creation date, and vRefNum is already on the
	list, we do nothing. If a volume with the same name and creation date is
	already on the list, we update its vRefNum. Otherwise, we add the
	specified volume to the list as a new, distinct volume.
*/

short	RememberVolume(short vRefNum)
{
	WDPBRec			wdpb;
	ParamBlockRec	pb;
	Str255			volName;
	short			index;
	OSErr			err;

	//
	// Resolve the vRefNum in case it�s a working directory
	//
	wdpb.ioNamePtr = NIL;
	wdpb.ioVRefNum = vRefNum;
	wdpb.ioWDIndex = 0;
	wdpb.ioWDProcID = 0;
	if (PBGetWDInfoSync(&wdpb) == noErr) {
		vRefNum = wdpb.ioWDVRefNum;
	}

	//
	// See if a volume with this vRefNum is already
	// on the list. If not, add it.
	//
	index = GetIndexFromVRefNum(vRefNum);
	if (index < 0) {
		pb.volumeParam.ioNamePtr = volName;
		pb.volumeParam.ioVRefNum = vRefNum;
		pb.volumeParam.ioVolIndex = 0;
		err = PBGetVInfoSync(&pb);
		if (err == noErr) {
			index = AddVolume(volName, pb.volumeParam.ioVCrDate);
			if (index < 0)
				DebugStr("\pOops, index < 0 after adding volume.");
		} else {
			DebugStr("\pOops, error getting info on perfectly good volume.");
		}
	}

	if (index < 0)
		DebugStr("\pOops, index < 0");

	return index;
}


//--------------------------------------------------------------------------------
/*
	Given a volume name and creation date, create a record for the volume and
	add it to the volume list.

	The reason why we enter volumes for which we can�t find a vRefNum is so we
	can restore the list of saved volumes from disk, even if the volumes they
	represent aren�t mounted. This is useful in case we�ve remembered files on
	AppleShare volumes or floppy disks.
*/

short	AddVolume(Str255 name, unsigned long crDate)
{
	VolumeEntryHdl	newEntry;
	short			index;

	newEntry = (VolumeEntryHdl) NewHandleSys(sizeof(VolumeEntry) - 2 + StrLength(name) + 1);

	(**newEntry).vRefNum = 0;
	(**newEntry).creationDate = crDate;
	PStringCopy((StringPtr) (**newEntry).name, name);

	AppendRecord(gVolumeList, (Handle) newEntry);
	index = GetIndexFromVolName(name, crDate);

	return index;
}


//--------------------------------------------------------------------------------
/*
	Walk our list of remembered volumes, assigning a vRefNum to each one.
*/

void	UpdateAllVRefNums()
{
	short			index;
	VolumeEntryHdl	entry;
	
	for (index = 0; index < (**gVolumeList).numRecords; index++)
	{
		entry = (VolumeEntryHdl) (**gVolumeList).records[index];
		UpdateVRefNum(entry);
	}
}


//--------------------------------------------------------------------------------
/*
	Given a handle to a volume entry, try to find its current vRefNum. We�d
	need to do this, for example, if the user has remembered a file on an
	ejectable volume, ejected the volume, and later remounted the volume with
	a different vRefNum. At this time, our entry holds the old (now bogus)
	vRefNum. This routine finds the current vRefNum. If it can�t find a volume
	matching the given name and creation date, it sets the vRefNum to zero.
*/

void	UpdateVRefNum(VolumeEntryHdl entry)
{
#if 0
	ParamBlockRec	pb;
	Str255			volName;
	OSErr			err;
	Boolean			walkIt;
	short			index;

	//
	// First, see if the volume with the given vRefNum
	// has the right name and creation date. This will
	// save us the time of walking the VCB list.
	//

	walkIt = TRUE;

	if ((**entry).vRefNum != 0) {
		pb.volumeParam.ioNamePtr = volName;
		pb.volumeParam.ioVRefNum = (**entry).vRefNum;
		pb.volumeParam.ioVolIndex = 0;
		err = PBGetVInfoSync(&pb);
		if (err == noErr) {
			walkIt = ((**entry).creationDate != pb.volumeParam.ioVCrDate)
					|| (PLstrcompare((StringPtr) volName,
									(StringPtr) (**entry).name) != 0);
		}
	}

	if (walkIt) {

		//
		// They don�t match. Index through all the volumes, trying to find
		// one with a matching name and creation date.
		//

		(**entry).vRefNum = 0;		// Assume massive failure and humiliation
		index = 0;
		pb.volumeParam.ioNamePtr = volName;
		do {
			pb.volumeParam.ioVolIndex = ++index;
			err = PBGetVInfoSync(&pb);
			if (err == noErr) {
				if ((**entry).creationDate == pb.volumeParam.ioVCrDate
				  && !PLstrcompare((StringPtr)volName, (StringPtr)(**entry).name)) {
					(**entry).vRefNum = pb.volumeParam.ioVRefNum;
					break;
				}
			}
		} while (err == noErr);
	}
#else
	VCB*			curVCB;
	unsigned long	crDate = (**entry).creationDate;
	
	(**entry).vRefNum = 0;		// Assume massive failure and humiliation

	curVCB = (VCB*) GetVCBQHdr()->qHead;
	while (curVCB != NIL)
	{
		if ((crDate == curVCB->vcbCrDate)
				&& (PLstrcompare((StringPtr) curVCB->vcbVN,
								(StringPtr) (**entry).name) == 0))
		{
			(**entry).vRefNum = curVCB->vcbVRefNum;
			break;
		}
		curVCB = (VCB*) curVCB->qLink;
	}
#endif
}


//--------------------------------------------------------------------------------
/*
	Given the index of a volume entry, return the vRefNum of that volume.
*/

short	GetVRefNumFromIndex(short index)
{
	VolumeEntryHdl	entry;

	//
	// What I'm trying to do here is maintain a one-level cache of
	// index -> vRefNum matches. If we call this function with the
	// same index as we did last time, simply return the result we
	// did last time. However, that last result will be invalid if
	// a volume's been mounted or unmounted in between those two
	// calls. So I patch MountVol and UnmountVol to look for these
	// events.
	//

	if (pMountOccured)
	{
		pMountOccured = false;
	}
	else if (index == pLastIndex)
	{
		return pLastVRefNum;
	}

	entry = (VolumeEntryHdl) (**gVolumeList).records[index];
	UpdateVRefNum(entry);
	
	pLastIndex = index;
	pLastVRefNum = (**entry).vRefNum;

	return (**entry).vRefNum;
}

//--------------------------------------------------------------------------------

pascal void	MyMountVol()
{
	asm
	{
		clr.l		-(sp)					; make room for the new return address
		move.l		A4,-(sp)
		lea			main,A4
		
		move.w		#true,pMountOccured
		
		move.l		gOldMountVol,1*4(sp)
		move.l		(sp)+,A4
	}
}

pascal void	MyUnmountVol()
{
	asm
	{
		clr.l		-(sp)					; make room for the new return address
		move.l		A4,-(sp)
		lea			main,A4
		
		move.w		#true,pMountOccured
		
		move.l		gOldUnmountVol,1*4(sp)
		move.l		(sp)+,A4
	}
}


//--------------------------------------------------------------------------------
/*
	Given a volume name and creation date, try to find the index of the entry
	in our volume list that represents it. If we can�t find the entry in our
	list of remembered volumes, we return -1.
*/

short	GetIndexFromVolName(Str255 name, unsigned long crDate)
{
	short			index;
	VolumeEntryHdl	entry;
	Boolean			found;

	UpdateAllVRefNums();
	for (index = 0; index < (**gVolumeList).numRecords; index++) {
		entry = (VolumeEntryHdl) (**gVolumeList).records[index];
		HLock((Handle) entry);
		found = ((**entry).creationDate == crDate
				&& PLstrcompare((StringPtr)(**entry).name, (StringPtr)name) == 0);
		HUnlock((Handle) entry);
		if (found)
			return index;
	}

	return -1;
}


//--------------------------------------------------------------------------------
/*
	Given a vRefNum, return the index of the volume that corresponds to the
	volume represented by the vRefNum. If we can�t find the volume on our list
	of remembered volumes, return -1.
*/

short	GetIndexFromVRefNum(short vRefNum)
{
	short			index;
	VolumeEntryHdl	entry;

	UpdateAllVRefNums();
	for (index = 0; index < (**gVolumeList).numRecords; index++) {
		entry = (VolumeEntryHdl) (**gVolumeList).records[index];
		if ((**entry).vRefNum == vRefNum)
			return index;
	}

	return -1;
}


//--------------------------------------------------------------------------------

Boolean	FileHandleExists(FileObjectHdl record)
{
	FSSpec	spec;
	return FSMakeFSSpec(GetVolRefNum(record), (**record).parID,
						*(**record).fName, & spec) == 0;
}


//--------------------------------------------------------------------------------

Boolean	FileSpecExists(FSSpecPtr spec)
{
	FInfo		finderInfo;
	return FSpGetFInfo(spec, &finderInfo) == 0;
}


//--------------------------------------------------------------------------------

void	DeleteFileObject(DynamicArrayHdl array, FileObjectHdl record)
{
	ArrayIndex		index;

	for (index = 0; index < (**array).numRecords; index++) {
		if (record == (FileObjectHdl) (**array).records[index]) {
			DisposeHandle((Handle) (**record).menuText);
			DisposeHandle((Handle) (**record).fName);
			DeleteRecord(array, index);
			break;
		}
	}
}

//--------------------------------------------------------------------------------
/*
	Try to find the given file system object in the given list of file system
	objects. If the object is found, its handle in the list is returned to the
	caller. If it is not found, NIL is returned.

	Matches are performed by comparing vRefNums, dirIDs, and file object
	names. We call PLstrcompare to compare the names, which does a non-case
	sensitive string compare.
*/

FileObjectHdl	SearchListForFileObject(DynamicArrayHdl array, FSSpecPtr filePtr)
{
	ArrayIndex		index;
	FileObjectHdl	obj;
	Boolean			result;
	FileObjectPtr	objPtr;
	StringHandle	name;

	for (index = 0; index < (**array).numRecords; index++) {
		obj = (FileObjectHdl) (**array).records[index];
		objPtr = *obj;
		name = objPtr->fName;
		HLock((Handle) name);
		result = ( (objPtr->parID == filePtr->parID)
				&& (GetVolRefNum(obj) == filePtr->vRefNum)
				&& (PLstrcompare(*name, filePtr->name) == 0));
		HUnlock((Handle) name);
		if (result)
			break;
	}

	if (index < (**array).numRecords)
		return obj;
	else
		return NIL;
}


//--------------------------------------------------------------------------------
/*
	Check to see if we�ve caught our limit in the given list of file objects.
	If so, find the oldest one and remove it. If not, then do nothing.
*/

void	CullOldest(DynamicArrayHdl array)
{
	ArrayIndex		index;
	ArrayIndex		oldestIndex;
	unsigned long	oldestDateTime;
	FileObjectHdl	record;

	while ((**array).numRecords > gMaxFileObjectsToRemember) {
		oldestIndex = -1;
		oldestDateTime = -1;

		for (index = 0; index < (**array).numRecords; index++) {
			record = (FileObjectHdl) (**array).records[index];
			if (!(**record).permanent && ((**record).timeAdded < oldestDateTime)) {
				oldestIndex = index;
				oldestDateTime = (**record).timeAdded;
			}
		}

		if (oldestIndex != -1) {
			record = (FileObjectHdl) (**array).records[oldestIndex];
			DeleteFileObject(array, record);
		} else
			break;
	}
}


//--------------------------------------------------------------------------------
/*
	Set the sort procedure in accordance with the value stored in gSortType.
	After the procedure is set, resort the lists of files and directories.
*/

void	SetSortProcedure(short sortType)
{
	gSortType = sortType;
	SortList(gFileList);
	SortList(gDirectoryList);
}


