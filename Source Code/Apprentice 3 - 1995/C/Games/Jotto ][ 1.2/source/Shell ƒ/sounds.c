#include "sounds.h"

SndChannelPtr		myChannel;
Handle				MySounds[NUM_SOUNDS];
Boolean				gSoundToggle;
Boolean				gSoundAvailable;
Boolean				gSoundIsFinishedPlaying;

pascal void SoundIsComplete(SndChannelPtr theChannel, SndCommand theCmd);

void InitTheSounds(void)
{
	short			i;
	
	gSoundAvailable=(SndNewChannel(&myChannel, 0, 0L, 0L)==noErr);
	if (gSoundAvailable)
		CloseTheSoundChannel();
	for (i=0; i<NUM_SOUNDS; i++)
		MySounds[i]=0L;
}

void DoSound(short whichSound, Boolean async)
{
	SndCommand			myCommand;
	SndCallBackUPP		callbackProc=NewSndCallBackProc(SoundIsComplete);
	
	CloseTheSoundChannel();
	
	whichSound-=1000;
	if ((gSoundToggle) && (gSoundAvailable))
	{
		if (!MySounds[whichSound])
			MySounds[whichSound]=GetResource('snd ', whichSound+1000);
		
		if (MySounds[whichSound])
		{
			if (SndNewChannel(&myChannel, 0, 0L, callbackProc) != noErr)
			{
				myChannel = 0;
				gSoundAvailable = FALSE;
			}
			else
			{
				myChannel->userInfo = (long)&gSoundIsFinishedPlaying;
				SndPlay(myChannel, (SndListHandle)MySounds[whichSound], async);
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

void CloseTheSoundChannel(void)
{
	if (myChannel)
		SndDisposeChannel(myChannel, TRUE);
	myChannel=0L;
	gSoundIsFinishedPlaying=FALSE;
}
