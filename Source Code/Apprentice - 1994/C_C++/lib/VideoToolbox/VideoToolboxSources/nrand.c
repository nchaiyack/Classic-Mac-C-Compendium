/*
nrand.c
nrand(n) returns a random integer in the range [0, n-1], provided n>0. If n is zero
the returned value is zero.

nrandU() and nrandUL() are obsolete; use nrand() instead.

HISTORY:
4/29/88	dgp	wrote it.
3/19/90	dgp	made it portable between THINK C and MPW C.
9/13/92	dgp	Using THINK C's Disassembler I noticed that I could substantially speed
			up the code, replacing the long division by a bit shift. The answers for 
			legal values of n, i.e. n>0, are unchanged, but the answers for values of 
			n outside that range have changed because I now explicitly cast n
			to unsigned long rather than long. (Bit shifting and division give different
			answers when the numerator is negative.)
			Added nrandU() and nrandUL().
9/18/92	dgp	Cast rand() from int to unsigned short to prompt THINK C to generate 
			tighter code.
3/26/94	dgp Replaced all three routines (nrand, nrandU, and nrandUL) by one universal
			routine, nrand, that uses integer arithmetic (as in nrandU) if n is small 
			enough, and otherwise uses double arithmetic (as in nrandUL). However,
			even for small n, the new routine differs from the old nrandU() because
			I replaced the call to rand() by a call to randU() because this
			allows us to use integer arithmetic for values of n up to
			USHRT_MAX instead of just SHRT_MAX.		
*/
#include "VideoToolbox.h"
#include <assert.h>

unsigned long nrand(unsigned long n)
{
	if(ULONG_MAX/USHRT_MAX/USHRT_MAX>=1 && n<=USHRT_MAX)return n*randU()>>16;
	else return (unsigned long)((double)n*randUL()/(ULONG_MAX+1.0));
}

#if 0	// Old version, before March, 1994.
	int nrand(short n)
	{
		assert(RAND_MAX+1UL==1UL<<15);
		return (unsigned long)n*rand()>>15;
	}
		
	#if RAND_MAX > ULONG_MAX/USHRT_MAX
		#error "nrandU() assumes that an unsigned long can hold the product of unsigned short & rand()."
	#endif
	
	unsigned short nrandU(unsigned short n)
	{
		assert(RAND_MAX+1UL==1UL<<15);
		return (unsigned long)n*(unsigned short)rand()>>15;
	}

	unsigned long nrandUL(unsigned long n)
	{
		return (unsigned long)((double)n*randUL()/(ULONG_MAX+1.0)); 
	}
#endif