/*********************************************************************
 * Basic Black �1993,1994 by Mason L. Bliss
 * All Rights Reserved
 * - version 1.4.1 -
 *
 * The trap patching shell is taken from a neat little program
 * Mike Scanlin wrote for the August '92 MacTutor.
 *
 * Basic Black's screen-blanking routine was inspired by code written
 * by Christopher Tate. (Public Blanket)
 *
 * Basic Black makes use of CShowProc, by Ken McLeod, to show its
 * icon animation at startup time.
 *
 * Basic Black would be nowhere near as nice as it is without the
 * consistent help of my beta testers, who are listed in the Basic Black
 * read-me file. Thank you! ;-)
 *
 *********************************************************************/

#include <GestaltEqu.h>
#include <ShutDown.h>
#include <Traps.h>
#include "BB.h"
#include "BBinit.h"



/*********************************************************************
 * main:
 *
 * Gets memory in the system heap, installs our patches, and initializes
 * our patch globals. This is the only routine that gets executed at
 * startup time (by the INIT mechanism).
 *
 * Note that the code resource should be purgeable. The routines listed
 * below aren't used - the code, after it's loaded, is COPIED into the
 * area we set up in the system heap, after which the originals are
 * discarded, as it's the COPIES that are used. If the code resource is
 * set to "Not Purgeable" then the originals - which are never again
 * referenced - stick around until shutdown time, wasting memory.
 *
 * Also, with Think C, make sure that "Custom Headers" is turned on.
 *
 * The block of memory that we allocate will look like this when main()
 * has finished:
 *
 *					 +--------------------+
 *					 |	  PatchGlobals	  |
 *					 +--------------------+
 *					 |  StartPatchCode()  |
 *	 SE trap addr -> +--------------------+
 *					 |  MySystemEvent()   |
 *	 IC trap addr -> +--------------------+
 *					 |  MyInitCursor()    |
 *	DMB trap addr -> +--------------------+
 *					 |  MyDrawMenuBar()   |
 *	 ER trap addr -> +--------------------+
 *					 |  MyEraseRect()     |
 *	 EO trap addr -> +--------------------+
 *					 |  MyEraseOval()     |
 *	 EG trap addr -> +--------------------+
 *					 |  MyEraseRgn()      |
 *	 ST trap addr -> +--------------------+
 *					 |  MySystemTask()    |
 *					 +--------------------+
 *					 |  FallAsleep()      |
 *					 +--------------------+
 *					 |  WakeUp()          |
 *					 +--------------------+
 *					 |  abs()             |
 *					 +--------------------+
 *					 |  DrawClock()       |
 *					 +--------------------+
 *					 |  RemoveICPatch()   |
 *					 +--------------------+
 *					 |  BBlkSelector()    |
 *					 +--------------------+
 *					 |  SAVRSelector()    |
 *					 +--------------------+
 *					 |  SAVCSelector()    |
 *					 +--------------------+
 *					 |  SaverControl()    |
 *					 +--------------------+
 *					 |  EndPatchCode()    |
 *					 +--------------------+
 *
 *********************************************************************/
