/*----------------------------------------------------------------------------

	qsort.c

	fastQSort() is a replacement for qsort().  Compared with the Think C library
	version of qsort, fastQSort is about 3 times faster (it takes 1/3 the time), 
	and its speed isn't dependent on the data being sorted.
	
	One problem with qsort is that when the data is not in random order -- for
	example when it's already ordered, in reverse order, or almost sorted -- then 
	qsort exhibits worst case behavior of N*N/2 operations.  For N=1024 this can 
	take about 30 seconds, instead of the usual 0.8 seconds.  The fastQSort 
	algorithm doesn't have this problem, because it randomly selects the pivot
	element, and so for N=1024 it requires about 0.26 secs +/- 0.02 secs.
	
	Also, because fastQSort doesn't exhibit worst case behavior, it doesn't 
	require as much stack space, even though it has 12 bytes more of local
	variables.
	
	The things that make fastQSort so much faster than qsort are:
	
	1)	fastQSort picks the pivot element randomly, so it doesn't display worst
		case behavior.
		
	2)	fastQSort uses pointers and pointer arithmetic, avoiding multiplication
		whenever possible.
	
	3)	qsort uses std_swap() to swap the data between two locations, and 
		std_swap loops through the data 3 times to perform the exchange!  In
		comparison, swapMem() loops through the data just once.
		
	4)	fastQSort uses register variables whenever it makes good sense to do so.
	
	
	Researched and written by:
	
		Haydn Huntley
		Haydn's Consulting
		203 West Stone
		Fairfield, Iowa  52556
		(515) 472-7025
		
	During the school year, I may be reached at the following address:
	
		Haydn Huntley
		Eigenmann Hall #289
		Indiana University
		Bloomington, IN  47406
		(812) 857-8620
		
	E-mail:  huntley@copper.ucs.indiana.edu
----------------------------------------------------------------------------*/

#include <stdlib.h>

#include "glob.h"
#include "qsort.h"


#define	PIVOT_CUTOFF	25
#define	SEED			TickCount ()
#define FASTER			1

static size_t	gSize;
static short		(*gCmpFn) (void *, void *);

static void memSwap (register char *p, register char *q, 
	register size_t qSize)
{
	register char tmp;
	
	while (qSize--) {
		tmp = *p;
		*p++ = *q;
		*q++ = tmp;
	}
}

#if FASTER
static void chooseMedian (void *pDest, void *pA, void *pB, void *pC)
{
	void	*pMedian;
	
	if (gCmpFn (pA, pB) > 0)						/* pA > pB, what is pC?			*/
		if (gCmpFn (pA, pC) > 0)					/* pA > pB, pA > pC				*/
			if (gCmpFn (pB, pC) > 0)
				pMedian = pB;						/* pA > pB > pC					*/
			else
				pMedian = pC;						/* pA > pC > pB					*/
		else
			pMedian = pA;							/* pC > pA > pB					*/
	else											/* pB > pA, what is pC?			*/
		if (gCmpFn (pA, pC) > 0)
			pMedian = pA;							/* pB > pA > pC					*/
		else										/* pB > pA, pC > pA				*/
			if (gCmpFn (pB, pC) > 0)
				pMedian = pC;						/* pB > pC > pA					*/
			else
				pMedian = pB;						/* pC > pB > pA					*/

	if (pDest != pMedian)
		memSwap (pDest, pMedian, gSize);
}
#endif /* FASTER */

static void doFastQSort (register char *first, char *last)
{
	register char	*i, *j;
	register size_t	n, qSize = gSize;
	
	/* first, last, i, and j are scaled by gSize in this function */
	 	
	while (!gCancel && (n = (last - first) / qSize) > 1)
	{
		#if FASTER
			if (n < PIVOT_CUTOFF)
				/* use a random pivot */
				memSwap (first + (rand () % n) * qSize, first, qSize);
			else
				/* use the median of first, middle, and a random volunteer */
				chooseMedian (first, first,
						first + (n / 2) * qSize,
						first + (rand () % n) * qSize);
		#else
			/* use a random pivot */
			memSwap (first + (rand () % n) * qSize, first, qSize);
		#endif /* FASTER */
		
		i = first;
		j = last;
		while (1)
		{
			i += qSize;
			while (i < last && gCmpFn (i, first) < 0)
				i += qSize;
				
			j -= qSize;
			while (j > first && gCmpFn (j, first) > 0)
				j -= qSize;
				
				
			if (i >= j)
 				break;
 				
			memSwap (i, j, qSize);
		}
		
		if (j == first)
		{
			first += qSize;
			continue;
		}
		memSwap (first, j, qSize);
		if (j - first < last - (j + qSize))
		{
			doFastQSort (first, j);
			
			first = j + qSize;
			/* equivalent to doFastQSort (j + qSize, last);		*/
			/* to save stack space								*/
		}
		else
		{
			doFastQSort (j + qSize, last);
			
			last = j;
			/* equivalent to doFastQSort (first, j);			*/
			/* to save stack space								*/
		}
	}
}

Boolean FastQSort (void *base, size_t n, size_t size, 
	short (*cmpFn) (void *,void *))
{
	/* setup the global variables used by fastQSort() */
	gSize = size;
	gCmpFn = cmpFn;
	srand (SEED);
		
	/* the args for fastQSort () must be scaled by gSize */
	doFastQSort (base, (char *) base + n * gSize);
	
	return !gCancel;
}
