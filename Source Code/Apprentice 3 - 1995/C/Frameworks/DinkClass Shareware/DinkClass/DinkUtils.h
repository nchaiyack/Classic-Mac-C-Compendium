/*
	File:		DinkUtils.h

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <3>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <2>	 9/20/92	MTG		Bringing the C++ code up to date with the THINKC version

	To Do:
*/

// these are the prototypes for the util fuctions
// used in DinkClass
#include <AppleEvents.h>


void InitToolBox(int numMoreMasers);
Boolean System7Available(void); 
TrapType	GetTrapType(short theTrap);
short	NumToolboxTraps(void);
Boolean TrapExists(short theTrap);

// apple event stuff

OSErr RequiredCheck( AppleEvent *theAppleEvent);

//Str255 stuff

void	pstrcpy(Str255 dest, Str255 src);
void	pstrcat(Str255 dest, Str255 src);


#define HiWrd(aLong)	(short) (((aLong) >> 16) & 0xFFFF)
#define LoWrd(aLong)	(short) ((aLong) & 0xFFFF)

#define Min(A,B)		((A) < (B) ? (A) : (B) )
#define Max(A,B)		((A) < (B) ? (B) : (A) )

#define ZERO 0L

//
// macros taken from CShell demo
//

/* define our own abs() so we don't need StdLib */
#define abs(val) (((val) < 0) ? (-(val)) : (val))
/* Pascal string copy */
#define PStrCopy(dest, src)	(BlockMove (src, dest, (*(char *)(src))+1))

#define TopLeft(r)		(* (Point *) &(r).top)
#define BotRight(r)		(* (Point *) &(r).bottom)

#define SETPT(pt, x, y)	(*(pt)).h = (x); (*(pt)).v = (y)
#define SETRECT(r, left, top, right, bottom)	\
						SETPT(&TopLeft(*(r)), (left), (top)); \
						SETPT(&BotRight(*(r)), (right), (bottom))
/* 
 *	Useful macros for testing gestalt attribute responses
 *
 *	BTstBool returns a true boolean value (0 or 1), but is slower than:
 *	BTstQ which simply returns a non-zero value if the bit is set which
 *	means the result could get lost if assigned to a short, for example.
 *
 *	arg is any integer value, bitnbr is the number of the bit to be tested.
 *	bitnbr = 0 is the least significant bit.
 */
#define BTstBool(arg, bitnbr)	((arg >> bitnbr) & 1)
#define BTstQ(arg, bitnbr)		(arg & (1 << bitnbr))

