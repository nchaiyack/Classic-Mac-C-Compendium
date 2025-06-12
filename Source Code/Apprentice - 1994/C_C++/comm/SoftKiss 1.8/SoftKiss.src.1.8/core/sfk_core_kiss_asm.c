/*
 * SoftKiss - kiss mode emulator
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core.h"
#include "sfk_core_private.h"

/*
 * turn off prototype synce asm routines need to lie about there arguments
 * in order to ovid being given a stack frame by the compiler
 */
#pragma options(!require_protos)

enum { JIODone = 0x8FC  };      /*[GLOBAL VAR] Jump vector for IODone function*/

/*
 * call the os routine jiodone
 * static void do_jiodone(kiss_port_pt p)
 */
void rp_jiodone(void)
{
	asm {
		move.l 4(sp),a1
#ifdef NOT_NEEDED
		move.l sfk_prt.krp_pb(a1),a0	/*iopb to finish*/
#endif
		clr.l kiss_port.kp_pb(a1)		/*mark it done*/
		move.l kiss_port.kp_de(a1),a1	/*dce for device*/
		move.l (a1),a1
		move.l JIODone,-(a7) 			/*not immidiate and are done*/
	}
}
