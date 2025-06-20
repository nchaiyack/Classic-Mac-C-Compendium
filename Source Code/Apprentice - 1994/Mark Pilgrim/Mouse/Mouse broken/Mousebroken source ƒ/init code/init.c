/**********************************************************************\

File:		init.c

Purpose:	This module handles INIT initialization & installation of
			the correct VBL (if any).


Mousebroken -=- your computer isn't truly broken until it's mousebroken
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

#include "structs.h"
#include "init.h"
#include "prefs.h"
#include "really notify.h"
#include "meat.h"
#include "SetUpA4.h"

void main(void)
{
	int				resultCode;
	PrefHandle		cdevStorage;
	Boolean			goon;
	
	RememberA0();
	SetUpA4();
	
	goon=TRUE;
	
	cdevStorage=NewHandle(sizeof(PrefStruct));
	if (cdevStorage==0L)
	{
		StartupError(kNoMemory);
		RestoreA4();
		return;
	}
	
	if (goon)
	{
		resultCode=PreferencesInit(cdevStorage);
		if ((resultCode!=prefs_allsWell) && (resultCode!=prefs_virginErr))
		{
			StartupError(resultCode);
			goon=FALSE;
		}
	}
	
	if (goon)
	{
		resultCode=GetModuleInfo(cdevStorage);
		if (resultCode!=allsWell)
		{
			StartupError(resultCode);
			goon=FALSE;
		}
	}
	
	if (goon)
	{
		if (((**cdevStorage).isOn) && ((**cdevStorage).moduleIndex!=0))
		{
			resultCode=OpenTheModule(cdevStorage);
			if (resultCode!=allsWell)
			{
				StartupError(resultCode);
				goon=FALSE;
			}
			
			if (goon)
			{
				resultCode=InstallTheMouseVBL(cdevStorage);
				CloseTheModule(cdevStorage);
				if (resultCode!=allsWell)
				{
					StartupError(resultCode);
					goon=FALSE;
				}
			}
		}
	}
	
	SaveThePrefs(cdevStorage);
	
	if (goon)
		StartupGood(cdevStorage);
	
	DisposeHandle(cdevStorage);
	RestoreA4();
}
