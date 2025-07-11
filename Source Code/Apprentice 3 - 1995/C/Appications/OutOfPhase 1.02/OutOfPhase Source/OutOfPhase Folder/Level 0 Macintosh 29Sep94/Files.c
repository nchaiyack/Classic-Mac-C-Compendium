/* Files.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Debug.h"
#include "Audit.h"
#include "Definitions.h"

#ifdef THINK_C
	#pragma options(pack_enums)
#endif
#include <Files.h>
#include <Folders.h>
#include <Finder.h>
#include <Script.h>
#include <Events.h>
#include <Errors.h>
#include <StandardFile.h>
#ifdef THINK_C
	#pragma options(!pack_enums)
#endif

#include "Files.h"
#include "Memory.h"
#include "Array.h"


/* open file abstraction.  for efficiency, we ought to be having buffers */
/* and all kinds of stuff in here, but we're too lazy to implement it. */
struct FileType
	{
		/* refnum for Macintosh file system OS calls */
		short									Refnum;
	};


/* global variable containing the last I/O error.  Haven't decided what */
/* to use it for yet. */
static OSErr							LastError;

/* things for debugging */
EXECUTE(static ArrayRec*	FileSpecList;)
EXECUTE(static ArrayRec*	FileDescriptorList;)


/* initialize the file subsystem.  should only be called from Screen */
MyBoolean						Eep_InitializeFiles(void)
	{
		APRINT(("+Eep_InitializeFiles"));
#if DEBUG
		FileSpecList = NewArray();
		if (FileSpecList == NIL)
			{
			 FailurePoint1:
				APRINT(("-Eep_InitializeFiles failed"));
				return False;
			}
		FileDescriptorList = NewArray();
		if (FileDescriptorList == NIL)
			{
			 FailurePoint2:
				DisposeArray(FileSpecList);
				goto FailurePoint1;
			}
#endif
		APRINT(("-Eep_InitializeFiles"));
		return True;
	}


/* shutdown the file subsystem. */
void								Eep_ShutdownFiles(void)
	{
		APRINT(("+Eep_ShutdownFiles"));
#if DEBUG
		ERROR(ArrayGetLength(FileSpecList) != 0,PRERR(AllowResume,
			"Eep_ShutdownFiles:  some file specs still exist"));
		ERROR(ArrayGetLength(FileDescriptorList) != 0,PRERR(AllowResume,
			"FileDescriptorList:  some files are still open"));
		DisposeArray(FileSpecList);
		DisposeArray(FileDescriptorList);
#endif
		APRINT(("-Eep_ShutdownFiles"));
	}


/* this is an internal routine -- do not use */
#if DEBUG
MyBoolean						Eep_RegisterFileSpec(FileSpec* Spec)
	{
		CheckPtrExistence(Spec);
		return ArrayAppendElement(FileSpecList,Spec);
	}
#endif


/* this routine validates a file specification to make sure it exists */
#if DEBUG
void							ValidateFileSpec(FileSpec* Spec)
	{
		CheckPtrExistence(Spec);
		ERROR(-1 == ArrayFindElement(FileSpecList,Spec),PRERR(ForceAbort,
			"ValidateFileSpec:  undefined file specification"));
	}
#endif


/* make a copy of a file specification */
/* this routine does not perform operations on file descriptors. */
FileSpec*						DuplicateFileSpec(FileSpec* Original)
	{
		FileSpec*					Copy;

		APRINT(("+DuplicateFileSpec %xl",Original));
		ERROR(-1 == ArrayFindElement(FileSpecList,Original),PRERR(ForceAbort,
			"DuplicateFileSpec:  undefined file specification"));
		Copy = (FileSpec*)AllocPtrCanFail(sizeof(FSSpec),"FileSpec");
		if (Copy != NIL)
			{
				CopyData((char*)Original,(char*)Copy,sizeof(FSSpec));
				if (!Eep_RegisterFileSpec(Copy))
					{
						ReleasePtr((char*)Copy);
						Copy = NIL;
					}
			}
		APRINT(("-DuplicateFileSpec %xl",Copy));
		return Copy;
	}


