/* CDiskCache.h */

#pragma once

#include "EventLoop.h"
#include "Memory.h"
#include "File.h"

class CSack;

/* options for IDiskCache */
#define CreatePermanent (-4)
#define CreateTemporary (-3)
#define OpenExisting (-2)
#define CopyToTemporaryAndOpen (-1)

/* errors returned from IDiskCache */
#define DiskError (-1)
#define FileCorrupted (-2)

/* options for FlushAll */
#define DoFlushHeldBlocks (True)
#define DontFlushHeldBlocks (False)

typedef ulong IndexType;  /* typename for indices */

typedef Handle* Trandle;

/* The file's structure is: */
/* ulong: Total size of file */
/* ulong: "Special" Index */
/* ulong: Number of blocks in file (allocated and unallocated) */
/* (as many as needed:) */
/*  ulong: Size of block */
/*  ulong: Index of block (when read into memory), 0xffffffff = not allocated */
/*  [size of block] bytes containing data for the object */
#define UnallocatedIndex (0xffffffff)
typedef struct
	{
		ulong		Size; /* total number of bytes in block (including header) */
		ulong		Index;  /* index of block (or 0xffffffff if not allocated) */
		ushort	Correction;  /* number of wasted bytes */
	} FileBlockNode;

#define TOTALSIZEOFFSET (0)
#define SPECIALINDEXOFFSET (TOTALSIZEOFFSET + sizeof(long))
#define NUMBLOCKSOFFSET (SPECIALINDEXOFFSET + sizeof(long))
#define MAXINDEXOFFSET (NUMBLOCKSOFFSET + sizeof(long))
#define DATASTARTOFFSET (MAXINDEXOFFSET + sizeof(long))

/* this is one record from the array making up one hash slice */
typedef struct
	{
		Handle		HandleToData; /* NIL == not loaded */
			/* if UsedCell == False, then HandleToData is a pointer to next free hash entry */
		long			FileOffset; /* offset into file of block's header */
			/* if UsedCell == False, then FileOffset is the index of this hash entry */
		MyBoolean	UsedCell : 1; /* false = available for allocation, true = used */
		MyBoolean	Changed : 1; /* true = write data back to disk */
		MyBoolean	NoPurge : 1; /* true = can't be unloaded */
		short			Unused; /* rounds it out to 12 bytes, to cause long alignment */
	} HashSliceEntry;

/* this is the number of cells per hash table slice (must be integral power of 2) */
#define HASHSLICESIZE 128
#define HASHSLICEMASK (HASHSLICESIZE-1)


/* this is one slice from the hash table */
typedef struct
	{
		HashSliceEntry*	SlicePtr; /* actually:  HashSliceEntry SlicePtr[HASHSLICESIZE] */
	} HashTableEntry;


typedef struct
	{
		long		FileOffset;
		long		Size;
	} FreeBlockEntry;


struct	CDiskCache	:	CIdle
	{
	 protected:
		static IndexType	LastIndexPurged;
		static MyBoolean	GrowZoneInstalled;
	 public:
		CDiskCache*				NextCache;
	 protected:
		CDiskCache*				PreviousCache;
		HashTableEntry**	HashTableHandle;
		long							TotalSize;
		long							FreeSpace;
		long							NumBlocks;
		short							FileReference;
		FSSpec						DiskFile;
		MyBoolean					DeleteWhenFinished;
		long							MaxIndex;
		MyBoolean					FileModified;
		HashSliceEntry*		FreeHashEntry;
		CSack*						FreeBlockList;
		EXECUTE(MyBoolean DiskCacheInitFlag;)

	 public:
		/* */					CDiskCache();
		/* */					~CDiskCache();
		short					IDiskCache(FSSpec* FileInfo, short Mode, OSType FileType);
		MyBoolean			SaveAs(FSSpec* WhereToSave, ulong Creator, ulong FileType);

		MyBoolean			DiskNew(long BlockSize, IndexType* IndexOut);
		void					DiskDispose(IndexType Index);
		MyBoolean			DiskLoad(IndexType Index);
		MyBoolean			DiskHold(IndexType Index);
		void					DiskUnhold(IndexType Index);
		void					DiskUnload(IndexType Index);
		MyBoolean			DiskResize(long NewBlockSize, IndexType Index);
		void					DiskChanged(IndexType Index);
		void					DiskSave(IndexType Index);

		MyBoolean			AttachHandle(Handle ObjToAttach, IndexType* IndexOut);
		Handle				Detach(IndexType Index);

		Trandle				Index2Trandle(IndexType Index);
		IndexType			Trandle2Index(Trandle BlockRef);
		Trandle				Handle2Trandle(Handle BlockHandle);
		IndexType			Handle2Index(Handle BlockHandle);

		void					SetSpecialIndex(IndexType Index);
		IndexType			GetSpecialIndex(void);

		void					CompactFile(void);
		void					FlushAll(MyBoolean FlushHeldBlocks);
		void					SaveAll(void);

	 private:
		IndexType			FindFreeHashEntry(void);
		MyBoolean			ValidateHashEntry(IndexType Index);
		void					ReleaseHashEntry(IndexType Index);
		MyBoolean			AllocateDiskBlock(long NumBytes, long* IndexToBlockStart,
										IndexType Index);
		void					ReleaseDiskBlock(long IndexToBlockStart);
		void					SaveBlock(HashSliceEntry* BlockInfo);
		long					SwapBlockOut(HashSliceEntry* BlockInfo);
		MyBoolean			SwapBlockIn(HashSliceEntry* BlockInfo);
		void					FlushVitalStats(void);
		void					DoIdle(long TimeSinceLastEvent);
		long					PurgeSome(long SizeNeeded, IndexType* StartingIndex);
		MyBoolean			InitAll(void);
		static long		MyGrowZone(ulong SizeNeeded);
	};

void		SystemErrorFlushAllDiskCaches(void);
