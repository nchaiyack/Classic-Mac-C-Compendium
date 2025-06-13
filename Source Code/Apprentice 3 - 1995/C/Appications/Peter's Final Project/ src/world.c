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
#include <stdio.h>

#include "face.h"
#include "list.h"
#include "maze.h"
#include "object.h"
#include "sector.h"
#include "sys.stuff.h"
#include "texture.h"
#include "world.h"

/*
 * This define affect the size (width) of a sector.
 */

#define CELL_SIZE NUM_ONE * 2

/*
 * Declare the functions private to this module.
 */

void world_make_points (void);
SECTOR world_make_sector (short, short);
FACE world_make_floor_face (short, short);
FACE world_make_ceiling_face (short, short);
FACE world_make_west_face (short, short);
FACE world_make_east_face (short, short);
FACE world_make_north_face (short, short);
FACE world_make_south_face (short, short);

/*
 * These global variables are used instead of passing
 *  lots of parameters around in function calls.
 */
 
static WORLD cur_world;
static MAZE cur_maze;

/*
 * The offset into the points array where the normals begin.
 */
 
static long normal_offset;

/*
 * The textures to use for the floor, ceiling, and walls respectively.
 */

static TEXTURE floor_texture;
static TEXTURE ceiling_texture;
static TEXTURE wall_texture;

/*
 * Allocate a world.
 */

WORLD
make_world ()
{
	WORLD world;

	world = (WORLD) ALLOC (sizeof (_WORLD));

	set_world_points (world, NULL);
	set_world_faces (world, NULL);

	return world;
}

/*
 * Free a world.
 */

void
free_world (w)
	WORLD w;
{
	FREE (w);
}

/*
 * Initialize a world from a maze.
 */

void
world_from_maze (w, m, wtn, ftn, ctn)
	WORLD w;
	MAZE m;
	char *wtn, *ftn, *ctn;
{
	long n_points;
	short i, j;
	SECTOR s;

	/*
	 * Set up some global variables so we don't have to
	 *  pass them around to every function.
	 */
	cur_world = w;
	cur_maze = m;

	/*
	 * Calculate the number of points needed for the world.
	 * "2 * (maze_size(cur_maze) - 1) * (maze_size(cur_maze) - 1)"
	 *  is the number of points needed to define the every possible
	 *  face in the maze.
	 * And, of course, we need 6 extra points for normal vectors.
	 *  (It's a simple maze, there are only 6 possible face orientations).
	 */
	n_points = (2 * (maze_size (cur_maze) - 1) * (maze_size (cur_maze) - 1) + 6);

	/*
	 * Allocate the points.
	 */
	world_points (cur_world) = (POINT) ALLOC (sizeof (_POINT) * n_points);
	assert (world_points (cur_world) != NULL);

	/*
	 * Initialize the points.
	 */
	world_make_points ();

	/*
	 * Create some textures.
	 */
	wall_texture = make_texture ();
	floor_texture = make_texture ();
	ceiling_texture = make_texture ();

	/*
	 * Read in the wall texture.
	 */
	if (wtn)
		texture_read (wall_texture, wtn);
	else
		texture_read (wall_texture, ":textures:marble_wall.jpg");

	/*
	 * Read in the floor texture.
	 */
	if (ftn)
		texture_read (floor_texture, ftn);
	else
		texture_read (floor_texture, ":textures:marble_floor.jpg");

	/*
	 * Read in the ceiling texture.
	 */
	if (ctn)
		texture_read (ceiling_texture, ctn);
	else
		texture_read (ceiling_texture, ":textures:marble_floor.jpg");

	/*
	 * Allocate and initialize all the sectors in this world.
	 * Initialization involves creating the faces.
	 */
	for (i = 0; i < maze_size (cur_maze); i++)
		for (j = 0; j < maze_size (cur_maze); j++)
		{
			if (!maze_element_state (cur_maze, i, j))
			{
				set_maze_element_sector (cur_maze, i, j, make_sector ());
				world_make_sector (i - 1, j - 1);
			}
			else
			{
				set_maze_element_sector (cur_maze, i, j, NULL);
			}
		}

	/*
	 * Once all the sectors have been created, all we need
	 *  to do is link them together. The result is one large
	 *  graph. (Or small, depends on how big the maze is).
	 * Links are only made to the neighbors in the 4 cardinal.
	 */
	for (i = 0; i < maze_size (cur_maze); i++)
	{
		for (j = 0; j < maze_size (cur_maze); j++)
		{
			s = maze_element_sector (cur_maze, i, j);
			if (s)
			{
				if (maze_element_sector (cur_maze, i, j - 1))
					sector_neighbors (s) = sectors_add_sector (sector_neighbors (s),
					maze_element_sector (cur_maze, i, j - 1));
				if (maze_element_sector (cur_maze, i + 1, j))
					sector_neighbors (s) = sectors_add_sector (sector_neighbors (s),
					maze_element_sector (cur_maze, i + 1, j));
				if (maze_element_sector (cur_maze, i, j + 1))
					sector_neighbors (s) = sectors_add_sector (sector_neighbors (s),
					maze_element_sector (cur_maze, i, j + 1));
				if (maze_element_sector (cur_maze, i - 1, j))
					sector_neighbors (s) = sectors_add_sector (sector_neighbors (s),
					maze_element_sector (cur_maze, i - 1, j));
			}
		}
	}
}

