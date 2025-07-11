/* z^3 + z^2 + z +c
**
** another morf code resource.
**
** brent burton, 9/5/92
**/

#include "complex.h"

// ------------

void main(ImagPt *Zold, ImagPt *C, ImagPt *Znew);

void main(ImagPt *Zold, ImagPt *C, ImagPt *Znew)
{
	ImagPt temp1, temp2;
	
	SquareC( Zold, &temp1);			// temp1 = z^2
	MultC(Zold, &temp1, &temp2);		// temp2 = z^3
	AddC( &temp2, &temp1, &temp2);		// temp2 = z^3 + z^2
	AddC( &temp2, Zold, &temp2);		// temp2 = z^3 + z^2 + z
	AddC( &temp2, C, Znew);				// Znew = temp2 + C
} // main()
