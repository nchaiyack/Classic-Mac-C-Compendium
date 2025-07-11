/*
Luminance.c
See Luminance.h for prototypes.
See Luminance.note for explanation.
Also see:
D.G. Pelli and L. Zhang (1991) Accurate control of contrast on microcomputer displays. 
Vision Research, 31:1337-1360.

Copyright (c) 1989-1993 Denis G. Pelli. This software is free; you may use it in
your research and give it away to others, with the following restrictions. Any
copy you give away must include this paragraph, unmodified, and, if you have
made any changes to the rest of the file, must include a note, added to HISTORY
below, giving your name, the date, and a description of the changes. This
software may not be sold, whether in source or compiled form, without my
permission. I hope you will find this software useful, but I can't promise that
it will work for you, and am not offering any support. That's why it's free. I
would appreciate reports of bugs and improvements.

Denis G. Pelli, Ph.D.
Institute for Sensory Research, Syracuse University, Syracuse, NY 13244-5290, U.S.A.
denis_pelli@isr.syr.edu

HISTORY:

4/24/89 dgp first working version.
4/28/89 dgp added device argument.
6/23/89 dgp added support for a video attenuator that has unequal gain in the three
			channels. Added LoadClut() and LuminanceClut() routines.
7/30/89 dgp replaced call to SetEntries (which calls the Color Manager) by a call
			to GDSetEntries (which calls the device driver).
8/10/89 dgp fixed bugs.
8/13/89 dgp rewrote LToV() to use nth order polynomial. Broke up LinearizeClut into
			three routines--SetLuminanceRange, SetLuminance, and SetLuminances--the
			last of which is equivalent to the old LinearizeClut.
8/16/89	dgp	After reading the Brooktree DAC manual I changed the documentation and
			increased tolerance from 0.5 to 1 LSB, and now use the highest
			luminance gain (worst case) over the requested range to transform it
			into a luminance difference tolerance.
8/21/89	dgp	Made minor improvements in LToV() to deal with failure to converge due to 
			getting stuck in a local minimum of the quartic. My solution is to now
			require that Lmin and Lmax be filled in by the user in the calibration
			structure. In practice this will guarantee that the minimum luminance
			will not be below the local minimum found at the base of the rising curve.
			Last week I also changed the LToV algorithm slightly, to do a bisection if
			Newton's method would take us out of range.
9/7/89 dgp	Fixed bug in sorting routine, Sort().
9/10/89 dgp	Commented out the polynomial versions of VToL and LToV and replace them by
			new versions that use a power law. The power law is a marginally better fit
			than a 6th order polynomial (using only 4 instead of 7 parameters) and its
			inverse can be computed much more quickly, about 0.3 ms instead of 2 ms.
9/11/89 dgp	Deciding to be indecisive, I introduced a conditional POWER_LAW_FIT
			to control the choice of power law or polynomial fit for LToV and VToL.
9/16/89	dgp	Having finished writing the paper (Pelli & Zhang, 1991) I came back to
			change the program to agree with what I wrote. I changed the equivalent
			number tolerance to be the sum instead of the maximum of the 
			variable-dac gains.
9/25/89 dgp Fixed minor bug in computation of tolerance. Now gives correct answer even
			when the lowLuminance or highLuminance is out of range.
10/9/89 dgp	Made minor change so that when some of the gain[] are zero SetLuminance will
			load zero into the unused colorSpec array elements. This is only a cosmetic
			change.
10/28/89 dgp Made minor changes. I declared LToVPolynomial and LToVPower and
			VToLPolynomial and VToLPower instead of having a conditional compilation. 
			This makes both flavors of routine always available. I also changed 
			LToVPolynomial to use LToVPower instead of LToVQuadratic for its initial 
			guess. The quadratic guess was often poor and occasionally led to 
			convergence problems. If the power law fit is not available then it reverts
			to using the quadratic fit. If that's not available then it reverts to using
			a middle-of-range guess.
10/28/89 dgp I eliminated the compile-time flag POWER_LAW_FIT and instead use whichever, 
			polynomial or power, provides a better fit, as determined at run time. I 
			biased the comparison of rms error to favor the power law fit since it has 
			few parameters and can be inverted much more quickly. Note: if the 
			power, polynomial, or quadratic fit parameters are not supplied then the 
			appropriate LR.powerError, LR.polynomialError, or LR.quadraticError field 
			should be set to infinity: INF or 1.0/0.0.
12/5/89	dgp	Added the trivial routine LToL() which enforces the bounds LP->LMin and 
			LP->LMax.
4/2/90	dgp	Capitalized the word "To" in all function names, e.g. LToL().
4/22/90	dgp	Version 1.4. Made minor changes to the documentation. Renamed LToV() to
			LToVFormulaic(), and introduced a new LToV() that uses a table to run faster.
			LToV() takes an average of 170 �s, whereas LToVFormulaic() takes
			1700 �s. I also now use the luminanceTable, if available, to speed up
			VToL() slightly, from 310 �s to 180 �s. Each call to SetLuminance() now
			takes 0.9 ms whereas it used to take 2.5 ms. The loss in accuracy is
			negligible. The interpolation error can be reduced still further by
			increasing LUMINANCES_IN_TABLE. Each doubling of the table size quarters the
			maximum possible error of the interpolation.
4/23/90	dgp	Added a few lines of code to LToV() to check near the last index, in the
			spirit of Numerical Recipes in C, hunt.c, before starting the bisection
			search. I updated the times in the 4/22/90 note to reflect the latest timing
			by TestLuminance.
7/27/90	dgp	Tightened up the error checking for illegal entries into the clut. I have
			the impression that, contrary to specification, the Apple video driver
			crashes and corrupts itself if given an out-of-range entry value in a
			setEntry Control call.
9/18/90	dgp	Changed all instances of "v" to "V". The final version of the Pelli
			& Zhang (1991) paper refers to the nominal voltage v; this file
			now refers to the "equivalent number" V; they are related by V=255*v.
9/24/90	dgp	Updated the documentation to correspond more closely to the (hopefully)
			final version of the Pelli & Zhang (1991) paper.
10/29/90 dgp Doubled speed of SetLuminance(). Now SetLuminance() takes 133 ms to
			do a complete clut for a small new luminance range, and takes 188 ms for a
			large new range. This required minor changes to SetLuminanceRange() & LToV().
			Changed all function headers to Standard C prototype style.
10/30/90 dgp Replaced phrase "clut value" by "nominal voltage", for consistency with
			Pelli & Zhang (1991).
			Introduced new fixed fraction data type called "Milli", defined in Milli.h.
			By judicious replacement of double by Milli, particularly in the luminance
			table, I have increased the speed of SetLuminance by a factor of three.
			A Mac II now takes only 42 to 50 ms to build a whole clut. This new feature
			is enabled by setting FAST_LUMINANCE to 1 in Luminance.h. There is a slight
			increase in the luminance tolerance.
10/31/90 dgp More fine tuning. Now takes 31 to 44 ms to build a whole clut. Introduced
			conditional FAST_LUMINANCE in Luminance.h that causes the same code to
			be compiled with either Milli or double variables. The _SetLuminance()
			routine is now quite polished, and runs fast. All variables and routines
			whose names begin with underscore _ are written here with type Milli, but
			if FAST_LUMINANCE is false then this type is redefined (in this file only)
			as double, and all the Milli macros are redefined to be appropriate for
			a double. So bear in mind that things beginning with underscore are of
			unknown type. The virtue of being able to run the same code with Milli or
			double computations is that if you suspect an error in the (homemade)
			Milli arithmetic, you can try out double arithmetic simply by setting
			FAST_LUMINANCE to 0 and recompiling.
			In the interest of speed I have streamlined the tolerance
			computation. The new tolerances are probably as good as the old ones.
11/2/90 dgp Added _SetLuminances() subroutine that substitutes linear interpolation
			for most of the calls to _LToV(). This has greatly speeded up
			SetLuminancesAndRange(), with minor loss of accuracy. A complete clut now
			takes 9 to 31 ms. Even a Mac II can now compute low-contrast cluts
			on the fly, between frames. (The user may wish to optimize the speed-
			accuracy trade-off controlled by the LINEAR_V_DOMAIN parameter, which
			determines the maximum width of the interpolation interval.)
11/6/90 dgp Replaced Milli by FIXED, which can be compiled as either double or Fixed.
			The Fixed math is sufficiently precise as to give the same DAC values as
			double math.
			Now figure out optimal bit shift LT->L.LShift to preserve as much resolution
			as possible when interpolating in _LToV(). 
			A complete clut now takes 8 to 30 ms. For threshold stimuli the average
			time will usually be less than 15 ms.
11/8/90 dgp	Eliminated gamma slope table since the speed-up it offered was too small
			to measure. Tidied up the computation of LShift.
8/24/91	dgp	Made compatible with THINK C 5.0.
12/17/91 dgp Added new routines: IncrementLuminance() and GetV().
			The former is used to obtain the lowest possible contrast. On a log contrast
			scale, minus infinity (zero contrast) is a poor approximation to even
			a small log contrast.
3/11/92	dgp	Minor editing of comments.
12/2/92 dgp Fixed minor bug in SetLuminanceRange() reported by Wei Xie and Ken Alexander
			that could cause the THINK C Debugger to report an error when an 
			uninitialized double V[] was converted to an int. The value was not used 
			for anything, so this was innocuous.
12/17/92 dgp 1.94 Began enhancement to allow any dacSize, not just 8-bit dacs. I have
			confirmed that everything still works fine with 8-bit dacs, but I don't
			have a 9-bit dac video card to test it any further. �CAUTION: this probably
			does not yet work correctly for 9-bit dacs. E.g. it seems likely that the 
			fixed point math will break if VMax is increased to 511; a lot of careful 
			error analysis went into the original coding, at which time I thought that 
			it was safe to assume an 8-bit dac (i.e. VMax==255). �Removed obsolete 
			support for THINK C 4. 
12/21/92 dgp No longer load unused dac bits.
1/4/93	dgp	LoadLuminances now checks the gdType.
5/10/93	dgp	If driver is in gray mode (i.e. not color) then LoadLuminances maps rgb to 
			gray by simply copying the green component to the red and blue components.
5/12/93	dgp removed debugging code that forced isGray to always be true.
3/24/94	dgp	added LToE, LToEOrdered, and EToL.
*/
#include "VideoToolbox.h"		/* prototype for GDSetEntries() */
#include "Luminance.h"
#include <assert.h>
#include <math.h>

