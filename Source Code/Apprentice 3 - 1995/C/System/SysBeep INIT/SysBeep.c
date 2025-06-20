#include <sound.h>


pascal void main(int duration)
{
Handle		soundHand;
int			numSnds,theSnd;
short		volume;

	asm
	{
			movem.l		d0-d7/a0-a6,-(sp)
			
			bra.s		@skip
			
			dc.b		"moof"
oldTrap:	dc.l		0
			dc.l		0

skip:		nop
	}
	
	GetSoundVol(&volume);
	
	if (volume > 0)
	{
		numSnds = CountResources((long) 'snd ');
		theSnd = ((Random() & 0x7FFF) % numSnds) + 1;
		
		soundHand = GetIndResource((long) 'snd ',theSnd);
	}
	
	if (volume <= 0 || soundHand == 0L)
	{
		asm 
		{
				movem.l		(sp)+,d0-d7/a0-a6
				unlk		a6
				move.l		@oldTrap,-(sp)
				rts
		}
	}
	
	
	SndPlay(0L,soundHand,FALSE);
	
	HUnlock(soundHand);
	HPurge(soundHand);
	
	asm 
	{
			movem.l		(sp)+,d0-d7/a0-a6
	}
}
