////////////
//
//	TimeSpeak.cp
//
//	A simple audible clock which speaks the time using voice samples
//	stored as sound resorces in the application.
//
//	Copyright � 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <sound.h>
#include "SpeakStrings.h"

void TimeSpeak(void);
static void	Sound(short	sndResID);

void TimeSpeak(void)
{
	Boolean	am = true;
	short	hour,minute;
	DateTimeRec	now;

	GetTime(&now);
	hour = now.hour;
	if (hour >= 12)
		am = false;
	if (hour > 12)
		hour -= 12;
	minute = now.minute;
	Sound(timeIs);
	Sound(ones[hour]);
	if (minute % 60 )
	{
		if (minute > 19 )
				{
					Sound(tens[minute/10]);
					if (minute % 10)
						Sound(ones[minute % 10]);
				}
		else
			Sound(ones[minute]);
		Sound(amFlag[am]);
	}
	else
		Sound(oClock);
}

void	Sound(short	sndResID)
{
	SndChannelPtr	mySndChan = nil;
	Handle			mySndHandle;
	Boolean			kAsync = true;
	OSErr			myErr;
	
	mySndHandle = GetResource('snd ',sndResID);
	if (mySndHandle)
	{
		myErr = SndPlay(mySndChan, mySndHandle, kAsync);
		if (myErr)
		{
			FlashMenuBar(0);
			SysBeep(1);
			FlashMenuBar(0);
		}
	}
	else //need a more informative error message here
	{
		FlashMenuBar(0);
		SysBeep(1);
		FlashMenuBar(0);
	}
}
