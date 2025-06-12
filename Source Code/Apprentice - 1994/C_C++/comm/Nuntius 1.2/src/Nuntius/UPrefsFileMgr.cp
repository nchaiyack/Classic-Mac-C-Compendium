// Copyright © 1994 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UPrefsFileMgr.cp

#include "UPrefsFileMgr.h"
#include "Tools.h"
#include "FileTools.h"
#include "UBufferedFileStream.h"
#include "UNntp.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>

#include <Errors.h>
#include <Folders.h>

#pragma segment MyTools

PPrefsFileMgr *gPrefsFileMgr = nil;

PPrefsFileMgr::PPrefsFileMgr()
	: PPrefsDatabase()
{
	fPrefsFile = nil;
	fTmpFile = nil;
	fPrefsIsValid = false;
}

void PPrefsFileMgr::IPrefsFileMgr(FSSpec spec)
{
	IPrefsDatabase();
	FailInfo fi;
	if (fi.Try())
	{
		fPrefsFile = NewFile(kPrefsFileType, kSignature, kUsesDataFork, preferResourceFork, !kDataOpen, !kRsrcOpen);
		fTmpFile = NewFile(kPrefsFileType, kSignature, kUsesDataFork, preferResourceFork, !kDataOpen, !kRsrcOpen);
		fPrefsFile->SetPermissions(fsRdWrPerm, fsRdWrPerm);
		fTmpFile->SetPermissions(fsRdWrPerm, fsRdWrPerm);		
		fPrefsFile->Specify(spec);
		gPrefsFileMgr = this;
		fi.Success();
	}
	else // fail
	{
		delete this;
		fi.ReSignal();
	}
}

PPrefsFileMgr::~PPrefsFileMgr()
{
	gPrefsFileMgr = nil;
	if (fPrefsFile)
		fPrefsFile->CloseFile();
	FreeIfObject(fPrefsFile); fPrefsFile = nil;
	FreeIfObject(fTmpFile); fTmpFile = nil;
}

TFile *PPrefsFileMgr::GetFile()
{
	return fPrefsFile;
}

void PPrefsFileMgr::Load()
{
	DeleteAll();
	TBufferedFileStream *aStream = nil;
	VOLATILE(aStream);
	FailInfo fi;
	if (fi.Try())
	{
		fPrefsFile->SetPermissions(fsRdWrPerm, fsRdWrPerm);
		fTmpFile->SetPermissions(fsRdWrPerm, fsRdWrPerm);		
		if (!FileExist(fPrefsFile))
		{
#if qDebug
			fprintf(stderr, "Prefs _file_ did not exist, does not read it...\n");
#endif
			fi.Success();
			return;
		}
		FailOSErr(fPrefsFile->OpenFile());
		aStream = NewBufferedFileStream(fPrefsFile, 16 * 1024, 0);
		DoRead(aStream);
		FreeIfObject(aStream); aStream = nil;
//		FailOSErr(fPrefsFile->CloseFile());
		fPrefsIsValid = true; // I hope
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(aStream); aStream = nil;
		fPrefsFile->CloseFile();
		if (fi.error != fnfErr)
			gApplication->ShowError(fi.error, messageOpenPrefsFailed);
	}
}

void PPrefsFileMgr::Save()
{
	if (fPrefsIsValid == false || IsDirty() == false)
		return;
	TBufferedFileStream *aStream = nil;
	VOLATILE(aStream);
	FailInfo fi;
	if (fi.Try())
	{
		fIsDirty = false; // if we fails, just drop the settings
		SpecifyTmpFile();
		FailOSErr(fTmpFile->CreateFile());
		FailOSErr(fTmpFile->OpenFile());
		aStream = NewBufferedFileStream(fTmpFile, 0, 16 * 1024);
		DoWrite(aStream);
		FreeIfObject(aStream); aStream = nil;
		if (FileExist(fPrefsFile))
		{
			fTmpFile->ExchangeFiles(fPrefsFile);
			// fPrefsFile now contains what we have just saved
			// so let us delete our temp file.
			// But, the file-access refnums have too been swapped,
			// so they have to be swapped back.
#define macroSwap(t,x,y) { t temp = x; x = y; y = temp; }
			macroSwap(short, fPrefsFile->fDataRefNum, fTmpFile->fDataRefNum);
			macroSwap(short, fPrefsFile->fRsrcRefNum, fTmpFile->fRsrcRefNum);
			// fTmpFile refers now to the old prefs file, in the Temp folder
			FailOSErr(fTmpFile->CloseFile());
			OSErr err = fTmpFile->DeleteFile();		
#if qDebug
			if (err != noErr)
				fprintf(stderr, "Couldn't delete temp file, err = %ld\n", long(err));
#endif
		}
		else
		{
			// FailOSErr(fTmpFile->CloseFile());
			// close temp file, and move it to the proper place
			FSSpec spec;
			fPrefsFile->GetFileSpec(spec);
			OSErr err = fTmpFile->MoveAndRename(spec);
#if qDebug
			if (err != noErr)
				fprintf(stderr, "Error from MoveAndRename = %ld\n", long(err));
#endif
		}
		FailOSErr(fPrefsFile->FlushVolume());
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(aStream); aStream = nil;
		fTmpFile->CloseFile();
		fTmpFile->DeleteFile();		
		gApplication->ShowError(fi.error, messageWritePrefsFailed);
	}
}

