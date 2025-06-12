/*
 * roll an n sided die
 */

#include "sfk_die.h"

#define MAXRAND 0x7fffffff

/*  die  --  roll an n-sided die
 *|wohl use long instead of int
 *
 *  i = die (max)
 *  long max;
 *  returns a value from 0 to n-1, drawn from a random population with
 *  even distribution.
 *
 *  Uses the random number generator (rand).  You must seed it with
 *  srand().
 *
 *  Written by Steven Shafer and Tom Rodeheffer, August 1978.
 *  Uses a Monte Carlo method; we have verified that it is not biased.
 *
 *  The maximum die value is MAXRAND, the largest number returned
 *  from the random number geneator.
 *
 * HISTORY
 * 06-Mar-80  Steven Shafer (sas) at Carnegie-Mellon University
 *	Created.
 *
 */
long die(long max)
{
	register unsigned long int binsize;		/* size of each of (max) bins */
	register unsigned long int draw;		/* the number we drew from rng */
	register unsigned long int minbad;		/* the smallest draw that's too big */

/* We divide the interval from 0 to MAXRAND into max "bins", each of
 * equal size and as large as possible.  We draw a random number which
 * must lie in a bin, and return the number of the bin into which
 * it falls.
 */

	if (max < 1)
		return (0);

	binsize = MAXRAND / max;
	minbad = max * binsize;		/* bigger than this is no good */

	do { draw = lrand(); } while (draw >= minbad);

	return (draw / binsize);
}
