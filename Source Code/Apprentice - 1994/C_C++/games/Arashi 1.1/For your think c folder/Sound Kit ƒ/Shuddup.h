/*/
     Project Arashi: Shuddup.h
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 22:12
     Created: Saturday, March 18, 1989, 0:55

     Copyright � 1989-1992, Juri Munkki
/*/

#ifndef __RETRACE__
#include <Retrace.h>
#endif

typedef	struct
{
	Ptr			SoundB;		/*	Sound data buffer address			*/
	long		SoundP;		/*	Sound data buffer offset			*/
	Ptr			ChanA;		/*	Channel A sampled data ptr			*/
	Ptr			ChanB;		/*	Channel B sampled data ptr			*/
	int			CountA;		/*	When count reaches 0, sound ends.	*/
	int			CountB;		/*	Initially Count = Len / 188			*/
	int			PriA;		/*	Channel A priority.					*/
	int			PriB;		/*	Channel B priority.					*/
	ioParam		SPar;		/*	Parameter block for our sound		*/
	VBLTask		VBL;		/*	VBLTask entry						*/
	long		*TickPtr;	/*	Private Tickcount Pointer			*/
	long		SKOpen;		/*	-1 if sound kit is open.			*/
	int			volume;		/*	Between 1 and 8						*/
}	VVars;

/*	Sound kit uses the following resource type for all resources:	*/
#define	SKRESTYPE	'SKIT'
#define	SKSYSJUMP	1000
#define	SKSTABLE	1001
#define	SKHUFFMANN	1002
/*
>>	The following structure exists for every sound.
*/
typedef	struct
{
	Ptr		Poin;
	long	Len;
	int		Count;	
}	SoundStuff;

#ifndef SOUNDKIT_MAIN
extern	VVars		Vv;
extern	SoundStuff	*Sounds;	/*	Storage for sound info			*/
extern	int			NumSounds;
extern	Ptr			SKPtr;
extern	int			OldSound;
#endif