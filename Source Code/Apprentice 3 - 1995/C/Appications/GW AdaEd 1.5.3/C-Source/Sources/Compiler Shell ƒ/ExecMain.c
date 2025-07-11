/*****
 * ExecMain.c - Mac shell for adaexec (monitor)
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
#include "imain.h"

#include "UtilsSys7.h"
#include "ExecMain.h"
#include "ExecShellInterface.h"
#include "ExecShellGlobals.h"
#include "MacUtilities.h"
//#include "SFGetFolder.h"
#include "MacAdalib.h"
#include "MacMemory.h"
#include "PStrUtilities.h"
#include "FileMgr.h"

// my confirmation alert [Fabrizio Oddone]
enum {
kALRT_CONFIRMQUIT = 143
};

static void	SetMinimumStack( long minSize);
static void	InitMacintosh(void);
static void	InitProcess(void);
static void DoOSEvent(EventRecord *passEvt);
static OSErr	HandleAEquit(AppleEvent *quitAppleEvent, AppleEvent *reply, long handlerRefCon);
static void	DoHighLevelEvent (EventRecord	*theEvent);

//int adamain(int argc, char **argv);	// nonsense here [Fabrizio Oddone]

/* Local variables */
static	Boolean	gFromFinder;
static	Boolean	gFromDebugger;
static	CursHandle	gWatchCursH = nil;


//static void	ShellCursors(void);

// fixed the cursor routines [Fabrizio Oddone]

#define	ShellCursors()	if (gWatchCursH == nil) gWatchCursH = GetCursor(watchCursor)
#define WatchCursorOn()	SetCursor(*gWatchCursH)
#define WatchCursorOff()	InitCursor()

/*
void	ShellCursors(void)
{
if (cursH == nil)
	cursH = GetCursor(watchCursor);
}
*/
/*
void	WatchCursorOn(void)
{
SetCursor(*cursH);
}
*/
/*
void	WatchCursorOff(void)
{
InitCursor();
}
*/

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

/*****
 * main()
 *****/

void main(void)
{
//OSErr	err;
//char 	optionsFile[300];
int result;
int val, i, len;
//FSSpec fSpec;
//Str255 wName;
int		argc;
char	**argv;

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
	ShellCursors();

	// If the process that launched us was named Finder,
	// then call the THINK C ccommand().  Otherwise, call
	// our routine getoptions() to read options from a file.

	if (gFromFinder == false) {
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
	
			// Do some simple preprocessing of commands here before
			// starting the monitor to get name and locations of
			// files
	
			// get name of unit being executed
			if ((strcmp(argv[i], "-m") == 0) && (i+1 < argc)) {
				Str255 name;
	
				strcpy((char *)&name, argv[i+1]);
				CtoPstr((char *)&name);
				SetUnitName(name);
			}
	
			// get location of user's library
			else if ((strcmp(argv[i], "-l") == 0) && (i+1 < argc)) {
				Str255 lib;
				FSSpec spec;
				OSErr err;
	
				strcpy((char *)&lib, argv[i+1]);
				CtoPstr((char *)&lib);
				err = FSMakeFSSpecCompat(0, 0, lib, &spec);
				if (err == noErr)
					SetLibraryFolder(&spec);
				// what to do here?
			}
		}
		StrLength(gCmdLineOptions) = len;	// clearer macro [Fabrizio Oddone]
	
		SetUpMenus();
		SetUpWindow();
	
		gShellStartupTime = TickCount();
		gShellQuitting = false;
		gProcessing = true;
	
		WatchCursorOn();
	
		val = setjmp(gJumpEnv);
		if (val == 0) {
			Str255 filename;
	//		int i;	// unused [Fabrizio Oddone]
	
			// call whatever main program
	
			//Str255 predefLib;
			//GetIndString(predefLib, 129, 1);
			//PtoCstr(predefLib);
			//set_predef((char *)predefLib);
	
			// remove file with return code before doing work
			GetReturnFile(filename);
			PtoCstr(filename);
			remove((char *)&filename);
	
			// run the monitor
			result = adamain(argc, argv);
		}
	
		FinishedRunning();
	
		gProcessing = false;
		WatchCursorOff();
	
	// removed code releasing cursors
	// never release System resources! [Fabrizio Oddone]
	
		while (gShellQuitting == false)
			HandleEvent(ULONG_MAX);	// we don't need the CPU [Fabrizio Oddone]
	
		FinishWindow();
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
//OSErr err;	// not used [Fabrizio Oddone]

MoreMasters();
MoreMasters();

InitGraf(&qd.thePort);
InitFonts();
InitWindows();
InitMenus();
TEInit();
InitDialogs(nil);
InitCursor();
FlushEvents(everyEvent, 0); // better here [Fabrizio Oddone]

	/* Install the AppleEvent handler */
// never had the SIZE resource modified in order to tell the OS
// that we support AE, so I commented this out.
// We should support oapp, odoc, pdoc, quit basically.
// The nice thing to do would be supporting option passing
// via AE, and supporting remote compiling.
// That is, let one Ada compiler run on a PowerMac, accepting
// remote requests via AppleEvents from, say, a Mac Plus.
// [Fabrizio Oddone]

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
//OSErr err;	// not used [Fabrizio Oddone]
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
		"MW Debug/68K 1.02", environmentName[0]) == 0;

}

