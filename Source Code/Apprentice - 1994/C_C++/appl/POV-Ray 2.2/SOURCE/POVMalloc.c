/*==============================================================================
Project:	POV-Ray

Version:	2.2

File:		POVMalloc.c

Description:
Library routines and definitions to re-route the Std C library
calls to malloc/free, track them, and handle garbage-collection.

There are some compile-time flags in here of importance.  They
are not defined (off), but if defined, will change the behavior
of the memory allocation code:

USE_NATIVE_MALLOC - if defined, will use the Mac's NewPtr() calls
instead of C's malloc().  This turns out to be a little slower,
but leaves a cleaner heap.  malloc allocates chunks that are
not actually discarded when free is called.

USE_MEMHANDLES - This allocates handles instead of pointers, then
locks and derefs them.

USE_MEMTAGS - if defined, will add a 4 byte tag to each memory
chunk allocated, and insure that the de-allocation of memory has
this tag.  This will detect any free calls that are passed bad
pointers.

MALLOC_TRACE - Adds info about the source file/line # of allocation
in the block allocated, so it can be displayed on cleanup
------------------------------------------------------------------------------
Author:
	Jim Nitchals and Eduard [esp] Schwan
------------------------------------------------------------------------------
	from Persistence of Vision Raytracer
	Copyright 1993 Persistence of Vision Team
------------------------------------------------------------------------------
	NOTICE: This source code file is provided so that users may experiment
	with enhancements to POV-Ray and to port the software to platforms other 
	than those supported by the POV-Ray Team.  There are strict rules under
	which you are permitted to use this file.  The rules are in the file
	named POVLEGAL.DOC which should be distributed with this file. If 
	POVLEGAL.DOC is not available or for more info please contact the POV-Ray
	Team Coordinator by leaving a message in CompuServe's Graphics Developer's
	Forum.  The latest version of POV-Ray may be found there as well.

	This program is based on the popular DKB raytracer version 2.12.
	DKBTrace was originally written by David K. Buck.
	DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
------------------------------------------------------------------------------
More Info:
	This Macintosh version of POV-Ray was created and compiled by Jim Nitchals
	(Think 5.0) and Eduard Schwan (MPW 3.2), based (loosely) on the original
	port by Thomas Okken and David Lichtman, with some help from Glenn Sugden.

	For bug reports regarding the Macintosh version, you should contact:
	Eduard [esp] Schwan
		CompuServe: 71513,2161
		Internet: jl.tech@applelink.apple.com
		AppleLink: jl.tech
	Jim Nitchals
		Compuserve: 73117,3020
		America Online: JIMN8
		Internet: jimn8@aol.com -or- jimn8@applelink.apple.com
		AppleLink: JIMN8
------------------------------------------------------------------------------
Change History:
	921023	[esp]	Created.
	921202	[esp]	Added POV_need_to_reclaim() routine
	921221	[esp]	Added BeVerbose parm to POV_Reclaim routine
	930423	[esp]	Added low-memory check in malloc/calloc (MIN_SAFE_BYTES).
	930618	[esp]	Added USE_NATIVE_MALLOC code (again) and renamed USE_MEMTAGS
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
	931020	[esp]	Added realloc function
	931020	[esp]	Began adding experimental NewHandle/MoveHHi (USE_MEMHANDLES) code
	931116	[esp]	Added out-of-mem handling code (gReserveBuffer, HandleOutOfMem)
	931119	[djh]	Added <ToolUtils.h> for PPC compatibility
==============================================================================*/

#define POVMALLOC_C

#include <types.h>
#include <stdlib.h>	// malloc
#include <memory.h>	// NewPtr
#include <errors.h>	// memFullErr
#include <ToolUtils.h> // watchCursor

#include "POVMalloc.h"
#include "POVLib.h"


// out-of-mem reserve space
#define RESERVE_MEM_SIZE	60000L

// This defines the lowest free memory is allowed to get before
// malloc & calloc fail.  This guarantees some free heap space for
// error recovery.
#define	MIN_SAFE_BYTES		60000L

#if USE_MEMTAGS
// long int tag used to detect bad ptrs passed to free()
#define MALLOC_TAG		'MTAG'
#endif // USE_MEMTAGS


// Memory allocation tracking (Garbage Collection)
static Handle		gReserveBuffer = NULL;
static Boolean		gDoMemTracking;
static Boolean		gReclaiming;
static long			gMallocListCount,
					gFreeListCount,
					gMallocListSize;
static Ptr			*gMallocList;		// array of pointers
static Ptr			*gFreeList;			// array of pointers
static long			gEscapedMallocsCount = 0L;	// track # of untracked mallocs (can't garbage collect)