/*
 * Initialize the points for the current world.
 */

void
world_make_points ()
{
	short i, j, n;
	POINT p;

	/*
	 * Initialize the floor points.
	 */
	n = 0;
	for (i = 0; i < (maze_size (cur_maze) - 1); i++)
	{
		for (j = 0; j < (maze_size (cur_maze) - 1); j++)
		{
			p = &world_point (cur_world, n++);

			set_point_x (p, j * CELL_SIZE);
			set_point_y (p, NUM_ZERO);
			set_point_z (p, i * CELL_SIZE);
			set_point_w (p, NUM_ONE);
		}
	}

	/*
	 * Initialize the ceiling points. (This is the same
	 *  as above except the y coordinate is set to 1).
	 */
	for (i = 0; i < (maze_size (cur_maze) - 1); i++)
	{
		for (j = 0; j < (maze_size (cur_maze) - 1); j++)
		{
			p = &world_point (cur_world, n++);

			set_point_x (p, j * CELL_SIZE);
			set_point_y (p, CELL_SIZE);
			set_point_z (p, i * CELL_SIZE);
			set_point_w (p, NUM_ONE);
		}
	}

	/*
	 * Remember the normal offset.
	 */
	normal_offset = n;

	/*
	 * Initialize the normals. We need a normal in the
	 *  both the positive and negative direction of x, y
	 *  and z;
	 */
	
	p = &world_point (cur_world, n++);
	set_point_x (p, NUM_ZERO);
	set_point_y (p, NUM_ONE);
	set_point_z (p, NUM_ZERO);
	set_point_w (p, NUM_ZERO);

	p = &world_point (cur_world, n++);
	set_point_x (p, NUM_ZERO);
	set_point_y (p, -NUM_ONE);
	set_point_z (p, NUM_ZERO);
	set_point_w (p, NUM_ZERO);

	p = &world_point (cur_world, n++);
	set_point_x (p, NUM_ONE);
	set_point_y (p, NUM_ZERO);
	set_point_z (p, NUM_ZERO);
	set_point_w (p, NUM_ZERO);

	p = &world_point (cur_world, n++);
	set_point_x (p, -NUM_ONE);
	set_point_y (p, NUM_ZERO);
	set_point_z (p, NUM_ZERO);
	set_point_w (p, NUM_ZERO);

	p = &world_point (cur_world, n++);
	set_point_x (p, NUM_ZERO);
	set_point_y (p, NUM_ZERO);
	set_point_z (p, NUM_ONE);
	set_point_w (p, NUM_ZERO);

	p = &world_point (cur_world, n++);
	set_point_x (p, NUM_ZERO);
	set_point_y (p, NUM_ZERO);
	set_point_z (p, -NUM_ONE);
	set_point_w (p, NUM_ZERO);
}

/*
 * Create a sector for the current world.
 */

SECTOR
world_make_sector (i, j)
	short i, j;
{
	SECTOR sector;
	FACE face;

	/*
	 * Grab the sector for this cell from the maze.
	 */
	sector = maze_element_sector (cur_maze, i + 1, j + 1);

	/*
	 * Create the various faces and add them to this
	 *  sectors list.
	 */
	
	face = world_make_floor_face (i, j);
	if (face)
		set_sector_faces (sector, faces_add_face (sector_faces (sector), face));

	face = world_make_ceiling_face (i, j);
	if (face)
		set_sector_faces (sector, faces_add_face (sector_faces (sector), face));

	face = world_make_west_face (i, j);
	if (face)
		set_sector_faces (sector, faces_add_face (sector_faces (sector), face));

	face = world_make_east_face (i, j);
	if (face)
		set_sector_faces (sector, faces_add_face (sector_faces (sector), face));

	face = world_make_north_face (i, j);
	if (face)
		set_sector_faces (sector, faces_add_face (sector_faces (sector), face));

	face = world_make_south_face (i, j);
	if (face)
		set_sector_faces (sector, faces_add_face (sector_faces (sector), face));

	return sector;
}

