#include "bmminclude.h"
#if MEM_DEBUG>0

/*
 * TCL menu and window interface to memory debugger
 * by Aaron Wohl (n3liw+@cmu.edu)
 * public domain no rights reserved
 */

#include "MemWatchInternal.h"
#include "bbt.h"
#include "MemConfig.h"
#include "CBartender.h"
#include <GestaltEQU.h>

extern CBartender *gBartender;

static unsigned long last_epoch=MEM_FIRST_BLOCK;

/*
 * is tmon running?
 */
static int is_tmon_present(void)
{
	long response;
	int iErr = Gestalt('TMON', &response);
	return ((iErr=0)&&(response!=0));
}

/*
 * is this a developers system we are running on?
 */
static int is_development_system(void)
{
	Str255 vol_name;
	int dsk_len=sizeof(MEM_WATCH_DISK_NAME_TO_ENABLE_DEBUG)-1;
	short vnum;
	GetVol(vol_name,&vnum);
	return ((vol_name[0]==dsk_len)&&
		(memcmp(vol_name+1,MEM_WATCH_DISK_NAME_TO_ENABLE_DEBUG,dsk_len)==0));
}

/*
 * install memory patches if debugging
 */
void mem_MaybeInstallPatch(void)
{
	if(is_development_system()) {
		gBreakFailure = TRUE;
		mem_InstallPatch();
	}
}

/*
 * hook called from applicaiton SetupMenus
 */
void mem_menu_setup_hook(void)
{
	if(!mem_is_debug_on())
		return;
	if(gBartender->FindMenuIndex(MEM_WATCH_MENU_debug)<0)
		gBartender->AddMenu(MEM_WATCH_MENU_debug,TRUE,0);
}

/*
 * update memu hook
 */
void mem_update_menu_hook(void)
{
	long i;
	if(!mem_is_debug_on())
		return;
	gBartender->EnableMenu(MEM_WATCH_MENU_debug);
	for(i=MEM_WATCH_MIN_CMD;i<MEM_WATCH_MAX_CMD;i++)
		gBartender->EnableCmd(i);
}

/*
 * handle debugging commands
 */
int mem_DoCommand(long the_cmd)
{
	if(the_cmd<MEM_WATCH_MIN_CMD)
		return FALSE;
	if(the_cmd>MEM_WATCH_MAX_CMD)
		return FALSE;
	if(!mem_is_debug_on())
		return TRUE;
	switch(the_cmd) {
	case MEM_DUMP_ALL_CMD:	/*dump all of memory*/
		mem_dump_memory_to_file(MEM_WATCH_FILE_NAME,MEM_FIRST_BLOCK);
		break;
	case MEM_EPOCH_CMD:	/*epoch to mark memory allocation*/
		last_epoch=mem_get_epoch();
		break;
	case MEM_DUMP_SINCE_CMD:	/*dump since epoch*/
		mem_dump_memory_to_file(MEM_WATCH_FILE_NAME,last_epoch);
		break;
	}
	return TRUE;
}
#endif
