/**********************************************************************\

File:		game.h

Purpose:	This is the screen interface module for the Tetris program.
			

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

#ifndef game_H
#define game_H

/*--------------------------------------------------------------------*/

extern Boolean game_init(void);
extern void game_begin(void);
extern void game_term(void);

/*--------------------------------------------------------------------*/

/* Support routines for the tetris module */

extern void game_del_row(int victim);
extern void game_set(int x, int y, unsigned char pattern);
extern unsigned char game_get(int x, int y);

extern void game_score_changed(void);
extern void game_over(void);

extern void describe_next_begin(void);
extern void describe_next_cell(int x, int y, unsigned char pattern);
extern void describe_next_end(void);

/*--------------------------------------------------------------------*/

/* Routines for higher level modules to call */

extern void game_periodic(void);

extern void game_new(void);
extern void game_save_as(void);
extern void game_save(void);
extern void game_open(void);
extern void game_load(Str255 fname, INTEGER vref);

extern OSErr game_save_location(INTEGER pref_file);
extern void game_load_location(void);

extern Boolean game_is_over(void);

/*--------------------------------------------------------------------*/

#endif

