/* MyMalloc.c */
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

#pragma options(pack_enums)
#include <Memory.h>
#include <Errors.h>
#include <Files.h>
#pragma options(!pack_enums)

#include "MyMalloc.h"


/* limitations: */
/*  - total address space hasn't been tested for more than 24 bits. */
/*  - blocks can't be more than 2^BOUNDARY - sizeof(BlockRec) blocks large */
/*  - only works with flat addressing */


/* alignment MUST be power of 2.  this cleverly aligns to long integers or */
/* pointers, whichever is larger */
#define ALIGNMENT ((sizeof(long) > sizeof(void*)) ? sizeof(long) : sizeof(void*))

/* provide 4 bits of correction (so that requested block sizes can be up to */
/* 15 bytes smaller than the actual heap block allocated.  Hopefully this will work */
/* even if unsigned longs are 64 bits. */
#define BOUNDARY ((8 * sizeof(unsigned long)) - 4)

/* this macro calculates the largest possible size a user could request and */
/* not break the allocator by finding out what the largest block size is and */
/* subtracting the header to find out the content size.  Actually, this will be */
/* a bit smaller, since not of all the header is actually present when the */
/* block is allocated (there is some overlap between header and data area) */
#define MEMBLOCKLIMIT ((1UL << BOUNDARY) - sizeof(BlockRec))

/* this defines the smallest amount of memory that will be requested from the */
/* system if there is not enough already in the heap to satisfy a request. */
/* (for debugging purposes, we make it ridiculously small.)  Make sure that */
/* this is an aligned value!  see notes about exo-core below in the code. */
#if DEBUG
	#define MINMORECORESIZE (16L)
	#define MINEXOCORESIZE (16L)
#else
	#define MINMORECORESIZE (4096L)
	#define MINEXOCORESIZE (262144L)
#endif

/* amount of space reserved below our first handle for system objects */
#define LOWMEMRESERVEDSIZE (49152L)

/* encode the total size of a block and the number of bytes less the requested */
/* size actually was into a single unsigned long word, so that we don't have */
/* to waste any more bytes than we need.  This is done by remembering the total */
/* allocated size and the number of bytes more than the requested size that the */
/* total size was. */
#define SIZEENCODE(size,correction) (((correction) << BOUNDARY) | (size))

/* find out what size the user requested by subtracting the amount of extra */
/* bytes from the total size of the block */
#define DECODEREQUESTEDSIZE(composite) (((composite) & ((1UL << BOUNDARY) - 1))\
					- (((composite) >> BOUNDARY) & 0x0f))

/* find out what the total size allocated was */
#define DECODETOTALSIZE(composite) ((composite) & ((1UL << BOUNDARY) - 1))


/* this structure defines the fields for a block header */
typedef struct BlockRec
	{
		/* BlockSize contains the size of a block, which has differing meanings depending */
		/* on whether the block is free or used.  If the block is free, then it is simply */
		/* the total number of bytes in the block.  If the block is used, then it is */
		/* an encoded size, with the block's total size & the correction for determining */
		/* how much was requested when the block was allocated. */
		unsigned long				BlockSize;
		/* pointer to Next block in the free list is only used when block is free.  If */
		/* block is allocated, then the data starts at the first byte of Next. */
		/* That last bit is important because it allows us to adjust the number and */
		/* size of header fields before the Next for debugging purposes. */
		struct BlockRec*		Next;
	} BlockRec;


/* list of blocks that are free, linked through the Next field */
static BlockRec*			FreeList = NIL;

/* last macintosh handle allocated.  when we need morecore, we try to extend this */
/* before allocating another handle so that we can keep all storage contiguous. */
static char**					LastHandleAllocated = NIL;

/* debugging variable for detecting memory leaks and multiple releases. */
EXECUTE(static long		AllocationCount = 0;)


/* initialize the memory allocator. */
MyBoolean		InitializeMyMalloc(void)
	{
		char**		Temp;

		/* since we stick a handle in the heap and lock it, but try to resize it, */
		/* we want to reserve some space below it for static objects. */
		ReserveMem(LOWMEMRESERVEDSIZE);
		Temp = NewHandle(LOWMEMRESERVEDSIZE);
		if (Temp == NIL)
			{
				return False;
			}
		HLock(Temp);
		/* now allocate an initial handle for our heap thing */
		LastHandleAllocated = NewHandle(0);
		if (LastHandleAllocated == NIL)
			{
				return False;
			}
		HLock(LastHandleAllocated);
		/* finally, dispose of the low memory space to free it up for the system */
		DisposeHandle(Temp);
		return True;
	}


