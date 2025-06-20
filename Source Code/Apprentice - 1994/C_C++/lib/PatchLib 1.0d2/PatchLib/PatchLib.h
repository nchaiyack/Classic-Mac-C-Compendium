/* See the file Distribution for distribution terms.
	(c) Copyright 1994 Ari Halberstadt */

#pragma once

#include <OSUtils.h>

typedef struct PatchStructure PatchStructure, *PatchType;
typedef const PatchStructure *PatchCType;

/* data needed by a patch */
struct PatchStructure {
	PatchType	next;					/* next patch */
	TrapType		type;					/* type of trap */
	short			number;				/* trap number */
	Boolean		installed;			/* true if patch is installed */
	pascal void (*addr)(...);		/* address of patch routine */
	pascal void	(*trap)(...);		/* saved trap address */
};

/* The PATCH_ENTER macro should be the first executable statement in
	your patch routine. PATCH_ENTER saves all of the registers and sets
	up register a5. PATCH_ENTER also ensures that the routine has a stack
	frame so that the PATCH_RETURN macro can be used. */
#define PATCH_ENTER() \
	{	long _patch_force_stack_frame; {		/* force stack frame of at least 4 bytes */ \
		asm { movem.l a0-a5/d0-d7, -(sp) }	/* save registers */ \
		asm { move.l #0x0904, a5 }				/* setup register a5 */ \
		asm { move.l (a5), a5 }

/* The PATCH_RETURN macro jumps to the address of the originally patched
	routine. You should call PATCH_RETURN at the end of your patch. The
	'patch' parameter should be a pointer returned from PatchBegin. The
	compiler must generate a stack frame (using register a6) for the patch
	routine. This should be ensured by the PATCH_ENTER macro. */
#define PATCH_RETURN(patch) \
   } } \
   { \
		asm { move.l a6, a1 }					/* get current value of a6 */ \
		asm { move.l (a6), -(a6) }				/* shift location of saved a6 */ \
		asm { move.l patch, a0 }				/* get pointer to patch record */ \
		asm { move.l PatchStructure.trap(a0), (a1) } /* put patched routine's address on stack */ \
		asm { movem.l (sp)+, a0-a5/d0-d7 }	/* restore registers */ \
		asm { unlk a6 }							/* pop stack frame */ \
		asm { rts }									/* return to patched routine */ \
	}

void PatchInstall(PatchType patch);
void PatchRemove(PatchType patch);
void PatchRemoveAll(void);
PatchType PatchBegin(void *addr, short number);
void PatchEnd(PatchType patch);
void PatchEndAll(void);
