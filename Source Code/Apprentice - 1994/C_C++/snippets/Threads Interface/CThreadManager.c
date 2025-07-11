/***
 * CThreadManager.c
 *
 *  Simple interface to the threads package.  Just for fun so we can create something
 *  all in one spot...
 *		Copyright � Gordon Watts 1994 (gwatts@fnal.fnal.gov)
 *
 ***/
#include "CThreadManager.h"
#include <Threads.h>
#include <Exceptions.h>
#include "ReminderConst.h"
#include "CMainThread.h"
#include "CErrorRecorder.h"
#include "util.h"

/**
 * CThreadManager
 *
 *  Make sure the thread manager is present!!
 *
 **/
CThreadManager :: CThreadManager (void)
{
	unsigned long	threadVersion;
	OSErr			theErr;

	/**
	 ** Do the gestalt thing to make sure the thread manager is present, and has
	 ** at least the basic capabilities.
	 **/
	
	theErr = Gestalt (gestaltThreadMgrAttr, &threadVersion);
	if (theErr != noErr) {
		CErrorRecorder :: gError -> Message ("\pNo thread manager present");
		Failure (errNoThreadManager, 0L);
	}

	/**
	 ** Make sure the version is present...
	 **/

	if ((threadVersion & (1 << gestaltThreadMgrPresent)) != 1) {
		CErrorRecorder :: gError -> Message ("\pNeed later version of thread manager!");
		Failure (errNoThreadManager, 0L);
	}

	/**
	 ** Start up the main thread.  This is just to make sure the thread switcher
	 ** is up and working...
	 **/
	
	theMainThread = 0;
	theMainThread = new CMainThread;
	AssertStr (theMainThread, "\pNo main thread was allocated! :( ");
	theMainThread -> Start ();

}

/**
 * ~CMainThread
 *
 *  Kill off the main thread
 *
 **/
CThreadManager :: ~CThreadManager (void)
{
	AssertStr (theMainThread, "\ptheMainThread was zero when tried to kill thread mgr!");
	theMainThread -> Kill ();
	delete theMainThread;
}