/*
 * Create a floor face.
 */

FACE
world_make_floor_face (i, j)
	short i, j;
{
	FACE face;
	short temp;
	short size;

	size = maze_size (cur_maze) - 1;

	face = make_face ();
	set_face_normal (face, &world_point (cur_world, normal_offset + 1));
	set_face_texture (face, floor_texture);
	set_face_obstructs (face, TRUE);

	temp = i * size + j + (size * size);
	face_points (face) = points_add_point (face_points (face),
		&world_point (cur_world, temp));
	face_points (face) = points_add_point (face_points (face),
		&world_point (cur_world, temp + 1));
	face_points (face) = points_add_point (face_points (face),
		&world_point (cur_world, temp + 1 + size));
	face_points (face) = points_add_point (face_points (face),
		&world_point (cur_world, temp + size));

	set_face_texture_o (face, &world_point (cur_world, temp));
	set_face_texture_u (face, &world_point (cur_world, temp + 1));
	set_face_texture_v (face, &world_point (cur_world, temp + size));

	return face;
}

/*
 * Create a ceiling face.
 */

FACE
world_make_ceiling_face (i, j)
	short i, j;
{
	FACE face;
	short temp;
	short size;

	size = maze_size (cur_maze) - 1;

	face = make_face ();
	set_face_normal (face, &world_point (cur_world, normal_offset + 0));
	set_face_texture (face, ceiling_texture);
	set_face_obstructs (face, TRUE);

	temp = i * size + j;

	face_points (face) = points_add_point (face_points (face),
		&world_point (cur_world, temp));
	face_points (face) = points_add_point (face_points (face),
		&world_point (cur_world, temp + 1));
	face_points (face) = points_add_point (face_points (face),
		&world_point (cur_world, temp + 1 + size));
	face_points (face) = points_add_point (face_points (face),
		&world_point (cur_world, temp + size));

	set_face_texture_o (face, &world_point (cur_world, temp));
	set_face_texture_u (face, &world_point (cur_world, temp + 1));
	set_face_texture_v (face, &world_point (cur_world, temp + size));

	return face;
}

/*
 * Create a west face.
 */

FACE
world_make_west_face (i, j)
	short i, j;
{
	FACE face;
	short temp;
	short size;
	short offset;

	if (maze_element_state (cur_maze, i + 1, j))
	{
		/*
		 * There is a wall next to us, so make a "real" face.
		 */
		
		size = maze_size (cur_maze) - 1;
		offset = size * size;

		face = make_face ();
		set_face_normal (face, &world_point (cur_world, normal_offset + 2));
		set_face_texture (face, wall_texture);
		set_face_obstructs (face, TRUE);

		temp = i * size + j;
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + size));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + size + offset));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + offset));

		set_face_texture_o (face, &world_point (cur_world, temp + size));
		set_face_texture_u (face, &world_point (cur_world, temp));
		set_face_texture_v (face, &world_point (cur_world, temp + size + offset));

		return face;
	}
	else
	{
		/*
		 * There is nothing next to us, so make a "fake" face.
		 * (That is, it won't be visible and won't obstruct movement).
		 */
		
		face = make_face ();
		set_face_normal (face, &world_point (cur_world, normal_offset + 2));
		set_face_texture (face, wall_texture);
		set_face_obstructs (face, FALSE);

		size = maze_size (cur_maze) - 1;
		temp = i * size + j;
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + size));

		set_face_texture_o (face, &world_point (cur_world, temp + size));
		set_face_texture_u (face, &world_point (cur_world, temp));
		set_face_texture_v (face, &world_point (cur_world, temp + size + offset));

		return face;
	}

	return NULL;
}

/*
 * Create an east face.
 */

