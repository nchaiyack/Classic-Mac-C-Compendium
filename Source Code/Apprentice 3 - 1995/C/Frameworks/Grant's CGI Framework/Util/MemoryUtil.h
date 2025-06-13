#pragma once
/*****
 *
 *	MemoryUtil.h
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright ©1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

/***  CONSTANT DECLARATIONS  ***/

/* estimate minimum needs here */

/* 'comfort' zone of free memory, 40k. This must be big enough to allow any essential functions
	to be called. Named kMemCushion in IM-Memory: 1-43, 1-36 */
#define kMemCushionSize			40960
/* 'comfort' zone of free memory, when printing, 70k. */
#define kPrintMemCushionSize	71680
/* for pre-allocation of master pointers. IM-Memory: 1-42 */
#define	kMoreMasterCalls		4
/* minimum free space needed for application to be able to run (should be multiple of 1024) */
#define kMinSegSize				4096L


/***  FUNCTION PROTOTYPES  ***/

		Handle		MyNewHandle					( long, OSErr * );
		Handle		MyNewHandleClear			( long, OSErr * );
		Ptr			MyNewPtr					( long, OSErr * );
		Ptr			MyNewPtrClear				( long, OSErr * );
		
		Boolean		InitializeEmergencyMemory	( OSErr * );
		Boolean		IsEmergencyMemAvail			( void );
		void		RecoverEmergencyMemory		( void );
pascal	long		MyGrowZone					( Size );


/*** EOF ***/