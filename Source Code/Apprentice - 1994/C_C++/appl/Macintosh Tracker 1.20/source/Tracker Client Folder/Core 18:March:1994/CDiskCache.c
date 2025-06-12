/* CDiskCache.c */

#include "CDiskCache.h"
#include "MiscInfo.h"
#include "CSack.h"
#include "Error.h"
#include "WatchCursor.h"


#define IDLETIME (60*2)
#define FILEFLUSHTHRESHHOLD (60*5)
#define FREEBLOCKNODESIZE (sizeof(FreeBlockEntry) * 96)

/* minimum number of free bytes required before file is compacted */
/* should be declared in MiscInfo.h */
#define MinFreeSpace (DiskCacheMinFreeSpace)

/* maximum percentage of total file that can be free before file is compacted */
/* should be declared in MiscInfo.h */
#define MaxFreeFraction (DiskCacheMaxFreeFraction)


static CDiskCache*	LastCachePurged = NIL; /* must be generally available */

static IndexType		CDiskCache::LastIndexPurged = 0;

static MyBoolean		CDiskCache::GrowZoneInstalled = False;


#ifdef MEMDEBUG
	static void	SetDiskTag(Handle TheHandle, IndexType TheIndex)
		{
			char		Temp[10] = "CDC$xxxxxx";
			char		Map[16] = "0123456789abcdef";
			short		Scan;

			for (Scan = 5; Scan >= 0; Scan -= 1)
				{
					Temp[Scan + 4] = Map[(TheIndex >> ((5-Scan)*4)) & 0x0f];
				}
			SetTag(TheHandle,Temp);
		}
#else
	#define SetDiskTag(TheHandle,TheIndex)
#endif


/* initialize the disk cache's parameters */
/* */					CDiskCache::CDiskCache()
	{
		CSack*		Temp;

		/* if we use the disk caches, provide an error recovery function */
		SetErrorFunction(&SystemErrorFlushAllDiskCaches);
		EXECUTE(DiskCacheInitFlag = False);
		if (!GrowZoneInstalled)
			{
				InstallGrowZone(&MyGrowZone);
				GrowZoneInstalled = True;
			}
		HashTableHandle = (HashTableEntry**)AllocHandle(0);
		SetTag(HashTableHandle,"CDC HashTable");
		TotalSize = 0;
		FreeSpace = 0;
		NumBlocks = 0;
		MaxIndex = 0;
		FileModified = False;
		FreeHashEntry = NIL;
		Temp = new CSack;
		Temp->ISack(sizeof(FreeBlockEntry),FREEBLOCKNODESIZE);
		FreeBlockList = Temp;
		this->NextCache = NIL;
		this->PreviousCache = NIL; /* initially isolated */
	}


/* remove this disk cache from the system */
/* */					CDiskCache::~CDiskCache()
	{
		long		Index;

		ERROR(!DiskCacheInitFlag,PRERR(ForceAbort,
			"CDiskCache::~CDiskCache called on uninitialized object."));
		if (this->NextCache != NIL)
			{
				FlushAll(DoFlushHeldBlocks);  /* save all data back to disk */
				CompactFile();  /* compress file on disk */
				FlushVitalStats(); /* rewrite crucial file information */
				if (LastCachePurged == this)
					{
						LastCachePurged = this->NextCache;
					}
				this->PreviousCache->NextCache = this->NextCache;
				this->NextCache->PreviousCache = this->PreviousCache; /* delink us from list */
				if (LastCachePurged == this)
					{
						/* this happens when we are the last cache */
						LastCachePurged = NIL;
					}
			}
		FCloseFile(FileReference); /* close the file */
		for (Index = (HandleSize((Handle)HashTableHandle) / sizeof(HashTableEntry)) - 1;
			Index >= 0; Index -= 1)
			{
				/* disposing of hash table entries */
				HRNGCHK(HashTableHandle,&((*HashTableHandle)[Index]),sizeof(HashTableEntry));
				ReleasePtr((Ptr)(*HashTableHandle)[Index].SlicePtr);
			}
		ReleaseHandle((Handle)HashTableHandle);
		if (DeleteWhenFinished)
			{
				FSSpec		Temp;

				Temp = DiskFile;
				FDeleteIfExistsFile(&Temp);
			}
		delete FreeBlockList;
		DeregisterIdler(this);
	}


