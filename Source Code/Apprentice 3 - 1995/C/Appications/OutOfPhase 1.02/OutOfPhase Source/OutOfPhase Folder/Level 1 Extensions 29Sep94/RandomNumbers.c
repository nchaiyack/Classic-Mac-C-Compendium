/* RandomNumbers.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "RandomNumbers.h"


#define CHECKPARKANDMILLER (1)


static long				ParkAndMillerSeed = 1;

#define ParkAndMillerA (16807L)
#define ParkAndMillerM (2147483647L)
#define ParkAndMillerQ (127773L)
#define ParkAndMillerR (2836L)
/* this implements the Park and Miller (Communications of the ACM, 1988) Minimal */
/* Standard random number generator. it returns a number in the range [1..2147483646] */
long							ParkAndMillerRandom(void)
	{
		long						Lo;
		long						Hi;
		long						Test;

		Hi = ParkAndMillerSeed / ParkAndMillerQ;
		Lo = ParkAndMillerSeed % ParkAndMillerQ;
		Test = ParkAndMillerA * Lo - ParkAndMillerR * Hi;
		if (Test > 0)
			{
				ParkAndMillerSeed = Test;
			}
		 else
			{
				ParkAndMillerSeed = Test + ParkAndMillerM;
			}
		ERROR((ParkAndMillerSeed < 1L) || (ParkAndMillerSeed > 2147483646L),PRERR(ForceAbort,
			"ParkAndMillerRandom:  seed exceeded range."));
		ERROR((1L != PARKANDMILLERMINIMUM) || (2147483646L != 2147483646L),PRERR(ForceAbort,
			"ParkAndMillerRandom:  limit macros are bad"));
		return ParkAndMillerSeed;
	}


/* set the Park and Miller random number seed.  it returns the old seed so that */
/* multiple clients can save and restore the seed so they don't interfere with */
/* each other.  the seed must be in the range [1..2147483646]. */
long							SetParkAndMillerRandomSeed(long NewSeed)
	{
		long						OriginalSeedValue;
#if DEBUG && CHECKPARKANDMILLER
		long						Counter;
		long						Value;
#endif

		ERROR((NewSeed < 1) || (NewSeed > 2147483646),PRERR(ForceAbort,
			"SetParkAndMillerRandomSeed:  seed is out of range"));
		OriginalSeedValue = ParkAndMillerSeed;
#if DEBUG && CHECKPARKANDMILLER
		ParkAndMillerSeed = 1;
		for (Counter = 1; Counter <= 10000; Counter += 1)
			{
				Value = ParkAndMillerRandom();
			}
		if (Value != 1043618065)
			{
				PRERR(ForceAbort,"SetParkAndMillerRandomSeed:  random number failure");
			}
#endif
		ParkAndMillerSeed = NewSeed;
		return OriginalSeedValue;
	}



static long				LEcuyerSeed1 = 1; /* must be in [1..2147483562] */
static long				LEcuyerSeed2 = 1; /* must be in [1..2147483398] */

/* this implements the L'Ecuyer (Communications of the ACM, 1988) hybrid 32-bit */
/* random number generator.  it returns a value in the range [1..2147483562] */
/* WARNING:  no test data or algorithm was supplied in the article, so the */
/* correctness of this implementation can not be guarranteed. */
long							LEcuyerRandom(void)
	{
		long						Z;
		long						K;

		K = LEcuyerSeed1 / 53668L;
		LEcuyerSeed1 = 40014L * (LEcuyerSeed1 - K * 53668L) - K * 12211L;
		if (LEcuyerSeed1 < 0)
			{
				LEcuyerSeed1 = LEcuyerSeed1 + 2147483563L;
			}
		K = LEcuyerSeed2 / 52774L;
		LEcuyerSeed2 = 40692L * (LEcuyerSeed2 - K * 52774L) - K * 3791L;
		if (LEcuyerSeed2 < 0)
			{
				LEcuyerSeed2 = LEcuyerSeed2 + 2147483399L;
			}
		Z = LEcuyerSeed1 - LEcuyerSeed2;
		if (Z < 1)
			{
				Z = Z + 2147483562L;
			}
		ERROR((LEcuyerSeed1 < 1L) || (LEcuyerSeed1 > 2147483562L),PRERR(ForceAbort,
			"LEcuyerRandom:  first seed is out of range"));
		ERROR((LEcuyerSeed2 < 1L) || (LEcuyerSeed2 > 2147483398L),PRERR(ForceAbort,
			"LEcuyerRandom:  second seed is out of range"));
		ERROR((LECUYERMINIMUM != 1L) || (LECUYERMAXIMUM != 2147483562L),PRERR(ForceAbort,
			"LEcuyerRandom:  limit macros are bad"));
		ERROR((Z < LECUYERMINIMUM) || (Z > LECUYERMAXIMUM),PRERR(ForceAbort,
			"LEcuyerRandom:  return value is outside of limit macro range"));
		return Z;
	}


/* sets the LEcuyer seeds.  the first seed must be in the range [1..2147483562] */
/* and the second seed in the range [1..2147483398].  the previous are returned */
/* so that multiple clients can save and restore seeds without interfering with */
/* each other.  the pointers may be NIL if the old seeds values are not needed. */
void							SetLEcuyerRandomSeed(long NewS1, long NewS2, long* OldS1Out,
										long* OldS2Out)
	{
		ERROR((NewS1 < 1L) || (NewS1 > 2147483562L),PRERR(ForceAbort,
			"SetLEcuyerRandomSeed:  first seed is out of range"));
		ERROR((NewS2 < 1L) || (NewS2 > 2147483398L),PRERR(ForceAbort,
			"SetLEcuyerRandomSeed:  second seed is out of range"));
		if (OldS1Out != NIL)
			{
				*OldS1Out = LEcuyerSeed1;
			}
		if (OldS2Out != NIL)
			{
				*OldS2Out = LEcuyerSeed2;
			}
		LEcuyerSeed1 = NewS1;
		LEcuyerSeed2 = NewS2;
	}
