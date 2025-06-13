/* MyMalloc.h */

#ifndef Included_MyMalloc_h
#define Included_MyMalloc_h

/* This module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */

/* initialize the memory allocator. */
MyBoolean		InitializeMyMalloc(void);

/* allocate a new block of memory.  If there is no free block large enough, the */
/* heap is extended.  If the block size is greater than MINMORECORESIZE, then */
/* a system block the size of the block will be allocated, otherwise a system */
/* block of MINMORECORESIZE bytes will be allocated. */
void*				BlockNew(long RequestedBlockSize);

/* this routine releases blocks back to the free list and unifies them with */
/* adjacent blocks if possible.  It keeps the free list in ascending */
/* sorted order to make scanning for adjacent blocks more efficient. */
void				BlockRelease(void* Block);

/* decode and return the size of an allocated block */
long				BlockSize(void* Block);

/* resize the block.  Minimum of the new size and the old size bytes of data will */
/* be preserved.  NOTE:  The block's address may change!!! */
/* this is a very naive implementation which simply allocates a new block, copies */
/* over all the data, and releases the old block.  This really needs to be improved. */
void*				BlockResize(void* Block, long NewRequestedBlockSize);

/* scan list and look for inconsistencies, such as overlapping blocks, blocks */
/* outside of the range of the heap, misaligned blocks, and other weirdness */
/* this routine only does something when debugging is enabled */
#if DEBUG
	void				CheckHeap(void);
#else
	#define CheckHeap()
#endif

/* this routine dumps a file to the working directory containing a list of the */
/* free blocks in the heap so that fragmentation performance can be analyzed */
/* this routine only does something when debugging is enabled */
#if DEBUG
	void				CheckFragmentation(void);
#else
	#define CheckFragmentation()
#endif

#endif
