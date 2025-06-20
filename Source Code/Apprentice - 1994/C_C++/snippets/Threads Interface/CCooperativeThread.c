/***
 * CCooperativeThread.c
 *
 *  Implement a cooperative thread
 *		Copyright � Gordon Watts 1994
 *
 ***/
#include "CCooperativeThread.h"
#include <Exceptions.h>
#include "util.h"

/**
 * Start a cooperative thread on its way
 *
 **/
void CCooperativeThread :: Start (void)
{
	ThreadID	theID;

	TestMemory();

	AssertStr (theThread == kNoThreadID, "\pStarting already started thread!");

	FailOSErr (NewThread (kCooperativeThread,			/* Allow interuptions */
						  &ThreadExtProc,				/* The thing that is going be called */
						  (void *) this,				/* Argument -- this object */
						  theStackSize,					/* How much space to allocate */
						  theOptions,					/* Runtime options (like suspended) */
						  0L,							/* We don't care about the outcome */
						  &theID));						/* The thread id comming back */

	theThread = theID;
	
	if (theID == kNoThreadID)
		Failure (errCouldNotMakeThread, 0L);

	/**
	 ** Get the thread switcher up and runnign!
	 **/

	SetupThreadSwitcher (FALSE);
}