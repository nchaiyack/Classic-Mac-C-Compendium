#include "Global.h"
#include "SpinningCursor.h"

#include <Retrace.h>

/*******************************************************************************

	Structure defining the 'acur' resource. Note that this resource as defined
	on disk actually has the following structure:

		typedef struct {
			unsigned short 	numCursors;
			unsigned short 	index;
			struct {
				short ID;
				short unused;
			} cursors[1];
		} Acur, *AcurPtr, **AcurHandle;

	However, we change this structure to the following in InitCursorCtl.

*******************************************************************************/

typedef struct {
	unsigned short 	numCursors;		/* Number of cursors */
	unsigned short 	index;			/* Next cursor to show */
	CursHandle		cursors[1];		/* Actually unbounded array */
} Acur, *AcurPtr, **AcurHandle;


/*******************************************************************************

	Constants and variables

*******************************************************************************/

AcurHandle			pCursors;		/* Handle to the 'acur' resource. */
VBLTask				pCursorTask;	/* The VBL task record. */
short				pDesiredCount;	/* Value to be placed in the vblCount field
									   of our VBL task record. */

char CrsrBusy : 0x08CD;				/* Pointer to CrsrBusy low-memory global. */


/*******************************************************************************

	InitCursorCtl

	Call this routine early in your program to initialize some structures.
	After this routine reads in the 'acur' resource, it looks at the data
	inside to find out what 'CURS' resources to read. At this point, the
	'acur' resource looks like this:

		short	number of cursors
		short	index of next cursor to show
		short	resource ID of first cursor
		short	<unused>
		short	resource ID of second cursor
		short	<unused
		...
		short	resource ID of last cursor
		short	<unused>

	When InitCursorCtl reads in a 'CURS' resource, it places the handle to the
	cursor into the space that used to hold the ID of the resource. This means
	that the above structure ends up looking like the following after all
	cursors have been read in:

		short	number of cursors
		short	index of next cursor to show
		long	handle to first cursor
		long	handle to second cursor
		...
		long	handle to last cursor

	This function does _not_ detach any of the resources from the resource
	fork. Although we change the 'acur' resource, we never call
	ChangedResource on it, so the changes never get written out to disk.
	However, make sure that the 'acur' resource is not marked as purgable or
	our changes will get wiped out.

*******************************************************************************/
void InitCursorCtl(short resID)
{
	short		cursorCount;
	CursHandle	theCursor;
	CursHandle*	workPtr;

	pCursors = (AcurHandle) GetResource('acur', resID);
	DetachResource((Handle) pCursors);

	cursorCount = (**pCursors).numCursors;
	(**pCursors).numCursors *= 32;
	(**pCursors).index = 0;

	HLock((Handle) pCursors);
	workPtr = (**pCursors).cursors;
	while (cursorCount--) {
		theCursor = (CursHandle) GetResource('CURS', *(short*) workPtr);
		DetachResource((Handle) theCursor);
		*workPtr++ = theCursor;
	}
	HUnlock((Handle) pCursors);
}


/*******************************************************************************

	StartAsyncSpinning

	We call this routine to install a VBL task that calls SpinCursor at
	regular intervals, regardless of whatever else is going on. This way, we
	don�t have to worry about calling SpinCursor by hand in the middle of
	calculations or any other lengthy process.

	Before starting the VBL task, we lock down all of the cursor data that
	we�ll access in the task. Next, we we fill out all of the fields of the
	VBL task record. After all the fields are filled out, we simply
	call VInstall.

*******************************************************************************/
void	StartAsyncSpinning(short period)
{
	LockCursorData();

	pDesiredCount = period;

	pCursorTask.qType = vType;
	pCursorTask.vblAddr = (ProcPtr) MySpinner;
	pCursorTask.vblCount = pDesiredCount;
	pCursorTask.vblPhase = 0;

	(void) VInstall((QElemPtr) &pCursorTask);
}


/*******************************************************************************

	StopAsyncSpinning

	Remove the VBL task that spins the cursor and unlock all of the data it
	used.

*******************************************************************************/
void	StopAsyncSpinning()
{
	(void) VRemove((QElemPtr) &pCursorTask);
	UnlockCursorData();
	InitCursor();
}


/*******************************************************************************

	SpinCursor

	Unless you are using the asynchronous routines, you should call this
	function at regular intervals during a lengthy process. Our �spin index�
	is incremented by the amount passed in. In keeping with MPW�s
	implementation, we only change the cursor every time the index passes
	through a multiple of 32.

*******************************************************************************/
void SpinCursor(short increment)
{
	short	oldIndex;
	short	newIndex;

	oldIndex = (**pCursors).index / 32;

	(**pCursors).index += increment;
	(**pCursors).index %= (**pCursors).numCursors;	// Take care of wraparound

	newIndex = (**pCursors).index / 32;

	if (newIndex != oldIndex) {
		SetCursor(*(**pCursors).cursors[newIndex]);
	}
}


/*******************************************************************************

	LockCursorData

	Lock all of the handles used to define our spinning cursor set. These
	handles consist of our 'acur' resource, which in turn is essentially an
	array of handles to our cursors. Each of these is moved high in memory and
	locked down using HLockHi (this call is new to MPW 3.2 and THINK C 5.0,
	but it will work on any Macintosh).

*******************************************************************************/
void LockCursorData()
{
	short	cursorCount;
	CursHandle*	workPtr;

	cursorCount = (**pCursors).numCursors / 32;

	HLockHi((Handle) pCursors);
	workPtr = (**pCursors).cursors;
	while (cursorCount--) {
		HLockHi((Handle) *workPtr++);
	}
}


/*******************************************************************************

	UnlockCursorData

	Unlock the data locked by LockCursorData. We call this after turning off
	our asynchronous cursor spinning routines so that we don�t keep the data
	locked in memory.

*******************************************************************************/
void UnlockCursorData()
{
	short		cursorCount;
	CursHandle*	workPtr;

	cursorCount = (**pCursors).numCursors / 32;

	workPtr = (**pCursors).cursors;
	while (cursorCount--) {
		HUnlock((Handle) *workPtr++);
	}
	HUnlock((Handle) pCursors);
}


/*******************************************************************************

	MySpinner

	VBL task procedure. This function is called at regular intervals to spin
	the cursor. It first checks the low-memory global CrsrBusy to make sure
	that the cursor is available to be changed. If so, we point A5 at our
	application�s globals, call SpinCursor to change to the next cursor,
	reprime our VBL task so it gets called again, and restore A5 to its
	original value.

	Note that the way we look at the low-memory global CrsrBusy is very THINK
	C-ish. THINK provides access to low-memory globals using a non-standard
	syntax. At the start of this file, we declared CrsrBusy to be:

			char CrsrBusy : 0x08CD;

	We could then use it as shown in MySpinner. Under MPW C, we would have to
	do the following:

			#define CrsrBusy 0x08CD
			if ( *(char *) CrsrBusy == 0) { ... };

*******************************************************************************/
void	MySpinner()
{
	VBLTask*			myTaskPtr;

	asm {
		move.l	A0, myTaskPtr
	}

	if (CrsrBusy == 0) {
		SetUpA4();
		SpinCursor(32);
		myTaskPtr->vblCount = pDesiredCount;
		RestoreA4();
	}
}

