/*
	PreDragon.c
	
	Edit, precompile, save as PreDragon, and #include "PreDragon" in the project Prefix box
*/

#include	<MacHeaders>

#ifdef	SystemSevenOrLater
#undef	SystemSevenOrLater
#endif

#define	SystemSevenOrLater	1		// This is to avoid linking in System–6 compatible glue for Gestalt and
									//	FindFolder — dragons need System 7 to run anyhow…
									
	// NOTE:	If you've changed your copy of the file "Mac #includes.c" and re–precompiled MacHeaders, then some
	//		glue code may remain

#include	<AppleEvents.h>
#include	<GestaltEqu.h>	/* Bummer … */
#include	<Folders.h>
#include	<Script.h>
