/*
 * interface to random number generator
 */

/*
 *  i = die (max)
 *  long max;
 *  returns a value from 0 to n-1, drawn from a random population with
 *  even distribution.
 */
long die(long max);

/*
 * return a random 32bit number
 */
unsigned long lrand(void);

/*
 * pick a starting point for lrand
 */
void init_lrand(void);
