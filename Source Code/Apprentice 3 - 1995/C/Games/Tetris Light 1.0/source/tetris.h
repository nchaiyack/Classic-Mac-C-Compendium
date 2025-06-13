/**********************************************************************\

File:		tetris.h

Purpose:	Header file for the tetris program module.
			

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

#ifndef tetris_H
#define tetris_H

/*--------------------------------------------------------------------*/

#define NUMBER_ROWS	18
#define NUMBER_COLS	10

#define MAX_BLOCK_SIZE		4

#define NUMBER_BLOCK_TYPES	7

typedef enum {
	move_left,
	move_right,
	move_down,
	move_drop,
	move_anticlockwise
} Move_direction;

/*--------------------------------------------------------------------*/

/* This structure is used when saving and restoring a game.  It stores
   all the information needed to save and restore the game. */

struct Tetris_state {
	unsigned short score;
	
	int left;
	int top;
	int block;
	int orientation;

	int next_block;
	int next_orientation;
};

/*--------------------------------------------------------------------*/

extern void tetris_start(struct Tetris_state *state);

extern Boolean tetris_try_move(Move_direction direction);
extern void tetris_periodic(void);

extern void tetris_pause(Boolean paused);

extern unsigned short tetris_score(void);
extern void tetris_state_get(struct Tetris_state *state);

/*--------------------------------------------------------------------*/

#endif
