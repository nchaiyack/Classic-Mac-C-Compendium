/**********************************************************************\

File:		tetris.c

Purpose:	This module implements the Tetris game.  It has been written
			in a generic manner to aid porting to other platforms.  Only
			the timer and random number generator are Macintosh specific.
			

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

#include "game.h"
#include "tetris.h"

/*--------------------------------------------------------------------*/

/* These items determine the levels in the game, a table of scores
   which indicate the next level is reached, and the time delay between
   block drops for the levels.  The higher the level, the faster they
   drop.  These time delays are in ticks (60ths of a second). */

#define NUMBER_LEVELS		11

static unsigned int level_threshold[NUMBER_LEVELS] =
	{  1,  2,  3,  4,  5,  6,  7, 25, 50, 100, 500 };
static long drop_delays[NUMBER_LEVELS + 1] =
	{ 60, 50, 40, 30, 27, 25, 23, 20, 19,  18,  17, 16 };

/*--------------------------------------------------------------------*/

/* Local globals */

static Boolean running = FALSE;
static Boolean paused = FALSE;

static long drop_time;
static unsigned int score;
static int level = 0;

/*--------------------------------------------------------------------*/

/* Information on the blocks */

typedef struct {
	unsigned char map[MAX_BLOCK_SIZE][MAX_BLOCK_SIZE];
	int width;
	int offset;
} Block_type;

static int block;
static int orientation;
static Block_type *current;

static int next_block;
static int next_orientation;
static Block_type *next;

static int left;
static int top;

static Block_type possible[NUMBER_BLOCK_TYPES * 4] = {

/* Object 0 - bar */
	
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 1, 1, 1 }}, 4, 0 },
{{{ 0, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 0, 0 }}, 1, 1 },
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 1, 1, 1 }}, 4, 0 },
{{{ 0, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 0, 0 }}, 1, 1 },
	
/* Object 1 - L */
	
{{{ 0, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 1, 0 }}, 2, 1 },
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 1, 0 }, { 1, 1, 1, 0 }}, 3, 0 },
{{{ 0, 0, 0, 0 }, { 1, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 0, 0 }}, 2, 0 },
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 1, 1, 0 }, { 1, 0, 0, 0 }}, 3, 0 },
	
/* Object 2 - reverse L */
	
{{{ 0, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 0, 0 }, { 1, 1, 0, 0 }}, 2, 0 },
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 1, 1, 0 }, { 0, 0, 1, 0 }}, 3, 0 },
{{{ 0, 0, 0, 0 }, { 0, 1, 1, 0 }, { 0, 1, 0, 0 }, { 0, 1, 0, 0 }}, 2, 1 },
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 0, 0, 0 }, { 1, 1, 1, 0 }}, 3, 0 },
	
/* Object 3 - S */
	
{{{ 0, 0, 0, 0 }, { 0, 0, 1, 0 }, { 0, 1, 1, 0 }, { 0, 1, 0, 0 }}, 2, 1 },
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 1, 0, 0 }, { 0, 1, 1, 0 }}, 3, 0 },
{{{ 0, 0, 0, 0 }, { 0, 0, 1, 0 }, { 0, 1, 1, 0 }, { 0, 1, 0, 0 }}, 2, 1 },
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 1, 0, 0 }, { 0, 1, 1, 0 }}, 3, 0 },
	
/* Object 4 - reverse S */
	
{{{ 0, 0, 0, 0 }, { 1, 0, 0, 0 }, { 1, 1, 0, 0 }, { 0, 1, 0, 0 }}, 2, 0 },
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 1, 1, 0 }, { 1, 1, 0, 0 }}, 3, 0 },
{{{ 0, 0, 0, 0 }, { 1, 0, 0, 0 }, { 1, 1, 0, 0 }, { 0, 1, 0, 0 }}, 2, 0 },
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 1, 1, 0 }, { 1, 1, 0, 0 }}, 3, 0 },

/* Object 5 - T */
	
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 1, 0, 0 }, { 1, 1, 1, 0 }}, 3, 0 },
{{{ 0, 0, 0, 0 }, { 0, 1, 0, 0 }, { 1, 1, 0, 0 }, { 0, 1, 0, 0 }}, 2, 0 },
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 1, 1, 0 }, { 0, 1, 0, 0 }}, 3, 0 },
{{{ 0, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 1, 0 }, { 0, 1, 0, 0 }}, 2, 1 },
	
