/*
 * CAnimCursor.c
 * Easy, flexible handling of animated cursors.  No TCL required.
 * Version 1.0b4, 14 September 1992
 *
 * Jon Pugh's Pascal "Watch" unit, which he posted to the tcl-talk
 * discussion group on 92 Apr 27, was the jumping-off point for
 * this class.  Thanks, Jon.
 * Debugging help was provided by Daryl Spitzer.  Thanks, Daryl.
 * Written by Jamie McCarthy.
 * Internet: k044477@kzoo.edu				AppleLink: j.mccarthy
 * Telephone:  800-421-4157 (9:00-5:00 Eastern time)
 * I'm releasing this code with the hope that someone will get something
 * out of it.  Feedback of any sort, even just letting me know that you're
 * using it, is greatly appreciated!
 *
 * CAnimCursor's source code is in the public domain.  If you make changes,
 * please do me the courtesy of letting me know before you redistribute it.
 *
 * HOW TO USE IT
 *
 * Add an 'acur' resource to your program, and pass its ID to
 * gAnimCursor->IAnimCursor().  Then call startAnimating() when you want
 * to start animating--for example, when you begin a lengthy operation--
 * and call stopAnimating() when you're done with it.  Call Dispose()
 * at the appropriate time, of course.
 *
 * You may change which cursors you're using, even during animation, by
 * calling useAnimCursorID().  You may change the animation speed with
 * setTicksBetweenCursors().  There is a delay between calling
 * startAnimating() and the appearance of the first cursor, which you
 * may change with setInitialDelay().  And you can turn the interrupt-
 * driven animation on or off with setUsingInterrupts().
 *
 * To release the memory used by the cursors without disposing of the
 * entire object, call stopUsingAnimCursor().  Thereafter, you must call
 * useAnimCursorID() before you start animating again.
 *
 * Any of these methods may be called at any time, and it will do what
 * you'd expect;  for example, if you call startAnimating() twice, the
 * second call will simply have no effect.
 *
 * THE THINK CLASS LIBRARY AND THE TWO MODES
 *
 * If you're using the TCL, there are three additional classes that you
 * probably want to patch into your project.  To understand why, you should
 * be aware that CSwitchboard and the CView hierarchy reset the cursor
 * every time through the event loop.  For further information, read the
 * source to CSwitchboard::ProcessEvent() and CDesktop::DispatchCursor().
 * There are two ways of dealing with this, called "modes."
 *
 * The first, "interrupted," is to accept it, and turn off animation each
 * time through ProcessEvent().  This is best if your animated cursor
 * doesn't resemble the arrow, and if you spend a long time away from the
 * event loop, because then it indicates "I'm busy, don't try clicking
 * or anything."  If this is all you want, add CAcurSwitchboard and
 * CAcurError to your project and follow the instructions given in
 * the source files.
 *
 * The second mode, "continuous," is to fight it, and keep the animation
 * going through the event loop.  This is best if your cursor is similar
 * to the arrow, and if you go through the event loop relatively quickly,
 * like once every few seconds or faster.  To use this mode, you must
 * curse Symantec up and down for not including multiple inheritance,
 * and for not writing the cursor-dispatching methods with a good
 * solid OOP paradigm.  After you get that out of your system, you must
 * call gAnimCursor->setMode(kCACModeContinuous), add CAcurSwitchboard,
 * CAcurError, and CAcurDesktop to your project, and follow the
 * instructions in those source files.
 *
 * You may switch back and forth between the two modes at any time, assuming
 * you're using all three "CAcur" subclasses in your project.
 *
 * By the way, remember not to call ForgetObject() on gAnimCursor.  That
 * function assumes it's being passed a (CObject*), which, despite
 * appearances, gAnimCursor is not.  You'll get a bus error or somesuch.
 *
 * COLOR CURSORS
 *
 * The action routine, animateCursor(), is constantly called by the interrupt
 * routine, but may also be called from "normal" program code.  In fact,
 * it _must_ be called from "normal" code if you want color cursors,
 * since SetCCursor may not be called from an interrupt.
 *
 * If you have color cursors, but they're showing as B&W part or all of the
 * time, then the interrupt routine, not you, is calling animateCursor()
 * first.  You may either (1) give your non-interrupt code a better chance
 * of being called, by increasing the slop value with setSlopTicks(),
 * or (2) call animateCursor() more often, or (3) turn interrupts off
 * entirely with setUsingInterrupts(FALSE), or any combination thereof.
 * If you have the ticks per cursor cranked way fast, you won't be able to
 * get the color displayed no matter what you do.
 *
 * Because I call Count1Resources('crsr') (instead of CountResources()) to
 * determine whether there are color cursors that can be shown, the file with
 * the cursors must be the current resource file when IAnimCursor() is
 * called.
 *
 * OH AND BY THE WAY
 *
 * In case you were wondering, there _is_ protection against animateCursor()
 * being simultaneously called from the "normal" and the interrupt level.
 * This is the very first time I've written "dining philosophers" code--
 * whoopie.
 *
 * VERSION HISTORY
 *
 * Changes from 1.0b3 to 1.0b4:
 * Added itsAcurID, to prevent unnecessary reloading of resources.
 * Removed _nooptimize and put interrupt-level global access in separate function.
 * Modified a few comments.
 * Put in the resource-file-tracking code.
 *
 * Changes from 1.0b1 to 1.0b3:
 * Daryl Spitzer (Daryl_Spitzer@mindlink.bc.ca) fixed the A5 bug for me.
 * Made nextCursor() a method.
 * Made installVBLTask() and removeVBLTask() methods.
 * Added the continuous mode, and CAcurDesktop.
 * Changed the "set" methods to use ticks.
 * Got the 'acur' with a Get1Resource() instead of GetResource().
 * Fixed a few miscellaneous bugs.
 * Changed a few variables' names around.
 * Added some comments.
 * Made a few code touch-ups.
 *
 *
 * It's laughable that someone wants Apple to animate their cursor for them.
 * I guess the Magic Toolbox has made us spoiled children.  If you need an
 * attitude adjustment, try programming an MS-DOS boat anchor.  That'll
 * animate your cursor!
 *   - "The Midnight Hacker," Letter to MacTutor, May 1988
 *
 */



