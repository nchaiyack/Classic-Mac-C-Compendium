/* 
SndPlay1.c

SndPlay1(snd) plays a sound, asynchronously, i.e. it returns immediately, while the
sound is still playing. The argument is a handle to a snd resource. If a sound is 
still playing from a previous call to SndPlay1, it is allowed to finish before
closing and reopening the channel and beginning the new sound.
SndPlay1(NULL) waits for the sound to end and then closes the channel.
SndStop1() closes the channel immediately.
SndDone1() returns true once the last sound initiated by SndPlay1() has 
finished.

GENERAL NOTE:
The Apple Sound Manager has the annoying characteristic of insisting on loading
any synth that's mentioned in a snd resource, even if that synth is already loaded,
which causes an error. This makes it necessary to dispose of and recreate the 
snd channel before each new sound, which these routines do. Apple advises against 
this approach because it leaves the channel open a lot of the time, 
which blocks SysBeep. 

To avoid this, make sure to call SndPlay1(NULL) or SndStop1() to close the channel after
you start a sound going.

The easiest way to get a sound to play is to call GetNamedResource(). 
	Handle snd;
	snd=GetNamedResource('snd ',"\pSimple Beep");
However, it's also easy to create your own snd in memory, as a series of commands,
following the instructions in Inside Mac VI.

HISTORY:
3/30/92	dgp wrote it.
4/1/92	dgp	renamed it and commented out the printf's.
7/9/93	dgp Test MATLAB in if() instead of #if. 
*/
#include "VideoToolbox.h"
#include <Sound.h>
static pascal void SndCallBack(SndChannelPtr channel,SndCommand command);
static SndChannelPtr channel=NULL;
static short soundDone;

OSErr SndPlay1(Handle snd)
{
	OSErr error=0;
	SndCommand callBack;
	static firstTime=1;

	if(firstTime){
		if(!MATLAB)_atexit(SndStop1);
		firstTime=0;
	}
	if(channel!=NULL)error=SndDisposeChannel(channel,FALSE);	// wait till done
	channel=NULL;
	if(error)return error;
	if(snd==NULL)return 0;
	error=SndNewChannel(&channel,0,0L,SndCallBack);
	if(error){
//		printf("SndPlay1:SndNewChannel failed with error %d\n",error);
		return error;
	}
	error=SndPlay(channel,snd,TRUE);
	if(error){
//		printf("SndPlay1:SndPlay failed with error %d\n",error);
		return error;
	}
	soundDone=0;
	callBack.cmd=callBackCmd;
	callBack.param1=1;
	callBack.param2=(long)&soundDone;
	error=SndDoCommand(channel,&callBack,FALSE);
//	if(error)printf("SndPlay1:SndDoCommand failed with error %d\n",error);
	return error;
}

void SndStop1(void)
{
	if(channel!=NULL)SndDisposeChannel(channel,TRUE);		// immediately
	channel=NULL;
}

short SndDone1(void)
// Returns true once the last sound initiated by SndPlay1 has 
// finished.
{
	return soundDone;
}

#pragma options(!profile)	// Disable profiling because A5 may be invalid.

static pascal void SndCallBack(SndChannelPtr channel,SndCommand command)
// Load a short int.
// Called back by sound manager.  Lets us know when sound is done.
{
	if(command.param2 != 0L) *(short *)command.param2=command.param1;
}