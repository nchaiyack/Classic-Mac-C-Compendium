/***
 * SpinCursor.c - code to diplay a spin cursor
 *
 * This code should be used like this
 * ----------------------------------
 *
 * #include "SpinCursor.h"
 *
 *		StartSpinning(id);		must call to start
 *		...
 *
 *		StopSpinning();			must call it when done
 *
 ***/

// various fixes in the 68K part;
// the PowerPC implementation is left to the reader;
// under PowerPC you only see the system watch cursor
// [Fabrizio Oddone]

#ifndef __RETRACE__
#include <Retrace.h>
#endif

#include "SpinCursor.h"

#define kInterval 30
#define kInitialDelay 120

typedef struct {
	VBLTask			vblTask;
	short			num_cursors;
	short			index;
	CursHandle		frames[10];
} CursorTask;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
typedef struct {
	short		n;
	short		index;
	short		frameX[20];
} **AcurHdl;
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#if defined(powerc) || defined (__powerc)

#else
static CursorTask gCursors;
static Boolean	  gInstalled = false;

pascal long GetVBLRec(void) = {0x2e88};
static pascal void	ChangeCursor(void);
#endif


/*----------------------------------------------------------------------------+
|	LoadSpinning                                                              |
+----------------------------------------------------------------------------*/
void LoadSpinning(short id)
{
#if defined(powerc) || defined (__powerc)

#else
AcurHdl		acur;
CursHandle	myC;
short		i;

gInstalled = false;
gCursors.num_cursors = -1;
acur = (AcurHdl) Get1Resource('acur', id);

/* Preload all cursors we will use */
if( acur ) {
	i = (*acur)->n;
	if (i <= 10) {
		gCursors.num_cursors = i;
		gCursors.index = 0;
		for( i = 0; i < gCursors.num_cursors; i++ ) {
			gCursors.frames[i] = myC = GetCursor((*acur)->frameX[i << 1]);
			if (myC)
				HLockHi((Handle)myC);
			}
		}
	ReleaseResource((Handle)acur);
	}
#endif
}

/*----------------------------------------------------------------------------+
|	ChangeCursor                                                                |
+----------------------------------------------------------------------------*/

#if defined(powerc) || defined (__powerc)

#else
pascal void	ChangeCursor(void)
{
CursorTask *ptr;
CursHandle	tempCH;

ptr = (CursorTask *)GetVBLRec();

if ((ptr->num_cursors > 0) && (LMGetCrsrBusy() == false) &&
	(tempCH = ptr->frames[ptr->index])) {
	SetCursor(*tempCH);
	ptr->index++;
	if (ptr->index == ptr->num_cursors)
		ptr->index = 0;
	}
ptr->vblTask.vblCount = kInterval;
}
#endif

/*----------------------------------------------------------------------------+
|	StartSpinning                                                                |
+----------------------------------------------------------------------------*/

Boolean StartSpinning(void)
{
#if defined(powerc) || defined (__powerc)
SetCursor(*GetCursor(watchCursor));
#else
OSErr err;

	gInstalled = false;
	ShowCursor();
	if (gCursors.num_cursors < 0)
		return false;

	gCursors.index = 1;
	gCursors.vblTask.qType = vType;
	gCursors.vblTask.vblAddr = (VBLUPP)ChangeCursor;
	gCursors.vblTask.vblCount = kInitialDelay;
	gCursors.vblTask.vblPhase = 0;

	err = VInstall((QElemPtr)&gCursors.vblTask);
	if (err == noErr)
		gInstalled = true;

	return err == noErr;
#endif
}

/*----------------------------------------------------------------------------+
|	StopSpinning                                                                 |
+----------------------------------------------------------------------------*/
void StopSpinning(void)
{
#if defined(powerc) || defined (__powerc)

#else
OSErr err;
short i;

if (gInstalled) {
	err = VRemove((QElemPtr)&gCursors.vblTask);
	
	for (i = 0; i < gCursors.num_cursors; i++) {
		HUnlock((Handle)gCursors.frames[i]);
		ReleaseResource((Handle)gCursors.frames[i]);
		}
	gInstalled = false;
	}
#endif
InitCursor();
}
