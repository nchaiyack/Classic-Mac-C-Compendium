/*
 * get/save resource manager state
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#define NO_OSP_DEF
#include "sfk_os_preserve.h"

/*
 * call a routine possibly switching to the system heap
 * take up to 4 arguments which are passed to the routine
 * to call.
 * The reserr proc in low memory is replaces so that if passed
 * function gets a resource error it won't call the installed
 * reserr procedure.
 */
long OSP_protected_call(
	int new_heap,
	long (*cproc)(long a1,
		long a2,
		long a3,
		long a4),
	long a1,
	long a2,
	long a3,
	long a4);
long OSP_protected_call(
	int new_heap,
	long (*cproc)(long a1,
		long a2,
		long a3,
		long a4),
	long a1,
	long a2,
	long a3,
	long a4)
{
	THz oldZone;
	long result;
	ProcPtr old_res_err=ResErrProc;
	int old_err_num=ResErr;
	int old_load;
	asm {
		bra.s @skip_rts
	new_err_proc:
		rts
	skip_rts:
		lea @new_err_proc,a0
		move.l a0,ResErrProc
	}
	oldZone = GetZone();
	if((new_heap&OSP_sys)!=0)
		SetZone(SystemZone());
	old_load=ResLoad;
	SetResLoad(((new_heap&OSP_noload)==0));
	result=(*cproc)(a1,a2,a3,a4);
	SetResLoad(old_load);
	if((new_heap&OSP_sys)!=0)
		SetZone(oldZone);
	ResErrProc=old_res_err;
	ResErr=old_err_num;
	return result;
}
