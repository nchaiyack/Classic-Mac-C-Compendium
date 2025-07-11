/*
 *  Peter's Final Project -- A texture mapping demonstration
 *  � 1995, Peter Mattis
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
#include <stdlib.h>

#include "collision.h"
#include "computer.h"
#include "engine.h"
#include "matrix.vector.h"
#include "maze.h"
#include "object.h"
#include "render.h"
#include "scan.h"
#include "sector.h"
#include "sys.stuff.h"
#include "timer.h"
#include "world.h"

/*
 * Declare the functions private to this module.
 */

void do_engine_refresh (void);
void do_move_objects (OBJECTS);
void do_move_my_object (OBJECT);
void do_object_movement (OBJECT);

/*
 * Define some values for key encoding.
 */

static const short LEFT = 0x1;
static const short RIGHT = 0x2;
static const short UP = 0x4;
static const short DOWN = 0x8;

/*
 * Define some movement values.
 * The maximum values are to make sure no one
 *  moves to fast.
 */
 
static const NUM TURN_STEP = NUM_ONE * 0.07;
static const NUM TURN_MAX = NUM_ONE * 0.25;
static const NUM TURN_MIN = NUM_ONE * 0.01;

static const NUM MOVE_STEP = NUM_ONE * 0.10;
static const NUM MOVE_MAX = NUM_ONE * 0.25;
static const NUM MOVE_MIN = NUM_ONE * 0.01;

/*
 * A global debug variable. Not really used much now.
 */
short debug_level = 0;

/*
 * Lots of private variables. 
 */
 
static long keys = 0;
static TIMER frame_timer;
static TIMER global_timer;
static long frame_count;

static short maze_size = 15;
static char *server_name = NULL;
static char *wall_texture_name = NULL;
static char *floor_texture_name = NULL;
static char *ceiling_texture_name = NULL;

static MAZE maze;
static WORLD world;
static OBJECT view = NULL;
static OBJECT player = NULL;
static OBJECT enemy = NULL;
static OBJECTS objects = NULL;

/*
 * Set the maze size.
 */
 
void
do_set_maze_size (size)
	short size;
{
	maze_size = size;
}

/*
 * Set the wall texture.
 */

void
do_set_wall_texture (name)
	char *name;
{
	wall_texture_name = name;
}

/*
 * Set the floor texture.
 */

void
do_set_floor_texture (name)
	char *name;
{
	floor_texture_name = name;
}

/*
 * Set the ceiling texture.
 */

void
do_set_ceiling_texture (name)
	char *name;
{
	ceiling_texture_name = name;
}

/*
 * Set the debug level.
 */

void
do_set_debug_level (state)
	short state;
{
	debug_level = state;
}

/*
 * Initialize the engine.
 */

