/* File.h */

#pragma once

OSErr			FCreate(FSSpec* FileInfo, ulong Creator, ulong FileType);
OSErr			FOpenFile(FSSpec* FileInfo, short* FileReferenceOut);
OSErr			FFlushFile(short FileReference);
OSErr			FCloseFile(short FileReference);
OSErr			FDeleteFile(FSSpec* FileInfo);
OSErr			FDeleteIfExistsFile(FSSpec* FileInfo);
OSErr			FWriteBlock(short FileReference, char* PointerToStart, ulong NumberOfBytes);
OSErr			FReadBlock(short FileReference, char* PlaceToPut, ulong NumberOfBytes);
OSErr			FSetEOF(short FileReference, ulong NewEOF);
OSErr			FGetEOF(short FileReference, long* EOFPos);
OSErr			FSetFilePos(short FileReference, ulong NewFilePos);
OSErr			FGetFilePos(short FileReference, long* FilePos);
OSErr			FMoveData(short SourceFile, ulong SourceIndex,
						short TargetFile, ulong TargetIndex, ulong NumBytes);
OSErr			FLastError(void);
void			FFindTempFolder(short* vRefNum, long* DirID);
void			FFindPrefsFolder(short* vRefNum, long* DirID);
void			FMakeFSSpec(short vRefNum, long DirID, PString FileName, FSSpec* FileInfo);
void			FMakeTempFileName(short VRefNum, long DirID, PString Name);
OSErr			FGetFInfo(FSSpec* FileInfo, FInfo* FinderInfo);
OSErr			FGetContainingDirectory(short FileRefNum, short* HardVRefNum, long* DirID);
OSErr			FDerefFolder(FSSpec* TheFolder, long* FolderID);
