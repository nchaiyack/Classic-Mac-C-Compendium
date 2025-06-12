/**********************************************************************\

File:		env.c

Purpose:	This module provides routines to determine the facilities
			available on the running machine. These routines provide
			glue to work on all Macintoshes (including the original
			128K Mac). The implementation does not rely on any 
			compiler supplied glues.
			

``Tetris Light'' - a simple implementation of a Tetris game.
Copyright (C) 1993 Hoylen Sue

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the
Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

\**********************************************************************/

#include "local.h"

#include <GestaltEqu.h>
#include <Traps.h>

#include "env.h"

/*--------------------------------------------------------------------*/

/* Local globals - initialized to worst case assumption, but will be
   set up correctly to reflect the machine when `env_init' is called. */

static Boolean env_64k_rom = TRUE;
static Boolean env_gestalt_available = FALSE;

/*--------------------------------------------------------------------*/

static void env_init(void)
/* Set up local globals which indicate whether the machine has 64K ROMs
   and if gestalt is available.  The routines which need this information
   must call this routine before accessing that information.  Calculation
   of these variables is done only once, but this approach simplifies the
   interface to this module, because it requries no explicit
   initialization.  Failure to call this module may result in those
   variables containing default values, which will indicate a very
   limited machine. */
{
	static Boolean initialized = FALSE;
	INTEGER rom_version;
	INTEGER machine;
	
	if (initialized)
		return;
	initialized = TRUE;
	
	/* Check for 64K ROMS. Use Environs since it is available in ALL
	   Macintoshes, although it has been replaced by Gestalt. Note that
	   this code does not rely on any compiler glues. */
	
	Environs(&rom_version, &machine);
	if (rom_version >= 0x75)
		env_64k_rom = FALSE;
	
	/* Check for Gestalt facility. Gestalt is never available on
	   original 64K ROM machines! */
	
	if (env_64k_rom)
		env_gestalt_available = FALSE;
	else
		env_gestalt_available = env_trap_available(0xA1AD);
}

/*--------------------------------------------------------------------*/

static INTEGER num_toolbox_traps(void)
/* Returns the size of the toolbox trap dispatch table.  Code from
   Inside Macintosh VI p. 3-8.  InitGraf (0xA86E) is always implemented.
   If the trap dispatch table has more than 0x200 entries, then 0xAA6E
   always points to either unimplemented or something else, but never
   to InitGraf. The 64K roms do not have NGetTrapAddress, but always
   have small dispatch tables.
   Internal routines which call this one must have ensured that the
   `env_64k_rom' variable has been set up correctly. */
{
	if (env_64k_rom ||
	    (NGetTrapAddress(_InitGraf, ToolTrap) ==
	     NGetTrapAddress(0xAA6E, ToolTrap)))
		return 0x0200;
	else
		return 0x0400;
}

Boolean env_trap_available(INTEGER the_trap)
/* Returns TRUE if the given trap is available. Based on code from
   Inside Macintosh VI.  This routine does not rely on any glue, and
   will work on all Macintoshes from the original 128K mac upwards.
   Short circuit calls to this routine by checking for 64K roms first. */
{
	register TrapType tType;

	env_init();
	
	if (the_trap & 0x0800) {
		tType = ToolTrap;
		the_trap &= 0x07FF;
		if (the_trap >= num_toolbox_traps())
			the_trap = _Unimplemented;
	}
	else
		tType = OSTrap;
	
	if (env_64k_rom)
		return (GetTrapAddress(the_trap) !=
		        GetTrapAddress(_Unimplemented));
	else
		return (NGetTrapAddress(the_trap, tType) != 
				NGetTrapAddress(_Unimplemented, ToolTrap));
}

/*--------------------------------------------------------------------*/

Boolean env_WaitNextEvent_available(void)
/* Determines if `WaitNextEvent' trap is available.  Returns TRUE if it
   is, FALSE if it is not available. */
{
	return env_trap_available(_WaitNextEvent);
}

/*--------------------------------------------------------------------*/

Boolean env_FindFolder_available(void)
/* Determines if the `Gestalt' trap is available.  Returns TRUE if it
   is, FALSE if it is not available. */
{	
	env_init();

	if (env_gestalt_available) {
		LONGINT result;
		
		Gestalt(gestaltFindFolderAttr, &result);
		if (result & (0x0001 << gestaltFindFolderPresent))
			return TRUE;
	}

	return FALSE;
}

/*--------------------------------------------------------------------*/

Boolean env_SndPlay_available(void)
/* Determines if 'SndPlay' is available.  Returns TRUE if it is, FALSE
   if it is not. */
{
	env_init();
	
	return env_trap_available(_SndPlay);
}

/*--------------------------------------------------------------------*/