/********************************/

#include "CAnimCursor.h"

/********************************/

#include <GestaltEqu.h>
#include <QuickDraw.h>
#include <Retrace.h>

/********************************/

	/*
	 * Some people use SysEqu.h, others use LoMem.h.  The two cannot both
	 * be used (see "Mac #includes.h").  The former has CrsrBusy defined,
	 * while the latter does not.  To avoid inclusion confusion, I'll
	 * just do the simplest thing:  hard-code it!
	 */
#define cursorIsBusy() ( (* (unsigned char*) 0x08CD) != 0x00)

/********************************/

static pascal void doAnimateCursor(void);

/********************************/

extern Boolean gInBackground;

CAnimCursor *gAnimCursor = NULL;

/********************************/



void CAnimCursor::IAnimCursor(short rsrcID)
{
	MoveHHi((Handle) this);
	
	setTopResFile(CurResFile());
	
	determineTryToUseColor();
	
	isInAnInterrupt = FALSE;
	
	isAnimating = FALSE;
	vblTaskIsInstalled = FALSE;
	itsAcurID = MININT;
	itsAcurHndl = NULL;
	itsLastTick = 0;
	
	setTicksBetweenCursors(kCACDefaultTicksBetweenCursors);
	setInitialDelayTicks(kCACDefaultInitialDelayTicks);
	setUsingInterrupts(TRUE);
	setSlopTicks(kCACDefaultColorInterruptSlop);
	setMode(kCACModeInterrupted);
	
	useAnimCursorID(rsrcID);
}



void CAnimCursor::Dispose(void)
{
	stopUsingAnimCursor();
	delete(this);
}



void CAnimCursor::useAnimCursorID(short rsrcID)
{
	OSErr theOSErr;
	short oldState;
	short oldResFile;
	Boolean wasAnimating;
	
	if (itsAcurID == rsrcID) return;
	
	oldResFile = CurResFile();
	useTopResFile();
	
	wasAnimating = getIsAnimating();
	
	stopUsingAnimCursor();
	
	itsAcurHndl = (acurHndl) Get1Resource('acur', rsrcID);
	if (itsAcurHndl == NULL || ResError()) return;
	
	DetachResource((Handle) itsAcurHndl);
	MoveHHi((Handle) itsAcurHndl);
	HLock((Handle) itsAcurHndl);
	
	loadIndividualCursors();
	
	itsAcurID = rsrcID;
	
	if (wasAnimating) {
		startAnimating();
	}
	
	UseResFile(oldResFile);
}



void CAnimCursor::stopUsingAnimCursor(void)
{
	if (itsAcurHndl == NULL) return;
	
	stopAnimating();
	
	disposeIndividualCursors();
	
		/* Dispose of the 'acur' handle. */
	DisposHandle((Handle) itsAcurHndl);
	itsAcurHndl = NULL;
}



