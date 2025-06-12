/* Quick3.h
HISTORY:
3/10/92 dgp	Cast PARAMS to be a short int.
			If appropriate, use the 68881 log and exp instructions for speed.
			For speed, defined exp10(), and use exp and log to compute pow.
			These changes speed up the computation enormously, perhaps 50-fold,
			yet when run on "sample.data" the resulting "sample.fit" is unchanged.
*/
#pragma once	/* prevent multiple inclusions of this file */
#include <math.h>
#include <stdio.h>	/* needed for prototypes */
#include <stdlib.h>
#define MAX_CONTRASTS 200
#define ILLEGAL_PARAMETERS -1.0	/* unique value indicating parameters out of bounds */
#define MACINTOSH 1
#include "mc68881.h"
#if THINK_C && mc68881
	#define exp _exp	/* use fast 68881 instruction instead of SANE */
	#define log _log	/* use fast 68881 instruction instead of SANE */
#endif
#ifndef exp10
#define exp10(x) exp(LOG10*(x))			/* faster than pow(10.0,x) */
#endif
#define pow(x,y) exp(log(x)*(y))		/* faster by use of 68881 instructions */
#if !defined(LOG10)
	#define LOG10	2.30258509299404568402	/* computed in Mathematica */
#endif

typedef struct {
	double contrast;
	long trials;
	long correct;
} contrastRecord;

typedef struct {
	int contrasts;
	contrastRecord c[MAX_CONTRASTS];	/* an array of records is easier to sort */
} dataRecord;

typedef struct {
	double logAlpha;
	double beta;
	double gamma;
	double delta;
} paramRecord;

#define PARAMS ((short)(sizeof(paramRecord)/sizeof(double)))

typedef double (*PsychometricFunctionPtr)(double contrast,paramRecord *paramPtr);

double Weibull(double contrast,paramRecord *paramPtr);
double LogLikelihood(dataRecord *data,paramRecord *params,
	PsychometricFunctionPtr PsychFun);
double PsychometricFit(paramRecord *paramPtr
	,PsychometricFunctionPtr PsychFun
	,dataRecord *dataPtr,double *logLikelihoodPtr,int degreesOfFreedom
	,double *chiSquarePtr,int *chiSquareDFPtr);
void MonotonicFit(dataRecord *data,double *logLikelihoodPtr,int *degreesOfFreedomPtr);
void SortAndMergeContrasts(dataRecord *dataPtr);
