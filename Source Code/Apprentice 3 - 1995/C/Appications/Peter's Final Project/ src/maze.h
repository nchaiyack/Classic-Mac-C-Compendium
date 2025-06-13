/*
 *  Peter's Final Project -- A texture mapping demonstration
 *  © 1995, Peter Mattis
 *
 *  E-mail:
 *  petm@soda.csua.berkeley.edu
 *
 *  Snail-mail:
 *   Peter Mattis
 *   557 Fort Laramie Dr.
 *   Sunnyvale, CA 94087
 *
 *  Avaible from:
 *  http://www.csua.berkeley.edu/~petm/final.html
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __MAZE_H__
#define __MAZE_H__

#include "type.defs.h"

MAZE make_maze (void);
void free_maze (MAZE);

void maze_init (MAZE, short, long);
void maze_describe (MAZE);

#define maze_size(m)		          ((m)->size)
#define maze_element(m, row, col)         (*((m)->data + (row) * maze_size(m) + (col)))
#define maze_element_state(m, row, col)   (maze_element(m, row, col).state)
#define maze_element_id(m, row, col)      (maze_element(m, row, col).id)
#define maze_element_sector(m, row, col)  (maze_element(m, row, col).sector)

#define set_maze_element(m, row, col, k)         (maze_element(m, row, col) = (k))
#define set_maze_element_state(m, row, col, k)   (maze_element_state(m, row, col) = (k))
#define set_maze_element_id(m, row, col, k)      (maze_element_id(m, row, col) = (k))
#define set_maze_element_sector(m, row, col, k)  (maze_element_sector(m, row, col) = (k))

#endif /* __MAZE_H__ */
