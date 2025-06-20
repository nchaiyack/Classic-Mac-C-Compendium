/**********************************************************************\

File:		init.c

Purpose:	This module handles INIT initialization and installation.
			

Shutdown FX -=- graphic effects on shutdown
Copyright (C) 1993-4, Mark Pilgrim & Dave Blumenthal

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

#include "init.h"
#include "main.h"
#include "prefs.h"
#include "show init.h"
#include "ShutDown.h"
#include "GestaltEQU.h"
#define		GOOD_ICON				130		/* icons displayed at startup */
#define		BAD_ICON				129

Handle			initHandle;			/* handle to main init code */

void __GetA4(void)
{
	asm {
		bsr.s	@1
		dc.l	0			;  store A4 here
@1		move.l	(sp)+,a1
	}
}

/* this be the init startup install routine thing */
void main(void)
{
	THz				saveZone;
	long			gestalt_temp;
	OSErr			isHuman;
	
	RememberA0();
	SetUpA4();
	
	isHuman=Gestalt(gestaltFSAttr, &gestalt_temp);
	gHasFSSpecs=((isHuman==noErr) && (gestalt_temp&(1<<gestaltHasFSSpecCalls)));
	
	saveZone = GetZone();
	SetZone(SysZone);
	
	asm
	{
		movea.l			a4, a0
		RecoverHandle
		move.l			a0, initHandle
	}
	if (MemError())
	{
		SetZone(saveZone);
		ShowIconFamily(BAD_ICON);
	}
	else
	{
		HLock(initHandle);
		HNoPurge(initHandle);
		if (DoSetup())	/* everything ok */
		{
			DetachResource(initHandle);
			SetZone(saveZone);
			ShowIconFamily(GOOD_ICON);
		}
		else
		{
			SetZone(saveZone);
			ShowIconFamily(BAD_ICON);
		}
	}
	
	RestoreA4();
}

Boolean DoSetup(void)
{
	int				resultCode;
	
	resultCode=PreferencesInit();
	if ((resultCode!=prefs_allsWell) && (resultCode!=prefs_virginErr))
		return FALSE;
	
	gTheDlog=gTheDitl=0L;
	gTheDlog=(DialogTHndl)GetResource('DLOG', 128);
	if (gTheDlog==0L)
		return FALSE;
	gTheDitl=GetResource('DITL', 128);
	if (gTheDitl==0L)
		return FALSE;
	DetachResource(gTheDlog);
	DetachResource(gTheDitl);
	
	ShutDwnInstall((ProcPtr)&sfxShutdownMain, sdOnDrivers+sdOnPowerOff);
	ShutDwnInstall((ProcPtr)&sfxRestartMain, sdOnDrivers+sdOnRestart);
	
	return TRUE;
}
