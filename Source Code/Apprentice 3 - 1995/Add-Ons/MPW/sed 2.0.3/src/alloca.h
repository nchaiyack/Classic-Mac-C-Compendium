/*
 *	alloca.h
 *
 *	For use with MPW C
 *
 *	Based on "alloca.a" found in an old port of GNU e?grep
 *	Franklin Chen
 *	Fri, Apr 16, 1993
 */

void *alloca (unsigned int size) =
  {
    0x201F, /*	move.l	(sp)+,d0	; pop parameter = size in bytes */
    0x5680, /*	addq.l	#3,d0		; round size up to long word */
    0x0280,
    0xFFFF,
    0xFFFC, /*	andi.l	#-4,d0		; mask out lower two bits of size */
    0x9FC0, /*	suba.l	d0,sp		; allocate by moving stack pointer */
    0x200F, /*	move.l	sp,d0		; return pointer */
    0x598F  /*	subq.l	#4,sp		; new top of stack */
  };
