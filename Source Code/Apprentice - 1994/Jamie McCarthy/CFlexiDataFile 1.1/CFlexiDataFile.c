/*
 * CFlexiDataFile.c
 *
 * A subclass of CDataFile that can pretend that a resource,
 * a handle, or a section of memory is actually a file on disk.
 *
 * © Copyright 1992 by Jamie R. McCarthy.  All rights reserved.
 * This code can be both distributed and used freely.
 * Internet: k044477@kzoo.edu			AppleLink: j.mccarthy
 * Telephone:  800-421-4157 or US 616-665-7075 (9:00-5:00 Eastern time)
 * I'm releasing this code with the hope that someone will get something
 * out of it.  Feedback of any sort, even just letting me know that
 * you're using it, is greatly appreciated!
 *
 * This version (Nov 92) doesn't support changing the size of
 * resources.  Note that writing past the end of a resource will
 * attempt to change its size (see WriteSome()).  As soon as
 * JPartialResources gets that code written, this class will work.
 * (Don't hold your breath.)
 *
 * CAVEAT HACKER:  the writing code (WriteSome() and especially
 * WriteAll()) is completely untested.  Please don't make your
 * un-backed-up master's thesis into my beta-test site;  step
 * through the code with the debugger until you're confident that
 * it works.  (And write me to let me know what you find.)
 *
 * CAVEAT HACKER:  using memory mode will give you a
 * wonderful high-level object-oriented interface to something
 * that's really as low-level as you can go.  What that means is,
 * you can crash your machine in a truly spectacular fashion
 * without even trying.  For example, it's a bad idea to assign
 * a CFlexiDataFile to a section of memory in a handle that might
 * move out from underneath it.  Another thing that you probably
 * don't want to do, is to have WriteSome() or WriteAll() go past
 * the end of your section of memory.  The code, as it stands,
 * catches that and raises an exception.  If you actually do
 * want to do that, change the #definition of the constant
 * "allowWriteToImplicitlyChangeMemoryLength".
 *
 */



/********************************/

#include "CFlexiDataFile.h"

/********************************/

#include <TCLUtilities.h>

#include "JPartialResources.h"

/********************************/

enum {
	kUnopenedDataMark = -1
} ;

/********************************/

#define allowWriteToImplicitlyChangeMemoryLength (0)

/********************************/



void CFlexiDataFile::IFlexiDataFile(void)
{
	inherited::IDataFile();
	
	itsMode = kModeUnspecified;
	itsDataHndl = NULL;
	itsDataPointer = NULL;
	itsDataMark = kUnopenedDataMark;
}



void CFlexiDataFile::Dispose(void)
{
	inherited::Dispose();
}



void CFlexiDataFile::Open(SignedByte permission)
{
	ASSERT(!IsOpen());
	
	switch (itsMode) {
		
		case kModeResource: {
			Boolean oldResLoad;
			oldResLoad = ResLoad;
			SetResLoad(FALSE);
			itsDataHndl = GetResource(itsRsrcType, itsRsrcID);
			SetResLoad(oldResLoad);
			if (itsDataHndl == NULL) {
				FailOSErr(resNotFound);
			}
			itsDataMark = 0;
		}	break;
			
		case kModeHandle: {
			ASSERT(itsDataHndl != NULL);
			ASSERT(*itsDataHndl != NULL);
			itsDataMark = 0;
		}	break;
			
		case kModeMemory: {
			ASSERT(itsDataPointer != NULL);
			itsDataMark = 0;
		}	break;
			
		case kModeDataFork: {
			inherited::Open(permission);
		}	break;
			
		default:
			FailOSErr(paramErr);
			break;
			
	}
	
	itsPermission = permission;
}



void CFlexiDataFile::Close(void)
{
	if (!IsOpen()) return;
	
	switch (itsMode) {
		
		case kModeResource:
			if (*itsDataHndl != NULL) {
				ForgetResource(itsDataHndl);
			} else {
				DetachResource(itsDataHndl);
				DisposHandle(itsDataHndl);
			}
			itsDataHndl = NULL;
			itsDataMark = kUnopenedDataMark;
			break;
			
		case kModeHandle: {
			HSetState(itsDataHndl, itsOldHState);
			itsDataHndl = NULL;
			itsDataMark = kUnopenedDataMark;
		} 	break;
			
		case kModeMemory: {
			itsDataPointer = NULL;
			itsDataMark = kUnopenedDataMark;
		} 	break;
			
		case kModeDataFork:
			inherited::Close();
			break;
			
		default:
			break;
			
	}
}



