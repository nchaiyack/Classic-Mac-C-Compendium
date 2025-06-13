/*****
 * ShellMain.c
 *
 *	Main program for adafront, adagen, and adabind.
 *
 *****/
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */


#include <console.h>
#include <string.h>
//#include <pascal.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif
#ifndef __EVENTS__
#include <Events.h>
#endif

#include "config.h"		// to get access to RC_xxxx codes
#include "ShellGlobals.h"

//#include "Shell.h"
#include "AdaFileTypes.h"
#include "ShellInterface.h"
#include "MacUtilities.h"
#include "MacMemory.h"
#include "Independents.h"
#include "SpinCursor.h"
#include "FileMgr.h"

/* Local variables */
static	Boolean	gFromFinder;
static	Boolean	gFromDebugger;

/* Prototypes */
static void	InitMacintosh(void);
static void	InitProcess(void);
static void	WriteErrorCode(short errCode);
static void	HandleMouseDown (EventRecord	*theEvent);
static OSErr	HandleAEquit(AppleEvent *quitAppleEvent, AppleEvent *reply, long handlerRefCon);
static void	DoHighLevelEvent (EventRecord	*theEvent);
static void	SetMinimumStack( long minSize);
static void	HandleEvent(void);

static int	adamain(int argc, char **argv);


/******************************************************************************
 SetMinimumStack

  	Sets the stack to at least minSize. Only call this once, as the very
	first statement in your program
	(borrowed from TCLUtilities.h)
******************************************************************************/

void SetMinimumStack( long minSize)
{
	long newApplLimit;
	
	if (minSize > LMGetDefltStack())
	{
		newApplLimit = (long) GetApplLimit() - (minSize - LMGetDefltStack());
		SetApplLimit( (Ptr) newApplLimit);
	}
}

/****
 *	main()
 *
 *	This is where everything happens
 *
 ****/

void main(void)
{
Str255	partName;
//OSErr	err;
int result;
int val, len;
int		argc;
char	**argv;
//short	cursor;
long	elapsedTime;
int	i;
//Str255	str;

// require more stack space when using the profiler for
// performance tuning [Fabrizio Oddone]

#if __profile__
	SetMinimumStack(384*1024L);		// stack to 50K
#else
	SetMinimumStack(64*1024L);		// stack to 50K
#endif

MaxApplZone();
	InitMacintosh();
	InitProcess();

	//GetDateTime((unsigned long *)&randSeed);
	//cursor = 128 + ((Random() * (136 - 128)) / 65536);
	LoadSpinning(130);

	//gResults[0] = 0;

	if (gFromFinder == false || ModifiersState(optionKey) ) {
		if (gFromDebugger) {
			argc = ccommand(&argv);
			}
		else {
			Str255 optionsFile;
	
			GetOptionsFile(optionsFile);
			PtoCstr((unsigned char *)optionsFile);
			argc = getoptions((char *)optionsFile, &argv);
			}
	
		// Build this line by concatenating all the argv strings
		// without the name of the process
		len = 0;
		for (i = 1; i < argc; i++)
			len += strlen(argv[i]) + 1;
	
		gCmdLineOptions = mmalloc(len + 2);
		gCmdLineOptions[1] = '\0';
		for (i = 1; i < argc; i++) {
			strcat(&gCmdLineOptions[1], argv[i]);
			strcat(&gCmdLineOptions[1], " ");
		}
		gCmdLineOptions[0] = len;
	
		SetUpMenus();
		GetPartName(partName);
		PtoCstr(partName);
		if (strncmp((char *)&partName, "adafront", strlen("adafront")) == 0)
			SetUpWindow(true);
		else
			SetUpWindow(false);
	
		gShellStartupTime = TickCount();
		gShellQuitting = false;
		gProcessing = true;
	
		StartSpinning();
	
		elapsedTime = TickCount();
		val = setjmp(gJumpEnv);
		if (val == 0) {
			Str255	filename;
			short	j;
	
			// right before we call this routine, call
			// HandleEvent a couple of times so that we
			// handle any events upfront before we go
			// through.
			for (j = 3; j >= 0; j--)
				HandleEvent();
	
			// remove file with return code before doing work
			GetReturnFile(filename);
			PtoCstr(filename);
			//GetPartName(filename);
			//strcat(filename, ".return");
			remove((char *)&filename);
	
			result = adamain(argc, argv);
		}
		else
			val -= 10;		// see note below
	
		elapsedTime = TickCount() - elapsedTime;
		//NumToString(elapsedTime, str);
		//DebugStr(str);
	
		// Four return codes are returned from adamain
		// or setjmp.  These are RC_SUCCESS, RC_ERRORS
		// RC_INTERNAL_ERROR, RC_ABORT.
		//
		// We always return via the setjmp.  Unfortunately
		// the ada programs were not written to return from
		// their main program, thus there is no exit point
		// in adamain.  Values returned are one of the RC
		// codes above with 10 added to them.  Why 10? Because
		// I don't like 11.  No, really, setjmp returns non-
		// zero when longjmp() is called.  That being the
		// case we couldn't return RC_SUCCESS (that is defined
		// to zero).  Thus we just add 10 to all the RCs and
		// we have our non-zero values.
	
	
		// A note on gShellQuitting:  The event loop at the
		// bottom is entered only if gShellQuitting is false.
		// The compiler part can set this variable to decide
		// what to do after compilation.  We do this to allow
		// this shell to continue executing or simply quit
		// and putting the decision on the individual compilation
		// tool.  (possibly, but not yet!)
	
		if (val != RC_SUCCESS)
			WriteErrorCode(val);
	
		//if (val == RC_SUCCESS) {
		//	pstrcpy(gResults, "\pRC_SUCCESS");
		//}
	
		//else if (val == RC_ERRORS) {
		//	pstrcpy(gResults, "\pRC_ERRORS");
		//}
	
		//else if (val == RC_INTERNAL_ERROR) {
		//	pstrcpy(gResults, "\pRC_INTERNAL_ERROR");
		//}
	
		//else if (val == RC_ABORT) {
		//	pstrcpy(gResults, "\pRC_ABORT");
		//}
		//else {
		//	pstrcpy(gResults, "\pHuh? Internal Error.");
		//}
	
	
		// force a redraw of the window
		EraseRect(&gProgressArea);
		InvalRect(&gShellWindow->portRect);
		gProcessing = false;
	
		ProgressBar(kProgressBarWidth);		// done
	
		StopSpinning();
	
		// If there was an error and If we were launched
		// by the Finder or by the debugger, then keep us around
		// (don't quit).
	
		if ((gFromFinder || gFromDebugger) && (val != RC_SUCCESS))
			while (!gShellQuitting)
				HandleEvent();
		}
}	/* end main */


