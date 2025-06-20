#include "Global.h"
#include "Filing.h"
#include "List.h"				// For DynamicArrayHdl
#include "MenuSetup.h"			// For gAppendVolume and gAppendParentDirectory
#include "MiniFinder.h"			// For gHotRecord
#include "Remember.h"			// For gFileList, gSortType and SetSortProcedure()
#include "Utilities.h"			// For OptionKeyIsDown()

#include <Script.h>				// For smSystemScript

typedef struct {
	short		sortType;
	long		hotKey;
	short		hotKeyUsed;
	short		maxFileObjectsToRemember;
	short		appendVolumeName;
	short		appendParentDirectoryName;
} ConfigRec, *ConfigPtr, **ConfigHdl;

#define kDataType			'DATA'
#define kPreferencesID		0
#define kFileDataID			1
#define kDirectoryDataID	2
#define kVolumeDataID		3
#define kConfigDataID		4
#define kConfigReboundID	5

FSSpec*				gPrefsFile;

void				ReadPreferences(void);
void				ReadFileInfo(void);
void				ReadDirectoryInfo(void);
void				ReadCommon(short resID, DynamicArrayHdl toBeRestored);
void				ReadVolumeInfo(void);
void				ReadReboundInfo(void);
void				SavePreferences(void);
void				SaveFileInfo(void);
void				SaveDirectoryInfo(void);
void				SaveCommon(short resID,
								DynamicArrayHdl toBeSaved, Str255 resourceName);
void				SaveVolumeInfo(void);
void				SaveReboundInfo(void);

Handle				GetData(short resID, Str255 name);


//--------------------------------------------------------------------------------
/*
	Called at startup time to read the preferences file. The zone must already
	be set to the system zone, and �gPresFile� must already be initialized.

	If we can�t open the file, nothing occurs and we return the error number.
	Otherwise, we read in the preferences and the saved file, directory,
	volume, and rebound lists.
*/

void		ReadPrefsFile(void)
{
	short		refNum;
	Handle		temp;

	refNum = FSpOpenResFile(gPrefsFile, fsRdPerm);
	if (refNum != -1) {
		temp = GetResource('DATA', 3);
		if (temp != NIL) {
			ReadPreferences();
			ReadFileInfo();
			ReadDirectoryInfo();
			ReadVolumeInfo();
			ReadReboundInfo();
		}
		CloseResFile(refNum);
	}

	SetSortProcedure(gSortType);
}


//--------------------------------------------------------------------------------
/*
	This procedure is installed as a ShutDown procedure. It is at this time
	(and no other) that we save out our lists and preferences. Yes, this means
	that if we crash and the user has to reboot, they loose all information
	they accumulated during the last session.

	We have a dead-man�s switch here: if the option key is held down while the
	machine is shutting down, we don�t write out the preferences in memory.
	Indeed, we delete the preferences file on disk. This is so we can flush
	bad files and information in memory.

	Apparently, at one time, I thought it would be a good idea to switch over
	to the system zone when writing out my information. I�m not sure why I do
	this, but it doesn�t hurt to keep it in.
*/

pascal void	SaveOnShutDown(void)
{
	short		refNum;
	THz			currentZone;

	#ifdef BREAK_ON_ENTRY
	DebugStr("\pBreaking on SaveOnShutDown");
	#endif

	SetUpA4();

	if (!OptionKeyIsDown()) {
		currentZone = GetZone();
		SetZone(SystemZone());

		refNum = FSpOpenResFile(gPrefsFile, fsRdWrPerm);
		if (refNum == -1) {
			FSpCreateResFile(gPrefsFile, 'PStk', 'Pref', smSystemScript);
			refNum = FSpOpenResFile(gPrefsFile, fsRdWrPerm);
		}

		if (refNum != -1) {
			SavePreferences();
			SaveFileInfo();
			SaveDirectoryInfo();
			SaveVolumeInfo();
			SaveReboundInfo();
			CloseResFile(refNum);
		}

		SetZone(currentZone);
	} else {
		FSpDelete(gPrefsFile);
	}

	RestoreA4();
}