void main()
{
	Ptr					patchPtr;
	PatchGlobalsPtr		pgPtr;
	long				codeSize, offset, oldA5;
	QDGlobals			qd;
	GrafPort			gp;
	Handle				procH;
	PrefStructureHandle	prefHandle;
	Boolean				drawStartupIcon = false;
	
	/* try and get some memory in the system heap for code and globals */
	codeSize = (long) EndPatchCode - (long) StartPatchCode;
	patchPtr = NewPtrSys(codeSize + sizeof(PatchGlobals));
	if (!patchPtr) {
		/* put up the error icon */
		if ((procH = GetIndResource('PROC', 1)) != 0L) {
			HLock(procH);
			CallPascal(128, -1, *procH);
			HUnlock(procH);
			ReleaseResource(procH);
		}
		return;	/* out of memory -- abort patching */
	}

	/* Set up pgPtr */
	pgPtr = (PatchGlobalsPtr) patchPtr;
	
	/* Read the configuration information from our 'PREF' resource. */
	prefHandle = (PrefStructureHandle) GetIndResource('PREF', 1);

	pgPtr->pgVersion = (**prefHandle).version;

	pgPtr->pgMustSleep = false;
	pgPtr->pgIntervalTime = 20;				/* Three times per second */
	
	pgPtr->pgSleepRect = (**prefHandle).sleepNowCorner;
	pgPtr->pgWakeRect = (**prefHandle).sleepNeverCorner;
	pgPtr->pgIdleTicks = ((long) (**prefHandle).idleTime) * 3600;
	pgPtr->pgBouncingClock = (**prefHandle).bounceClock;
	pgPtr->pgMustSave = (**prefHandle).saverOn;
	pgPtr->pgRefreshTime = (**prefHandle).refreshSeconds * 60;
	pgPtr->pgMenubarKluge = (**prefHandle).zeroBar;


	drawStartupIcon = (**prefHandle).showIcon;

	if ((**prefHandle).fadeToWhite) {				// Fade to white
		StuffHex(&(pgPtr->pgForePat), "\pFFFFFFFFFFFFFFFF");
		StuffHex(&(pgPtr->pgBackPat), "\p0000000000000000");
	} else {										// Fade to black
		StuffHex(&(pgPtr->pgForePat), "\p0000000000000000");
		StuffHex(&(pgPtr->pgBackPat), "\pFFFFFFFFFFFFFFFF");
	}

	/* initialize the patch globals at the beginning of the block */
	pgPtr->pgOldSE = (SEProcPtr) GetTrapAddress(_SystemEvent);
	pgPtr->pgOldIC = (ICProcPtr) GetTrapAddress(_InitCursor);
	pgPtr->pgOldDMB = (DMBProcPtr) GetTrapAddress(_DrawMenuBar);
	pgPtr->pgOldER = (ERProcPtr) GetTrapAddress(_EraseRect);
	pgPtr->pgOldEO = (EOProcPtr) GetTrapAddress(_EraseOval);
	pgPtr->pgOldEG = (EGProcPtr) GetTrapAddress(_EraseRgn);
	pgPtr->pgOldST = (STProcPtr) GetTrapAddress(_SystemTask);

	/* move the code into place after the globals */
	BlockMove(StartPatchCode, patchPtr + sizeof(PatchGlobals), codeSize);
	
	/* set whatever patches seem appropriate at the time */
	patchPtr += sizeof(PatchGlobals);

	offset = (long) MySystemEvent - (long) StartPatchCode;
	SetTrapAddress((long) patchPtr + offset, _SystemEvent);

	offset = (long) MySystemTask - (long) StartPatchCode;
	SetTrapAddress((long) patchPtr + offset, _SystemTask);

	if ((**prefHandle).initCursorPatch) {
		offset = (long) MyInitCursor - (long) StartPatchCode;
		SetTrapAddress((long) patchPtr + offset, _InitCursor);

		/* install shutdown procedure only if InitCursor patch is installed */
		offset = (long) RemoveICPatch - (long) StartPatchCode;
		ShutDwnInstall( (ShutDwnProcPtr) ((long) patchPtr + offset),
													sdRestartOrPower + sdOnDrivers);
	}

	if ((**prefHandle).drawMenuBarPatch) {
		offset = (long) MyDrawMenuBar - (long) StartPatchCode;
		SetTrapAddress((long) patchPtr + offset, _DrawMenuBar);
	}

	if ((**prefHandle).eraseRectPatch) {
		offset = (long) MyEraseRect - (long) StartPatchCode;
		SetTrapAddress((long) patchPtr + offset, _EraseRect);
	}

	if ((**prefHandle).eraseOvalPatch) {
		offset = (long) MyEraseOval - (long) StartPatchCode;
		SetTrapAddress((long) patchPtr + offset, _EraseOval);
	}

	if ((**prefHandle).eraseRgnPatch) {
		offset = (long) MyEraseRgn - (long) StartPatchCode;
		SetTrapAddress((long) patchPtr + offset, _EraseRgn);
	}
	
	/* We're done with our prefs, so we might as well get rid of it now. */
	ReleaseResource(prefHandle);


	/* install a new 'BBlk' gestalt selector */
	offset = (long) BBlkSelector - (long) StartPatchCode;
	NewGestalt('BBlk', (ProcPtr) ((long) patchPtr + offset));

	/* install a new 'SAVR' gestalt selector */
	offset = (long) SAVRSelector - (long) StartPatchCode;
	NewGestalt('SAVR', (ProcPtr) ((long) patchPtr + offset));

	/* install a new 'SAVC' gestalt selector */
	offset = (long) SAVCSelector - (long) StartPatchCode;
	NewGestalt('SAVC', (ProcPtr) ((long) patchPtr + offset));
	
	pgPtr->pgSaverOn = false;
	pgPtr->pgMustSleep = false;
	pgPtr->pgMustWake = false;
	pgPtr->pgInSleepRect = false;
	pgPtr->pgLastAction = Ticks;
	pgPtr->pgLastCheck = Ticks;
	pgPtr->pgLastRefresh = Ticks;
		
	
	/* Read in the clock background picture. */
	/* This assumes no errors - we'll correct the assumption in the next released. */
	pgPtr->pgClockBg = (PicHandle) GetResource('PICT', 255);
	DetachResource(pgPtr->pgClockBg);

	
	/* The next three lines fake out some QuickDraw globals so we can
		figure out the correct sleep and wake rectangles, as we don't
		have access to the 'proper' QuickDraw globals. */
	oldA5 = SetA5((long) &qd.qdend);	// Point A5 at our fake globals
	InitGraf(&qd.thePort);				// ...and initialize 'em
	OpenPort((GrafPtr) &gp);
		
	pgPtr->pgCorners[0].left = (gp.portRect).left - 1;		// top left
	pgPtr->pgCorners[0].right = (gp.portRect).left + 8;
	pgPtr->pgCorners[0].top = (gp.portRect).top - 1;
	pgPtr->pgCorners[0].bottom = (gp.portRect).top + 8;

	pgPtr->pgCorners[1].left = (gp.portRect).right - 8;		// top right
	pgPtr->pgCorners[1].right = (gp.portRect).right + 1;
	pgPtr->pgCorners[1].top = (gp.portRect).top - 1;
	pgPtr->pgCorners[1].bottom = (gp.portRect).top + 8;

	pgPtr->pgCorners[2].left = (gp.portRect).right - 8;		// bottom right
	pgPtr->pgCorners[2].right = (gp.portRect).right + 1;
	pgPtr->pgCorners[2].top = (gp.portRect).bottom - 8;
	pgPtr->pgCorners[2].bottom = (gp.portRect).bottom + 1;

	pgPtr->pgCorners[3].left = (gp.portRect).left - 1;		// bottom left
	pgPtr->pgCorners[3].right = (gp.portRect).left + 8;
	pgPtr->pgCorners[3].top = (gp.portRect).bottom - 8;
	pgPtr->pgCorners[3].bottom = (gp.portRect).bottom + 1;
	
	ClosePort((GrafPtr) &gp);
	oldA5 = SetA5(oldA5);				// Restore A5 to its previous value


	/* put up our icon animation because the nice user wants us to */
	if (drawStartupIcon && (procH = GetIndResource('PROC', 1)) != 0L)	{
		HLock(procH);
		/* put up either the normal icon, or the 'off' icon, as appropriate */
		if (pgPtr->pgMustSave)
			CallPascal(-4064, -1, *procH);
		else
			CallPascal(257, -1, *procH);
		HUnlock(procH);
	}
	ReleaseResource(procH);
}



