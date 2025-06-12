/**********************************************************************\

File:		resources.c

Purpose:	This header file contains definition of resource ID numbers.
			

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

#ifndef resources_H
#define resources_H

/*--------------------------------------------------------------------*/

/* File types and application signatures.  These have been registered
   with Apple for use with this application. */

#define CREATOR_SIGNATURE	'TetL'
#define SAVE_FILE_SIGNATURE	'TLsv'
#define PREF_FILE_SIGNATURE 'pref'

/*--------------------------------------------------------------------*/

/* Resources for saving preferences and high scores */

#define PREF_RSRC_TYPE			'PREF'

#define KEYS_PREF_ID			7000
#define GAME_WPOS_PREF_ID		7001
#define HI_WPOS_PREF_ID			7002

#define HIGH_SCORE_BASE_PREF_ID	8000

/*--------------------------------------------------------------------*/

#define PATTERN_ID			0

#define KEY_ICON_ID			7000
#define KEY_PRESSED_ICON_ID	7001

/*--------------------------------------------------------------------*/

/* SND resources */

#define HIT_SND_ID			7000
#define END_SND_ID			7001

/*--------------------------------------------------------------------*/

/* MENU resource IDs */

#define MBAR_ID			7000

#define APPLE_MENU_ID	7000
#define FILE_MENU_ID	7001
#define EDIT_MENU_ID	7002
#define OPTIONS_MENU_ID	7003

/*--------------------------------------------------------------------*/

/* Apple Menu Items */

#define APPLE_MITEM_ABOUT	1


/* File Menu Items */

#define FILE_MITEM_NEW		1
#define FILE_MITEM_OPEN		2

#define FILE_MITEM_SAVE		4
#define FILE_MITEM_SAVE_AS	5

#define FILE_MITEM_QUIT		7


/* Edit Menu Items */

#define EDIT_MITEM_UNDO		1

#define EDIT_MITEM_CUT		3
#define EDIT_MITEM_COPY		4
#define EDIT_MITEM_PASTE	5
#define EDIT_MITEM_CLEAR	6

#define EDIT_MITEM_KEYS		8


/* Options Menu Items */

#define OPTIONS_MITEM_SHOWHIGH	1
	
#define OPTIONS_MITEM_SOUND		3
#define OPTIONS_MITEM_NEXTPIECE	4

/*--------------------------------------------------------------------*/

/* CNTL resource templates */

#define GAME_PAUSE_CNTL_ID	7000
#define HIGH_OK_CNTL_ID		7001

/*--------------------------------------------------------------------*/

/* STR resources */

#define PREF_FILE_STR_ID	7000
#define SCORE_STR_ID		7001
#define GAME_OVER_STR_ID	7002
#define PAUSE_STR_ID		7003
#define CONTINUE_STR_ID		7004

/*--------------------------------------------------------------------*/

/* STR# resources */

#define FATAL_ALERT_STRs_ID			7000
#define CAUTION_ALERT_STRs_ID		7001
#define CAUTION_ERC_ALERT_STRs_ID	7002

/*--------------------------------------------------------------------*/

/* WIND resources */

#define GAME_WINDOW_ID		7000
#define HIGHSCORE_WINDOW_ID	7001

/*--------------------------------------------------------------------*/

/* Dialog resources */

#define FATAL_DIAL_ID		6000
#define CAUTION_DIAL_ID		6001
#define CAUTION_ERC_DIAL_ID	6002

#define GET_NAME_DIAL_ID	7000
#define ABOUT_DIAL_ID		7002
#define KEY_DIAL_ID			7003

/*--------------------------------------------------------------------*/

/* Fatal dialog items */

#define FATAL_DIAL_HIGHLIGHT_ITEM	2
#define FATAL_DIAL_TEXT_ITEM		3

/* Caution dialog items */

#define CAUTION_DIAL_HIGHLIGHT_ITEM	2
#define CAUTION_DIAL_TEXT_ITEM		3

/* Caution erc dialog item numbers */

#define CAUTION_ERC_DIAL_HIGHLIGHT_ITEM	2
#define CAUTION_ERC_DIAL_TEXT_ITEM		3
#define CAUTION_ERC_ERC_ITEM_NO			4

/* Items for GET_NAME_DIAL */

#define NAME_ITEM_NO			3
#define NAME_HIGHLIGHT_ITEM_NO	5

/* About dialog items */

#define ABOUT_HIGHLIGHT_ITEM	2

/* Keys dialog items */

#define KEYS_ITEM_OK			1
#define KEYS_ITEM_CANCEL		2
#define KEYS_ITEM_HIGHLIGHT		3
#define KEYS_ITEM_LEFT_KEY		4
#define KEYS_ITEM_ROT_KEY		5
#define KEYS_ITEM_RIGHT_KEY		6
#define KEYS_ITEM_DROP_KEY		7

/*--------------------------------------------------------------------*/

#endif