void
do_engine_init ()
{
	/*
	 * Initialize the scan converter.
	 */
	scan_init ();

	/*
	 * Make and initialize the maze. The third
	 *  value is the random seed value.
	 */ 
	maze = make_maze ();
	maze_init (maze, maze_size, 0);

	/*
	 * Print and ascii description of the maze.
	 */
	if (debug_level >= 1)
		maze_describe (maze);

	/*
	 * Make and initialize the world form the maze.
	 */
	world = make_world ();
	world_from_maze (world, maze, wall_texture_name, floor_texture_name, ceiling_texture_name);

	/*
	 * Make the user controlled player.
	 */
	player = make_object ();
	set_object_id (player, 1);
	set_object_radius (player, my_float_to_num (0.36));
	set_object_red (player, my_float_to_num (1));
	set_object_green (player, my_float_to_num (0));
	set_object_blue (player, my_float_to_num (0));
	set_object_move (player, (void *) do_move_my_object);

	set_object_pos_x (player, my_float_to_num (1.0));
	set_object_pos_y (player, my_float_to_num (1.0));
	set_object_pos_z (player, my_float_to_num (2.5));

	set_object_dir_x (player, my_float_to_num (0.0));
	set_object_dir_y (player, my_float_to_num (0.0));
	set_object_dir_z (player, my_float_to_num (1.0));

	object_update_camera (player);
	set_object_sector (player, maze_element_sector (maze, 2, 1));
	sector_objects (object_sector (player)) = objects_add_object (
	sector_objects (object_sector (player)), player);
	object_sectors (player) = sectors_add_sector (object_sectors (player), 
	object_sector (player));
	objects = objects_add_object (objects, player);

	/*
	 * Make the computer controlled player.
	 */
	enemy = make_object ();
	set_object_id (enemy, 1);
	set_object_radius (enemy, my_float_to_num (0.36));
	set_object_red (enemy, my_float_to_num (0));
	set_object_green (enemy, my_float_to_num (0));
	set_object_blue (enemy, my_float_to_num (1));

	set_object_data (enemy, player);
	set_object_move (enemy, (void *) computer_move);

	set_object_pos_x (enemy, my_float_to_num (1.0));
	set_object_pos_y (enemy, my_float_to_num (1.0));
	set_object_pos_z (enemy, my_float_to_num (1.0));

	set_object_dir_x (enemy, my_float_to_num (0.0));
	set_object_dir_y (enemy, my_float_to_num (0.0));
	set_object_dir_z (enemy, my_float_to_num (-1.0));

	object_update_camera (enemy);
	set_object_sector (enemy, maze_element_sector (maze, 1, 1));
	sector_objects (object_sector (enemy)) = objects_add_object (
	sector_objects (object_sector (enemy)), enemy);
	object_sectors (enemy) = sectors_add_sector (object_sectors (enemy), 
	object_sector (enemy));
	objects = objects_add_object (objects, enemy);

	/*
	 * Tie the view to the computer controlled player.
	 */
	view = enemy;

	/*
	 * Create the various timers and initialize the frame count.
	 */
	frame_timer = make_timer ();
	global_timer = make_timer ();
	frame_count = 0;
}

/*
 * Exit the engine.
 */

void
do_engine_exit ()
{
	/*
	 * Stop the global timer.
	 */
	timer_stop (global_timer);

	/*
	 * Print memory debugging information.
	 */
	if (debug_level >= 1)
	{
		printf ("Face memory usage: %d k\n", face_mem_usage () /1024);
		printf ("Sector memory usage: %d k\n", sector_mem_usage () /1024);
		printf ("Average fps: %0.4f\n", frame_count /timer_elapsed (global_timer));
	}

	/*
	 * Free stuff.
	 */
	free_timer (frame_timer);
	free_timer (global_timer);
	free_maze (maze);
	free_world (world);
	free_object (player);
	free_object (enemy);
}

/*
 * Display a single frame.
 * This involves moving the objects and then
 *  refreshing the display.
 */
 
void
do_engine_frame ()
{
	do_move_objects (objects);
	do_engine_refresh ();
}

/*
 * Refresh the display.
 */

void
do_engine_refresh ()
{
	/*
	 * Start the global timer if this is the first frame.
	 */
	if (frame_count == 0)
		timer_start (global_timer);
	
	/*
	 * Increment the frame count.
	 */
	frame_count++;

	if (debug_level >= 2)
		timer_start (frame_timer);

	/*
	 * Clear the frame buffer. This doesn't really need
	 *  to be done since we'll be drawing over every single
	 *  pixel.
	 */
/*
	do_set_color (1);
	do_clear_frame_buffer ();
*/

	/*
	 * Render the world from the current view. Then
	 *  update the screen.
	 */
	do_render_world (view);
	do_update_screen ();

	if (debug_level >= 2)
	{
		timer_stop (frame_timer);
		printf ("fps: %0.4f\n", 1.0 /timer_elapsed (frame_timer));
	}
}

/*
 * Handle key presses. We'll simply encode them so 
 *  that we can keep track of which keys are currently
 *  down.
 */

