/**********************************************************************\

File:		ghost error.c

Purpose:	This module handles fatal and non-fatal Ghost-specific
			errors.


Ghost -=- a classic word-building challenge
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

#include "ghost globals.h"
#include "ghost error.h"
#include "msg dialogs.h"
#include "msg main.h"

void HandleError(int resultCode)
{
	Str255			tempStr;
	
	if (resultCode!=allsWell)
	{
		GetIndString(tempStr, 129, resultCode);
		ParamText(tempStr, "\p", "\p", "\p");
		PositionDialog('ALRT', prefsErrorAlert);
		StopAlert(prefsErrorAlert, 0L);
		switch (resultCode)
		{
			case kNoIndex:
			case kNoDictionaries:
			case kCantFindSmallDict:
			case kCantReadSmallDict:
			case kSmallDictDamaged:
			case kCantFindLargeDict:
			case kCantReadLargeDict:
			case kLargeDictDamaged:
			case kNoMemory:
				ShutDownEnvironment();
				ExitToShell();
				break;
		}
	}
}