static void Sort(int n,double arr[],int krr[]);	/* for internal use only */

#undef LongToFix
#undef FixToLong
#undef DoubleToFix
#undef FixToDouble
#if FAST_LUMINANCE
	#define FIXED Fixed
	#if !MACINTOSH
		typedef long Fixed;
		#define FixMul(x,y) DoubleToFix(FixToDouble(x)*FixToDouble(y))
		#define FixDiv(x,y) DoubleToFix(FixToDouble(x)/FixToDouble(y))
	#endif
	#define LongToFix(x) ((long)(x)<<16)
	#define FixToLong(x) ((x)>>16)
	#define DoubleToFix(x) ((Fixed)((x)*65536.+0.5))
	#define FixToDouble(x) ((double)(x)*(1./65536.))
	#define D(x) FixToDouble(x)							/* handy for debugging */
#else
	#define FIXED double
	#define FixMul(x,y) ((double)(x)*(y))
	#define FixDiv(x,y) ((double)(x)/(y))
	#define LongToFix(x) ((double)(x))
	#define FixToLong(x) ((long)(x))
	#define DoubleToFix(x) ((double)(x))
	#define FixToDouble(x) ((double)(x))
#endif

#undef MAX
#undef MIN
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

double EToL(luminanceRecord *LP,int entry)
{
	return GetLuminance(NULL,LP,entry);
}

