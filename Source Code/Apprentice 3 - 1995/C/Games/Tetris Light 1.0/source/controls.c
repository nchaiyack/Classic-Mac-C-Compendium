/**********************************************************************\

File:		controls.c

Purpose:	Module contains code relating to the controls for playing
			the game, and data relating to the user settings of the game.
			

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

#include "controls.h"
#include "dialutil.h"
#include "pstring.h"
#include "resources.h"
#include "windows.h"

/*--------------------------------------------------------------------*/

/* Control keys and playing preferences. */

struct Ctrls ctrls = {
	{ 38, 40, 37, 49 },
	{ 'J', 'K', 'L', ' ' },
	TRUE,	/* Show next piece */
	TRUE	/* Sound on */
};

/*--------------------------------------------------------------------*/

/* Static globals for interface for setting control keys */

static INTEGER key_target = 0;
static FontInfo key_font_info;
static Handle key_icon;
static Handle key_pressed_icon;

/*====================================================================*/

Boolean controls_init(void)
/* Initializes this module.  Loads up the icons for displaying the
   control key editing dialog.  Returns FALSE on success, TRUE if it
   failed. */
{
	/* Load icons */
	
	key_icon = GetIcon(KEY_ICON_ID);
	key_pressed_icon = GetIcon(KEY_PRESSED_ICON_ID);
	if (! key_icon || ! key_pressed_icon)
		return TRUE; /* Failed */
		
	return FALSE; /* Success */
}

/*--------------------------------------------------------------------*/

static pascal void key_item_proc(WindowPtr wind, INTEGER item)
/* Draw routine for the key userItem.  Draws the key using the 
   appropriately highlighted icon, and draws the character of it
   in the centre. */
{
	INTEGER dummy;
	Rect box;
	Handle h;
	Str255 str;
	
	switch (ctrls.sym[item - KEYS_ITEM_LEFT_KEY]) {
	case ' ':
		pstrcpy(str, "\pSP");
		break;
	default:
		str[0] = 1;
		str[1] = ctrls.sym[item - KEYS_ITEM_LEFT_KEY];
		break;
	}
	
	GetDItem(wind, item, &dummy, &h, &box);
	
	PlotIcon(&box, (key_target != item) ? key_icon : key_pressed_icon);
	MoveTo((box.left + box.right - StringWidth(str)) / 2, 
	       (box.top + box.bottom + key_font_info.ascent - key_font_info.descent) / 2);
	TextMode(patXor);
	DrawString(str);
}

/*--------------------------------------------------------------------*/

static pascal Boolean keys_filter_proc(DialogPtr dp, EventRecord *evt,
									   INTEGER *itemhit)
