/*
** This is a 'morf' ECR that implements the above function.
*/

#include "complex.h"

// ------------

void main(ImagPt *Zold, ImagPt *C, ImagPt *Znew);

void main(ImagPt *Zold, ImagPt *C, ImagPt *Znew)
{
	ImagPt temp1, temp2;
	
	SquareC( Zold, &temp1);        // temp1 = z^2
	MultC( Zold, &temp1, &temp2);  // temp2 = Z^3
	SubC( &temp2, &temp1, &temp2); // temp2 = Z^3 - Z^2
	AddC( &temp2, C, Znew);        // Znew = Z^3 - Z^2 + C
} // z3-z2+c