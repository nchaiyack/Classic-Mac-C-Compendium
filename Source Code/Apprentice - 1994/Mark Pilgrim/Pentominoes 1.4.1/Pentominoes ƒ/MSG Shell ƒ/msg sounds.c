/**********************************************************************\

File:		msg sounds.c

Purpose:	This module handles playing asynchronous sounds.  Also,
			a modal dialog filter that lets the user cough to continue
			(if an internal microphone is available).


Pentominoes - a 2-D geometry board game
Copyright (C) 1993 Mark Pilgrim

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

#include "msg sounds.h"
#include "msg graphics.h"

SndChannelPtr		myChannel;
Handle				MySounds[NUM_SOUNDS];
int					gSoundToggle;
Boolean				gSoundAvailable;

#include "SoundInput.h"

static long		gSoundInputRefNum;

OSErr OpenTheSoundDevice(void)
{
	OSErr isHuman;
	short meterState = 1;
	
	if (!(isHuman = SPBOpenDevice(0L, siWritePermission, &gSoundInputRefNum)))
		isHuman = SPBSetDeviceInfo(gSoundInputRefNum, siLevelMeterOnOff, &meterState);
	return isHuman;
}

void CloseTheSoundDevice(void)
{
	SPBCloseDevice(gSoundInputRefNum);
	gSoundInputRefNum = 0;
}

pascal Boolean ProcOFilter(DialogPtr dialog, EventRecord *event, short *item)
{
	unsigned char	theKey;
	OSErr			isHuman;
	short			recordingStatus = 0;
	short			meterLevel = 0;
	unsigned long	totalSamplesToRecord = 0;
	unsigned long	numberOfSamplesRecorded = 0;
	unsigned long	totalMsecsToRecord;
	unsigned long	numberOfMsecsRecorded;
	
	if(gSoundInputRefNum)
	{
		isHuman = SPBGetRecordingStatus(gSoundInputRefNum, &recordingStatus, &meterLevel,
									&totalSamplesToRecord, &numberOfSamplesRecorded,
									&totalMsecsToRecord,&numberOfMsecsRecorded);
		
		if (meterLevel > 250)
		{
			*item=1;
			return TRUE;
		}
	}
	
	switch(event->what)
	{
		case keyDown:
		case autoKey:
			theKey = event->message & charCodeMask;
			
			if (theKey == 0x0D || theKey == 0x03 || theKey == 0x1B)
			{
				*item=1;
				return TRUE;
			}
	}
	return FALSE;
}

void InitSounds(void)
{
	int				i;
	
	gSoundToggle=TRUE;
	gSoundAvailable=(SndNewChannel(&myChannel, 0, 0L, 0L)==noErr);
	if (!gSoundAvailable)
		myChannel=0;
	for (i=0; i<NUM_SOUNDS; i++)
		MySounds[i]=0L;
}

void DoSound(int whichSound)
{
	SndCommand		myCommand;
	OSErr			isHuman;
	Str255			tempStr;
	
	if (myChannel!=0)
	{
		SndDisposeChannel(myChannel, TRUE);
		myChannel=0;
	}
	
	whichSound-=1000;
	if ((gSoundToggle) && (gSoundAvailable))
	{
		if (!MySounds[whichSound])
			MySounds[whichSound]=GetResource('snd ', whichSound+1000);
		
		if (MySounds[whichSound])
		{
			if(SndNewChannel(&myChannel, 0, 0L, 0L) != noErr)					
			{
				myChannel = 0;
				gSoundAvailable = FALSE;
			}
			else SndPlay(myChannel, MySounds[whichSound], TRUE);
		}
	}
}

void CloseSounds(void)
{
	int				i;
	
	if(myChannel)
		SndDisposeChannel(myChannel, TRUE);
}
