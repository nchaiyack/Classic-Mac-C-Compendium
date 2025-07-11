/*
Log2L.c
HISTORY:
12/3/91 dgp changed zero argument return from -1 to LONG_MIN.
*/
#include "VideoToolbox.h"

long Log2L(unsigned long j)
// returns the integer part of log2(j)
{
	long L=-1;
	register unsigned long i=j;	// Because THINK C won't put arguments into registers

	if(i==0)return LONG_MIN;
	while(i>0){
		i>>=1;
		L++;
	}
	return L;
}