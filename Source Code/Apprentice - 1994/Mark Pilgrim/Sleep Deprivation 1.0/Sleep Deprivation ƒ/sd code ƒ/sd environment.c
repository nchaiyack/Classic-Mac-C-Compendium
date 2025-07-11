/**********************************************************************\

File:		sd environment.c

Purpose:	This module handles environment checking (this INIT will
			only work on Macs which can sleep).


Sleep Deprivation -- graphic effects on sleep
Copyright (C) 1993 Mark Pilgrim & Dave Blumenthal

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "Traps.h"
#include "GestaltEQU.h"
#include "sd environment.h"
#include "sd init.h"

/* Dave says: */
/* The following will stop the compiler from using Gestalt glue without */
/* killing all the other pre-system 7 glue for other routines. */
/* Mark says: wow, the following just impresses the s--- out of me */
#pragma parameter __D0 RealGestalt(__D0,__A1)
pascal OSErr RealGestalt(OSType selector,long *response) = {0xA1AD,0x2288};
#define Gestalt			RealGestalt
#define GESTALT_TRAP	_Gestalt

#define GetTrapType(T)	(((T&0x0800)>0) ? ToolTrap : OSTrap)

void PrepareEnvironment(void)
{
	saveZone = GetZone();
	SetZone(SysZone);
}

void RestoreEnvironment(void)
{
	SetZone(saveZone);
}

Boolean OkayEnvironmentQQ(void)
{
	OSErr			isHuman;
	long			gestaltReturn;
	
	if (TrapAvailable(GESTALT_TRAP))
	{
		isHuman=Gestalt(gestaltPowerMgrAttr, &gestaltReturn);
		return ((!isHuman) && (gestaltReturn&(1<<gestaltPMgrCPUIdle)));
	}
	else
		return TrapAvailable(_IdleUpdate);
}

Boolean TrapAvailable(int theTrap)
{
	/* figure out if a particular trap is available; this code is completely */
	/* opaque to me  -MP */
	
	TrapType		theTrapType;
	
	theTrapType=GetTrapType(theTrap);
	if (theTrapType==ToolTrap)
	{
		theTrap&=0x07FF;
		if(theTrap>=((NGetTrapAddress(_InitGraf, ToolTrap)==
					NGetTrapAddress(0xAA6E, ToolTrap)) ? 0x0200	: 0x0400))
			theTrap=_Unimplemented;
	}
	return (NGetTrapAddress(theTrap, theTrapType)!=
			NGetTrapAddress(_Unimplemented, ToolTrap));
}
