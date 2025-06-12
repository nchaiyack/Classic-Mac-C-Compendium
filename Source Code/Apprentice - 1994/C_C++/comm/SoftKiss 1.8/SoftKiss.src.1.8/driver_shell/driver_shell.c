/*
 * driver_shell - code for dynamicly installing a driver
 * at runtime
 * SoftKiss
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

typedef char **Handle;
typedef long ResType;

#include <SysEqu.h>
#include "driver_shell.h"

void main()
{
  	asm {
@drvrFlags	dc.w 0
@drvrDelay	dc.w 0
@drvrEMask	dc.w 0
@drvrMenu	dc.w 0

@drvrOpen	dc.w @open_entry+8
@drvrPrime	dc.w @prime_entry+10
@drvrCtl	dc.w @ctl_entry+12
@drvrStatus	dc.w @status_entry+14
@drvrClose	dc.w @close_entry+16

/*drvrname,call_me,a4a5, and extra arg are patched when we are installed*/
@drvrName				/*space for 48char name*/
	dc.l 0,0,0,0
	dc.l 0,0,0,0
	dc.l 0,0,0,0

@call_me
	dc.l 'call'			/*routine to call*/
@a4a5
	dc.l 'iba4','iba5'	/*values to load into a4 and a5*/
@extra_arg
	dc.l	'more'		/*extra arg for call_me to tell which driver this is*/
/*8 bytes reserved for future expansion*/
	dc.l 0,0

@strip_pc
//	dc.w 0xA9FF		/*debugger trap*/
	move.l Lo3Bytes,d0
	and.l d0,(sp)
	rts

@prime_entry
	moveq.l #sh_PRIME,d0
	bra.s @pr_ct_st_common
@ctl_entry
	moveq.l #sh_CONTROL,d0
	bra.s @pr_ct_st_common
@status_entry	
	moveq.l #sh_STATUS,d0

@pr_ct_st_common
	jsr @call_callme
	cmpi.w #1,d0		/*still going on?*/
	beq.s @ct_not_done
	move.w d0,16(a0)	/*done so set error code*/
	btst #1,6(a0)		/*immediate? check ioTrap*/
	bne.s @ct_com_exit	/*yes so normal exit*/
	move.l JIODone,-(a7) /*not immidiate and are done*/
	rts

@ct_not_done
	move.w #0,d0		/*still working on it*/
@ct_com_exit
	rts

@open_entry
	moveq.l #sh_OPEN,d0
	bra.s @call_set_result

@close_entry	
	moveq.l #sh_CLOSE,d0

@call_set_result
	jsr @call_callme
	move.w d0,16(a0)	/*done so set error code*/
	rts

@call_callme
 	movem.l a0/a1/a4-a5,-(a7) /*save volitiles and global ptrs*/
  	movem.l @a4a5,a4/a5	/*setup users global regs*/
  	move.l @extra_arg,-(a7)
	move.w d0,-(a7)
	move.l a1,-(a7)		/*dce*/
	move.l a0,-(a7)		/*iopb*/
	move.w #1,16(a0)	/*set io block busy*/
	move.l @call_me,a1	/*call user driver code*/
	jsr	@strip_pc
	jsr (a1)
	add #14,a7
	movem.l (a7)+,a0/a1/a4-a5
	} /*run into 'free' rts compiler gives us*/
}
