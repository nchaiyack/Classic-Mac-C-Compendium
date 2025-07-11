/*
	Terminal 2.0
	"Sieve.s"

This script was used to test the "Terminal" script interpreter. It is of no
practical use. It is the standard Sieve benchmark often used to test the
performance of computers. Note that because the "Terminal" script language
is an interpreted language it is rather slow for loop intensive operations,
but for its intended purpose as a communications script language it is fast
enough.

The timing results were as follows (compared to the same program compiled
with a real C compiler), Terminal compiled with THINK C 4.0

	Macintosh IIcx             : 108 seconds ( 43.2 ms ==> 2500 times)
 	Macintosh IIcx 68020 option: 108 seconds
	Macintosh Plus             : 536 seconds (271.0 ms ==> 1978 times)

Terminal compiled with MPW 3.0

	Macintosh IIcx             :  93 seconds (86 % of THINK C version)
	Macintosh IIcx 68020 option:  91 seconds (84 % of THINK C version)
	Macintosh Plus             : 503 seconds (93 % of THINK C version)
*/

int SIZE = 8191;
int LAUF = 1;
char FALSE = 0;
char TRUE = 1;

main()
{
	int tick, iter, count, i, k, prime;
	char *flags, *p;

	display("Sieve benchmark %i passes\rThis may take several minutes...\r",
		LAUF);
	if (!(flags = new(SIZE))) {
		display("Not enough memory\r")
		return;
	}
	tick = time();							/* <====== start */

	for (iter = 0; iter < LAUF; ++iter) {
		count = 0;
		p = flags + SIZE;
		while (p > flags)
			*(--p) = TRUE;
		/* p == flags */
		for (i = 0; i < SIZE; ++i) {
			if (*p) {
				k = i + (prime = i + i + 3);
				while (k < SIZE) {
					flags[k] = FALSE;
					k = k + prime;
				}
				++count;
				/*display("%i\r", prime);*/
			}
			++p;
		}
	}

	tick = time() - tick;					/* <====== stop */
	free(flags);
	display("%i primes in %i seconds\r", count, tick);
}
