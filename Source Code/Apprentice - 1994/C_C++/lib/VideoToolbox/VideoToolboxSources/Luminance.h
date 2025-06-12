/*
Luminance.h
This is the include file for Luminance.c and ReadLuminanceRecord.c
Copyright 1989,1990,1991,1992 (c) Denis G. Pelli 
HISTORY:
9/18/90	dgp	Changed all instances of "v" to "V". The final version of the Pelli
			& Zhang (1991) manuscript refers to a nominal voltage v; this file
			now refers to the "equivalent number" V; they are related by V=255*v.
			To avoid "breaking" all the old LuminanceRecord.h files, I've 
			introduced a #define that converts vMin & vMax to VMin & VMax. However,
			this is a temporary fix. I'll probably remove the define in a few months,
			so you should still update any old calibration files, either by editing,
			replacing "vM" by "VM" or by redoing the calibration.
9/24/90	dgp	Added screen & date. Renamed nBackground to VBackground.
			Renamed index to lastIndex.
9/28/90	dgp	Renamed lastIndex to latestIndex. Spruced up some of the comments.
10/31/90 dgp Added the conditional FAST_LUMINANCE. Setting it to 1 (true) makes
			SetLuminancesAndRange() run twice as fast, by using fixed point
			arithmetic instead of doubles. Setting it to 0 (false) causes the
			same code to be compiled to use doubles. Note that the prototypes of
			some internal functions (names beginning with underscore) are conditional
			on FAST_LUMINANCE.
11/6/90 dgp Replaced Milli by Fixed. 
11/8/90 dgp	Eliminated gamma slope table shiftedLSlope[] since the speed-up it offered
			was too small to measure.
7/30/91	dgp	Added prototype for ReadLuminanceRecord().
8/4/91	dgp	Change ReadLuminanceRecord() to return type int.
8/5/91	dgp	Trying to compile under MPW C 3.2 uncovered a name-space conflict.
			The MPW C 3.2 CType.h header file defines _L as a preprocessor constant,
			whereas Luminance.h uses it as a field of the luminanceTable structure,
			and as a parameter name.
			The THINK C 4.05 ctype.h header file names the same constant __LOWR,
			which of course doesn't conflict. The obvious solution would be to
			change MPW's CType.h _L to _LOWR, but that would compromise portability.
			I decided not to change _L in Luminance.h and Luminance.c,
			because it would be hard to do that in a way that wouldn't compromise 
			the readability of the code. However, I did do a quick hack, 
			about ten lines below, that redefines _L as an enum, which solves the
			problem, provided CType.h is included before Luminance.h in files that
			need to explicitly access _L. In fact _L is intended for use
			only by the routines that are in Luminance.c, so I suspect the
			problem is solved.
12/17/92	dgp Removed obsolete support for THINK C 4. 
12/21/92 dgp Added dacSize and leftShift.
6/5/93	dgp	Removed the ancient #defines that were required to read ²1990 calibration
			files.
8/12/93	dgp	updated ReadLuminanceRecord() prototype to match source file.
*/

#pragma once		/* suppress multiple inclusions of this file */
#ifndef _LUMINANCE_	/* suppress multiple inclusions of this file */
#define _LUMINANCE_

#ifndef __QUICKDRAW__
	#include <QuickDraw.h>
#endif
#ifndef __SOUND__
	#include <Sound.h>
#endif
/*
The following trickery redefines the MPW C 3.2 CType.h define constant
_L as an enum, so that it won't conflict with the use of _L as a parameter name or
structure field.
*/
#ifdef _L
	enum{____L=_L};
	#undef _L
	enum{_L=____L};
#endif

/* These 3 #defines are a temporary fix, to allow use of old LuminanceRecord.h files */
#if 0
	#define vMin VMin
	#define vMax VMax
	#define nBackground VBackground
#endif

/* LINEAR_V_DOMAIN is the maximum interval in V over which the gamma function is
to be assumed linear by LToV(). The value 4 gives almost the same accuracy as 1, yet
results in SetLuminances() on a Mac II taking only 29 ms instead of 50 ms. 
See _LToV() in Luminance.c  for more information. */

#define MACINTOSH	1		/* set to 0 to use on any other computer */
#define LINEAR_V_DOMAIN 4	/* see above */
#define FAST_LUMINANCE 1	/* 1 for Fixed math (twice as fast), 0 for double math */
#define DACS 3				/* number of digital-to-analog converters that we support */
#define COLORS 256			/* size of ColorSpec table */
#define MAX_COEFFICIENTS 9	/* polynomial fit */
#define LUMINANCES_IN_TABLE 128	/* size of gamma table */

