/* DirStuff.c */
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
#include <Errors.h>
#include <Aliases.h>
#include <Script.h>
#ifdef THINK_C
	#pragma options(!pack_enums)
#endif

#include "DirStuff.h"
#include "Memory.h"
#include "Files.h"


#define MaxFilenameSize (256)

typedef struct DirEntry
	{
		struct DirEntry*		Next;
		FileSpec*						WhereIsIt;
		DirFileTypes				ItemType;
	} DirEntry;

struct DirectoryRec
	{
		DirEntry*						List;
	};


#define AUDITFILESPEC(x) APRINT((" "#x": v=%s,par=%l,%p",((FSSpec*)&(x))->vRefNum,\
					((FSSpec*)&(x))->parID,((FSSpec*)&(x))->name))


/* local routine to dereference a folder.  You supply the folder's location */
/* in TheFolder (name + parent) and it returns the ID of the folder in *FolderID */
/* (so you can use it as the parent of something inside the folder) */
static OSErr			FDerefFolder(FSSpec* TheFolder, long* FolderID)
	{
		CInfoPBRec		MyPB;
		OSErr					Error;
		FSSpec				Copy;
		Boolean				TargetWasFolder;
		Boolean				TargetWasAlias;

		APRINT(("+FDerefFolder v=%s,par=%l,%p",TheFolder->vRefNum,TheFolder->parID,
			TheFolder->name));
		Copy = *TheFolder;
		AUDITFILESPEC(Copy);
		Error = ResolveAliasFile(&Copy,False/*oneonly*/,&TargetWasFolder,&TargetWasAlias);
		APRINT((" ResolveAliasFile:  Err = %s, Folder = %b, Alias = %b",(short)Error,
			(short)TargetWasFolder,(short)TargetWasAlias));
		if (Error == fnfErr)
			{
				/* assume it's a volume. */
				/* return the volume's root directory number (no indirection) */
				*FolderID = Copy.parID;
				APRINT(("-FDerefFolder dirID=%l",*FolderID));
				return noErr;
			}
		MyPB.dirInfo.ioCompletion = NIL;
		MyPB.dirInfo.ioVRefNum = Copy.vRefNum;
		MyPB.dirInfo.ioNamePtr = Copy.name;
		MyPB.dirInfo.ioFDirIndex = 0;
		MyPB.dirInfo.ioDrDirID = Copy.parID;
		Error = PBGetCatInfo(&MyPB,False);
		if (Error == noErr)
			{
				Error = MyPB.dirInfo.ioResult;
			}
		if (Error != noErr)
			{
			}
		 else
			{
				if ((MyPB.dirInfo.ioFlAttrib & 16) == 0)
					{
						/* if it's not a directory, then we return error */
						Error = dirNFErr;
					}
				 else
					{
						*FolderID = MyPB.dirInfo.ioDrDirID;
						APRINT(("-FDerefFolder dirID=%l",*FolderID));
					}
			}
		ERROR(Error != noErr,APRINT(("-FDerefFolder failed %s",Error)));
		return Error;
	}


