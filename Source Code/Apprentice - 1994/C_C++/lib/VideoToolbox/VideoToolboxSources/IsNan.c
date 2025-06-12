/* IsNan.c
Fast, portable routines to check for IEEE transfinite numbers: INF and NAN. You
should also look at the macro IsFinite() in VideoToolbox.h.

IsNan and IsInf assume that every NAN's most significant mantissa bit is set,
which is apparently an informal convention supported by the Intel and Motorola
math chips. The definition of a NAN in the Apple Numerics book only says that a
NAN is distinguished from an INF by having a nonzero mantissa. Unfortunately, a
strict test of this would require testing all 8 bytes, and in fact the routines
in Plaugher's The Standard C Library do exactly this. However, this seems
wasteful since the 255 different NANs that can be specified by the one byte NAN
type are obviously more than enough for most programs. Indeed, since the
Motorola chip doesn't preserve NAN type, the number of NAN types is rather
academic; a single NAN type seems to be plenty for most applications. While it
is possible to assemble a NAN with zero type, which is preserved through 8881
operations, and which printf("%f") prints as NAN00, the SANE routines all seem
to immediately convert the zero type NAN to type 4, so IsNan() does too.

The assert() tests referring to sizeof(double) are evaluated by the compiler,
with no runtime penalty.

The Numerical Extensions to C group has proposed in their draft standard that
routines very similar to these become a part of Standard C. Anyone interested in
writing such routines should read: Plauger, P. J. (1992) The Standard C Library.
Englewood Cliffs, NJ: Prentice Hall.

PORTABILITY: Standard C. Should work on Motorola and Intel processors, but has
only been tested on 680x0 processors in Macintosh computers.

HISTORY:
8/24/91 dgp	made compatible with THINK C 5.0.
12/23/91 dgp I replaced the #if statements by ordinary if statements, which are 
	more readable and are allowed to use the sizeof() operator.
	Note that most of the if statements will be evaluated and removed by the
	compiler, with no runtime penalty.
12/23/91 dgp Wrote my own code to replace the SANE code since it's too slow.
	This makes IsNan() about 5 times faster.
	My code is based on the Apple Numerics Manual, 2nd edition. It says that
	a number is a NAN iff the exponent is all ones and the fraction is nonzero.
	I make a slight shortcut in checking only the top 15 bits of the fraction,
	since that includes the byte that specifies the NAN type, on the premise
	that all NANs that I will actually see in practice will have nonzero type.
	My code handles the ordinary cases of 10 or 12 byte doubles. The weird
	case of shorter doubles (which are unlikely since they run very slowly)
	are detected by the assert() test at the beginning.
12/23/91 dgp Asked the THINK C compiler not to time this routine.
12/29/91 dgp Eliminated the need for Sane.h and Types.h. The sane stuff
	now appears in its own file: Sane.c. 
	Wrote IsInf().
	Wrote a new macro definition, in VideoToolbox.h, for IsFinite(), 
	that allows fast inline testing for whether a number is ok, i.e. neither 
	NAN nor INF. 
1/14/92	dgp	Changed IsNan() to now return the type (1..255) of the NAN, or zero
	if not a NAN. This will break programs that assume the true value is always 1,
	e.g. nans+=IsNan(a);
	Fixed IsInf() to correctly return sign of ±INF.
1/18/92	dgp	Rewrote routines, making them simpler, and always checking the most 
	significant bit of the mantissa in testing for NAN.
6/5/93	dgp	Updated documentation.
*/

/* #include "VideoToolbox.h" */
#include <assert.h>
#pragma options(!profile)	/* THINK C: attribute to the caller the time spent here. */
#define EXPONENT 0
#define MANTISSA (1+(sizeof(double)-10)/sizeof(short))
int IsNan(double x);
int IsInf(double x);

int IsNan(double x)
/* Returns x's NAN type (1...255) or zero if x is not a NAN. */
/* If NaN type is zero, return 4, as do the Apple routines. */
{
	register short i;

	assert(sizeof(double)==10 || sizeof(double)==12);
	if((((short *)&x)[EXPONENT] & 0x7FFF)==0x7FFF){		/* either NAN or INF */
		i=((short *)&x)[MANTISSA] & 0x7FFF;
		if(i==0)return 0;
		i&=0xFF;
		if(i!=0)return i;
		else return 4;
	} else return 0;
}

int IsInf(double x)
/* Returns -1 for -INF, 0 for not INF, and +1 for +INF. */
{
	register short i;

	assert(sizeof(double)==10 || sizeof(double)==12);
	i=((short *)&x)[EXPONENT];
	if((i & 0x7FFF)==0x7FFF){							/*  either NAN or INF */
		if((((short *)&x)[MANTISSA] & 0x7FFF)==0){
			if(i<0)return -1;							/*  -INF */
			else return 1;								/*  +INF */
		}
	}
	return 0;
}