/*********************************************************************
 * StartPatchCode:
 *
 * Dummy proc to mark the beginning of the code for the
 * patches. Make sure all of your patch code is between
 * here and EndPatchCode.
 *
 *********************************************************************/
void StartPatchCode()
{
}



/*********************************************************************
 * MySystemEvent:
 *
 * Tail patch on SystemEvent.
 *
 * The reason this returns a short instead of a Boolean is because we
 * need to make sure the low byte of the top word on the stack is zero
 * because some programs do a Tst.W (SP)+ when this returns instead of
 * Tst.B (SP)+ like they should (which is technically their bug but, we
 * might as well work around it since it's not hard).
 *
 * If you want to eat the event (not pass it on to the caller) then set
 * returnValue to true.
 *
 *********************************************************************/
pascal short MySystemEvent(EventRecord *theEvent)
{
	PatchGlobalsPtr	pgPtr;
	short			returnValue;
	register short	foo;
	
	/* find our globals */
	pgPtr = (PatchGlobalsPtr) ((long) StartPatchCode - sizeof(PatchGlobals));
	
	/* call original SE first */
	returnValue = (*pgPtr->pgOldSE) (theEvent);
	
	if ((foo = theEvent->what) >= 1 && foo <= 5 || foo == 7) {
		pgPtr->pgLastAction = Ticks;		/* Update last time counter */
		pgPtr->pgInSleepRect = false;
		if (pgPtr->pgSaverOn) {
			if (foo != 7) {					/* mask keypresses and clicks */
				returnValue = true;			/* we've "handled" the event */
				theEvent->what = 0;
			}
			pgPtr->pgMustWake = true;
		}
	}
	
	/* return to original caller */
	return returnValue;
}



