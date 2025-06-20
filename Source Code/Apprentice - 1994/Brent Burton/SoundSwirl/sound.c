/**************************
** sound.c
**
** manages the sound-related functions
***************************/

#include "main.h"
#include <SoundInput.h>

#define kAsynch TRUE

static long myInRefNum;
static SPB  mySPB;


/******************************************/
void InitSound(void)
{
} /* InitSound() */


/******************************************/
/** KillSound() is called only when the  **/
/** program is making its grand exit.    **/
/******************************************/
void KillSound(void)
{
	if (gListening)
		SndListenStop();     /* turn off listening */
} /* KillSound() */


/******************************************/
void SndListenBegin(void)
{
	OSErr myErr;

	myErr = SPBOpenDevice( NULL, siWritePermission, &myInRefNum);
	
	if (myErr == noErr)
	{
		mySPB.inRefNum = myInRefNum;
		mySPB.count = 0;
		mySPB.milliseconds = 0;
		mySPB.bufferLength = 0;
		mySPB.bufferPtr = NULL;     /* record forever, don't store it */
		mySPB.completionRoutine = NULL;
		mySPB.interruptRoutine = NULL;
		mySPB.userLong = 0;
		mySPB.error = noErr;
		mySPB.unused1 = 0;
		
		myErr = SPBRecord( &mySPB, kAsynch);  /* record asynchronously */
		
		if (myErr != noErr)
		{
			Str255 s1;
			NumToString( (long)myErr, s1);
			Error("\pError during SPBRecord(): ", s1, "\p", "\p");
			myErr = SPBCloseDevice( myInRefNum);
		}
	}
	else
		Error("\pCouldn't open the sound input device in InitSound().",
				"\p", "\p", "\p");
	return;
} /* SndListenBegin() */


/******************************************/
void SndListenStop(void)
{
	OSErr myErr;
	myErr = SPBStopRecording( myInRefNum);
	myErr = SPBCloseDevice(myInRefNum);
	myInRefNum = 0;
	return;
} /* SndListenStop() */


/******************************************/
int FetchSndOffset(void)
{
	OSErr myErr;
	short recStatus, meterLevel;
	unsigned long totSamp, numSamp, totMsecs, numMsecs;
	
	myErr = SPBGetRecordingStatus( myInRefNum,
			&recStatus, &meterLevel, &totSamp,
			&numSamp, &totMsecs, &numMsecs);
	return(meterLevel/2);
} /* FetchSndOffset() */

