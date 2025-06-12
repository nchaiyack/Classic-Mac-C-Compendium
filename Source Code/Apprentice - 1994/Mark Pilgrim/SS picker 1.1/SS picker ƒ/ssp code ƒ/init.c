/**********************************************************************\

File:		init.c

Purpose:	This module handles INIT initialization and installation.
			

Startup Screen Picker -=- pick a random startup screen
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
#include "main.h"
#include "environment.h"
#include "really notify.h"
#include "prefs.h"

void __GetA4(void)
{
	asm {
		bsr.s	@1
		dc.l	0			;  store A4 here
@1		move.l	(sp)+,a1
	}
}
void main(void)
{
	int			resultCode;
	
	RememberA0();
	SetUpA4();
	
	CheckEnvironment();
	
	resultCode=PreferencesInit();
	if ((resultCode!=prefs_allsWell) && (resultCode!=prefs_virginErr))
		StartupError(resultCode);
	else
	{
		resultCode=DoSSThing();
		if (resultCode==allsWell)
			StartupGood();
		else
			StartupError(resultCode);
	}
	
	RestoreA4();
}
