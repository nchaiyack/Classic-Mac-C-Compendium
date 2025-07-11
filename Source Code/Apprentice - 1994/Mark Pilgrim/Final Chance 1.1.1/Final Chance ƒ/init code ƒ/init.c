/**********************************************************************\

File:		init.c

Purpose:	This module handles INIT initialization and installation of
			the dialog resources in memory (for use on shutdown).
			

Final Chance -=- a "do you really want to do this?" dialog on shutdown
Copyright �1993-4, Mark Pilgrim

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
#include "notify.h"
#include "ShutDown.h"

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
	
	RememberA0();	/* assume A0 has the address of the start of the code */
	SetUpA4();
	
	saveZone=GetZone();
	SetZone(SysZone);
	
	asm
	{
		movea.l			a4, a0
		RecoverHandle
		move.l			a0, initHandle
	}
	if (MemError())
	{
		StartupError();
	}
	else
	{
		HLock(initHandle);
		HNoPurge(initHandle);
		
		if (DoSetup())
			StartupGood();
		else
			StartupError();
	}
	
	SetZone(saveZone);
	
	RestoreA4();
}

Boolean DoSetup(void)
{
	unsigned long	temp;
	Handle			tempHandle;
	unsigned int	totalNumberOfQuotes;
	
	gChanceDlog=gChanceDitl=tempHandle=0L;
	
	gChanceDlog=(DialogTHndl)GetResource('DLOG', 128);
	if (gChanceDlog==0L)
		return FALSE;
	gChanceDitl=GetResource('DITL', 128);
	if (gChanceDitl==0L)
		return FALSE;
	
	tempHandle=GetResource('STR#', 128);
	if (tempHandle==0L)
		return FALSE;
	
	totalNumberOfQuotes=*((unsigned int*)(*tempHandle));
	ReleaseResource(tempHandle);
	if (totalNumberOfQuotes==0)
		return FALSE;
	
	GetDateTime(&temp);
	GetIndString(gTheQuote, 128, (temp%totalNumberOfQuotes)+1);
	
	DetachResource(gChanceDlog);
	DetachResource(gChanceDitl);
	
	ShutDwnInstall((ProcPtr)&FinalChance, sdOnDrivers+sdOnPowerOff);
	
	return TRUE;
}