Boolean CFlexiDataFile::IsOpen(void)
{
	switch (itsMode) {
		case kModeUnspecified:	return FALSE;											break;
		case kModeDataFork:		return (refNum!=0);									break;
		case kModeResource:
		case kModeHandle:
		case kModeMemory:			return (itsDataMark != kUnopenedDataMark);	break;
		default:						Debugger();												break;
	}
}



/********************************/



void CFlexiDataFile::Specify(Str63 aName, short aVolNum)
{
	ASSERT(!IsOpen());
	itsMode = kModeDataFork;
	inherited::Specify(aName, aVolNum);
}



void CFlexiDataFile::SpecifyFSSpec(const FSSpec *aFileSpec)
{
	ASSERT(!IsOpen());
	itsMode = kModeDataFork;
	inherited::SpecifyFSSpec(aFileSpec);
}



void CFlexiDataFile::SFSpecify(SFReply *macSFReply)
{
	ASSERT(!IsOpen());
	
		/*
		 * Here's the code that checks for the munged SFReply record.
		 */
		
	if (macSFReply->version == kResFileFlag) {
		
		rsrcSpecify(macSFReply->fType, macSFReply->vRefNum);
		
	} else if (macSFReply->version == kHndlFileFlag) {
		
		hndlSpecify((Handle) macSFReply->fType);
		
	} else if (macSFReply->version == kMemoryFileFlag) {
		
		memorySpecify((void*) macSFReply->fType, (unsigned short) macSFReply->vRefNum);
		
	} else {
		
		inherited::SFSpecify(macSFReply);
		
	}
}



void CFlexiDataFile::ResolveFileAlias(void)
{
		/*
		 * Resources, handles, and sections of memory can't be aliases,
		 * so there's nothing to resolve.  However, if you want (e.g.)
		 * a resource to be able to resolve to (e.g.) a file on disk,
		 * all you have to do is override this method--rsrcSpecify(),
		 * hndlSpecify(), and memorySpecify() do indeed call
		 * ResolveFileAlias().
		 */
	
	if (itsMode == kModeDataFork) {
		inherited::ResolveFileAlias();
	}
}



void CFlexiDataFile::rsrcSpecify(ResType theRsrcType, short theRsrcID)
{
	ASSERT(!IsOpen());
	
	itsMode = kModeResource;
	
		/*
		 * The handle will be loaded in when the file is opened.
		 */
		
	itsDataHndl = NULL;
	itsDataMark = kUnopenedDataMark;
	
	itsRsrcType = theRsrcType;
	itsRsrcID = theRsrcID;
	
	ResolveFileAlias();
}



void CFlexiDataFile::hndlSpecify(Handle theHndl)
{
	ASSERT(!IsOpen());
	
	itsMode = kModeHandle;
	
	itsDataHndl = theHndl;
	itsDataMark = kUnopenedDataMark;
	
	ASSERT(theHndl != NULL && *theHndl != NULL);
	itsOldHState = HGetState(theHndl);
	HNoPurge(theHndl);
	
	ResolveFileAlias();
}



void CFlexiDataFile::memorySpecify(void *thePointer, long theLength)
{
	ASSERT(!IsOpen());
	
	itsMode = kModeMemory;
	
	itsDataPointer = thePointer;
	itsDataLength = theLength;
	itsDataMark = kUnopenedDataMark;
	
	ASSERT(thePointer != NULL);
	
	ResolveFileAlias();
}



/********************************/



void CFlexiDataFile::GetName(Str63 theName)
{
	switch (itsMode) {
		
		case kModeResource: {
			short dummyID;
			ResType dummyResType;
			ASSERT(itsDataHndl != NULL);
			GetResInfo(itsDataHndl, &dummyID, &dummyResType, theName);
		} break;
			
		case kModeHandle:
			theName[0] = 0;		// Handles don't have names
			break;
			
		case kModeMemory:
			theName[0] = 0;		// Sections of memory don't have names
			break;
			
		default:
			inherited::GetName(theName);
			break;
			
	}
}



void CFlexiDataFile::GetFSSpec(FSSpec *aFileSpec)
{
	if (itsMode == kModeDataFork) {
		inherited::GetFSSpec(aFileSpec);
	} else {
		Failure(paramErr, 0);
	}
}



/********************************/



void CFlexiDataFile::CreateNew(OSType creator, OSType fType)
{
		/* See comments in the .h file on this method and ThrowOut(). */
	ASSERT(itsMode == kModeDataFork);
	inherited::CreateNew(creator, fType);
}