int LToE(luminanceRecord *LP,double L,int firstEntry,int lastEntry)
//Returns the index of the table entry in specified range with luminance closest to L.
{
	int i,entry;
	double dL,latestDL;
	
	latestDL=1.0/0.0;
	entry=0;
	for(i=firstEntry;i<=lastEntry;i++){
		dL=fabs(L-GetLuminance(NULL,LP,i));
		if(dL<latestDL){
			entry=i;
			latestDL=dL;
		}
	}
	return entry;
}

int LtoEOrdered(luminanceRecord *LP,double L,int firstEntry,int lastEntry)
/*
Returns the index of the table entry in specified range with luminance closest to L.
Assumes ordered table from firstEntry to lastEntry, either increasing or decreasing.
Returned entry is guaranteed to be in the range firstEntry...lastEntry.
Based on Numerical Recipes "locate.c", page 117
*/
{
	register int lower,middle,upper;
	register double LLower,LMiddle,LUpper;
	int ascend;
	
	lower=firstEntry;
	upper=lastEntry;
	LLower=GetLuminance(NULL,LP,lower);
	LUpper=GetLuminance(NULL,LP,upper);
	ascend=(LUpper>LLower);
	while(upper-lower>1){
		middle=(upper+lower)/2;
		LMiddle=GetLuminance(NULL,LP,middle);
		if(L>LMiddle == ascend){
			lower=middle;
			LLower=LMiddle;
		}
		else{
			upper=middle;
			LUpper=LMiddle;
		}
	}
	if(L-LLower<LUpper-L == ascend)return lower;
	else return upper;
}
double SetLuminance(GDHandle device,luminanceRecord *LP
	,int theEntry,double luminance
	,double lowLuminance,double highLuminance)
/*
Set one entry in the ColorSpec table (and the clut if device is not NULL) to
a specified luminance. It's ok for lowLuminance to be greater than highLuminance; 
they still designate a range.
*/
{
	FIXED _luminance;
	
	if(theEntry<0 || theEntry>=COLORS 
		|| device!=NULL && theEntry>=GDCLUTSIZE(device))
			PrintfExit("\007SetLuminance: illegal entry %d\n",theEntry);
	
	SetLuminanceRange(LP,lowLuminance,highLuminance);
	_luminance=DoubleToFix(luminance);
	_SetLuminance(LP,theEntry,_luminance);
	if(device != NULL)LoadLuminances(device,LP,theEntry,theEntry);
	return FixToDouble(_Tolerance(LP,_luminance));
}

double SetLuminances(GDHandle device,luminanceRecord *LP
	,int firstEntry,int lastEntry
	,double firstLuminance,double lastLuminance)
/*
Set a series of entries in the ColorSpec table (and the clut if device is not NULL)
to a linear sequence of luminances. Assume this is the entire luminance range of 
interest.
*/
{
	return SetLuminancesAndRange(device,LP,firstEntry,lastEntry
		,firstLuminance,lastLuminance,firstLuminance,lastLuminance);
}

double SetLuminancesAndRange(GDHandle device,luminanceRecord *LP
	,int firstEntry,int lastEntry
	,double firstLuminance,double lastLuminance
	,double lowLuminance,double highLuminance)
/*
Set a series of entries in the ColorSpec table (and the clut if device is not
NULL) to a linear sequence of luminances. Uses last two arguments to set the
luminance range.

I introduced a stack-space check before calling _SetLuminances(), since it calls
itself recursively and may use 1000 bytes all together. However, printing an
error message requires at least 4500 bytes. So I quit if the stack gets below
6000, so that the user will be presented with an understandable error message
rather than a mysterious quit or crash.
*/
{
	FIXED _tolerance,_t,_dL64;
	FIXED _firstL,_lastL,_firstV,_lastV;
	
	if(firstEntry<0 || firstEntry>lastEntry || lastEntry>=COLORS 
		|| device!=NULL && lastEntry>=GDCLUTSIZE(device) )
		PrintfExit("\007SetLuminancesAndRange: illegal entries %d %d\n",firstEntry,lastEntry);
	if(StackSpace()<6000)PrintfExit("SetLuminancesAndRange: not enough stack space.\007\n");
	SetLuminanceRange(LP,lowLuminance,highLuminance);
	_firstL=DoubleToFix(firstLuminance);
	_lastL=DoubleToFix(lastLuminance);
	_firstV=_LToV(LP,_firstL + LP->_LOffset);
	_lastV=_LToV(LP,_lastL + LP->_LOffset);
	if(lastEntry != firstEntry){
		#if FAST_LUMINANCE
		_dL64=DoubleToFix(64.0*(lastLuminance-firstLuminance)/(lastEntry-firstEntry));
		#else
		_dL64=DoubleToFix((lastLuminance-firstLuminance)/(lastEntry-firstEntry));
		#endif
	}else _dL64=0;
	_SetLuminances(LP,firstEntry,lastEntry,_firstL,_dL64,_firstV,_lastV);
	if(device != NULL)LoadLuminances(device,LP,firstEntry,lastEntry);	
	
	/* quick and dirty estimate of tolerance */
	_tolerance=LP->L._L[0] - _firstL;
	_t=_lastL - LP->L._L[LUMINANCES_IN_TABLE-1];
	if(_tolerance<_t)_tolerance=_t;
	if(_tolerance<0)_tolerance=0;
	_tolerance+=LP->_tolerance;
	return FixToDouble(_tolerance);	/* estimate of max error in �L */
}

void _SetLuminances(luminanceRecord *LPtr,int first,int last
	,FIXED _firstL,FIXED _dL64,FIXED _firstV,FIXED _lastV)
