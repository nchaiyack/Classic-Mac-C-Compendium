/*
 *		mrandom.h
 *
 *	A wrapper for random(), to allow safe restart.
 *
 *	Original Implementation:
 *		Clark Thomborson, September 1991
 *
 *	This material is based upon work supported by the National
 *	Science Foundation under grant number MIP-9023238.  The
 *	Government has certain rights in this material.
 *
 *	Any opinions, findings, and conclusions or recommendations
 *	expressed in this material are those of the author and do
 *	not necessarily reflect the view of the National Science
 *	Foundation.
 *
 *	This code is neither copyrighted nor patented.
 */

#define RNGIDSTRLEN 80 /* max length of string written by describe_rng() */

char *describe_rng (/* char rngid[RNGIDSTRLEN] */);
/* Write a RNG state identifier into the user-supplied string rngid,
 * which must be of length at least RNGIDSTRLEN.  If the user has not
 * initialized the rng with init_rng(), restart_rng(), or reconstruct_rng(),
 * abort with an error message to stderr.  Otherwise return the value of rngid.
 */

int init_rng (/* long seed; char *filename; */);
/* Create a random number statefile initialized with the given seed.
 * Return 1 if file is successfully created, 0 otherwise.
 */

int restart_rng (/* char *filename; */);
/* Restart a generator from a statefile.  Print a message on stderr
 * if the restart failed due to a garbled or non-existent statefile, 
 * and return 1.  Otherwise return 0.
 */

double frandom ();
/* Generate a uniformly-distributed number a, 0.0 <= a < 1.0, using
 * the 4.3bsd additive congruential generator random().
 */ 

long mrandom (/* long m; */);
/* Generate a random integer, uniformly distributed in the range 0..m-1.
 * We use the most-significant bits of the result of a random() call.
 */

int save_rng(/* char *filename; */);
/* Save the RNG state to a statefile, after calling random() enough
 * times to reset its internal state variables to their initial values.
 * Check to be sure the RNG can be restarted by calling restart_rng().
 * Return 0 if a problem is detected, printing an error message on stderr.
 * Otherwise return 1.
 */

void reconstruct_rng (/* long seed, count1, count2; */);
/* Rebuild a random() state by reseeding the generator, then making
 * (count2*1e9 + count1) calls to random().  Useful for error-checking
 * and error-recovery routines, although it is very slow for large counts.
 */

/* end mrandom.h */
