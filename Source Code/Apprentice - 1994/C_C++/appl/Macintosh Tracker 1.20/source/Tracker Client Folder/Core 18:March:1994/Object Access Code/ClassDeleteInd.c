#define INDIRECT

/*
 *  delete.c - destroy an object
 *
 *  Copyright (c) 1991 Symantec Corporation.  All rights reserved.
 *
 */

#define OOPS_PRIVATE
#include "oops.h"

#ifdef INDIRECT
	#define __delete __delete_indirect
#else
	#define __delete __delete_direct
#endif

extern void __delete(...);



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
 *  __delete - destroy an object
 *
 *  The compiler, for
 *
 *		delete obj;
 *
 *  generates:
 *
 *			MOVE.L	obj,-(SP)
 *			JSR		__delete
 *			ADDQ.L	#4,SP
 *
 *  This routine calls the destructors for the class and each of its superclasses
 *  in turn, root class last, then deallocates the object, calling an appropriate
 *  "operator delete" method if available.
 *
 */

void
__delete()
{
	asm {
		link	a6,#0
		movem.l	d5/d6/d7/a2,-(sp)
		moveq	#0,d5				;  D5 = deallocator Ref (unknown)

;;
;
;  set D7 = object
;  set D6 = class Ref
;
;;

		move.l	8(a6),d7
		beq.s	@5					;  object is 0
#ifdef MEMDEBUG
		movem.l	d0/a0,-(sp)
		move.l	d7,d0  ;copy object over to be inspected
		move.l	messagetestfuncptr,a0
		jsr		(a0)
		movem.l	(sp)+,d0/a0
#endif
		movea.l	d7,a0
	#ifdef INDIRECT
		movea.l	(a0),a0
	#endif
		move._	(a0),d6

;;
;
;  set A2 ==> class info
;
;;

@1		movea._	d6,a2
	#ifdef BASE_REG
		adda.l	BASE_REG,a2
	#endif
		moveq	#1,d0
		add.w	(a2)+,d0
		lsl.w	#DSHIFT,d0
		adda.w	d0,a2

;;
;
;  determine deallocator to use
;
;;

		tst._	d5
		bne.s	@2					;  already have one
		move._	ClassInfo_(deallocator)(a2),d5

;;
;
;  set class Ref in object
;
;;

@2		movea.l	d7,a0
	#ifdef INDIRECT
		movea.l	(a0),a0
	#endif
		move._	d6,(a0)

;;
;
;  call destructor
;
;;

		move._	ClassInfo_(destructor)(a2),d1
		beq.s	@3					;  class has no destructor
;
		movea._	d1,a0
	#ifdef BASE_REG
		adda.l	BASE_REG,a0
	#endif
		move.l	d7,-(sp)
		jsr		(a0)

;;
;
;  advance to superclass
;
;;

@3		move._	ClassInfo_(superclass)(a2),d6
		bne.s	@1

;;
;
;  deallocate object using "operator delete" from appropriate class
;
;;

		tst._	d5
		beq.s	@4					;  no deallocator
;
		movea._	d5,a0
	#ifdef BASE_REG
		adda.l	BASE_REG,a0
	#endif
		move.l	d7,-(sp)
		jsr		(a0)
		bra.s	@5

;;
;
;  deallocate object using default deallocator
;
;;

@4		movea.l	d7,a0
	#ifdef INDIRECT
		_DisposHandle
	#else
		_DisposPtr
	#endif

;;
;
;  done
;
;;

@5		movem.l	-16(a6),d5/d6/d7/a2
		unlk	a6
	}
}