FACE
world_make_east_face (i, j)
	short i, j;
{
	FACE face;
	short temp;
	short size;
	short offset;

	if (maze_element_state (cur_maze, i + 1, j + 2))
	{
		/*
		 * There is a wall next to us, so make a "real" face.
		 */
		
		size = maze_size (cur_maze) - 1;
		offset = size * size;

		face = make_face ();
		set_face_normal (face, &world_point (cur_world, normal_offset + 3));
		set_face_texture (face, wall_texture);
		set_face_obstructs (face, TRUE);

		temp = i * size + j + 1;
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + size));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + size + offset));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + offset));

		set_face_texture_o (face, &world_point (cur_world, temp));
		set_face_texture_u (face, &world_point (cur_world, temp + size));
		set_face_texture_v (face, &world_point (cur_world, temp + offset));

		return face;
	}
	else
	{
		/*
		 * There is nothing next to us, so make a "fake" face.
		 * (That is, it won't be visible and won't obstruct movement).
		 */
		
		face = make_face ();
		set_face_normal (face, &world_point (cur_world, normal_offset + 3));
		set_face_texture (face, wall_texture);
		set_face_obstructs (face, FALSE);

		size = maze_size (cur_maze) - 1;
		temp = i * size + j + 1;
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + size));

		set_face_texture_o (face, &world_point (cur_world, temp));
		set_face_texture_u (face, &world_point (cur_world, temp + size));
		set_face_texture_v (face, &world_point (cur_world, temp + offset));

		return face;
	}

	return NULL;
}

/*
 * Create a north face.
 */

FACE
world_make_north_face (i, j)
	short i, j;
{
	FACE face;
	short temp;
	short size;
	short offset;

	if (maze_element_state (cur_maze, i, j + 1))
	{
		/*
		 * There is a wall next to us, so make a "real" face.
		 */
		
		size = maze_size (cur_maze) - 1;
		offset = size * size;

		face = make_face ();
		set_face_normal (face, &world_point (cur_world, normal_offset + 4));
		set_face_texture (face, wall_texture);
		set_face_obstructs (face, TRUE);

		temp = i * size + j;
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + 1));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + 1 + offset));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + offset));

		set_face_texture_o (face, &world_point (cur_world, temp));
		set_face_texture_u (face, &world_point (cur_world, temp + 1));
		set_face_texture_v (face, &world_point (cur_world, temp + offset));

		return face;
	}
	else
	{
		/*
		 * There is nothing next to us, so make a "fake" face.
		 * (That is, it won't be visible and won't obstruct movement).
		 */
		
		face = make_face ();
		set_face_normal (face, &world_point (cur_world, normal_offset + 4));
		set_face_texture (face, wall_texture);
		set_face_obstructs (face, FALSE);

		size = maze_size (cur_maze) - 1;
		temp = i * size + j;
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + 1));

		set_face_texture_o (face, &world_point (cur_world, temp));
		set_face_texture_u (face, &world_point (cur_world, temp + 1));
		set_face_texture_v (face, &world_point (cur_world, temp + offset));

		return face;
	}

	return NULL;
}

/*
 * Create a south face.
 */

FACE
world_make_south_face (i, j)
	short i, j;
{
	FACE face;
	short temp;
	short size;
	short offset;

	if (maze_element_state (cur_maze, i + 2, j + 1))
	{
		/*
		 * There is a wall next to us, so make a "real" face.
		 */
		
		size = maze_size (cur_maze) - 1;
		offset = size * size;

		face = make_face ();
		set_face_normal (face, &world_point (cur_world, normal_offset + 5));
		set_face_texture (face, wall_texture);
		set_face_obstructs (face, TRUE);

		temp = (i + 1) * size + j;
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + 1));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + 1 + offset));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + offset));

		set_face_texture_o (face, &world_point (cur_world, temp + 1));
		set_face_texture_u (face, &world_point (cur_world, temp));
		set_face_texture_v (face, &world_point (cur_world, temp + 1 + offset));

		return face;
	}
	else
	{
		/*
		 * There is nothing next to us, so make a "fake" face.
		 * (That is, it won't be visible and won't obstruct movement).
		 */
		
		face = make_face ();
		set_face_normal (face, &world_point (cur_world, normal_offset + 5));
		set_face_texture (face, wall_texture);
		set_face_obstructs (face, FALSE);

		size = maze_size (cur_maze) - 1;
		temp = (i + 1) * size + j;
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp));
		face_points (face) = points_add_point (face_points (face),
			&world_point (cur_world, temp + 1));

		set_face_texture_o (face, &world_point (cur_world, temp + 1));
		set_face_texture_u (face, &world_point (cur_world, temp));
		set_face_texture_v (face, &world_point (cur_world, temp + 1 + offset));

		return face;
	}

	return NULL;
}