/*********************************************************************
 * MyInitCursor:
 *
 * Optional patch that disables InitCursor if we're asleep.
 *
 *********************************************************************/
pascal void MyInitCursor()
{
	PatchGlobalsPtr	pgPtr;
	
	/* find our globals */
	pgPtr = (PatchGlobalsPtr) ((long) StartPatchCode - sizeof(PatchGlobals));
	
	/* call original IC first */
	if (pgPtr->pgSaverOn == false)
		(*pgPtr->pgOldIC) ();
}



/*********************************************************************
 * MyDrawMenuBar:
 *
 * Optional patch that disables DrawMenuBar if we're asleep.
 *
 *********************************************************************/
pascal void MyDrawMenuBar()
{
	PatchGlobalsPtr	pgPtr;
	
	/* find our globals */
	pgPtr = (PatchGlobalsPtr) ((long) StartPatchCode - sizeof(PatchGlobals));
	
	/* call original DMB first */
	if (pgPtr->pgSaverOn == false)
		(*pgPtr->pgOldDMB) ();
}



/*********************************************************************
 * MyEraseRect:
 *
 * Optional patch that disables EraseRect if we're asleep.
 *
 *********************************************************************/
pascal void MyEraseRect(Rect *rect)
{
	PatchGlobalsPtr	pgPtr;
	
	/* find our globals */
	pgPtr = (PatchGlobalsPtr) ((long) StartPatchCode - sizeof(PatchGlobals));
	
	/* call original ER first */
	if (pgPtr->pgSaverOn == false)
		(*pgPtr->pgOldER) (rect);
}



/*********************************************************************
 * MyEraseOval:
 *
 * Optional patch that disables EraseOval if we're asleep.
 *
 *********************************************************************/
pascal void MyEraseOval(Rect *rect)
{
	PatchGlobalsPtr	pgPtr;
	
	/* find our globals */
	pgPtr = (PatchGlobalsPtr) ((long) StartPatchCode - sizeof(PatchGlobals));
	
	/* call original EO first */
	if (pgPtr->pgSaverOn == false)
		(*pgPtr->pgOldEO) (rect);
}



/*********************************************************************
 * MyEraseRgn:
 *
 * Optional patch that disables EraseRgn if we're asleep.
 *
 *********************************************************************/
