// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UResFile.h

#ifndef __RESOURCES__
#include <Resources.h>
#endif

#ifndef __ERRORS__
#include <Errors.h>
#endif

#include "UResFile.h"

#pragma segment Nuntius

TResFile::TResFile()
{
}

pascal void TResFile::Initialize()
{
	inherited::Initialize();
}

pascal void TResFile::IResFile(OSType itsFileType, OSType itsCreator, 
								Boolean usesDataFork, ResourceUsage usesRsrcFork,
							  Boolean keepsDataOpen, Boolean keepsRsrcOpen)
{
	inherited::IFile(itsFileType, itsCreator, 
						usesDataFork, usesRsrcFork, keepsDataOpen, keepsRsrcOpen);
}

pascal void TResFile::Free()
{
	inherited::Free(); // closes file
}

pascal OSErr TResFile::CloseRsrcFork()
{
	short errNo = inherited::CloseRsrcFork();
	FlushVolume();
	return errNo;
}
	
void TResFile::UpdateFile()
{
	FailInfo fi;
	short oldRefNum = UseResource();
	VOLATILE(oldRefNum);
	if (fi.Try()) {
		if (IsRsrcForkOpen()) {
			UpdateResource();
			FlushVolume();
		}
		fi.Success();
	}
	else { // fail
		UseResFile(oldRefNum);
		fi.ReSignal();
	}
}

Handle TResFile::GetResource(ResType theType, short theID)
{
	FailInfo fi;
	short oldRefNum = UseResource();
	VOLATILE(oldRefNum);
	if (fi.Try()) {
		Handle H = ::Get1Resource(theType, theID);
		FailNILResource(H);
		UseResFile(oldRefNum);
		fi.Success();
		return H;
	}
	else { // fail
		UseResFile(oldRefNum);
		fi.ReSignal();
	}
}

Handle TResFile::GetRes(ResType theType, ConstCStr255Param name)
{
	FailInfo fi;
	short oldRefNum = UseResource();
	VOLATILE(oldRefNum);
	if (fi.Try()) {
		Handle H = ::Get1NamedResource(theType, name);
		FailNILResource(H);
		UseResFile(oldRefNum);
		fi.Success();
		return H;
	}
	else { // fail
		UseResFile(oldRefNum);
		fi.ReSignal();
	}
}

Boolean TResFile::GetResBool(ResType theType, ConstCStr255Param name, Handle &H)
{
	FailInfo fi;
	short oldRefNum = UseResource();
	VOLATILE(oldRefNum);
	if (fi.Try()) {
		H = ::Get1NamedResource(theType, name);
		short errNo = ResError();
		Boolean gotIt;
		if (H == nil && (errNo == noErr || errNo == resNotFound)) {
			gotIt = false;
		}
		else {
			FailNILResource(H);
			gotIt = true;
		}
		UseResFile(oldRefNum);
		fi.Success();
		return gotIt;
	}
	else { // fail
		UseResFile(oldRefNum);
		fi.ReSignal();
	}
}

Boolean TResFile::GetResourceBool(ResType theType, short theID, Handle &H)
{
	FailInfo fi;
	short oldRefNum = UseResource();
	VOLATILE(oldRefNum);
	if (fi.Try()) {
		H = ::Get1Resource(theType, theID);
		short errNo = ResError();
		Boolean gotIt;
		if (H == nil && (errNo == noErr || errNo == resNotFound)) {
			gotIt = false;
		}
		else {
			FailNILResource(H);
			gotIt = true;
		}
		UseResFile(oldRefNum);
		fi.Success();
		return gotIt;
	}
	else { // fail
		UseResFile(oldRefNum);
		fi.ReSignal();
	}
}


void TResFile::RemoveRes(ResType theType, ConstCStr255Param name)
{
	FailInfo fi;
	short oldRefNum = UseResource();
	VOLATILE(oldRefNum);
	if (fi.Try()) {
		::SetResLoad(false);
		Handle H = ::Get1NamedResource(theType, name);
		short errNo = ResError();
		::SetResLoad(true);
		if (H == nil && (errNo == noErr || errNo == resNotFound)) {
			// did not exist
		}
		else {
			FailNILResource(H);
			RmveResource(H); // H _must_ belong to current CurResFile (se IM: The ResourceMgr)
		}
		UseResFile(oldRefNum);
		fi.Success();
	}
	else { // fail
		UseResFile(oldRefNum);
		fi.ReSignal();
	}
}