/* allocate a new block of memory.  If there is no free block large enough, the */
/* heap is extended.  If the block size is greater than MINMORECORESIZE, then */
/* a system block the size of the block will be allocated, otherwise a system */
/* block of MINMORECORESIZE bytes will be allocated. */
void*				BlockNew(long RequestedBlockSize)
	{
		BlockRec*						Scan;
		BlockRec*						Lag;
		unsigned long				Extra;
		unsigned long				TotalSize;
		void*								ReturnValue;
		long								OSRequestedSize;
		char**							NewMemHandle;

		APRINT(("+BlockNew %l",RequestedBlockSize));
		ERROR((RequestedBlockSize < 0) || (RequestedBlockSize > MEMBLOCKLIMIT),
			PRERR(ForceAbort,"BlockNew:  allocation block size is out of range"));
	 TryAgainPoint:
		Scan = FreeList;
		Lag = NIL;
		/* (BlockSize + Extra) % ALIGNMENT must == 0 */
		Extra = (ALIGNMENT - 1) - ((RequestedBlockSize + 3) & (ALIGNMENT - 1));
		/* figure out amount of extra space to add to request to get the actual */
		/* block size we need to allocate.  The funny bit on the end figures out */
		/* how many bytes are in the USED block's header.  (the free block's header */
		/* has sizeof(BlockRec) bytes in it, but we can't use this since we overwrite */
		/* the Next field, thus making the used header smaller.) */
		TotalSize = RequestedBlockSize + Extra
			+ (long)((char*)&(((BlockRec*)NIL)->Next) - (char*)NIL);
		if (TotalSize < sizeof(BlockRec))
			{
				/* for tiny blocks, we must be sure we can always free the block */
				TotalSize = sizeof(BlockRec);
			}
		/* scan through list of free blocks to find one that's big enough */
		while (Scan != NIL)
			{
				if (Scan->BlockSize >= TotalSize)
					{
						/* found a big enough block */
						if (Scan->BlockSize < TotalSize + sizeof(BlockRec))
							{
								/* block is too small to split -- take the whole thing */
								/* eliminate this block from the free list */
								if (Lag != NIL)
									{
										Lag->Next = Scan->Next;
									}
								 else
									{
										FreeList = Scan->Next;
									}
								/* adjust block size of Scan */
								/* since we are allocating the whole block, we use [preserve] the */
								/* block's original size (see below) */
								ERROR(DECODETOTALSIZE((unsigned long)SIZEENCODE(Scan->BlockSize,
									Scan->BlockSize - RequestedBlockSize))
									- DECODEREQUESTEDSIZE((unsigned long)SIZEENCODE(Scan->BlockSize,
									Scan->BlockSize - RequestedBlockSize))
									!= Scan->BlockSize - RequestedBlockSize,PRERR(ForceAbort,
									"BlockNew:  not enough bits specified for size correction"));
								Scan->BlockSize = SIZEENCODE(Scan->BlockSize,
									Scan->BlockSize - RequestedBlockSize);
								/* set up return value */
								ReturnValue = (void*)&(Scan->Next);
							}
						 else
							{
								/* block is big enough to be split */
								/* first, make a new block header where the second part of the */
								/* block starts, and set up the fields in the header */
								if (Lag != NIL)
									{
										Lag->Next = (BlockRec*)((char*)Scan + TotalSize);
										/* pointer update first! (else for 0 length blocks, it will */
										/* get overwritten) */
										Lag->Next->Next = Scan->Next;
										Lag->Next->BlockSize = Scan->BlockSize - TotalSize;
									}
								 else
									{
										FreeList = (BlockRec*)((char*)Scan + TotalSize);
										/* pointer update first! (else for 0 length blocks, it will */
										/* get overwritten) */
										FreeList->Next = Scan->Next;
										FreeList->BlockSize = Scan->BlockSize - TotalSize;
									}
								/* construct the header for the current block */
								/* since we are splitting the block, we use the plopped off size */
								/* so that both block's sizes add up to the original (see above) */
								ERROR(DECODETOTALSIZE((unsigned long)SIZEENCODE(TotalSize,TotalSize
									- RequestedBlockSize))
									- DECODEREQUESTEDSIZE((unsigned long)SIZEENCODE(TotalSize,TotalSize
									- RequestedBlockSize))
									!= TotalSize - RequestedBlockSize,PRERR(ForceAbort,
									"BlockNew:  not enough bits specified for size correction"));
								Scan->BlockSize = SIZEENCODE(TotalSize,TotalSize - RequestedBlockSize);
								/* set up the return value */
								ReturnValue = (void*)&(Scan->Next);
							}
						EXECUTE(CheckHeap());
						EXECUTE(AllocationCount += 1;)
						APRINT(("-BlockNew %r",ReturnValue));
						return ReturnValue;
					}
				/* this block wasn't big enough -- try the next one */
				Lag = Scan;
				Scan = Scan->Next;
			}

		/* no memory on free list -- try to get more memory from system */
		APRINT((" MoreCore"));
		if (TotalSize > MINMORECORESIZE)
			{
				/* no alignment worries since TotalSize is made to be aligned above */
				OSRequestedSize = TotalSize;
			}
		 else
			{
				OSRequestedSize = MINMORECORESIZE;
			}

		/* first, try to grow the last handle we allocated */
		if (LastHandleAllocated != NIL)
			{
				long				OldHandleSize;

				OldHandleSize = GetHandleSize(LastHandleAllocated);
				SetHandleSize(LastHandleAllocated,OldHandleSize + OSRequestedSize);
				if (MemError() == noErr)
					{
						/* set up the header for this new block, as if it were used */
						ERROR(DECODETOTALSIZE((unsigned long)SIZEENCODE(OSRequestedSize,
							sizeof(BlockRec))) - DECODEREQUESTEDSIZE((unsigned long)SIZEENCODE(
							OSRequestedSize,sizeof(BlockRec))) != sizeof(BlockRec),PRERR(ForceAbort,
							"BlockNew:  not enough bits specified for size correction"));
						(*(BlockRec*)(*(char**)LastHandleAllocated + OldHandleSize)).BlockSize
							= SIZEENCODE(OSRequestedSize,sizeof(BlockRec));
						/* release it with the normal block release routine */
						EXECUTE(AllocationCount += 1;) /* offset the effect of the following release */
						BlockRelease((void*)
							&((*(BlockRec*)(*(char**)LastHandleAllocated + OldHandleSize)).Next));
						/* try to allocate the block now */
						goto TryAgainPoint;
					}
			}

		ERROR(MINEXOCORESIZE < MINMORECORESIZE,PRERR(ForceAbort,
			"BlockNew:  MINEXOCORESIZE < MINMORECORESIZE"));
		if (OSRequestedSize <= LOWMEMRESERVEDSIZE)
			{
				/* keep from clobbering the reserved space that we so carefully set aside */
				OSRequestedSize = LOWMEMRESERVEDSIZE + (4 * ALIGNMENT);
			}
		ReserveMem(OSRequestedSize);
		NewMemHandle = NewHandle(OSRequestedSize);
#if !DEBUG  /* when debugging, we stay in local heap */
		if (NewMemHandle == NIL)
			{
				OSErr				Error;

				if (OSRequestedSize < MINEXOCORESIZE)
					{
						/* if we allocate outside of our heap, we will grab larger */
						/* blocks of memory since we have less control over what may */
						/* get allocated on top of us.  hopefully this will reduce fragmentation */
						/* of blocks outside of the heap */
						OSRequestedSize = MINEXOCORESIZE;
					}
				NewMemHandle = TempNewHandle(OSRequestedSize,&Error);
			}
#endif
		if (NewMemHandle != NIL)
			{
				ERROR((((unsigned long)(*NewMemHandle) & (ALIGNMENT - 1)) != 0),
					PRERR(ForceAbort,"Hey! The memory manager's block is not aligned!"));
				/* make sure the mac doesn't move our memory */
				HLock(NewMemHandle);
				/* set up the header for this new block, as if it were used */
				ERROR(DECODETOTALSIZE((unsigned long)SIZEENCODE(OSRequestedSize,
					sizeof(BlockRec))) - DECODEREQUESTEDSIZE((unsigned long)SIZEENCODE(
					OSRequestedSize,sizeof(BlockRec))) != sizeof(BlockRec),PRERR(ForceAbort,
					"BlockNew:  not enough bits specified for size correction"));
				(**(BlockRec**)NewMemHandle).BlockSize
					= SIZEENCODE(OSRequestedSize,sizeof(BlockRec));
				/* release it with the normal block release routine */
				EXECUTE(AllocationCount += 1;) /* offset the effect of the following release */
				BlockRelease((void*)&((**(BlockRec**)NewMemHandle).Next));
				/* remember the block's location so we can try to grow it later */
				LastHandleAllocated = NewMemHandle;
				/* try to allocate the block now */
				goto TryAgainPoint;
			}
		EXECUTE(CheckHeap());
		APRINT(("-BlockNew NIL"));
		return NIL;
	}


