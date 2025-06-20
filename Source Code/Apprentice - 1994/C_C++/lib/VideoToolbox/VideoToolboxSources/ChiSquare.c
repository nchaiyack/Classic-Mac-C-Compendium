/*
ChiSquare.c
Copyright 1990-1992 � Denis G. Pelli
PChiSquare() calculates the significance level at which a fit may be rejected.

Also see: Binomial.c, Exponential.c, Normal.c, Uniform.c

HISTORY:
1/7/84	dgp	wrote it in FORTRAN
8/9/89	dgp	translated it to C
4/5/90	dgp	tested it against tables in Abramowitz in Stegun. Works fine.
4/8/90	dgp	renamed file ChiSquare.c from PChiSquare.c
7/30/91	dgp	now use NAN defined in VideoToolbox.h
1/12/92	dgp	Renamed NormalPDF() to NormalPdf().
*/
#include "VideoToolbox.h"
#include <math.h>

double PChiSquare (double chiSquare,int n)
/*
Returns one minus the cumulative probability of the
Chi-Square distribution for value chisq with n degrees of freedom.
The formula is from Abramowitz and Stegun, Eqs. 26.4.4 and 26.4.5, pg 941.
The formula is exact.
Range: chisq>=0.0, n>=0,
Returns zero if n is zero.

M. Abramowitz and I.A. Stegun (1964) Handbook of mathematical functions. Dover.
*/
{
	double x,a,P;
	int i,ii;
	
	if(chiSquare<0.0 || n<0) return NAN;	/* return NAN to signal error */
	x = sqrt(chiSquare);
	if(n%2 != 0) {
		/* n is odd */
		P = 2.0*Normal(-x);
		a = 2.0*NormalPdf(x)*x;
		ii = (n-1)/2;
		for(i=1;i<=ii;i++){
			P += a;
			a *= chiSquare/(2*i+1);
		}
	}
	else {
		/* n is even */
		P = 0.0;
		a = sqrt(2.0*PI)*NormalPdf(x);
		ii = n/2;
		for( i=1;i<=ii;i++){
			P += a;
			a *= chiSquare/(2*i);
		}
	}
	return P;
}

