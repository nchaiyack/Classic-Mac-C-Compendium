/*
VLambda.c
Copyright 1992 (c) Denis Pelli

VLambda(double nm) returns the photopic sensitivity of the standard
CIE observer, relative to the peak at 555 nm.

VLambdaPrime(double nm) returns the scotopic sensitivity of the standard
CIE observer, relative to the peak at 507 nm.

Both are based on the Wyszecki and Stiles tables. Intermediate values
are interpolated geometrically. Sensitivity at wavelengths outside
the range covered by the tables is assumed to be zero.

Based on Tables I(3.3.1) and I(4.3.2) in G. Wyszecki and W.S. Stiles (1982) 
Color Science, 2nd Ed., Wiley, New York.

HISTORY:
1990	dgp wrote it.
10/23/92 dgp sped it up by taking the log only once, the first time it's called.
12/13/92 dgp cosmetic editing of the code.
*/
#include "VideoToolbox.h"
#include <assert.h>
#include <math.h>
#include "mc68881.h"
#if THINK_C && mc68881		// use built-in fpu instructions for speed
	#define log _log
	#define exp _exp
#endif

double VLambda(double nm)
{
	/* at 5 nm intervals, from Table I(3.3.1) page 725,
	G. Wyszecki and W.S.Stiles (1982) Color Science,
	2nd Ed., Wiley, New York. 
	*/
	static double V[]={ 
		0.00000391700, 0.00000696500, 0.00001239000, 0.00002202000, 
		0.00003900000, 0.00006400000, 0.00012000000, 0.00021700000, 
		0.00039600000, 0.00064000000, 0.00121000000, 0.00218000000, 
		0.00400000000, 0.00730000000, 0.01160000000, 0.01684000000, 
		0.02300000000, 0.02980000000, 0.03800000000, 0.04800000000, 
		0.06000000000, 0.07390000000, 0.09098000000, 0.11260000000, 
		0.13902000000, 0.16930000000, 0.20802000000, 0.25860000000, 
		0.32300000000, 0.40730000000, 0.50300000000, 0.60820000000, 
		0.71000000000, 0.79320000000, 0.86200000000, 0.91485010000, 
		0.95400000000, 0.98030000000, 0.99495010000, 1.00000000000, 
		0.99500000000, 0.97860000000, 0.95200000000, 0.91540000000, 
		0.87000000000, 0.81630000000, 0.75700000000, 0.69490000000, 
		0.63100000000, 0.56680000000, 0.50300000000, 0.44120000000, 
		0.38100000000, 0.32100000000, 0.26500000000, 0.21700000000, 
		0.17500000000, 0.13820000000, 0.10700000000, 0.08160000000, 
		0.06100000000, 0.04458000000, 0.03200000000, 0.02320000000, 
		0.01700000000, 0.01192000000, 0.00821000000, 0.00572300000, 
		0.00410200000, 0.00292900000, 0.00209100000, 0.00148400000, 
		0.00104700000, 0.00074000000, 0.00052000000, 0.00036110000, 
		0.00024920000, 0.00017190000, 0.00012000000, 0.00008480000, 
		0.00006000000, 0.00004240000, 0.00003000000, 0.00002120000, 
		0.00001499000, 0.00001060000, 0.00000746570, 0.00000525780, 
		0.00000370290, 0.00000260780, 0.00000183660, 0.00000129340, 
		0.00000091093, 0.00000064153, 0.00000045181
	};
	const double nmMin=360.,nmMax=830.,nmDelta=5.;
	const int iMax=sizeof(V)/sizeof(V[0])-1;
	register double a;
	register int i;
	static firstTime=1;
	
	assert(iMax==(nmMax-nmMin)/nmDelta);
	if(firstTime){
		for(i=0;i<=iMax;i++)V[i]=log(V[i]);
		firstTime=0;
	}
	a=(nm-nmMin)/nmDelta;
	i=a;	// integer part
	a-=i;	// fractional part
	if(i==iMax && a==0.0)return exp(V[iMax]);
	if(i<0 || i>=iMax)return 0.0;
	return exp(V[i]+(V[i+1]-V[i])*a);	// geometric interpolation
}

double VLambdaPrime(double nm)
{
	/*
	At 5 nm intervals, from Table I(4.3.2) page 789,
	G. Wyszecki and W.S.Stiles (1982) Color Science,
	2nd Ed., Wiley, New York
	*/
	static double V[]={
		0.0005890000, 0.0011080000, 0.0022090000, 0.0045300000, 
		0.0092900000, 0.0185200000, 0.0348400000, 0.0604000000, 
		0.0966000000, 0.1436000000, 0.1998000000, 0.2625000000, 
		0.3281000000, 0.3931000000, 0.4550000000, 0.5130000000, 
		0.5670000000, 0.6200000000, 0.6760000000, 0.7340000000, 
		0.7930000000, 0.8510000000, 0.9040000000, 0.9490000000, 
		0.9820000000, 0.9980000000, 0.9970000000, 0.9750000000, 
		0.9350000000, 0.8800000000, 0.8110000000, 0.7330000000, 
		0.6500000000, 0.5640000000, 0.4810000000, 0.4020000000, 
		0.3288000000, 0.2639000000, 0.2076000000, 0.1602000000, 
		0.1212000000, 0.0899000000, 0.0655000000, 0.0469000000, 
		0.0331500000, 0.0231200000, 0.0159300000, 0.0108800000, 
		0.0073700000, 0.0049700000, 0.0033350000, 0.0022350000, 
		0.0014970000, 0.0010050000, 0.0006770000, 0.0004590000, 
		0.0003129000, 0.0002146000, 0.0001480000, 0.0001026000, 
		0.0000715000, 0.0000501000, 0.0000353300, 0.0000250100, 
		0.0000178000, 0.0000127300, 0.0000091400, 0.0000066000, 
		0.0000047800, 0.0000034820, 0.0000025460, 0.0000018700, 
		0.0000013790, 0.0000010220, 0.0000007600, 0.0000005670, 
		0.0000004250, 0.0000003196, 0.0000002413, 0.0000001829, 
		0.0000001390
	};
	const double nmMin=380.,nmMax=780.,nmDelta=5.;
	const int iMax=sizeof(V)/sizeof(V[0])-1;
	register double a;
	register int i;
	static firstTime=1;
	
	assert(iMax==(nmMax-nmMin)/nmDelta);
	if(firstTime){
		for(i=0;i<=iMax;i++)V[i]=log(V[i]);
		firstTime=0;
	}
	a=(nm-nmMin)/nmDelta;
	i=a;	// integer part
	a-=i;	// fractional part
	if(i==iMax && a==0.0)return exp(V[iMax]);
	if(i<0 || i>=iMax)return 0.0;
	return exp(V[i]+(V[i+1]-V[i])*a);	// geometric interpolation
}
