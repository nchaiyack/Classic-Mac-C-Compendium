#define INDIRECT

/*
 *  new.c - create a new object
 *
 *  Copyright (c) 1991 Symantec Corporation.  All rights reserved.
 *
 */

#define OOPS_PRIVATE
#include "oops.h"

#ifdef INDIRECT
	#define __new __new_indirect
#else
	#define __new __new_direct
#endif

extern void *__new(...);


/*
 *  __new - create a new object
 *
 *  The compiler, for
 *
 *		obj = new C;
 *
 *  generates:
 *
 *			PEA		C
 *			JSR		__new
 *			ADDQ.L	#4,SP
 *			MOVE.L	D0,obj
 *
 *  This routine allocates the object, calling an appropriate "operator new"
 *  method if available, and calls the constructors for the class and each of
 *  its superclasses in turn, root class first.
 *
 *  The new object is returned in D0.  If the object could not be allocated,
 *  0 is returned.
 *
 */

void *
__new()
{
	asm {
		link	a6,#0
		movem.l	d6/d7/a2,-(sp)

;;
;
;  initiate recursive traversal of superclass chain
;
;;

		move.l	8(a6),d6			;  D6 = class Ref
	#ifdef BASE_REG
		sub.l	BASE_REG,d6
	#endif
		moveq	#0,d0				;  D0.L = size (unknown)
		moveq	#0,d7				;  D7 ==> object (unallocated)
		bsr.s	@do_new

;;
;
;  done
;
;;

abort:
		move.l	d7,d0
		movem.l	-12(a6),d6/d7/a2
		unlk	a6
		rts

;;
;
;  The remainder is a recursive routine, "@do_new".
;
;  "@do_new" recursively traverses the superclass chain.  On the way down,
;  it tries to allocate the object using the first allocator it finds; if
;  the root is reached without finding an allocator, the default allocator
;  is used.  If an allocator is called and fails, we jump immediately to
;  "@abort", above, which cleans up the stack, restores registers, and
;  returns 0 without further ado.
;
;  On the way back up, it sets the object's class Ref appropriately and
;  calls any constructors.
;
;;

do_new:

;;
;
;  set A2 ==> class info
;
;;

		movea._	d6,a2
	#ifdef BASE_REG
		adda.l	BASE_REG,a2
	#endif
		moveq	#1,d1
		add.w	(a2)+,d1
		lsl.w	#DSHIFT,d1
		adda.w	d1,a2

;;
;
;  set D0 = number of bytes to allocate
;
;;

		tst.l	d7
		bne.s	@3					;  object already allocated
		tst.l	d0
		bne.s	@1					;  size already determined
		move.l	ClassInfo_(size)(a2),d0			

;;
;
;  allocate object using "operator new" from this class
;
;;

@1		move._	ClassInfo_(allocator)(a2),d1
		beq.s	@2					;  class doesn't have allocator
;
		movea._	d1,a0
	#ifdef BASE_REG
		adda.l	BASE_REG,a0
	#endif
		move.l	d0,-(sp)
		jsr		(a0)
		addq.l	#4,sp
		move.l	d0,d7
		bne.s	@3
		bra.s	@abort				;  object could not be allocated

;;
;
;  allocate object using default allocator
;
;;

@2		move._	ClassInfo_(superclass)(a2),d1
		bne.s	@3					;  haven't reached root class yet
;
	#ifdef INDIRECT
		_NewHandle CLEAR
	#else
		_NewPtr CLEAR
	#endif
		move.l	a0,d7
		beq.s	@abort				;  object could not be allocated

;;
;
;  recurse to superclass
;
;;

@3		movem.l	d6/a2,-(sp)
		move._	ClassInfo_(superclass)(a2),d6
		beq.s	@4					;  reached root
		bsr.s	@do_new
@4		movem.l	(sp)+,d6/a2

;;
;
;  set class Ref in object
;
;;

		movea.l	d7,a0
	#ifdef INDIRECT
		movea.l	(a0),a0
	#endif
		move._	d6,(a0)

;;
;
;  call constructor
;
;;

		move._	ClassInfo_(constructor)(a2),d1
		beq.s	@5					;  class has no constructor
;
		movea._	d1,a0
	#ifdef BASE_REG
		adda.l	BASE_REG,a0
	#endif
		move.l	d7,-(sp)
		jsr		(a0)
		addq.l	#4,sp
@5		;rts
	}
}
