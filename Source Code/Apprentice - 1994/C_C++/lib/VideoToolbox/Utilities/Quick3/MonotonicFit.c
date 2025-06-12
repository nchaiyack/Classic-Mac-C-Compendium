/*
MonotonicFit.c
Copyright (c) 1979,1989,1990 Denis G. Pelli 

FIND MAXIMUM LIKELIHOOD MONOTONIC FIT (MLMF)
Given the number of right and wrong responses at each of several
contrasts, this routine finds the maximum likelihood monotonic
psychometric function that could have given rise to the data.

The original dataRecord is OVERWRITTEN and replaced by the "collapsed"
data which represent the maximum likelihood monotonic fit.

The log likelihood and degrees of freedom are returned, using the
user-supplied points. The degrees of freedom is equal to the number of
points in the maximum likelihood monotonic fit (after collapsing), 
minus 1 if both limiting probabilities (0 and 1) are included.

THEOREM: The MLMF when two successive points have a nonmonotonic
proportion correct is equivalent to the MLMF to collapsed data where
the two points have been combined into one. "Equivalent" means that
the two MLMFs will be equal at all points other than the two in
question, and the original MLMF will have the same value at both of
the two nonmonotonic points as the equiv. MLMF will have at the
combined point. Repeated application of the theorem eventually gives
collapsed data with monotonically increasing proportion correct. The
MLMF is equal to these proportion correct.

The principal use of the MLMF routine is to deal with the problem of assessing
the goodness of a function fit (e.g. Weibull) to data
collected by a staircase that has put different numbers of trials at
each contrast. Clearly the unconstrained fit is sort of silly for this case, 
especially when there are many
contrasts with only single trials, and it's not clear how to count
its degrees of freedom. (One contrast may have 200 trials, another may
have one trial. Do they each count as a degree of freedom? No good answer.)
The MLMF finds the maximum likelihood monotonic
fit, (and along the way determines the number of degrees of freedom), 
and in practice makes a good null hypothesis for
assessing the Weibull fit. What's cute about MLMF is that it is fast.
In time proportional to the number of contrasts it computes the 
monotonic fit directly. I worked this out for myself while in graduate 
school, but Misha Pavel tells me that there is actually a whole literature
on this topic.
HISTORY:
29-JUL-79	dgp	wrote it in FORTRAN
8/9/89 		dgp	translated it to C, but but didn't test it.
4/5/90		dgp	debugged.
4/7/90		dgp	added qsort().
10/29/90	dgp	tidied up the comments.
*/
#include "VideoToolbox.h"
#include "Quick3.h"

void MonotonicFit(dataRecord *dataPtr,double *logLikelihoodPtr,int *degreesOfFreedomPtr)
{
	enum{END=-1,DELETED=-2};
	int head;
	long y[MAX_CONTRASTS];		/* long to allow huge number of trials per point */
	long t[MAX_CONTRASTS];
	long n;
	double p[MAX_CONTRASTS];	/* the values of the MLMF psychometric function. */
	int next[MAX_CONTRASTS];
	int flag;
	int i,j,jj;

	/*
	Sort the contrastRecords in order of increasing contrast
	& merge data at equal contrasts.
	*/
	SortAndMergeContrasts(dataPtr);

	/* Load up the arrays. next[] is a linked list. */
	head=0;
	for(i=0;i<dataPtr->contrasts;i++){
		y[i]=dataPtr->c[i].correct;
		t[i]=dataPtr->c[i].trials;
		p[i]=y[i]/(double)t[i];
		next[i]=i+1;
	}
	next[i-1]=END;
	/*
	Assume the data are in order of increasing contrast.
	Now repetitively apply the theorem.
	Each time a nonmonotonic pair of points is found,
	they are replaced by a single point with the sum of the
	data at the two points.
	Successive passes are made through the linked list until
	no nonmonotonicities are found (i.e. flag==FALSE).
	*/
	flag=TRUE;
	while(flag){
		flag=FALSE;
		j=head;
		for(i=0;i<dataPtr->contrasts;i++){
			jj=next[j];
			if(jj==END)break;
			if(t[j]==0 || t[jj]==0 || p[j] > p[jj]) {
				/* these two points are nonmonotonic, so merge them */
				/* Points with zero trials should be merged as well */
			    flag=TRUE;							
			    y[j] += y[jj];
			    t[j] += t[jj];
			    p[j]=y[j]/(double)t[j];
			    next[j]=next[jj];
			    next[jj]=DELETED;				/* mark the deleted point */
			    jj=j;
			}
			j=jj;
		}
	}

	/* 
	Assign probabilities to the deleted points.
	*/
	for(i=1;i<dataPtr->contrasts;i++){
		if(next[i]==DELETED) p[i]=p[i-1];
	}

	/* 
	Count the number of degrees of freedom.
	*/
	*degreesOfFreedomPtr=0;
	for(i=0;i<dataPtr->contrasts;i++){
		if(next[i]!=DELETED) (*degreesOfFreedomPtr)++;
	}
	if(p[0]==0.0 && p[dataPtr->contrasts-1] == 1.0) (*degreesOfFreedomPtr)--;

	/*
	Compute the log likelihood
	*/
	*logLikelihoodPtr=0.0;
	for(i=0;i<dataPtr->contrasts;i++){
		n=dataPtr->c[i].correct;
		if(n>0) *logLikelihoodPtr +=n*log(p[i]);
		n=dataPtr->c[i].trials-dataPtr->c[i].correct;
		if(n>0) *logLikelihoodPtr +=n*log(1.0-p[i]);
	}

	/*
	To return the results, OVERWRITE the dataRecord with
	the maximum likelihood monotonic fit, where p=correct/trials.
	*/
	for(i=0;i<dataPtr->contrasts;i++){
		if(next[i]!=DELETED) {
			dataPtr->c[i].correct=y[i];
			dataPtr->c[i].trials=t[i];
		}else{
			dataPtr->c[i].correct=dataPtr->c[i-1].correct;
			dataPtr->c[i].trials=dataPtr->c[i-1].trials;
		}
	}
}