/* this routine releases blocks back to the free list and unifies them with */
/* adjacent blocks if possible.  It keeps the free list in ascending */
/* sorted order to make scanning for adjacent blocks more efficient. */
void				BlockRelease(void* Block)
	{
		BlockRec*		Scan;
		BlockRec*		Lag;
		BlockRec*		OurNewBlock;

		/* this first thing is an ugly but portable way of treating *Block as the */
		/* 'Next' field of a BlockRec and finding out where the BlockRec begins */
		OurNewBlock = (BlockRec*)((char*)Block
			- ((char*)&(((BlockRec*)NIL)->Next) - (char*)NIL));
		APRINT(("+BlockRelease %r (size %l)",Block,DECODETOTALSIZE(OurNewBlock->BlockSize)));
		/* adjust block's header so that it is free */
		OurNewBlock->BlockSize = DECODETOTALSIZE(OurNewBlock->BlockSize);
		/* scan the free list looking for items that can be appended / prepended to block */
		Lag = NIL;
		Scan = FreeList;
		while ((Scan != NIL) && (Scan < OurNewBlock))
			{
				/* searching so that Lag is before OurNewBlock and Scan is after it */
				Lag = Scan;
				Scan = Scan->Next;
			}
		/* now, scan is either NIL or the block right after this block. */
		/* now trying to put block into the list */
		OurNewBlock->Next = Scan;
		if (Lag != NIL)
			{
				Lag->Next = OurNewBlock;
			}
		 else
			{
				FreeList = OurNewBlock;
			}
		/* first, try to coalesce block with the one before it */
		if (Lag != NIL)
			{
				if ((char*)Lag + Lag->BlockSize == (char*)OurNewBlock)
					{
						/* if we can coalesce, dump block & add to Lag's size */
						Lag->BlockSize += OurNewBlock->BlockSize;
						Lag->Next = OurNewBlock->Next;
						OurNewBlock = Lag;
					}
			}
		/* now, try to coalesce block with the one after it */
		if (Scan != NIL)
			{
				if ((char*)OurNewBlock + OurNewBlock->BlockSize == (char*)Scan)
					{
						OurNewBlock->BlockSize += Scan->BlockSize;
						OurNewBlock->Next = Scan->Next;
					}
			}
		EXECUTE(CheckHeap());
		EXECUTE(AllocationCount -= 1;)
		APRINT(("-BlockRelease"));
	}


