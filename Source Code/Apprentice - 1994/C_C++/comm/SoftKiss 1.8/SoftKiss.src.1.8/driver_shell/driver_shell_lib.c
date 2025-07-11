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

#include "driver_shell.h"
#include "sfk_os_preserve.h"
#include <string.h>

/*
 * prepare a driver template to be a certain driver
 * returns true if successfull
 */
int sh_prepare(
	driver_shell_header_pt ads,
	unsigned char *aname,
	short aFlags,
	short aDelay,
	long acall_me,
	long aiba4,
	long aiba5,
	long ash_more)
{
	if(ads->call_me!='call')return FALSE;
	if(ads->iba4   !='iba4')return FALSE;
	if(ads->iba5   !='iba5')return FALSE;
	if(aname[0]==0)return FALSE;
	if(aname[0]>(ds_NAME_LEN-1))return FALSE;

/*the template was ok so now fill it in*/
	memcpy(ads->drvrName,aname,aname[0]+1);
	ads->call_me=acall_me;
	ads->iba4=aiba4;
	ads->iba5=aiba5;
	ads->sh_more=ash_more;
	ads->drvrFlags=aFlags;
	ads->drvrDelay=aDelay;
	return TRUE;
}

/*
 * return a5 for driver_shell to set when it calls us back
 * a5 is used when this module is linked to a test application
 * normaly we are part of a driver and a4 is used
 */
long sh_geta5()
{
	asm { move.l a5,d0 }
}

/*
 * return a4 for driver_shell to set when it calls us back
 */
long sh_geta4()
{
	asm { move.l a4,d0 }
}

/*
 * strip address that strips even before 32bit addressing is on
 * so the address will still be valid later
 */
void *sh_strip_address(void *addr)
{
	asm {
	move.l addr,d0
	and.l Lo3Bytes,d0
	}
}

/*
 * install the passed driver
 * apple never got around to the glue for this call
 * if they ever do this routine should be removed
 */
pascal void _DrvrInstall(void) = 0xa03d;
short DrvrInstall(Handle drvrHandle, short dRef)
{
	asm {
		move.w dRef,d0
		move.l drvrHandle,a0
		move.l (a0),a0
	}
	_DrvrInstall();
}

/*
 * remove the passed device driver
 * apple never got around to the glue for this call
 * if they ever do this routine should be removed
 */
pascal void _DrvrRemove(void) = 0xa03e;
short DrvrRemove(short refNum)
{
	asm {
		move.w refNum,d0
		_DrvrRemove
	}
}

/*
 * Drvr install doesn't do anything except fill in dCtlRefNum
 * so follow up and copy over the drivers info.
 * Handle passed in should be locked
 */
short Real_DrvrInstall(Handle DrvrHandle, short dRef)
{
	int oserr;
	DCtlHandle dCtl;
	driver_shell_header_pt ash= (driver_shell_header_pt)(*DrvrHandle);
	if((oserr=DrvrInstall(DrvrHandle,dRef))!=0)
		return oserr;
	dCtl=GetDCtlEntry(dRef);
	(*dCtl)->dCtlDriver=(Ptr) DrvrHandle;	/*use handle, tech note is wrong*/
	(*dCtl)->dCtlStorage=(Handle) 0L;
	(*dCtl)->dCtlDelay=ash->drvrDelay;
	(*dCtl)->dCtlEMask=ash->drvrEMask;
	(*dCtl)->dCtlFlags|=ash->drvrFlags;
	return 0;
}

/*
 * a resource into the system heap by name
 * returns nil if it can't be loaded
 */
Handle sh_read_in_sys_res(ResType id,unsigned char *name)
{
	Handle res=0;
	int needs_release=TRUE;
	if((res=GetNamedResource(id,name))==0)
		return 0;
	HUnlock(res);
	if(MemError()!=0)
		goto error_exit;
	DetachResource(res);
	if(ResError()!=0)
		goto error_exit;
	/*detached now so if dail need to dispose not release*/
	needs_release=FALSE;
	MoveHHi(res);
	if(MemError()!=0)
		goto error_exit;
	HLock(res);
	if(MemError()!=0)
		goto error_exit;
	return res;

error_exit:
	if(needs_release)
		ReleaseResource(res);
	else
		DisposHandle(res);
	return 0;
}

/*
 * load the driver template into memory
 * returns nil if it can't be loaded
 */
Handle sh_read_in_driver_shell(void)
{
	return sh_read_in_sys_res('PROC',"\pdriver_shell");
}

/*
 * return a duplicate of the passed handle
 * the copy will be in the system heap
 * and moved high and locked
 * must be called with the system heap zone set
 */
static Handle sh_copy_templatei(Handle template)
{
	long template_size;
	Handle result=0;
	if(template==0)
		return 0;
	template_size=GetHandleSize(template);
	if(template_size==0)
		return 0;
	if((result=NewHandleSys(template_size))==0)
		return 0;
	MoveHHi(result);
	if(MemError()!=0)
		goto error_exit;
	HLock(result);
	if(MemError()!=0)
		goto error_exit;
	memcpy(*result,*template,template_size);
	return result;

error_exit:
	DisposHandle(result);
	return 0;
}

/*
 * return a duplicate of the passed handle
 * the copy will be in the system heap
 * and moved high and locked
 */
Handle sh_copy_template(Handle template)
{
	return (Handle)OSP_protected_call(OSP_sys,sh_copy_templatei,template,0,0,0);
}
