#include "complex.h"
#include "morph.h"
#include "default.h"

/*****************************************
** default.c
**
** this module has the default calculation
** routine to use.  This is so if resources
** can't be retrieved for some reason, we can
** still operate to some extent.
******************************************/


void Default(ImagPt *Zold, ImagPt *C, ImagPt *Znew)
{
	ImagPt temp;
	// calculate z^2-c  -- the Mandelbrot equation
	SquareC(Zold, &temp);
	SubC( &temp, C, Znew);
}