/*
 * CRandom.h
 * Version 1.1.1, 14 April 1992
 *
 * © Copyright 1992 by James R. McCarthy.  All rights reserved.
 * This code can be both distributed and used freely.
 *
 */



/******************************/

#pragma once

/******************************/

#define NRANDOMSLOTS (97)
#define NDISTINCTRANDOMVALUES (714025)

/******************************/

#include <Global.h>
#include <oops.h>

/******************************/



struct CRandom : indirect {
	unsigned long	seeds[NRANDOMSLOTS];
	unsigned long	dummy;
	unsigned long	value;
	
	void			IRandom(void);
	
	void			seed(unsigned long seedVal);
	
	short			linearShort(short lowest, short highest);
	
	void			randomizeArray(short nElements, Ptr theArray, short elementSize);
	void			randomizeShortArray(short nElements, short *theArray);
	void			randomizeLongArray(short nElements, long *theArray);
} ;

extern CRandom *gRandom;