//--------------------------------------------------------------------------------
/*
	Read the resource that contains the preferences. This is the information
	that the user sets in the configuration dialog box. If there is no such
	resource, we run with the defaults stored in our global variables.

	The on disk representation of our preferences is provided by the ConfigRec
	struct declared at the top of this file. Here�s how the various fields are
	dispersed:

		sortType	- moved into the global variable �gSortType�. This
					  is a value used to determine how the menus are
					  sorted. Currently, we support only �Sort by name�
					  and �Sort by date�.
		hotKey		- the full hotKey used by the CEToolBox, in the form
					  defined byt the CEToolBox documentation. This value
					  is moved directly into the CEToolBox parameter block.
		hotKeyUsed	- A boolean used to determine if we tell the CEToolBox
					  to recognize our hotKey or not. There is a bit in
					  the CEToolBox parameter block that it uses to determine
					  this. We either set or clear this flag depending on
					  the value of the hotKeyUsed boolean.
		maxFileObjectsToRemember	- The number of files or directories
					  to remember before dropping old ones off the list.
					  There is only one value that is applied to both
					  the files and directories lists. This value is moved
					  into the global �gMaxFileObjectsToRemember.�
		appendVolumeName
		appendParentDirectoryName	- A couple of Booleans used when building
					  the file and directory menus. Each determines if
					  additional information is appended to to the file or
					  directory name. Each of these fields goes into a global
					  variable with a similar name, to be read in the MenuSetup
					  routines.
*/

void	ReadPreferences()
{
	ConfigHdl		configData;
	ConfigPtr		configPtr;
	long			handleSize;


	configData = (ConfigHdl) GetResource(kDataType, kConfigDataID);
	if (configData != NIL) {
		handleSize = GetHandleSize((Handle) configData);
		configPtr = *configData;
		gSortType = configPtr->sortType;
		gHotRecord.HBKeyStroke = configPtr->hotKey;
		if (configPtr->hotKeyUsed) {
			gHotRecord.HBFlags |= HBMHasHotKey;
		} else {
			gHotRecord.HBFlags &= ~HBMHasHotKey;
		}
		gMaxFileObjectsToRemember = configPtr->maxFileObjectsToRemember;
		gAppendVolumeName = configPtr->appendVolumeName;
		gAppendParentDirectoryName = configPtr->appendParentDirectoryName;
	} else {
		// Set statically
		// gSortType = kSortByName;
		// gHotRecord.HBKeyStroke = opt-ctrl-O
		// gHotRecord.HBFlags = includes HotKey
		// gMaxFileObjectsToRemember = 30
		// gAppendVolumeName = FALSE
		// gAppendParentDirectoryName = FALSE
	}
}


//--------------------------------------------------------------------------------
/*
	Read the list of recently accessed files. Because the routine to do this
	is very similar to the routine to read the list of recently accessed
	directories, we use a common routine to read them both.
*/

void	ReadFileInfo()
{
	ReadCommon(kFileDataID, gFileList);
}


//--------------------------------------------------------------------------------
/*
	Read the list of recently accessed directories. Because the routine to do
	this is very similar to the routine to read the list of recently accessed
	files, we use a common routine to read them both.
*/

void	ReadDirectoryInfo()
{
	ReadCommon(kDirectoryDataID, gDirectoryList);
}


//--------------------------------------------------------------------------------
/*
*/

void	ReadReboundInfo()
{
	ReadCommon(kConfigReboundID, gReboundList);
}


//--------------------------------------------------------------------------------
/*
	Common routine to read the list of recently accessed files or folders. The
	on-disk representation of the record for each item is a �flattened�
	version of the in-memory representation. The in-memory representation is a
	record with a bunch of fields holds values or handles. The fields are
	written out �as is�, and the handles are flattened. When reading the
	fields back, we recreate the handles.
*/

void	ReadCommon(short resID, DynamicArrayHdl toBeRestored)
{
#undef kAllSize
#undef kStdSize
#define kAllSize sizeof(FileObject)
#define kStdSize (sizeof(FileObject) - 8)

	Handle			saved;
	Ptr				dataEnd;
	Ptr				workPtr;
	StringPtr		namePtr;
	FileObjectHdl	newObject;

	saved = GetResource(kDataType, resID);
	if (saved != NIL) {
		HLock(saved);
		workPtr = *saved;
		dataEnd = workPtr + GetHandleSize(saved);
		while (workPtr < dataEnd) {
			namePtr = (StringPtr) (workPtr + kStdSize);
			newObject = (FileObjectHdl) NewHandleSysClear(kAllSize);
			if (newObject != NIL) {
				BlockMove(workPtr, *newObject, kStdSize);
				(**newObject).fName = NewStringSys(namePtr);
				AppendRecord(toBeRestored, (Handle) newObject);
			}
			workPtr += kStdSize + StrLength(namePtr) + 1;
		}
		ReleaseResource(saved);
	}
}


