/***
 * CTestThread.c
 *
 *  Method to do the threads
 *		Copyright � Gordon Watts 1994 (gwatts@fnal.fnal.gov)
 *
 ***/
#include "CTestThread.h"
#include "Exceptions.h"
#include "util.h"

/**
 * CTestThread
 *
 *  Make sure we delete ourselves when we quit...
 *
 **/
CTestThread :: CTestThread (void)
{
	DeleteOnFinish (true);
	quitNow = false;
	theStackSize = 2048;
}
/**
 * ThreadRoutine
 *
 *  Called to do the work...
 *
 **/
void CTestThread :: ThreadRoutine (void)
{
	extern short numActive;
	short	randNum;

	/**
	 ** Let everyone know we are now up and running...
	 **/
	
	numActive++;
	TRY {
	
		/**
		 ** Loop till we quit or do something else nasty
		 **/
		
		while (!quitNow) {
			randNum = Random ();
			randNum = randNum < 0 ? -randNum : randNum;

			randNum = randNum % 100;
			
			DoTest (randNum);
		}
		
	} CATCH {
		numActive --;
	} ENDTRY;

	/**
	 ** Let em know we are ok and we are quitting...
	 **/

	numActive--;
}

/**
 * DoTest
 *
 *  Do test number "num".
 *
 **/
void CTestThread :: DoTest (short thisTest)
{
	unsigned long howLong;

	switch (thisTest) {
		case 99:
			quitNow = true;
			break;

		case 98:
			FailOSErr (-2938);
			break;
		
		case 97:
			SysBeep (10);
			break;

		default:
			if (thisTest < 20) {
				YieldToAnyThread ();
			} else {
				howLong = (Random () % 120) + TickCount();
				SleepForAWhile (howLong);
			}
			break;
	}
}