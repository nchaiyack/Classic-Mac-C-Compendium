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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "list.h"
#include "maze.h"
#include "sys.stuff.h"
#include "utils.h"

/*
 * A private type declaration.
 */
typedef struct {
	short x, y;
} WALL;

/*
 * Declare the functions private to this module.
 */

static WALL *make_wall (short, short);
static void free_wall (WALL *);
static void add_wall (WALL *);
static WALL *random_wall (short);

/*
 * Make a maze object.
 */

MAZE
make_maze ()
{
	MAZE maze;

	maze = (MAZE) ALLOC (sizeof (struct _MAZE));

	maze->data = NULL;
	maze->size = 0;

	return maze;
}

/*
 * Free a maze object.
 */

void
free_maze (maze)
	MAZE maze;
{
	if (maze)
		FREE (maze->data);
	FREE (maze);
}

/*
 * Initialize a maze to the given size. The seed value
 *  is used to initialize the random number generator
 *  which allows use to generate random mazes of virtually
 *  any size. 
 */

void
maze_init (maze, size, seed)
	MAZE maze;
	short size;
	long seed;
{
	short i, j;
	short wall_count;
	short cell_count;
	short id_search;
	short id_replace;
	WALL *wall;

	if (maze == NULL)
		fatal_error ("NULL maze passed to maze_init");
	if (size < 3)
		fatal_error ("Maze size too small -- %d", size);

	/*
	 * Even maze sizes are unwanted. (Try commenting this
	 *  out to see why).
	 */
	if ((size % 2) == 0)
	{
		printf ("maze size is even (%d)...making it odd(%d)\n", size, size + 1);
		size++;
	}

	/*
	 * Set the maze size and allocate the data array.
	 */
	maze->size = size;

	maze->data = (MAZE_CELL *) ALLOC (sizeof (MAZE_CELL) * maze->size * maze->size);

	/*
	 * Initialize every element to FALSE. That is, to empty.
	 * Give each element a unique id.
	 */
	for (i = 0; i < maze_size (maze); i++)
		for (j = 0; j < maze_size (maze); j++)
		{
			set_maze_element_state (maze, i, j, FALSE);
			set_maze_element_id (maze, i, j, i * maze_size (maze) + j);
		}

	/*
	 * Turn the edges on.
	 */
	for (i = 0; i < maze->size; i++)
	{
		set_maze_element_state (maze, i, 0, TRUE);
		set_maze_element_state (maze, 0, i, TRUE);
		set_maze_element_state (maze, i, maze_size (maze) - 1, TRUE);
		set_maze_element_state (maze, maze_size (maze) - 1, i, TRUE);
	}

	/*
	 * Fill in the walls. Certain walls should never be removed
	 *  so don't add them to the wall list. Also, only increment
	 *  the wall count if a wall is added to the list.
	 */
	wall_count = 0;
	for (i = 1; i < maze_size (maze) - 1; i++)
	{
		for (j = 1; j < maze_size (maze) - 1; j++)
		{
			if (((i + j) % 2) == 1)
			{
				set_maze_element_state (maze, i, j, TRUE);
				add_wall (make_wall (i, j));
				wall_count++;
			}
			else if (((i % 2) == 0) || ((j % 2) == 0))
			{
				set_maze_element_state (maze, i, j, TRUE);
			}
		}
	}

	/*
	 * Seed the random number generator.
	 */
	srand (seed);

	/*
	 * The goal here is to take all the disconnected cells in the
	 *  maze and connect them by removing random walls. Each cell
	 *  can be thought of as a group of cells. We start off with
	 *  each cell being in its own group. As walls are knocked out
	 *  groups are joined until in the end we have one big group.
	 * This is really one of the best maze generation algorithms
	 *  around.
	 */
	cell_count = (maze_size (maze) >> 1) * (maze_size (maze) >> 1);
	while (cell_count > 1)
	{
		wall = random_wall (wall_count--);

		if ((wall->x % 2) == 0)
		{
			if (maze_element_id (maze, wall->x - 1, wall->y) !=
				maze_element_id (maze, wall->x + 1, wall->y))
			{
				cell_count--;
				set_maze_element_state (maze, wall->x, wall->y, FALSE);

				id_search = maze_element_id (maze, wall->x + 1, wall->y);
				id_replace = maze_element_id (maze, wall->x - 1, wall->y);

				for (i = 1; i < maze_size (maze) - 1; i++)
					for (j = 1; j < maze_size (maze) - 1; j++)
						if (maze_element_id (maze, i, j) == id_search)
							set_maze_element_id (maze, i, j, id_replace);
			}
		}
		else if ((wall->y % 2) == 0)
		{
			if (maze_element_id (maze, wall->x, wall->y - 1) !=
				maze_element_id (maze, wall->x, wall->y + 1))
			{
				cell_count--;
				set_maze_element_state (maze, wall->x, wall->y, FALSE);

				id_search = maze_element_id (maze, wall->x, wall->y + 1);
				id_replace = maze_element_id (maze, wall->x, wall->y - 1);

				for (i = 1; i < maze_size (maze) - 1; i++)
					for (j = 1; j < maze_size (maze) - 1; j++)
						if (maze_element_id (maze, i, j) == id_search)
							set_maze_element_id (maze, i, j, id_replace);
			}
		}
	}
}

/*
 * Print a description of a maze.
 */

void
maze_describe (maze)
	MAZE maze;
{
	short i, j;

	assert (maze);
	assert (maze->data);

	for (i = 0; i < maze_size (maze); i++)
	{
		for (j = 0; j < maze_size (maze); j++)
		{
			if (maze_element_state (maze, i, j))
				printf ("**");
			else
				printf ("  ");
		}
		printf ("\n");
	}
}

/*
 * Keep a list of walls and a list of free walls.
 */
static LIST wall_list = NULL;
static LIST free_wall_list = NULL;

/*
 * Make a wall and initialize its values appropriately
 */
 
static WALL *
make_wall (x, y)
	short x;
	short y;
{
	WALL *wall;
	LIST list;

	if (free_wall_list)
	{
		wall = list_datum (wall_list);

		list = wall_list;
		wall_list = list_next (wall_list);

		free_list (list);
	}
	else
	{
		wall = (WALL *) ALLOC (sizeof (WALL));
	}

	wall->x = x;
	wall->y = y;

	return wall;
}

/*
 * Free a wall.
 */
 
static void
free_wall (wall)
	WALL *wall;
{
	 FREE (wall);
}

/*
 * Add a wall to the wall list.
 */

static void
add_wall (wall)
	WALL *wall;
{
	LIST list;

	list = make_list ();
	set_list_datum (list, wall);
	set_list_next (list, wall_list);
	wall_list = list;
}

/*
 * Pick a random wall from the wall list and
 *  remove that wall from the list.
 */

static WALL *
random_wall (total)
	short total;
{
	short n;
	LIST t1, t2;
	WALL *wall;

	n = rand () % total;

	if (n == 0)
	{
		wall = list_datum (wall_list);

		t1 = wall_list;
		wall_list = list_next (wall_list);
		free_list (t1);
	}
	else
	{
		t2 = wall_list;
		t1 = list_next (wall_list);

		while (--n)
		{
			t2 = t1;
			t1 = list_next (t1);
		}

		wall = list_datum (t1);
		set_list_next (t2, list_next (t1));
		free_list (t1);
	}

	return wall;
}
