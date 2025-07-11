/*
SortAndMergeContrasts.c
HISTORY:
8/24/91	dgp	Added typedef and cast to make it compatible with THINK C 5.0.
1/25/93 dgp removed obsolete support for THINK C 4.
*/
#include "Quick3.h"

typedef int (*qsort_cmp_func)(const void *,const void *);

static int CompareContrasts(contrastRecord *c1,contrastRecord *c2);

void SortAndMergeContrasts(dataRecord *dataPtr)
{
	int i,j;
	
	/* Sort the contrastRecords in order of increasing contrast. */
	qsort(dataPtr->c,dataPtr->contrasts,sizeof(contrastRecord)
		,(qsort_cmp_func)&CompareContrasts);
	
	/* Merge records at equal contrast. */
	i=j=0;
	for(i=0;i<dataPtr->contrasts-1;i++){
		if(dataPtr->c[i].contrast==dataPtr->c[i+1].contrast){
			dataPtr->c[i].trials += dataPtr->c[i+1].trials;
			dataPtr->c[i].correct += dataPtr->c[i+1].correct;
			for(j=i+1;j<dataPtr->contrasts-1;j++) dataPtr->c[j]=dataPtr->c[j+1];
			dataPtr->contrasts--;
		}
	}
}

static int CompareContrasts(contrastRecord *c1,contrastRecord *c2)
{
	if(c1->contrast >  c2->contrast) return 1;
	if(c1->contrast == c2->contrast) return 0;
	return -1;
}