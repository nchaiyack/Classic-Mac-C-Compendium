/*
Shuffle.c
Like the Standard C function qsort(), Shuffle reorders the elements of
an array, but Shuffle produces a randomized sequence.

HISTORY:
3/19/94	dgp	rewrote it to accept an arbitrary element size. It probably now
runs ten times slower than when it only accepted shorts. If the loss of speed
matters to anybody, let me know, as it would be easy to add fast special-case
code for a few popular element sizes, e.g. sizeof(short).
*/
#include "VideoToolbox.h"
#include <assert.h>

void Shuffle(void *array,long elements,size_t elementSize)
{
	long i,j;
	void *scratch;
	char workArea[256],useMalloc;
	
	assert(sizeof(char)==1);
	useMalloc=sizeof(workArea)<elementSize;
	if(useMalloc){
		scratch=malloc(elementSize);
		assert(scratch!=NULL);
	}else scratch=workArea;
	for(i=0;i<elements;i++){
		j=nrand(elements);
		// swap i-th and j-th elements
		memcpy(scratch,(char *)array+j*elementSize,elementSize);
		memcpy((char *)array+j*elementSize,(char *)array+i*elementSize,elementSize);
		memcpy((char *)array+i*elementSize,scratch,elementSize);
	}
	if(useMalloc)free(scratch);
}
