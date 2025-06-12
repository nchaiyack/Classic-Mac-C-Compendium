/**********************************************************************\

File:		controls.h

Purpose:	This module contains code relating to the controls for
			playing the game.
			

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

#ifndef controls_H
#define controls_H

/*--------------------------------------------------------------------*/

/* This structure contains the information about the keys used to
   control the game and the play options chosen by the user.  The key
   scan code is used for processing the keys and the symbol is used for
   showing to the user which key it is. */

enum { KEY_LEFT, KEY_ROT, KEY_RIGHT, KEY_DROP, KEY_NUMBER_OF };

struct Ctrls {
	unsigned char code[KEY_NUMBER_OF];
	unsigned char sym[KEY_NUMBER_OF];
	Boolean show_next_piece;
	Boolean sound_on;
};

extern struct Ctrls ctrls;

/*--------------------------------------------------------------------*/

extern Boolean controls_init(void);

extern void controls_edit(void);

extern void controls_load(void);
extern OSErr controls_save(INTEGER pref_file);

/*--------------------------------------------------------------------*/

#endif
