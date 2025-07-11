#include "Global.h"
#include "MenuSetup.h"
#include "Conversions.h"			// HandleToFSSpec
#include "Pack3.h"					// gSelector
#include "PLStringFuncs.h"			// PStringCopy
#include "Utilities.h"				// ReportError


typedef struct {
	long		fileType;
	short		localIconID;
} FREF, *FREFPtr, **FREFHdl;

short				gAppendVolumeName;
short				gAppendParentDirectoryName;

MenuHandle			CreateMenuCommon(short resID);
void				AddCommon(MenuHandle menu, DynamicArrayHdl array);
void				GetParentName(FileObjectHdl entry, StringPtr name);
void				GetVolumeName(FileObjectHdl entry, StringPtr name);
void				ClearMarks(DynamicArrayHdl array);
#ifdef __NEVER__
Boolean				CountOpenableFile(FileObjectHdl record, short* index);
#endif


//--------------------------------------------------------------------------------

MenuHandle			CreateDirectoryMenu(void)
{
	MenuHandle	menu;
	
	menu = CreateMenuCommon(kDirectoryMenuResID);
	return menu;
}


//--------------------------------------------------------------------------------

MenuHandle			CreateFileMenu(void)
{
	MenuHandle	menu;
	menu = CreateMenuCommon(kFileMenuResID);	
	return menu;
}


//--------------------------------------------------------------------------------

MenuHandle			CreateMenuCommon(short resID)
{
	MenuHandle	menu;
	
	menu = NewMenu(88, "\p?");
	if (menu != NIL) {
		(**menu).menuID = GetFreeHMenuID();
	} else {
		ReportError(kCantMakeMenu, 0);
	}
	
	return menu;
}


//--------------------------------------------------------------------------------

short	GetFreeHMenuID(void)
{
	short	index;
	
	for (index = 235-20; index > 0; index--) {
		if (GetMHandle(index) == NIL)
			return index;
	}
	return -1;
}


//--------------------------------------------------------------------------------

void	AddDirectories(MenuHandle menu)
{
	AddCommon(menu, gDirectoryList);
}


//--------------------------------------------------------------------------------

void	AddFiles(MenuHandle menu)
{
	AddCommon(menu, gFileList);
}


//--------------------------------------------------------------------------------

void	AddCommon(MenuHandle menu, DynamicArrayHdl array)
{
	short			count;
	ArrayIndex		index;
	FileObjectHdl 	fileObject;
	short			item;
	Str255			composite;
	
	if (menu == NIL)
		ReportError(kMenuNILInAddCommon, 0);
	else {
		count = CountMItems(menu);
		while (count--) {
			DelMenuItem(menu, 1);
		}

		if (array != NIL) {
			for (index = 0; index < (**array).numRecords; index++) {
				fileObject = (FileObjectHdl) (**array).records[index];

				CreateMenuText(fileObject, composite);
			
				if (StrLength(composite) == 0) {
					composite[0] = 1;
					composite[1] = '?';
				}
			
				AppendMenu(menu, "\p0");
				item = CountMItems(menu);
				SetItem(menu, item, composite);

				if ((**fileObject).permanent)
					SetItemStyle(menu, item, underline);
				if (!(**fileObject).openableByCurrentApp)
					DisableItem(menu, item);
			}
		}
	}
}


//--------------------------------------------------------------------------------

void	CreateMenuText(FileObjectHdl entry, StringPtr menuText)
{
	Str255			volName;
	Str255			parentName;

	if ((**entry).openableByCurrentApp) {
		if ((**entry).menuText != NIL) {
			HandleToStr255((**entry).menuText, menuText);
		} else {
			PStringCopy(menuText, *(**entry).fName);
		
			if (gAppendVolumeName || gAppendParentDirectoryName) {
				
				PLstrcat(menuText, "\p (");
				if (gAppendVolumeName) {
					GetVolumeName(entry, volName);
					PLstrcat(menuText, volName);
					if (gAppendParentDirectoryName)
						PLstrcat(menuText, "\p�");
				}
				
				if (gAppendParentDirectoryName) {
					GetParentName(entry, parentName);
					PLstrcat(menuText, parentName);
				}
				PLstrcat(menuText, "\p)");
				(**entry).menuText = NewStringSys(menuText);
			}
		}
	} else {
		PStringCopy(menuText, *(**entry).fName);
		DisposeHandle((Handle) (**entry).menuText);	// OK even if NIL
		(**entry).menuText = NIL;
	}
}

//--------------------------------------------------------------------------------

void	GetParentName(FileObjectHdl entry, StringPtr name)
{
	OSErr			err;
	CInfoPBRec		pb;
	
	pb.dirInfo.ioNamePtr = name;
	pb.dirInfo.ioVRefNum = GetVolRefNum(entry);
	pb.dirInfo.ioFDirIndex = -1;
	pb.dirInfo.ioDrDirID = (**entry).parID;
	
	if (pb.dirInfo.ioVRefNum != 0)
		err = PBGetCatInfoSync(&pb);
	else
		err = nsvErr;

	if (err != noErr)
		name[0] = 0;
}


//--------------------------------------------------------------------------------

