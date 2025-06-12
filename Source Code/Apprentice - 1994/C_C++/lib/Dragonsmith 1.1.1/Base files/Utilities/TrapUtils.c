/*
	TrapUtils.c
	
	OS and Toolbox trap-related utility functions.  Adapted from code in Inside Macintosh Volume VI, pages 3Ð8 to 3Ð9
	
	Created	17 Sep 1992	TrapAvailable and its static "herlper" functions
	Modified	

	Copyright © 1992 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (except as expressly permitted by the author)
		2.	This statement and the above copyright notice are left intact.
	
*/

#include	"TrapUtils.h"
#include	<Traps.h>

static short NumToolboxTraps (void);
static long AA6EAddress (void);
static long UnimplementedTrapAddress (void);
static TrapType GetTrapType (short trapWord);

Boolean TrapAvailable (short theTrap)
{
	TrapType		type;
	
	type = GetTrapType (theTrap);
	if (type == ToolTrap) {
		theTrap &= 0x07FF;
		if (theTrap >= NumToolboxTraps ())
			return FALSE;
	}
	return (NGetTrapAddress (theTrap, type) != UnimplementedTrapAddress ());
}

static short NumToolboxTraps (void)
{	
	static short	NUM_TBX_TRAPS = -1;		// Initialize NUM_TBX_TRAPS to a nonsensical value

	if (NUM_TBX_TRAPS == -1)
		NUM_TBX_TRAPS = ((NGetTrapAddress (_InitGraf, ToolTrap) == AA6EAddress ()) ? 0x0200 : 0x0400);
		
	return NUM_TBX_TRAPS;
}

static long AA6EAddress (void)
{
	static long	AA6E_ADDR = -1;			// Initialize _AA6E_ADDR to a nonsensical value
	
	if (AA6E_ADDR == -1)
		AA6E_ADDR = NGetTrapAddress (0xAA6E, ToolTrap);
		
	return AA6E_ADDR;
}

static long UnimplementedTrapAddress (void)
{
	static long	UNIMP_TRAP_ADDR = -1;	// // Initialize UNIMP_TRAP_ADDR to a nonsensical value
	
	if (UNIMP_TRAP_ADDR == -1)
		UNIMP_TRAP_ADDR = NGetTrapAddress (_Unimplemented, ToolTrap);
		
	return UNIMP_TRAP_ADDR;
}

static TrapType GetTrapType (short trapWord)
{
	return (trapWord & 0x0800) ? ToolTrap : OSTrap;
}