void	catch_exit(int n);


// =====================================================================
void AllocateSafetyBuffer(void)
{
	if (gReserveBuffer == NULL)
	{
		gReserveBuffer = NewHandle(RESERVE_MEM_SIZE);
		if (gReserveBuffer)
			MoveHHi(gReserveBuffer);
	}
} // AllocateSafetyBuffer


// =====================================================================
void PurgeSafetyBuffer(void)
{
	if (gReserveBuffer)
	{
		DisposeHandle(gReserveBuffer);
		gReserveBuffer = NULL;
	}
} // PurgeSafetyBuffer


// =====================================================================
OSErr POV_init_memtracking(size_t max_trackable)
{
	gReclaiming = false; // not reclaiming just now

	// allocate malloc() garbage collection buffers
	gMallocListSize = max_trackable;
	if (gMallocListSize < 500)
		gMallocListSize = 500;
	gMallocList  = (Ptr*) NewPtr((gMallocListSize+2L)*sizeof(Ptr));
	gFreeList = (Ptr*) NewPtr((gMallocListSize+2L)*sizeof(Ptr));

	if ((gMallocList == NULL) || (gFreeList == NULL))
	{
		// fatal error
//		displayDialog(kdlog_GenericFatalErr, "Cannot allocate memory for garbage collection",
//					gMallocListSize, ewcDoCentering, eMainDevice);
//		exit_handler();
		return memFullErr;
	}
	else
	{
		// unmark first item on list
		*gMallocList = NULL;
		gMallocListCount = 0;
		*gFreeList = NULL;
		gFreeListCount = 0;
	}

	// allocate our safety buffer
	AllocateSafetyBuffer();

	return noErr;

} // POV_init_memtracking


// =====================================================================
void POV_enable_memtracking(Boolean dotracking)
{
	gDoMemTracking = dotracking;
} // POV_enable_memtracking


// =====================================================================
void HandleOutOfMem(void)
{
	// We are most likely out of memory.  For now, we will free all the
	// allocated memory, and exit back to the main loop with an error.
	// We could be down to 5 or 10 bytes of available memory, and to do
	// anything else (like return from malloc and let the caller handle it)
	// risks a hard lockup or crash!  So, first, make sure there's room
	// for loading dialogs & stuff in POV_reclaim(), by dumping the buffer memory.
	PurgeSafetyBuffer();

	// open some more heap space
	(void)CompactMem(FreeMem());

	// beep!
	SysBeep(2);

	// Now free all memory
	POV_reclaim(false, NULL);

	// tell user we failed from here
	puts("\n## Error! Ran out of memory.  Action aborted.");

	// reallocate our safety net for next time
	AllocateSafetyBuffer();

	// now exit out of all fn calls
	catch_exit(1);

} // HandleOutOfMem


// =====================================================================
#if MALLOC_TRACE
void *POV_malloc(size_t size, char * src_file, short src_line_num)
#else
void *POV_malloc(size_t size)
#endif // MALLOC_TRACE
{
	void	*myptr = NULL;
#if USE_MEMHANDLES
	Handle	myhdl = NULL;
#endif

	if (MaxBlock() > MIN_SAFE_BYTES)
	{
#if MALLOC_TRACE
		size += 32; // add room for tag file name/line #
#endif // MALLOC_TRACE

#if USE_MEMTAGS
		size += 4; // add room for tag MALLOC_TAG
#endif // USE_MEMTAGS

#if USE_NATIVE_MALLOC
 #if USE_MEMHANDLES
		// use locked handles
		myhdl = NewHandle(size);
		if (myhdl)
		{
			MoveHHi(myhdl); // move it up
			HLock(myhdl); // and lock it down
			// it's really a handle, turn it into a pointer
			myptr = *myhdl;
		}
 #else
		// use regular pointers
		myptr = NewPtr(size);
 #endif // USE_MEMHANDLES
#else
		// use Std C Lib allocator
		myptr = malloc(size);
#endif // USE_NATIVE_MALLOC

		if (myptr)
		{ // successfully allocated it
 
#if MALLOC_TRACE
			*(short*)myptr = src_line_num;
			myptr = (void*)(((long)myptr)+2L); // point beyond line #
			strcpy(myptr, src_file);
			myptr = (void*)(((long)myptr)+30L); // point beyond file name
#endif // MALLOC_TRACE

#if USE_MEMTAGS
			*(long*)myptr = MALLOC_TAG;
			myptr = (void*)(((long)myptr)+4L); // point beyond tag
#endif // USE_MEMTAGS

			if (gDoMemTracking)
			{  // remember this pointer for auto-disposing later
#if USE_MEMHANDLES
				// remember the handle
				gMallocList[gMallocListCount] = (Ptr)myhdl;
#else
				gMallocList[gMallocListCount] = myptr;
#endif
				if (gMallocListCount < gMallocListSize)
					gMallocListCount++;
				else
					gEscapedMallocsCount++; // oops, overflow, one got away!
			}
		}
	}

	// allocation was unsuccessful
	if (myptr == NULL)
		HandleOutOfMem();

	return myptr;

} // POV_malloc