#if FAST_LUMINANCE & !defined(__TOOLUTILS__)
	#include <ToolUtils.h>	/* prototypes for FixMul etc. */
#endif

typedef struct{
	ColorSpec table[COLORS];	/* a table of values destined for the clut */
} Clut;

enum {Michelson,Weber};	/* contrastType */

typedef struct {
	short exists;	/* set to luminanceSet once this table has been initialized */
	short latestIndex;/* last-used index of LR.L._L[] is good place to start search */
	#if FAST_LUMINANCE
		Fixed _VMin;	/* bounds monotonic part of domain of gamma function */
		Fixed _VMax;	/* bounds monotonic part of domain of gamma function */
		Fixed _dV;		/* V = _VMin,_VMin+_dV,_VMin+2dV,_VMin+3dV, . . . ,_VMax */
						/* _dV=(_VMax-_VMin)/(LUMINANCES_IN_TABLE-1) */
		Fixed _L[LUMINANCES_IN_TABLE];
		long LShift;	/* bit shift to be applied to luminance differences */
	#else
		double _VMin;	/* bounds monotonic part of domain of gamma function */
		double _VMax;	/* bounds monotonic part of domain of gamma function */
		double _dV;		/* V = _VMin,_VMin+_dV,_VMin+2dV,_VMin+3dV, . . . ,_VMax */
						/* _dV=(_VMax-_VMin)/(LUMINANCES_IN_TABLE-1) */
		double _L[LUMINANCES_IN_TABLE];
	#endif
} luminanceTable;

typedef struct {	/* "table" MUST be the first thing in the structure! */
	ColorSpec table[COLORS];/* a table of values destined for the clut */
	short dacSize;		/* bits */
	short leftShift;	/* bit shift of V to produce 16 bit value=16-dacSize */
	short VMin,VMax;	/* lowest and highest DAC values allowed: 0 and 255 */
	double LMin,LMax;	/* luminances at VMin & VMax */
	double LBackground;	/* the background luminance used during luminance calibration */
	short VBackground;	/* the background number used during luminance calibration */
	short screen;		/* device=GetScreenDevice(LR.screen); */
	char *id;			/* make, model, and serial number of monitor */
	char *name;			/* informal name of monitor */
	char *date;			/* when calibrated */
	char *notes;		/* description of calib conditions: who & how */
	double dpi;			/* pixels per inch */
	double Hz;			/* frames per second */
	char *units;		/* Luminance units, e.g. "cd/m^2" */
	long coefficients;	/* the number of coefficients, not more than MAX_COEFFICIENTS, */
						/*	usually 9, giving an 8th-order polynomial, */
	double p[MAX_COEFFICIENTS];
						/* coefficients of a polynomial in value V, yielding L in cd/m^2 */
	double polynomialError;	/* RMS error of polynomial fit */
						/* L(V)=p[0]+p[1]*V+p[2]*V*V+ . . . ±polynomialError */
	double q[3];		/* coefficients of a quadratic polynomial in V */
	double quadraticError;	/* RMS error of quadratic fit */
						/* L(V)=q[0]+q[1]*V+q[2]*V*V±quadraticError */
	double power[4];	/* coefficients of a power law fit */
	double powerError;	/* RMS error of power law fit */
						/* L(V)=power[0]+Rectify(power[1]+power[2]*V)^power[3]±powerError */
						/* Rectify(x)=x if x³0, Rectify(x)=0 if x<0 */
						/* Pelli & Zhang (1991) Eqs.9&10 use symbols: */
						/* v=V/255, alpha=power[0], beta=power[1], kappa=power[2]*255, */
						/* gamma=power[3] */
	double fixedPower[4];	/* coefficients of a power law fit, with fixed exponent */
	double fixedPowerError;	/* RMS error of power law fit */
						/* L(V)=fixedPower[0]+Rectify(fixedPower[1]+fixedPower[2]*V)^fixedPower[3]±fixedPowerError */
	double r,g,b;		/* voltage gains for the three pathways. r+g+b=1. All must be ³0. */
	double gainAccuracy;/* possible error in r,g,b */
	double gm;			/* The monitor's contrast gain. The Michelson contrast produced by
							a small Æv at the background luminance is c=gm*Æv */
	/* The rest of the parameters are for temporary storage by SetLuminanceRange() */
	double lowLuminance;	/* the bottom of the range */
	double highLuminance;	/* the top of the range */
	short rangeSet;			/* a check that the range really has been set */
	short dacs;				/* the number of dacs with nonzero gain, usually 1 or 3 */
	short fixed;			/* the number of dacs whose value will be fixed for this range */
	short dac[DACS];		/* which dac corresponds to each gain, r=0,g=1,b=2 */
	double gain[DACS];		/* the ordered normalized gains of the Video Attenuator */
	double VHalfStep;		/* half a step of the finest dac */
	double VFixed;			/* the value produced by the fixed dacs */
	double LOffset;			/* a small shift of the requested luminance range */
	double tolerance;		/* the luminance error corresponding to half 
								a step of the coarsest of the variable dacs */
	#if FAST_LUMINANCE
		Fixed _gain[DACS];
		Fixed _VHalfStep;
		Fixed _VFixed;
		Fixed _LOffset;
		Fixed _tolerance;
	#else
		double _gain[DACS];
		double _VHalfStep;
		double _VFixed;
		double _LOffset;
		double _tolerance;
	#endif
	RGBColor rgb;			/* cache the values of the fixed DACs */
	luminanceTable L;
} luminanceRecord;

