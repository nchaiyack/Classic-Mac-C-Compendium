#include "cursor layer.h"

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif
struct MyAcurStruct
{
	short numberOfFrames;		/* number of cursors to animate */
	short whichFrame;			/* current frame number */
	CursHandle frame[];			/* Pointer to the first cursor */
};
#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

typedef struct MyAcurStruct		MyAcurStruct, *MyAcurPtr, **MyAcurHandle;

#define HiWrd(aLong)	(((aLong) >> 16) & 0xFFFF)

static	short			gTickInterval;	/* number of ticks between a frame switch */
static	long			gLastTick;	/* tick count of last call to SpinCursor */
static	MyAcurHandle	gFrameList;		/* our cursor list */

Boolean SetupTheAnimatedCursor(short acurID, short interval)
{
	register	short			i=0;
	register	short			cursID;
				Boolean 		noErrFlag = FALSE;

	if ((gFrameList=(MyAcurHandle)GetResource('acur', acurID))!=0L)
	{
		noErrFlag = TRUE;
		while((i<(*gFrameList)->numberOfFrames) && noErrFlag)
		{
			cursID = (short) HiWrd((long) (*gFrameList)->frame[i]);
			(*gFrameList)->frame[i] = GetCursor(cursID);
			if((*gFrameList)->frame[i])
				i++;
			else
				noErrFlag=FALSE;
		}
	}
	if(noErrFlag)
	{
		gTickInterval = interval;
		gLastTick = TickCount();
		(*gFrameList)->whichFrame = 0;
	}
	return noErrFlag;
}

void ShutdownTheAnimatedCursor(void)
{
	short i;

	for (i=0; i<(*gFrameList)->numberOfFrames; i++)
		ReleaseResource((Handle) (*gFrameList)->frame[i]);
	ReleaseResource((Handle) gFrameList);
}

void AnimateTheCursor(void)
{
	register long		newTick=TickCount();

	if (newTick < (gLastTick + gTickInterval))
		return;

	SetCursor(*((*gFrameList)->frame[(*gFrameList)->whichFrame++]));
	if ((*gFrameList)->whichFrame == (*gFrameList)->numberOfFrames)
		(*gFrameList)->whichFrame = 0;
	gLastTick = newTick;
}