/* dispose of a file specification thing */
void								DisposeFileSpec(FileSpec* Spec)
	{
		APRINT(("+DisposeFileSpec %xl",Spec));
		ERROR(-1 == ArrayFindElement(FileSpecList,Spec),PRERR(ForceAbort,
			"DisposeFileSpec:  undefined file specification"));
		EXECUTE(ArrayDeleteElement(FileSpecList,ArrayFindElement(FileSpecList,Spec)));
		ReleasePtr((char*)Spec);
		APRINT(("-DisposeFileSpec"));
	}


/* create a new temporary file with a known unique name, and return a specification */
/* leading to it.  the creator and filetype codes may or may not be used by */
/* the implementation; the Macintosh does use them.  The file is actually created. */
/* this routine does not perform operations on file descriptors. */
FileSpec*						NewTempFileSpec(unsigned long Creator, unsigned long FileType)
	{
		short							vRefNum;
		long							DirID;
		FSSpec*						Temp;
		unsigned char			Name[32] = "\pTempFile........";
		int								Scan;
		unsigned long			Key;

		APRINT(("+NewTempFileSpec"));
		FindFolder(kOnSystemDisk,kTemporaryFolderType,kCreateFolder,&vRefNum,&DirID);
		Temp = (FSSpec*)AllocPtrCanFail(sizeof(FSSpec),"FileSpec");
		if (Temp == NIL)
			{
				APRINT(("-NewTempFileSpec NIL"));
				return NIL;
			}
		Key = TickCount();
	 TryPoint:
		for (Scan = 0; Scan < 8; Scan += 1)
			{
				Name[Scan + 9] = ((Key >> (Scan * 4)) & 0x0f) + 'a';
			}
		FSMakeFSSpec(vRefNum,DirID,Name,Temp);
		LastError = FSpCreate(Temp,Creator,FileType,smSystemScript);
		if (LastError == dupFNErr)
			{
				Key += 1;
				goto TryPoint;
			}
		if (LastError != noErr)
			{
				ReleasePtr((char*)Temp);
				Temp = NIL;
			}
		if (Temp != NIL)
			{
				if (!Eep_RegisterFileSpec((FileSpec*)Temp))
					{
						ReleasePtr((char*)Temp);
						Temp = NIL;
					}
			}
		APRINT(("-NewTempFileSpec %r",Temp));
		return (FileSpec*)Temp;
	}


/* create a file spec leading to a named preference file.  The location of preference */
/* files are implementation defined:  UNIX may put them in the user's home directory */
/* with a leading period; Macintosh puts them in a "Preferences" folder.  The file */
/* is actually created unless it already exists.  PrefsFileName is a null terminated */
/* string. */
/* this routine does not perform operations on file descriptors. */
FileSpec*						NewPrefsFileSpec(char* PrefsFileName, unsigned long Creator,
											unsigned long FileType)
	{
		short							vRefNum;
		long							DirID;
		FSSpec*						Temp;
		unsigned char			Name[32];
		int								Scan;

		APRINT(("+NewPrefsFileSpec name = %t",PrefsFileName));
		FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,&vRefNum,&DirID);
		Temp = (FSSpec*)AllocPtrCanFail(sizeof(FSSpec),"FileSpec");
		if (Temp == NIL)
			{
				APRINT(("-NewPrefsFileSpec NIL"));
				return NIL;
			}
		Scan = 0;
		while ((Scan < 31) && (PrefsFileName[Scan] != 0))
			{
				Name[Scan + 1] = PrefsFileName[Scan];
				Scan += 1;
			}
		Name[0] = Scan;
		FSMakeFSSpec(vRefNum,DirID,Name,Temp);
		LastError = FSpCreate(Temp,Creator,FileType,smSystemScript);
		if (!Eep_RegisterFileSpec((FileSpec*)Temp))
			{
				ReleasePtr((char*)Temp);
				Temp = NIL;
			}
		APRINT(("-NewPrefsFileSpec %r",Temp));
		return (FileSpec*)Temp;
	}