// =====================================================================
#if MALLOC_TRACE
void *POV_calloc(size_t nmemb, size_t size, char * src_file, short src_line_num)
#else
void *POV_calloc(size_t nmemb, size_t size)
#endif // MALLOC_TRACE
{
	void *myptr = NULL;
#if USE_MEMHANDLES
	Handle	myhdl = NULL;
#endif

	if (MaxBlock() > MIN_SAFE_BYTES)
	{
#if MALLOC_TRACE
		size += 32; // add room for tag file name/line #
#endif // MALLOC_TRACE

#if USE_MEMTAGS
		size += 4; // add room for tag MALLOC_TAG
#endif // USE_MEMTAGS

#if USE_NATIVE_MALLOC
 #if USE_MEMHANDLES
		// use locked handles
		myptr = NewHandleClear(nmemb*size);
		if (myhdl)
		{
			MoveHHi(myhdl); // move it up
			HLock(myhdl); // and lock it down
			// it's really a handle, turn it into a pointer
			myptr = *myhdl;
		}
 #else
		// use regular pointers
		myptr = NewPtrClear(nmemb*size);
 #endif // USE_MEMHANDLES
#else
		myptr = calloc(nmemb,size);
#endif // USE_NATIVE_MALLOC

		if (myptr)
		{ // successfully allocated it 
#if MALLOC_TRACE
			*(short*)myptr = src_line_num;
			myptr = (void*)(((long)myptr)+2L); // point beyond line #
			strcpy(myptr, src_file);
			myptr = (void*)(((long)myptr)+30L); // point beyond file name
#endif // MALLOC_TRACE

#if USE_MEMTAGS
			*(long*)myptr = MALLOC_TAG;
			myptr = (void*)(((long)myptr)+4L); // point beyond tag
#endif // USE_MEMTAGS

			if (gDoMemTracking)
			{  // remember this pointer for auto-disposing later
#if USE_MEMHANDLES
				// remember the handle
				gMallocList[gMallocListCount] = (Ptr)myhdl;
#else
				gMallocList[gMallocListCount] = myptr;
#endif
				if (gMallocListCount < gMallocListSize)
					gMallocListCount++;
				else
					gEscapedMallocsCount++; // oops, overflow, one got away!
			}
		}
	}

	// allocation was unsuccessful
	if (myptr == NULL)
		HandleOutOfMem();

	return myptr;

} // POV_calloc