pascal void MyEraseRgn(RgnHandle rgn)
{
	PatchGlobalsPtr	pgPtr;
	
	/* find our globals */
	pgPtr = (PatchGlobalsPtr) ((long) StartPatchCode - sizeof(PatchGlobals));
	
	/* call original EG first */
	if (pgPtr->pgSaverOn == false)
		(*pgPtr->pgOldEG) (rgn);
}



/*********************************************************************
 * MySystemTask:
 *
 * Patches SystemTask. This is where most of our maintenance is done.
 *
 *********************************************************************/
pascal void MySystemTask(void)
{
	PatchGlobalsPtr	pgPtr;
	Point			mousePt;
	
	/* find our globals */
	pgPtr = (PatchGlobalsPtr) ((long) StartPatchCode - sizeof(PatchGlobals));
	
	/* Has enough time passed for us to run? */
	if (Ticks - pgPtr->pgLastCheck > pgPtr->pgIntervalTime) {
	
		pgPtr->pgLastCheck = Ticks;
	
		/* Check to see if the mouse has moved */
		GetMouse(&mousePt);
		LocalToGlobal(&mousePt);
		if (!EqualPt(mousePt, pgPtr->pgLastMouse)) {	/* id est, mouse moved */
			pgPtr->pgLastMouse = mousePt;
			pgPtr->pgLastAction = Ticks;
			pgPtr->pgInSleepRect = false;
			if (pgPtr->pgSaverOn)
				pgPtr->pgMustWake = true;
		}
		
		if (!pgPtr->pgSaverOn) {	/* wide-awake maintenance */
		
			/* Does the nice user want us to fall asleep now? */
			if (!pgPtr->pgInSleepRect &&
						PtInRect(mousePt, &pgPtr->pgCorners[pgPtr->pgSleepRect])) {
	
				pgPtr->pgInSleepRect = true;
				/* Make it look like we haven't moved for a while. */
				pgPtr->pgLastAction -= ((pgPtr->pgIdleTicks) - DELAYTICKS);
			} else {
				if (PtInRect(mousePt, &pgPtr->pgCorners[pgPtr->pgWakeRect]))
					pgPtr->pgLastAction = Ticks;
			}
			
			/* Has enough idle time passed for us to fall asleep naturally? */
			if ((Ticks - pgPtr->pgLastAction) > pgPtr->pgIdleTicks)
				pgPtr->pgMustSleep = true;
	
		} else {	/* Do sleepy-time maintenance. */
		
			if (!pgPtr->pgMustWake) {
				/* Take care of the refresh timing */
				if ((Ticks - pgPtr->pgLastRefresh) > pgPtr->pgRefreshTime) {
					pgPtr->pgLastRefresh = Ticks;
					pgPtr->pgMustSleep = true;
				}
				
				/* Has the menu bar been messed with? (Don't mess with me, man!) */
				/* Canvas users truly hate this. It's Aldus' fault - not mine. */
				if (pgPtr->pgMenubarKluge && MBarHeight != 0)
					pgPtr->pgMustSleep = true;
			} else WakeUp();
		}
	
		/* Do we need to fall asleep? */
		if (pgPtr->pgMustSleep && pgPtr->pgMustSave)
			FallAsleep();
		
	}
	
	/* Give everyone else some time! */
	(*pgPtr->pgOldST) ();
}



/*********************************************************************
 * FallAsleep:
 *
 * This is where we black out the screen, and maybe draw a clock.
 *
 *********************************************************************/