/* present a dialog box allowing the user to select where to create a new file. */
/* If the file will overwrite an existing file, verify this with the user and then */
/* delete the existing file before returning. DefaultFileName is a null terminated */
/* string. */
/* this routine does not perform operations on file descriptors. */
FileSpec*						PutFile(char* DefaultFileName)
	{
		unsigned char			Name[32];
		StandardFileReply	MySFR;
		int								Scan;
		FSSpec*						ReturnValue;

		APRINT(("+PutFile name = %t",DefaultFileName));
		LastError = noErr;
		Scan = 0;
		while ((Scan < 31) && (DefaultFileName[Scan] != 0))
			{
				Name[Scan + 1] = DefaultFileName[Scan];
				Scan += 1;
			}
		Name[0] = Scan;
		StandardPutFile(NIL,Name,&MySFR);
		if (MySFR.sfGood)
			{
				if (MySFR.sfReplacing)
					{
						FSpDelete(&MySFR.sfFile);
					}
				ReturnValue = (FSSpec*)AllocPtrCanFail(sizeof(FSSpec),"FileSpec");
				if (ReturnValue == NIL)
					{
						APRINT(("-PutFile NIL"));
						return NIL;
					}
				*ReturnValue = MySFR.sfFile;
				if (MySFR.sfReplacing)
					{
						FSpDelete(&MySFR.sfFile);
					}
			}
		 else
			{
				ReturnValue = NIL;
			}
		if (ReturnValue != NIL)
			{
				if (!Eep_RegisterFileSpec((FileSpec*)ReturnValue))
					{
						ReleasePtr((char*)ReturnValue);
						ReturnValue = NIL;
					}
			}
		APRINT(("-PutFile %r",ReturnValue));
		return (FileSpec*)ReturnValue;
	}


/* let the user find a file with the specified list of types.  Whether the types */
/* are actually used is implementation defined.  The Macintosh uses up to 4 types. */
/* returns NIL if the operation was cancelled. */
/* this routine does not perform operations on file descriptors. */
FileSpec*						GetFileStandard(long NumFileTypes, unsigned long* ArrayOfFileTypes)
	{
		StandardFileReply	MySFR;
		FSSpec*						Where;

		APRINT(("+GetFileStandard"));
		ERROR((NumFileTypes != 0) && (ArrayOfFileTypes == NIL),PRERR(ForceAbort,
			"GetFileStandard:  file type array is NIL"));
		LastError = noErr;
		StandardGetFile(NIL,NumFileTypes,ArrayOfFileTypes,&MySFR);
		if (MySFR.sfGood)
			{

				Where = (FSSpec*)AllocPtrCanFail(sizeof(FSSpec),"FileSpec");
				if (Where != NIL)
					{
						*Where = MySFR.sfFile;
					}
			}
		 else
			{
				Where = NIL;
			}
		if (Where != NIL)
			{
				if (!Eep_RegisterFileSpec((FileSpec*)Where))
					{
						ReleasePtr((char*)Where);
						Where = NIL;
					}
			}
		APRINT(("-GetFileStandard %r",Where));
		return (FileSpec*)Where;
	}


/* get any file.  like GetFileStandard except it shows all possible files */
/* this routine does not perform operations on file descriptors. */
FileSpec*						GetFileAny(void)
	{
		StandardFileReply	MySFR;
		FSSpec*						Where;

		APRINT(("+GetFileAny"));
		LastError = noErr;
		StandardGetFile(NIL,-1,NIL,&MySFR);
		if (MySFR.sfGood)
			{
				Where = (FSSpec*)AllocPtrCanFail(sizeof(FSSpec),"FileSpec");
				if (Where != NIL)
					{
						*Where = MySFR.sfFile;
					}
			}
		 else
			{
				Where = NIL;
			}
		if (Where != NIL)
			{
				if (!Eep_RegisterFileSpec((FileSpec*)Where))
					{
						ReleasePtr((char*)Where);
						Where = NIL;
					}
			}
		APRINT(("-GetFileAny %r",Where));
		return (FileSpec*)Where;
	}


/* return a pointer containing a non-null-terminated string which is the filename. */
/* this routine does not perform operations on file descriptors. */
char*								ExtractFileName(FileSpec* Spec)
	{
		char*							Name;

		CheckPtrExistence(Spec);
		ERROR(-1 == ArrayFindElement(FileSpecList,Spec),PRERR(ForceAbort,
			"ExtractFileName:  undefined file specification"));
		Name = AllocPtrCanFail(((FSSpec*)Spec)->name[0],"ExtractFileName");
		if (Name != NIL)
			{
				CopyData((char*)&(((FSSpec*)Spec)->name[1]),&(Name[0]),((FSSpec*)Spec)->name[0]);
			}
		return Name;
	}


