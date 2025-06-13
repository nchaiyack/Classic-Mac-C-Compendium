/* RandomNumbers.h */

#ifndef Included_RandomNumbers_h
#define Included_RandomNumbers_h

/* RandomNumbers module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */

#define PARKANDMILLERMINIMUM (1L)
#define PARKANDMILLERMAXIMUM (2147483646L)

/* this implements the Park and Miller (Communications of the ACM, 1988) Minimal */
/* Standard random number generator. it returns a number in the range [1..2147483646] */
long							ParkAndMillerRandom(void);

/* set the Park and Miller random number seed.  it returns the old seed so that */
/* multiple clients can save and restore the seed so they don't interfere with */
/* each other.  the seed must be in the range [1..2147483646]. */
long							SetParkAndMillerRandomSeed(long NewSeed);


#define LECUYERMINIMUM (1L)
#define LECUYERMAXIMUM (2147483562L)

/* this implements the L'Ecuyer (Communications of the ACM, 1988) hybrid 32-bit */
/* random number generator.  it returns a value in the range [1..2147483562] */
/* WARNING:  no test data or algorithm was supplied in the article, so the */
/* correctness of this implementation can not be guarranteed. */
long							LEcuyerRandom(void);

/* sets the LEcuyer seeds.  the first seed must be in the range [1..2147483562] */
/* and the second seed in the range [1..2147483398].  the previous are returned */
/* so that multiple clients can save and restore seeds without interfering with */
/* each other.  the pointers may be NIL if the old seeds values are not needed. */
void							SetLEcuyerRandomSeed(long NewS1, long NewS2, long* OldS1Out,
										long* OldS2Out);

#endif
