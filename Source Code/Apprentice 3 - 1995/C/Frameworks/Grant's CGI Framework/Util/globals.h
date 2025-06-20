#pragma once
/*****
 *
 *	globals.h
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright �1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

/***  GLOBAL DECLARATIONS  ***/

#ifdef __MainSegment__
	#define	_GLOBAL_	/* local */
#else
	#define	_GLOBAL_	extern
#endif

/* MEMORY */

/* handle to emergency memory reserve. Used in MemoryFunc.c. IM-Memory 1-46 */
_GLOBAL_	Handle		gEmergencyMemory;


/** PROCESS **/

_GLOBAL_	Boolean		gQuit;			/* application is set to quit */
_GLOBAL_	Boolean		gFrontProcess;	/* application is in the foreground */
_GLOBAL_	long		gSleepTicks;	/* sleep time for WaitNextEvent | threads */


/**  APPLE EVENTS  **/

_GLOBAL_	AEIdleUPP	gAEIdleUPP;


/* SCREEN RECTS */

_GLOBAL_	Rect		gScreenRect;	/* main monitor */
_GLOBAL_	Rect		gGrayRgnRect;	/* entire screen */


/* STRINGS */

_GLOBAL_	Str255		gVersionStr;


/* ERRORS */

_GLOBAL_	StringHandle	gSystemErrorStr;


/* TOOLBOX MANAGERS AND FUNCTION CALLS AVAILABLE */

_GLOBAL_	Boolean		gHasThreadMgr;		/* Thread Manager present */
_GLOBAL_	Boolean		gHasColorQD;		/* Color QuickDraw present */



#ifdef _GLOBAL_
	#undef _GLOBAL_
#endif

/***** EOF *****/