/* decode and return the size of an allocated block */
long				BlockSize(void* Block)
	{
		BlockRec*		TrueBlockBase;

		TrueBlockBase = (BlockRec*)((char*)Block
			- ((char*)&(((BlockRec*)NIL)->Next) - (char*)NIL));
		APRINT(("*BlockSize %r %l",Block,DECODEREQUESTEDSIZE(TrueBlockBase->BlockSize)));
		return DECODEREQUESTEDSIZE(TrueBlockBase->BlockSize);
	}


/* resize the block.  Minimum of the new size and the old size bytes of data will */
/* be preserved.  NOTE:  The block's address may change!!! */
/* this is a very naive implementation which simply allocates a new block, copies */
/* over all the data, and releases the old block.  For performance reasons, this */
/* might be improved to intelligently look for a free block adjacent to the */
/* allocated block.  However, by always relocating the block, it is very good at */
/* pointing out bugs that assume blocks don't move when resized. */
void*				BlockResize(void* Block, long NewRequestedBlockSize)
	{
		void*				NewPointer;
		long				ValidSectionSize;
		long				OldBlockSize;

		APRINT(("+BlockResize %r",Block));
		ERROR((NewRequestedBlockSize < 0) || (NewRequestedBlockSize > MEMBLOCKLIMIT),
			PRERR(ForceAbort,"BlockSize:  allocation block size is out of range"));
		NewPointer = BlockNew(NewRequestedBlockSize);
		if (NewPointer == NIL)
			{
				return NIL;
			}
		OldBlockSize = BlockSize(Block);
		if (NewRequestedBlockSize < OldBlockSize)
			{
				ValidSectionSize = NewRequestedBlockSize;
			}
		 else
			{
				ValidSectionSize = OldBlockSize;
			}
		CopyData((char*)Block,(char*)NewPointer,ValidSectionSize);
		BlockRelease(Block);
		APRINT(("-BlockResize %r",NewPointer));
		return NewPointer;
	}


