#include "UseAsm.h"
#ifndef	__TOOLUTILS__
#include <ToolUtils.h>
#include <FixMath.h>
#endif


#if __MWERKS__
#       pragma require_prototypes off
#elif THINK_C || THINK_CPLUS
#       pragma options(!require_protos)
#endif


/**/


#if !USE_ASM


ASM_FUNC long mc68k_fixmul(/*long a, long b*/)
{
  ASM_BEGIN
	//-------------------------
	//
	// Routine       : m68k_fixmul -- 16:16 fixed-point multiply
	// Registers used: d0-d2
	// REQUIRES A 68020
	//

	//-------------------------
	//
	// Pickup parameters...
	//
	move.l	4(A7), d0
	move.l	8(A7), d1
	dc.w	0x4c01, 0x0c01	// muls.l d1, d1:d0
	move.w	d1, d0
	swap	d0
  ASM_END
}


// Quickie -- use FracMul instead of rolling my own...
ASM_FUNC long mc68k_fixmul2_30(/*long a, long b*/)
{
	// return FracMul(a, b);
	// requires 68020!

  ASM_BEGIN

	move.l	4(A7), d0
	move.l	8(A7), d1
	dc.w	0x4c01, 0x0c01	// muls.l d1, d1:d0

	// we could probably just do a "move.l d1, d0" here
	// and lose the lower 2 bits, but...

	lsl.l	#2, d1			// shift 'em back up;
	rol.l	#2, d0			// shift top 2 bits of D0 down to bottom
	and.l	#3, d0			// clear out all other bits
	or.l	d1, d0			// or in the "real" result!

  ASM_END
}


ASM_FUNC long mc68k_fixdiv(/*long num, long denom*/)
{
  ASM_BEGIN
	//-------------------------
	//
	// Routine       : mc68k_fixdiv -- 16:16 fixed-point divide
	// Registers used: d0-d2
	// REQUIRES A 68020
	//

	move.l	4(A7), d2		// numerator
	move.l	8(A7), d1		// denominator
	beq.s	@bad			// divide by zero -- oops!

	move.w	d2, d0			// split d2 between d2 and d0
	swap	d0
	clr.w	d0
	swap	d2
	ext.l	d2
	dc.w	0x4C41, 0x0C02	// divs.l	d1, d2:d0
	bra.s	@end
@bad:
	moveq	#-2, d0
	ror.l	#1, d0			// sneakily get 0x7FFFFFFF into D0.L
	tst.l	d2
	bgt.s	@end			// if numerator > 0, return 0x7FFFFFFF
	neg.l	d0				// return 0x80000001
@end:

  ASM_END
}

#else	/* USE_ASM */

long mc68k_fixmul(long a, long b)
{
	return FixMul(a, b);
}

long mc68k_fixmul2_30(long a, long b)
{
	return FracMul(a, b);
}

long mc68k_fixdiv(long num, long denom)
{
	return FixDiv(num, denom);
}

#endif	/* USE_ASM */
