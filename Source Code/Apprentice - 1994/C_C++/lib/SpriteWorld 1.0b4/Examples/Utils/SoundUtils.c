/*
//	SoundUtils.c
//
//	Created: 7/20/92 at 6:50:42 PM
//	By:	Tony Myles
//
//	Copyright: � 1992-94 Tony Myles, All rights reserved worldwide.
*/


#ifndef __SOUNDUTILS__
#include "SoundUtils.h"
#endif


Boolean SndChannelBusy(
	SndChannelPtr sndChannelP)
{
	OSErr err;
	Boolean sndChannelBusy = true;
	SCStatus sndChannelStatus;

	err = SndChannelStatus(sndChannelP, sizeof(SCStatus), &sndChannelStatus);

	if (err == noErr)
	{
		sndChannelBusy = sndChannelStatus.scChannelBusy;
	}

	return sndChannelBusy;
}


OSErr InstallSoundCallBack(
	SndChannelPtr sndChannelP)
{
	SndCommand sndCmd;

	sndCmd.cmd = callBackCmd;
	sndCmd.param1 = kSoundComplete;
	sndCmd.param2 = SetCurrentA5();

	return SndDoCommand(sndChannelP, &sndCmd, kWaitIfFull);
}


Handle GetSoundResource(
	short soundResourceID,
	OSErr* err)
{
	Handle soundResourceH = GetResource(soundListRsrc, soundResourceID);

	if (soundResourceH == NULL)
	{
		*err = ResError();
		
		if (*err == noErr)
		{
			*err = resNotFound;
		}
	}

	return soundResourceH;
}