/* read the first level list of items in the specified directory.  NIL specifies */
/* a root directory.  If NIL is returned, then the operation could not be completed */
DirectoryRec*			ReadDirectory(FileSpec* Directory)
	{
		DirectoryRec*			Dir;
		DirEntry*					Tail;

		if (Directory != NIL)
			{
				CheckPtrExistence(Directory);
				ValidateFileSpec(Directory);
				APRINT(("+ReadDirectory v=%s,par=%l,%p",((FSSpec*)Directory)->vRefNum,
					((FSSpec*)Directory)->parID,((FSSpec*)Directory)->name));
			}
		 else
			{
				APRINT(("+ReadDirectory (root)"));
			}
		Dir = (DirectoryRec*)AllocPtrCanFail(sizeof(DirectoryRec),"DirectoryRec");
		if (Dir == NIL)
			{
				/* fault tolerance is most studly... */
				APRINT(("-ReadDirectory failed"));
				return NIL;
			}
		Tail = NIL;
		Dir->List = NIL;
		if ((NIL != Directory)
			|| ((((FSSpec*)Directory)->name[0] == 0)
			&& (((FSSpec*)Directory)->vRefNum == 0)
			&& (((FSSpec*)Directory)->parID == 0)))
			{
				CInfoPBRec			CInfo;
				OSErr						Error;
				short						Index;
				Str255					FileName;
				MyBoolean				EnergizerBunny;
				short						VolumeRefNum;
				long						DirectoryID;

				/* read through this directory and build a list of contained items */
				VolumeRefNum = ((FSSpec*)Directory)->vRefNum;
				if (noErr != FDerefFolder((FSSpec*)Directory,&DirectoryID))
					{
						/* oops, we weren't given a directory to look at. */
						ReleasePtr((char*)Dir);
						APRINT(("-ReadDirectory failed"));
						return NIL;
					}
				Index = 0;
				EnergizerBunny = True;
				while (EnergizerBunny)
					{
						Index += 1; /* index started at 0 up there, so we increment first */
						CInfo.hFileInfo.ioVRefNum = VolumeRefNum;
						CInfo.hFileInfo.ioNamePtr = FileName;
						CInfo.hFileInfo.ioDirID = DirectoryID;
						CInfo.hFileInfo.ioFDirIndex = Index;
						Error = PBGetCatInfo(&CInfo,False/*synchronously*/);
						if (Error == noErr)
							{
								DirEntry*				New;
								FSSpec*					Where;

								APRINT((" ReadDirectory filescan: '%p'",FileName));
								/* allocate a new record to hold the directory */
								New = (DirEntry*)AllocPtrCanFail(sizeof(DirEntry),"DirEntry");
								if (New == NIL)
									{
										/* this operation is legal because the directory structure is */
										/* always consistent at all times. */
									 FailurePoint:
										DisposeDirectory(Dir);
										APRINT(("-ReadDirectory failed"));
										return NIL;
									}
								/* make a file specification that references the item */
								Where = (FSSpec*)AllocPtrCanFail(sizeof(FSSpec),"DirFSSpec");
								if (Where == NIL)
									{
									 WhereFailurePoint:
										ReleasePtr((char*)New);
										goto FailurePoint;
									}
								EXECUTE(if (!Eep_RegisterFileSpec((FileSpec*)Where))
									{ReleasePtr((char*)Where); goto WhereFailurePoint;})
								Error = FSMakeFSSpec(VolumeRefNum,DirectoryID,FileName,Where);
								if (Error != noErr)
									{
										APRINT((" ReadDirectory filescan:  Error %s making FSSpec",Error));
										ReleasePtr((char*)New);
										DisposeFileSpec((FileSpec*)Where);
										goto TrySomeMore;
									}
								ERROR(((FSSpec*)Where)->name[0] == 0,PRERR(AllowResume,
									"ReadDirectory:  FSSpec has empty filename"));
								if ((CInfo.hFileInfo.ioFlAttrib & 16/*majiknumber!*/) != 0)
									{
										/* it's a directory */
										APRINT((" ReadDirectory filescan: Directory"));
										New->ItemType = eDirectory;
									}
								 else
									{
										Boolean					TargetWasFolder;
										Boolean					TargetWasAlias;
										FSSpec					WhereCopy;

										/* it's a file, but is it an alias? */
										WhereCopy = *Where; /* we don't want to keep the changes! */
										AUDITFILESPEC(WhereCopy);
										Error = ResolveAliasFile(&WhereCopy,False/*stopafterfirst*/,
											&TargetWasFolder,&TargetWasAlias);
										APRINT((" ResolveAliasFile:  Err = %s, Folder = %b, Alias = %b",
											(short)Error,(short)TargetWasFolder,(short)TargetWasAlias));
										if (TargetWasAlias && (Error == noErr))
											{
												APRINT((" ReadDirectory filescan: Symbolic Link"));
												New->ItemType = eSymbolicLink;
											}
										else
											{
												APRINT((" ReadDirectory filescan: File"));
												New->ItemType = eFile;
											}
									}
								New->WhereIsIt = (FileSpec*)Where;
								New->Next = NIL;
								if (Tail != NIL)
									{
										Tail->Next = New;
									}
								 else
									{
										Dir->List = New;
									}
								Tail = New;
							 TrySomeMore:
								;/* we jump here after weird "Desktop DB" stuff */
							}
						else /* Error != noErr */
							{
								/* error means there are no more items in the directory */
								EnergizerBunny = False;
							}
					}
			}
		 else
			{
				OSErr						Error;
				short						Index;
				Str255					DiskName;
				MyBoolean				EnergizerBunny;
				HParamBlockRec	Params;

				/* root directory.  We fake up a directory containing all of the */
				/* mounted volumes, kind of like under UNIX, so that it looks like */
				/* there is only one huge file system. */
				EnergizerBunny = True;
				Index = 0;
				while (EnergizerBunny)
					{
						Index += 1; /* index started at 0 up there, so we increment first */
						Params.volumeParam.ioNamePtr = DiskName;
						Params.volumeParam.ioVRefNum = 0;  /* says "use the index" */
						Params.volumeParam.ioVolIndex = Index;
						Error = PBHGetVInfo(&Params,False/*synchronous*/);
						if (Error == noErr)
							{
								DirEntry*			New;
								FSSpec*				Where;

								/* add this "directory" to the list */
								APRINT((" ReadDirectory volscan: %p",DiskName));
								New = (DirEntry*)AllocPtrCanFail(sizeof(DirEntry),"DirEntry");
								if (New == NIL)
									{
										/* this operation is legal because the directory structure is */
										/* always consistent at all times. */
									 DeathPoint:
										DisposeDirectory(Dir);
										APRINT(("-ReadDirectory failed"));
										return NIL;
									}
								Where = (FSSpec*)AllocPtrCanFail(sizeof(FSSpec),"FSSpec");
								if (Where == NIL)
									{
									 AnotherWhereFailurePoint:
										ReleasePtr((char*)New);
										goto DeathPoint;
									}
								EXECUTE(if (!Eep_RegisterFileSpec((FileSpec*)Where))
									{ReleasePtr((char*)Where); goto AnotherWhereFailurePoint;})
								FSMakeFSSpec(Params.volumeParam.ioVRefNum,0/*don'tknowroot*/,
									DiskName,Where);
								New->WhereIsIt = (FileSpec*)Where;
								New->ItemType = eDirectory;
								New->Next = NIL;
								if (Tail != NIL)
									{
										Tail->Next = New;
									}
								 else
									{
										Dir->List = New;
									}
								Tail = New;
							}
						 else
							{
								/* error -- no more volumes mounted.  We is done */
								EnergizerBunny = False;
							}
					}
			}
		return Dir;
		APRINT(("-ReadDirectory %xl",Dir));
	}


