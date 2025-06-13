/**********************************************************************\

File:		sd environment.c

Purpose:	This module handles environment checking (this INIT will
			only work on Macs which can sleep).

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
	
	isHuman=Gestalt(gestaltPowerMgrAttr, &gestaltReturn);
	return ((!isHuman) && (gestaltReturn&(1<<gestaltPMgrCPUIdle)));
}
