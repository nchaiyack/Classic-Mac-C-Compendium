/*
LogLikelihood.c
Copyright (c) 1990 Denis G. Pelli
4/7/90 
Calculate log likelihood of the data for a given psychometric function.
If the psychometric function indicates that the parameters are out of bounds,
then LogLikelihood() returns the lowest possible log likelihood, minus infinity. 
If your machine doesn't deal in infinities, then replace 1.0/0.0 by DBL_MAX in
the definition of INF.

The third argument is a pointer to a psychometric function, e.g. Weibull.c

HISTORY:
8/24/91 dgp cosmetic
*/
#include "Quick3.h"

#define INF (1.0/0.0)

double LogLikelihood(dataRecord *data,paramRecord *params,
	PsychometricFunctionPtr PsychFun)
{
	double LL,p;
	long n;
	int i;
	contrastRecord *cPtr;

	LL=0.0;
	for(i=0;i<data->contrasts;i++){
		cPtr=&data->c[i];
		p = (*PsychFun)(cPtr->contrast, params);
		if(p==ILLEGAL_PARAMETERS)return -INF;
		n=cPtr->correct;
		if(n>0L) LL += n*log(p);
		n=cPtr->trials - cPtr->correct;
		if(n>0L) LL += n*log(1.0-p);
	}
	return LL;
}