/* Used by the next 5 routines only */

// changed to the correct type [Fabrizio Oddone]

static ApplState applState = applRunning;

void	PauseApplication(void)
{
applState = applPaused;
}

void	ResumeApplication(void)
{
applState = applRunning;
}


void	FinishedRunning(void)
{
applState = applFinished;
}

ApplState ApplicationState(void)
{
return applState;
}

Boolean	ApplicationIsPaused(void)
{
return applState == applPaused;
}

/****
 *	QuitApplication()
 *
 ****/

// now we can cancel quitting when a program is
// running [Fabrizio Oddone]

void	QuitApplication(void)
{
if (!(ApplicationState() == applRunning && (WatchCursorOff(), CautionAlert(kALRT_CONFIRMQUIT, nil) == cancel))) {
	gShellQuitting = true;
	longjmp(gJumpEnv, 1);
	}
WatchCursorOn();
}

/****
 * HandleEvent()
 *
 *		The main event dispatcher. This routine should be called
 *		repeatedly (it  handles only one event).
 *
 *****/

// now accepts a sleep value because it can be called either
// when we want the most of the CPU for us, or when
// we want to leave it to other processes [Fabrizio Oddone]

void HandleEvent(unsigned long sleepVal)
{
EventRecord	theEvent;

if (WaitNextEvent (everyEvent, &theEvent, sleepVal, nil))
	Do1Event(&theEvent);
}		/* end HandleEvent */

/****
 *	Do1Event
 *
 ****/

void	Do1Event(EventRecord *theEvent)
{
  switch (theEvent->what) {
	case mouseDown:
		WatchCursorOff();
		HandleMouseDown(theEvent);


		// Turn watch cursor On if the application is
		// running
		if (ApplicationState() == applRunning)
			WatchCursorOn();
		break;
	case keyDown: 
	case autoKey:
		HandleKeys(theEvent);
		break;
	case updateEvt:
	case activateEvt:
		HandleWindowEvent(theEvent);
		break;
	case osEvt :
		DoOSEvent(theEvent);
		break;
	case kHighLevelEvent:
		DoHighLevelEvent(theEvent);
		break;			/* we have received an AppleEvent */
	}
}

void DoOSEvent(EventRecord *passEvt)
{
register Boolean	willbeActive;

if ((*(Byte *)&passEvt->message) == suspendResumeMessage) {
	if (willbeActive = (CHARFROMMESSAGE(passEvt->message) & resumeFlag)) {
//		gInTheForeground = true;
		if (ApplicationState() == applRunning)
			WatchCursorOn();
		else
			WatchCursorOff();
		if (CHARFROMMESSAGE(passEvt->message) & convertClipboardFlag)
			(void)TEFromScrap();
		}
	else {
//		gInTheForeground = false;
		WatchCursorOff();
		}
	}
}

/****
 *	HandleKeys(theEvent)
 *
 ****/
// HandleKeys now behaves in a more user-friendly manner [Fabrizio Oddone]

void	HandleKeys(EventRecord *theEvent)
{
if (theEvent->modifiers & cmdKey) {
	AdjustMenus();
	HandleMenu(MenuKey(theEvent->message & charCodeMask));
	}

// Turn watch cursor off if the application is
// paused or if the application is finished.

if (ApplicationIsPaused() ||
	(ApplicationState() == applFinished))
	WatchCursorOff();
else
	WatchCursorOn();
}

/****
 * HandleMouseDown (theEvent)
 *
 *	Take care of mouseDown events.
 *
 ****/

void HandleMouseDown (EventRecord *theEvent)
{
	WindowPtr	theWindow;
	short		windowCode = FindWindow (theEvent->where, &theWindow);	// FindWindow returns short, not int [Fabrizio Oddone]

	switch (windowCode) {
		case inSysWindow: 
			SystemClick (theEvent, theWindow);
			break;
	    
		case inMenuBar:
			AdjustMenus();
			HandleMenu(MenuSelect(theEvent->where));
			break;
	
		case inDrag:
			if (theWindow == gShellWindow)
				DragWindow(gShellWindow, theEvent->where, &qd.screenBits.bounds);	// got rid of useless extra variable [Fabrizio Oddone]
			break;

		case inGoAway:	// if they click in goAway, then just quit
		  	if (theWindow == gShellWindow)
				if (TrackGoAway(gShellWindow, theEvent->where))
					QuitApplication();
			break;

		case inContent:
			if (theWindow == gShellWindow) {
				if (theWindow != FrontWindow())
					SelectWindow(gShellWindow);
				else
					InvalRect(&gShellWindow->portRect);
			}
	  		break;
	}

}		/* end HandleMouseDown */


/****
 *	HandleWindowEvent
 *
 ****/

void	HandleWindowEvent(EventRecord *theEvent)
{
	WindowPtr theWindow = (WindowPtr) theEvent->message;

	switch (theEvent->what) {
		case updateEvt:
			if (theWindow == gShellWindow) {
				BeginUpdate(gShellWindow);
				DrawWContents(((WindowPeek) gShellWindow)->hilited);
				EndUpdate(gShellWindow);
			}
		    break;
		    
		case activateEvt:
			if (theWindow == gShellWindow)
				InvalRect(&gShellWindow->portRect);
			break;
	}
}


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
//OSErr err;	// not used [Fabrizio Oddone]

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