/* create a file.  Returns True if the creation succeeded.  Whether or not */
/* the Creator and FileType codes are used is implementation defined.  The Macintosh */
/* does use them. */
/* this routine does not perform operations on file descriptors. */
MyBoolean						CreateFile(FileSpec* FileLocation, unsigned long Creator,
											unsigned long FileType)
	{
		APRINT(("+CreateFile name=%p dir=%l vol=%s",(*(FSSpec*)FileLocation).name,
			(*(FSSpec*)FileLocation).parID,(*(FSSpec*)FileLocation).vRefNum));
		CheckPtrExistence(FileLocation);
		ERROR(-1 == ArrayFindElement(FileSpecList,FileLocation),PRERR(ForceAbort,
			"CreateFile:  undefined file specification"));
		LastError = FSpCreate((FSSpec*)FileLocation,Creator,FileType,smSystemScript);
		APRINT(("-CreateFile err=%s",LastError));
		return (LastError == noErr);
	}


/* delete a file.  The file must not be in use.  Returns True if successful */
/* this routine does not perform operations on file descriptors. */
MyBoolean						DeleteFile(FileSpec* FileLocation)
	{
		APRINT(("+DeleteFile"));
		CheckPtrExistence(FileLocation);
		ERROR(-1 == ArrayFindElement(FileSpecList,FileLocation),PRERR(ForceAbort,
			"DeleteFile:  undefined file specification"));
		LastError = FSpDelete((FSSpec*)FileLocation);
		APRINT(("-DeleteFile err=%s",LastError));
		return (LastError == noErr);
	}


/* Open a file for the specified access.  Returns True if successful. */
/* this routine DOES perform operations on file descriptors. */
MyBoolean						OpenFile(FileSpec* FileLocation, FileType** FileRefOut,
											FileModesType FileAccessMode)
	{
		FileType*					FileRef;
		int								Permission;

		APRINT(("+OpenFile name=%p dir=%l vol=%s",(*(FSSpec*)FileLocation).name,
			(*(FSSpec*)FileLocation).parID,(*(FSSpec*)FileLocation).vRefNum));
		CheckPtrExistence(FileLocation);
		ERROR(-1 == ArrayFindElement(FileSpecList,FileLocation),PRERR(ForceAbort,
			"OpenFile:  undefined file specification"));
		FileRef = (FileType*)AllocPtrCanFail(sizeof(FileType),"FileType");
		if (FileRef == NIL)
			{
			 FailurePoint1:
				APRINT(("-OpenFile failed"));
				return False;
			}
		switch (FileAccessMode)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"OpenFile:  bad file permission specifier"));
					break;
				case eReadOnly:
					Permission = fsRdPerm;
					break;
				case eReadAndWrite:
					Permission = fsRdWrPerm;
					break;
			}
		LastError = FSpOpenDF((FSSpec*)FileLocation,Permission,&(FileRef->Refnum));
		if (LastError != noErr)
			{
				ReleasePtr((char*)FileRef);
				APRINT((" Error",LastError));
				goto FailurePoint1;
			}
		EXECUTE(if (!ArrayAppendElement(FileDescriptorList,FileRef))
			{FSClose(FileRef->Refnum); ReleasePtr((char*)FileRef); FileRef = NIL;
			goto FailurePoint1;})
		*FileRefOut = FileRef;
		APRINT(("-OpenFile %r %s",FileRef,FileRef->Refnum));
		return True;
	}


/* close a file.  The file must have been open.  Implicitly calls FlushLocalBuffers. */
/* this routine DOES perform operations on file descriptors. */
void								CloseFile(FileType* FileRef)
	{
		APRINT(("+CloseFile %r %s",FileRef,FileRef->Refnum));
		CheckPtrExistence(FileRef);
		ERROR(-1 == ArrayFindElement(FileDescriptorList,FileRef),PRERR(ForceAbort,
			"CloseFile:  undefined file descriptor"));
		FlushLocalBuffers(FileRef);
		LastError = FSClose(FileRef->Refnum);
		EXECUTE(ArrayDeleteElement(FileDescriptorList,
			ArrayFindElement(FileDescriptorList,FileRef)));
		ReleasePtr((char*)FileRef);
		APRINT(("-CloseFile err=%s",LastError));
	}