//--------------------------------------------------------------------------------

void	ReadVolumeInfo()
{
#undef kStdSize
#define kStdSize (sizeof(VolumeEntry) - 2)

	Handle			saved;
	Ptr				dataEnd;
	Ptr				workPtr;
	StringPtr		namePtr;

	saved = GetResource(kDataType, kVolumeDataID);
	if (saved != NIL) {
		HLock(saved);
		workPtr = *saved;
		dataEnd = workPtr + GetHandleSize(saved);
		while (workPtr < dataEnd) {
			namePtr = (StringPtr) (workPtr + kStdSize);
			AddVolume(namePtr, ((VolumeEntryPtr) workPtr)->creationDate);
			workPtr += kStdSize + StrLength(namePtr) + 1;
		}
		ReleaseResource(saved);
	}
}


//--------------------------------------------------------------------------------

void	SavePreferences()
{
	ConfigHdl		configData;
	ConfigPtr		configPtr;

	configData = (ConfigHdl) GetData(kConfigDataID, "\pPreferences");
	SetHandleSize((Handle) configData, sizeof(ConfigRec));

	configPtr = *configData;
	configPtr->sortType = gSortType;
	configPtr->hotKey = gHotRecord.HBKeyStroke;
	configPtr->hotKeyUsed = gHotRecord.HBFlags & HBMHasHotKey;
	configPtr->maxFileObjectsToRemember = gMaxFileObjectsToRemember;
	configPtr->appendVolumeName = gAppendVolumeName;
	configPtr->appendParentDirectoryName = gAppendParentDirectoryName;

	ChangedResource((Handle) configData);
}


//--------------------------------------------------------------------------------

void	SaveFileInfo()
{
	SaveCommon(kFileDataID, gFileList, "\pRecent Files");
}


//--------------------------------------------------------------------------------

void	SaveDirectoryInfo()
{
	SaveCommon(kDirectoryDataID, gDirectoryList, "\pRecent Directories");
}


//--------------------------------------------------------------------------------

void	SaveReboundInfo()
{
	SaveCommon(kConfigReboundID, gReboundList, "\pRebound Files");
}


//--------------------------------------------------------------------------------

void	SaveCommon(short resID, DynamicArrayHdl toBeSaved, Str255 resourceName)
{
#undef kAllSize
#undef kStdSize

#define kAllSize sizeof(FileObject)
#define kStdSize (sizeof(FileObject) - 8)

	Handle			saved;
	ArrayIndex		index;
	FileObjectHdl	thisFileHdl;

	saved = GetData(resID, resourceName);

	for (index = 0; index < (**toBeSaved).numRecords; index++) {
		thisFileHdl = (FileObjectHdl) (**toBeSaved).records[index];
		HLock((Handle) thisFileHdl);
		HLock((Handle) (**thisFileHdl).fName);
		PtrAndHand(*thisFileHdl, saved, kStdSize);
		HandAndHand((Handle) (**thisFileHdl).fName, saved);
		HUnlock((Handle) thisFileHdl);
		HUnlock((Handle) (**thisFileHdl).fName);
	}

	ChangedResource(saved);
}


//--------------------------------------------------------------------------------

void	SaveVolumeInfo()
{
	Handle			saved;
	ArrayIndex		index;
	VolumeEntryHdl	thisVolumeHdl;

	saved = GetData(kVolumeDataID, "\pVolumes");

	for (index = 0; index < (**gVolumeList).numRecords; index++) {
		thisVolumeHdl = (VolumeEntryHdl) (**gVolumeList).records[index];
		HLock((Handle) thisVolumeHdl);
		HandAndHand((Handle) thisVolumeHdl, saved);
		HUnlock((Handle) thisVolumeHdl);
	}

	ChangedResource(saved);
}


//--------------------------------------------------------------------------------

Handle	GetData(short resID, Str255 name)
{
	Handle	saved;

	saved = GetResource(kDataType, resID);
	if (saved == NIL) {
		saved = NewHandle(0);
		AddResource(saved, kDataType, resID, name);
	} else {
		SetHandleSize(saved, 0);
	}

	return saved;
}
