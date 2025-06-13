// Copyright © 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// FileTools.cp

#include "FileTools.h"
#include "UASyncFile.h"
#include "Tools.h"

#include "RsrcGlobals.h"

#include <Errors.h>
#include <Packages.h>
#include <stdio.h>
#include <Folders.h>
#include <Finder.h>

#pragma segment MyTools

#define qDebugFindFolder qDebug & 0

Boolean FileExist(const FSSpec &spec)
{
	HParamBlockRec pb;
	OSErr err = MyGetFileInfo(spec, pb);
	if (err == fnfErr)
		return false;
	FailOSErr(err);
	return true;
}

Boolean FileExist(TFile *file)
{
	if (file->fDataRefNum != kNoFileRefnum || file->fRsrcRefNum != kNoFileRefnum)
		return true; // file open: file must exist
	FSSpec spec;
	file->GetFileSpec(spec);
	return FileExist(spec);
}

void ResolveAliasSpec(FSSpec &spec)
{
	HParamBlockRec pb;
	OSErr err = MyGetFileInfo(spec, pb);
	if (err == fnfErr)
		return;
	FailOSErr(err);
	if ((pb.fileParam.ioFlFndrInfo.fdFlags & kIsAlias) == 0)
		return; // not an alias file
	Boolean targetIsFolder, wasAliased;
	err = ResolveAliasFile(&spec, true, &targetIsFolder, &wasAliased);
	if (err != fnfErr)
		FailOSErr(err);
}

void ResolveAliasTFile(TFile *file)
{
	ResolveAliasSpec(file->fFileSpec);
}

void MyRenameFile(TFile *file, const CStr255 &newname)
{
	file->RenameFile(newname);
	CopyCString2String(newname, file->fFileSpec.name); // RenameFile() doesn't update this
}

void CheckFilenameSanity(CStr255 &filename)
{
// substitute invalid chars
	for (short i = filename.Length(); i; i--)
	{
		unsigned char ch = (unsigned char)filename[i];
		if (ch == ':')
			filename[i] = '_';
		else if (ch == '/' && gConfiguration.hasAUX)
			filename[i] = '_';
		else if (ch < 32)
			filename.Delete(i, 1);
	}
// chop off extra spaces
	while (filename.Length() && filename[1] == 32)
		filename.Delete(1, 1);
	while (filename.Length() && filename[filename.Length()] == 32)
		filename.Length()--;
// no colon to start with
	if (filename.Length() && filename[1] == '.')
		filename[1] = '¥';
// not too long name
	if (filename.Length() > 31)
		filename.Length() = 31;
// use something if empty
	if (filename.Length() == 0)
		MyGetIndString(filename, kUntitledFileName);
}

void MakeFilenameUnique(FSSpec &spec)
{
	if (!FileExist(spec))
		return;
	CStr255 s(spec.name);
	short pos = s.Pos("¥");
	if (pos)
		s.Delete(pos, s.Length() - pos + 1);
	long nr = 0;
	while (FileExist(spec))
	{
		nr++;
		CStr255 num;
		NumToString(nr, num);
		num.Insert("¥", 1);
		if (s.Length() + num.Length() > 31)
			s.Length() = 31 - num.Length();
		CopyCString2String(s + num, spec.name);
	}
}

void MakeFilenameUnique(TFile *file)
{
	FSSpec spec;
	file->GetFileSpec(spec);
	MakeFilenameUnique(spec);
	if (file->fDataRefNum != kNoFileRefnum || file->fRsrcRefNum != kNoFileRefnum)
		MyRenameFile(file, spec.name);
	else
		file->Specify(spec);
}

void FlushVols()
{
	CStr255 name("");
	short vRefNum;
	short index = 0;
	while (true)
	{
		short errNo = GetIndVolume(++index, vRefNum);
		if (errNo == nsvErr)
			break; // ignore all others!
		FlushVol(name, vRefNum);
	}
}

void GoInsideFolder(FSSpec &spec)
{
	CStr255 s;
	s = spec.name;
	CheckFilenameSanity(s);
	CopyCString2String(s, spec.name);
	short errNo;
	Boolean targetIsFolder, wasAliased;
	errNo = ResolveAliasFile(&spec, true, &targetIsFolder, &wasAliased);
	if (errNo != fnfErr)
		FailOSErr(errNo);
	CInfoPBRec info;
	MyGetCatInfo(spec, info);
	if (info.dirInfo.ioResult == fnfErr)
	{
		long createdDirID;
		FailOSErr(FSpDirCreate(&spec, smRoman, &createdDirID));
		spec.parID = createdDirID;
	}
	else
	{
		FailOSErr(info.dirInfo.ioResult);
		if ((info.dirInfo.ioFlAttrib & ioDirMask) == 0) // if it is not a dir
			FailOSErr(notAFileErr);
		spec.parID = info.dirInfo.ioDrDirID;
	}
}

