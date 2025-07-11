#include "bmminclude.h"
#if MEM_DEBUG==0
#include "BMemWatch.h"
/*
 * dummy routines for version without memory debugging
 * by Aaron Wohl (n3liw+@cmu.edu)
 * public domain no rights reserved
 */

/*
 * describe what a handle is
 */
void mem_describe_handle(const void *mem_blk,const CObject *owner,const char *description)
{
#ifdef CODECHECK
	UNUSED_ARG(mem_blk);
	UNUSED_ARG(owner);
	UNUSED_ARG(description);
#endif
}

/*
 * describe what a pointer is is
 */
void mem_describe_ptr(const void *mem_blk,const CObject *owner,const char *description)
{
#ifdef CODECHECK
	UNUSED_ARG(mem_blk);
	UNUSED_ARG(owner);
	UNUSED_ARG(description);
#endif
}

/*
 * record the birth of a new object
 * called with c linkage
 */
void mem_record_birth(void)
{
}


/*
 * install memory patches if debugging
 */
void MaybeInstallMemWatchPatch()
{
}


/*
 * hook called from applicaiton SetupMenus
 */
void mem_menu_setup_hook(void)
{
}

/*
 * update memu hook
 */
void mem_update_menu_hook(void)
{
}

/*
 * handle debugging commands
 */
int mem_DoCommand(long the_cmd)
{
#ifdef CODECHECK
	UNUSED_ARG(the_cmd);
#endif
	return FALSE;
}

/*
 * remove memory allocation watching hooks
 */
void RemoveMemWatchPatch(void)
{
}

/*
 * record the birth of a new object
 * called with c linkage
 */
void record_birth_c(CObject *new_object)
{
#ifdef CODECHECK
	UNUSED_ARG(new_object);
#endif
}
#endif
