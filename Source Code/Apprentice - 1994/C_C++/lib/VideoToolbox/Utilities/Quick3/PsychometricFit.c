/* 
PsychometricFit.c
Copyright 1990 (c) Denis G. Pelli
A general-purpose function that does a maximum likelihood fit of any
psychometric function to psychometric data. The returned value is the level of
significance at which the fit can be rejected. The degreesOfFreedom may be zero,
in which case no parameters will be adjusted, but you'll get the log likelihood
and significance of the supplied parameter values.

The psychometric function (which you supply as an argument) takes two arguments:
a contrast and a pointer to a paramRecord. The function Weibull() is provided in
Weibull.c. Others may be added as desired. It is assumed that the first
parameter is the log of threshold. No assumptions are made about the other
parameters, except that any that are to be iteratively fit are assumed to be of
type "double".

Quick3 is a stand-alone program that uses PsychometricFit() to do the real work.
I suggest you read the introductory comments at the beginning of Quick3.c

HISTORY:
4/7/90		dgp wrote it
10/29/90	dgp	tidied up the comments
11/17/92	dgp "
1/25/93 dgp removed obsolete support for THINK C 4.

SOURCES:
Quick3.h
LogLikelihood.c
MonotonicFit.c
PsychometricFit.c
SortAndMergeContrasts.c
Weibull.c
#From Denis Pelli's VideoToolbox:
VideoToolbox.h
Binomial.c
ChiSquare.c
Normal.c
SetFileInfo.c		# Used only on the Macintosh
#From Numerical Recipes in C:
nr.h
NRUTIL.h
BRENT.C
F1DIM.C
LINMIN.C
MNBRAK.C
NRUTIL.C
POWELL.C

LIMITATIONS

This program uses routines from Numerical Recipes in C. They're copyrighted, 
so I can't distribute them. You can order the software:
	Numerical Recipes C Diskette for Macintosh $29.95
and book:
	Numerical Recipes in C: The Art of Scientific Computing $44.50
from:
	Cambridge University Press
	Order Department
	510 North Avenue
	New Rochelle, NY 10801

Note that I have made several changes to the Numerical Recipes in C routines: 
1.In every file I changed "float" to "FLOAT", and #included nr.h. I inserted the 
statement "typedef double FLOAT;" in the file nr.h. This is because the 
Macintosh computes doubles much faster than floats. If you'd rather run
slowly than modify your Numerical Recipes in C files, then you will need to insert 
"typedef float FLOAT;"
in CalibrateLuminance.c & PsychometricFit.c in order to compile those files.
The rest of the VideoToolbox doesn't care.
*/
#include "VideoToolbox.h"
#include "Quick3.h"
#include <nr.h>
#include <nrutil.h>

#define TOLERANCE 0.001	/* fractional tolerance of log likelihood. Not critical. */

/*
I hate global variables because they hide the flow of information. However,
some sort of cludge is necessary to pass the extra arguments to Error(), bypassing
the Numerical Recipes routines that call it, since they only pass the 
parameters they know about. These static declations at least restrict the scope of
these "globals" to this file.
*/
static double Error(FLOAT *p);
static dataRecord *myDataPtr;				/* for Error() */
static PsychometricFunctionPtr MyPsychFun;	/* for Error() */
static paramRecord myParams;				/* for Error() */
static int myDegreesOfFreedom;				/* for Error() */
static int iter;							/* for Error() */

double PsychometricFit(paramRecord *paramPtr,PsychometricFunctionPtr PsychFun
	,dataRecord *dataPtr,double *logLikelihoodPtr,int degreesOfFreedom
	,double *chiSquarePtr,int *chiSquareDFPtr)
{
	int i,j;
	FLOAT *p,**direction,ftol,fret;
	dataRecord monotonicData;
	double monotonicLL;
	int monotonicDF;
	double P;
	
	myDataPtr=dataPtr;		/* copy these for use by Error() */
	MyPsychFun=PsychFun;
	myParams=*paramPtr;
	myDegreesOfFreedom=degreesOfFreedom;
	
	p=vector(1,degreesOfFreedom);
	direction=matrix(1,degreesOfFreedom,1,degreesOfFreedom);	/* initial directions */
	if(p==NULL || direction == NULL)
		PrintfExit("PsychometricFit: not enough room for arrays.\007\n");
	for(i=1;i<=degreesOfFreedom;i++) p[i]=((double *)paramPtr)[i-1];
	for(i=1;i<=degreesOfFreedom;i++)for(j=1;j<=degreesOfFreedom;j++)direction[i][j]=0.0;
	for(i=1;i<=degreesOfFreedom;i++)direction[i][i]=0.03;	/* initial step size */
	ftol=TOLERANCE;	/* fractional tolerance on Error value when done */
	iter=0;
	
	/* do it. The psychometric function is passed to Error by the global MyPsychFun */
	if(degreesOfFreedom==0)fret=Error(p);
	else powell(p,direction,degreesOfFreedom,ftol,&iter,&fret,&Error);

	for(i=1;i<=degreesOfFreedom;i++) ((double *)paramPtr)[i-1]=p[i];
	free_matrix(direction,1,degreesOfFreedom,1,degreesOfFreedom);
	free_vector(p,1,degreesOfFreedom);

	*logLikelihoodPtr=-fret;
	
	/* Now compute the degree of significance at which the fit can be rejected */
	monotonicData= *dataPtr;
	MonotonicFit(&monotonicData,&monotonicLL,&monotonicDF);	/* overwrites data with fit */
	*chiSquarePtr= -2.0*(*logLikelihoodPtr-monotonicLL);	/* -2 log likelihood ratio of hypotheses */
	*chiSquareDFPtr=monotonicDF-degreesOfFreedom;			/* difference in degrees of freedom */
	P=PChiSquare(*chiSquarePtr,*chiSquareDFPtr);			/* significance */
	return P;
}

/* There is a subtlety here. I thought that I could use Powell with the whole
paramRecord, yet ask Powell to only twiddle the first few parameters, figuring that
even when I was asking Powell to fit only the first few parameters the other
parameters would still be there in the array when the pointer to the array was
passed to Error(). Alas, Powell() and its subroutines make COPIES of the array,
and naturally fail to copy the non-twiddled parameters, since they don't know about
them. The solution is for Error() to have its own complete copy of the paramRecord.
Each time Error() is called it updates the twiddled parameters before calling
LogLikelihood(), which calls the psychometric function (*MyPsychFun)().
*/

double Error(FLOAT *p)
{
	double error;
	int i;
	static int lastIter=0;
	
	for(i=1;i<=myDegreesOfFreedom;i++) ((double *)&myParams)[i-1]=p[i];
	
	error=-LogLikelihood(myDataPtr,&myParams,MyPsychFun);
	
	/* Diagnostic printout for difficult cases */
	if(iter>0 && iter%50 == 0 && iter!=lastIter){
		printf("Error(): Warning, %d iterations:\n",iter);
		printf("logAlpha %5.2f, beta %5.1f, gamma %5.2f, delta %6.3f -log likelihood %9.0g\n"
			,myParams.logAlpha,myParams.beta,myParams.gamma,myParams.delta,error);
		lastIter=iter;
	}
	return error;
}