/* get rid of the directory structure when we are done with it */
void							DisposeDirectory(DirectoryRec* Dir)
	{
		CheckPtrExistence(Dir);
		while (Dir->List != NIL)
			{
				DirEntry*			Temp;

				Temp = Dir->List;
				Dir->List = Dir->List->Next;
				DisposeFileSpec(Temp->WhereIsIt);
				ReleasePtr((char*)Temp);
			}
		ReleasePtr((char*)Dir);
	}


/* find out how many entries there are in the directory structure */
long							GetDirectorySize(DirectoryRec* Dir)
	{
		DirEntry*				Scan;
		long						Count;

		CheckPtrExistence(Dir);
		Count = 0;
		Scan = Dir->List;
		while (Scan != NIL)
			{
				Count += 1;
				Scan = Scan->Next;
			}
		return Count;
	}


/* return the item type of an indexed directory entry (indices start from 0) */
/* Indices start from 0 up to GetDirectorySize() - 1 */
DirFileTypes			GetDirectoryEntryType(DirectoryRec* Dir, long Index)
	{
		DirEntry*				Scan;
		long						Count;

		CheckPtrExistence(Dir);
		Count = 0;
		Scan = Dir->List;
		while (Scan != NIL)
			{
				if (Count == Index)
					{
						return Scan->ItemType;
					}
				Count += 1;
				Scan = Scan->Next;
			}
		EXECUTE(PRERR(ForceAbort,"GetDirectoryEntryType:  Index is out of range"));
	}


