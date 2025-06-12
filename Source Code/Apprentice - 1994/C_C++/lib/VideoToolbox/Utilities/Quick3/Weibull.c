/*
Weibull.c
Copyright (c) 1990, 1991, 1992 Denis G. Pelli 
A psychometric function for use by PsychometricFit() and Quick3.
Computes a Weibull function. E.g.
alpha=0.01; threshold
beta=3.5;	steepness
gamma=0.5;	guessing rate
delta=0.0;	rate of random guesses at high, suprathreshold contrasts
P=Weibull(contrast,&myParamRecord);
Note: Weibull's prototype must be consistent with PsychometricPtr in Quick3.h

HISTORY:
Fall'89		dgp	wrote it, but didn't use it.
4/8/90		dgp	polished it. Added ILLEGAL_PARAMETERS return value to deal gracefully
				with the fact that general purpose minimization routines will quite 
				naturally try illegal values of the parameters and that the appropriate 
				response is that such parameters produce a very bad fit, inducing the 
				minimization program to stay within legal values of the parameters.
10/29/90	dgp	Tidied up comments.
3/11/92		dgp For speed I changed pow(10,...) to exp10(...), which is defined in
				Quick3.h.
*/
#include "Quick3.h"

double Weibull(double contrast,paramRecord *paramPtr)
{
	double p,exponent,alpha,beta,gamma,delta;

	alpha = exp10(paramPtr->logAlpha);
	beta=paramPtr->beta;
	gamma=paramPtr->gamma;
	delta=paramPtr->delta;
	if(beta<0.0 || gamma<0.0 || gamma>1.0 ||
		delta<0.0 || delta>1.0) return ILLEGAL_PARAMETERS;
	contrast /= alpha;
	exponent = pow(contrast,beta);
	if ( exponent > 30.0 )
		p = 1.0;	/* This avoids floating underflow */
	else
		p = 1.0 - (1.0 - gamma) * exp(-exponent);
	p = (1.0-delta)*p + delta*gamma; 
	return p;
}