void GoGroupFile(const CStr255 &groupName, FSSpec &spec)
{
#if qDebug
	long numIter = 0;
#endif
	CStr255 gn(groupName);
	short dotPos;
	while (gn.Length())
	{
		const short maxGroupNameLength = 29;
		if (gn.Length() <= maxGroupNameLength)
		{
			CopyCString2String(gn, spec.name);
			return;
		}
		dotPos = maxGroupNameLength;
		while (dotPos && gn[dotPos] != '.')
			dotPos--;
		CStr255 s, suffix;
		MyGetIndString(suffix, kLongGroupNameFolderSuffix);
		if (dotPos <= 1)
		{
			s = gn.Copy(1, maxGroupNameLength) + suffix;
			gn.Delete(1, maxGroupNameLength);
		}
		else
		{
			s = gn.Copy(1, dotPos - 1) + suffix;
			gn.Delete(1, dotPos);
		}
#if qDebug
		if (++numIter > 10)
			ProgramBreak("Endless loop en GoGroupFile");
#endif
		CopyCString2String(s, spec.name);
		GoInsideFolder(spec);
	}
}

void GetPathNameFromDirID(short vRefNum, long dirID, CStr255 &pathName)
{
#if qDebug & 0
	fprintf(stderr, "GetPathNameFromDirID: \n");
	fprintf(stderr, "  vRefNum = %hd\n", vRefNum);
	fprintf(stderr, "  dirID = %ld\n", dirID);
#endif
	CInfoPBRec block;
	BlockSet(&block, sizeof(CInfoPBRec), 0x00);// zero-out our storage 
	CStr255 directoryName = "";
	pathName = "";
	block.dirInfo.ioNamePtr = StringPtr(&directoryName);
	block.dirInfo.ioDrParID = dirID;
	do
	{
		block.dirInfo.ioVRefNum = vRefNum;
		block.dirInfo.ioFDirIndex = -1;
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
		if (MyGetCatInfo(block) != noErr)
			break;
		if (block.dirInfo.ioDrDirID == block.dirInfo.ioDrParID)
		{
#if qDebug
			if (block.dirInfo.ioDrDirID != 2)
				fprintf(stderr, "GetPathName: ioDrDirID == ioDrParID == %ld\n", long(block.dirInfo.ioDrDirID));
#endif
			break; // at root at a shared folder!
		}
		if (gConfiguration.hasAUX)
		{
			if (directoryName[1] != '/')
				// If this isn't root (i.e. "/"), append a slash ('/')
				directoryName += '/';
		}
		else
		{
			directoryName += ':';
		}
		pathName.Insert(directoryName, 1);
	}
	while (block.dirInfo.ioDrDirID != fsRtDirID);
}


void ChangeFileTypeAndCreator(TFile *file, OSType newType, OSType newCreator)
{
	FailNonObject(file);
	FInfo fndrInfo;
	FailOSErr(file->GetFinderInfo(fndrInfo));
	fndrInfo.fdType = newType;
	fndrInfo.fdCreator = newCreator;
	FailOSErr(file->SetFinderInfo(fndrInfo));
	file->fFileType = newType;
	file->fCreator = newCreator;
// modify folder so Finderª updates the display
	CInfoPBRec info;
	CStr255 name(gEmptyString); // from TechNote: never nil pointer!
	BlockSet(&info, sizeof(CInfoPBRec), 0);// zero-out our storage 
	// use special: -1 means use DrDirID as dir id for directory
	info.dirInfo.ioFDirIndex = -1;
	info.dirInfo.ioDrDirID = file->fFileSpec.parID;
	info.dirInfo.ioVRefNum = file->fFileSpec.vRefNum;
	info.dirInfo.ioNamePtr = name;
	FailOSErr(MyGetCatInfo(info));
	GetDateTime(&info.dirInfo.ioDrMdDat);
	// use normal: 0 means use trio: vrefnum, parID and name on dir
	info.dirInfo.ioFDirIndex = 0;
	info.dirInfo.ioDrDirID = info.dirInfo.ioDrParID;
	info.dirInfo.ioVRefNum = file->fFileSpec.vRefNum;
	info.dirInfo.ioNamePtr = name;
	OSErr errNo = MySetCatInfo(info);
#if qDebug
	if (errNo)
	{
		fprintf(stderr, "Could not set modification date on folder, err = %hd\n", errNo);
		ProgramBreak(gEmptyString);
	}
#endif
}

