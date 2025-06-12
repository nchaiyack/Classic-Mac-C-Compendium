/*
TestPsychometricFit.c
Copyright (c) 1990-1992 Denis G. Pelli
This is a simple driver to show that PsychometricFit.c works.

I use the Weibull psychometric function to provide the probability at each contrast
and the Numerical Recipes bnldev() binomial deviates function to simulate the appropriate
number of trials at that contrast.

Then I call PsychometricFit() and ask it to fit the Weibull() function to the data, and
then I print out the results. As one would expect, the fits are very good. It seems
reasonable to reject fits at the 5% significance level. So expect to reject about
5% of your fits even if all the assumptions of this model are correct.

Optionally, also compares QUICK3 with QUEST. (Note: QUEST is not part of the 
VideoToolbox. Sorry.)

HISTORY:
4/6/90	dgp	wrote it. Seems to work fine for all cases, 1 to 100,000,000 trials per
			contrast, and few and many contrasts.
11/18/92 dgp added comparison with QUEST.
2/20/93	dgp	added call to Require().
*/
#include "VideoToolbox.h"
#define QUEST 0
#if QUEST
	#include "Quest.h"
#endif
#include "Quick3.h"
#include <assert.h>
#include <time.h>
#include <nr.h>		/* Numerical Recipes in C */

void main(void)
{
	dataRecord data,monotonicData;
	contrastRecord *cPtr;
	paramRecord params;
	int i;
	double chiSquare,weibullLL,monotonicLL;	/* log likelihood */
	int chiSquareDF,weibullDF,monotonicDF;	/* degrees of freedom */
	double p,logC,range;
	long idum;
	int j,cond=0;
	#if QUEST
		Quest *q;
	#endif
	double mode;
	
	Require(0);
	params.logAlpha=0;
	params.beta=3;
	params.gamma=0.111173; 
	params.delta=0.01;
	
	idum=time(NULL);
	printf("Random seed %d\n",idum);	/* So we can reproduce interesting cases */
	data.contrasts=5;
	range=0.5;							/* log contrast range, centered on logAlpha */
	range=2;
	for(i=0;i<data.contrasts;i++){
		logC=params.logAlpha + range*(i/(data.contrasts-1.0) - 0.5);
		data.c[i].contrast=pow(10.0,logC);
		data.c[i].trials=10;				/* trials at this contrast */
		data.c[i].correct=bnldev(Weibull(data.c[i].contrast,&params),data.c[i].trials,&idum);
	}
	#if QUEST
		/* Quest Parameters */
		q=(Quest *)malloc(sizeof(Quest));
		assert(q!=NULL);
		q->nConds=1;	
		q->nLevels=600;
		q->nTrials=0;
		q->grain=0.01;	/* step size of grid, in log contrast */
		q->initialSD=1;
	 	q->nResponses=2;
	 	q->quantileOrder=NAN;
	 	q->fakeIt=0;
	 	q->function=WeibullPResponse;
		q->beta=params.beta;
		q->gamma=params.gamma;
		q->delta=params.delta;
		q->epsilon=0.0;
		cond=0;
		q->guess[cond]=0;
		QuestOpen(q);
		for(i=0;i<data.contrasts;i++){
			logC=log10(data.c[i].contrast);
			for(j=0;j<data.c[i].correct;j++)QuestUpdate(q,cond,logC,1);
			for(;j<data.c[i].trials;j++)QuestUpdate(q,cond,logC,0);
		}
		q_removePrior(q->qConds[cond]);
		mode = q_mode(q->qConds[cond]);
		QuestClose(q);
	#endif
	printf("Testing the function PsychometricFit.\n");
	printf("Simulating an observer with a Weibull psychometric function.\n");
	weibullDF=1;	/* number of parameters to be adjusted in fitting */
	printf("The simulated data will be fit using %d degrees of freedom.\n",weibullDF);
	printf("Observer: ");
	printf("logAlpha%6.2f, beta%4.1f, gamma%5.2f, delta%5.2f\n",params.logAlpha,params.beta,params.gamma,params.delta);
	p=PsychometricFit(&params,&Weibull,&data,&weibullLL,weibullDF,&chiSquare,&chiSquareDF);
	printf("Fit:      ");
	printf("logAlpha%6.2f, beta%4.1f, gamma%5.2f, delta%5.2f, significance%5.2f\n",params.logAlpha,params.beta,params.gamma,params.delta,p);
	#if QUEST
		printf("QUEST mode %.2f\n",mode);
	#endif

	/*
	We're done, but just to show off, let's print out everything that anyone
	could possibly want. In real life I would skip this junk.
	*/
	monotonicData=data;
	MonotonicFit(&monotonicData,&monotonicLL,&monotonicDF);	/* overwrites data with fit */
	printf("\ncontrast Trials Right  Ratio  Weibull Monotone\n");
	for(i=0;i<data.contrasts;i++){
		cPtr=&data.c[i];
		printf("%6.3f   %5ld %5ld %7.3f %7.3f %7.3f\n",
			cPtr->contrast,cPtr->trials,cPtr->correct,
			cPtr->correct/(double)cPtr->trials,
			Weibull(cPtr->contrast,&params),
			monotonicData.c[i].correct/(double)monotonicData.c[i].trials
			);
	}

	chiSquare=2.0*(monotonicLL-weibullLL);
	chiSquareDF=monotonicDF-weibullDF;
	p=PChiSquare(chiSquare,chiSquareDF);
	printf("\nChi square %.1f with %d degrees of freedom, yielding a significance of %.2f\n",chiSquare,chiSquareDF,p);
}