/* Object 6 - square */
	
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 1, 0, 0 }, { 1, 1, 0, 0 }}, 2, 0 },
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 1, 0, 0 }, { 1, 1, 0, 0 }}, 2, 0 },
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 1, 0, 0 }, { 1, 1, 0, 0 }}, 2, 0 },
{{{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 1, 0, 0 }, { 1, 1, 0, 0 }}, 2, 0 }
};

/*--------------------------------------------------------------------*/

/* Nudge information for rotations. Nudges are used when blocks can't
   be rotated because they are too close to the edge.  The blocks are
   tried at the following positions so that the rotate can succeed. */

#define NUM_NUDGES	4

static int nudges[NUM_NUDGES] = {0, -1, +1, - 2};

/*--------------------------------------------------------------------*/

static int random_number(int limit)
/* Returns a random number in the range 0 to limit-1 inclusive. This
   implementation uses the Macintosh QuickDraw random number routine. */
{
	return (Random() & 0x7FFF) % limit;
}

/*--------------------------------------------------------------------*/

static unsigned long timer(void)
/* Returns the timer clock ticks. This routine uses the Macintosh
   `TickCount' routine. */
{
	return TickCount();
}

/*--------------------------------------------------------------------*/

static void set_next_drop_time(void)
/* Set the time when the block can next drop.  This time is the current
   time plus the delay time appropriate to the current level. */
{
	drop_time = timer() + drop_delays[level];
}

/*--------------------------------------------------------------------*/

static void new_block(void)
/* Randomly generates a new block which is stored in the next block
   variables.  The old next block is copied to the current block. */
{
	register int x, y;
	
	/* Copy old next block to current block */
	
	block = next_block;
	orientation = next_orientation;
	current = next;
	
	/* Place the new current block randomly at the top of the field */
	
	left = random_number(NUMBER_COLS - current->width + 1) - current->offset;
	top = -MAX_BLOCK_SIZE;
	
	set_next_drop_time();
	
	/* Randomly generate a next block */
	
	next_block = random_number(NUMBER_BLOCK_TYPES);
	next_orientation = random_number(4);
	next = possible + (4 * next_block + next_orientation);
			
	/* Describe a preview of new block */
	
	describe_next_begin();
	for (x = 0; x < MAX_BLOCK_SIZE; x++)
		for (y = 0; y < MAX_BLOCK_SIZE; y++)
			if (next->map[y][x])
				describe_next_cell(x, y, next_block + 1);
	describe_next_end();
}

/*--------------------------------------------------------------------*/

static void touchdown(void)
/* Routine to handle when the block has dropped as far as it can.
   Checks for filled rows, processes them, and generates a new block. */
{
	register int row;
	
	if (top == -MAX_BLOCK_SIZE) {
		running = FALSE;
		game_over();
		return;
	}

	/* Check for completed rows */
	
	for (row = NUMBER_ROWS - 1; row >= 0; row--) {
		register int count = 0;
		register int col = 0;
		
		while (col < NUMBER_COLS && game_get(col, row) != 0)
			col++;
		
		if (col == NUMBER_COLS) {
			game_del_row(row);
			score++;
			game_score_changed();
			
			if (level < NUMBER_LEVELS && score >= level_threshold[level])
				level++;
			row++; /* undo decrement of row to retest this row again */
		}
	}
	
	new_block();
}

/*--------------------------------------------------------------------*/

void tetris_start(struct Tetris_state *state)
/* This routine is called to reset and start playing the game.  This is
   called when a new game starts, or a game is loaded up for playing.
   If the `state' is null, a new game is started, otherwise the game
   is restored from the `state' information. */
{
	if (! state) {
		/* Generate new blocks */
		
		new_block();
		new_block();
		score = 0;
		level = 0;	/* Score of zero corresponds to level zero */
	} else {
		register int x, y;
		
		/* Restore state */
		
		score = state->score;
		level = 0;
		while (level < NUMBER_LEVELS && score >= level_threshold[level])
			level++;

		left = state->left;
		top = state->top;
		block = state->block;
		orientation = state->orientation;
		current = possible + (4 * block + orientation);

	 	next_block = state->next_block;
		next_orientation = state->next_orientation;
		next = possible + (4 * next_block + next_orientation);
	
		/* Describe a preview of new block */
	
		describe_next_begin();
		for (x = 0; x < MAX_BLOCK_SIZE; x++)
			for (y = 0; y < MAX_BLOCK_SIZE; y++)
				if (next->map[y][x])
					describe_next_cell(x, y, next_block + 1);
		describe_next_end();
	}

	/* Start the game */
	
	paused = FALSE;
	running = TRUE;
}

/*--------------------------------------------------------------------*/