void AskFileName(const CStr255 &prompt, const CStr255 &defaultFilename, FSSpec &spec)
{
	// should make one which Yield time to other threads
	FailOSErr(MAInteractWithUser(kNoTimeOut, gNotificationPtr, gMacAppAppleEventIdleProc));
	StandardFileReply customReply;
	CopyCString2String(defaultFilename, customReply.sfFile.name);

	static ModalFilterYDUPP modalFilterYDProcPtrUPP = nil;
	if (!modalFilterYDProcPtrUPP)
		modalFilterYDProcPtrUPP = NewModalFilterYDProc(gModalFilterYDProcPtr);

	gClipboardMgr->AboutToLoseControl(TRUE);		// so scrap gets converted
	CustomPutFile(prompt, defaultFilename, &customReply, sfPutDialogID,
	/**/   CPoint(-1, -1), nil, modalFilterYDProcPtrUPP, nil, nil, nil);
	gClipboardMgr->RegainControl(TRUE);			// so scrap gets converted
	if (!customReply.sfGood)
		Failure(0, 0);
	spec = customReply.sfFile;
}
//==========================================================================================
struct FinderFolderInfo
{
	OSErr err;
	short vRefNum, foundVRefNum;
	OSType folderType;
	Boolean createFolder;
	long foundDirID;
	FinderFolderInfo *next;
};

OSErr MyFindFolder(short vRefNum, OSType folderType, Boolean createFolder, short *foundVRefNum, long *foundDirID)
{
	static FinderFolderInfo *root;
	FinderFolderInfo *ffi = root;
	while (ffi)
	{
		if (vRefNum == ffi->vRefNum && folderType == ffi->folderType)
		{
			if (createFolder && !ffi->createFolder)
				break;
			*foundVRefNum = ffi->foundVRefNum;
			*foundDirID = ffi->foundDirID;
#if qDebugFindFolder
			fprintf(stderr, "FindFolder, reuses %ld:%s -> %ld:%ld\n", long(vRefNum), OSType2String(folderType), long(ffi->foundVRefNum), ffi->foundDirID);
#endif
			return ffi->err;
		}
		ffi = ffi->next;
	}
	if (ffi == nil)
	{
		ffi = new FinderFolderInfo;
		if (ffi == nil)
			return memFullErr;
		ffi->next = root;
		root = ffi;
	}
	ffi->vRefNum = vRefNum;
	ffi->folderType = folderType;
	ffi->createFolder = createFolder;
	ffi->err = FindFolder(vRefNum, folderType, createFolder, foundVRefNum, foundDirID);
	ffi->foundVRefNum = *foundVRefNum;
	ffi->foundDirID = *foundDirID;
#if qDebugFindFolder
	fprintf(stderr, "FindFolder, new: %ld:%s -> %ld:%ld\n", long(vRefNum), OSType2String(folderType), long(ffi->foundVRefNum), ffi->foundDirID);
#endif
	return ffi->err;
}

class TMyFileHandler : public TFileHandler
{
public:
	TMyFileHandler();
	void SaveViaTemp(CommandNumber itsCommandNumber,
									  Boolean makingCopy,
									  Boolean copyFInfo,
									  TFile* itsFile);
	// MacApp uses blindly FindFolder, even though some AppleShare
	// servers forget to create the Temporaries folder.
};

TFileHandler *NewMyFileHandler(TFileBasedDocument *itsDocument, TFile* itsFile)
{
	TFileHandler *aFileHandler = new TMyFileHandler;
	aFileHandler->IFileHandler(itsDocument, itsFile);
	return aFileHandler;
}

TMyFileHandler::TMyFileHandler()
{
}