void CFlexiDataFile::ThrowOut(void)
{
		/*
		 * Apparently, HDelete() moves memory--or at least, it has for me.
		 * So the object needs to be locked down when it's called.  Here's
		 * as good a place as any to do it.
		 */
		
	Boolean wasLocked;
	ASSERT(itsMode == kModeDataFork);
	wasLocked = Lock(TRUE);
	inherited::ThrowOut();
	Lock(wasLocked);
}



/********************************/



void CFlexiDataFile::ChangeName(Str63 newName)
{
	if (itsMode == kModeDataFork) {
		inherited::ChangeName(newName);
	} else {
		Failure(paramErr, 0);
	}
}



Boolean CFlexiDataFile::ExistsOnDisk(void)
{
	if (itsMode == kModeDataFork) {
		return inherited::ExistsOnDisk();
	} else {
		Failure(paramErr, 0);
	}
}



void CFlexiDataFile::GetMacFileInfo(FInfo *fileInfo)
{
	if (itsMode == kModeDataFork) {
		inherited::GetMacFileInfo(fileInfo);
	} else {
		Failure(paramErr, 0);
	}
}



/********************************/



void CFlexiDataFile::SetLength(long aLength)
{
	ASSERT(IsOpen());
	
	testWritePermission();
	
	switch (itsMode) {
		
		case kModeResource:
			jSetResourceSize(itsDataHndl, aLength);
			FailResError();
			break;
			
		case kModeHandle:
			SetHandleSize(itsDataHndl, aLength);
			FailMemError();
			break;
			
		case kModeMemory:
			itsDataLength = aLength;
			testMemoryValidity();
			break;
			
		default:
			inherited::SetLength(aLength);
			break;
			
	}
}



long CFlexiDataFile::GetLength(void)
{
	ASSERT(IsOpen());
	
	switch (itsMode) {
		
		case kModeResource:
			return SizeResource(itsDataHndl);
			break;
			
		case kModeHandle:
			return GetHandleSize(itsDataHndl);
			break;
			
		case kModeMemory:
			return itsDataLength;
			break;
			
		default:
			return inherited::GetLength();
			break;
			
	}
}



void CFlexiDataFile::SetMark(long howFar, short fromWhere)
{
	ASSERT(IsOpen());
	
	switch (itsMode) {
		
		case kModeResource:
		case kModeHandle:
		case kModeMemory:
			switch (fromWhere) {
				case fsFromStart:		itsDataMark = howFar;						break;
				case fsFromLEOF:		itsDataMark = GetLength() - howFar;		break;
				case fsFromMark:		itsDataMark += howFar;						break;
				default:					FailOSErr(paramErr);							break;
			}
			break;
			
		default:
			inherited::SetMark(howFar, fromWhere);
			break;
			
	}
}



long CFlexiDataFile::GetMark(void)
{
	ASSERT(IsOpen());
	
	switch (itsMode) {
		
		case kModeResource:
		case kModeHandle:
		case kModeMemory:
			return itsDataMark;
			break;
			
		default:
			inherited::GetMark();
			break;
			
	}
}



Handle CFlexiDataFile::ReadAll(void)
{
	ASSERT(IsOpen());
	
	switch (itsMode) {
		
		case kModeResource: {
			
			short oldResLoad;
			Handle theReturnHndl;
			
			oldResLoad = ResLoad;
			theReturnHndl = itsDataHndl;
			LoadResource(theReturnHndl);
			DetachResource(theReturnHndl);
			
			SetResLoad(FALSE);
			itsDataHndl = GetResource(itsRsrcType, itsRsrcID);
			SetResLoad(oldResLoad);
			
			return theReturnHndl;
			
		}	break;
			
		case kModeHandle: {
			
			Handle copyOfDataHndl;
			copyOfDataHndl = itsDataHndl;
			HandToHand(&copyOfDataHndl);
			return copyOfDataHndl;
			
		}	break;
			
		case kModeMemory: {
			
			Handle copyOfData;
			copyOfData = NewHandle(itsDataLength);
			if (copyOfData != NULL) {
				BlockMove(itsDataPointer, *copyOfData, itsDataLength);
			}
			return copyOfData;
			
		}	break;
			
		default:
			return inherited::ReadAll();
			break;
			
	}
}



