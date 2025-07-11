/**********************************************************************\

File:		sounds.c

Purpose:	This module handles playing syncronous and asyncronous
			sounds.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

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
	
	CloseTheSoundChannel();
	
	whichSound-=1000;
	if ((gSoundToggle) && (gSoundAvailable))
	{
		if (!MySounds[whichSound])
			MySounds[whichSound]=GetResource('snd ', whichSound+1000);
		
		if (MySounds[whichSound])
		{
			if (SndNewChannel(&myChannel, 0, 0L, (ProcPtr)SoundIsComplete) != noErr)					
			{
				myChannel = 0;
				gSoundAvailable = FALSE;
			}
			else
			{
				myChannel->userInfo = (long)&gSoundIsFinishedPlaying;
				SndPlay(myChannel, MySounds[whichSound], async);
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