/* scan list and look for inconsistencies, such as overlapping blocks, blocks */
/* outside of the range of the heap, misaligned blocks, and other weirdness */
/* this routine only does something when debugging is enabled */
#if DEBUG
void				CheckHeap(void)
	{
		BlockRec*		Scan;
		BlockRec*		Lag;
		Zone*				Zone;
		char*				ZoneBeginning;
		char*				ZoneEnd;

		APRINT(("+CheckHeap"));
		if (AllocationCount < 0)
			{
				PRERR(ForceAbort,"CheckHeap:  Number of allocated blocks is negative");
			}
		Lag = NIL;
		Scan = FreeList;
		Zone = GetZone();
		ZoneBeginning = (char*)Zone;
		ZoneEnd = (char*)(Zone->bkLim);
		while (Scan != NIL)
			{
				if (((unsigned long)Scan & (ALIGNMENT - 1)) != 0)
					{
						PRERR(ForceAbort,"CheckHeap:  Bad link pointer encountered");
					}
				if ((Scan->BlockSize & (ALIGNMENT - 1)) != 0)
					{
						PRERR(ForceAbort,"CheckHeap:  Misaligned free block size value");
					}
				if (((char*)Scan < ZoneBeginning) || ((char*)Scan >= ZoneEnd))
					{
						PRERR(ForceAbort,"CheckHeap:  Reference is outside heap area!");
					}
				if (Scan->BlockSize < sizeof(BlockRec))
					{
						PRERR(ForceAbort,"CheckHeap:  Free block's size is too small");
					}
				if (Scan <= Lag)
					{
						PRERR(ForceAbort,"CheckHeap:  Block occurs lower than previous block");
					}
				if ((Scan->Next != NIL) && ((char*)Scan + Scan->BlockSize >= (char*)(Scan->Next)))
					{
						PRERR(ForceAbort,"CheckHeap:  Block + Size overshoots next block");
					}
				Scan = Scan->Next;
			}
		APRINT(("-CheckHeap"));
	}
#endif


/* this routine dumps a file to the working directory containing a list of the */
/* free blocks in the heap so that fragmentation performance can be analyzed */
/* this routine only does something when debugging is enabled */
#if DEBUG
void				CheckFragmentation(void)
	{
		BlockRec*			Scan;
		char					DumpName[] = "\p!!HeapFragmentationDump";
		char					Hex[16] = "0123456789abcdef";
		short					MemDumpFile;
		char					NumAllocatedBlocks[38] = "Number of allocated blocks = xxxxxxxx\x0d";
		long					Index;

		FSDelete((unsigned char*)DumpName,0);
		ERROR(Create((unsigned char*)DumpName,0,AUDITCREATOR,'TEXT') != noErr,
			PRERR(ForceAbort,"Couldn't create fragmentation dump file"));
		ERROR(FSOpen((unsigned char*)DumpName,0,&MemDumpFile) != noErr,PRERR(ForceAbort,
			"Couldn't open fragmentation dump file for writing"));
		for (Index = 0; Index < 8; Index += 1)
			{
				NumAllocatedBlocks[29 + Index]
					= Hex[(AllocationCount >> ((7 - Index) * 4)) & 0x0f];
			}
		Index = 38;
		FSWrite(MemDumpFile,&Index,NumAllocatedBlocks);
		Scan = FreeList;
		while (Scan != NIL)
			{
				char						Buffer[30] = "xxxxxxxx..xxxxxxxx (xxxxxxxx)\x0d";
				long						ByteCount = 30;
				unsigned long		Low;
				unsigned long		High;

				Low = (unsigned long)Scan;
				High = Low + Scan->BlockSize - 1;
				for (Index = 0; Index < 8; Index += 1)
					{
						Buffer[Index] = Hex[(Low >> ((7 - Index) * 4)) & 0x0f];
					}
				for (Index = 0; Index < 8; Index += 1)
					{
						Buffer[Index + 10] = Hex[(High >> ((7 - Index) * 4)) & 0x0f];
					}
				for (Index = 0; Index < 8; Index += 1)
					{
						Buffer[Index + 20] = Hex[(Scan->BlockSize >> ((7 - Index) * 4)) & 0x0f];
					}
				FSWrite(MemDumpFile,&ByteCount,Buffer);
				Scan = Scan->Next;
			}
		FSClose(MemDumpFile);
	}
#endif