/* create or open a new disk cache */
short		CDiskCache::IDiskCache(FSSpec* FileInfo, short Mode, OSType FileType)
	{
		PString		Temp;
		FSSpec		LocalFSSpec;
		short			VRefNum;
		long			DirID;
		short			Error;
		short			OtherFile;
		short			MyFile;
		long			EOF;

		ERROR(DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::IDiskCache called twice for same object."));
		EXECUTE(DiskCacheInitFlag=True);
		switch (Mode)
			{
				case CreateTemporary:
					FFindTempFolder(&VRefNum,&DirID);
					FMakeTempFileName(VRefNum,DirID,Temp);
					FMakeFSSpec(VRefNum,DirID,Temp,&LocalFSSpec);
					Error = FCreate(&LocalFSSpec,CREATORCODE,FileType);
					Error |= FOpenFile(&LocalFSSpec,&MyFile); /* preserve all disk errors */
					DiskFile = LocalFSSpec;
					DeleteWhenFinished = True;
					break;
				case OpenExisting:
					Error = FOpenFile(FileInfo,&MyFile);
					DiskFile = *FileInfo;
					DeleteWhenFinished = False;
					break;
				case CreatePermanent:
					FDeleteIfExistsFile(FileInfo);
					Error = FCreate(FileInfo,CREATORCODE,FileType);
					Error |= FOpenFile(FileInfo,&MyFile);
					DiskFile = *FileInfo;
					DeleteWhenFinished = False;
					break;
				case CopyToTemporaryAndOpen:
					FFindTempFolder(&VRefNum,&DirID);
					FMakeTempFileName(VRefNum,DirID,Temp);
					FMakeFSSpec(VRefNum,DirID,Temp,&LocalFSSpec);
					Error = FCreate(&LocalFSSpec,CREATORCODE,FileType);
					Error |= FOpenFile(&LocalFSSpec,&MyFile);
					DiskFile = LocalFSSpec;
					DeleteWhenFinished = True;
					Error |= FOpenFile(FileInfo,&OtherFile);
					Error |= FGetEOF(OtherFile,&EOF);
					FMoveData(OtherFile,0,MyFile,0,EOF);
					FCloseFile(OtherFile);
					break;
				default:
					PRERR(ForceAbort,"Illegal option passed to CDiskCache::IDiskCache.");
			}
		FileReference = MyFile;
		if (FLastError() != noErr)
			{
				SetErrorStatus();
				EXECUTE(PRERR(AllowResume,"CDiskCache::IDiskCache disk error occurred."));
				return DiskError;
			}
		if (!InitAll())
			{
				SetErrorStatus();
				EXECUTE(PRERR(AllowResume,
					"CDiskCache::IDiskCache file corrupted error occurred."));
				return FileCorrupted;
			}
		/* finally, linking it into the list of other objects */
		/* since the system error handler uses this list to save unsaved documents, */
		/* we don't want to link it in until we know everything is OK. */
		if (LastCachePurged == NIL)
			{
				/* this is the first cache allocated */
				this->NextCache = this;
				this->PreviousCache = this; /* link to yourself */
				LastCachePurged = this;
			}
		 else
			{
				/* there are others to link to */
				this->NextCache = LastCachePurged;
				this->PreviousCache = LastCachePurged->PreviousCache; /* our links set */
				LastCachePurged->PreviousCache->NextCache = this;
				LastCachePurged->PreviousCache = this;  /* other links set */
			}
		FileModified = True;
		RegisterIdler(this,IDLETIME);
		return noErr;
	}


/* save file, compress it, and then duplicate it into the specified location */
MyBoolean			CDiskCache::SaveAs(FSSpec* WhereToSave, ulong Creator, ulong FileType)
	{
		short		OtherFile;
		long		EOF;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::SaveAs called on uninitialized object."));
		FlushAll(DontFlushHeldBlocks);
		CompactFile();
		FDeleteIfExistsFile(WhereToSave);
		if (FCreate(WhereToSave,Creator,FileType) != noErr)
			{
				EXECUTE(PRERR(AllowResume,"CDiskCache::SaveAs create file error occurred."));
				return False;
			}
		if (FOpenFile(WhereToSave,&OtherFile) != noErr)
			{
				EXECUTE(PRERR(AllowResume,"CDiskCache::SaveAs open file error occurred."));
				return False;
			}
		if (FGetEOF(FileReference,&EOF))
			{
				EXECUTE(PRERR(AllowResume,"CDiskCache::SaveAs couldn't get EOF position."));
				return False;
			}
		if (FMoveData(FileReference,0,OtherFile,0,EOF) != noErr)
			{
				FCloseFile(OtherFile);
				EXECUTE(PRERR(AllowResume,"CDiskCache::SaveAs move data error occurred."));
				return False;
			}
		if (FCloseFile(OtherFile) != noErr)
			{
				EXECUTE(PRERR(AllowResume,"CDiskCache::SaveAs close file error occurred."));
				return False;
			}
		return True;
	}


/* attempt to allocate a new disk object */
MyBoolean			CDiskCache::DiskNew(long BlockSize, IndexType* IndexOut)
	{
		IndexType				Index;
		long						Offset;
		register HashSliceEntry*	HSE;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::DiskNew called on uninitialized object."));
		Index = FindFreeHashEntry();
		if (Index == UnallocatedIndex)
			{
				return False; /* allocation failed */
			}
		if (!AllocateDiskBlock(BlockSize,&Offset,Index))
			{
				SetErrorStatus();
				EXECUTE(PRERR(AllowResume,
					"CDiskCache::DiskNew allocate disk block error occurred."));
				return False;
			}
		HRNGCHK(HashTableHandle,&((*HashTableHandle)[Index/HASHSLICESIZE]),
			sizeof(HashTableEntry));
		HSE = &((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]);
		PRNGCHK((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr,
			&((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]),
			sizeof(HashSliceEntry));
		HSE->HandleToData = NIL;
		HSE->UsedCell = True;
		HSE->Changed = False;
		HSE->FileOffset = Offset;
		HSE->NoPurge = False;
		*IndexOut = Index;
		return True;
	}


/* dispose of a disk thing */
void					CDiskCache::DiskDispose(IndexType Index)
	{
		long						Offset;
		register HashSliceEntry*	HSE;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::DiskDispose called on uninitialized object."));
		ERROR((((Index/HASHSLICESIZE)*sizeof(HashTableEntry) >=
			HandleSize((Handle)HashTableHandle)) || ((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK].UsedCell == False)),
			PRERR(ForceAbort,"CDiskCache::DiskDispose called on nonexistent block."));

		HSE = &((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK]);
		HRNGCHK(HashTableHandle,&((*HashTableHandle)[Index/HASHSLICESIZE]),
			sizeof(HashTableEntry));
		PRNGCHK((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr,
			&((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]),
			sizeof(HashSliceEntry));
		Offset = HSE->FileOffset;
		if (HSE->HandleToData != NIL)
			{
				ReleaseHandle(HSE->HandleToData);
			}
		ReleaseHashEntry(Index);
		ReleaseDiskBlock(Offset);
		if (((FreeSpace >> 5)/(TotalSize) > (long)((1 << 5)*MaxFreeFraction))
			&& (FreeSpace > MinFreeSpace))
			{
				CompactFile();
			}
	}


/* load a block from disk, if it isn't already loaded */
MyBoolean			CDiskCache::DiskLoad(IndexType Index)
	{
		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::DiskLoad called on uninitialized object."));
		ERROR((((Index/HASHSLICESIZE)*sizeof(HashTableEntry) >=
			HandleSize((Handle)HashTableHandle)) || ((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK].UsedCell == False)),
			PRERR(ForceAbort,"CDiskCache::DiskLoad called on nonexistent block."));

		HRNGCHK(HashTableHandle,&((*HashTableHandle)[Index/HASHSLICESIZE]),
			sizeof(HashTableEntry));
		PRNGCHK((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr,
			&((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]),
			sizeof(HashSliceEntry));
		return SwapBlockIn(&((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK]));
	}


/* load an object and prevent it from being purged out */
MyBoolean			CDiskCache::DiskHold(IndexType Index)
	{
		register HashSliceEntry*		HSE;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::DiskHold called on uninitialized object."));
		ERROR((((Index/HASHSLICESIZE)*sizeof(HashTableEntry) >=
			HandleSize((Handle)HashTableHandle)) || ((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK].UsedCell == False)),
			PRERR(ForceAbort,"CDiskCache::DiskHold called on nonexistent block."));

		HRNGCHK(HashTableHandle,&((*HashTableHandle)[Index/HASHSLICESIZE]),
			sizeof(HashTableEntry));
		HSE = &((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]);
		PRNGCHK((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr,HSE,sizeof(HashSliceEntry));
		HSE->NoPurge = True;
		return SwapBlockIn(HSE); /* SwapBlockIn checks NoPurge and Changed flags */
	}


/* allow an object to be purged out */
void					CDiskCache::DiskUnhold(IndexType Index)
	{
		register HashSliceEntry*	HSE;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::DiskUnhold called on uninitialized object."));
		ERROR((((Index/HASHSLICESIZE)*sizeof(HashTableEntry) >=
			HandleSize((Handle)HashTableHandle)) || ((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK].UsedCell == False)),
			PRERR(ForceAbort,"CDiskCache::DiskUnhold called on nonexistent block."));

		HRNGCHK(HashTableHandle,&((*HashTableHandle)[Index/HASHSLICESIZE]),
			sizeof(HashTableEntry));
		HSE = &((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]);
		PRNGCHK((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr,HSE,sizeof(HashSliceEntry));
		HSE->NoPurge = False;
		SwapBlockIn(HSE); /* block's already in, this just checks NoPurge & Changed */
	}


/* purge an object */
void					CDiskCache::DiskUnload(IndexType Index)
	{
		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::DiskUnload called on uninitialized object."));
		ERROR((((Index/HASHSLICESIZE)*sizeof(HashTableEntry) >=
			HandleSize((Handle)HashTableHandle)) || ((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK].UsedCell == False)),
			PRERR(ForceAbort,"CDiskCache::DiskUnload called on nonexistent block."));

		HRNGCHK(HashTableHandle,&((*HashTableHandle)[Index/HASHSLICESIZE]),
			sizeof(HashTableEntry));
		PRNGCHK((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr,
			&((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]),
			sizeof(HashSliceEntry));
		SwapBlockOut(&((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK]));
	}


/* change the size of an object */
MyBoolean			CDiskCache::DiskResize(long NewBlockSize, IndexType Index)
	{
		long						NewFileOffset;
		FileBlockNode		BlockHeader;
		register HashSliceEntry*	HSE;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::DiskResize called on uninitialized object."));
		ERROR((((Index/HASHSLICESIZE)*sizeof(HashTableEntry) >=
			HandleSize((Handle)HashTableHandle)) || ((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK].UsedCell == False)),
			PRERR(ForceAbort,"CDiskCache::DiskResize called on nonexistent block."));

		HRNGCHK(HashTableHandle,&((*HashTableHandle)[Index/HASHSLICESIZE]),
			sizeof(HashTableEntry));
		PRNGCHK((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr,
			&((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]),
			sizeof(HashSliceEntry));
		HSE = &((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK]);
		SwapBlockOut(HSE);
		FSetFilePos(FileReference,HSE->FileOffset);
		FReadBlock(FileReference,(char*)&BlockHeader,sizeof(FileBlockNode));
		if (!AllocateDiskBlock(NewBlockSize,&NewFileOffset,Index))
			{
				SetErrorStatus();
				EXECUTE(PRERR(AllowResume,
					"CDiskCache::Resize allocate disk block error occurred."));
				return False;
			}
		FMoveData(FileReference,HSE->FileOffset + sizeof(FileBlockNode),
			FileReference,NewFileOffset + sizeof(FileBlockNode),BlockHeader.Size
			- BlockHeader.Correction - sizeof(FileBlockNode));
		ReleaseDiskBlock(HSE->FileOffset);
		HSE->FileOffset = NewFileOffset;
		return True;
	}


/* mark an index so that it should be saved to disk */
void					CDiskCache::DiskChanged(IndexType Index)
	{
		register HashSliceEntry*	HSE;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::DiskChanged called on uninitialized object."));
		ERROR((((Index/HASHSLICESIZE)*sizeof(HashTableEntry) >=
			HandleSize((Handle)HashTableHandle)) || ((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK].UsedCell == False)),
			PRERR(ForceAbort,"CDiskCache::DiskChanged called on nonexistent block."));

		HRNGCHK(HashTableHandle,&((*HashTableHandle)[Index/HASHSLICESIZE]),
			sizeof(HashTableEntry));
		PRNGCHK((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr,
			&((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]),
			sizeof(HashSliceEntry));
		HSE = &((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]);
		ERROR(HSE->HandleToData == NIL,
			PRERR(AllowResume,"CDiskCache::DiskChanged called on an unloaded block."));
		HSE->Changed = True;
		SwapBlockIn(HSE); /* just adjusts the handle's NoPurge status */
	}


/* save a block to disk, but don't unload it */
/* saves ONLY if changed flag is set */
void					CDiskCache::DiskSave(IndexType Index)
	{
		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::DiskSave called on uninitialized object."));
		ERROR((((Index/HASHSLICESIZE)*sizeof(HashTableEntry) >=
			HandleSize((Handle)HashTableHandle)) || ((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK].UsedCell == False)),
			PRERR(ForceAbort,"CDiskCache::DiskSave called on nonexistent block."));

		HRNGCHK(HashTableHandle,&((*HashTableHandle)[Index/HASHSLICESIZE]),
			sizeof(HashTableEntry));
		PRNGCHK((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr,
			&((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]),
			sizeof(HashSliceEntry));
		SaveBlock(&((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]));
	}


/* take a handle and make it part of the file */
MyBoolean			CDiskCache::AttachHandle(Handle ObjToAttach, IndexType* IndexOut)
	{
		IndexType		Index;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::AttachHandle called on uninitialized object."));
		if (!DiskNew(HandleSize(ObjToAttach),&Index))
			{
				return False;
			}
		 else
			{
				register HashSliceEntry*	HSE;

				HRNGCHK(HashTableHandle,&((*HashTableHandle)[Index/HASHSLICESIZE]),
					sizeof(HashTableEntry));
				PRNGCHK((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr,
					&((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]),
					sizeof(HashSliceEntry));
				HSE = &((*HashTableHandle)[Index/HASHSLICESIZE]
					.SlicePtr[Index & HASHSLICEMASK]);
				HSE->HandleToData = ObjToAttach;
				HSE->Changed = True;
				HNoPurge(ObjToAttach);
				*IndexOut = Index;
				return True;
			}
	}


/* make a copy of data and return the handle to it */
Handle				CDiskCache::Detach(IndexType Index)
	{
		Handle					Temp;
		register HashSliceEntry*	HSE;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::Detach called on uninitialized object."));
		ERROR((((Index/HASHSLICESIZE)*sizeof(HashTableEntry) >=
			HandleSize((Handle)HashTableHandle)) || ((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK].UsedCell == False)),
			PRERR(ForceAbort,"CDiskCache::Detach called on nonexistent block."));

		HRNGCHK(HashTableHandle,&((*HashTableHandle)[Index/HASHSLICESIZE]),
			sizeof(HashTableEntry));
		PRNGCHK((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr,
			&((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]),
			sizeof(HashSliceEntry));
		HSE = &((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]);
		SaveBlock(HSE); /* save changes, if any */
		if (!SwapBlockIn(HSE))  /* make sure the object is loaded */
			{
				return NIL;
			}
		Temp = HSE->HandleToData;
		HSE->HandleToData = NIL;
		HNoPurge(Temp);
		return Temp;
	}


/* convert an index into a triple pointer */
Trandle				CDiskCache::Index2Trandle(IndexType Index)
	{
		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::Index2Trandle called on uninitialized object."));
		ERROR((((Index/HASHSLICESIZE)*sizeof(HashTableEntry) >=
			HandleSize((Handle)HashTableHandle)) || ((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK].UsedCell == False)),
			PRERR(ForceAbort,"CDiskCache::Index2Trandle called on nonexistent block."));

		HRNGCHK(HashTableHandle,&((*HashTableHandle)[Index/HASHSLICESIZE]),
			sizeof(HashTableEntry));
		PRNGCHK((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr,
			&((*HashTableHandle)[Index/HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]),
			sizeof(HashSliceEntry));
		return &( (*HashTableHandle)[Index/HASHSLICESIZE].
			SlicePtr[Index & HASHSLICEMASK].HandleToData );
	}


/* convert a triple pointer into an index */
IndexType			CDiskCache::Trandle2Index(Trandle BlockRef)
	{
		long			Scan;
		short			Index;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::Trandle2Index called on uninitialized object."));
		for (Scan = HandleSize((Handle)HashTableHandle) / sizeof(HashTableEntry) - 1;
			Scan >= 0; Scan -= 1)
			{
				HRNGCHK(HashTableHandle,&((*HashTableHandle)[Scan]),sizeof(HashTableEntry));
				PRNGCHK((*HashTableHandle)[Scan].SlicePtr,&((*HashTableHandle)[Scan]
					.SlicePtr[HASHSLICESIZE - 1]),sizeof(HashSliceEntry));
				if ((BlockRef >= &((*HashTableHandle)[Scan].SlicePtr[0].HandleToData))
					&& (BlockRef <= &((*HashTableHandle)[Scan]
					.SlicePtr[HASHSLICESIZE - 1].HandleToData)))
					{
						return (((char*)BlockRef - (char*)&((*HashTableHandle)[Scan].
							SlicePtr[0].HandleToData)) / sizeof(HashSliceEntry));
					}
			}
		EXECUTE(PRERR(ForceAbort,"CDiskCache::Trandle2Index called on invalid Trandle."));
	}


/* convert double pointer to triple pointer */
Trandle				CDiskCache::Handle2Trandle(Handle BlockHandle)
	{
		long		Scan;
		short		Index;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::Handle2Trandle called on uninitialized object."));
		for (Scan = HandleSize((Handle)HashTableHandle) / sizeof(HashTableEntry) - 1;
			Scan >= 0; Scan -= 1)
			{
				HRNGCHK(HashTableHandle,&((*HashTableHandle)[Scan]),sizeof(HashTableEntry));
				for (Index = HASHSLICESIZE - 1; Index >= 0; Index -= 1)
					{
						PRNGCHK((*HashTableHandle)[Scan].SlicePtr,&((*HashTableHandle)[Scan]
							.SlicePtr[Index]),sizeof(HashSliceEntry));
						if ((BlockHandle == (*HashTableHandle)[Scan].SlicePtr[Index]
							.HandleToData) && ((*HashTableHandle)[Scan].SlicePtr[Index].UsedCell))
							{
								return &((*HashTableHandle)[Scan].SlicePtr[Index].HandleToData);
							}
					}
			}
		EXECUTE(PRERR(ForceAbort,"CDiskCache::Handle2Trandle called on invalid handle."));
	}


IndexType			CDiskCache::Handle2Index(Handle BlockHandle)
	{
		long			Scan;
		short			Index;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::Handle2Index called on uninitialized object."));
		for (Scan = HandleSize((Handle)HashTableHandle) / sizeof(HashTableEntry);
			Scan >= 0; Scan -= 1)
			{
				HRNGCHK(HashTableHandle,&((*HashTableHandle)[Scan]),sizeof(HashTableEntry));
				for (Index = HASHSLICESIZE - 1; Index >= 0; Index -= 1)
					{
						PRNGCHK((*HashTableHandle)[Scan].SlicePtr,&((*HashTableHandle)[Scan]
							.SlicePtr[Index]),sizeof(HashSliceEntry));
						if ((BlockHandle == (*HashTableHandle)[Scan].SlicePtr[Index]
							.HandleToData) && ((*HashTableHandle)[Scan].SlicePtr[Index].UsedCell))
							{
								return ((Scan * HASHSLICESIZE) + Index);
							}
					}
			}
		EXECUTE(PRERR(ForceAbort,"CDiskCache::Handle2Index called on invalid Handle."));
	}


/* set the special index stored 4 bytes from the beginning of the file. */
/* this is useful for specifying a root node of some sort */
void					CDiskCache::SetSpecialIndex(IndexType Index)
	{
		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::SetSpecialIndex called on uninitialized object."));
		ERROR((((Index/HASHSLICESIZE)*sizeof(HashTableEntry) >=
			HandleSize((Handle)HashTableHandle)) || ((*HashTableHandle)[Index/HASHSLICESIZE]
			.SlicePtr[Index & HASHSLICEMASK].UsedCell == False)),
			PRERR(ForceAbort,"CDiskCache::SetSpecialIndex called on nonexistent block."));
		FSetFilePos(FileReference,SPECIALINDEXOFFSET);
		FWriteBlock(FileReference,(char*)&Index,sizeof(IndexType));
		FFlushFile(FileReference);
	}


/* find out what the special index is */
IndexType			CDiskCache::GetSpecialIndex(void)
	{
		IndexType		Temp;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::GetSpecialIndex called on uninitialized object."));
		FSetFilePos(FileReference,SPECIALINDEXOFFSET);
		FReadBlock(FileReference,(char*)&Temp,sizeof(IndexType));
		return Temp;
	}


/* squeeze unallocated blocks out of file */
void					CDiskCache::CompactFile(void)
	{
		long					SourceScan;
		long					TargetScan;
		FileBlockNode	BlockHeader;
		CSack*				Temp;
		long					BlockCountTemp;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::CompactFile called on uninitialized object."));
		StartTimeConsumingOperation();
		SourceScan = DATASTARTOFFSET;
		TargetScan = SourceScan;
		while (SourceScan < TotalSize)
			{
				CheckCursor();
				FSetFilePos(FileReference,SourceScan);
				FReadBlock(FileReference,(char*)&BlockHeader,sizeof(FileBlockNode));
				if (BlockHeader.Index == UnallocatedIndex)
					{
						/* blank, so skip it */
						SourceScan += BlockHeader.Size;
						NumBlocks -= 1; /* we've just deleted a block */
					}
				 else
					{
						/* used, so move it */
						if (SourceScan != TargetScan)
							{
								/* why bother copying data if it's in the right place */
								FMoveData(FileReference,SourceScan,FileReference,TargetScan,
									BlockHeader.Size);
								HRNGCHK(HashTableHandle,&((*HashTableHandle)[BlockHeader.Index
									/ HASHSLICESIZE]),sizeof(HashTableEntry));
								PRNGCHK((*HashTableHandle)[BlockHeader.Index / HASHSLICESIZE].SlicePtr,
									&((*HashTableHandle)[BlockHeader.Index / HASHSLICESIZE].SlicePtr[
									BlockHeader.Index & HASHSLICEMASK]),sizeof(HashSliceEntry));
								(*HashTableHandle)[BlockHeader.Index / HASHSLICESIZE].SlicePtr[
									BlockHeader.Index & HASHSLICEMASK].FileOffset = TargetScan;
							}
						SourceScan += BlockHeader.Size;
						TargetScan += BlockHeader.Size;
					}
			}
		FreeSpace = 0;
		TotalSize = TargetScan; /* new end of file */
		FSetEOF(FileReference,TotalSize);
		FSetFilePos(FileReference,TOTALSIZEOFFSET);
		FWriteBlock(FileReference,(char*)&TargetScan,sizeof(long));
		FSetFilePos(FileReference,NUMBLOCKSOFFSET);
		BlockCountTemp = NumBlocks;
		FWriteBlock(FileReference,(char*)&BlockCountTemp,sizeof(long));
		delete FreeBlockList;
		Temp = new CSack;
		Temp->ISack(sizeof(FreeBlockEntry),FREEBLOCKNODESIZE);
		FreeBlockList = Temp;
		FileModified = True;
		FlushVitalStats();
		EndTimeConsumingOperation();
	}


/* save all data to disk & remove it from memory */
void					CDiskCache::FlushAll(MyBoolean FlushHeldBlocks)
	{
		register long			Scan;
		register short		Index;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::FlushAll called on uninitialized object."));
		StartTimeConsumingOperation();
		for (Scan = HandleSize((Handle)HashTableHandle) / sizeof(HashTableEntry) - 1;
			Scan >= 0; Scan -= 1)
			{
				for (Index = HASHSLICESIZE - 1; Index >= 0; Index -= 1)
					{
						register HashSliceEntry*	HSE;

						CheckCursor();
						HRNGCHK(HashTableHandle,&((*HashTableHandle)[Scan]),sizeof(HashTableEntry));
						PRNGCHK((*HashTableHandle)[Scan].SlicePtr,&((*HashTableHandle)[Scan]
							.SlicePtr[Index]),sizeof(HashSliceEntry));
						HSE = &((*HashTableHandle)[Scan].SlicePtr[Index]);
						if (FlushHeldBlocks)
							{
								HSE->NoPurge = False; /* force it to flush */
							}
						SwapBlockOut(HSE);
					}
			}
		FlushVitalStats();
		EndTimeConsumingOperation();
	}


/* save all data to disk, but keep it in memory */
void					CDiskCache::SaveAll(void)
	{
		register long			Scan;
		register short		Index;

		ERROR(!DiskCacheInitFlag,
			PRERR(ForceAbort,"CDiskCache::SaveAll called on uninitialized object."));
		StartTimeConsumingOperation();
		for (Scan = HandleSize((Handle)HashTableHandle) / sizeof(HashTableEntry) - 1;
			Scan >= 0; Scan -= 1)
			{
				for (Index = HASHSLICESIZE - 1; Index >= 0; Index -= 1)
					{
						CheckCursor();
						HRNGCHK(HashTableHandle,&((*HashTableHandle)[Scan]),sizeof(HashTableEntry));
						PRNGCHK((*HashTableHandle)[Scan].SlicePtr,&((*HashTableHandle)[Scan]
							.SlicePtr[Index]),sizeof(HashSliceEntry));
						SaveBlock(&((*HashTableHandle)[Scan].SlicePtr[Index]));
					}
			}
		FlushVitalStats();
		EndTimeConsumingOperation();
	}


/* find an unallocated hash entry */
IndexType			CDiskCache::FindFreeHashEntry(void)
	{
		register HashSliceEntry*	Temp;
		MyBoolean									Succeeded;

		if (FreeHashEntry == NIL)
			{
				/* create another one */
				Succeeded = ValidateHashEntry(HASHSLICESIZE / sizeof(HashTableEntry)
					* HandleSize((Handle)HashTableHandle)); /* add new row */
				if (!Succeeded)
					{
						return UnallocatedIndex; /* allocation failed */
					}
			}
		/* return an existing one */
		Temp = FreeHashEntry;
		FreeHashEntry = (HashSliceEntry*)(Temp->HandleToData); /* delink entry */
		return Temp->FileOffset; /* return index */
	}


/* make sure the hash entry exists */
MyBoolean			CDiskCache::ValidateHashEntry(register IndexType Index)
	{
		register long							HashTableScan;
		register long							LastHashEntry;
		register short						SliceScan;
		HashTableEntry**					Temp;
		register HashSliceEntry*	HSE;

		while ((Index/HASHSLICESIZE) >= (HandleSize((Handle)HashTableHandle)
			/ sizeof(HashTableEntry)) )
			{
				Temp = (HashTableEntry**)AllocHandle(HandleSize((Handle)HashTableHandle)
					+ sizeof(HashTableEntry));
				SetTag(Temp,"CDC HashTable");
				LastHashEntry = HandleSize((Handle)HashTableHandle) / sizeof(HashTableEntry);
				for (HashTableScan = LastHashEntry - 1; HashTableScan >= 0; HashTableScan -= 1)
					{
						HRNGCHK(HashTableHandle,&((*HashTableHandle)[HashTableScan]),
							sizeof(HashTableEntry));
						HRNGCHK(Temp,&((*Temp)[HashTableScan]),sizeof(HashTableEntry));
						(*Temp)[HashTableScan] = (*HashTableHandle)[HashTableScan];
					}
				HRNGCHK(Temp,&((*Temp)[LastHashEntry].SlicePtr),sizeof(HashTableEntry));
				(*Temp)[LastHashEntry].SlicePtr =
					(HashSliceEntry*)AllocPtrCanFail(HASHSLICESIZE*sizeof(HashSliceEntry));
				if ((*Temp)[LastHashEntry].SlicePtr == NIL)
					{
						ReleaseHandle((Handle)Temp); /* couldn't allocate new entry, so return */
						return False; /* allocation failed */
					}
				ReleaseHandle((Handle)HashTableHandle);
				HashTableHandle = Temp;
				SetTag((*Temp)[LastHashEntry].SlicePtr,"CDC Slice");
				HSE = &((*Temp)[LastHashEntry].SlicePtr[HASHSLICESIZE]);
				for (SliceScan = HASHSLICESIZE - 1; SliceScan >= 0; SliceScan -= 1)
					{
						/* this is the same operation ReleaseHashEntry performs */
						HSE -= 1; /* decrement to previous HashSliceEntry */
						PRNGCHK((*Temp)[LastHashEntry].SlicePtr,HSE,sizeof(HashSliceEntry));
						HSE->UsedCell = False;
						/* linking this to the free list */
						HSE->HandleToData = (char**)FreeHashEntry;
						FreeHashEntry = HSE; /* now its been pushed into list */
						HSE->FileOffset = (LastHashEntry * HASHSLICESIZE)
							+ SliceScan; /* store index for easy reference */
					}
				MaxIndex = (LastHashEntry + 1) * HASHSLICESIZE;
				FileModified = True;
				FlushVitalStats();
			}
		return True;
	}


/* make a hash entry once again available */
void					CDiskCache::ReleaseHashEntry(register IndexType Index)
	{
		register HashSliceEntry*	HSE;

		ERROR((Index/HASHSLICESIZE) >= (HandleSize((Handle)HashTableHandle)
			/ sizeof(HashTableEntry)),PRERR(ForceAbort,
			"CDiskCache::ReleaseHashEntry called on undefined index."));
		HRNGCHK(HashTableHandle,&((*HashTableHandle)[Index / HASHSLICESIZE]),
			sizeof(HashTableEntry));
		HSE = &((*HashTableHandle)[Index / HASHSLICESIZE].SlicePtr[Index & HASHSLICEMASK]);
		PRNGCHK((*HashTableHandle)[Index / HASHSLICESIZE].SlicePtr,HSE,
			sizeof(HashSliceEntry));
		ERROR((HSE->UsedCell)==False,PRERR(ForceAbort,
			"CDiskCache::ReleaseHashEntry was called on an index that was already released."));
		HSE->UsedCell = False;
		HSE->HandleToData = (char**)FreeHashEntry; /* linking this to free linked list */
		FreeHashEntry = HSE; /* now its pushed into list */
		HSE->FileOffset = Index; /* store index in for easy reference */
	}


/* allocate a block of data in the disk file somewhere */
MyBoolean			CDiskCache::AllocateDiskBlock(long NumBytes, long* IndexToBlockStart,
								IndexType Index)
	{
		FileBlockNode		BlockHeader;
		FreeBlockEntry	FreeBlock;

		/* looking for block somewhere */
		FreeBlockList->ResetScan();
		while (FreeBlockList->GetNext(&FreeBlock))
			{
				if (FreeBlock.Size >= NumBytes + sizeof(FileBlockNode))
					{
						/* found one! */
						FreeBlockList->KillElement(&FreeBlock); /* remove from free list */
						*IndexToBlockStart = FreeBlock.FileOffset;
						if (FreeBlock.Size - (NumBytes + sizeof(FileBlockNode))
							>= sizeof(FileBlockNode) + sizeof(long))
							{
								/* we'll have to split the block, since there's enough room to hold */
								/* another block header AND a long int */
								/* writing first block header */
								NumBlocks += 1; /* by splitting, we increase the number of blocks */
								BlockHeader.Index = Index;
								BlockHeader.Size = NumBytes + sizeof(BlockHeader);
								BlockHeader.Correction = 0;
								FreeSpace -= BlockHeader.Size; /* remove this from freespace pool */
								FSetFilePos(FileReference,FreeBlock.FileOffset);
								ERROR(FLastError()!=noErr,PRERR(AllowResume,
									"FSetFilePos error occurred in CDiskCache::AllocateDiskBlock."));
								FWriteBlock(FileReference,(char*)&BlockHeader,sizeof(FileBlockNode));
								ERROR(FLastError()!=noErr,PRERR(AllowResume,
									"FWriteBlock error occurred in CDiskCache::AllocateDiskBlock."));
								/* making second block header */
								FreeBlock.FileOffset += BlockHeader.Size; /* right after first block */
								FreeBlock.Size -= BlockHeader.Size; /* it's the left over space */
								FreeBlockList->PushElement(&FreeBlock); /* add to the free world */
								BlockHeader.Size = FreeBlock.Size;
								BlockHeader.Index = UnallocatedIndex;
								BlockHeader.Correction = 0;
								FSetFilePos(FileReference,FreeBlock.FileOffset);
								ERROR(FLastError()!=noErr,PRERR(AllowResume,
									"FSetFilePos error occurred in CDiskCache::AllocateDiskBlock."));
								FWriteBlock(FileReference,(char*)&BlockHeader,sizeof(FileBlockNode));
								ERROR(FLastError()!=noErr,PRERR(AllowResume,
									"FWriteBlock error occurred in CDiskCache::AllocateDiskBlock."));
							}
						 else
							{
								/* we'll have to allocate the whole block */
								/* BlockHeader.Size stays the same */
								BlockHeader.Size = FreeBlock.Size;
								BlockHeader.Index = Index;
								BlockHeader.Correction = FreeBlock.Size - sizeof(FileBlockNode)
									- NumBytes;
								FreeSpace -= FreeBlock.Size; /* remove from freespace pool */
								FSetFilePos(FileReference,FreeBlock.FileOffset);
								ERROR(FLastError()!=noErr,PRERR(AllowResume,
									"FSetFilePos error occurred in CDiskCache::AllocateDiskBlock."));
								FWriteBlock(FileReference,(char*)&BlockHeader,sizeof(FileBlockNode));
								ERROR(FLastError()!=noErr,PRERR(AllowResume,
									"FWriteBlock error occurred in CDiskCache::AllocateDiskBlock."));
							}
						return True; /* and we're outa here */
					}
			}
		/* end of file reached, with no suitable blocks:  so append, my friend */
		NumBlocks += 1; /* by appending, we add a block */
		FSetEOF(FileReference,TotalSize + sizeof(FileBlockNode) + NumBytes);
		if (FLastError() != noErr)
			{
				FSetEOF(FileReference,TotalSize); /* undo the change */
				CompactFile(); /* try to squeeze out free space */
				FSetEOF(FileReference,TotalSize + sizeof(FileBlockNode) + NumBytes);
				if (FLastError() != noErr)
					{
						FSetEOF(FileReference,TotalSize); /* undo again, since it still failed */
						EXECUTE(PRERR(AllowResume,
							"CDiskCache::AllocatediskBlock unable to allocate disk space."));
						return False; /* indicate to your superiors that you're incompetent */
					}
			}
		/* writing the block header */
		BlockHeader.Size = sizeof(FileBlockNode) + NumBytes;
		BlockHeader.Correction = 0;
		BlockHeader.Index = Index;
		FSetFilePos(FileReference,TotalSize);
		ERROR(FLastError()!=noErr,PRERR(AllowResume,
			"FSetFilePos error occurred in CDiskCache::AllocateDiskBlock."));
		*IndexToBlockStart = TotalSize; /* record start of block */
		FWriteBlock(FileReference,(char*)&BlockHeader,sizeof(FileBlockNode));
		ERROR(FLastError()!=noErr,PRERR(AllowResume,
			"FWriteBlock error occurred in CDiskCache::AllocateDiskBlock."));
		/* adjusting our and the file's total size count */
		TotalSize += BlockHeader.Size;
		FileModified = True;
		FlushVitalStats();
		return True;
	}


/* unallocate a block of data in the disk file somewhere, try to combine it */
/* with any neighboring blocks */
void				CDiskCache::ReleaseDiskBlock(long IndexToBlockStart)
	{
		FileBlockNode		BlockHeader;
		FreeBlockEntry	FreeBlock;
		short						Error;

		/* getting info on our block */
	 GetInfoPoint:
		FSetFilePos(FileReference,IndexToBlockStart);
		FReadBlock(FileReference,(char*)&BlockHeader,sizeof(FileBlockNode));
		BlockHeader.Index = UnallocatedIndex; /* set it as unallocated */
		/* note: we'll be writing the BlockHeader back at the end */
		/* seeing if we can combine left */
	 CombLeftPoint:
		FreeBlockList->ResetScan();
		while (FreeBlockList->GetNext(&FreeBlock))
			{
				if (FreeBlock.FileOffset + FreeBlock.Size == IndexToBlockStart)
					{
						/* we can combine left! */
						FreeBlockList->KillElement(&FreeBlock); /* get rid of old */
						/* move block start to beginning of this new left block: */
						FreeSpace -= FreeBlock.Size;
						IndexToBlockStart -= FreeBlock.Size;
						BlockHeader.Size += FreeBlock.Size; /* size contains both */
						NumBlocks -= 1; /* we're combining two, so one disappears */
						goto CombRightPoint;
					}
			}
		/* seeing if we can combine right */
	 CombRightPoint:
		FreeBlockList->ResetScan();
		while (FreeBlockList->GetNext(&FreeBlock))
			{
				if (BlockHeader.Size + IndexToBlockStart == FreeBlock.FileOffset)
					{
						/* we can combine right! */
						FreeBlockList->KillElement(&FreeBlock); /* get rid of old */
						FreeSpace -= FreeBlock.Size;
						BlockHeader.Size += FreeBlock.Size; /* increase the size */
						NumBlocks -= 1; /* again, two combine into one */
						goto RewritePoint;
					}
			}
		/* writing data back */
	 RewritePoint:
		/* update our internal free block list */
		FreeBlock.Size = BlockHeader.Size;
		FreeBlock.FileOffset = IndexToBlockStart;
		FreeBlockList->PushElement(&FreeBlock);
		/* udpate disk file */
		Error = FSetFilePos(FileReference,IndexToBlockStart);
		ERROR(Error!=noErr,PRERR(ForceAbort,
			"CDiskCache::ReleaseDiskBlock couldn't set file position."));
		Error = FWriteBlock(FileReference,(char*)&BlockHeader,sizeof(FileBlockNode));
		ERROR(Error!=noErr,PRERR(ForceAbort,
			"CDiskCache::ReleaseDiskBlock couldn't write block header back."));
		/* update internal count of free space */
		FreeSpace += BlockHeader.Size;
		FileModified = True;
		FlushVitalStats();
	}


/* save a block out to disk */
void					CDiskCache::SaveBlock(HashSliceEntry* BlockInfo)
	{
		char		HandleStat;
		short		Error;
		EXECUTE(FileBlockNode BlockHeader;)
		EXECUTE(long HandleSizeTemp;)

		if (BlockInfo->UsedCell)
			{
				if ((BlockInfo->HandleToData != NIL) && (*(BlockInfo->HandleToData) == NIL))
					{
						/* it was purged by the memory manager */
						ReleaseHandle(BlockInfo->HandleToData);
						BlockInfo->HandleToData = NIL;
					}
				if (BlockInfo->Changed && (BlockInfo->HandleToData != NIL))
					{
						EXECUTE(FSetFilePos(FileReference,BlockInfo->FileOffset));
						EXECUTE(FReadBlock(FileReference,(char*)&BlockHeader,sizeof(FileBlockNode)));
						EXECUTE(HandleSizeTemp = HandleSize(BlockInfo->HandleToData));
						ERROR(BlockHeader.Size - BlockHeader.Correction - sizeof(FileBlockNode)
							!= HandleSizeTemp,PRERR(ForceAbort,
							"CDiskCache::SaveBlock block size on disk doesn't match handle size."));
						Error = FSetFilePos(FileReference,BlockInfo->FileOffset
							+ sizeof(FileBlockNode));
						ERROR(Error!=noErr,PRERR(AllowResume,
							"CDiskCache::SaveBlock unable to set file position."));
						if (Error == noErr)
							{
								HandleStat = HGetState(BlockInfo->HandleToData);
								HLock(BlockInfo->HandleToData);
								Error = FWriteBlock(FileReference,*(BlockInfo->HandleToData),
									HandleSize(BlockInfo->HandleToData));
								ERROR(Error!=noErr,PRERR(AllowResume,
									"CDiskCache::SaveBlock unable to write block to disk."));
								HSetState(BlockInfo->HandleToData,HandleStat);
								if (Error == noErr)
									{
										BlockInfo->Changed = False;
									}
								FileModified = True;
							}
					}
			}
	}


/* save a block out to disk and dispose of its loaded image */
long					CDiskCache::SwapBlockOut(HashSliceEntry* BlockInfo)
	{
		long		SizeTemp;

		SizeTemp = 0;
		if (BlockInfo->UsedCell)
			{
				SaveBlock(BlockInfo);
				if ((BlockInfo->HandleToData != NIL) && (BlockInfo->NoPurge == False)
					&& (BlockInfo->Changed == False))
					{
						SizeTemp = HandleSize(BlockInfo->HandleToData);
						ReleaseHandle(BlockInfo->HandleToData);
						BlockInfo->HandleToData = NIL;
					}
			}
		return SizeTemp;
	}


MyBoolean			CDiskCache::SwapBlockIn(HashSliceEntry* BlockInfo)
	{
		FileBlockNode		BlockHeader;

		ERROR(!BlockInfo->UsedCell,PRERR(ForceAbort,
			"CDiskCache::SwapBlockIn called on unallocated block."));
		if (BlockInfo->HandleToData != NIL)
			{
				if (*(BlockInfo->HandleToData) != NIL)
					{
						if (BlockInfo->NoPurge || BlockInfo->Changed)
							{
								HNoPurge(BlockInfo->HandleToData);
							}
						 else
							{
								HPurge(BlockInfo->HandleToData);
							}
						return True;
					}
				 else
					{
						ReleaseHandle(BlockInfo->HandleToData);
					}
			}
		FSetFilePos(FileReference,BlockInfo->FileOffset);
		FReadBlock(FileReference,(char*)&BlockHeader,sizeof(FileBlockNode));
		BlockInfo->HandleToData = AllocHandleCanFail(BlockHeader.Size
			- BlockHeader.Correction - sizeof(FileBlockNode));
		if (BlockInfo->HandleToData != NIL)
			{
				SetDiskTag(BlockInfo->HandleToData,Trandle2Index(&(BlockInfo->HandleToData)));
				HLock(BlockInfo->HandleToData);
				FReadBlock(FileReference,*(BlockInfo->HandleToData),
					HandleSize(BlockInfo->HandleToData));
				HUnlock(BlockInfo->HandleToData);
				BlockInfo->Changed = False;
				if (!BlockInfo->NoPurge)
					{
						HPurge(BlockInfo->HandleToData);
					}
				return True;
			}
		return False;
	}


/* rewrite TotalSize, NumBlocks, and MaxIndex back to file */
void					CDiskCache::FlushVitalStats(void)
	{
		OSErr		Error;
		long		Temp;

		Error = FSetFilePos(FileReference,TOTALSIZEOFFSET);
		Temp = TotalSize;
		Error = FWriteBlock(FileReference,(char*)&Temp,sizeof(long));

		Error = FSetFilePos(FileReference,NUMBLOCKSOFFSET);
		Temp = NumBlocks;
		Error = FWriteBlock(FileReference,(char*)&Temp,sizeof(long));

		Error = FSetFilePos(FileReference,MAXINDEXOFFSET);
		Temp = MaxIndex;
		Error = FWriteBlock(FileReference,(char*)&Temp,sizeof(long));
	}


void				CDiskCache::DoIdle(long TimeSinceLastEvent)
	{
		if (FileModified && ((ulong)(TickCount() - TimeSinceLastEvent) > FILEFLUSHTHRESHHOLD))
			{
				FFlushFile(FileReference);
				FileModified = False;
			}
	}


/* start at StartingIndex and purge blocks to the end until */
/* SizeNeeded bytes are released */
long					CDiskCache::PurgeSome(long SizeNeeded, IndexType* StartingIndex)
	{
		long		Accr;

		Accr = 0;
		while (((*StartingIndex) < MaxIndex) && (SizeNeeded / 4 > Accr))
			{
				HRNGCHK(HashTableHandle,&((*HashTableHandle)[(*StartingIndex)/HASHSLICESIZE]),
					sizeof(HashTableEntry));
				PRNGCHK((*HashTableHandle)[(*StartingIndex)/HASHSLICESIZE].SlicePtr,
					&((*HashTableHandle)[(*StartingIndex)/HASHSLICESIZE].
					SlicePtr[(*StartingIndex) & HASHSLICEMASK]),sizeof(HashSliceEntry));
				Accr += SwapBlockOut(&((*HashTableHandle)[(*StartingIndex)/HASHSLICESIZE].
					SlicePtr[(*StartingIndex) & HASHSLICEMASK]));
				(*StartingIndex) += 1;
			}
		return Accr;
	}


/* verify file integrity and load index nodes.  True = successful, False = bad file */
/* any existing index nodes are assumed to be invalid, so call only at start */
MyBoolean			CDiskCache::InitAll(void)
	{
		long		FileSize;

		if (FGetEOF(FileReference,&FileSize) != noErr)
			{
				return False;
			}
		if (FileSize == 0)
			{
				long		Temp;

				TotalSize = DATASTARTOFFSET;
				NumBlocks = 0;
				Temp = DATASTARTOFFSET;
				FSetFilePos(FileReference,TOTALSIZEOFFSET);
				FWriteBlock(FileReference,(char*)&Temp,sizeof(long));
				Temp = UnallocatedIndex;
				FSetFilePos(FileReference,SPECIALINDEXOFFSET);
				FWriteBlock(FileReference,(char*)&Temp,sizeof(long));
				Temp = 0;
				FSetFilePos(FileReference,NUMBLOCKSOFFSET);
				FWriteBlock(FileReference,(char*)&Temp,sizeof(long));
				return True;
			}
		 else
			{
				FileBlockNode		Scan;
				long						FilePos;
				long						TotalSizeTemp;
				long						NumBlocksTemp;
				long						MaxIndexTemp;
				long						HashTableScan;
				long						SliceScan;

				FSetFilePos(FileReference,TOTALSIZEOFFSET);
				FReadBlock(FileReference,(char*)&TotalSizeTemp,sizeof(long)); /* get file size */
				if (FileSize < TotalSizeTemp)
					{
						return False; /* if not equal, then file is corrupt */
						/* if >, then maybe the EOF was lost.  We'll keep checking */
					}
				FSetFilePos(FileReference,NUMBLOCKSOFFSET);
				FReadBlock(FileReference,(char*)&NumBlocksTemp,sizeof(long));
				FSetFilePos(FileReference,MAXINDEXOFFSET);
				FReadBlock(FileReference,(char*)&MaxIndexTemp,sizeof(long));
				NumBlocks = 0;
				TotalSize = TotalSizeTemp;
				MaxIndex = MaxIndexTemp;
				FilePos = DATASTARTOFFSET;
				FreeSpace = 0;
				/* creating all hash table entries and setting them as unused */
				ReleaseHandle((Handle)HashTableHandle);
				HashTableHandle = (HashTableEntry**)
					AllocHandle(sizeof(HashTableEntry**) * (MaxIndexTemp / HASHSLICESIZE));
				for (HashTableScan = (MaxIndexTemp / HASHSLICESIZE) - 1; HashTableScan >= 0;
					HashTableScan -= 1)
					{
						(*HashTableHandle)[HashTableScan].SlicePtr =
							(HashSliceEntry*)AllocPtr(sizeof(HashSliceEntry) * HASHSLICESIZE);
						for (SliceScan = HASHSLICESIZE - 1; SliceScan >= 0; SliceScan -= 1)
							{
								(*HashTableHandle)[HashTableScan].SlicePtr[SliceScan].UsedCell = False;
							}
					}
				/* accounting for all blocks in the file */
				while (FilePos < TotalSizeTemp)
					{
						/* getting block from disk */
						FSetFilePos(FileReference,FilePos);
						FReadBlock(FileReference,(char*)&Scan,sizeof(FileBlockNode));
						/* creating index entry */
						NumBlocks += 1; /* count this block in the total */
						if (Scan.Index != UnallocatedIndex)
							{
								register HashSliceEntry*	HSE;

								if (Scan.Index >= MaxIndexTemp)
									{
										return False; /* index exceeded, file is corrupt */
									}
								HSE = &((*HashTableHandle)[Scan.Index / HASHSLICESIZE]
									.SlicePtr[Scan.Index & HASHSLICEMASK]);
								HSE->HandleToData = NIL;
								HSE->FileOffset = FilePos;
								HSE->UsedCell = True;
								HSE->Changed = False;
								HSE->NoPurge = False;
							}
						 else
							{
								FreeBlockEntry	Free;

								Free.Size = Scan.Size;
								Free.FileOffset = FilePos;
								FreeBlockList->PushElement(&Free);
								FreeSpace += Scan.Size;
							}
						/* stepping to next block */
						FilePos += Scan.Size;
					}
				/* adding unused blocks to the free block list */
				for (HashTableScan = (MaxIndexTemp / HASHSLICESIZE) - 1; HashTableScan >= 0;
					HashTableScan -= 1)
					{
						for (SliceScan = HASHSLICESIZE - 1; SliceScan >= 0; SliceScan -= 1)
							{
								HashSliceEntry*	HSE;

								/* this is similar to the procedure used by ReleaseHashEntry */
								HSE = &((*HashTableHandle)[HashTableScan].SlicePtr[SliceScan]);
								if (!(HSE->UsedCell))
									{
										HSE->HandleToData =
											(char**)FreeHashEntry; /* linking this to free linked list */
										FreeHashEntry = HSE; /* now its pushed into list */
										HSE->FileOffset = (HashTableScan * HASHSLICESIZE)
											+ SliceScan; /* store index in for quick reference */
									}
							}
					}
				if (NumBlocks != NumBlocksTemp)	
					{
						return False; /* this is really bad */
					}
				return True; /* assume its ok */
			}
	}


/* the master grow zone function calls this to dispose of blocks from CDiskCaches */
static long		CDiskCache::MyGrowZone(ulong SizeNeeded)
	{
		long				Accr;
		CDiskCache*	StartingCache;

		if (LastCachePurged == NIL)
			{
				return 0; /* failed, since there are no caches */
			}
		Accr = 0;
		StartingCache = LastCachePurged;
		do
			{
				Accr += LastCachePurged->PurgeSome(SizeNeeded - Accr,&LastIndexPurged);
				if (Accr < SizeNeeded)
					{
						LastIndexPurged = 0;
						LastCachePurged = LastCachePurged->NextCache;
					}
			} while ((LastCachePurged != StartingCache) && (Accr < SizeNeeded));
		return Accr;
	}


/* when a system error occurs, call this to flush all existing caches and close them, */
/* so that the user might recover data later. */
void		SystemErrorFlushAllDiskCaches(void)
	{
		CDiskCache*	StartingCache;
		CDiskCache*	Scan;

		if (LastCachePurged != NIL)
			{
				Scan = LastCachePurged;
				StartingCache = LastCachePurged;
				do
					{
						Scan->FlushAll(DoFlushHeldBlocks);
						Scan = Scan->NextCache;
					} while (Scan != StartingCache);
			}
	}