void CAnimCursor::startAnimating(void)
{
	OSErr theOSErr;
	
	if (itsAcurHndl == NULL) return;
	if (getIsAnimating()) return;
	
	itsInitialTick = TickCount();
	
	theOSErr = noErr;
	if (usingInterrupts) {
		theOSErr = installVBLTask();
	}
	
	if (theOSErr == noErr) {
		isAnimating = TRUE;
	}
}



void CAnimCursor::stopAnimating(void)
{
	OSErr theOSErr;
	
	theOSErr = removeVBLTask();
	
	if (!getIsAnimating()) return;
	isAnimating = FALSE;
	itsInitialTick = 0;
}



Boolean CAnimCursor::getIsAnimating(void)
{
	return isAnimating;
}



void CAnimCursor::animateCursor(void)
{
	register long cTickCount;
	
	if (isInAnimateCursor) return;
	if (!getIsAnimating()) return;
	if (gInBackground) return;
	
		/* "Lock" the routine, so that the interrupt can't re-enter it. */
	isInAnimateCursor = TRUE;
	
		/*
		 * Put the current TickCount into a variable.  This is for speed--
		 * calling TickCount() takes time--but also because strange things
		 * might happen if the Ticks were incremented in the middle of this
		 * routine...
		 */
	cTickCount = TickCount();
	
	if ((itsLastTick+itsTicksBetweenCursorsForInterrupt <= cTickCount)
		|| (!isInAnInterrupt &&
			itsLastTick+itsTicksBetweenCursorsForNormalCode <= cTickCount)
		) {
		
		if (cursorIsBusy()) {
			
				/* Sigh.  Just my luck.  Let's try again next time around. */
			itsVBLTask.vblCount = 1;
			
		} else {
			
			register acurPtr theAcurPtr;
			
			theAcurPtr = *itsAcurHndl;
			
			nextCursor();
			
			if (usingColorCursors) {
				
				if (isInAnInterrupt) {
						/*
						 * We can't call SetCCursor at interrupt time, so fake it:
						 * draw the B&W cursor that's built into the color one.
						 */
					SetCursor( (CursPtr) &
						(*((CCrsrHandle) theAcurPtr->cursor[theAcurPtr->cCursor]))
							->crsr1Data);
				} else {
						/*
						 * We're OK; animateCursor() was not called from an
						 * interrupt, so we can use the real color cursor.
						 */
					SetCCursor( (CCrsrHandle) theAcurPtr->cursor[theAcurPtr->cCursor] );
				}
				
			} else {
				
				SetCursor( * theAcurPtr->cursor[theAcurPtr->cCursor] );
				
			}
			
			itsLastTick = cTickCount;
			
			itsVBLTask.vblCount = itsTicksBetweenCursorsForInterrupt;
			
		}
		
	}
	
		/* We're done, so the interrupt routine is again free to call. */
	isInAnimateCursor = FALSE;
}



void CAnimCursor::setTicksBetweenCursors(short newTicksBetweenCursors)
{
		// A quick sanity check.  No fewer than 0 ticks, no more than 20 seconds.
	ASSERT(newTicksBetweenCursors >= 0 && newTicksBetweenCursors <= 1200);
	
	if (newTicksBetweenCursors != itsTicksBetweenCursors) {
		itsTicksBetweenCursors = newTicksBetweenCursors;
		setSlopTicks(getSlopTicks());
	}
}



void CAnimCursor::setInitialDelayTicks(short newInitialDelayTicks)
{
		// A quick sanity check.  No fewer than 0 ticks, no more than one minute.
	ASSERT(newInitialDelayTicks >= 0 && newInitialDelayTicks <= 3600);
	
	itsInitialDelayTicks = newInitialDelayTicks;
}



void CAnimCursor::setUsingInterrupts(Boolean newUsingInterrupts)
{
	newUsingInterrupts = (newUsingInterrupts != FALSE);
	
	if (usingInterrupts == newUsingInterrupts) return;
	
	if (usingInterrupts && getIsAnimating()) {
		OSErr theOSErr;
		theOSErr = removeVBLTask();
	}
	
	usingInterrupts = newUsingInterrupts;
	
	if (usingInterrupts && getIsAnimating()) {
		OSErr theOSErr;
		theOSErr = installVBLTask();
	}
}



Boolean CAnimCursor::getUsingInterrupts(void)
{
	return usingInterrupts;
}