void	GetVolumeName(FileObjectHdl entry, StringPtr name)
{
	OSErr			err;
	ParamBlockRec	pb;
	
	pb.volumeParam.ioNamePtr = name;
	pb.volumeParam.ioVolIndex = 0;
	pb.volumeParam.ioVRefNum = GetVolRefNum(entry);
	
	if (pb.volumeParam.ioVRefNum != 0)
		err = PBGetVInfoSync(&pb);
	else
		err = nsvErr;

	if (err != noErr)
		name[0] = 0;
}


//--------------------------------------------------------------------------------

void	FilterDirectories()
{
	ArrayIndex		index;
	FileObjectHdl 	dirObject;

	ClearMarks(gFileList);

#if 0
	for (index = 0; index < (**gDirectoryList).numRecords; index++) {
		dirObject = (FileObjectHdl) (**gDirectoryList).records[index];
		if (GetVolRefNum(dirObject) < 0) {
			if (FileHandleExists(dirObject)) {
				(**dirObject).openableByCurrentApp = TRUE;
			} else {
				DeleteFileObject(gDirectoryList, dirObject);
				index--;
			}
		}
	}
#endif
}


//--------------------------------------------------------------------------------

void	FilterFilesByFREF()
{
	ArrayIndex		index;
	FileObjectHdl 	fileObject;
	short			oldRefNum;
	short			typeCount;
	OSType**		typeHandle;
	FREFHdl			fref;
	FSSpec			spec;
	FileObjectPtr	recordPtr;
	FInfo			finderInfo;
	short			typeNumber;
	long			fileType;

	ClearMarks(gFileList);

#if 0
	oldRefNum = CurResFile();
	UseResFile(CurApRefNum);

	typeCount = Count1Resources('FREF');
	if (typeCount > 0) {
		typeHandle = (OSType**) NewHandle(typeCount * sizeof(OSType));
		for (index = 1; index <= typeCount; index++) {
			fref = (FREFHdl) Get1IndResource('FREF', index);
			(*typeHandle)[index-1] = (**fref).fileType;
			ReleaseResource((Handle) fref);
		}
		
		for (index = 0; index <= typeCount; index++) {
			if ((*typeHandle)[index] == kApplicationSig) {
				(*typeHandle)[index] = (*typeHandle)[typeCount-1];
				typeCount--;
			}
		}

		for (index = 0; index < (**gFileList).numRecords; index++) {
			fileObject = (FileObjectHdl) (**gFileList).records[index];
			if (GetVolRefNum(fileObject) != 0) {
				if (HandleToFSSpec(fileObject, &spec) == noErr) {
					recordPtr = *fileObject;
					if (recordPtr->u.type == kUnknownSig
					  || (long) recordPtr->u.type < 0x000FFFFF) {
						if (FSpGetFInfo(&spec, &finderInfo) == noErr) {
							recordPtr->u.type = finderInfo.fdType;
						}
					}
				
					for (typeNumber = 0; typeNumber < typeCount; typeNumber++) {
						fileType = (*typeHandle)[typeNumber];
						if (fileType == kOpenAnyFile || fileType == recordPtr->u.type) {
							recordPtr->openableByCurrentApp = TRUE;
						}
					}
				} else {
					/*
						Remove anybody who cannot be found and who is supposed
						to be on a mounted volume.
					*/
					DeleteFileObject(gFileList, fileObject);
					index--;
				}
			}
		}
		DisposeHandle((Handle) typeHandle);
	}
	UseResFile(oldRefNum);
#endif
}


//--------------------------------------------------------------------------------

void	FilterFilesByHookAndList()
{
	ArrayIndex		index;
	FileObjectHdl 	fileObject;
	FSSpec			file;
	OSType			itsType;
	OSErr			itsErr;
	
	ClearMarks(gFileList);

#if 0
	for (index = 0; index < (**gFileList).numRecords; index++) {
		fileObject = (FileObjectHdl) (**gFileList).records[index];
		file.vRefNum = GetVolRefNum(fileObject);
		file.parID = (**fileObject).parID;
		PStringCopy(file.name, *(**fileObject).fName);
		if (CanOpenFile(&file, &itsType, &itsErr)) {
			(*fileObject)->u.type = itsType;
			(*fileObject)->openableByCurrentApp = TRUE;
		} else {
			if (itsErr != noErr) {
				DeleteFileObject(gFileList, fileObject);
				index--;
			}
		}
	}
#endif
}


//--------------------------------------------------------------------------------

void	ClearMarks(DynamicArrayHdl array)
{
	ArrayIndex		index;
	FileObjectHdl 	fileObject;

	for (index = 0; index < (**array).numRecords; index++) {
		fileObject = (FileObjectHdl) (**array).records[index];
#if 0
		(**fileObject).openableByCurrentApp = FALSE;
#else
		(**fileObject).openableByCurrentApp = TRUE;
#endif
	}
}


//--------------------------------------------------------------------------------

FileObjectHdl	GetNthOpenableFileObject(DynamicArrayHdl array, short index)
{
	FileObjectHdl	objHdl;
	
	objHdl = (FileObjectHdl) (**array).records[index];
	if (objHdl != NIL)
		return objHdl;
	else {
		ReportError(kCantFindFileForMenu, 0);
		return NIL;
	}
}

#ifdef __NEVER__
Boolean	CountOpenableFile(FileObjectHdl record, short* index)
{
	if ((**record).openableByCurrentApp) {
		if (--*index < 0) {
			return TRUE;
		}
	}
	return FALSE;
}
#endif