/* return a handle containing the name of the specified directory entry or NIL */
/* if allocation failed */
char*							GetDirectoryEntryName(DirectoryRec* Dir, long Index)
	{
		DirEntry*				Scan;
		long						Count;

		CheckPtrExistence(Dir);
		Count = 0;
		Scan = Dir->List;
		while (Scan != NIL)
			{
				if (Count == Index)
					{
						char*						NamePtr;

						NamePtr = ExtractFileName(Scan->WhereIsIt);
						if (NamePtr == NIL)
							{
								return NIL;
							}
						return NamePtr;
					}
				Count += 1;
				Scan = Scan->Next;
			}
		EXECUTE(PRERR(ForceAbort,"GetDirectoryEntryName:  Index is out of range"));
	}


/* get a file spec describing a directory entry */
/* this entry is a standard FileSpec, the same type as used in the Files module */
/* and should be manipulated and disposed using routines from Files */
FileSpec*					GetDirectoryEntryFileSpec(DirectoryRec* Dir, long Index)
	{
		DirEntry*				Scan;
		long						Count;

		CheckPtrExistence(Dir);
		Count = 0;
		Scan = Dir->List;
		while (Scan != NIL)
			{
				if (Count == Index)
					{
						return DuplicateFileSpec(Scan->WhereIsIt);
					}
				Count += 1;
				Scan = Scan->Next;
			}
		EXECUTE(PRERR(ForceAbort,"GetDirectoryEntryFileSpec:  Index is out of range"));
	}


typedef enum {eLessThan EXECUTE(= -4521), eEqualTo, eGreaterThan} SortTypes;

/* local routine for comparing the directories */
static SortTypes	NoCaseSort2(char* Left, char* Right, long MaxCount)
	{
		unsigned char		LeftTemp;
		unsigned char		RightTemp;

	 LoopPoint:
		if (((*Left == 0) && (*Right == 0)) || (MaxCount == 0))
			{
				return eEqualTo;
			}
		LeftTemp = *Left;
		if ((LeftTemp >= 'A') && (LeftTemp <= 'Z'))
			{
				LeftTemp = LeftTemp - 'A' + 'a';
			}
		RightTemp = *Right;
		if ((RightTemp >= 'A') && (RightTemp <= 'Z'))
			{
				RightTemp = RightTemp - 'A' + 'a';
			}
		if (LeftTemp < RightTemp)
			{
				return eLessThan;
			}
		if (LeftTemp > RightTemp)
			{
				return eGreaterThan;
			}
		Left += 1;
		Right += 1;
		MaxCount -= 1;
		goto LoopPoint;
	}

static SortTypes	NoCaseSort(char* Left, char* Right)
	{
		unsigned char		LeftTemp;
		unsigned char		RightTemp;
		long						MaxCount;

		MaxCount = PtrSize(Left);
		if (PtrSize(Right) < MaxCount)
			{
				MaxCount = PtrSize(Right);
			}
	 LoopPoint:
		if (((*Left == 0) && (*Right == 0)) || (MaxCount == 0))
			{
				return eEqualTo;
			}
		LeftTemp = *Left;
		if ((LeftTemp >= 'A') && (LeftTemp <= 'Z'))
			{
				LeftTemp = LeftTemp - 'A' + 'a';
			}
		RightTemp = *Right;
		if ((RightTemp >= 'A') && (RightTemp <= 'Z'))
			{
				RightTemp = RightTemp - 'A' + 'a';
			}
		if (LeftTemp < RightTemp)
			{
				return eLessThan;
			}
		if (LeftTemp > RightTemp)
			{
				return eGreaterThan;
			}
		Left += 1;
		Right += 1;
		MaxCount -= 1;
		goto LoopPoint;
	}


