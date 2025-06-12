/* Sane.c
These routines provide more convenient access to the Apple provided routines
that convert back and forth between 12-byte and 10-byte floating point formats.
The two formats contain exactly the same information. The 12-byte format, used
by the Motorola floating point chips, has an unused two byte gap between the
exponent and mantissa.

These routines used to be in the IsNan.c file, but I found that I never used
them, and I didn't like the fact that they force me to include the header
SANE.h, so I put them here. Users of THINK C version 5 (or later) won't need to
do these conversions if they use THINK's "universal" floating point format, which
is compatible with BOTH the SANE library AND the Motorola chip.

Note that these prototypes are NOT included in VideoToolbox.h, since I did not
want to include SANE.h there. Including SANE.h in a file has side effects that I
wouldn't want to impose on all users of VideoToolbox.h.

Warning to MATLAB users: This file does NOT #include VideoToolbox.h

HISTORY:
12/91	dgp	wrote it.
*/
#include <SANE.h>
extended DoubleToExtended(double x);
double ExtendedToDouble(extended x80);
numclass ClassDouble(double x);

extended DoubleToExtended(double x)
{
	extended x80;
	
	if(sizeof(double)==sizeof(extended)) return *(extended *)&x;
	else {
		x96tox80((extended96 *)&x,&x80);
		return x80;
	}
}

double ExtendedToDouble(extended x80)
{
	double x;

	if(sizeof(double)==sizeof(extended)) return *(double *)&x80;
	else {
		x80tox96(&x80,(extended96 *)&x);
		return x;
	}
}

numclass ClassDouble(double x)
{
	return classdouble(DoubleToExtended(x));
}