void
do_engine_key (key, down)
	long key;
	short down;
{
	if (down)
	{
		/*
		 * The key was pressed so "or" it in with the
		 *  rest of them.
		 */
		if (key == LEFT_KEY)
			keys |= LEFT;
		else if (key == RIGHT_KEY)
			keys |= RIGHT;
		else if (key == UP_KEY)
			keys |= UP;
		else if (key == DOWN_KEY)
			keys |= DOWN;
		else if (key == TAB_KEY)
		{
			/*
			 * Switch the view if the tab key was pressed.
			 */
			 view = (view == player) ? enemy : player;
		}
	}
	else
	{
		/*
		 * The key was released so "and-not" it with the
		 *  rest of them.
		 */
		if (key == LEFT_KEY)
			keys &= ~LEFT;
		else if (key == RIGHT_KEY)
			keys &= ~RIGHT;
		else if (key == UP_KEY)
			keys &= ~UP;
		else if (key == DOWN_KEY)
			keys &= ~DOWN;
	}
}

/*
 * Move the objects.
 */

void
do_move_objects (objs)
	OBJECTS objs;
{
	OBJECT obj;

	/*
	 * Simply call the corresponding move function
	 *  for every object. Then use "do_object_movement"
	 *  to actually move the object and do collision
	 *  detection.
	 */
	while (objs)
	{
		obj = objects_first (objs);
		objs = objects_rest (objs);

		if (object_move (obj))
			(*object_move (obj)) (obj);
		do_object_movement (obj);
	}
}

/*
 * User controlled movement.
 */
 
void
do_move_my_object (o)
	OBJECT o;
{
	/*
	 * Simply add to the appropriate velocities.
	 */
	 
	if (keys & LEFT)
		set_object_dir_vel (o, object_dir_vel (o) + TURN_STEP);
	if (keys & RIGHT)
		set_object_dir_vel (o, object_dir_vel (o) - TURN_STEP);
	if (keys & UP)
		set_object_pos_vel (o, object_pos_vel (o) + MOVE_STEP);
	if (keys & DOWN)
		set_object_pos_vel (o, object_pos_vel (o) - MOVE_STEP);
}

/*
 * Move an object based on its velocities.
 */

void
do_object_movement (o)
	OBJECT o;
{
	VECTOR temp_vector;
	MATRIX temp_matrix;

	/*
	 * Make sure the object isn't moving too fast.
	 */
	if (object_dir_vel (o) > TURN_MAX)
		set_object_dir_vel (o, TURN_MAX);
	if (object_dir_vel (o) < -TURN_MAX)
		set_object_dir_vel (o, -TURN_MAX);
	if (object_pos_vel (o) > MOVE_MAX)
		set_object_pos_vel (o, MOVE_MAX);
	if (object_pos_vel (o) < -MOVE_MAX)
		set_object_pos_vel (o, -MOVE_MAX);

	/*
	 * Perform object rotation.
	 */
	vector_copy (object_dir (o), temp_vector);
	matrix_rotate_y (temp_matrix, object_dir_vel (o));
	matrix_vector (temp_matrix, temp_vector, object_dir (o));

	/*
	 * Perform object movement.
	 */
	vector_copy (object_pos (o), temp_vector);
	vector_x (temp_vector) += my_mul (object_dir_x (o), object_pos_vel (o));
	vector_y (temp_vector) += my_mul (object_dir_y (o), object_pos_vel (o));
	vector_z (temp_vector) += my_mul (object_dir_z (o), object_pos_vel (o));
	vector_w (temp_vector) = 1;

	/*
	 * Reduce the velocities to simulate drag.
	 */
	set_object_dir_vel (o, my_div (object_dir_vel (o), object_dir_drag (o)));
	set_object_pos_vel (o, my_div (object_pos_vel (o), object_pos_drag (o)));

	/*
	 * If the object actually moved (rotation doesn't count),
	 *  then perform collision detection.
	 */
	if ((vector_x (temp_vector) != object_dir_x (o)) ||
		(vector_y (temp_vector) != object_dir_y (o)) ||
		(vector_z (temp_vector) != object_dir_z (o)))
	{
		collision (o, object_pos (o), temp_vector);
		vector_copy (temp_vector, object_pos (o));
	}

	/*
	 * Update the objects camera.
	 */
	object_update_camera (o);
}
