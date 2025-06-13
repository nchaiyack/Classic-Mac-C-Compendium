/**********************************************************************\

File:		about.c

Purpose:	This module handles the displaying of the about box.


``Tetris Light'' - a simple implementation of a Tetris game.
Copyright (C) 1993 Hoylen Sue

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the
Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

\**********************************************************************/

#include "local.h"
#include "about.h"
#include "dialutil.h"
#include "resources.h"
#include "windows.h"

/*--------------------------------------------------------------------*/

static pascal Boolean about_filter_proc(DialogPtr dp, EventRecord *evt,
										INTEGER *itemhit)
/* Filter proc for the dialog.  Traps mouseDown events anywhere to
   remove the dialog.  Pressing the return or enter key also removes
   the dialog.  Processes activateEvt and updateEvt normally, so that
   windows behind it get updated correctly. */
{
	INTEGER type;
	Handle handle;
	Rect box;
	Point local;
	
	switch (evt->what) {
	case mouseDown:
		local = evt->where;
		SetPort(dp);
		GlobalToLocal(&local);
		GetDItem(dp, OK, &type, &handle, &box);
		if (!PtInRect(local, &box)) {
			/* Was not pressed inside (near enough) the button */
			*itemhit = OK;
			return TRUE;
		}
		break;
	case keyDown:
		if ((evt->message & charCodeMask) == ENTER_CODE ||
		    (evt->message & charCodeMask) == RETURN_CODE) {			
			simulate_key_hit(dp, OK);
			*itemhit = OK;
			return TRUE;
		}
		break;
	case activateEvt:
		if (dp != (WindowPtr) evt->message) {
			if (evt->modifiers & activeFlag)
				window_activate((WindowPtr) evt->message);
			else
				window_deactivate((WindowPtr) evt->message);
		}
		break;
	case updateEvt:
		if (dp != (WindowPtr) evt->message)
			window_update((WindowPtr) evt->message);
		break;
	}

	return FALSE;
}

/*--------------------------------------------------------------------*/

void about_box(void)
/* Brings up the about box, and processes it until the user makes it
   go away.  The dialog box is removed if the mouse is pressed or the
   return or enter key is pressed. */
{
	DialogPtr	dial;
	INTEGER		item;
	
	dial = GetNewDialog(ABOUT_DIAL_ID, NIL, (WindowPtr) -1);
	install_hilight_button(dial, OK, ABOUT_HIGHLIGHT_ITEM);
	ModalDialog(about_filter_proc, &item);
	DisposDialog(dial);
}

/*--------------------------------------------------------------------*/

