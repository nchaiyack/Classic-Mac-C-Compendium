/**********************************************************************\

File:		sd init.c

Purpose:	This module handles initialization at INIT time.

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

#include "sd init.h"
#include "sd main.h"
#include "sd environment.h"
#include "notify.h"
#include "Shutdown.h"

Handle			initHandle;			/* handle to main init code */
THz				saveZone;
SleepQRec		*theSleepPtr;
	
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
	RememberA0();
	SetUpA4();
	PrepareEnvironment();
	if (!OkayEnvironmentQQ())
	{
		StartupError();
	}
	else
	{
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
			
			DoSetup();
			StartupGood();
		}
	}
	
	RestoreEnvironment();
	RestoreA4();
}

void DoSetup(void)
{
	theSleepPtr=(SleepQRec*)NewPtrSys(sizeof(SleepQRec));
	theSleepPtr->sleepQLink=0;
	theSleepPtr->sleepQType=slpQType;
	theSleepPtr->sleepQProc=(ProcPtr)sdMain;
	theSleepPtr->sleepQFlags=0;
	SleepQInstall(theSleepPtr);
	ShutDwnInstall((ProcPtr)&TheShutDownProc, sdOnDrivers+sdOnPowerOff);
}

void TheShutDownProc(void)
{
	// for some reason, the system will call our sleep proc with a code "sleepDemand"
	// on shutdown.  If we don't remove the sleep proc before this happens, this
	// call will crash the machine with an error 41 (can't find "Finder" on disk).
	// I offer no explanation, merely observation...
	
	SetUpA4();
	SleepQRemove(theSleepPtr);
	RestoreA4();
}
