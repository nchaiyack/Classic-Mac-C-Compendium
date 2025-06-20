/*
	SpinLibInterns.h
	
	Internal header for the beachball cursor spinning library.
	
*/

#ifndef __H_SpinLibInterns__
#define __H_SpinLibInterns__

#include <Retrace.h>

typedef struct {
	VBLTask				vblTask;		// the task used by the system to enter VBL tasks.
	long					vblKeyTag; 	// for picking this structure out of the VBL queue.
	long					vblA5;		// a5 for the application using the library.
	CursHandle*			cursors;		// the array of cursors to display.
	short				direction;		// direction determines how value is modified (+/- 1).
	short				value;		// value determines which cursor will be displayed.
} CursorTask, *CursorTaskPtr;

OSErr SpinInit(void);
OSErr SpinStart(short direction);
OSErr SpinStop(void);
OSErr SpinCleanup(void);
Boolean SpinSpinning(void);

pascal void SpinTask(void);

#endif
