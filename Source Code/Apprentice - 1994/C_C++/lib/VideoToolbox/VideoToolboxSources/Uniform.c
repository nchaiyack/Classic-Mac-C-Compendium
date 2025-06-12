/*
Uniform.c
UniformSample() returns a random sample from the interval [0,1). 
It has a mean of 1/2 and a variance of 1/12.

Also see: Binomial.c, ChiSquare.c, Exponential.c, Normal.c

HISTORY:
12/28/91 dgp extracted it from Normal.c
*/
#include "VideoToolbox.h"

double UniformSample(void)
{
	return rand()*(1.0/(RAND_MAX+1.0));
}
/*
The constant is evaluated by the compiler. Multiplication is faster than division.
*/