void TMyFileHandler::SaveViaTemp(CommandNumber itsCommandNumber,
									  Boolean makingCopy,
									  Boolean copyFInfo,
									  TFile* itsFile)
{
	Boolean validInfo;
	CInfoPBRec cInfo;
	FSSpec saveFileSpec;
	CStr63 tmpName;
	short tmpVRefNum;
	long tmpDirID;
	OSErr err;
	VOLATILE(itsFile);
	VOLATILE(makingCopy);
	VOLATILE(tmpName);
	validInfo = this->GetSaveInfo(itsCommandNumber, copyFInfo, cInfo);
	itsFile->GetFileSpec(saveFileSpec);		// Save file spec since it may be clobbered 
	this->GetTempName(tmpName);
	// If we are using the folder manager, put in the temporary folder, creating if necessary 
	if (qNeedsFolderMgr || gConfiguration.hasFolderMgr)
	{
		err = MyFindFolder(saveFileSpec.vRefNum, kTemporaryFolderType, kCreateFolder, &tmpVRefNum, &tmpDirID);
		if (err == afpAccessDenied)
			itsFile->SetName(tmpName);
		else
		{
			FailOSErr(err);  // only use temp folder if we have access rights to it
			FailOSErr(itsFile->SpecifyWithTrio(tmpVRefNum, tmpDirID, tmpName));
		}
	}
	else
		itsFile->SetName(tmpName);
	FailInfo fi;
	Try(fi)
	{
		FailOSErr(itsFile->CreateFile());
		if (validInfo)							// Change attributes of itsFile??
			FailOSErr(itsFile->SetCatInfo(cInfo));
		FailOSErr(itsFile->OpenFile());
		this->DoWrite(itsFile, makingCopy);
		fi.Success();
	}
	else	// Recover
	{
		OSErr err;
		err = itsFile->CloseFile();
		if (qDebugMsg && err != noErr)
			fprintf(stderr, "In SaveViaTemp error recovery: error from CloseFile is %d", err);
		err = itsFile->DeleteFile();
		if (qDebugMsg && ((err != noErr) && (err != fnfErr)))
			fprintf(stderr, "In SaveViaTemp error recovery: error from itsFile.DeleteFile is %d", err);
		fi.ReSignal();
	}
	FailOSErr(itsFile->CloseFile());
	if (!makingCopy)
		this->CloseFile();							// Close the documents file 
	Try(fi)
	{
		if (qNeedsFolderMgr || gConfiguration.hasFolderMgr)
		{
			if (fFileExists && copyFInfo)
			{
				// Since ExchangeFiles doesn't check that the destination file is locked
				// we need to check ourselves.
				CInfoPBRec cInfo;
			
				BlockSet((Ptr) & cInfo, sizeof(CInfoPBRec), 0x00);// zero-out our storage 
				FailOSErr(fFile->GetCatInfo(cInfo));
				
				if (cInfo.hFileInfo.ioFlAttrib & 0x01)
					FailOSErr(fLckdErr);
				// Exchange the existing file for the one in the temporary folderÉ 
				FailOSErr(itsFile->ExchangeFiles(fFile));
				// and delete the one left in the temporary folder 
				err = itsFile->DeleteFile();
				if ((err != noErr) && (err != fnfErr))
					Failure(err, 0);
			}
			else
				// The temp file was created in the temporary directory.
				// Move it back where it belongs
				FailOSErr(itsFile->MoveAndRename(saveFileSpec));
		}
		else
		{
			itsFile->SetName(saveFileSpec.name);
			err = itsFile->DeleteFile();			// Delete it if it exists 
			if ((err != noErr) && (err != fnfErr))
				Failure(err, 0);
			itsFile->SetName(tmpName);
			FailOSErr(itsFile->RenameFile(saveFileSpec.name));
		}
		// Specify the file again since it may have changed if it was saved in a temporary folder 
		itsFile->Specify(saveFileSpec);
		fi.Success();
	}
	else	// Recover
	{
		if (!qNeedsFolderMgr && !gConfiguration.hasFolderMgr)
			itsFile->SetName(tmpName);				// Make sure we delete the temp file.
		OSErr err;
		err = itsFile->DeleteFile();
		if (qDebugMsg && ((err != noErr) && (err != fnfErr)))
			fprintf(stderr, "In SaveViaTemp error recovery: error from DeleteFile is %d", err);
		if (fFileExists &&!makingCopy)
			err = fFile->OpenFileIfKeptOpen();
		if (qDebugMsg && (err != noErr))
			fprintf(stderr, "In SaveViaTemp error recovery: error from OpenFileIfKeptOpen is %d", err);
		// Set the name to something intelligent in case we have to display it to the user.
		itsFile->SetName(saveFileSpec.name);
		fi.ReSignal();
	}
}
//==========================================================================================

