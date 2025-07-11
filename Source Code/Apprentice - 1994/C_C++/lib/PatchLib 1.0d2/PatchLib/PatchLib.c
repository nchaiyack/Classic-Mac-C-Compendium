/* See the file Distribution for distribution terms.
	(c) Copyright 1994 Ari Halberstadt */

/*	�Patch Library

	Patch Library is used to manage patches to traps. Installing and removing
	patches is simpler than using the Toolbox routines NSetTrapAddress and
	NGetTrapAddress. In addition, macros are provided that setup and restore
	the environment for the patch routine. C source code is provided. */

/*	Revision History:

	94/04/18 aih -	Added some more comments and made it a stand-alone library
	94/03/02 aih -	Removed almost all of the assembly language glue since it:
						wasn't really needed; was overly complex; and wouldn't
						compile in native PowerPC mode. Now there's just a small
						bit of assembly language glue in a couple of macros.	
	94/02/03 aih -	removed ExitToShell patch since it isn't needed if the
						application just calls PatchEndAll before exiting and
						the fewer patches we install the better	
	93/12/22 aih -	uses NewPtrClear/DisposPtr instead of calling MemoryLib
						functions, so that we don't need to depend on or intialize
						MemoryLib
	93/11/22 aih -	totally rewrote so it no longer uses a self-modifying
						code resource to save the original address of the trap,
						instead the code and data about a patch are stored in a
						non-relocatable block
	93/11/20 aih -	modified so patch can prevent execution of original trap
	93/03/26 AIH -	Changed PatchType to void* instead of pointer to function
						returning void.
	91/05/15 AIH -	Added a few comments
	91/04/19 AIH -	The trap type is determined at run-time, so there's no
						need to pass the trap type as a parameter to the patch
						install function.
	91/03/23 AIH -	StripAddress is called before calling NSetTrapAddress.
						This probably is not needed, but TN#213 recommends calling
						StripAddress before patching traps (ok, only for traps
						that start at a block, if I read it correctly), but the
						issue is so confusing it seems simplest just to call
						StripAddress.
	91/02/28 aih -	Added default definitions for parameter declarations
	91/01/18 aih -	Wrote this generic patch handling function/macro
						combination. */

#include <Memory.h>
#include "PatchLib.h"

/* list of patches */
static PatchType gPatches;

/*	GetTrapType returns the type of the trap (ToolBox or Operating System). */
static TrapType GetTrapType(short trap)
{
	return((trap & 0x0800) > 0 ? ToolTrap : OSTrap);
}

/*	PatchInsert inserts the patch into the list of patches, and returns the
	new head of the list. */
static PatchType PatchInsert(PatchType list, PatchType patch)
{
	patch->next = list;
	return(patch);
}

/*	PatchDelete removes the patch from the list of patches, and returns the
	new head of the list. */
static PatchType PatchDelete(PatchType list, PatchType patch)
{
	PatchType p, prev;

	prev = NULL;	
	for (p = list; p && p != patch; p = p->next)
		prev = p;
	if (p) {
		if (prev)
			prev->next = p->next;
		else
			list = p->next;
	}
	return(list);
}

/*	�PatchInstall installs the patch. */
void PatchInstall(PatchType patch)
{
	if (! patch->installed) {
		NSetTrapAddress((long) patch->addr, patch->number, patch->type);
		patch->installed = true;
	}
}

/*	�PatchRemove removes the patch. */
void PatchRemove(PatchType patch)
{
	if (patch->installed) {
		NSetTrapAddress((long) patch->trap, patch->number, patch->type);
		patch->installed = false;
	}
}

/*	�PatchRemoveAll removes all patches. */
void PatchRemoveAll(void)
{
	PatchType patch;
	
	for (patch = gPatches; patch; patch = patch->next)
		PatchRemove(patch);
}

/*	�PatchBegin creates and installs a patch, and returns a pointer to the
	patch structure, or NULL if the patch couldn't be created. */
PatchType PatchBegin(void *addr, short number)
{
	PatchType patch;
	
	patch = (PatchType) NewPtrClear(sizeof(PatchStructure));
	if (patch) {
		patch->number = number;
		patch->type = GetTrapType(patch->number);
		patch->addr = (void *) StripAddress(addr);
		patch->trap = (void *) NGetTrapAddress(patch->number, patch->type);
		gPatches = PatchInsert(gPatches, patch);
		PatchInstall(patch);
	}
	return(patch);
}

/*	�PatchEnd removes and disposes of the patch. */
void PatchEnd(PatchType patch)
{
	if (patch) {
		gPatches = PatchDelete(gPatches, patch);
		PatchRemove(patch);
		DisposePtr((Ptr) patch);
	}
}

/*	�PatchEndAll removes and disposes of all patches. */
void PatchEndAll(void)
{
	while (gPatches)
		PatchEnd(gPatches);
}
