/**********************************************************************\

File:		alert.c

Purpose:	This module provides routines to display various messages
			to the user in alerts.
			

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

#include "alert.h"
#include "dialutil.h"
#include "resources.h"
#include "windows.h"

/*--------------------------------------------------------------------*/

static pascal Boolean alert_filter_proc(DialogPtr dp, EventRecord *evt,
										INTEGER *itemhit)
/* Event filter procedure for the alert dialogs.  Recognizes the return
   and enter key equivalents for the default button (all these dialogs
   only have one active button.) */
{
	switch (evt->what) {
	case keyDown:
		if ((evt->message & charCodeMask) == RETURN_CODE ||
		    (evt->message & charCodeMask) == ENTER_CODE) {			
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

void alert_caution(INTEGER msg_id)
/* Displays an alert message to the user. Note that a dialog is used
   rather than a real alert, because the alert dialog boxes supplied
   by the ToolBox do not handle update events properly (i.e. their
   icon and default button highlighting does not get redrawn. The
   static text is set to the alert 'STR#' string with `msg_id' index If
   `msg_id' is zero, the default message is used.  Note that these 
   message strings may contain ParamText substitutes, which may be
   filled in by calling ParamText before this. */
{
	DialogPtr	dial;
	INTEGER		item;
	
	dial = GetNewDialog(CAUTION_DIAL_ID, NIL, (WindowPtr) -1);
	install_hilight_button(dial, OK, CAUTION_DIAL_HIGHLIGHT_ITEM);

	/* Set up the message with parameters */
	
	if (msg_id != 0) {
		INTEGER type;
		Handle handle;
		Rect box;
		Str255 str;
	
		GetDItem(dial, CAUTION_DIAL_TEXT_ITEM, &type, &handle, &box);
		GetIndString(str, CAUTION_ALERT_STRs_ID, msg_id);
		if (*str != 0)
			SetIText(handle, str);		
	}

	/* Display the alert */
	
	SysBeep(5);
	ModalDialog(alert_filter_proc, &item);
	DisposDialog(dial);
}

/*--------------------------------------------------------------------*/

void alert_erc(INTEGER msg_id, OSErr erc)
/* Similar to `alert_caution' above, except an error code `erc' is
   also reported. */
{
	DialogPtr	dial;
	INTEGER		item;

	dial = GetNewDialog(CAUTION_ERC_DIAL_ID, NIL, (WindowPtr) -1);
	install_hilight_button(dial, OK, CAUTION_ERC_DIAL_HIGHLIGHT_ITEM);

	/* Set up the message string and parameter text */
	
	if (msg_id != 0) {
		INTEGER type;
		Handle handle;
		Rect box;
		Str255 str;
	
		GetDItem(dial, CAUTION_ERC_DIAL_TEXT_ITEM, &type, &handle, &box);
		GetIndString(str, CAUTION_ERC_ALERT_STRs_ID, msg_id);
		if (*str != 0)
			SetIText(handle, str);		
	}

	/* Set up the error code text */
	
	if (erc != noErr) {
		INTEGER type;
		Handle handle;
		Rect box;
		Str255 code_string;
	
		GetDItem(dial, CAUTION_ERC_ERC_ITEM_NO, &type, &handle, &box);
		NumToString(erc, code_string);
		SetIText(handle, code_string);
	}
	
	/* Display the dialog */
	
	SysBeep(5);
	ModalDialog(alert_filter_proc, &item);
	DisposDialog(dial);
}

/*--------------------------------------------------------------------*/

static pascal Boolean fatal_filter_proc(DialogPtr dp, EventRecord *evt,
										INTEGER *itemhit)
/* Event filter procedure for the fatal alert dialogs.  Recognizes the
   return and enter key equivalents for the default button, but for
   other events do not process them normally, because the application
   might be in such a bad state that they could crash. */
{
	switch (evt->what) {
	case keyDown:
		if ((evt->message & charCodeMask) == RETURN_CODE ||
		    (evt->message & charCodeMask) == ENTER_CODE) {			
			simulate_key_hit(dp, OK);
			*itemhit = OK;
			return TRUE;
		}
		break;
	}

	return FALSE;
}

/*--------------------------------------------------------------------*/

void alert_fatal(INTEGER msg_id)
/* Emergency abort routine. Used when the program cannot continue 
   any further (e.g. missing resources). The multiple beeps indicate
   that it was a `controlled' abort, rather than an unexpected one. */
{
	DialogPtr	dial;
	INTEGER		item;
	
	dial = GetNewDialog(FATAL_DIAL_ID, NIL, (WindowPtr) -1);
	install_hilight_button(dial, OK, FATAL_DIAL_HIGHLIGHT_ITEM);

	if (msg_id != 0) {
		INTEGER type;
		Handle handle;
		Rect box;
		Str255 str;
	
		GetDItem(dial, FATAL_DIAL_TEXT_ITEM, &type, &handle, &box);
		GetIndString(str, FATAL_ALERT_STRs_ID, msg_id);
		if (*str != 0)
			SetIText(handle, str);		
	}

	SysBeep(5);
	ModalDialog(fatal_filter_proc, &item);
	DisposDialog(dial);
	
	ExitToShell();
}

/*--------------------------------------------------------------------*/
