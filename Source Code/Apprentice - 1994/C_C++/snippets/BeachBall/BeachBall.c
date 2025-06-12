/******************************************************************

BeachBall.c
Routines for asynchronous spinning beach-ball cursor

Based on & expanded from a code sample in Scott Knaster's excellent
book, "Macintosh Programming Secrets", full of tons of truly
useful & informative stuff.


**************

©1993 Peter Vanags, Electronic Ink; portions ©1992 Scott Knaster & Keith Rollin
All rights reserved.

Distributed by:

Electronic Ink
72 Caselli Avenue
San Francisco, CA 94114

AppleLink: INK.LINK
CompuServe: 70401,3202
Internet: ink.link@applelink.apple.com

***************

ABOUT THE BEACH BALL ROUTINES

These routines can be used either synchronously (without the VBL task)
or asynchronously. To use them in their intended manner (async):

	StartBeachBall (bbAcurID,TRUE,0); //starts the beach ball spinning
	
	...complete whatever processes you need to...
	
	ReverseBeachBall (); //reverses the spin direction
	
	...complete some more processes....
	
	StopBeachBall(); //stops the beach ball spinning
	

Additionally, you can set the third argument to StartBeachBall to
a non-zero value (say, 300 for 5 seconds' timeOut) and the beach
ball will stop spinning after that many ticks have elapsed:

	#define timeOut 300	//set a standard timeout of 300 ticks (5 secs)
	
	StartBeachBall (bbAcurID,TRUE,timeOut); //starts the beach ball spinning
	
	...do something...
	
	KeepSpinning (timeOut); //reset the timeOut counter
	
	...do some more stuff...
	
	StopBeachBall(); //stops the beach ball spinning
	

This is to prevent the beach ball from spinning merrily away as your
program hangs.



To use the BeachBall routines synchronously, call them this way:

	StartBeachBall (bbAcurID,FALSE,0); //sets up the beach ball structures
	
	for (c=0;c<limit;c++) { 	//run some type of loop

		SpinBeachBall();		//call this as often as you like
		
		...do something...
	
	}
	
	StopBeachBall (); disposes of the beach ball structures
	
	
******************************************************************/


#include <retrace.h>
#include <traps.h>
#include "BeachBall.h"

		
//timing values for beach ball
#define bbArrowTime		10		//ticks before we switch to a watch
#define bbWatchTime		90		//ticks before the watch becomes a beachball
#define bbSpinTime 		8		//ticks between beachball spins
#define bbRetryTime		5		//time to wait before re-trying (when CrsrBusy is true)
#define bbSuspendTime	50		//time between re-checks while spinning is suspended
#define	vblTaskKey		666		//a "key" value to check whether SpinBeachBall is VBL or not


typedef struct {
	unsigned short	numCursors;
	unsigned short	index;
	CursHandle		cursors[];
} Acur,*AcurPtr,**AcurHandle;

typedef struct {
	VBLTask			theTask;			//VBL task record, empty if we're not using a VBL task
	short			key;				//a "key" value to detect whether SpinBeachBall is a VBL call or not
	long			vblCountDown;		//tickCount at which we'll stop spinning
	long			a5;					//the a5 value of the current application
	short			cursorPhase;		//phase of the cursor: suspended, arrow, watch or beachball
	short			spinIncrement;		//value is either 1 or -1, to determine which way the ball spins
	AcurHandle		theAcur;			//handle to the animated cursor
	CursHandle		watchHandle;		//handle to the watch cursor, since we can't move memory in VBL
	GrafPtr			startWindow;		//the front window when we started
} BeachBallTask,*BeachBallTaskPtr;

//cursor phases
enum {suspendedPhase,arrowPhase,watchPhase,bbPhase};

//low-memory globals
char CrsrBusy	: 0x08cd;
char MBState	: 0x0172;


//some 'acur' utility routines...
static AcurHandle InitAcur (short resID);
static void LockAcur (AcurHandle theAcur);
static void UnlockAcur (AcurHandle theAcur);
static void DisposeAcur (AcurHandle theAcur);

//..and two global variables.
static BeachBallTaskPtr	bbTask = nil;
static long	lastTicks = 0;