/* resort the directory alphabetically.  Returns True if it succeeded or */
/* False if it failed. */
MyBoolean					ResortDirectory(DirectoryRec* Dir)
	{
		DirEntry*				OldThangs;

		CheckPtrExistence(Dir);
		OldThangs = Dir->List;
		if (OldThangs == NIL)
			{
				return True; /* hey, nothing to sort */
			}
		Dir->List = OldThangs; /* first item doesn't sort anyway, so putting it on */
		OldThangs = OldThangs->Next; /* right away simplifies things later... */
		Dir->List->Next = NIL;
		/* ...since we can always assume Dir->List != NIL */
		while (OldThangs != NIL) /* yup, a good olde n^2 insertion sort */
			{
				DirEntry*				OurItem;
				DirEntry*				DirScan;
				DirEntry*				DirLag;
				char*						ItemName;
				char*						ScanName;

				OurItem = OldThangs;
				OldThangs = OldThangs->Next; /* advance before we destroy .Next */
				ItemName = ExtractFileName(OurItem->WhereIsIt);
				if (ItemName == NIL)
					{
					 FailurePoint1:
						return False;
					}
				ERROR(PtrSize(ItemName) == 0,PRERR(AllowResume,
					"ResortDirectory:  An item's FSSpec has an empty name"));
				DirLag = NIL;
				DirScan = Dir->List;
				while (DirScan != NIL)
					{
						SortTypes					CompareResult;

						ScanName = ExtractFileName(DirScan->WhereIsIt);
						if (ScanName == NIL)
							{
								ReleasePtr(ItemName);
								goto FailurePoint1;
							}
						ERROR(PtrSize(ScanName) == 0,PRERR(AllowResume,
							"ResortDirectory:  An item's FSSpec has an empty name"));
						CompareResult = NoCaseSort(ItemName,ScanName);
						ReleasePtr(ScanName);
						switch (CompareResult)
							{
								case eLessThan:
									/* our item comes first, insert */
									OurItem->Next = DirScan;
									if (DirLag != NIL)
										{
											DirLag->Next = OurItem;
										}
									 else
										{
											Dir->List = OurItem;
										}
									goto ExitInnerLoopSortPoint;
									break;
								case eEqualTo:
									/* we continue to next one so that this is a stable sort. */
									break;
								case eGreaterThan:
									/* we continue to next one. */
									break;
							}
						DirLag = DirScan;
						DirScan = DirScan->Next;
					}
				/* if we got all the way through, then tack it on the end */
				ERROR(DirLag == NIL,PRERR(ForceAbort,"ResortDirectory:  Internal error"));
				OurItem->Next = DirLag->Next; /* DirLag != NIL; see outer while comment */
				DirLag->Next = OurItem;
			 ExitInnerLoopSortPoint: /* exited from inner loop since we were able to insert */
				ReleasePtr(ItemName);
			}
	}


/* this compares to file specifications and returns True if they refer to the */
/* same file */
MyBoolean					CompareFileSpecs(FileSpec* First, FileSpec* Second)
	{
		CheckPtrExistence(First);
		CheckPtrExistence(Second);
		ValidateFileSpec(First);
		ValidateFileSpec(Second);
		if ((((FSSpec*)First)->vRefNum != ((FSSpec*)Second)->vRefNum)
			|| (((FSSpec*)First)->parID != ((FSSpec*)Second)->parID))
			{
				return False;
			}
		if (((FSSpec*)First)->name[0] != ((FSSpec*)Second)->name[0])
			{
				return False;
			}
		/* remember to use a case sensitive compare for UNIX! */
		return (eEqualTo == NoCaseSort2((char*)&(((FSSpec*)First)->name[1]),
			(char*)&(((FSSpec*)Second)->name[1]),((FSSpec*)First)->name[0]));
	}


/* dereference a symbolic link one level only */
FileSpec*					DereferenceSymbolicLink(FileSpec* Source)
	{
		FSSpec*					New;
		OSErr						Error;
		Boolean					TargetWasFolder;
		Boolean					TargetWasAlias;

		APRINT(("+DereferenceSymbolicLink %r",Source));
		CheckPtrExistence(Source);
		ValidateFileSpec(Source);
		New = (FSSpec*)AllocPtrCanFail(sizeof(FSSpec),"FSSpec");
		*New = *(FSSpec*)Source;
		if (New == NIL)
			{
				APRINT(("-DereferenceSymbolicLink Failed"));
				return NIL;
			}
		AUDITFILESPEC(*New);
		Error = ResolveAliasFile(New,False/*onelevel*/,&TargetWasFolder,&TargetWasAlias);
		APRINT((" ResolveAliasFile:  Err = %s, Folder = %b, Alias = %b",(short)Error,
			(short)TargetWasFolder,(short)TargetWasAlias));
		if (Error == noErr)
			{
				if (TargetWasAlias)
					{
						EXECUTE(if (!Eep_RegisterFileSpec((FileSpec*)New))
							{ReleasePtr((char*)New); New = NIL;})
						/* it was an alias */
						APRINT(("-DereferenceSymbolicLink %r",New));
						return (FileSpec*)New;
					}
			}
		/* wasn't an alias, so we return NIL */
		ReleasePtr((char*)New); /* hasn't been registered yet */
		APRINT(("-DereferenceSymbolicLink NIL"));
		return NIL;
	}


