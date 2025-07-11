/*
//	SoundUtils.c
//
//	Created: 7/20/92 at 6:50:42 PM
//	By:	Tony Myles
//
//	Copyright � 1992-94 Tony Myles, All rights reserved worldwide.
*/


#ifndef __SOUNDUTILS__
#define __SOUNDUTILS__

#ifndef __SOUND__
#include <Sound.h>
#endif


	// constants
enum
{
	kWaitIfFull = true,
	kSoundComplete = 1
};


	// prototypes
Boolean SndChannelBusy(SndChannelPtr sndChannelP);
OSErr InstallSoundCallBack(SndChannelPtr sndChannelP);
Handle GetSoundResource(short soundResourceID, OSErr* err);


#endif /* __SOUNDUTILS__ */