void PPrefsFileMgr::SpecifyTmpFile()
{
//- NewFile
	short vRefNum = fPrefsFile->GetVolRefNum();
	FSSpec spec;
	OSErr err = FindFolder(vRefNum, kTemporaryFolderType, kCreateFolder, spec.vRefNum, spec.parID);
	if (err != noErr)
	{
#if qDebug
		fprintf(stderr, "Error from FindTempFolder: %ld\n", long(err));
#endif
		fPrefsFile->GetFileSpec(spec);
	}
	CStr255 s;
	MyGetIndString(s, kTempPrefsName);
	SubstituteStringItems(s, "ÇnumÈ", long(TickCount() & 0x7FFFFFFF));
	CheckFilenameSanity(s);
	CopyCString2String(s, spec.name);
	MakeFilenameUnique(spec);
	fTmpFile->Specify(spec);
	fPrefsFile->SetPermissions(fsRdWrPerm, fsRdWrPerm);
	fTmpFile->SetPermissions(fsRdWrPerm, fsRdWrPerm);		
}

void PPrefsFileMgr::SetPrefsValidationState(Boolean valid)
{
	fPrefsIsValid = valid;
}

//============================================================================
void GetPrefsLocationFileUntitledName(CStr255 &name, short stringID)
{
	MyGetIndString(name, stringID);
}

void GetPrefsDocLocation(FSSpec &spec)
{
	if (gPrefsFileMgr)
	{
		TFile *prefFile = gPrefsFileMgr->GetFile();
		if (prefFile)
		{
			spec.vRefNum = prefFile->GetVolRefNum();
			spec.parID = prefFile->GetDirID();
#if qDebug
			fprintf(stderr, "Got prefs loc = %ld:%ld from gPrefsFileMgr\n", long(spec.vRefNum), spec.parID);
#endif
			return;
		}
	}
	FailOSErr(FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, spec.vRefNum, spec.parID));
	CStr255 folderName;
	MyGetIndString(folderName, kPrivateFilesFolderName);
	if (UsesFakeNntp())
		folderName += " (DB)";
	CopyCString2String(folderName, spec.name);
	Boolean targetIsFolder, wasAliased;
	OSErr errNo = ResolveAliasFile(spec, true, targetIsFolder, wasAliased);
	if (errNo != fnfErr)
		FailOSErr(errNo);
#if qDebug
	fprintf(stderr, "targetIsFolder = %ld\n", long(targetIsFolder));
#endif
	if (errNo != fnfErr && !targetIsFolder)
	{
		StdAlert(phLocalPrefsNotAllowed);
		Failure(0, 0);
	}
	GoInsideFolder(spec);
#if qDebug
	fprintf(stderr, "Got prefs loc = %ld:%ld from FindFolder\n", long(spec.vRefNum), spec.parID);
#endif
}

void GoPrefsLocation(TFile *file, short stringID)
{
	FailInfo fi;
	if (fi.Try())
	{
		FSSpec spec;
		GetPrefsDocLocation(spec);
		CStr255 name;
		GetPrefsLocationFileUntitledName(name, stringID);
		CopyCString2String(name, spec.name);
	// create TFile
		file->Specify(spec);
		file->SetPermissions(fsRdWrPerm, fsRdWrPerm);
		ResolveAliasFile(file);
#if qDebug
		if (!IsObject(file))
			ProgramBreak("NewPrefsLocationFile obj is not object");
#endif
#if qDebugNewPrefsLocationFile
		CStr255 s(file->fFileSpec.name);
		CStr255 pathName;
		extern void GetPathNameFromDirID(short vRefNum, long dirID, CStr255 &pathName);
		GetPathNameFromDirID(file->fFileSpec.vRefNum, file->fFileSpec.parID, pathName);
		fprintf(stderr, "Name of NewPrefsLocationFile file: ");
		fprintf(stderr, "path = '%s', ", (char*) pathName);
		fprintf(stderr, "name = '%s'\n", (char*)s);
#endif
		fi.Success();
	}
	else // fail
		fi.ReSignal();
}