// =====================================================================
void *POV_realloc(void * p, size_t newsize)
{
printf("ERROR! realloc not ready!\n");
/*
---------------------------------------- not ready yet!
#if USE_NATIVE_MALLOC
	size_t	oldsize;
#endif
#if USE_MEMHANDLES
	Handle	myhdl = NULL;
#endif

	short	k;
	void	*myptr = NULL;

	// Warn user if nil pointer passed in!
	if (p == NULL)
	{
		printf("## Warning! Ignoring an attempt to realloc a nil pointer!\n");
#if USE_MEMTAGS
		DebugStr("\pTried to free a nil pointer");
#endif // USE_MEMTAGS
		return NULL;
	}

	if (MaxBlock() > MIN_SAFE_BYTES)
	{
#if USE_MEMTAGS
		newsize += 4; // add room for tag MALLOC_TAG
		pOrig = (void*)(((long)p)-4L); // back up to tag/original position
		if (*(long*)pOrig != MALLOC_TAG)
		{
			printf("## Error!  Tried to realloc a bad/stomped block! ($%lx)\n",pOrig);
			DebugStr("\pTried to realloc a bad/stomped block 1");
		}
#endif // USE_MEMTAGS

#if USE_NATIVE_MALLOC

 #if USE_MEMHANDLES
		// get previous size for shrink/grow check
		oldsize = GetHandleSize(RecoverHandle(p));
		// allocate new space... use locked handles
		myhdl = (Ptr)NewHandle(newsize);
 #else
		// get previous size for shrink/grow check
		oldsize = GetPtrSize(p);
		// allocate new space... use regular pointers
		myptr = NewPtr(newsize);
 #endif // USE_MEMHANDLES

		if (myptr)
		{
 #if USE_MEMHANDLES
			MoveHHi(myptr); // move it up
			HLock(myptr); // and lock it down
			// it's really a handle, turn it into a pointer
			myptr = *(Handle)myptr;
 #endif // USE_MEMHANDLES
 
#if USE_MEMTAGS
			oldsize -= 4; // remove size of tag MALLOC_TAG, just copy data
			newsize -= 4; // remove size of tag MALLOC_TAG, just copy data
#endif
			// copy old contents into new space (trim if less space)
			BlockMove(p, myptr, (oldsize<newsize)?oldsize:newsize);

			// now release old memory...
			POV_free(p);
		}
#else
		// do the ANSI thing
		myptr = realloc(p, newsize);
#endif // USE_NATIVE_MALLOC

		if (myptr)
		{ // successfully re-allocated it
#if USE_MEMTAGS
			*(long*)myptr = MALLOC_TAG;
			myptr = (void*)(((long)myptr)+4L); // point beyond tag
#endif // USE_MEMTAGS

			if (gDoMemTracking)
			{  // remember this new pointer for auto-disposing later
				gMallocList[gMallocListCount] = myptr;
				if (gMallocListCount < gMallocListSize)
					gMallocListCount++;
				else
					gEscapedMallocsCount++; // oops, overflow, one got away!
			}
		}
	}

	// allocation was unsuccessful
	if (myptr == NULL)
		HandleOutOfMem();

	return myptr;
------------------------------------
*/
} // POV_realloc


// =====================================================================
void POV_free(void *myptr)
{
#if USE_MEMHANDLES
 	Handle		myhdl = NULL;
#endif // USE_MEMHANDLES
 
	if (myptr == NULL)
	{
		printf("## Warning! Ignoring an attempt to free a nil pointer!\n");
#if USE_MEMTAGS
		DebugStr("\pTried to free a nil pointer");
#endif // USE_MEMTAGS
	}
	else
	{

#if USE_MEMTAGS
		myptr = (void*)(((long)myptr)-4L); // back up to tag
		if (*(long*)myptr != MALLOC_TAG)
		{
			printf("## Error!  Tried to free a bad/stomped block! ($%lx)\n",myptr);
			DebugStr("\pTried to free a bad/stomped block 1");
		}
#endif // USE_MEMTAGS

#if MALLOC_TRACE
		myptr = (void*)(((long)myptr)-32L); // back up over file name/line #
#endif // MALLOC_TRACE

#if USE_NATIVE_MALLOC
 #if USE_MEMHANDLES
		// use Mac locked handles
		myhdl = RecoverHandle(myptr);
		if (myhdl)
			DisposeHandle(myhdl);
		else
		{
			printf("## Error %d!  Cannot recover handle from ptr! ($%lx)\n",
					MemError(), myptr);
  #if USE_MEMTAGS
			DebugStr("\pCannot recover handle from ptr");
  #endif // USE_MEMTAGS
		}
 #else
		// use regular Mac pointers
		DisposePtr(myptr);
 #endif // USE_MEMHANDLES
#else
		// The Std C library way...
		free(myptr);
#endif // USE_NATIVE_MALLOC

		// remember that this guy was freed for later
		// (if we're tracking, and not in the middle of reclaiming)
		if (gDoMemTracking && !gReclaiming)
		{
#if MALLOC_TRACE
			// point beyond fname again, so that gMallocList & gFreeList match!
			myptr = (void*)(((long)myptr)+32L);
#endif // MALLOC_TRACE

#if USE_MEMTAGS
			// point beyond tag again, so that gMallocList & gFreeList match!
			myptr = (void*)(((long)myptr)+4L);
#endif // USE_MEMTAGS
#if USE_MEMHANDLES
			gFreeList[gFreeListCount] = (Ptr)myhdl;
#else
			gFreeList[gFreeListCount] = myptr;
#endif // USE_MEMHANDLES
			if (gFreeListCount < gMallocListSize)
				gFreeListCount++;
		}
	}
} // POV_free


// =====================================================================
Boolean POV_need_to_reclaim(void)
{
	return (gMallocListCount > 0);
} // POV_need_to_reclaim


