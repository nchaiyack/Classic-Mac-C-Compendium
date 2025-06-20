/* Exponential.c 
Routines related to the exponential probability distribution.
Also see: Binomial.c, ChiSquare.c, Normal.c, Uniform.c
*/
#include <math.h>

double ExponentialPdf(double x);

double ExponentialPdf(double x)
// Mean and variance are 1.
{
	if(x<0.0)return 0.0;
	else return exp(-x);
}
