/* File.c */

#include "File.h"
#include <Script.h>
#include "Memory.h"
#include <Finder.h>
#include "Compatibility.h"
#include "Error.h"
#include <Folders.h>


static OSErr		LastError = 0;


OSErr		FCreate(FSSpec* FileInfo, ulong Creator, ulong FileType)
	{
		LastError = FSpCreate(FileInfo,Creator,FileType,smSystemScript);
		if (LastError != noErr)
			{
				SetErrorStatus();
			}
		return LastError;
	}


OSErr		FOpenFile(FSSpec* FileInfo, short* FileReferenceOut)
	{
		short		RefNum;

		LastError = FSpOpenDF(FileInfo,fsRdWrPerm,FileReferenceOut);
		if (LastError != noErr)
			{
				SetErrorStatus();
			}
		return LastError;
	}


OSErr		FFlushFile(short FileReference)
	{
		short		vRefNum;

		LastError = GetVRefNum(FileReference,&vRefNum);
		if (LastError == noErr)
			{
				LastError = FlushVol(NIL,vRefNum);
			}
		if (LastError != noErr)
			{
				SetErrorStatus();
			}
		return LastError;
	}


OSErr		FCloseFile(short FileReference)
	{
		short		vRefNum;

		LastError = GetVRefNum(FileReference,&vRefNum);
		if (LastError == noErr)
			{
				LastError = FSClose(FileReference);
				if (LastError == noErr)
					{
						LastError = FlushVol(NIL,vRefNum);
					}
			}
		if (LastError != noErr)
			{
				SetErrorStatus();
			}
		return LastError;
	}


OSErr		FDeleteFile(FSSpec* FileInfo)
	{
		LastError = FSpDelete(FileInfo);
		if (LastError != noErr)
			{
				SetErrorStatus();
			}
		return LastError;
	}


OSErr		FDeleteIfExistsFile(FSSpec* FileInfo)
	{
		LastError = FSpDelete(FileInfo);
		return LastError;
	}


OSErr		FWriteBlock(short FileReference, char* PointerToStart, ulong NumberOfBytes)
	{
		long		NumBytesDone;

		StackSizeTest();
	 LoopPoint:
		NumBytesDone = NumberOfBytes;
		LastError = FSWrite(FileReference,&NumBytesDone,PointerToStart);
		if (LastError != noErr)
			{
				SetErrorStatus();
			}
		PointerToStart += NumBytesDone;
		NumberOfBytes -= NumBytesDone;
		if ((LastError == noErr) && (NumberOfBytes != 0)) goto LoopPoint;
		return LastError;
	}


OSErr		FReadBlock(short FileReference, char* PlaceToPut, ulong NumberOfBytes)
	{
		long		NumBytesDone;

		StackSizeTest();
	 LoopPoint:
		NumBytesDone = NumberOfBytes;
		LastError = FSRead(FileReference,&NumBytesDone,PlaceToPut);
		if (LastError != noErr)
			{
				SetErrorStatus();
			}
		PlaceToPut += NumBytesDone;
		NumberOfBytes -= NumBytesDone;
		if ((LastError == noErr) && (NumberOfBytes != 0)) goto LoopPoint;
		return LastError;
	}


OSErr		FSetEOF(short FileReference, ulong NewEOF)
	{
		LastError = SetEOF(FileReference,NewEOF);
		if (LastError != noErr)
			{
				SetErrorStatus();
			}
		return LastError;
	}


OSErr		FGetEOF(short FileReference, long* EOFPos)
	{
		LastError = GetEOF(FileReference,EOFPos);
		if (LastError != noErr)
			{
				SetErrorStatus();
			}
		return LastError;
	}


OSErr		FSetFilePos(short FileReference, ulong NewFilePos)
	{
		long			EOF;

		LastError = GetEOF(FileReference,&EOF);
		if (EOF < NewFilePos)
			{
				LastError = SetEOF(FileReference,NewFilePos);
			}
		if (LastError == noErr)
			{
				LastError = SetFPos(FileReference,fsFromStart,NewFilePos);
			}
		if (LastError != noErr)
			{
				SetErrorStatus();
			}
		return LastError;
	}


OSErr		FGetFilePos(short FileReference, long* FilePos)
	{
		LastError = GetFPos(FileReference,FilePos);
		if (LastError != noErr)
			{
				SetErrorStatus();
			}
		return LastError;
	}


