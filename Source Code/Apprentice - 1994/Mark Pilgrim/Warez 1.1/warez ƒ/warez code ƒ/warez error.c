/**********************************************************************\

File:		warez error.c

Purpose:	This module handles fatal and non-fatal WAREZ-specific
			errors.


WAREZ -=- nostalgia isn't what it used to be
Copyright �1994, Mark Pilgrim

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

#include "program globals.h"
#include "warez error.h"
#include "warez progress.h"
#include "msg dialogs.h"
#include "msg main.h"
#include "msg environment.h"

void HandleError(int resultCode)
{
	Str255			tempStr;
	
	if (resultCode!=allsWell)
	{
		if (showThatTharProgress)
		{
			DismissProgressDialog();
			gInProgress=FALSE;
		}
		
		GetIndString(tempStr, 129, resultCode);
		ParamText(tempStr, "\p", "\p", "\p");
		PositionDialog('ALRT', prefsErrorAlert);
		StopAlert(prefsErrorAlert, 0L);
	}
}
