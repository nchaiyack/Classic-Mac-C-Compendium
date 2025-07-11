#define INDIRECT

/*
 *  msg.c - message dispatcher
 *
 *  Copyright (c) 1991 Symantec Corporation.  All rights reserved.
 *
 */

#define OOPS_PRIVATE
#include "oops.h"

#ifdef INDIRECT
	#define __msg __msg_indirect
#else
	#define __msg __msg_direct
#endif

extern void __msg(...);



#ifdef MEMDEBUG
/* some extra debugging code */

#include "Memory.h"

static void MyTestFunc(void);

static void* messagetestfuncptr = MyTestFunc;

static void MyTestFunc(void)
	{
		void*			Temp;

		asm
			{
				move.l		d0,Temp
			}
		CheckHandleExistence((Handle)Temp);
	}
#endif

/*
 *  __msg - message dispatcher
 *
 *  The compiler, for
 *
 *		obj->method(args)
 *
 *  generates:
 *
 *			<<push args>>
 *			MOVE.L	obj,-(SP)
 *			JSR		selector
 *			...
 *
 *  	selector:
 *			JSR		__msg
 *			JMP		(A0)
 *
 *  The dispatcher identifies which message was sent by examining the
 *  "JSR selector" instruction.  It determines which method should be
 *  invoked by searching the dispatch table of the object's actual
 *  class, and, as necessary, its superclasses.
 *
 *  The address of the method to be called is returned in A0.
 *
 */

void
__msg()
{
	asm {

;;
;
;  set D1 = selector Ref, A1 ==> object
;
;;

		movea.l	4(sp),a1
		move._	-(a1),d1
		movea.l	8(sp),a1
#ifdef MEMDEBUG
		movem.l	d0/a0,-(sp)
		move.l	a1,d0  ;copy object over to be inspected
		move.l	messagetestfuncptr,a0
		jsr		(a0)
		movem.l	(sp)+,d0/a0
#endif
	#ifdef DEBUG
		move.l	a1,d0
		beq.s	@1
		lsr.b	#1,d0
		bcc.s	@2
@1		jmp		__noObject			;  object is 0 or odd
	#endif
@2
	#ifdef INDIRECT
		movea.l	(a1),a1
	#endif INDIRECT

;;
;
;  set A1 ==> dispatch table
;
;;

@3		movea._	(a1),a1
	#ifdef DEBUG
		move.l	a1,d0
		bne.s	@4
		jmp		__noMethod			;  method not found
	#endif
@4
	#ifdef BASE_REG
		adda.l	BASE_REG,a1
	#endif

;;
;
;  search dispatch table
;
;;

		move.w	(a1)+,d0			;  D0.W = #methods - 1
		bmi.s	@3					;  (no methods defined)
@5		movea._	(a1)+,a0			;  A0 = method Ref
		cmp._	(a1)+,d1			;  match selector Ref
		dbeq	d0,@5
		bne.s	@3					;  not found - search superclass

;;
;
;  done
;
;;

	#ifdef BASE_REG
		adda.l	BASE_REG,a0
	#endif

	}
}