// =====================================================================
void POV_reclaim(Boolean BeVerbose, DialogPtr progressDialogPtr)
{
	register Ptr	*theMallocListPtr;
	register Ptr	*theFreedListPtr;
	register long 	i,j;
	long 			min_index, max_index;
#if MALLOC_TRACE
	FILE			*mtfp;
#endif // MALLOC_TRACE

	// Let everyone know we're busy in reclaiming mode
	gReclaiming = true;

	// anything to do?
	if (gMallocListCount > 0)
	{		
		SetCursor(*GetCursor(watchCursor)); // could take a little while..

		// First, remove any entries from the malloc pile if already freed.
		// Do this by scanning the freed pile, and if an entry matches one
		// in malloc pile, delete it.  To help speed things up, keep track of
		// the lowest and highest valid entries in the malloc list, so we don't
		// revisit them in the inner loop (min_index, max_index)
		if (gFreeListCount > 0)
		{
			theFreedListPtr = gFreeList;

			min_index = 0;
			max_index = gMallocListCount-1;
			// find starting point
			theMallocListPtr = &gMallocList[min_index];
			while ((*theMallocListPtr == NULL) && (min_index <= max_index))
			{
				theMallocListPtr++;
				min_index++;
			}
			// find ending point
			theMallocListPtr = &gMallocList[max_index];
			while ((*theMallocListPtr == NULL) && (max_index > min_index))
			{
				theMallocListPtr--;
				max_index--;
			}

// printf("## DEBUG; min=%ld  max=%ld total=%ld\n",min_index,max_index,gMallocListCount); // debug

			for (i=0; i<gFreeListCount; i++, theFreedListPtr++)
			{
				// Update the progress bar only occasionally (every 32 loops)
				if ( progressDialogPtr && !(i & 31) )
				{
					// go to the 1/2 way mark in this loop
					updateProgressDialog(progressDialogPtr, 0, gFreeListCount, i>>1);
				}

				// check this freed pointer against the malloc list
				theMallocListPtr = &gMallocList[min_index];
				for (j = min_index; j<=max_index; j++, theMallocListPtr++)
				{
					// is there a malloc entry to check against?
					if (*theMallocListPtr != NULL)
					{
						if (*theMallocListPtr == *theFreedListPtr)
						{ // found it, remove from malloc pile
							*theMallocListPtr = NULL;
							break; // out of this for loop
						}
					}
				} // for j
			} // for i
		}

#if MALLOC_TRACE
	mtfp = fopen("MallocTrace.Out", "w+");
	fprintf(mtfp, "## Leftover un-freed memory:\n");
#endif // MALLOC_TRACE

		// free whatever is left over in the malloc list
		theMallocListPtr = &gMallocList[min_index];
		for (i=min_index; i<max_index; i++, theMallocListPtr++)
		{
			// Update the progress bar only occasionally (every 32 loops)
			if ( progressDialogPtr && !(i & 31) )
			{
				// do the last 1/2 in this loop
				updateProgressDialog(progressDialogPtr, 0, gMallocListCount,
					(gMallocListCount+i) >> 1);
			}
			if (*theMallocListPtr != NULL)
			{
#if MALLOC_TRACE
				char	* p;
				p = (char*)((long)*theMallocListPtr)-32L;
				fprintf(mtfp, "File %s; line %d\n", p+2, *(short*)p);
#endif // MALLOC_TRACE
#if USE_MEMHANDLES
				// A true handle is stored there, don't mess around, dispose it!
				DisposeHandle((Handle)*theMallocListPtr);
#else
				POV_free(*theMallocListPtr);
#endif // USE_MEMHANDLES
			}
		}

#if MALLOC_TRACE
	if (mtfp) fclose(mtfp);
#endif // MALLOC_TRACE

		if (BeVerbose)
		{
			printf("-- Garbage Collection Statistics:\n");
			printf("--                     malloc()    free()\n");
			printf("--    Max trackable:   %8ld  %8ld\n", gMallocListSize, gMallocListSize);
			printf("--    Total tracked:   %8ld  %8ld\n", gMallocListCount, gFreeListCount);
			if (gEscapedMallocsCount > 0)
			{
				printf("## Extra untracked: %8ld       ---\n", gEscapedMallocsCount);
				printf("##\n");
				printf("## Warning, couldn't track and dispose all memory allocations.\n");
				printf("## Increase the heap size or increase the application's CNFG\n");
				printf("## resource value to help alleviate this.\n");
			}
		}

		// open some heap space
		(void)CompactMem(FreeMem());
	
		*gMallocList = NULL;
		gMallocListCount = 0;
		*gFreeList = NULL;
		gFreeListCount = 0;
		gEscapedMallocsCount = 0L;

		SetCursor(&qd.arrow);
	}

	// Let everyone (POV_Free) know we're out of reclaiming mode
	gReclaiming = false;

} // POV_reclaim