/*
This routine eliminates most of the the slow _LToV() table lookups and
interpolations by assuming that the gamma function is smooth enough that we may
linearly interpolate over any interval of V no wider than LINEAR_V_DOMAIN. Since
the gamma function is roughly parabolic, the consequent error in L will be
proportional to the square of LINEAR_V_DOMAIN, and independent of where this
interval is along V. Since we're inscribing straight line segments in a
positively accelerating gamma function, we will overestimate luminance, and
consequently V will be too low. The error will be greatest at the middle of each
linearly interpolated V interval.

If the requested V interval is larger than LINEAR_V_DOMAIN, then it is split
into two intervals by making a pair of recursive calls. LINEAR_V_DOMAIN is
defined in Luminance.h. I suggest a value of 4, but it may be set larger to
attain slightly higher speed at lower accuracy.

CAUTION: This routine is a stack hog. Each call uses up about 64 bytes on the
stack, and it calls itself recursively, up to log2(256/LINEAR_V_DOMAIN) times.
Do NOT change any of the declarations of the "new" variables used in the first
if{} to "static", as that would cause the recursion to fail.
*/
{
	static int doLast=1;
	int saveDoLast;
	int newOne;
	FIXED _newL,_newV;
	register luminanceRecord *LP=LPtr;
	register int i,Vi;
	register FIXED _VToGo,_g;
	static RGBColor *RGBPtr;	/* static in order to minimize stack usage */
	static int theEntry;		/* static in order to minimize stack usage */
	static FIXED _V,_dV;		/* static in order to minimize stack usage */
	register short leftShift;
	
	if(last-first>1 ){
		_dV=_lastV-_firstV;
		if(_dV>LongToFix(LINEAR_V_DOMAIN) || _dV<LongToFix(-LINEAR_V_DOMAIN)){
			newOne=(first+last)>>1;
			#if FAST_LUMINANCE
				_newL=_firstL+((newOne-first)*_dL64>>6);
			#else
				_newL=_firstL+(newOne-first)*_dL64;
			#endif
			_newV=_LToV(LP,_newL + LP->_LOffset);
			saveDoLast=doLast;
			doLast=0;
			_SetLuminances(LP,first,newOne,_firstL,_dL64,_firstV,_newV);
			doLast=saveDoLast;
			_SetLuminances(LP,newOne,last,_newL,_dL64,_newV,_lastV);
			return;
		}
	}
	if(last!=first)_dV=(_lastV-_firstV)/(last-first);
	else _dV=0;
	if(!doLast)last--;
	leftShift=LP->leftShift;
	for(theEntry=first,_V=_firstV;theEntry<=last;theEntry++,_V+=_dV){
		/****** This section of code is copied from _SetLuminance() ****/
		RGBPtr = &LP->table[theEntry].rgb;
		*RGBPtr=LP->rgb;						/* load for fixed DACs */
		_VToGo=_V - LP->_VFixed + LP->_VHalfStep;
		for(i=LP->fixed;i<LP->dacs;i++) {
			_g=LP->_gain[i];
			Vi=_VToGo/_g;						/* truncate to integer */
			if(Vi>LP->VMax)Vi=LP->VMax;
			if(Vi<LP->VMin)Vi=LP->VMin;
			_VToGo -= Vi*_g;
			((short *)RGBPtr)[LP->dac[i]]=Vi<<leftShift;
		}
		/***************************************************************/
	}
	return;
}

void _SetLuminance(luminanceRecord *LPtr,int theEntry,FIXED _luminance)
/*
Set one entry in the ColorSpec table to a specified luminance. This is the
private subroutine that actually does all the work for SetLuminance(). This
routine is designed to run as fast as possible.
*/
{
	register luminanceRecord *LP=LPtr;
	register int i,Vi;
	register FIXED _VToGo,_g;
	RGBColor *RGBPtr;
	register short leftShift=LP->leftShift;
	
	RGBPtr = &LP->table[theEntry].rgb;
	*RGBPtr=LP->rgb;						/* load for fixed DACs */
	_VToGo=_LToV(LP,_luminance + LP->_LOffset) - LP->_VFixed + LP->_VHalfStep;
	for(i=LP->fixed;i<LP->dacs;i++) {
		_g=LP->_gain[i];
		Vi=_VToGo/_g;						/* truncate to integer */
		if(Vi>LP->VMax)Vi=LP->VMax;
		if(Vi<LP->VMin)Vi=LP->VMin;
		_VToGo -= Vi*_g;
		((short *)RGBPtr)[LP->dac[i]]=Vi<<leftShift;
	}
}

void IncrementLuminance(GDHandle device,luminanceRecord *LPtr,int theEntry)
/*
Make smallest possible increase of the luminance of one entry in the ColorSpec
table.
*/
{
	double V;
	register luminanceRecord *LP=LPtr;
	
	V=GetV(device,LP,theEntry);
	V+=LP->VHalfStep;
	V+=LP->VHalfStep;
	SetLuminance(device,LP,theEntry,VToL(LP,V),LP->lowLuminance,LP->highLuminance);
}

FIXED _Tolerance(luminanceRecord *LPtr,FIXED _luminance)
{
	register luminanceRecord *LP=LPtr;
	register FIXED _tolerance,_t;
	
	/* quick and dirty estimate of tolerance */
	if(LP->L.exists==luminanceSet){
		_tolerance=LP->L._L[0] - _luminance;
		_t=_luminance - LP->L._L[LUMINANCES_IN_TABLE-1];
		if(_tolerance<_t)_tolerance=_t;
		if(_tolerance<0)_tolerance=0;
		_tolerance+=LP->_tolerance;
	}
	else _tolerance=LP->_tolerance;
	return _tolerance;
}

double GetLuminance(GDHandle device,luminanceRecord *LP,int theEntry)
/*
If device is not NULL then examines one entry in the actual clut, otherwise
examines the ColorSpec table contained in *LP, and in either case returns the
luminance that will be produced. Supplying an illegal entry value results in a
returned value of -INF.
*/
{
	return VToL(LP,GetV(device,LP,theEntry));
}

