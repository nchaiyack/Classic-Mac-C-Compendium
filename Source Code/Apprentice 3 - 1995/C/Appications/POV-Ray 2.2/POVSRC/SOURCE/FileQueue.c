/*
==============================================================================
Project:	POV-Ray

Version:	2.2

File Name:	FileQueue.c

Description:
	Generic Macintosh File Spec queueing routine.  This is intended to
	be used to collect the list of files passed in via System 7 ODOC
	AppleEvents, for processing at a more convenient time.

	This is the main implementation of the functions.

Related Files:
	FileQueue.h	- External interface for these routines
------------------------------------------------------------------------------
Author:
	Eduard [esp] Schwan
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
	920820	[esp]	Created
	921221	[esp]	Added logic in Get() routine to call destructor after getting last item
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
==============================================================================
*/

#include "FileQueue.h"

#include <memory.h>	// NewHandle


// Class definition of internal File Queue object
typedef struct
{
	flisthdl_t	fFlist;
	flisthdl_t	fFlistEnd;
	flisthdl_t	fFlistGetter;
	short		fFlistNumItems;
} ClassFileQueueRec_t, *ClassFileQueuePtr_t, **ClassFileQueueHdl_t;


// a static global instance for our internal use
ClassFileQueueRec_t	gFileQueue;


// ------------------------------------------------------------------
// Initialize the list handling structures prior to use
void FileQ_c(void)
{
	gFileQueue.fFlist = NULL;
	gFileQueue.fFlistEnd = NULL;
	gFileQueue.fFlistGetter = NULL;
	gFileQueue.fFlistNumItems = 0;
} // FileQ_c



// ------------------------------------------------------------------
// Add a file to the list
void FileQ_Put(FSSpecPtr	pFSSpecPtr)
{
	flisthdl_t	newFlistHdl;

	// create new record
	newFlistHdl = (flisthdl_t)NewHandle(sizeof(flistrec_t));

	if (newFlistHdl)
	{
		// fill it up
		(**newFlistHdl).fFSSpec = *pFSSpecPtr;
		(**newFlistHdl).fNext = NULL;
	
#if defined (DO_DEBUG)
printf("PutFileInList - '%P'\n",(**newFlistHdl).fFSSpec.name);
#endif // DO_DEBUG

		// add it to the list
		gFileQueue.fFlistNumItems++;
		if (gFileQueue.fFlist == NULL)
		{ // first on list
			gFileQueue.fFlist = newFlistHdl;
			gFileQueue.fFlistEnd = newFlistHdl;
			// track head of list for Get routine
			gFileQueue.fFlistGetter = gFileQueue.fFlist;
		}
		else
		{ // next on list
			// connect new record onto end of list
			(**gFileQueue.fFlistEnd).fNext = (Handle)newFlistHdl;
			// point list's end to this new record
			// (essentially, we appended the new record at end of list)
			gFileQueue.fFlistEnd = newFlistHdl;
		}
	}
} // FileQ_Put



// ------------------------------------------------------------------
// Retrieve the next file from the list (increments after each call)
Boolean FileQ_Get(FSSpecPtr	pFSSpecPtr)
{
	Boolean		GotIt = false;

	if (gFileQueue.fFlistGetter != NULL)
	{
		gFileQueue.fFlistNumItems--;
		// get it
		*pFSSpecPtr = (**gFileQueue.fFlistGetter).fFSSpec;
		GotIt = true;
		// move to next record
		gFileQueue.fFlistGetter = (flisthdl_t)(**gFileQueue.fFlistGetter).fNext;
		// if we fell off last item in list, destroy the list
		if (gFileQueue.fFlistGetter == NULL)
			FileQ_d();
#if defined (DO_DEBUG)
printf("GetFileFromList - '%P'\n",pFSSpecPtr->name);
#endif // DO_DEBUG
	}

	return GotIt;

} // FileQ_Get


// ------------------------------------------------------------------
// Return the # of items in the queue
short FileQ_NumItems(void)
{
	return gFileQueue.fFlistNumItems;
} // FileQ_NumItems


// ------------------------------------------------------------------
// Delete the list and its contents after its use
void FileQ_d()
{
	flisthdl_t	anFlistHdl, nextFlistHdl;

	// start at head of list
	anFlistHdl = gFileQueue.fFlist;

	// dispose of every record on the list
	while (anFlistHdl != NULL)
	{
#if defined (DO_DEBUG)
printf("FileQ_d - '%P'\n",(**anFlistHdl).fFSSpec.name);
#endif // DO_DEBUG
		// remember next in line
		nextFlistHdl = (flisthdl_t)(**anFlistHdl).fNext;
		// delete current
		DisposeHandle((Handle)anFlistHdl);
		// move to next in line
		anFlistHdl = nextFlistHdl;
	}

	// the coup de gras
	FileQ_c();

} // FileQ_d


