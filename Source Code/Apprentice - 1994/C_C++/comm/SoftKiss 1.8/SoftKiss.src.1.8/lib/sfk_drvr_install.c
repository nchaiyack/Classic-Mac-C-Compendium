/*
 * install softkiss driver
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core.h"
#include "sfk_core_private.h"
#include "driver_shell.h"
#include "sfk_drvr_find.h"
#include "sfk_os_preserve.h"

/*
 * find the softkiss driver
 * if it is not present, install it
 */
static short sfk_drvr_find_or_installi(short *refnum)
{
	short new_refnum;
	Handle shell_template=0;
	Handle sfk_header=0;
	Handle sfk_body=0;
	unsigned char *sfk_body_ptr;
	driver_shell_header_pt ash;
	if(((*refnum)=sfk_drvr_find(sfk_driver_name,&new_refnum))!=0)
		return 0;
	if((shell_template=sh_read_in_driver_shell())==0)
		goto error_exit;
	if((sfk_header=sh_copy_template(shell_template))==0)
		goto error_exit;
	ash= sh_strip_address(*sfk_header);
	if((sfk_body=sh_read_in_sys_res('PROC',sfk_driver_resource))==0)
		goto error_exit;
	sfk_body_ptr=sh_strip_address(*sfk_body);
	if(!sh_prepare(ash,sfk_driver_name,
			dNeedLock|dCtlEnable|dRAMBased|dNeedTime,
			2,
			(long)sfk_body_ptr,
			(long)sfk_body_ptr,
			(long)sfk_body_ptr,
			(long)shell_template))
				goto error_exit;
	if(Real_DrvrInstall(sfk_header,new_refnum)) 
		goto error_exit;
	return new_refnum;

error_exit:
	if(sfk_body!=0)
		DisposeHandle(sfk_body);
	if(sfk_header!=0)
		DisposeHandle(sfk_header);
	if(shell_template!=0)
		DisposeHandle(shell_template);
	return 0;
}

/*
 * find the softkiss driver
 * if it is not present, install it
 */
short sfk_drvr_find_or_install(short *refnum)
{
	return OSP_protected_call(OSP_sys,sfk_drvr_find_or_installi,refnum,0,0,0);
}
