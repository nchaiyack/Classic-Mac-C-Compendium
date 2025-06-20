/**********************************************************************\

File:		menu_da.c

Purpose:	This module handles the menus and desk accessories for the
			``Tetris Light'' program.
			

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
#include "controls.h"
#include "game.h"
#include "highscore.h"
#include "menu_da.h"
#include "resources.h"

/*--------------------------------------------------------------------*/

/* Local globals */

static MenuHandle	apple_menu;
static MenuHandle	file_menu;
static MenuHandle	edit_menu;
static MenuHandle	options_menu;

/*--------------------------------------------------------------------*/

Boolean menu_init(void)
/* Sets up the menus for the application.  This routine is called at
   the very beginning of the program.  It loads up the menus, and sets
   them up for use.  If it cannot load the menu bar or menus, it will
   return TRUE, FALSE is returned on success. */
{
	Handle	menu_bar;

	/* Load in the menus from the resources */
		
	menu_bar = GetNewMBar(MBAR_ID);
	if (menu_bar == 0)
		return TRUE; /* Failed */
		
	SetMenuBar(menu_bar);
	DrawMenuBar ();

	apple_menu = GetMHandle(APPLE_MENU_ID);
	file_menu = GetMHandle(FILE_MENU_ID);		
	edit_menu = GetMHandle(EDIT_MENU_ID);
	options_menu = GetMHandle(OPTIONS_MENU_ID);

	if (!apple_menu || !file_menu || !edit_menu || !options_menu)
		return TRUE; /* Failed */

	/* Add entries for desk accessories */
	
	AddResMenu(apple_menu, 'DRVR');
	
	return FALSE; /* Success */
}

/*--------------------------------------------------------------------*/

static void handle_menu_apple(INTEGER item)
/* Processes selection of an item from the Apple menu. */
{
	Str255	da_name;
	
	switch (item) {
	case APPLE_MITEM_ABOUT:
		about_box();
		break;
	case 2:
		/* Separator line, should not be selected */
		SysBeep(5);
		break;
	default:
		GetItem(apple_menu, item, da_name);
		OpenDeskAcc(da_name);
		break;
	}
}

/*--------------------------------------------------------------------*/

static Boolean handle_menu_file(INTEGER item)
/* Processes selection of an item from the File menu.  Returns TRUE if
   the ``Quit'' item was chosen, FALSE otherwise. */
{	
	switch ( item ) {
	case FILE_MITEM_NEW:
		game_new();
		break;
	case FILE_MITEM_OPEN:
		game_open();
		break;
	case FILE_MITEM_SAVE:
		game_save();
		break;
	case FILE_MITEM_SAVE_AS:
		game_save_as();
		break;
	case FILE_MITEM_QUIT:
		return TRUE;
		break;
	default:
		/* Unknown item */
		SysBeep(5);
		break;
	}
	
	return FALSE;
}

/*--------------------------------------------------------------------*/

static void handle_menu_options(INTEGER item)
/* Processes selection of an item from the Options menu. */
{	
	switch (item) {
	case OPTIONS_MITEM_SHOWHIGH:
		highscore_start();
		break;
	case OPTIONS_MITEM_SOUND:
		ctrls.sound_on = !ctrls.sound_on;
		break;
	case OPTIONS_MITEM_NEXTPIECE:
		ctrls.show_next_piece = !ctrls.show_next_piece;
		break;
	default:
		/* Unknown item */
		SysBeep(5);
		break;
	}
}

/*--------------------------------------------------------------------*/

Boolean menu_choice(LONGINT choice)
/* This routine is used to process a selection of a menu item. Returns
   TRUE if the Quit option was chosen. */
{
	register Boolean quit_chosen = FALSE;
	
	if (choice) {
		register INTEGER item = LoWord(choice);
		
		switch (HiWord(choice)) {
		case APPLE_MENU_ID:
			handle_menu_apple(item);
			break;
		case FILE_MENU_ID:
			quit_chosen = handle_menu_file(item);
			break;
		case EDIT_MENU_ID:
			if (item == EDIT_MITEM_KEYS)
				controls_edit();
			else
				SystemEdit(item - 1);
			break;
		case OPTIONS_MENU_ID:
			handle_menu_options(item);
			break;
		default:
			/* Unknown menu */
			SysBeep(5);
			break;
		}
		HiliteMenu(0);
	}
	
	return quit_chosen;
}

/*--------------------------------------------------------------------*/

void adjust_menus()
/* This routine activates or deactivates the "Edit" menu according to
   whether the front window belongs to a DA or not. */
{
	if (is_DA_window(FrontWindow())) {
		EnableItem(edit_menu, EDIT_MITEM_UNDO);
		EnableItem(edit_menu, EDIT_MITEM_CUT);
		EnableItem(edit_menu, EDIT_MITEM_COPY);
		EnableItem(edit_menu, EDIT_MITEM_PASTE);
		EnableItem(edit_menu, EDIT_MITEM_CLEAR);
	}
	else {
		DisableItem(edit_menu, EDIT_MITEM_UNDO);
		DisableItem(edit_menu, EDIT_MITEM_CUT);
		DisableItem(edit_menu, EDIT_MITEM_COPY);
		DisableItem(edit_menu, EDIT_MITEM_PASTE);
		DisableItem(edit_menu, EDIT_MITEM_CLEAR);
	}

	/* Set up menu to reflect internal settings */

	SetItemMark(options_menu, OPTIONS_MITEM_SOUND,
				ctrls.sound_on ? checkMark : noMark);
	SetItemMark(options_menu, OPTIONS_MITEM_NEXTPIECE,
				ctrls.show_next_piece ? checkMark : noMark);
				
	/* Don't allow user to save finished games */
	
	if (game_is_over()) {
		DisableItem(file_menu, FILE_MITEM_SAVE);
		DisableItem(file_menu, FILE_MITEM_SAVE_AS);
	} else {
		EnableItem(file_menu, FILE_MITEM_SAVE);
		EnableItem(file_menu, FILE_MITEM_SAVE_AS);
	}
}

/*--------------------------------------------------------------------*/

Boolean is_DA_window(WindowPtr w_ptr)
/* Determines whether the given 'w_ptr' is a desk accessory window. */
{	
	if (w_ptr == 0)
		return FALSE;
	else	/* DA windows have negative windowKinds */
		return (((WindowPeek) w_ptr) -> windowKind < 0);
}

/*--------------------------------------------------------------------*/