/* get root file specification.  On UNIX, this would return "/"; on Macintosh, */
/* it returns a bogus file descriptor */
FileSpec*					GetRootFileSpec(void)
	{
		FSSpec*						Thang;

		Thang = (FSSpec*)AllocPtrCanFail(sizeof(FSSpec),"RootFileSpec");
		if (Thang != NIL)
			{
				Thang->vRefNum = 0;
				Thang->parID = 0;
				Thang->name[0] = 0;
				if (!Eep_RegisterFileSpec((FileSpec*)Thang))
					{
						ReleasePtr((char*)Thang);
						Thang = NIL;
					}
			}
		return (FileSpec*)Thang;
	}


/* get statistics for a file */
MyBoolean					GetFileStatistics(struct FileSpec* File, FileInfoRec* InfoOut)
	{
		FInfo						FinderInfo;
		CInfoPBRec			InfoPB;
		OSErr						Error;
		DateTimeRec			When;

		CheckPtrExistence(File);
		ValidateFileSpec(File);
		FSpGetFInfo((FSSpec*)File,&FinderInfo);
		InfoOut->CreatorCode = FinderInfo.fdCreator;
		InfoOut->FileTypeCode = FinderInfo.fdType;
		InfoPB.hFileInfo.ioVRefNum = ((FSSpec*)File)->vRefNum;
		InfoPB.hFileInfo.ioNamePtr = ((FSSpec*)File)->name;
		InfoPB.hFileInfo.ioDirID = ((FSSpec*)File)->parID;
		InfoPB.hFileInfo.ioFDirIndex = 0;
		Error = PBGetCatInfo(&InfoPB,False);
		if (Error != noErr)
			{
				return False;
			}
		Secs2Date(InfoPB.hFileInfo.ioFlCrDat,&When);
		InfoOut->CreationDate.Year = When.year;
		InfoOut->CreationDate.Month = When.month - 1;
		InfoOut->CreationDate.Day = When.day - 1;
		InfoOut->CreationDate.Hour = When.hour;
		InfoOut->CreationDate.Minute = When.minute;
		InfoOut->CreationDate.Second = When.second;
		InfoOut->CreationDate.DayOfTheWeek = When.dayOfWeek - 1;
		Secs2Date(InfoPB.hFileInfo.ioFlMdDat,&When);
		InfoOut->LastModificationDate.Year = When.year;
		InfoOut->LastModificationDate.Month = When.month - 1;
		InfoOut->LastModificationDate.Day = When.day - 1;
		InfoOut->LastModificationDate.Hour = When.hour;
		InfoOut->LastModificationDate.Minute = When.minute;
		InfoOut->LastModificationDate.Second = When.second;
		InfoOut->LastModificationDate.DayOfTheWeek = When.dayOfWeek - 1;
		return True;
	}


/* create a new directory with the specified file specification. */
MyBoolean					CreateNewDirectory(struct FileSpec* DirLocation)
	{
		long						NewDirID;

		CheckPtrExistence(DirLocation);
		ValidateFileSpec(DirLocation);
		return (noErr == FSpDirCreate((FSSpec*)DirLocation,smSystemScript,&NewDirID));
	}


/* find out of the specified file specification is a directory */
MyBoolean					IsTheFileSpecADirectory(struct FileSpec* Spec)
	{
		OSErr					Error;
		Boolean				TargetWasFolder;
		Boolean				TargetWasAlias;
		FSSpec				Copy;

		APRINT(("+IsTheFileSpecADirectory %r",Spec));
		CheckPtrExistence(Spec);
		ValidateFileSpec(Spec);
		Copy = *(FSSpec*)Spec;
		AUDITFILESPEC(Copy);
		Error = ResolveAliasFile(&Copy,True/*all of them*/,&TargetWasFolder,&TargetWasAlias);
		APRINT((" ResolveAliasFile:  Err = %s, Folder = %b, Alias = %b",(short)Error,
			(short)TargetWasFolder,(short)TargetWasAlias));
		if (Error == fnfErr)
			{
				/* assume it's a volume. */
				APRINT(("-IsTheFileSpecADirectory True (assumed volume)"));
				return True;
			}
		APRINT(("-IsTheFileSpecADirectory %b",(short)TargetWasFolder));
		return TargetWasFolder;
	}