/* make sure all data associated with a file gets written out */
/* this routine DOES perform operations on file descriptors. */
void								FlushLocalBuffers(FileType* FileRef)
	{
		short							vRefNum;

		APRINT(("+FlushLocalBuffers %r %s",FileRef,FileRef->Refnum));
		CheckPtrExistence(FileRef);
		ERROR(-1 == ArrayFindElement(FileDescriptorList,FileRef),PRERR(ForceAbort,
			"FlushLocalBuffers:  undefined file descriptor"));
		LastError = GetVRefNum(FileRef->Refnum,&vRefNum);
		if (LastError == noErr)
			{
				LastError = FlushVol(NIL,vRefNum);
			}
		APRINT(("-FlushLocalBuffers err=%s",LastError));
	}


/* write a block of data to the file.  Returns the number of bytes which could */
/* not be written or 0 if all were written. */
/* This is the only function that calls "FSWrite" */
/* this routine DOES perform operations on file descriptors. */
long								WriteToFile(FileType* FileRef, char* Buffer, long NumBytes)
	{
		APRINT(("+WriteToFile %r %s",FileRef,FileRef->Refnum));
		CheckPtrExistence(FileRef);
		ERROR(-1 == ArrayFindElement(FileDescriptorList,FileRef),PRERR(ForceAbort,
			"WriteToFile:  undefined file descriptor"));
		do
			{
				long							NumBytesDone;

				NumBytesDone = NumBytes;
				LastError = FSWrite(FileRef->Refnum,&NumBytesDone,Buffer);
				Buffer += NumBytesDone;
				NumBytes -= NumBytesDone;
				ERROR(NumBytes != 0,PRERR(AllowResume,
					"WriteToFile:  couldn't write whole block, trying again"));
			} while ((LastError == noErr) && (NumBytes != 0));
		APRINT(("-WriteToFile err=%s",LastError));
		return NumBytes; /* return number of bytes that couldn't be written */
	}


/* read a block of data from the file.  Returns the number of bytes which */
/* could not be read, or 0 if all were read. */
/* This is the only function that calls "FSRead" */
/* this routine DOES perform operations on file descriptors. */
long								ReadFromFile(FileType* FileRef, char* Buffer, long NumBytesDesired)
	{
		EXECUTE(long			TryCount = 0;)

		APRINT(("+ReadFromFile %r %s",FileRef,FileRef->Refnum));
		CheckPtrExistence(FileRef);
		ERROR(-1 == ArrayFindElement(FileDescriptorList,FileRef),PRERR(ForceAbort,
			"ReadFromFile:  undefined file descriptor"));
		do
			{
				long							NumBytesDone;

				NumBytesDone = NumBytesDesired;
				LastError = FSRead(FileRef->Refnum,&NumBytesDone,Buffer);
				Buffer += NumBytesDone;
				NumBytesDesired -= NumBytesDone;
				ERROR((NumBytesDesired != 0) && (TryCount > 0),PRERR(AllowResume,
					"ReadFromFile:  couldn't read whole block, trying again"));
				EXECUTE(TryCount += 1;)
			} while ((LastError == noErr) && (NumBytesDesired != 0));
		APRINT(("-ReadFromFile err=%s",LastError));
		return NumBytesDesired; /* return number of bytes that couldn't be read */
	}


/* get the current index into the specified file */
/* this routine DOES perform operations on file descriptors. */
long								GetFilePosition(FileType* FileRef)
	{
		long							Where;

		APRINT(("+GetFilePosition %r %s",FileRef,FileRef->Refnum));
		CheckPtrExistence(FileRef);
		ERROR(-1 == ArrayFindElement(FileDescriptorList,FileRef),PRERR(ForceAbort,
			"GetFilePosition:  undefined file descriptor"));
		LastError = GetFPos(FileRef->Refnum,&Where);
		APRINT(("-GetFilePosition err=%s",LastError));
		return Where;
	}