void CAnimCursor::setSlopTicks(short newSlopTicks)
{
	ASSERT(newSlopTicks >= 0);
	itsTicksBetweenCursorsForNormalCode = itsTicksBetweenCursors - (newSlopTicks+1)/3;
	if (itsTicksBetweenCursorsForNormalCode < 1) {
		itsTicksBetweenCursorsForNormalCode = 1;
	}
	if (itsTicksBetweenCursorsForNormalCode < (itsTicksBetweenCursors*2)/3) {
		itsTicksBetweenCursorsForNormalCode = (itsTicksBetweenCursors*2)/3;
	}
	itsTicksBetweenCursorsForInterrupt =
		itsTicksBetweenCursorsForNormalCode + newSlopTicks;
}



short CAnimCursor::getSlopTicks(void)
{
	return itsTicksBetweenCursorsForInterrupt - itsTicksBetweenCursorsForNormalCode;
}



void CAnimCursor::setMode(short newMode)
{
	ASSERT(newMode >= kCACModeInterrupted && newMode <= kCACModeContinuous);
	itsMode = newMode;
}



short CAnimCursor::getMode(void)
{
	return itsMode;
}


void CAnimCursor::setTopResFile(short newTopResFile)
{
	itsTopResFile = newTopResFile;
}



void CAnimCursor::useTopResFile(void)
{
	UseResFile(itsTopResFile);
}



/********************************/



void CAnimCursor::determineTryToUseColor(void)
{
	long theQDVersion;
	OSErr theOSErr;
	
	theOSErr = Gestalt(gestaltQuickdrawVersion, &theQDVersion);
	if (theOSErr == noErr) {
		tryToUseColor = (theQDVersion >= gestalt8BitQD);
	} else {
		SysEnvRec theWorld;
		theOSErr = SysEnvirons(2, &theWorld);
		if (theOSErr == noErr) {
			tryToUseColor = theWorld.hasColorQD;
		} else {
			tryToUseColor = FALSE;
		}
	}
}



void CAnimCursor::loadIndividualCursors(void)
{
	register acurPtr theAcurPtr;
	short nColorCursors;
	short cCursor;
	short oldState;
	short oldResFile;
	
	if (itsAcurHndl == NULL) return;
	
		/*
		 * A quick sanity check.
		 * If the handle is too short to be a real 'acur', bail out.
		 */
	ASSERT(GetHandleSize((Handle) itsAcurHndl) >= sizeof(acurStruct));
	
	oldResFile = CurResFile();
	useTopResFile();
	
	oldState = HGetState((Handle) itsAcurHndl);
	HLock((Handle) itsAcurHndl);
	theAcurPtr = *itsAcurHndl;
	
	if (tryToUseColor) {
		nColorCursors = Count1Resources('crsr');
		usingColorCursors = (nColorCursors > 0);
	}
	
	for (cCursor = 0;  cCursor < theAcurPtr->nCursors;  ++cCursor) {
		
		short theID;
		
			/* Determine the ID. */
		theID = * (short*) &(theAcurPtr->cursor[cCursor]);
		
			/* Load in the cursor from disk. */
		if (usingColorCursors) {
			theAcurPtr->cursor[cCursor] = (CursHandle) GetCCursor(theID);
			DetachResource( (Handle) theAcurPtr->cursor[cCursor] );
		} else {
			CursHandle theCursHndl;
			theCursHndl = GetCursor(theID);
			
			if (theCursHndl != NULL) {
				
				DetachResource( (Handle) theCursHndl );
				
			} else {
				
					/*
					 * If there wasn't a (B&W) 'CURS', try to fake it with 
					 * a (color) 'crsr'.  I'm still waiting for that "true stud"
					 * certificate, Jon...
					 */
				CCrsrHandle theCCrsrHndl;
				
				theCCrsrHndl = GetCCursor(theID);
				if (theCCrsrHndl != NULL) {
					
						/*
						 * Put this stuff on the stack, to avoid hassle with
						 * memory allocation.  (It's OK, it's only about 70 bytes.)
						 */
					Bits16 theData, theMask;
					Point theHotSpot;
					
					BlockMove(&(**theCCrsrHndl).crsr1Data, &theData, sizeof(Bits16));
					BlockMove(&(**theCCrsrHndl).crsrMask, &theMask, sizeof(Bits16));
					theHotSpot = (**theCCrsrHndl).crsrHotSpot;
					DisposeCCursor(theCCrsrHndl);
					
					theCursHndl = (CursHandle) NewHandle(sizeof(Cursor));
					if (theCursHndl != NULL) {		// only a paranoid would check this
						BlockMove(&theData, &(**theCursHndl).data, sizeof(Bits16));
						BlockMove(&theMask, &(**theCursHndl).mask, sizeof(Bits16));
						(**theCursHndl).hotSpot = theHotSpot;
					}
					
				}
				
			}
			
			theAcurPtr->cursor[cCursor] = theCursHndl;
		}
		
			/* If an error was found, just continue. */
		if (theAcurPtr->cursor[cCursor] != NULL && ResError() == noErr) {
			MoveHHi((Handle) theAcurPtr->cursor[cCursor]);
			HLock((Handle) theAcurPtr->cursor[cCursor]);
		}
		
	}
	
		/* Start with the first cursor. */
	theAcurPtr->cCursor = 0;
	
	HSetState((Handle) itsAcurHndl, oldState);
	
	UseResFile(oldResFile);
}