/* find out if the specified file is a symbolic link */
MyBoolean					IsTheFileSpecASymbolicLink(struct FileSpec* Spec)
	{
		OSErr					Error;
		Boolean				TargetWasFolder;
		Boolean				TargetWasAlias;
		FSSpec				Copy;

		APRINT(("+IsTheFileSpecASymbolicLink %r",Spec));
		CheckPtrExistence(Spec);
		ValidateFileSpec(Spec);
		Copy = *(FSSpec*)Spec;
		AUDITFILESPEC(Copy);
		Error = ResolveAliasFile(&Copy,False/*one of them*/,&TargetWasFolder,&TargetWasAlias);
		APRINT((" ResolveAliasFile:  Err = %s, Folder = %b, Alias = %b",(short)Error,
			(short)TargetWasFolder,(short)TargetWasAlias));
		if (Error == fnfErr)
			{
				/* assume it's a volume. */
				APRINT(("-IsTheFileSpecASymbolicLink False (assumed volume)"));
				return False;
			}
		APRINT(("-IsTheFileSpecASymbolicLink %b",(short)TargetWasAlias));
		return TargetWasAlias;
	}


/* obtain a file spec for a file inside of the specified directory.  the file */
/* name must be a non-null-terminated heap block */
struct FileSpec*	FileSpecForFileInDirectory(struct FileSpec* DirLocation,
										char* Filename)
	{
		FSSpec*					Result;
		OSErr						Error;
		Boolean					TargetWasFolder;
		Boolean					TargetWasAlias;
		FSSpec					Copy;
		long						DirID;
		Str63						Name;
		long						Scan;
		long						Limit;

		APRINT(("+FileSpecForFileInDirectory Dir=%r, Name=%r",DirLocation,Filename));

		CheckPtrExistence(DirLocation);
		ValidateFileSpec(DirLocation);
		CheckPtrExistence(Filename);

		Result = (FSSpec*)AllocPtrCanFail(sizeof(FSSpec),"FileSpecForFileInDirectory FSSpec");
		if (Result == NIL)
			{
			 FailurePoint1:
				APRINT(("-FileSpecForFileInDirectory failed"));
				return NIL;
			}
		Copy = *(FSSpec*)DirLocation;
		AUDITFILESPEC(Copy);
		Error = ResolveAliasFile(&Copy,True/*all of them*/,&TargetWasFolder,&TargetWasAlias);
		APRINT((" ResolveAliasFile:  Err = %s, Folder = %b, Alias = %b",(short)Error,
			(short)TargetWasFolder,(short)TargetWasAlias));
		if (Error == fnfErr)
			{
				/* assume it's a volume. */
			 FailurePoint2:
				ReleasePtr((char*)Result);
				goto FailurePoint1;
			}
		if (!TargetWasFolder)
			{
			 FailurePoint3:
				goto FailurePoint2;
			}
		if (noErr != FDerefFolder(&Copy,&DirID))
			{
			 FailurePoint4:
				goto FailurePoint3;
			}
		Limit = PtrSize(Filename);
		if (Limit > 31)
			{
				Limit = 31;
			}
		Name[0] = Limit;
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				Name[1 + Scan] = Filename[Scan];
			}
		Error = FSMakeFSSpec(((FSSpec*)DirLocation)->vRefNum,DirID,Name,Result);
		APRINT((" ResolveAliasFile:  FSMakeFSSpec returned %s",(short)Error));
		if ((noErr != Error) && (fnfErr != Error))
			{
				/* file not found is natural, since they're probably creating it. */
			 FailurePoint5:
				goto FailurePoint4;
			}
		if (!Eep_RegisterFileSpec((FileSpec*)Result))
			{
			 FailurePoint6:
				goto FailurePoint5;
			}

		APRINT(("-FileSpecForFileInDirectory %r",Result));
		return (FileSpec*)Result;
	}
