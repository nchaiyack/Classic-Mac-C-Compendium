#include "sound layer.h"
#include <Sound.h>

Boolean				gSoundToggle;
Boolean				gSoundAvailable;
Boolean				gSoundIsFinishedPlaying;

static	SndChannelPtr	myChannel;
static	Handle			gSoundHandle[NUM_SOUNDS];

pascal void SoundIsComplete(SndChannelPtr theChannel, SndCommand theCmd);

void InitTheSounds(void)
{
	short			i;
	
	gSoundAvailable=(SndNewChannel(&myChannel, 0, 0L, 0L)==noErr);
	if (gSoundAvailable)
		CloseTheSoundChannel();
	for (i=0; i<NUM_SOUNDS; i++)
	{
		gSoundHandle[i]=GetResource('snd ', i+1000);
		if (gSoundHandle[i]!=0L)
			HLockHi(gSoundHandle[i]);
	}
}

void DoSound(short whichSound, Boolean async)
{
	SndCommand			myCommand;
// hey, this will leak memory if compiled for native PowerMac
	SndCallBackUPP		callbackProc=NewSndCallBackProc(SoundIsComplete);
	
	CloseTheSoundChannel();
	whichSound-=1000;
	if ((gSoundToggle) && (gSoundAvailable))
	{
		if (!gSoundHandle[whichSound])
		{
			gSoundHandle[whichSound]=GetResource('snd ', whichSound+1000);
			if (gSoundHandle[whichSound]!=0L)
				HLockHi(gSoundHandle[whichSound]);
		}
		
		if (gSoundHandle[whichSound])
		{
			if (myChannel==0L)
			{
				if (SndNewChannel(&myChannel, 0, 0L, callbackProc) != noErr)
				{
					myChannel = 0;
					gSoundAvailable = FALSE;
				}
			}
			
			if (myChannel!=0L)
			{
				myChannel->userInfo = (long)&gSoundIsFinishedPlaying;
				SndPlay(myChannel, (SndListHandle)gSoundHandle[whichSound], async);
				if (async)
				{
					myCommand.cmd=callBackCmd;
					myCommand.param1=myCommand.param2=0;
					SndDoCommand(myChannel, &myCommand, false);
				}
			}
		}
	}
}

pascal void SoundIsComplete(SndChannelPtr theChannel, SndCommand theCmd)
{
	*(Boolean*)(theChannel->userInfo) = TRUE;
}

Boolean SoundChannelIsOpenQQ(void)
{
	return (myChannel!=0L);
}

void CloseTheSoundChannel(void)
{
	if (myChannel)
		SndDisposeChannel(myChannel, TRUE);
	myChannel=0L;
	gSoundIsFinishedPlaying=FALSE;
}
