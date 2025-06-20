/*/
     Project Arashi: VARandom.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:42
     Created: Tuesday, April 23, 1991, 0:16

     Copyright � 1991-1992, Juri Munkki
/*/

#include "VA.h"

#ifdef	PURE_C_VERSION_OF_RANDOM
#define	RANDCONST	((unsigned long)(0x41A7))
#define	HIGH(x)		((unsigned int)(x>>16))
#define	LOW(x)		((unsigned int)x)

int		VARandom()
{
	register	unsigned	long	temp;
	
	temp = RANDCONST * HIGH(VARandSeed) + HIGH(RANDCONST * LOW(VARandSeed));
	VARandSeed = ((temp & 0x7fff) << 16) + HIGH(temp << 1) + LOW(RANDCONST * VARandSeed);
	
	return	LOW(VARandSeed);
}
#endif

int		VARandom()
{
asm	{
		move.w	#0x41A7,D0
		move.w	D0,D2
		mulu.w	2+VARandSeed,D0
		move.l	D0,D1
		clr.w	D1
		swap.w	D1
		mulu.w	VARandSeed,D2
		add.l	D1,D2
		move.l	D2,D1
		add.l	D1,D1
		clr.w	D1
		swap.w	D1
		and.l	#0x0000ffff,D0
		sub.l	#0x7fffFFFF,D0
		and.l	#0x00007fff,D2
		swap.w	D2
		add.l	D1,D2
		add.l	D2,D0
		bpl.s	@positive
		add.l	#0x7fffFFFF,D0
@positive
		move.l	D0,VARandSeed
	}
}

int		FastRandom()
{
asm	{
		move.w	#0x41A7,D0
		move.w	D0,D2
		mulu.w	2+randSeed,D0
		move.l	D0,D1
		clr.w	D1
		swap.w	D1
		mulu.w	randSeed,D2
		add.l	D1,D2
		move.l	D2,D1
		add.l	D1,D1
		clr.w	D1
		swap.w	D1
		and.l	#0x0000ffff,D0
		sub.l	#0x7fffFFFF,D0
		and.l	#0x00007fff,D2
		swap.w	D2
		add.l	D1,D2
		add.l	D2,D0
		bpl.s	@positive
		add.l	#0x7fffFFFF,D0
@positive
		move.l	D0,randSeed
	}
}