double GetV(GDHandle device,luminanceRecord *LP,int theEntry)
/*
If device is not NULL then examines one entry in the actual clut, otherwise
examines the ColorSpec table contained in *LP, and in either case returns the V
that will be produced. Supplying an illegal entry value results in a returned
value of -INF.
*/
{
	RGBColor *myRGBPtr=NULL;
	ColorSpec myCSpec;
	double V;
	int error;
	
	if(device != NULL) {
		if(theEntry<0 || theEntry>=GDCLUTSIZE(device)){
			PrintfExit("GetLuminance: illegal entry %d\n",theEntry);
		}
		error=GDGetEntries(device,theEntry,0,&myCSpec);
		if(error){
			PrintfExit("GetLuminance: GDGetEntries error %d\007",error);
		}
		myRGBPtr=&myCSpec.rgb;
	}
	else {
		if(LP!=NULL && theEntry>=0 && theEntry<COLORS) 
			myRGBPtr=&LP->table[theEntry].rgb;
	}
	if(myRGBPtr == NULL) return -INF;
	V = LP->r*(myRGBPtr->red>>8);
	V += LP->g*(myRGBPtr->green>>8);
	V += LP->b*(myRGBPtr->blue>>8);
	return V;
}

void LoadLuminances(GDHandle device, luminanceRecord *LP,
	int firstEntry, int lastEntry)
/*
This just calls GDSetEntries() or GDDirectSetEntries() to load your ColorSpec
table into the clut of your screen device. It is here simply to provide a
cosmetic match to the call to SetLuminances(), for which loading the clut is
optional. Note: if you prefer, instead of LP you may send just the address of a
ColorSpec table, cast to (luminanceRecord *), since a luminanceRecord begins
with a ColorSpec table.
*/
{
	int error;
	short isGray,i;
	unsigned short j;
	RGBColor *RGBPtr;
	ColorSpec table[256],*tablePtr;

	isGray=!TestDeviceAttribute(device,gdDevType);
	if(isGray){
		for(i=firstEntry;i<=lastEntry;i++){
			RGBPtr=&table[i].rgb;
			RGBPtr->red=RGBPtr->green=RGBPtr->blue=LP->table[i].rgb.green;
		}
		tablePtr=&table[firstEntry];
	}else tablePtr=&LP->table[firstEntry];

	switch((*device)->gdType){
	case fixedType:
		printf("LoadLuminances: this device has a fixed CLUT.\n");
		return;
	case clutType:
		error=GDSetEntries(device,firstEntry,lastEntry-firstEntry,tablePtr);
		break;
	case directType:
		error=GDDirectSetEntries(device,firstEntry,lastEntry-firstEntry,tablePtr);
		break;
	}
	if(error) printf("\007LoadLuminances: error %d\n",error);
}

double LToL(luminanceRecord *LP,double L)
{
	if(L > LP->LMax) return LP->LMax;
	if(L < LP->LMin) return LP->LMin;
	return L;
}

double VToL(luminanceRecord *LP,double V)
/*
Return the luminance that would result from a nominal voltage.
*/
{
	return FixToDouble(_VToL(LP,DoubleToFix(V)));
}

FIXED _VToL(luminanceRecord *LP,FIXED _V)
/*
Return the luminance that would result from a nominal voltage.
*/
{
	register int i;
	register luminanceTable *LT;
	register FIXED _di;
	double LF,VF;
	
	LT=&LP->L;
	if(LT->exists==luminanceSet){
		_di=FixDiv(_V-LT->_VMin,LT->_dV);
		i=FixToLong(_di);
		_di -= LongToFix(i);
		if(i<0)return LT->_L[0];
		if(i>=LUMINANCES_IN_TABLE-1)return LT->_L[LUMINANCES_IN_TABLE-1];
		return LT->_L[i]+FixMul(_di,LT->_L[i+1]-LT->_L[i]);
	}
	VF=FixToDouble(_V);
	if(LP->powerError < 2.0*LP->polynomialError) LF=VToLPower(LP,VF);
	else LF=VToLPolynomial(LP,VF);
	return DoubleToFix(LF);
}

double LToV(luminanceRecord *LP,double L)
{
	return FixToDouble(_LToV(LP,DoubleToFix(L)));
}