void FallAsleep(void)
{
	PatchGlobalsPtr	pgPtr;
	GrafPort		myPort;
	GrafPtr			oldPort;
	
	/* find our globals */
	pgPtr = (PatchGlobalsPtr) ((long) StartPatchCode - sizeof(PatchGlobals));
	
	/* hide the cursor */
	ObscureCursor();
	
	/* Fix the menu bar if the kluge is on */
	if (pgPtr->pgMenubarKluge)
		if (MBarHeight != 0) {
			pgPtr->pgOldHeight = MBarHeight;
			MBarHeight = 0;
		} else if (!pgPtr->pgSaverOn && MBarHeight == 0)
			pgPtr->pgOldHeight = 0;
	
	/* Save the old port and set our new characteristics */
	GetPort(&oldPort);
	OpenPort(&myPort);
	PenPat(pgPtr->pgForePat);
	BackPat(pgPtr->pgBackPat);
	
	/* Get a region for everything, and paint it. */
	UnionRgn(GrayRgn, myPort.visRgn, myPort.visRgn);
	FillRgn(myPort.visRgn, pgPtr->pgBackPat);
	
	/* Draw our clock, if it's turned on. */
	if (pgPtr->pgBouncingClock)
		DrawClock(&myPort);
	
	/* Empty the visRgns of all windows */
	CopyRgn(GrayRgn, myPort.visRgn);
	SetEmptyRgn(GrayRgn);
	CalcVisBehind((WindowPeek) FrontWindow(), myPort.visRgn);
	CopyRgn(myPort.visRgn, GrayRgn);

	/* Restore the old port */
	ClosePort(&myPort);
	SetPort(oldPort);

	/* Set our flags */
	pgPtr->pgSaverOn = true;
	pgPtr->pgMustSleep = false;
	pgPtr->pgLastRefresh = Ticks;
}



/*********************************************************************
 * WakeUp:
 *
 * This is where we liven up.
 *
 *********************************************************************/
void WakeUp(void)
{
	PatchGlobalsPtr	pgPtr;
	GrafPtr			oldPort;

	/* find our globals */
	pgPtr = (PatchGlobalsPtr) ((long) StartPatchCode - sizeof(PatchGlobals));
	
	pgPtr->pgMustWake = false;
	pgPtr->pgSaverOn = false;
	pgPtr->pgLastAction = Ticks + 60;		/* can't sleep until a second from now */
	
	/* restore the menu bar */
	if (pgPtr->pgMenubarKluge)
		MBarHeight = pgPtr->pgOldHeight;
	DrawMenuBar();
	
	/* Grab the current port. */
	GetPort(&oldPort);
	
	/* Redraw window frames and set update regions equal to visible regions */
	PaintBehind((WindowPeek) FrontWindow(), GrayRgn);

	/* Calculate visRgns */
	CalcVisBehind((WindowPeek) FrontWindow(), GrayRgn);
	
	/* Reset the port - the previous two toolbox calls disrupt it. */
	SetPort(oldPort);
}



/*********************************************************************
 * abs:
 *
 * Return the absolute value of a short.
 *
 *********************************************************************/
short abs(short x)
{
	return x < 0 ? -x : x;
}



/*********************************************************************
 * DrawClock:
 *
 * Draw an analog clock at a random place in the given
 * GrafPort.
 *
 *********************************************************************/
void DrawClock(GrafPtr myPort)
{
	PatchGlobalsPtr	pgPtr;
	DateTimeRec		theTime;
	Rect			theTimeRect;
	short			mid;
	
	/* find our globals */
	pgPtr = (PatchGlobalsPtr) ((long) StartPatchCode - sizeof(PatchGlobals));

	GetTime(&theTime);
	
	/* find our clock rectangle */
	theTimeRect.top = (abs(Random()) % (myPort->portRect.bottom - 110)) + 5;
	theTimeRect.bottom = theTimeRect.top + 100;
	theTimeRect.left = (abs(Random()) % (myPort->portRect.right - 110)) + 5;
	theTimeRect.right = theTimeRect.left + 100;
	
	/* draw our background */
	DrawPicture(pgPtr->pgClockBg, &theTimeRect);
	
	/* move the rect in, and draw the minute hand */
	InsetRect(&theTimeRect, 5, 5);
	PaintArc(&theTimeRect, (theTime.minute) * 6 - 3, 6);
	
	/* move the rect in more, and draw the hour hand */
	InsetRect(&theTimeRect, 15, 15);
	PaintArc(&theTimeRect, (theTime.hour % 12) * 30 - 5 + (theTime.minute / 2), 10);	
}