TFile *NewTempFile(TFile *dataFile)
{
	TFile *tmpFile = nil;
	VOLATILE(tmpFile);
	FailInfo fi;
	Try(fi)
	{
#if qDebug
		if (!dataFile)
			ProgramBreak("dataFile may not be nil!");
#endif
		FailNIL(dataFile);
		tmpFile = NewAsyncFile(dataFile->fFileType, dataFile->fCreator, 
											dataFile->fUsesDataFork, dataFile->fUsesRsrcFork,
											dataFile->fKeepDataOpen, dataFile->fKeepRsrcOpen);
		FSSpec dataSpec, tmpSpec;
		dataFile->GetFileSpec(dataSpec);
		OSErr err = MyFindFolder(dataSpec.vRefNum, kTemporaryFolderType, kCreateFolder, &tmpSpec.vRefNum, &tmpSpec.parID);
		if (err == afpAccessDenied)
		{
			// cannot access the "Temporary Items" folder, use the same folder
			// as the file is saved into
			tmpSpec = dataSpec;
		}
		long count = 0;
		do {
			CStr255 tmpName = dataSpec.name;
			if (tmpName.Length() > 20)
				tmpName.Length() = 20;
			CStr255 s;
			NumToString(TickCount() & 0x1FFFFFFF, s);
			tmpName += s;
			CopyCString2String(tmpName, tmpSpec.name);
		} while (FileExist(tmpSpec) && ++count < 50);
		tmpFile->Specify(tmpSpec);
		tmpFile->SetPermissions(fsRdWrPerm, fsRdWrPerm);// Since we intend to write to it 
		FailOSErr(tmpFile->CreateFile());
		FInfo theInfo;
		FailOSErr(tmpFile->GetFinderInfo(theInfo));
		theInfo.fdCreator = dataFile->fCreator;
		theInfo.fdType = dataFile->fFileType;
		FailOSErr(tmpFile->SetFinderInfo(theInfo));
		FailOSErr(tmpFile->OpenFile());
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(tmpFile); tmpFile = nil;
		fi.ReSignal();
	}
	return tmpFile;
}

void DoPostSaveTmpFile(TFile *dataFile, TFile *tmpFile)
{
	FailInfo fi;
	Try(fi)
	{
		if (FileExist(dataFile))
		{
			FailOSErr(tmpFile->ExchangeFiles(dataFile));
			// dataFile now contains what we have just saved
			// so let us delete our temp file.
			// But, the file-access refnums have too been swapped,
			// so they have to be swapped back.
#define macroSwap(t,x,y) { t temp = x; x = y; y = temp; }
			macroSwap(short, dataFile->fDataRefNum, tmpFile->fDataRefNum);
			macroSwap(short, dataFile->fRsrcRefNum, tmpFile->fRsrcRefNum);
			// tmpFile refers now to the old prefs file, in the Temp folder
			FailOSErr(tmpFile->CloseFile());
			OSErr err = tmpFile->DeleteFile();		
#if qDebug
			if (err != noErr)
				fprintf(stderr, "Couldn't delete temp file, err = %ld\n", long(err));
#endif
		}
		else
		{
			// move temp file to the proper place
			FSSpec spec;
			dataFile->GetFileSpec(spec);
			FailOSErr(tmpFile->MoveAndRename(spec));
			// fix op the file-access refnums
			macroSwap(short, dataFile->fDataRefNum, tmpFile->fDataRefNum);
			macroSwap(short, dataFile->fRsrcRefNum, tmpFile->fRsrcRefNum);
		}
		fi.Success();
	}
	else // fail
	{
		tmpFile->CloseFile();
		tmpFile->DeleteFile();		
		fi.ReSignal();
	}
	return;
}

void DoFailedSaveTempFile(TFile *tmpFile)
{
	if (tmpFile)
	{
		tmpFile->CloseFile();
		tmpFile->DeleteFile();		
	}
}

Handle LoadFileIntoHandle(const FSSpec &spec, Boolean isPICT)
{
	Handle h = nil;
	VOLATILE(h);
	TFile *file = nil;
	VOLATILE(file);
	FailInfo fi;
	Try(fi)
	{
		long offset = (isPICT ? 512 : 0);
		file = NewAsyncFile('????', '????', kUsesDataFork, noResourceFork, !kDataOpen, !kRsrcOpen);
		file->Specify(spec);
		FailOSErr(file->OpenFile());
		FailOSErr(file->SetDataMark(offset, fsFromStart));
		long size;
		FailOSErr(file->GetDataLength(size));
		size -= offset;
		h = NewPermHandle(size);
		HLock(h);
		FailOSErr(file->ReadData(*h, size));
		HUnlock(h);
		FailOSErr(file->CloseFile());
		FreeIfObject(file); file = nil;
		fi.Success();
	}
	else // fail
	{
		h = DisposeIfHandle(h);
		FreeIfObject(file); file = nil;
		fi.ReSignal();
	}
	return h;
}
