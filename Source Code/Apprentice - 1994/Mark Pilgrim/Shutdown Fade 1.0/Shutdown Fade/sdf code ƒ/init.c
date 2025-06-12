/**********************************************************************\

File:		init.c

Purpose:	This module handles INIT initialization and installation.
			

Shutdown Fade -=- fade the screen to black on shutdown
Copyright (C) 1993 Mark Pilgrim

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
#include "fade.h"
#include "environment.h"
#include "notify.h"
#include "gamma.h"
#include "ShutDown.h"

Handle			initHandle;			/* handle to main init code */
THz				saveZone;

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
	OSErr			isHuman;
	int				resultCode;
	
	RememberA0();
	SetUpA4();
	
	PrepareEnvironment();
	if (IsGammaAvailable())
	{
		asm
		{
			movea.l			a4, a0
			RecoverHandle
			move.l			a0, initHandle
		}
		if(isHuman = MemError())
		{
			StartupError();
		}
		else
		{
			HLock(initHandle);
			HNoPurge(initHandle);
			
			DoSetup();
			StartupGood();
		}
	}
	else
	{
		StartupError();
	}
	
	RestoreEnvironment();
	
	RestoreA4();
}

void DoSetup(void)
{
	ShutDwnInstall((ProcPtr)&DoTheFadeThing, sdOnPowerOff);
}