FIXED _LToV(luminanceRecord *LP,FIXED _L)
/*
New, faster version uses a tabulated version of the gamma function VToL(). This
replaces the old function LToVFormulaic(), formerly called LToV(). A subtlety is
that I tabulate the gamma function _L[]=VToL() rather than its inverse, because
this yields a much lower upper bound on the error of the linearly interpolated
L. This is true because the gamma function is roughly parabolic. Equally spaced
samples of a parabolic function yield equal maximum error of linear
interpolation in all intervals. The cost of tabulating _L[] rather than its
inverse is that we must search the table in order to find the largest _L[] less
than or equal to L. The search time is minimized by using a search procedure
that begins the search at the place found by the last search, since successive
calls to LToV() are likely to request similar luminances.
*/
{
	register luminanceTable *LT;
	register int lo,hi,i;
	register FIXED _dL,_dLTemp;
	FIXED _V,_LLo;
	
	LT=&LP->L;
	if(LT->exists != luminanceSet){
		/* get domain of the monotonic section of the gamma function */
		LT->_VMin=DoubleToFix(LToVFormulaic(LP,LP->LMin));
		LT->_VMax=DoubleToFix(LToVFormulaic(LP,LP->LMax));
		LT->_dV=(LT->_VMax-LT->_VMin)/(LUMINANCES_IN_TABLE-1);
		_V=LT->_VMin;
		for(i=0;i<LUMINANCES_IN_TABLE;i++,_V+=LT->_dV) LT->_L[i]=_VToL(LP,_V);
		/* just in case, impose monotonicity, from center on out */
		for(i=LUMINANCES_IN_TABLE/2;i>0;i--)
			if(LT->_L[i-1] > LT->_L[i]) LT->_L[i-1]=LT->_L[i];
		for(i=1+LUMINANCES_IN_TABLE/2;i<LUMINANCES_IN_TABLE;i++)
			if(LT->_L[i-1] > LT->_L[i]) LT->_L[i]=LT->_L[i-1];
		#if FAST_LUMINANCE	/* compute the slope dL/dV */
			/* find shift that maximizes �L precision without overflow */
			_dL=0;
			for(i=0;i<LUMINANCES_IN_TABLE-1;i++){
				_dLTemp=LT->_L[i+1]-LT->_L[i];
				if(_dL<_dLTemp)_dL=_dLTemp;
			}
			LT->LShift=Log2L(LONG_MAX/_dL);
		#endif				
		LT->exists=luminanceSet;
		LT->latestIndex=-2;	/* invalid, so hunt will start from scratch */
	}
	/* hunt for L in LT->_L[] */
	/* first check at latestIndex, latestIndex�1, ... */
	lo=-1;
	hi=LUMINANCES_IN_TABLE;
	i=LT->latestIndex;
	if(i>lo && i<hi){
		if(_L>LT->_L[i])
			{lo=i;i++;}
		else
			{hi=i;i--;}
	}
	/* simple bisection search, see Numerical Recipes in C, pages 98-99. */
	while(hi-lo>1){
		i=(hi+lo)>>1;
		if(_L>LT->_L[i])lo=i;
		else hi=i;
	}
	LT->latestIndex=lo;
	if(lo<0){
		LT->latestIndex=0;			/* nearest legal index */
		return LT->_VMin;
	}
	if(lo>=LUMINANCES_IN_TABLE-1)return LT->_VMax;
	_LLo=LT->_L[lo];
	_dL=_L-_LLo;
	#if FAST_LUMINANCE
		return LT->_VMin+lo*LT->_dV
			+(_dL<<LT->LShift)/((LT->_L[lo+1]-_LLo<<LT->LShift)/LT->_dV);
	#else
		return LT->_VMin+LT->_dV*(lo+_dL/(LT->_L[lo+1]-_LLo));
	#endif
}

double LToVFormulaic(luminanceRecord *LP,double L)
/*
Find a nominal voltage that would give luminance L. Failing that, it returns the
closest possible value. The answer is computed by inverting the formula for the
gamma function.
*/
{
	if(LP->powerError < 2.0*LP->polynomialError) return LToVPower(LP,L);
	else return LToVPolynomial(LP,L);
}

double VToLPolynomial(luminanceRecord *LP,double V)
/*
Return the luminance that would result from a nominal voltage. Uses a polynomial
equation L = p[0] + p[1]*V^1 +... of any degree. dgp 8/11/89
*/
{
	register double L,VV;
	register int i,m;
	
	m=LP->coefficients;
	L=0.0;
	VV=1.0;
	for(i=0;i<m;i++){
		L+=LP->p[i]*VV;
		VV*=V;
	}
	return L;
}

double VToLPower(luminanceRecord *LP,double V)
/*
Return the luminance that would result from a nominal voltage.
Uses a rectified power law:

	L(V)=power[0]+Rectify(power[1]+power[2]*V)^power[3]
	
	Rectify(x)=x if x�0
	Rectify(x)=0 if x<0

dgp 10/28/89
*/
{
	double L;
	
	L=LP->power[1]+LP->power[2]*V;
	if(L>0.0) L=LP->power[0]+pow(L,LP->power[3]);
	else L=LP->power[0];
	return L;
}

double LToVPolynomial(luminanceRecord *LP,double L)
/*
Find a nominal voltage that would give luminance L. Failing that, it returns the
closest possible value. Solves a polynomial equation L = p[0] + p[1]*V^1 +... of
any degree. First we find a quick approximate answer by solving the power law
fit, LToVPower(). Then we use Newton's method to home in quickly on the solution
to the polynomial fit. Newton's method is taken from Numerical Recipes in C. For
a polynomial of degree 8 (which is what I recommend) this routine now does about
3? iterations and takes about 1.1? ms. The error in V is less than TOLERANCE,
i.e. relative to full scale of 256 we get an accuracy of 14.6 bits. You can
increase the TOLERANCE to reduce the number of iterations to make it slightly
faster. A TOLERANCE of 1.0 (for 8-bit accuracy) takes 0.8? ms, not much of a
savings in time, and a large loss in accuracy. Reducing TOLERANCE to 1e-8 yields
an accuracy of nearly 35 bits and takes 1.5? ms, only slightly longer.

This routine will be called essentially once per clut entry, so computing a
whole new clut with entries 0..255 will take 256*1.1? ms = 282? ms. Some
experiments use only part of the clut, and will therefore take less time. In
some of the experiments that do use the whole clut it may be desirable to
compute the clut table in advance.
dgp 8/11/89
*/
{
	#define TOLERANCE 0.01					/* desired accuracy of V, see above */
	#define JMAX 20
	double a[MAX_COEFFICIENTS];
	register int i;
	int m,j;
	register double f,df,u,VV,V,dV;
	
	if(L>LP->LMax) return LP->VMax;
	if(L<LP->LMin) return LP->VMin;

	m=LP->coefficients;
	if(m>MAX_COEFFICIENTS || m<1)
		PrintfExit("LToVPolynomial: %d coefficients is too many or too few\007\n",m);
	for(i=0;i<m;i++) a[i]=LP->p[i];
	a[0]-=L;
	if(LP->powerError < 0.2*LP->LMax) V=LToVPower(LP,L);	/* very good initial guess */
	else {
		if(LP->quadraticError < 0.2*LP->LMax) V=LToVQuadratic(LP,L);/* fair initial guess */
		else PrintfExit("LToVPolynomial: neither power nor quadratic fit is available\007\n");
	}
	for(j=0;j<JMAX;j++){
		/* evaluate function, i.e. the polynomial, and its derivative */
		f=a[0];
		df=0.0;
		VV=1.0;
		for(i=1;i<m;i++){
			u=a[i]*VV;
			df+=i*u;
			f+=V*u;
			VV*=V;
		}
		dV=f/df;	/* apply Newton's method */
		if(V-dV<LP->VMin) dV=(V-LP->VMin)/2.0;	/* bisect */
		if(V-dV>LP->VMax) dV=(V-LP->VMax)/2.0;	/* bisect */
		V -= dV;
		if(fabs(dV) < TOLERANCE) return V;
	}
	printf("LToVPolynomial(%7.2f): Warning, too many iterations. VToL(%5.1f)=%7.2f\n",L,V,VToL(LP,V));
	return V;
	#undef TOLERANCE
	#undef JMAX
}