/* move to a new location within the specified file */
/* if the new location is past the EOF, the EOF will be extended to the new location */
/* this routine DOES perform operations on file descriptors. */
MyBoolean						SetFilePosition(FileType* FileRef, long NewLocation)
	{
		long							EOF;

		APRINT(("+SetFilePosition %r %s",FileRef,FileRef->Refnum));
		CheckPtrExistence(FileRef);
		ERROR(-1 == ArrayFindElement(FileDescriptorList,FileRef),PRERR(ForceAbort,
			"SetFilePosition:  undefined file descriptor"));
		LastError = GetEOF(FileRef->Refnum,&EOF);
		if (EOF < NewLocation)
			{
				LastError = SetEOF(FileRef->Refnum,NewLocation);
			}
		if (LastError == noErr)
			{
				LastError = SetFPos(FileRef->Refnum,fsFromStart,NewLocation);
			}
		APRINT(("-SetFilePosition err=%s",LastError));
		return (LastError == noErr);
	}


/* get the length of the specified file */
/* this routine DOES perform operations on file descriptors. */
long								GetFileLength(FileType* FileRef)
	{
		long							EOFPos;

		APRINT(("+GetFileLength %r %s",FileRef,FileRef->Refnum));
		CheckPtrExistence(FileRef);
		ERROR(-1 == ArrayFindElement(FileDescriptorList,FileRef),PRERR(ForceAbort,
			"GetFileLength:  undefined file descriptor"));
		LastError = GetEOF(FileRef->Refnum,&EOFPos);
		APRINT(("-GetFileLength err=%s",LastError));
		return EOFPos;
	}


/* set the length of the specified file. */
/* returns True if everything went well or False if there was an error */
/* If the file could not be extended due to lack of disk space, the EOF */
/* remains unchanged */
/* this routine DOES perform operations on file descriptors. */
MyBoolean						SetFileLength(FileType* FileRef, long NewFileLength)
	{
		long							OldEOF;

		APRINT(("+SetFileLength %r %s",FileRef,FileRef->Refnum));
		CheckPtrExistence(FileRef);
		ERROR(-1 == ArrayFindElement(FileDescriptorList,FileRef),PRERR(ForceAbort,
			"SetFileLength:  undefined file descriptor"));
		LastError = GetEOF(FileRef->Refnum,&OldEOF);
		if (LastError == noErr)
			{
				LastError = SetEOF(FileRef->Refnum,NewFileLength);
				if (LastError == dskFulErr)
					{
						SetEOF(FileRef->Refnum,OldEOF);
					}
			}
		APRINT(("-SetFileLength err=%s",LastError));
		return (LastError == noErr);
	}


/* copy the entire contents of one file to another.  Data overwrites destination */
/* this routine does not perform operations on file descriptors. */
#define REALBIGBUFFSIZE (65536)
#define LITTLEBUFFSIZE (1024)
MyBoolean						CopyFile(FileType* Original, FileType* Destination)
	{
		char*							Buffer;
		MyBoolean					ReleaseBuffer;
		long							ActualBufferSize;
		long							ByteCount;
		char							EmergencyBuff[LITTLEBUFFSIZE];

		CheckPtrExistence(Original);
		CheckPtrExistence(Destination);
		if (!SetFilePosition(Original,0))
			{
				return False;
			}
		if (!SetFilePosition(Destination,0))
			{
				return False;
			}
		if (!SetFileLength(Destination,0))
			{
				return False;
			}
		Buffer = AllocPtrCanFail(REALBIGBUFFSIZE,"CopyFileBuffer");
		if (Buffer != NIL)
			{
				ActualBufferSize = REALBIGBUFFSIZE;
				ReleaseBuffer = True;
			}
		 else
			{
				ActualBufferSize = LITTLEBUFFSIZE;
				ReleaseBuffer = False;
				Buffer = EmergencyBuff;
			}
		ByteCount = GetFileLength(Original);
		while (ByteCount != 0)
			{
				long							LocalBytes;

				if (ByteCount > ActualBufferSize)
					{
						LocalBytes = ActualBufferSize;
					}
				 else
					{
						LocalBytes = ByteCount;
					}
				if (ReadFromFile(Original,Buffer,LocalBytes) != 0)
					{
					 FailurePoint:
						if (ReleaseBuffer)
							{
								ReleasePtr(Buffer);
							}
						return False;
					}
				if (WriteToFile(Destination,Buffer,LocalBytes) != 0)
					{
						goto FailurePoint;
					}
				ByteCount -= LocalBytes;
			}
		if (ReleaseBuffer)
			{
				ReleasePtr(Buffer);
			}
		return True;
	}


