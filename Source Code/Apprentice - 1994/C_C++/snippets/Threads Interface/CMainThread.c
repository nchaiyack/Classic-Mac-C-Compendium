/***
 * CMainThread.c
 *
 *  Method to implement the main thread.
 *		Copyright � Gordon Watts 1994 (gwatts@fnal.fnal.gov)
 *
 ***/

#include "CMainThread.h"
#include "CErrorRecorder.h"
#include <Exceptions.h>
#include "util.h"

extern FailInfo		*gTopHandler;
extern char			gDefaultPropagation;

/**
 * CMainThread
 *
 *  Init.  We must be created in the main thread.  If we are not, then we aren't
 *  going to work properly!
 *
 **/
CMainThread :: CMainThread (void)
{
	HLock (this);
	FailOSErr(GetCurrentThread (&theThread));
	HUnlock (this);
	AssertStr (theThread != kNoThreadID, "\pBad Thread Id!");

	theSInfo = (switcherInfo *) NewPtrClear (sizeof (switcherInfo));
	FailMemError ();
	AssertStr (theSInfo, "\pHey -- bad bad nil pointer but no mem error!");
}

/**
 * ~CMainThread
 *
 *  Close up shop
 *
 **/
CMainThread :: ~CMainThread (void)
{
	theThread = kNoThreadID;

	DisposPtr (theSInfo);
	FailMemError ();
	theSInfo = 0;
}


/**
 * isRunning
 *
 *  Return true if this thread is up and going
 *
 **/
Boolean CMainThread :: isRunning (void)
{
	return (theThread != kNoThreadID);
}

/**
 * Start
 *
 *  We don't actually start the thread (this puppy is already running) but we do
 *  setup the correct catch/fail handlers...
 *
 **/
void CMainThread :: Start (void)
{
	SetupThreadSwitcher (TRUE);
}

/**
 * Stop
 *
 *  Tear down the thread switcher.  We don't really stop the thread because this
 *  is the main thread.
 *
 **/
void CMainThread :: Kill (void)
{
	KillThreadSwitcher ();
}

/**
 * SetupThreadSwitcher
 *
 *  Setup the thread switcher routine.  If the argument is false, us the orignal
 *  settings of everything we know about.  Otherwise, just use the current values cnd
 *  take them as correct (tjis is so a regular thread won't be using the application's
 *  failure handler when it dies -- might not even exist any longer!).
 *
 **/
void CMainThread :: SetupThreadSwitcher (Boolean useCurrentValues)
{
	AssertStr (theThread != kNoThreadID, "\pNo thread up and running yet!");
	AssertStr (theSInfo, "\pNo space allocated for storage block!");

	/**
	 ** Init the block of values that we are going to use to pass to the thread
	 ** switcher routine.
	 **/

	theSInfo->theAppA5 = SetCurrentA5 ();
	theSInfo->threadObj = this;

	/**
	 ** Save the current values of the exceptions stuff
	 **/

	if (useCurrentValues) {
		savedLastError = gLastError;
		savedLastMessage = gLastMessage;
		savedDefaultPropagation = gDefaultPropagation;
		savedTopHandler = gTopHandler;
	} else {
		savedLastError = 0;
		savedLastMessage = 0;
		savedTopHandler = 0L;
		savedDefaultPropagation = TRUE;
	}

	FailOSErr (SetThreadSwitcher (theThread, &switcherCallBackIn, theSInfo, true));
	FailOSErr (SetThreadSwitcher (theThread, &switcherCallBackOut, theSInfo, false));
}

/**
 * KillThreadSwitcher
 *
 *  Turn off the thread switcher now -- don't need it any longer...
 *
 **/
void CMainThread :: KillThreadSwitcher (void)
{
	FailOSErr (SetThreadSwitcher (theThread, 0L, 0L, true));
	FailOSErr (SetThreadSwitcher (theThread, 0L, 0L, false));
}

/**
 * switcherCallBack
 *
 *  Routine that is called back with the switcher info -- we call the object routine
 *  to do the actual work (after setting up the A5 world, of course)
 *
 **/
static pascal void CMainThread :: switcherCallBackIn (ThreadID threadSwitching, void *ptr)
{
	switcherInfo 	*theInfo = (switcherInfo *) ptr;
	long			oldA5;

	oldA5 = SetA5 (theInfo -> theAppA5);
	
	if (theInfo -> threadObj == 0) {
		CErrorRecorder :: gError -> IMessage ("\pBad thread object in switch in routine");
	} else {
		(theInfo -> threadObj) -> DoSwitchIn ();
	}
	SetA5 (oldA5);
}

/**
 * swktcherCallBackOut
 *
 *  Called during interupt time when we are attempting to swtich out a thread.
 *  Setup the a5 world, and call a real thread method to do the work
 *
 **/
static pascal void CMainThread :: switcherCallBackOut (ThreadID theThread, void *ptr)
{
	switcherInfo	*theInfo = (switcherInfo *) ptr;
	long			oldA5;
	
	oldA5 = SetA5 (theInfo -> theAppA5);
	if (theInfo -> threadObj == 0) {
		CErrorRecorder :: gError -> IMessage ("\pBad thread object in switch out routine");
	} else {
		(theInfo -> threadObj) -> DoSwitchOut ();
	}
	SetA5 (oldA5);
}

/**
 * DoSwitchIn
 *
 *  Ok, this thread is on its way in, better set up everything!
 *
 **/
void CMainThread :: DoSwitchIn (void)
{
	gLastError = savedLastError;
	gLastMessage = savedLastMessage;
	gTopHandler = savedTopHandler;
	gDefaultPropagation = savedDefaultPropagation;
}

/**
 * DoSwktchOut
 *
 *  Thread is on its way out -- save our stuff!
 *
 **/
void CMainThread :: DoSwitchOut (void)
{
	savedLastError = gLastError;
	savedLastMessage = gLastMessage;
	savedTopHandler = gTopHandler;
	savedDefaultPropagation = gDefaultPropagation;
}