void CFlexiDataFile::ReadSome(Ptr info, long howMuch)
{
	ASSERT(IsOpen());
	
	switch (itsMode) {
		
		case kModeResource: {
			ASSERT(itsDataMark + howMuch <= GetLength());
			jReadPartialResource(itsDataHndl, itsDataMark, info, howMuch);
			itsDataMark += howMuch;
		}	break;
			
		case kModeHandle: {
			ASSERT(itsDataMark + howMuch <= GetLength());
			BlockMove(*itsDataHndl + itsDataMark, info, howMuch);
			itsDataMark += howMuch;
		}	break;
			
		case kModeMemory: {
			ASSERT(itsDataMark + howMuch <= itsDataLength);
			BlockMove((Ptr) itsDataPointer + itsDataMark, info, howMuch);
			itsDataMark += howMuch;
		}	break;
			
		default:
			inherited::ReadSome(info, howMuch);
			break;
			
	}
}



void CFlexiDataFile::WriteAll(Handle contents)
{
	ASSERT(IsOpen());
	
	testWritePermission();
	
	switch (itsMode) {
		
		case kModeResource: {
			Str255 theName;
			ASSERT(itsDataHndl != NULL);
			SetMark(0, fsFromStart);
			GetName(theName);
			RmveResource(itsDataHndl);
			ForgetHandle(itsDataHndl);
			TRY {
				HandToHand(&contents);
				FailMemError();
				TRY {
					AddResource(contents, itsRsrcType, itsRsrcID, theName);
					FailResError();
					itsDataHndl = contents;
				} CATCH {
						// couldn't add the resource--forget the allocation
					ForgetHandle(contents);
				} ENDTRY;
			} CATCH {
					// couldn't write--re-open the file
				Open(fsRdWrPerm);
			} ENDTRY;
		}	break;
			
		case kModeHandle:
			ForgetHandle(itsDataHndl);
			TRY {
				HandToHand(&contents);
				FailMemError();
				itsDataHndl = contents;
			} CATCH {
					// couldn't dupe the handle--close the file.
				itsDataMark = kUnopenedDataMark;
			} ENDTRY;
			break;
			
		case kModeMemory:
#if allowWriteToImplicitlyChangeMemoryLength
			SetLength(GetHandleSize(contents));
#else
			ASSERT(GetLength() == GetHandleSize(contents));
#endif
			SetMark(0, fsFromStart);
			BlockMove(*contents, itsDataPointer, itsDataLength);
			break;
			
		default:
			inherited::WriteAll(contents);
			break;
			
	}
}



void CFlexiDataFile::WriteSome(Ptr info, long howMuch)
{
	ASSERT(IsOpen());
	
	testWritePermission();
	
		/*
		 * This is necessary for writing to resources and handles, so
		 * you don't write data over a resource map or BlockMove() past
		 * the end of a handle.  (Well, actually, jWritePartialResource
		 * will call jSetResourceSize for you, but calling it here
		 * doesn't hurt.  And it is necessary for kModeHandle.  And
		 * jSetResourceSize doesn't work yet anyway, so all this is
		 * hypothetical discussion, sigh...)
		 *
		 * As far as I know, this has no effect on writing to disk:
		 * FSWrite sets the eof as far ahead as it needs to, right?
		 */
	
#if allowWriteToImplicitlyChangeMemoryLength
	if (itsMode == kModeMemory) {
		ASSERT(GetMark() + howMuch <= GetLength());
	}
#endif
	
	if (GetMark() + howMuch > GetLength()) {
		SetLength(GetMark() + howMuch);
	}
	ASSERT(GetMark() + howMuch <= GetLength());
	
	switch (itsMode) {
		
		case kModeResource:
			jWritePartialResource(itsDataHndl, itsDataMark, info, howMuch);
			itsDataMark += howMuch;
			break;
			
		case kModeHandle:
			BlockMove(info, *itsDataHndl + itsDataMark, howMuch);
			itsDataMark += howMuch;
			break;
			
		case kModeMemory:
			BlockMove(info, (Ptr) itsDataPointer + itsDataMark, howMuch);
			itsDataMark += howMuch;
			break;
			
		default:
			inherited::WriteSome(info, howMuch);
			break;
			
	}
}



/********************************/



short CFlexiDataFile::getMode(void)
{
	return itsMode;
}



void CFlexiDataFile::testWritePermission(void)
{
	if (itsPermission != fsRdWrPerm) {
		FailOSErr(wrPermErr);
	}
}



void CFlexiDataFile::testMemoryValidity(void)
{
	// I don't really know what to do here...
	ASSERT(itsDataPointer <= (void*) 0x20000000);
	ASSERT((Ptr) itsDataPointer + itsDataMark <= (Ptr) 0x20000000);
}
