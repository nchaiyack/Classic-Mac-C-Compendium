/**********************************************************************\

File:		v error.c

Purpose:	This module handles displaying error messages specific to
			Voyeur.


Voyeur -- a no-frills file viewer
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

#include "v error.h"
#include "v structs.h"
#include "msg dialogs.h"

void HandleError(int resultCode)
{
	switch (resultCode)
	{
		case diskReadErr:
			ParamText("\pAn error occurred trying to read from disk.","\p","\p","\p");
			break;
		case diskWriteErr:
			ParamText("\pAn error occurred trying to write to the disk.  The disk may be full or locked.","\p","\p","\p");
			break;
		case fileBusyErr:
			if (forknum==0)
				ParamText("\pThe data fork of the file is already open by another application.","\p","\p","\p");
			else
				ParamText("\pThe resource fork of the file is already open by another application.","\p","\p","\p");
			break;
		case patternNotFoundErr:
			ParamText("\p“",findString,"\p” not found (end of file reached).","\p");
			break;\
		case offsetTooLargeErr:
			ParamText("\pThe offset you entered  is larger than the file’s length.","\p","\p","\p");
			break;
		case invalidHexErr:
			ParamText("\pThe offset you entered is not a valid hexadecimal number.","\p","\p","\p");
			break;
		case diskFullErr:
			ParamText("\pThe EOF was not changed, because an error occurred.  The disk may be full or locked.","\p","\p","\p");
			break;
		case badTypeCreatorErr:
			ParamText("\pThe type and creator were not changed, because they must both be exactly four characters long.","\p","\p","\p");
			break;
		default:
			ParamText("\pAn unknown error occurred.","\p","\p","\p");
			break;
	}
	if ((resultCode!=userCancelErr) && (resultCode!=allsWell))
	{
		PositionDialog('ALRT', generalAlert);
		StopAlert(generalAlert, 0L);
	}
}