/* Filter proc for control keys dialog. */
{
	switch (evt->what) {
	case mouseDown:
		if (key_target != 0) {
			/* Unhighlight any currently highlighted key */
			
			register INTEGER old = key_target;
			
			key_target = 0;
			SetPort(dp);
			key_item_proc(dp, old);
			return FALSE;	/* [sic] yes, return FALSE */
		}
		break;
		
	case keyDown:
		if ((evt->message & keyCodeMask) >> 8 == ESC_KEY) {
			simulate_key_hit(dp, Cancel);
			*itemhit = Cancel;
			return TRUE;
		}
		
		switch (evt->message & charCodeMask) {
		case ENTER_CODE:
		case RETURN_CODE:
			simulate_key_hit(dp, OK);
			*itemhit = OK;
			return TRUE;

		case '\t':
			/* TAB key */
			
			if (key_target == KEYS_ITEM_DROP_KEY) {
				register INTEGER old = key_target;
				
				key_target = 0;
				SetPort(dp);
				key_item_proc(dp, old);
				return FALSE;	/* [sic] */
			}
			else {
				if (key_target == 0)
					*itemhit = KEYS_ITEM_LEFT_KEY;
				else
					*itemhit = key_target + 1;
				return TRUE;
			}
			break;
		case '.':
			if (evt->modifiers & cmdKey) {
				simulate_key_hit(dp, Cancel);
				*itemhit = Cancel;
				return TRUE;
			}
			/* continue and process as default case */
		default:
			if (key_target) {
				/* Set this control to this key */
				
				int index = key_target - KEYS_ITEM_LEFT_KEY;
				register unsigned char code = (evt->message & keyCodeMask) >> 8;
				register int x;
				
				/* Check for clashes */
				for (x = 0; x < KEY_NUMBER_OF; x++)
					if (x != index && ctrls.code[x] == code) {
						SysBeep(5);
						return FALSE;	/* sic */
					}
					
				ctrls.sym[index] = evt->message & charCodeMask;
				ctrls.code[index] = code;
				SetPort(dp);
				key_item_proc(dp, key_target);
			}
			return FALSE;	/* [sic] */
			break;
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

void controls_edit(void)
/* Brings up and processes the dialog that lets the user to edit the
   game control keys. */
{
	DialogPtr	dial;
	INTEGER		item;
	INTEGER		x;
	INTEGER		prev;
	struct Ctrls old_key;
	
	for (x = 0; x < KEY_NUMBER_OF; x++) {
		old_key.sym[x] = ctrls.sym[x];
		old_key.code[x] = ctrls.code[x];
	}
	key_target = 0;
	
	/* Load the dialog and set the user items */
	
	dial = GetNewDialog(KEY_DIAL_ID, NIL, (WindowPtr) -1);
	
	install_hilight_button(dial, OK, KEYS_ITEM_HIGHLIGHT);
	for (x = KEYS_ITEM_LEFT_KEY; x <= KEYS_ITEM_DROP_KEY; x++) {
		INTEGER dummy;
		Rect box;
		Handle h;
		
		GetDItem(dial, x, &dummy, &h, &box);
		SetDItem(dial, x, userItem, key_item_proc, &box);
	}
	
	/* Get the height of the default font */
	
	SetPort(dial);
	GetFontInfo(&key_font_info);

	do {
		ModalDialog(keys_filter_proc, &item);
	
		switch (item) {
		case OK:
			break;
		case Cancel:
			/* Restore old codes */
			for (x = 0; x < KEY_NUMBER_OF; x++) {
				ctrls.sym[x] = old_key.sym[x];
				ctrls.code[x] = old_key.code[x];
			}
			break;
		default:
			SetPort(dial);
			prev = key_target;
			
			key_target = item;
			if (prev)
				key_item_proc(dial, prev);
			key_item_proc(dial, key_target);
			break;
		}
	} while (item != OK && item != Cancel);
	
	DisposDialog(dial);
}

/*====================================================================*/

void controls_load(void)
/* Loads the user game controls from the given resource file.  If they
   are not found, nothing is done. */
{
	register OSErr erc;
	LONGINT size;
	struct Ctrls **handle;
	
	handle = (struct Ctrls **) GetResource(PREF_RSRC_TYPE, KEYS_PREF_ID);
	if (handle)
		ctrls = **handle;
}

/*--------------------------------------------------------------------*/

OSErr controls_save(INTEGER pref_file)
/* Saves the game controls and user settings to the given resource
   file. */
{
	OSErr erc;
	LONGINT size;
	struct Ctrls **handle;
	
	handle = (struct Ctrls **) GetResource(PREF_RSRC_TYPE, KEYS_PREF_ID);
	if (handle != NIL && HomeResFile(handle) == pref_file) {
		RmveResource(handle);
		erc = ResError();
		if (erc != noErr)
			return erc;
		
		DisposHandle(handle);
	}
	
	/* Create new resource */
	
	erc = PtrToHand(&ctrls, &handle, sizeof(ctrls));
	if (erc != noErr)
		return erc;
		
	AddResource(handle, PREF_RSRC_TYPE, KEYS_PREF_ID, "\p");
	erc = ResError();
	if (erc != noErr)
		return erc;
	
	return noErr;
}

/*--------------------------------------------------------------------*/
