/* Main.c */

/* this contains the main code (which isn't much) which initializes the system and */
/* invokes the event loop */

#include "MiscInfo.h"
#include "CMyApplication.h"
#include "CMyScrap.h"
#include "MenuController.h"
#include "EventLoop.h"
#include "Memory.h"
#include "Compatibility.h"
#include "CWindow.h"
#include "Profiler.h"


void	InitSystem(void);


#if __option(mc68020)
	#define CodeFor68020
#else
	#define CodeFor68000
#endif
#pragma options(!mc68020) /* this code works no matter what */


/* initialize the operating system */
void	InitSystem(void)
	{
		InitGraf(&thePort);
		InitFonts();
		FlushEvents(everyEvent,0);
		InitWindows();
		InitMenus();
		TEInit();
		InitDialogs(&MyResumeProc);
		InitCursor();
	}


/* the main loop */
void	main(void)
	{
		#if __option(profile)
		InitProfile(ProfileNumSymbols,ProfileNumLevels);
		#endif

		/* initialize the operating system's things */
		InitSystem(); /* now initialize the rest of the system */
		InitPRERR(); /* this gets first pick of the memory */
		INITAUDIT();
		InitCompatibility(); /* find out what kind of system we are running on */
		InitMemory();

		#if __option(mc68881)
			if (!HasMathCoprocessor)
				{
					PRERR(ForceAbort,"This program requires a math coprocessor.");
				}
		#endif

		#ifdef CodeFor68020
			if (!Has020orBetter)
				{
					PRERR(ForceAbort,"This program requires a 68020 or better.");
				}
		#endif

		if (!HasAppleEvents || !HasFSSpecStandardFile || !HasFSSpec || !HasFindFolder
			|| !HasGreyishTextOr)
			{
				PRERR(ForceAbort,"This program needs System 7.0 or better to run.");
			}

		/* initialize the major components of the program */
		Application = new CMyApplication;
		Scrap = new CMyScrap;
		Scrap->IScrap();
		InitMyMenus();
		Application->InitMenuBar();

		InitMyEventLoop();
		if (!FirstMemCacheValid())
			{
				PRERR(ForceAbort,"There is not enough memory for this program to run.  "
					"Try increasing it's partition size or quitting some other programs.");
			}
		ActiveWindow = Application;
		ActiveWindow->DoResume();
		TheEventLoop(NIL);  /* this IS the main event loop */
		ShutDownMyEventLoop();

		/* this is the shutdown sequence */
		Scrap->CommitSuicide();
		ShutDownMyMenus();
		/* give the application one more chance to do something */
		delete Application; /* always the last to go */

		FlushMemory();
		ENDAUDIT();
	}

#ifdef CodeFor68020
	#pragma options(mc68020) /* turn it back on if necessary */
#endif