/* move data from one place on the disk to another. */
/* data can overlap when it is to be moved backward, but not forward. */
/* non-overlapping data has no problem */
#define BigDiskBufferSize 65536
#define SmallDiskBufferSize 8192
#define EmergencyBufferSize 256
OSErr		FMoveData(short SourceFile, ulong SourceIndex,
					short TargetFile, ulong TargetIndex, ulong NumBytes)
	{
		static char		EmergencyBuffer[EmergencyBufferSize];
		Handle				DiskBuffer;
		ulong					DiskBufferSize;
		char*					BufPtr;

		DiskBuffer = AllocHandleCanFail(BigDiskBufferSize);
		if (DiskBuffer == NIL)
			{
				DiskBuffer = AllocHandleCanFail(SmallDiskBufferSize);
			}
		if (DiskBuffer != NIL)
			{
				HLock(DiskBuffer);
				BufPtr = *DiskBuffer;
				DiskBufferSize = HandleSize(DiskBuffer);
			}
		 else
			{
				BufPtr = EmergencyBuffer;
				DiskBufferSize = EmergencyBufferSize;
			}
		while (NumBytes > 0)
			{
				ulong		Count;

				if (NumBytes > DiskBufferSize)
					{
						Count = DiskBufferSize;
					}
				 else
					{
						Count = NumBytes;
					}
				FSetFilePos(SourceFile,SourceIndex);
				if (LastError != noErr)
					{
						goto ErrorPoint;
					}
				FReadBlock(SourceFile,BufPtr,Count);
				if (LastError != noErr)
					{
						goto ErrorPoint;
					}
				FSetFilePos(TargetFile,TargetIndex);
				if (LastError != noErr)
					{
						goto ErrorPoint;
					}
				FWriteBlock(TargetFile,BufPtr,Count);
				if (LastError != noErr)
					{
						goto ErrorPoint;
					}
				NumBytes -= Count;
				SourceIndex += Count;
				TargetIndex += Count;
			}
		if (DiskBuffer != NIL)
			{
				ReleaseHandle(DiskBuffer);
			}
	 ErrorPoint:
		return LastError;
	}


OSErr		FLastError(void)
	{
		return LastError;
	}


void		FFindTempFolder(short* vRefNum, long* DirID)
	{
		FindFolder(kOnSystemDisk,kTemporaryFolderType,kCreateFolder,vRefNum,DirID);
	}


void		FFindPrefsFolder(short* vRefNum, long* DirID)
	{
		FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,vRefNum,DirID);
	}


void		FMakeFSSpec(short vRefNum, long DirID, PString FileName, FSSpec* FileInfo)
	{
		FSMakeFSSpec(vRefNum,DirID,FileName,FileInfo);
	}


/* find an unused temporary file name in the specified directory */
void			FMakeTempFileName(short VRefNum, long DirID, PString Name)
	{
		long			Count;
		short			Scan;
		FSSpec		LocalFSSpec;
		char			LocalName[] = "\pTempFile 01234567";
		OSErr			Error;
		FInfo			Junk;

		Count = TickCount();
	 LoopPoint:
		for (Scan = 0; Scan < 8; Scan += 1)
			{
				LocalName[LocalName[0]-Scan] = Byte2Hex(0x0f & (Count >> (Scan*4)));
			}
		FMakeFSSpec(VRefNum,DirID,(uchar*)LocalName,&LocalFSSpec);
		Error = FSpGetFInfo(&LocalFSSpec,&Junk);
		if (Error != fnfErr)
			{
				Count += 1;
				goto LoopPoint;
			}
		MemCpy((char*)&(Name[0]),(char*)&(LocalFSSpec.name[0]),LocalFSSpec.name[0] + 1);
		LastError = noErr;
	}


OSErr			FGetFInfo(FSSpec* FileInfo, FInfo* FinderInfo)
	{
		LastError = FSpGetFInfo(FileInfo,FinderInfo);
		if (LastError != noErr)
			{
				SetErrorStatus();
			}
		return LastError;
	}


OSErr			FGetContainingDirectory(short FileRefNum, short* HardVRefNum, long* DirID)
	{
		FCBPBRec		MyFCB;

		MyFCB.ioCompletion = NIL;
		MyFCB.ioFCBIndx = 0;
		MyFCB.ioVRefNum = 0;
		MyFCB.ioRefNum = FileRefNum;
		MyFCB.ioNamePtr = NIL;
		LastError = PBGetFCBInfo(&MyFCB,False);
		if (LastError == noErr)
			{
				LastError = MyFCB.ioResult;
			}
		if (LastError != noErr)
			{
				SetErrorStatus();
			}
		 else
			{
				*HardVRefNum = MyFCB.ioFCBVRefNum;
				*DirID = MyFCB.ioFCBParID;
			}
		return LastError;
	}


OSErr			FDerefFolder(FSSpec* TheFolder, long* FolderID)
	{
		CInfoPBRec		MyPB;

		MyPB.dirInfo.ioCompletion = NIL;
		MyPB.dirInfo.ioVRefNum = TheFolder->vRefNum;
		MyPB.dirInfo.ioNamePtr = TheFolder->name;
		MyPB.dirInfo.ioFDirIndex = 0;
		MyPB.dirInfo.ioDrDirID = TheFolder->parID;
		LastError = PBGetCatInfo(&MyPB,False);
		if (LastError == noErr)
			{
				LastError = MyPB.dirInfo.ioResult;
			}
		if (LastError != noErr)
			{
				SetErrorStatus();
			}
		 else
			{
				if ((MyPB.dirInfo.ioFlAttrib & 16) == 0)
					{
						/* if it's not a directory, then we return error */
						LastError = dirNFErr;
						SetErrorStatus();
					}
				 else
					{
						*FolderID = MyPB.dirInfo.ioDrDirID;
					}
			}
		return LastError;
	}