void CAnimCursor::disposeIndividualCursors(void)
{
	register acurPtr theAcurPtr;
	register short cCursor;
	
	if (itsAcurHndl == NULL) return;
	
	HLock((Handle) itsAcurHndl);
	theAcurPtr = *itsAcurHndl;
	
	for (cCursor = 0;  cCursor < theAcurPtr->nCursors;  ++cCursor) {
		
			/* Dispose of an individual cursor. */
		if (usingColorCursors) {
			DisposeCCursor((CCrsrHandle) theAcurPtr->cursor[cCursor]);
		} else {
			HUnlock((Handle) theAcurPtr->cursor[cCursor]);
			DisposHandle((Handle) theAcurPtr->cursor[cCursor]);
		}
		
	}
	
	HUnlock((Handle) itsAcurHndl);
}



OSErr CAnimCursor::installVBLTask(void)
{
	OSErr theOSErr;
	unsigned long theInitialVBLCount;
	
	if (vblTaskIsInstalled) return noErr;
	
	MoveHHi((Handle) this);
	HLock((Handle) this);
	
	itsA5 = SetCurrentA5();
	itsVBLTask.qType = vType;
	itsVBLTask.vblAddr = (ProcPtr) StripAddress(doAnimateCursor);
	theInitialVBLCount = itsInitialTick + itsInitialDelayTicks - TickCount();
	if (theInitialVBLCount < 1) theInitialVBLCount = 1;
	itsVBLTask.vblCount = theInitialVBLCount;
	itsVBLTask.vblPhase = 0;
	theOSErr = VInstall( (QElemPtr) StripAddress(&itsVBLTask) );
	
	if (theOSErr != noErr) {
		HUnlock((Handle) this);
	} else {
		vblTaskIsInstalled = TRUE;
	}
	
	return theOSErr;
}



OSErr CAnimCursor::removeVBLTask(void)
{
	OSErr theOSErr;
	
	if (!vblTaskIsInstalled) return noErr;
	
	theOSErr = VRemove( (QElemPtr) StripAddress(&itsVBLTask) );
	
	if (theOSErr == noErr) {
		HUnlock((Handle) this);
		vblTaskIsInstalled = FALSE;
	}
	
	return theOSErr;
}



void CAnimCursor::nextCursor(void)
{
	short cCursor;
	register acurPtr theAcurPtr;
	
	theAcurPtr = *itsAcurHndl;
	
	cCursor = theAcurPtr->cCursor;
	++cCursor;
	if (cCursor >= theAcurPtr->nCursors) {
		cCursor = 0;
	}
	theAcurPtr->cCursor = cCursor;
}



	static long getVBLRec(void) = 0x2008;		// MOVE.L A0,D0
	static void doActualAnimateCursor(void);
	
static pascal void doAnimateCursor(void)
{
		/* See Tech Notes #180 and #208. */
	long myVBLRecAddress;
	long oldA5;
	
		/*
		 * The address of the queue entry is stored in A0.
		 * The application's A5 is stored one longword before 
		 * the VBL queue entry.
		 */
	myVBLRecAddress = getVBLRec();
	oldA5 = SetA5( * ( (long*)myVBLRecAddress - 1 ) );
	
		/*
		 * OK, globals are valid now, so we can get at the object.  But
		 * the compiler may have decided to resolve any global references
		 * _before_ A5 was set up.  To forestall that, all such references
		 * are in doActualAnimateCursor().  See Tech Note #180.
		 */
	doActualAnimateCursor();
	
		/* Restore the A5 to what it was before we did our thing. */
	SetA5(oldA5);
}



static void doActualAnimateCursor(void)
{
	if (!gAnimCursor->isInAnInterrupt) {
		gAnimCursor->isInAnInterrupt = TRUE;
		gAnimCursor->animateCursor();
		gAnimCursor->isInAnInterrupt = FALSE;
	}
}