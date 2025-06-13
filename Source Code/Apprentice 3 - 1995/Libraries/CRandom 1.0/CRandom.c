/*
 * CRandom.c
 * A handy little random-number generator.  Just drop it in your project,
 * include the header, and call "x = gRandom->linearShort(low, high)".
 * Version 1.1.1, 14 April 1992
 *
 * by Jamie McCarthy
 * © Copyright 1992 by James R. McCarthy.  All rights reserved.
 * This code can be both distributed and used freely.
 * Internet: k044477@kzoo.edu			AppleLink: j.mccarthy
 * Telephone:  800-421-4157 or US 616-665-7075 (9:00-5:00 Eastern time)
 * I'm releasing this code with the hope that someone will get something
 * out of it.  Feedback of any sort, even just letting me know that you're
 * using it, is greatly appreciated!
 *
 * So far, the only implemented method to get random numbers is
 * linearShort(), which will return a signed 16-bit value between two
 * numbers you specify, with equal probability of each number occurring.
 * I've been meaning to write spinoffs of linearShort(), like
 * poissonShort() and bellCurveShort(), but haven't gotten around to it.
 * If you'd like to see them (or if you make one), drop me a line.
 *
 * Plugging a given value into seed() will always produce the same series.
 * The object is seeded automatically when it's initialized.
 *
 * The algorithm is straight out of Numerical Recipes, translated from
 * the original Fortran version.  I've made it as clear as possible, and
 * kept it reasonably fast without dropping into assembly.
 *
 * I haven't needed a 32-bit routine yet, so I haven't written one.
 * Again, if you want one or have written one, drop me a line.
 *
 * The randomize...Array() methods are spinoffs of linearShort that I
 * threw in for fun.  They scramble up the elements of an array, and
 * should be pretty self-explanatory.  Some people on comp.programming
 * assure me that this algorithm is the proper one.
 *
 * CHANGE HISTORY:
 *
 * From version 1.1 to 1.1.1:
 * Changed a few comments, and added a register declaration.
 *
 * From version 1.0 to 1.1:
 * Added "extern CRandom *gRandom" to the header file.
 *
 */



/******************************/

#include "CRandom.h"

/******************************/

#define MULTIPLICAND (4096)
#define MULTIPLY(x) ((x) << 12)

#define ADDEND (150889)
#define ADD(x) ((x) + ADDEND)

	/* this probably isn't really a word, but what the hell */
#define MODULEND (NDISTINCTRANDOMVALUES)
#define MODULO(x) ((x) % MODULEND)

#define NEXTVAL(x) (MODULO(ADD(MULTIPLY(x))))

/******************************/

CRandom *gRandom;

/******************************/



void CRandom::IRandom(void)
{
	unsigned long theDateTime;
	GetDateTime(&theDateTime);
	seed(theDateTime ^ TickCount());
}



void CRandom::seed(unsigned long seedVal)
{
	register short j;
	
	dummy = MODULO(Abs(ADDEND - seedVal));
	for (j = NRANDOMSLOTS-1; j >= 0; j--) {
		seeds[j] = dummy = NEXTVAL(dummy);
	}
	value = dummy = NEXTVAL(dummy);
}



short CRandom::linearShort(register short lowest, short highest)
{
	register long interval = highest - lowest + 1;
	register long partitionSize;
	register short j;
	
	if (interval <= 0) return lowest;
	partitionSize = (NDISTINCTRANDOMVALUES-1)/interval;
	while (TRUE) {
		j = (NRANDOMSLOTS*value) / NDISTINCTRANDOMVALUES;
		value = seeds[j];
		seeds[j] = dummy = NEXTVAL(dummy);
		if (value < partitionSize * interval) {
			return lowest + value/partitionSize;
		}
	}
}



void CRandom::randomizeArray(register short nElements, register Ptr theArray,
	register short elementSize)
{
	register short i, j;
	register long sourceOffset;
	
	sourceOffset = 0;
	
	for (i = 0; i < nElements-1; i++) {
		short swapWith;
		register char temp;
		register long destOffset;
		
		swapWith = linearShort(i, nElements-1);
		destOffset = swapWith * elementSize;
		
		for (j = 0; j < elementSize; j++) {
			temp = theArray[destOffset + j];
			theArray[destOffset + j] = theArray[sourceOffset + j];
			theArray[sourceOffset + j] = temp;
		}
		
		sourceOffset += elementSize;
	}
}



void CRandom::randomizeShortArray(short nElements, short *theArray)
{
	randomizeArray(nElements, (Ptr) theArray, 2);
}



void CRandom::randomizeLongArray(short nElements, long *theArray)
{
	randomizeArray(nElements, (Ptr) theArray, 4);
}



