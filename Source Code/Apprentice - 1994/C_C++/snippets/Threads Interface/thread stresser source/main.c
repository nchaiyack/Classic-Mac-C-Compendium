/***
 * main
 *
 *  This puppy will run the thread guys in a "stressful" environment.
 *
 *  It will create 100 copies of the "random" thread and start them off.  The random
 *  thread just runs and runs and runs.  It rolls the dice and every now and then
 *  will beep, quit normally, or fail with an OS Error.
 *
 *  The main program will watch for a mouse down event and quit when it sees one.
 *  Every minute it will check to see how many threads are currently up and running.
 *  It will add till there are 100 up and running.
 *
 *  The mouse down will not try to quit all the events -- it will just die right off.
 *		Copyright © Gordon Watts 1994 (gwatts@fnal.fnal.gov)
 *
 ***/
#include "Exceptions.h"
#include "util.h"
#include "CErrorRecorder.h"
#include <Threads.h>
#include "CThreadManager.h"
#include "CErrorRecorder.h"
#include "CTestThread.h"
#include "CTimeManager.h"

void	StartMoreThreads (void);

short	numActive = 0;
#define MAX_ACTIVE_THREADS 70
#define MODE_CHANGE_TIME	36000

typedef enum {modeReFill, modeWaitTillEmpty } testMode;

main ()
{
	Boolean		quit = FALSE;
	Boolean 	gotEvent;
	EventRecord	theEvent;
	short		theEventMask;
	long		lastThreadCheck;
	CThreadManager	*threadManager;
	long		seconds;
	testMode	mode;
	long		nextModeChange;


	/**
	 ** Get memory set up
	 **/

	MaxApplZone ();
	MoreMasters ();
	MoreMasters ();

	/**
	 ** Init the QD globals -- cause we need to have them around so we can access the
	 ** random number generator...  Also, randomize the random number seed so we
	 ** don't just repeat the same experiment over and over again!
	 **/
	
	InitGraf(&thePort);
	GetDateTime (&seconds);
	randSeed = seconds;

	/**
	 ** Init the menu manager...
	 **/

	InitFonts();
	InitWindows();
	InitMenus ();

	/**
	 ** Start testing by sitting in the wait till all threads are dead before doing
	 ** a refill...
	 **/
	
	mode = modeWaitTillEmpty;
	nextModeChange = MODE_CHANGE_TIME;
	nextModeChange += TickCount();

	/**
	 ** Start up the error logger
	 **/
	
	new CErrorRecorder;

	/**
	 ** Boot up the thread manager
	 **/
	
	threadManager = new CThreadManager;

	/**
	 ** Get the time manager going too
	 **/
	
	new CTimeManager;

	/**
	 ** Set the last Thread Check to be zero.  That way we will create a bunch
	 ** of new threads the first time through the loop.
	 **/
	
	lastThreadCheck = 0;

	/**
	 ** We only want high level events (not that we are going to do anything with
	 ** them...) and mouse down events
	 **/

	theEventMask = highLevelEventMask+mDownMask;

	/**
	 **  Do everything in an error type thing...
	 **/
	
	TRY {

	/**
	 ** The main event loop
	 **/

		while (!quit) {
	
			TestMemory ();				// Just stress things out to make sure nothing is
										// happening

			if (TickCount() > nextModeChange) {
				switch (mode) {
				case	modeReFill:
					mode = modeWaitTillEmpty;
					break;
				case	modeWaitTillEmpty:
					mode = modeReFill;
					break;
				}
				nextModeChange = TickCount() + MODE_CHANGE_TIME;
				CErrorRecorder::gError -> OSError (mode, "\pChanging modes...");
			}

			switch (mode) {

				case modeReFill:
				
					if (TickCount() > lastThreadCheck) {
						StartMoreThreads ();
						lastThreadCheck = TickCount() + 60*60;
					}
					break;
				
				case modeWaitTillEmpty:
					if (numActive == 0)
						StartMoreThreads ();
					break;
			}

			gotEvent = WaitNextEvent(theEventMask,&theEvent,30,0L);
		
			switch (theEvent.what) {
	
			case nullEvent:							// Do our thread jobs
				YieldToAnyThread ();
				break;

			case mouseDown:
				quit = TRUE;
				break;
			}
		}

	} CATCH {
		/**
		 ** Error has occured.  Write it out and retry
		 **/

		CErrorRecorder :: gError -> OSError (gLastError, "\pMain Loop Error");

		RETRY;
	} ENDTRY;
	
	/**
	 ** Kill off the error recorder
	 **/
	
	delete CErrorRecorder :: gError;

}

/**
 * StartMoreThreads
 *
 *  This thing will start up more threads -- till the number of active threads
 *  is MAX_ACTIVE_THREADS.
 *
 **/
void StartMoreThreads (void)
{
	short i;
	CTestThread	*test;

	for (i = numActive; i < MAX_ACTIVE_THREADS; i++) {
		test = new CTestThread;
		test -> Start ();
	}
}