/* create a temporary file in the directory.  the file is actually created so it */
/* exists (so nobody can grab it out from under you). */
/* this routine DOES perform operations on file descriptors. */
FileSpec*						NewTempFileInTheSameDirectory(FileSpec* SameDirectoryAsThis)
	{
		FSSpec*						NewOne;
		unsigned char			Name[32] = "\pTempFile........";
		int								Scan;
		unsigned long			Key;

		ERROR(-1 == ArrayFindElement(FileSpecList,SameDirectoryAsThis),PRERR(ForceAbort,
			"DisposeFileSpec:  undefined file specification"));
		CheckPtrExistence(SameDirectoryAsThis);
		NewOne = (FSSpec*)AllocPtrCanFail(sizeof(FSSpec),"SameDirectoryNewName");
		if (NewOne == NIL)
			{
				return NIL;
			}
		Key = TickCount();
	 TryPoint:
		for (Scan = 0; Scan < 8; Scan += 1)
			{
				Name[Scan + 9] = ((Key >> (Scan * 4)) & 0x0f) + 'a';
			}
		FSMakeFSSpec(((FSSpec*)SameDirectoryAsThis)->vRefNum,
			((FSSpec*)SameDirectoryAsThis)->parID,Name,NewOne);
		LastError = FSpCreate(NewOne,'\?\?\?\?','\?\?\?\?',smSystemScript);
		if (LastError == dupFNErr)
			{
				Key += 1;
				goto TryPoint;
			}
		if (LastError != noErr)
			{
				/* some other error occurred -- may be a disk full error. */
				ReleasePtr((char*)NewOne);
				NewOne = NIL;
			}
		if (NewOne != NIL)
			{
				if (!Eep_RegisterFileSpec((FileSpec*)NewOne))
					{
						ReleasePtr((char*)NewOne);
						NewOne = NIL;
					}
			}
		return (FileSpec*)NewOne;
	}


/* this operation swaps the data forks for two files.  this is used for safe */
/* saving (i.e. write data before clobbering so that the old file still exists */
/* if the write fails).  You create a new file and write the data to it.  Then you */
/* pass the location of the new file in NewFile and the location of the old */
/* file in OldFile.  The routine swaps the data in the files and the last modified */
/* dates.  If successful, the NewFile (temporary file) will be deleted and the */
/* OldFileRef reference will be updated (it will NOT be the same) and the NewFileRef */
/* will be closed. */
/* NOTE:  the files should be created in the same directory. */
/* this routine DOES perform operations on file descriptors. */
MyBoolean						SwapFileDataForks(FileSpec* NewAndTempFile, FileSpec* OldFile,
											FileType* NewFileRef, FileType** OldFileRef)
	{
		ERROR(-1 == ArrayFindElement(FileSpecList,NewAndTempFile),PRERR(ForceAbort,
			"SwapFileDataForks:  undefined file specification"));
		ERROR(-1 == ArrayFindElement(FileSpecList,OldFile),PRERR(ForceAbort,
			"SwapFileDataForks:  undefined file specification"));
		ERROR(-1 == ArrayFindElement(FileDescriptorList,NewFileRef),PRERR(ForceAbort,
			"SwapFileDataForks:  undefined file descriptor"));
		ERROR(-1 == ArrayFindElement(FileDescriptorList,*OldFileRef),PRERR(ForceAbort,
			"SwapFileDataForks:  undefined file descriptor"));
		CheckPtrExistence(NewAndTempFile);
		CheckPtrExistence(OldFile);
		CheckPtrExistence(NewFileRef);
		ERROR(OldFileRef == NIL,PRERR(ForceAbort,"SwapFileDataForks:  OldFileRef is NIL"));
		CheckPtrExistence(*OldFileRef);
		LastError = FSpExchangeFiles((FSSpec*)NewAndTempFile,(FSSpec*)OldFile);
		if (LastError != noErr)
			{
				return False;
			}
		CloseFile(*OldFileRef);
		*OldFileRef = NewFileRef;
		DeleteFile(NewAndTempFile);
		SetTag(NewAndTempFile,"SwapFileDataForks: NewAndTempFile");
		CheckPtrExistence(*OldFileRef);
		return True;
	}