/****
 * InitMacintosh()
 *
 * Initialize all the managers & memory
 *
 ****/

void InitMacintosh(void)

{
//OSErr err;

MoreMasters();
MoreMasters();

InitGraf(&qd.thePort);
InitFonts();
InitWindows();
InitMenus();
TEInit();
InitDialogs(nil);
InitCursor();
FlushEvents(everyEvent, 0);

	/* Install the AppleEvent handler */
//	err = AEInstallEventHandler(kCoreEventClass,
//		kAEQuitApplication, (AEEventHandlerUPP)HandleAEquit, 0, false);

	/* Install an Open Event AppleEvent handler */
}	/* end InitMacintosh */


/***
 *	InitProcess()
 *
 *	Initialize variables based on process information.  Call the FileMgr
 *	to store information about the compiler and the environment.
 *
 ***/

void InitProcess(void)
{
//OSErr err;
ProcessInfoRec info;

/* Compiler part (adafront, adagen, adabind) */
ProcessSerialNumber	compilerPSN;
Str255				compilerName;
FSSpec				compilerFSpec;

/* GWAda Programming Environment */
ProcessSerialNumber	environmentPSN;
Str255				environmentName;
FSSpec				environmentFSpec;

	// initialize these to zero strings
	compilerName[0] = 0;
	environmentName[0] = 0;

	// Get information about the current process
	compilerPSN.highLongOfPSN = 0;
	compilerPSN.lowLongOfPSN = kCurrentProcess;	// use current process

	info.processName = (StringPtr) &compilerName;
	info.processAppSpec = &compilerFSpec;
	info.processInfoLength = sizeof(ProcessInfoRec);

	if (GetProcessInformation(&compilerPSN, &info) == noErr) {

		SetCompilerFolder(&compilerFSpec);
		SetPartName(compilerName);

		// Get process information about the environment, that is
		// the process that launched the compiler part.

		environmentPSN = info.processLauncher;		// from compiler process info

		info.processName = (StringPtr) &environmentName;
		info.processAppSpec = &environmentFSpec;
		info.processInfoLength = sizeof(ProcessInfoRec);

		if (GetProcessInformation(&environmentPSN, &info) == noErr) {

			SetApplicationFolder(&environmentFSpec);
		}

	}
	else {
		ParamText("\pInternal Error: GetProcessInformation() failed. ",
			"\pThe application could not be initialized properly.",
			"\p", "\p");
		StopAlert(128, NULL);
		ExitToShell();
	}

	// If the process that launched us was named Finder,
	// then call the THINK C ccommand().  Otherwise, call
	// our routine getoptions() to read options from a file.

	gFromFinder = strncmp((char *)&environmentName[1],
		"Finder", environmentName[0]) == 0;
	gFromDebugger = strncmp((char *)&environmentName[1],
		"MW Debug/68K 1.1", environmentName[0]) == 0;

}

/****
 *	WriteErrorCode
 *
 *	Write error code before returning to the GWAda environment.
 *
 ****/

