/*
 * mmc_macmail interface to to turn off/on mailcheck while macmail is running
 * should be called every five min or so to inhibit mailcheck
 */

#include "mmc_macmail.h"
#include "mmc_drvr_find.h"
#include "mmc_core.h"
#include "mmc_prep.h"
#include "string.h"

/*
 * warning, for debugging this routine is nearly duplicated in mmc_test.c
 * be sure to fix bugs there too
 */
void mmc_macmail(long want_inhibit,char *name)
{
	int ref_num;
	int len;
	mmc_state s;
	mmc_init_state(&s);
	ref_num=mmc_drvr_find(MMC_name,0L);		/*find it if present*/
	if(ref_num==0)				/*not running so nothing to tell it*/
		return;
	if(want_inhibit!=MMC_normal)
		s.mmc_st|=MCS_snooze;
	len=imax(0,imin(*name,MMC_uname_len-1));
	if(len!=0)
		memcpy(s.mmc_uname,name+1,len);
	s.mmc_uname[len]=0;
	mmc_set_drvr_state_sound_no(ref_num,&s,MMC_setmmuser);
}