double LToVPower(luminanceRecord *LP,double L)
/*
Find a nominal voltage that would give luminance L. Failing that, it returns the
closest possible value. Solves the rectified power law:
	L(V)=power[0]+Rectify(power[1]+power[2]*V)^power[3]
	Rectify(x)=x if x�0
	Rectify(x)=0 if x<0
LToVPower takes about 300 microseconds on a Mac II with 8881 arithmetic. This
routine will be called essentially once per clut entry, so computing a whole new
clut with entries 0..255 will take 256*0.3 ms = 77 ms. Some experiments use only
part of the clut, and will therefore take less time. In experiments that do use
the whole clut it may be desirable to compute the clut table in advance.
dgp 10/28/89
*/
{
	double V;
	
	if(L<LP->power[0]) L=LP->power[0];
	V=(pow(L-LP->power[0],1.0/LP->power[3])-LP->power[1])/LP->power[2];
	if(V>LP->VMax) V=LP->VMax;
	return V;
}

double LToVQuadratic(luminanceRecord *LP,double L)
/*
This is a quick, approximate version of LToV() that is used by LToVPolynomial
for an initial guess if no power law fit is available. Find nominal voltage that
would give luminance L. Since a quadratic fit to the luminance calibration is
only a fair approximation, this routine serves only to find a quick approximate
answer to the root of a higher order polynomial fit. Solves the quadratic
equation by method recommended in Numerical Recipes in C. My choice of root
assumes that L is an increasing function of V over the operating range of the
display. If concave (i.e. q[2]<0) use the larger root. If convex (i.e. q[2]>0)
use the smaller root. In plain English, if the parabola is right side up then we
take the right hand branch. If the parabola is upside down then we take the left
hand branch. dgp 8/12/89
*/
{
	register double a,b,c,q;
	double V;

	if(L>LP->LMax) return LP->VMax;
	if(L<LP->LMin) return LP->VMin;
	c=LP->q[0]-L;
	b=LP->q[1];
	a=LP->q[2];
	if(a == 0.0) return -c/b;
	if(b<0.0)
		q=-0.5*(b-sqrt(b*b-4.0*a*c));
	else
		q=-0.5*(b+sqrt(b*b-4.0*a*c));
	if(a*b<0.0)
		V=q/a;
	else
		V=c/q;
	return V;
}