static Boolean can_move(int new_left, int new_top, Block_type *new_block)
/* Trys changing the current block to the `new_block' in the
   new position. If it does fit, the old object is erased and
   the new object drawn. The current variables are not changed.
   Returns TRUE if it succeeds, FALSE if it failed. */
{
	register int row, col;

	/* Check if position is possible */
	
	if (new_top > NUMBER_ROWS - MAX_BLOCK_SIZE ||
		new_left + new_block->offset < 0 ||
		new_left + new_block->offset + new_block->width > NUMBER_COLS)
		return FALSE; /* falls outside field */
	
	for (row = 0; row < MAX_BLOCK_SIZE; row++)
		for (col = new_block->offset;
			 col < new_block->offset + new_block->width; col++) {
			if (new_block->map[row][col]) {
				/* Solid part at this grid position, possible collision */
				
				register int ox = col + new_left - left;
				register int oy = row + new_top - top;
				
				if (ox < 0 || ox >= MAX_BLOCK_SIZE ||
				    oy < 0 || oy >= MAX_BLOCK_SIZE || (! current->map[oy][ox])) {
					/* Old was not here, so could hit */
					
					if (game_get(col + new_left, row + new_top) != 0)
						return FALSE;
				}
			}
		}
	
	/* Draw in new position */
	
	for (row = 0; row < MAX_BLOCK_SIZE; row++)
		for (col = new_block->offset;
			 col < new_block->offset + new_block->width; col++)
			if (new_block->map[row][col])
				game_set(col + new_left, row + new_top, block + 1);
			
	/* Clear old parts that don't exist in the new */

	for (row = 0; row < MAX_BLOCK_SIZE; row++)
		for (col = current->offset; col < current->offset + current->width; col++) {
			register int ox, oy;
			
			ox = left + col - new_left;
			oy = top + row - new_top;
			if (current->map[row][col] &&
				((ox < 0) || (ox >= MAX_BLOCK_SIZE) || (oy < 0) || (oy >= MAX_BLOCK_SIZE) ||
											! new_block->map[oy][ox]))
				game_set(left + col, top + row, 0);
		}

	return TRUE;
}

/*--------------------------------------------------------------------*/

Boolean tetris_try_move(Move_direction direction)
/* Tries to make a move in the given `direction'. Returns FALSE if it failed. */
{
	register int index;
	
	if (!running || paused)
		return FALSE;
	
	switch (direction) {
	case move_left:
		if (can_move(left - 1, top, current)) {
			left--;
			return TRUE;
		}
		break;
	case move_right:
		if (can_move(left + 1, top, current)) {
			left++;
			return TRUE;
		}
		break;
	case move_down:
		if (can_move(left, top + 1, current)) {
			top++;
			return TRUE;
		}
		break;
	case move_drop:
		while (can_move(left, top + 1, current)) {
			/* Small amount of delay for fast machines */
			register unsigned long t = timer();
			while (timer() == t)
				;
			
			top++;
		}
		touchdown();
		return FALSE;
		break;
	case move_anticlockwise:
		/* Try in all nudged positions */
		
		for (index = 0; index < NUM_NUDGES; index++) {
			register int new_orient;
			register Block_type *new_block;
			register new_left = left + nudges[index];

			new_orient = (orientation + 1) % 4;
			new_block = possible + (4 * block + new_orient);
			if (can_move(new_left, top, new_block)) {
				orientation = new_orient;
				current = new_block;
				left = new_left;
				return TRUE;
			}
		}
				
		break;
	}
	
	return FALSE;
}

/*--------------------------------------------------------------------*/

void tetris_periodic(void)
/* This routine must be called as often as possible when playing the
   game.  It moves the block down when the delay time has been reached. */
{
	register long now;
	
	if (running && !paused) {
		now = timer();
		if (now > drop_time) {
			if (! tetris_try_move(move_down))
				touchdown();
			set_next_drop_time();
		}
	}
}

/*--------------------------------------------------------------------*/

void tetris_pause(Boolean set_paused)
/* Pauses the game. */
{
	if (running) {
		if (paused && !set_paused || !paused && set_paused) {
			paused = set_paused;
			if (!paused)
				set_next_drop_time();
		}
	}
}

/*--------------------------------------------------------------------*/

unsigned short tetris_score(void)
/* Returns the current score. */
{
	return score;
}

/*--------------------------------------------------------------------*/

void tetris_state_get(struct Tetris_state *state)
{
	state->score = score;
	
	state->left = left;
	state->top = top;
	state->block = block;
	state->orientation = orientation;

	state->next_block = next_block;
	state->next_orientation = next_orientation;
}