Boolean TResFile::ResExist(ResType theType, short theID)
{
	FailInfo fi;
	short oldRefNum = UseResource();
	VOLATILE(oldRefNum);
	if (fi.Try()) {
		::SetResLoad(false);
		Handle H = ::Get1Resource(theType, theID);
		short errNo = ResError();
		::SetResLoad(true);
		Boolean doesExist;
		if (H == nil && (errNo == noErr || errNo == resNotFound)) {
			doesExist = false;
		}
		else {
			FailNILResource(H);
			doesExist = true;
		}
		UseResFile(oldRefNum);
		fi.Success();
		return doesExist;
	}
	else { // fail
		UseResFile(oldRefNum);
		fi.ReSignal();
	}
}

Boolean TResFile::NamedResExist(ResType theType, ConstCStr255Param name)
{
	FailInfo fi;
	short oldRefNum = UseResource();
	VOLATILE(oldRefNum);
	if (fi.Try()) {
		::SetResLoad(false);
		Handle H = ::Get1NamedResource(theType, name);
		short errNo = ResError();
		::SetResLoad(true);
		Boolean doesExist;
		if (H == nil && (errNo == noErr || errNo == resNotFound)) {
			doesExist = false;
		}
		else {
			FailNILResource(H);
			doesExist = true;
		}
		UseResFile(oldRefNum);
		fi.Success();
		return doesExist;
	}
	else { // fail
		UseResFile(oldRefNum);
		fi.ReSignal();
	}
}

void TResFile::AddResource(void *theResource, ResType theType, short theID, ConstCStr255Param name)
{
	FailInfo fi;
	short oldRefNum = UseResource();
	VOLATILE(oldRefNum);
	if (fi.Try()) {
		::AddResource(Handle(theResource), theType, theID, name);
		FailResError();
		::WriteResource(Handle(theResource));
		FailResError();
		UseResFile(oldRefNum);
		fi.Success();
	}
	else {
		UseResFile(oldRefNum);
		fi.ReSignal();
	}
}

void TResFile::AddRes(void *theResource, ResType theType, ConstCStr255Param name)
{
	short id;
	do id = UniqueID(theType); while (id < 1024);
	AddResource(theResource, theType, id, name);
}

void TResFile::UpdateRes(void *theResource)
{
	::ChangedResource(Handle(theResource));
	FailResError();
	::WriteResource(Handle(theResource));
	FailResError();
}	

void TResFile::ReleaseResource(void *theResource)
{
	::ReleaseResource(Handle(theResource));
	FailResError();
}

void TResFile::DetachResource(void *theResource)
{
	::DetachResource(Handle(theResource));
	FailResError();
}

void TResFile::RemoveAllResources()
{
	FailInfo fi;
	short oldRefNum = UseResource();
	VOLATILE(oldRefNum);
	if (fi.Try()) {
		// Apple still have not supplied a function to this!
		// so we remove _one_ resource at a time	
		while (Count1Types() > 0) {
			ResType theType;
			Get1IndType(theType, 1);
			while (Count1Resources(theType) > 0) {
				Handle H;
				SetResLoad(false);
				H = Get1IndResource(theType, 1);
				short err = ResError();
				SetResLoad(true);
				FailNILResource(H);
				RmveResource(H);
				FailResError();
			}
		}
		UseResFile(oldRefNum);
		fi.Success();
	}
	else {
		UseResFile(oldRefNum);
		fi.ReSignal();
	}
}

short TResFile::UniqueID(ResType theType)
{
	short id;
	do 
	{
		id = UniqueID(theType); 
	} while (id < 1024);
	return id;
}
	
void TResFile::RemoveResource(ResType theType, short theID)
{
	FailInfo fi;
	short oldRefNum = UseResource();
	VOLATILE(oldRefNum);
	if (fi.Try()) {
		SetResLoad(false);
		Handle H = ::Get1Resource(theType, theID);
		SetResLoad(true);
		if (H == nil && ResError() != noErr && ResError() != resNotFound)
		{
			FailNILResource(H);
			::RmveResource(H);
			DisposeHandle(H);
			H = nil;
		}
		UseResFile(oldRefNum);
		fi.Success();
		return;
	}
	else { // fail
		UseResFile(oldRefNum);
		fi.ReSignal();
	}
}