double SetLuminanceRange(luminanceRecord *LPtr,double lowLuminance,double highLuminance)
/*
The user will never call this routine directly, as it is called automatically by
SetLuminance and SetLuminances. This routine does the hard work of figuring out
which DACs to fix to optimally represent a given luminance range. The goal is to
fix the coarser DACs and vary the finer DACs to represent the image. The CHANGES
in luminance (i.e. the contrast) will have the accuracy of the coarsest of the
variable DACs. To avoid the overhead of figuring all this out every single time
you call SetLuminance(), the results are stored temporarily in your
luminanceRecord, and are only recomputed if you request a different luminance
range.

This function checks and returns immediately if you call it with the same
luminance range that it already has stored in your luminanceRecord.
*/
{
	register luminanceRecord *LP=LPtr;
	register int i;
	double dL,dL2,LOffset;
	double VSum,VFixed;
	int dacs,fixed,dac[DACS];
	double gain[DACS],V[DACS];
	double VGoal;
	double lowV,highV;
	double tolerance;
	double VHalfStep;
	double dV;
	unsigned short *RGBArray;
	
	/* -1. If necessary, swap low & high */
	if(lowLuminance > highLuminance){
		dL=lowLuminance;
		lowLuminance=highLuminance;
		highLuminance=dL;
	}

	/* 0. Return right away if our work has already been done */
	if(LP->rangeSet == luminanceSet &&
		LP->lowLuminance==lowLuminance &&
		LP->highLuminance==highLuminance)
			return LP->tolerance;

	/* 1. sort the DACs by gain, discard any with zero or negative gain.
	Gain is defined as the change in V when a single DAC is increased from 0 to 1.
	Sort the gains so that gain[0]�gain[1]�gain[2]...
	Note this order is opposite to that used in Pelli & Zhang (1991). The answers
	are the same, but the notation is different.
	*/
	if(LP->rangeSet != luminanceSet){
		/* This only needs to be done once, even if the range changes. */
		gain[0]=LP->r;
		gain[1]=LP->g;
		gain[2]=LP->b;
		#if DACS>3
			#error "Current implementation doesn't support more than 3 DACs."
		#endif
		for(i=0;i<DACS;i++)dac[i]=i;
		Sort(DACS,gain,dac);			/* sort so that gain[0]�gain[1]�gain[2] */
		dacs=0;
		for(i=0;i<DACS;i++) if(gain[i]>0.0)dacs++;
		VHalfStep=gain[dacs-1]/2.0;		/* half the smallest step */
		for(i=0;i<COLORS;i++)LP->table[i].value=i;
		LP->dacSize=Log2L(LP->VMax*2-1);	// in bits, rounded up
		if(LP->dacSize!=8)printf(
			"Caution: Your video card seems to have %ld-bit video dacs, and Luminance.c\n"
			"has only been tested with 8-bit dacs.\n",LP->dacSize);
		/* Apple's convention is to provide a 16-bit value to the dac. Luminance.c
		saves time and space by computing values with only as much precision as
		needed for the the video card's dac (i.e. 0 to VMax, or dacSize bits). This
		value must be shifted left by leftShift bits to yield a standard 16-bit value.
		Note that Apple normally recommends scaling the value to fill the entire
		16-bit range, e.g. multiplying an 8-bit value by 0x101, but that doesn't
		seem appropriate here, where we know the dac size, and are more concerned with
		step size that with range.
		*/
		LP->leftShift=16-LP->dacSize;
	}
	else {
		for(i=0;i<DACS;i++){
			gain[i]=LP->gain[i];
			dac[i]=LP->dac[i];
		}
		dacs=LP->dacs;
		VHalfStep=LP->VHalfStep;
	}
	for(i=0;i<DACS;i++)V[i]=0.0;	/* initialize fixed dac voltages */
	
	/* 2. Transform lowLuminance and highLuminance to lowV and highV. */
	lowV=LToV(LP,lowLuminance);		/* takes 0.15 ms */
	highV=LToV(LP,highLuminance);	/* takes 0.15 ms */

	/* 3. Designate the finest dacs as variable until we have enough to cover
	the range, then designate the rest as fixed. (DACs 0..fixed-1 will be fixed.)
	*/
	VGoal=fabs(highV-lowV);
	VSum=0.0;
	fixed=0;
	for(i=dacs-1;i>=0;i--) {
		if(VSum >= VGoal)fixed++;
		VSum += gain[i]*(LP->VMax - LP->VMin);
	}
	/* sum gains of all variable dacs. This is called gVary by Pelli & Zhang (1991) */
	tolerance=0.0;
	for(i=dacs-1;i>=fixed;i--)tolerance+=gain[i];
	/* scale by highest luminance gain in the requested range */
	dL=fabs(VToL(LP,lowV+1.0)-VToL(LP,lowV));
	dL2=fabs(VToL(LP,highV)-VToL(LP,highV-1.0));
	tolerance *= MAX(dL,dL2);

	/* 4. Temporarily set the variable DACs to their midpoints. Now set the fixed DACs
	to most accurately represent (lowV+highV)/2.
	*/
	VGoal=(lowV+highV)/2.0;
	VSum=0.0;
	VFixed=(LP->VMax + LP->VMin)/2.0;
	for(i=fixed;i<dacs;i++) VSum+=gain[i]*VFixed;	/* The mid-voltage of the variable DACs */
	for(i=0;i<fixed;i++) {
		V[i]=floor((VHalfStep+VGoal-VSum)/gain[i]);	/* the unattenuated voltage of DAC i */
		V[i]=MAX(LP->VMin,MIN(LP->VMax,V[i]));
		VSum += V[i]*gain[i];
	}
	VSum=0.0;
	for(i=0;i<fixed;i++) VSum+=gain[i]*V[i];
	VFixed=VSum;
	
	/* 5. The limited precision of the fixed DACs may result in a 
	small offset between the requested and now available range. The offset will be
	at most half a step of the finest of the fixed DACs. It seems reasonable
	to offset the requested luminance range by up to that small amount to better fit
	the now available range. */
	LOffset=0.0;
	if(fixed>0){
		VSum=VFixed;
		for(i=fixed;i<dacs;i++) VSum += LP->VMin*gain[i];
		dV= VSum - MIN(lowV,highV);
		dV= MIN(dV,gain[fixed-1]/2.0);
		if(dV>0.0) LOffset+=VToL(LP,VSum+dV)-VToL(LP,VSum);
		VSum=VFixed;
		for(i=fixed;i<dacs;i++) VSum += LP->VMax*gain[i];
		dV= MAX(lowV,highV) - VSum;
		dV= MIN(dV,gain[fixed-1]/2.0);
		if(dV>0.0) LOffset-=VToL(LP,VSum+dV)-VToL(LP,VSum);
	}
	/* Now save this information in the luminanceRecord for future use */
	for(i=0;i<DACS;i++){
		LP->dac[i]=dac[i];
		LP->gain[i]=gain[i];
	}
	LP->rangeSet=luminanceSet;
	LP->lowLuminance=lowLuminance;
	LP->highLuminance=highLuminance;
	LP->dacs=dacs;
	LP->VHalfStep=VHalfStep;
	LP->fixed=fixed;
	LP->VFixed=VFixed;
	LP->tolerance=tolerance;
	LP->LOffset=LOffset;

	/* Copy into FIXED variables */
	for(i=0;i<DACS;i++)LP->_gain[i]=DoubleToFix(LP->gain[i]);
	LP->_VHalfStep=DoubleToFix(VHalfStep);
	LP->_VFixed=DoubleToFix(VFixed);
	LP->_tolerance=DoubleToFix(tolerance);
	LP->_LOffset=DoubleToFix(LOffset);

	/* cache the fixed DAC values, and zero the rest for good measure */
	RGBArray = (unsigned short *) &LP->rgb;	/* treat as an array */
	for(i=0;i<DACS;i++){
		RGBArray[LP->dac[i]] = (short)V[i]<<LP->leftShift;
	}
	return tolerance;
}

static void Sort(int n,double arr[],int krr[])
/*
Slightly modified sort routine piksr2() from Numerical Recipes in C. I changed
"float" to "double", and I changed second array to int. I reversed the order, so
largest element would be first. I changed it to use conventional c arrays,
starting at index 0.
*/
{
	register int i,j;
	double a;
	int k;

	for(j=1;j<n;j++) {
		a=arr[j];
		k=krr[j];
		i=j-1;
		while (i >= 0 && arr[i] < a) {
			arr[i+1]=arr[i];
			krr[i+1]=krr[i];
			i--;
		}
		arr[i+1]=a;
		krr[i+1]=k;
	}
}