/******************************************************************

StartBeachBall sets things rolling. Call this routine
with vblInstall set to TRUE at the start of a long
operation to have automatic, asynchronous user feedback
about the operation.

if vblInstall is FALSE, you'll nead to call SpinBeachBall()
repeatedly from your routines to keep the ball rolling.

vblTimeOut is measured in ticks. If vblInstall is TRUE, and
vblTimeOut is non-zero, KeepSpinning() must be called before the
number of ticks in vblTimeOut have elapsed.

If KeepSpinning() is not called, the cursor stops spinning
(freezes in whatever state it's in). This is so the cursor doesn't
keep spinning merrily away when your program hangs. If you want
to be sloppy (like me) or your programs never hang (not like me),
set vblTimeOut to 0, and don't worry about making calls to
KeepSpinning(). 
 
The beachball routine automatically adapts to the duration of your
operations. Very short operations (<10 ticks) will not switch cursors.
Operations up to about 2 seconds will get a wristwatch.
After about 2 seconds, the cursor will change to a spinning beach ball.
So to be safe, bracket ANY operation that might take a long time with
calls to StartBeachBall and StopBeachBall. Remember that an
instantaneous operation on the Quadra you program with
might take a few seconds or longer on a Mac Classic!

******************************************************************/

OSErr StartBeachBall (short acurID, Boolean vblInstall, long vblTimeOut) {

	if (bbTask = (BeachBallTaskPtr) NewPtrClear (sizeof(BeachBallTask))) {

		//initialize the generic fields
		
		bbTask->key = vblTaskKey;
		bbTask->cursorPhase = arrowPhase;
		bbTask->spinIncrement = 1;
		bbTask->watchHandle = GetCursor(watchCursor);
		bbTask->startWindow = FrontWindow();
		
		if (vblTimeOut)
			bbTask->vblCountDown = Ticks + vblTimeOut;
		else
			bbTask->vblCountDown = 0;

		if (!(bbTask->theAcur = InitAcur (acurID)))  {
			DisposePtr (bbTask);
			bbTask = nil;
			return ResError();
		}

		bbTask->theTask.qType = vType;
		bbTask->theTask.vblAddr = (ProcPtr) SpinBeachBall;
		bbTask->theTask.vblCount = bbArrowTime;
		bbTask->theTask.vblPhase = 0;
		bbTask->a5 = (long) CurrentA5;

		if (vblInstall)
			VInstall ((QElemPtr) bbTask);
		else	
			lastTicks = Ticks; //set up a global so we can manually decrement
	}
	else
		return MemError();

	return noErr;
}


/******************************************************************

KeepSpinning() keeps the beachball spinning if vblTimeOut is non-zero.

******************************************************************/

void KeepSpinning (long vblTimeOut) {
	if (bbTask->vblCountDown)
		bbTask->vblCountDown = Ticks + vblTimeOut;
}


/******************************************************************

ReverseBeachBall() spins the beach ball the other way.

******************************************************************/

void ReverseBeachBall (void) {
	bbTask->spinIncrement *= -1;
}


/******************************************************************

SuspendBeachBall and ResumeBeachBall are macros to optimize the 
VBL task's performance

******************************************************************/

#define SUSPEND_BEACH_BALL(theBBTask)  \
	if (theBBTask && theBBTask->cursorPhase) { \
		theBBTask->cursorPhase = suspendedPhase; \
		theBBTask->theTask.vblCount = bbSuspendTime; \
		SetCursor (&arrow); \
	} \


#define RESUME_BEACH_BALL(theBBTask)  \
	if (theBBTask && !theBBTask->cursorPhase) { \
		theBBTask->cursorPhase = bbPhase; \
		theBBTask->theTask.vblCount = bbSpinTime; \
	} \



/******************************************************************

SpinBeachBall is meant to be called as a VBL task, so we
have to make sure to set the a5 world properly.

But SpinBeachBall can also be called from your own routines
repeatedly, if you decide not to use it as a VBL task. It
should work fine in either case.

While the beachball is spinning, we need to keep on the lookout
for alert dialogs and mouseclicks. In both instances, we need
to suspend the beach ball spinning until the dialog is
dismissed or the mouse button is released. These actions mainly
apply to the VBL usage.

If you don't use the VBL version, be sure to set the cursor to an
arrow when you put up alert dialogs. You don't have to worry about this
for the VBL version.

SpinBeachBall checks the status of the mouse button and the front
window. If the mouse button is down, or the front window
is not the same window as it was at the time StartBeachBall
was called (and that window is a dialog-type), beach ball spinning
will be suspended.

******************************************************************/

