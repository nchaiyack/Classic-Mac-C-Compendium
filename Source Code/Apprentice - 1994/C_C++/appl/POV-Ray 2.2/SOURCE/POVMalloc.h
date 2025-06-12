/*==============================================================================
Project:	POV-Ray

Version:	2.2

File:		POVMalloc.h

Description:
Library routines and definitions to re-route the Std C library
calls to malloc/free, track them, and handle garbage-collection.

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
	921023	[esp]	Created.
	921202	[esp]	Added POV_need_to_reclaim() routine
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
	931020	[esp]	Added realloc function
	931020	[esp]	Added debug file/line info to malloc/calloc (MALLOC_TRACE)
==============================================================================*/

#if !defined(POVMALLOC_H)
#define POVMALLOC_H


#include <dialogs.h>	// DialogPtr

// rename std. C library calls for all users (except POVMalloc.c itself!)
#if !defined (POVMALLOC_C)
#if MALLOC_TRACE
#define malloc(p)				POV_malloc(p, __FILE__, __LINE__)
#define calloc(size, nitems)	POV_calloc(size, nitems, __FILE__, __LINE__)
#else
#define malloc(p)				POV_malloc(p)
#define calloc(size, nitems)	POV_calloc(size, nitems)
#endif // MALLOC_TRACE
#define realloc(p, newsize)		POV_realloc(p, newsize)
#define free(p)					POV_free(p)
#endif // POVMALLOC_C

extern	OSErr POV_init_memtracking(size_t max_trackable);
extern	void POV_enable_memtracking(Boolean dotracking);
#if MALLOC_TRACE
extern	void *POV_malloc(size_t size, char * src_file, short src_line_num);
#else
extern	void *POV_malloc(size_t size);
#endif // MALLOC_TRACE
#if MALLOC_TRACE
extern	void *POV_calloc(size_t nmemb, size_t size, char * src_file, short src_line_num);
#else
extern	void *POV_calloc(size_t nmemb, size_t size);
#endif // MALLOC_TRACE
extern	void *POV_realloc(void * p, size_t newsize);
extern	void POV_free(void *p);
extern	Boolean POV_need_to_reclaim(void);
extern	void POV_reclaim(Boolean BeVerbose, DialogPtr progressDialogPtr);


#endif // POVMALLOC_H
