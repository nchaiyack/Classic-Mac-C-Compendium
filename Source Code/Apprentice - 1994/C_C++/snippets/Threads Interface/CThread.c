/***
 * CThread.c
 *
 *  Simple, abstract, interface to the threads package.
 *		Copyright � Gordon Watts 1994 (gwatts@fnal.fnal.gov)
 *
 ***/
#include "CThread.h"
#include "CErrorRecorder.h"
#include <Exceptions.h>
#include "util.h"

#define errStartBUG 400					/* Hey mon, internal programming error! */

static short gThreadCount = 0;
extern FailInfo *gTopHandler;

/**
 * CThread
 *
 *  Zero our variables
 *
 **/
CThread :: CThread (void)
{
	theThread = 0;
	theOptions = kFPUNotNeeded;
	recycleThread = false;
	theStackSize = 0;					/* Use default size. */
	deleteWhenDone = false;
}

/**
 * ~CThread
 *
 * Kill off this thread if we need to
 *
 **/
CThread :: ~CThread (void)
{
	if (isRunning()) {
		Kill ();
	}
	AssertStr (theThread == 0, "\pBad bug -- thread not zero after dele of thread obj!");
}

/**
 * Start
 *
 *  Start up the thread.  This is a subclass responsibility in a really big way.
 *
 **/
void CThread :: Start (void)
{
	Failure (errStartBUG, 0L);
}

/**
 * ThreadRoutine
 *
 *  This is the method that does the work for this thread.  It will be called in
 *  the thread's environment
 *
 **/
void CThread :: ThreadRoutine (void)
{
}

/**
 * ThreadExtProc
 *
 *  The class, external procedure that calls the thread routine
 *
 **/
static pascal void *CThread :: ThreadExtProc (void *param)
{
	CThread *theThreadObj = (CThread *) param;
	short	myCount = gThreadCount;
	FailInfo		*myHandler;

	gThreadCount++;

	TRY {

		myHandler = gTopHandler;
		theThreadObj -> ThreadRoutine ();
		if (myHandler != gTopHandler)
			CErrorRecorder :: gError -> Message ("\pHandler does not match!");

	} CATCH {

		if (gLastError != kSilentErr)
			CErrorRecorder :: gError -> OSError (gLastError, "\pA thread died abnormally");

		NO_PROPAGATE;

	} ENDTRY;

	/**
	 ** Clean up now
	 **/

	theThreadObj->theThread = 0;

	/**
	 ** If we are going to delete this thread, had better do it now...
	 **/

	if (theThreadObj->deleteWhenDone)
		delete theThreadObj;

	return noErr;
}

/**
 * Kill
 *
 *  Kill this thread.  Right off the bat.  Boom.
 *
 **/
void CThread :: Kill (void)
{
	ThreadID	tempID;

	AssertStr (theThread != kNoThreadID, "\pCan not kill a non-existant thread!");

	FailOSErr (ThreadBeginCritical());

	tempID = theThread;
	theThread = kNoThreadID;

	FailOSErr (DisposeThread (tempID, 0L, recycleThread));

	FailOSErr (ThreadEndCritical());
}

/**
 * Sleep
 *
 *  Put this thread to sleep
 *
 **/
void CThread :: Sleep (void)
{
	AssertStr (theThread != kNoThreadID, "\pBad thread to put to sleep");

	FailOSErr (SetThreadState (theThread, kStoppedThreadState, kNoThreadID));
}

/**
 * DeleteOnFinish
 *
 *  Let outside folks decide if they should release our memory when our thread
 *  executes.  In other words -- it is a one time only type of thing.
 *
 **/
void CThread :: DeleteOnFinish (Boolean doIt)
{
	deleteWhenDone = doIt;
}
