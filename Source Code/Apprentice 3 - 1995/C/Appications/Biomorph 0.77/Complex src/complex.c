/****************************************************
*                                                   *
* File:  complex.c -- routines for complex numbers. *
*                                                   *
****************************************************/

#include "complex.h"

#define ar A->r       /* just to make math translation easier... */
#define ai A->i
#define br B->r
#define bi B->i

void AddC(ImagPt *A, ImagPt *B, ImagPt *result)
{
	result->r = ar + br;
	result->i = ai + bi;
	result->mag = result->i * result->i + result->r * result->r;
	return;
}

void SubC(ImagPt *A, ImagPt *B, ImagPt *result)  /* returns A-B */
{
	result->r = ar -br;
	result->i = ai-bi;
	result->mag = result->i*result->i + result->r * result->r;
	return;
}

void MultC(ImagPt *A, ImagPt *B, ImagPt *result)
{
	result ->r = (ar * br) - (ai * bi);
	result->i  = (ar * bi) + (br * ai);
	result->mag = result->i*result->i + result->r * result->r;
	return;
}

void SquareC(ImagPt *A, ImagPt *result)
{
	MultC( A, A, result);
	return;
}

void CubeC(ImagPt *A, ImagPt *result)
{
	ImagPt temp;
	
	MultC( A, A, &temp);
	MultC( A, &temp, result);
	return;
}
