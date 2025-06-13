#pragma once
/*	
*	gVariables.h
*
*	AppLauncher
*	^^^^^^^^^^^
*
*	Declaration of global variables used by application.
*	Tuck them all into one struct which can then be accessed app. wide.
*	Do this to cut down on global name–space polution & keep track of
*	program globals (they are easy to find if they are all in one struct!)
*	© Andrew Nemeth (where applicable), Warrimoo Australia 1995
*
*	File created:		8 Mar 95.
*	Modified:			8 Mar 95.
*/

struct	myGlobalStruct {
					Boolean	boolDone;

					//	Values that can be adjusted by other application code to change
					//	the behavior of the MainEventLoop.
					//
					//	Rules of thumb:
					//
					//		Increase 'xxxRunQuantum' (and decrease 'xxxSleepQuantum') when:
					//			The application has many threads running that need time
					//
					//		Decrease 'xxxRunQuantum' when:
					//			Sending AppleEvents to other applications
					//			Launching other applications
					//			Running in the background

					unsigned long	foregroundRunQuantum,
								foregroundSleepQuantum,
								backgroundRunQuantum,
								backgroundSleepQuantum;
					};

extern 	struct myGlobalStruct 	*gptrGlobalsRec;