void SpinBeachBall (void) {
	long				oldA5;
	BeachBallTaskPtr	theBBTask;
	GrafPtr				w;
	Boolean				isVBL;
	
	//this assembly code puts a0 (which points to our VBLTask record)
	//into a local variable, so we can access our beach ball data
	asm {
		move.l A0,theBBTask
	}
	
	//check the "key" value to see if a0 was pointing to
	//our VBL task record, or just a random address
	
	if (isVBL = (theBBTask && (theBBTask->key == vblTaskKey)))
		oldA5 = SetA5 (theBBTask->a5);
	else {
		//not a VBL Task
		theBBTask=bbTask;
		//Manually decrement & check the VBL count
		if ((theBBTask->theTask.vblCount -= (Ticks - lastTicks)) > 0) {
			lastTicks = Ticks;
			goto exit;
		}
	}

	if (!CrsrBusy) { //make sure we can change the cursor now
	
		//check if the timeOut has elapsed
		if (theBBTask->vblCountDown && (Ticks > theBBTask->vblCountDown)) {
			theBBTask->theTask.vblCount = bbSuspendTime;
			goto exit;
		}
			
		//check if we should suspend beach ball spinning
		if (((MBState == 0) && theBBTask->cursorPhase) ||
			(((w=FrontWindow()) != theBBTask->startWindow) &&
			(w && (((WindowPeek)w)->windowKind == dialogKind)))) {
			SUSPEND_BEACH_BALL(theBBTask);
		}
		else if (MBState && !theBBTask->cursorPhase)
			RESUME_BEACH_BALL(theBBTask);

		if (theBBTask->cursorPhase) {
			switch (theBBTask->cursorPhase) {
				case arrowPhase:
					SetCursor(*theBBTask->watchHandle);
					theBBTask->theTask.vblCount = bbWatchTime;
					theBBTask->cursorPhase = watchPhase;
					break;
				case watchPhase:
					//set the phase & fall thru to the default action
					theBBTask->cursorPhase = bbPhase;
				default:
					(**(theBBTask->theAcur)).index += theBBTask->spinIncrement;
					(**(theBBTask->theAcur)).index %= (**(theBBTask->theAcur)).numCursors;
					//the next two lines turn negative increments around, but leave positive increments alone
					(**(theBBTask->theAcur)).index += (**(theBBTask->theAcur)).numCursors;
					(**(theBBTask->theAcur)).index %= (**(theBBTask->theAcur)).numCursors;
					SetCursor (*(**(theBBTask->theAcur)).cursors[(**(theBBTask->theAcur)).index]);
					theBBTask->theTask.vblCount = bbSpinTime;
					break;
			}
		}
		else 
			theBBTask->theTask.vblCount = bbSuspendTime;

	}
	else   //the cursor can't be changed now, so try again in a little while...
		theBBTask->theTask.vblCount = bbRetryTime;

exit:
	if (isVBL)
		SetA5 (oldA5);

}


/******************************************************************

StopBeachBall stops the spinning and disposes of all the
beachball structures

******************************************************************/

void StopBeachBall (void) {
	if (bbTask) {
		VRemove ((QElemPtr)bbTask);
		DisposeAcur (bbTask->theAcur);
		DisposePtr (bbTask);
		bbTask = nil;
		SetCursor(&arrow);
	}
}



/******************************************************************

Animated cursor ('acur') utilities
These utilities deal with loading an 'acur' and its related
'CURS' resources, locking & unlocking them in memory,
and proper disposal

******************************************************************/

static AcurHandle InitAcur (short resID) {
	short 		cursorCount;
	CursHandle	*workPtr;
	AcurHandle	theAcur = (AcurHandle) GetResource('acur',resID);
	
	if (theAcur) {
		DetachResource (theAcur);
		cursorCount = (*theAcur)->numCursors;
		(*theAcur)->index = 0;
		
		HLock(theAcur);
		workPtr = (*theAcur)->cursors;
		while (cursorCount--)
			if (!(*workPtr++ = (CursHandle) GetResource ('CURS',*(short*)workPtr))) {
				HUnlock (theAcur);
				DisposeHandle (theAcur);
				return nil;
			}
		HUnlock (theAcur);
	}
	return theAcur;
}

static void LockAcur (AcurHandle theAcur) {
	short		cursorCount;
	CursHandle	*workPtr;
	
	cursorCount = (*theAcur)->numCursors;
	
	HLockHi (theAcur);
	workPtr = (*theAcur)->cursors;
	while (cursorCount--)
		HLockHi(*workPtr++);
}

static void UnlockAcur (AcurHandle theAcur) {
	short		cursorCount;
	CursHandle	*workPtr;
	
	cursorCount = (*theAcur)->numCursors;
	
	workPtr = (*theAcur)->cursors;
	while (cursorCount--)
		HUnlock(*workPtr++);
	HUnlock (theAcur);
}

static void DisposeAcur (AcurHandle theAcur) {
	short		cursorCount;
	CursHandle	*workPtr;
	
	cursorCount = (*theAcur)->numCursors;
	
	HLock(theAcur);
	workPtr = (*theAcur)->cursors;
	while (cursorCount--)
		ReleaseResource(*workPtr++);
	HUnlock(theAcur);
	DisposeHandle (theAcur);
}
