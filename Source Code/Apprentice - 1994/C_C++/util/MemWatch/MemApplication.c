/*
 * Application object with commands for memory debugging
 * by Aaron Wohl (n3liw+@cmu.edu)
 * public domain no rights reserved
 */

#include "MemApplication.h"
#include "MemWatchInternal.h"

/*
 * install the memwatch debuggin menu if debugging
 */
void MemApplication::SetUpMenus(void)
{
	inherited::SetUpMenus();
	mem_menu_setup_hook();
}

/*
 * install the memory recording patches now if not done earlyer
 */
void MemApplication::InstallPatches(void)
{
	inherited::InstallPatches();
	mem_MaybeInstallPatch();
}

/*
 * enable the memory debugging menu if it is installed
 */
void MemApplication::UpdateMenus(void)
{
	inherited::UpdateMenus();
	mem_update_menu_hook();
}

/*
 * clean up patches on exit
 */
void MemApplication::RemovePatches(void)
{
	inherited::RemovePatches();
	mem_RemovePatch();
}

/*
 * handle memory debugging commands
 */
void MemApplication::DoCommand(long theCommand)
{
	if(!mem_DoCommand(theCommand))
		inherited::DoCommand(theCommand);
}
