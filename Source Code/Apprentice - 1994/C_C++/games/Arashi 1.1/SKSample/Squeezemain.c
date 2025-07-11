/*/
     Project Arashi/STORM: Squeezemain.c
     Major release: 9/9/92

     Last modification: Wednesday, September 9, 1992, 21:46
     Created: Saturday, October 6, 1990, 1:16

     Copyright � 1990-1992, Juri Munkki
/*/

/*
>>	This program is meant to illustrate how
>>	the sound kit can be used. It also does
>>	the packing phase.
*/

/*
>>	Use the following two defines to control
>>	what this program does. Ugly, but simple.
*/

#define	no_COMPRESS
#define	no_DYNAMIC

#include "Shuddup.h"

/*		NOTE:
>>
>>			If you can find something funny about the
>>			routine DoInits, please contact me immediately.
>>			Your help will be appreciated.
>>
*/
void	main()
{
	int		i;
	long	ticker;
	Handle	filename;
	
	DoInits();

	filename = (Handle)GetString(128);
	HLock(filename);
	OpenResFile(*filename);

#ifdef	COMPRESS
	SetPort(GetNewWindow(1000,0,(void *)-1));

	DoCompress();
#else
	ticker=TickCount();
	InitSoundKit();
	ticker=TickCount()-ticker;
	SKVolume(8);

#ifndef DYNAMIC
	for(i=0;i<NumSounds;i++)
	{	PlayA(i,999-i);
		while(!Button());
		while(Button());
	}
#else	
	{	Handle	thedata;

		/*	Load a sampled sound resource (SoundWave format)		*/	
		OpenResFile("\pDynamic Sample Resource");
		thedata = GetResource('wave',128);
		DetachResource(thedata);
		SKProcessHandle(thedata);

		/*	Play the sampled sound resource.						*/
		HLock(thedata);
		SKPlayHandleA(thedata);
		while(!Button());
	}
#endif
	
	CloseSoundKit();
#endif
}