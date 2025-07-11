/*
 * CFlexiDataFile.h
 *
 * A subclass of CDataFile that can pretend that a resource,
 * a handle, or a section of memory is actually a file on disk.
 *
 * � Copyright 1992 by Jamie R. McCarthy.  All rights reserved.
 * This code can be both distributed and used freely.
 * Internet: k044477@kzoo.edu			AppleLink: j.mccarthy
 * Telephone:  800-421-4157 or US 616-665-7075 (9:00-5:00 Eastern time)
 * I'm releasing this code with the hope that someone will get something
 * out of it.  Feedback of any sort, even just letting me know that you're
 * using it, is greatly appreciated!
 *
 * Read the comment at the beginning of CFlexiDataFile.c.
 *
 */



/********************************/

#pragma once

/********************************/

#include <CDataFile.h>

/********************************/

enum {
	kModeUnspecified,
	kModeDataFork,
	kModeResource,
	kModeHandle,
	kModeMemory
} ;

/********************************/

	/*
	 * Here's an added bonus for you.  I stuck this in because it makes
	 * it easy to integrate an open-a-resource-as-a-file dialog with the
	 * TCL.  If you're not using the TCL, you can skip this comment.
	 *
	 * You can munge data in a SFReply record and pass it to a CFlexiDataFile,
	 * to indicate to it that it should open, not a regular file, but a
	 * resource or a handle.  For example, if you want to present an "open a
	 * resource as a file" dialog instead of the standard file dialog, just
	 * override CApplication::ChooseFile to present your dialog.  Then put
	 * the ResType and ID chosen into the appropriate places in the SFReply
	 * record, set the version field to kResFileFlag, and return.
	 * CApplication will cheerfully pass the munged SFReply to
	 * CYourApp::OpenDocument, which will cheerfully create a doc and pass
	 * it to CYourDoc::OpenFile.  CYourDoc::OpenFile will cheerfully create
	 * a CFlexiDataFile and pass the same munged SFReply to it, the
	 * CFlexiDataFile will recognize the flag and open the resource as a
	 * file, and no one will be any the wiser.
	 *
	 * To open a resource as a data file, set the version field to
	 * kResFileFlag, the fType field to the resource's type, and the
	 * vRefNum field to the resource's ID.
	 *
	 * To open a handle as a data file, set the version field to
	 * kHndlFileFlag and the fType field to the handle.
	 *
	 * To open a section of memory as a data file, set the version
	 * field to kMemoryFileFlag, the fType field to the handle, and
	 * the vRefNum field to the length.  (Sorry--if you want to
	 * access more than 64K, you'll have to really use
	 * memorySpecify().  There's no more room in the SFReply record.)
	 *
	 * I really have to stretch to come up with a real-world situation
	 * in which you'd want the user to be able to specify handles or
	 * sections of memory to operate on.  But letting the user specify
	 * a resource is definitely useful, and it's easy to let the
	 * other two tag along.
	 * 
	 */
	
enum {
	kResFileFlag = -999,			// Set aSFReply->version to this to indicate a resource
	kHndlFileFlag,					// Set aSFReply->version to this to indicate a Handle
	kMemoryFileFlag				// Set aSFReply->version to this to indicate a section of memory
} ;

/********************************/



class CFlexiDataFile : public CDataFile {
	
public:
	
	void			IFlexiDataFile(void);
	void			Dispose(void);
	
	void			Open(SignedByte permission);						// override
	void			Close(void);											// override
	
	Boolean		IsOpen(void);											// not an override!
	
	void			Specify(Str63 aName, short aVolNum);			// override
	void			SpecifyFSSpec(const FSSpec *aFileSpec);		// override
	void			SFSpecify(SFReply *macSFReply);					// override
	void			ResolveFileAlias(void);								// override
	void			rsrcSpecify(ResType theRsrcType, short theRsrcID);
	void			hndlSpecify(Handle theHndl);
	void			memorySpecify(void *thePointer, long theLength);
	
	void			GetName(Str63 theName);								// override
	void			GetFSSpec(FSSpec *aFileSpec);						// override
	
		/*
		 * These two methods could be implemented for resources and handles;
		 * actually, it'd be kinda cool to be able to call CreateNew()
		 * instead of AddResource().  But I don't need them, so I haven't
		 * gotten around to making either of these work for resources and
		 * handles.
		 */
	void			CreateNew(OSType creator, OSType fType);		// override
	void			ThrowOut(void);										// override
	
	void			ChangeName(Str63 newName);							// override
	Boolean		ExistsOnDisk(void);									// override
	void			GetMacFileInfo(FInfo *fileInfo);					// override

	void			SetLength(long aLength);							// override
	long			GetLength(void);										// override
	void			SetMark(long howFar, short fromWhere);			// override
	long			GetMark(void);											// override
	
	Handle		ReadAll(void);											// override
	void			ReadSome(Ptr info, long howMuch);				// override
	void			WriteAll(Handle contents);							// override
	void			WriteSome(Ptr info, long howMuch);				// override
	
	short			getMode(void);
	
	
protected:
	
	short					itsPermission;
	
	short					itsMode;
	
		/* Used when itsMode is either kModeResource or kModeHandle. */
	Handle				itsDataHndl;
		/* Used only when itsMode is kModeMemory. */
	void					*itsDataPointer;
	long					itsDataLength;
	
		/* Used when itsMode is kModeResource, kModeHandle, or kModeMemory. */
	long					itsDataMark;
	
		/* Used only when itsMode is kModeHandle. */
	short					itsOldHState;
	
		/* Used only when itsMode is kModeResource. */
	ResType				itsRsrcType;
	short					itsRsrcID;
	
	
		/* Raises an exception if the file has read-only permission. */
	void			testWritePermission(void);
	
		/*
		 * Raises an exception if the file is open in memory mode, and the
		 * section of memory specified by itsDataPointer/itsDataLength is
		 * invalid.
		 */
	void			testMemoryValidity(void);
	
	
} ;
