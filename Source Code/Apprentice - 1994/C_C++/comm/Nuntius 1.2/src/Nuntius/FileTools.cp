// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// FileTools.cp

#include "FileTools.h"
#include "Tools.h"
#include "UThread.h"

#include <RsrcGlobals.h>

#include <Errors.h>
#include <Packages.h>

#pragma segment MyTools

#define qDebugAsyncFile qDebug & 0

Boolean FileExist(const FSSpec &spec)
{
	FInfo fndrInfo;
	short errNo = HGetFInfo(spec.vRefNum, spec.parID, spec.name, fndrInfo);
	if (errNo == fnfErr) 
		return false;
	FailOSErr(errNo);
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

void ResolveAliasFile(TFile *file)
{
	Boolean targetIsFolder, wasAliased;
	FSSpec spec;
	file->GetFileSpec(spec);
	OSErr err = ResolveAliasFile(spec, true, targetIsFolder, wasAliased);
	if (wasAliased)
		file->Specify(spec);
	if (err != fnfErr)
		FailOSErr(err);
}

void MyRenameFile(TFile *file, const CStr255 &newname)
{
	file->RenameFile(newname);
	CopyCString2String(newname, file->fFileSpec.name); // RenameFile() doesn't update this
}

void CheckFilenameSanity(CStr255 &filename)
{
// substitute invalid chars
	unsigned char ch;
	if (gConfiguration.hasAUX)
	{
		for (short i = filename.Length(); i; i--)
		{
			ch = (unsigned char)filename[i];
			if (ch == '/' || ch == '.')
				filename[i] = '_';
			else if (ch < 32)
				filename.Delete(i, 1);
		}
	}
	else // HFS (I presume!)
	{
		for (short i = filename.Length(); i; i--)
		{
			ch = (unsigned char)filename[i];
			if (ch == ':' || ch < 32)
				filename[i] = '-';
		}
		if (filename.Length() && filename[1] == '.')
			filename[1] = '¥';
	}
	if (filename.Length() > 31) // dunno with A/UX
		filename.Length() = 31;
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
		FlushVol(&name, vRefNum);
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
	errNo = ResolveAliasFile(spec, true, targetIsFolder, wasAliased);
	if (errNo != fnfErr)
		FailOSErr(errNo);
	CInfoPBRec info;
	info.dirInfo.ioCompletion = nil;
	info.dirInfo.ioNamePtr = StringPtr(spec.name);
	info.dirInfo.ioVRefNum = spec.vRefNum;
	info.dirInfo.ioFDirIndex = 0;
	info.dirInfo.ioDrDirID = spec.parID;
	errNo = PBGetCatInfoSync(&info);
	if (errNo == fnfErr)
	{
		long createdDirID;
		FailOSErr(FSpDirCreate(spec, smRoman, createdDirID));
		spec.parID = createdDirID;
	}
	else
	{
		FailOSErr(errNo);
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
	CStr255 directoryName = "";
	pathName = "";
	block.dirInfo.ioNamePtr = StringPtr(&directoryName);
	block.dirInfo.ioDrParID = dirID;
	do
	{
		block.dirInfo.ioVRefNum = vRefNum;
		block.dirInfo.ioFDirIndex = -1;
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
		short err = PBGetCatInfoSync(&block);
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


void ChangeFileTypeAndCreator(TFile *file, OSType newCreator, OSType newType)
{
	FInfo fndrInfo;
	FailOSErr(file->GetFinderInfo(fndrInfo));
	fndrInfo.fdType = newType;
	fndrInfo.fdCreator = newCreator;
	FSSpec theFileSpec = file->fFileSpec;					
	FailOSErr(FSpSetFInfo(theFileSpec, fndrInfo));
// modify folder so Finderª updates the display
	CInfoPBRec info;
	info.dirInfo.ioFDirIndex = -1;
	info.dirInfo.ioDrDirID = file->fFileSpec.parID;
	info.dirInfo.ioVRefNum = file->fFileSpec.vRefNum;
	info.dirInfo.ioNamePtr = nil;
	FailOSErr(PBGetCatInfoSync(&info));
	GetDateTime(info.dirInfo.ioDrMdDat);
	short errNo = PBSetCatInfoSync(&info);
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
	CustomPutFile(prompt, defaultFilename, customReply, sfPutDialogID, CPoint(-1, -1), nil, gModalFilterYDProcPtr, nil, nil, nil);
	if (!customReply.sfGood)
		Failure(0, 0);
	spec = customReply.sfFile;
}


void WriteASyncToFile(ParamBlockRec &pb, TFile *file, const void *p, long numBytes)
{
	Ptr fromP = Ptr(p);
	while (numBytes > 0)
	{
		BlockSet(Ptr(&pb), sizeof(pb), 0);
		pb.ioParam.ioRefNum = file->fDataRefNum;
		pb.ioParam.ioBuffer = fromP;
		pb.ioParam.ioReqCount = numBytes;
		pb.ioParam.ioPosMode = fsFromMark;
		pb.ioParam.ioPosOffset = 0;
		FailOSErr(PBWriteAsync(&pb));
		if (pb.ioParam.ioResult == 1)
		{
#if qDebugAsyncFile
			fprintf(stderr, "Waiting for async file write (%ld bytes)\n", numBytes);
#endif
			while (pb.ioParam.ioResult == 1)
				gCurThread->YieldTime();
#if qDebugAsyncFile
			fprintf(stderr, "Async file write done\n");
#endif
		}
		FailOSErr(pb.ioParam.ioResult);
		numBytes -= pb.ioParam.ioActCount;
		fromP += pb.ioParam.ioActCount;
	}
}

void ReadASyncFromFile(ParamBlockRec &pb, TFile *file, void *p, long &numBytes)
{
	if (numBytes <= 0)
		return;
	BlockSet(Ptr(&pb), sizeof(pb), 0);
	pb.ioParam.ioRefNum = file->fDataRefNum;
	pb.ioParam.ioBuffer = Ptr(p);
	pb.ioParam.ioReqCount = numBytes;
	pb.ioParam.ioPosMode = fsFromMark;
	pb.ioParam.ioPosOffset = 0;
	OSErr err = PBReadAsync(&pb);
	if (err != eofErr)
		FailOSErr(err);
	if (pb.ioParam.ioResult == 1)
	{
#if qDebugAsyncFile
		fprintf(stderr, "Waiting for async file read (%ld bytes)\n", numBytes);
#endif
		while (pb.ioParam.ioResult == 1)
			gCurThread->YieldTime();
#if qDebugAsyncFile
		fprintf(stderr, "Async file read done (%ld)\n", pb.ioParam.ioActCount);
#endif
	}
	if (pb.ioParam.ioResult != eofErr || pb.ioParam.ioActCount == 0)
		FailOSErr(pb.ioParam.ioResult);
	numBytes = pb.ioParam.ioActCount;
}