enum {luminanceSet=12345};	/* a unique value that we can check for later */

/* Luminance.c */
double EToL(luminanceRecord *LP,int entry);
int LToE(luminanceRecord *LP,double L,int firstEntry,int lastEntry);
int LtoEOrdered(luminanceRecord *LP,double L,int firstEntry,int lastEntry);
double SetLuminance(GDHandle device,luminanceRecord *LP
	,int theEntry,double luminance
	,double lowLuminance,double highLuminance);
double SetLuminances(GDHandle device,luminanceRecord *LP
	,int firstEntry,int lastEntry
	,double firstLuminance,double lastLuminance);
double SetLuminancesAndRange(GDHandle device,luminanceRecord *LP
	,int firstEntry,int lastEntry
	,double firstLuminance,double lastLuminance
	,double lowLuminance,double highLuminance);
void LoadLuminances(GDHandle device, luminanceRecord *LP,
	int firstEntry, int lastEntry);
void IncrementLuminance(GDHandle device,luminanceRecord *LP,int theEntry);
double GetLuminance(GDHandle device,luminanceRecord *LP,int theEntry);
double GetV(GDHandle device,luminanceRecord *LP,int theEntry);
double VToL(luminanceRecord *LP,double V);
double LToV(luminanceRecord *LP,double L);
double LToVFormulaic(luminanceRecord *LP,double L);
double LToL(luminanceRecord *LP,double L);

/* The following routines are primarily for internal use. */
#if FAST_LUMINANCE
	void _SetLuminance(luminanceRecord *LPtr,int theEntry,Fixed _luminance);
	Fixed _Tolerance(luminanceRecord *LP,Fixed _luminance);
	void _SetLuminances(luminanceRecord *LP,int first,int last
		,Fixed _firstL,Fixed _dL64,Fixed _firstV,Fixed _lastV);
	Fixed _VToL(luminanceRecord *LP,Fixed _V);
	Fixed _LToV(luminanceRecord *LP,Fixed _L);
#else
	void _SetLuminance(luminanceRecord *LPtr,int theEntry,double _luminance);
	double _Tolerance(luminanceRecord *LP,double _luminance);
	void _SetLuminances(luminanceRecord *LP,int first,int last
		,double _firstL,double _dL8,double _firstV,double _lastV);
	double _VToL(luminanceRecord *LP,double _V);
	double _LToV(luminanceRecord *LP,double _L);
#endif
double SetLuminanceRange(luminanceRecord *LP
	,double lowLuminance,double highLuminance);
double VToLPower(luminanceRecord *LP,double V);			/* use VToL() instead */
double VToLPolynomial(luminanceRecord *LP,double V);	/* use VToL() instead */
double LToVPower(luminanceRecord *LP,double L);			/* use LToV() instead */
double LToVPolynomial(luminanceRecord *LP,double L);	/* use LToV() instead */
double LToVQuadratic(luminanceRecord *LP,double L);		/* use LToV() instead */

/* ReadLuminanceRecord.c */

long ReadLuminanceRecord(char *filename,luminanceRecord *LP,short flags);
long WriteLuminanceRecord(char *filename,luminanceRecord *LP,short flags);
Description *DescribeLuminanceRecord(luminanceRecord *LP);

#endif _LUMINANCE_