/*********************************************************************
 * RemoveICPatch:
 *
 * Shutdown procedure that replaces the InitCursor patch.
 *
 * This is an unspeakably evil way to do this, but since our patch
 * globals are destroyed before shutdown, leading to our InitCursor
 * routine gagging and dying horribly when called, there isn't any
 * better way to do this that I know of. Blame Apple for not preserving
 * memory to the end.
 *
 * Anyways, yes, this *is* what it looks like. For shutdown purposes,
 * we're replacing InitCursor with ShowCursor... :/
 *
 *********************************************************************/
pascal void RemoveICPatch()
{
	SetTrapAddress(GetTrapAddress(_ShowCursor), _InitCursor);
}



/*********************************************************************
 * BBlkSelector:
 *
 * 'BBlk' gestalt selector that provides the address of our patch
 * globals. This is *much* cleaner than patching SizeResource().
 *
 *********************************************************************/
pascal void BBlkSelector(OSType selector, long *result)
{
	/* find our globals ...and give 'em back! */
	*result = (long) ((long) StartPatchCode - sizeof(PatchGlobals));
}



/*********************************************************************
 * SAVRSelector:
 *
 * 'SAVR' gestalt selector that tells the world when we're alive and
 * also when we're sleeping.
 *
 *********************************************************************/
pascal void SAVRSelector(OSType selector, long *result)
{
	PatchGlobalsPtr	pgPtr;
	
	/* find our globals */
	pgPtr = (PatchGlobalsPtr) ((long) StartPatchCode - sizeof(PatchGlobals));

	*result = 0;

	/* if we haven't been disabled, tell the world of our presence */
	if (pgPtr->pgMustSave == true)
		*result |= 1;				// saver enabled

	/* If screen is black, tell the world we're alive */
	if (pgPtr->pgSaverOn == true) {
		*result |= 2;				// saver asleep
		*result |= 16;				// app drawing disabled
	}
}



/*********************************************************************
 * SAVCSelector:
 *
 * The 'SAVC' gestalt selector passes back the address of our control
 * routine so that other applications can control us.
 *
 * We can be turned on and off remotely, and we can be told to put
 * the screen to sleep and wake the screen up. Neat, huh?
 *
 *********************************************************************/
pascal void SAVCSelector(OSType selector, long *result)
{
	*result = (long) SaverControl;
}



/*********************************************************************
 * SaverControl:
 *
 * Executes a command from an application or whatever.
 *
 *********************************************************************/
pascal OSErr SaverControl(short theCommand)
{
	PatchGlobalsPtr	pgPtr;
	
	/* find our globals */
	pgPtr = (PatchGlobalsPtr) ((long) StartPatchCode - sizeof(PatchGlobals));

	switch (theCommand) {
		case eSaverWakeUp:					// Make Basic Black wake up.
			if (pgPtr->pgSaverOn)
				pgPtr->pgMustWake = true;
			break;
			
		case eSaverSleep:					// Make Basic Black go to sleep.
			if (pgPtr->pgMustSave)
				pgPtr->pgMustSleep = true;
			break;
			
		case eSaverOn:						// Turn Basic Black on.
			pgPtr->pgMustSave = true;
			pgPtr->pgMustSleep = false;
			break;
			
		case eSaverOff:						// Turn Basic Black off.
			if (pgPtr->pgSaverOn)
				pgPtr->pgMustWake = true;
			pgPtr->pgMustSave = false;
			break;
	}
	
	return noErr;
}



/*********************************************************************
 * EndPatchCode:
 *
 * Dummy proc to mark the end of the code for the patches.
 * Make sure all of your patch code is between here and
 * StartPatchCode.
 *
 *********************************************************************/
void EndPatchCode()
{
}