void WriteErrorCode(short errCode)
{
	Str255 filename;
	FILE *fp;

	// Now handle writing the errors to the file
	GetReturnFile(filename);
	PtoCstr(filename);

	FileType('ARTN');
	fp = fopen((char *)&filename, "w");
	if (fp) {
		fprintf(fp, "%ld\n", (long)errCode);
		fclose(fp);
	}
}

/****
 * HandleMouseDown (theEvent)
 *
 *	Take care of mouseDown events.
 *
 ****/

void HandleMouseDown (EventRecord	*theEvent)

{
	WindowPtr	theWindow;
	short		windowCode = FindWindow (theEvent->where, &theWindow);
	
    switch (windowCode)
      {
	  case inSysWindow: 
	    SystemClick (theEvent, theWindow);
	    break;
	    
	  case inMenuBar:
	  	AdjustMenus();
	    HandleMenu(MenuSelect(theEvent->where));
	    break;
	    
	  case inDrag:
	  	if (theWindow == gShellWindow)
	  	  DragWindow(gShellWindow, theEvent->where, &qd.screenBits.bounds);
	  	  break;
	  	  
	  case inContent:
	  	if (theWindow == gShellWindow)
	  	  {
	  	  if (theWindow != FrontWindow())
	  	    SelectWindow(gShellWindow);
	  	  else
	  	    InvalRect(&gShellWindow->portRect);
	  	  }
	  	break;
	}
}		/* end HandleMouseDown */

/****
 * HandleEvent()
 *
 *		The main event dispatcher. This routine should be called
 *		repeatedly (it  handles only one event).
 *
 *****/

// now uses current event handling routines [Fabrizio Oddone]

void HandleEvent(void)

{
EventRecord	theEvent;

if (WaitNextEvent(everyEvent, &theEvent, 0L, nil)) {
	switch (theEvent.what)
		{
		case mouseDown:
			HandleMouseDown(&theEvent);
			break;
		case keyDown: 
		case autoKey:
		    if (theEvent.modifiers & cmdKey)
		      {
		      AdjustMenus();
			  HandleMenu(MenuKey((char) (theEvent.message & charCodeMask)));
			  }
			break;
			
		case updateEvt:
			BeginUpdate(gShellWindow);
			DrawWContents(((WindowPeek) gShellWindow)->hilited);
			EndUpdate(gShellWindow);
		    break;
		    
		case activateEvt:
			InvalRect(&gShellWindow->portRect);
			break;
	
		case kHighLevelEvent:
			DoHighLevelEvent(&theEvent);
			break;			/* we have received an AppleEvent */
	    }
	}
}		/* end HandleEvent */


/*******************************************************************************}
{* HandleAEquit - Handler for 'quit' AppleEvent}
{*}
{* This is the AppleEvent handler for the 'quit' AppleEvent as passed in the}
{* quitAppleEvent parameter by the AppleEvent Manager.  The DoQuit routine is}
{* called which causes this application to quit at the start of the next}
{* iteration of the main event loop.}
{*}
{* Though the quit AppleEvent doesn�t contain any parameters, the standard thing}
{* to do in reaction to any AppleEvent is to check to see if there are any}
{* required parameters in the AppleEvent that this routine doesn�t recognise.}
{* DoneRequiredParms checks for this condition and returns an error if there are}
{* in fact required parameters in the AppleEvent or if some other error occurs}
{* during the check.}
{*******************************************************************************/
OSErr HandleAEquit(AppleEvent *quitAppleEvent, AppleEvent *reply, long handlerRefCon)
{
//OSErr err;

	/* quit AE has no parms, but check in case the client requires any */
	/* err = DoneRequiredParams(quitAppleEvent); */

	/* Handle the Quit command */
//	ExitToShell();		I shouldn't do this here!
	gShellQuitting = true;
	return noErr;
}


/*******************************************************************************}
{* DoHighLevelEvent - Handle a high-level event}
{*}
{* This routine handles the high-level event specified by anEvent.  The only}
{* high-level events that this application handles are AppleEvents, so I just}
{* pass the high-level event to AEProcessAppleEvent.  AEProcessAppleEvent calls}
{* the appropriate AppleEvent handler routine to handle that particular kind of}
{* AppleEvent.}
{*******************************************************************************/

void DoHighLevelEvent (EventRecord	*theEvent)
{
OSErr err;

	err = AEProcessAppleEvent(theEvent);
}




/* Call to get the name of the compiler part that is running.
 * This routine is used to write status information to disk
 * by appending an extensions to this name.  For example,
 * the result code from adafront is written in a file with the
 * name adafront.options.  The name passed back contains the
 * full path of the compiler part.
 */
//void	xxGetPartName(char *name)
//{
//	BlockMoveData(&compilerName[1], name, compilerName[0]);
//	name[compilerName[0]] = '\0';
//}
