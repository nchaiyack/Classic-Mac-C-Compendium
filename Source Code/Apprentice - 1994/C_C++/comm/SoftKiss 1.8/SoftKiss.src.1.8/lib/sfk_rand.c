/*
 * generate random 32bit numbers
 */

#include "sfk_die.h"

static unsigned long seed;

/*
 * see knuth Volume 2 pages 9 and 15
 */
unsigned long lrand()
{
	seed=seed*11597L+199L;
	return seed;
}
/*
 * pick a starting point for lrand
 */
void init_lrand(void)
{
	GetDateTime(&seed);
}


