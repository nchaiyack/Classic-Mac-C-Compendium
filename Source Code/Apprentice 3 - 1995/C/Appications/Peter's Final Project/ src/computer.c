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
#include <math.h>

#include "computer.h"
#include "matrix.vector.h"
#include "object.h"
#include "point.h"
#include "sys.stuff.h"

/*
 * Define some movement and distance constants for
 *  the computer to use in moving. (I know, some of
 *  these are duplicated in "engine.c". Bad Peter...bad.)
 */

static const NUM TURN_STEP = NUM_ONE * 0.07;
static const NUM MOVE_STEP = NUM_ONE * 0.10;
static const NUM MOVE_FAST = NUM_ONE * 0.15;

static const NUM TURN_HALF_STEP = NUM_ONE * 0.035;
static const NUM TURN_DOUBLE_STEP = NUM_ONE * 0.14;
static const NUM MOVE_HALF_STEP = NUM_ONE * 0.05;

static const NUM DISTANCE_FAR = NUM_ONE * 3.0;
static const NUM DISTANCE_NORM = NUM_ONE * 1.5;
static const NUM DISTANCE_NEAR = NUM_ONE * 1.0;

/*
 * Let the computer decide how to move. The current
 *  movement function basically follows a certain object.
 */
 
void 
computer_move (o)
	OBJECT o;
{
	OBJECT you;
	VECTOR v;
	VECTOR cross;
	NUM turn_angle;
	NUM distance;

	/*
	 * This is the object we follow.
	 */
	you = object_data (o);

	/*
	 * First, get the vector describing the distance between
	 *  me and you.
	 */
	vector_sub (object_pos (you), object_pos (o), v);
	distance = vector_magnitude (v);
	
	/*
	 * Normalize the vector so we can use it in cross product
	 *  calculations correctly.
	 */
	vector_normalize (v);
	vector_cross (object_dir (o), v, cross);

	/*
	 * The dot product between "v" and my direction is the cosine
	 *  of the angle between those two vectors. Take the acos to
	 *  find the angle to turn so I'd be facing you.
	 * Note: This angle is always positive.
	 */
	turn_angle = my_float_to_num (acos (my_num_to_float (vector_dot (object_dir (o), v))));
	if (turn_angle > TURN_DOUBLE_STEP)
	{
		/*
		 * I'm not even close to looking at you, so let's use
		 *  the cross product I calculated earlier to determine
		 *  which direction to turn. Remember, the cross product
		 *  is proportional to the sine of the angle between to
		 *  vectors.
		 */
		if (vector_y (cross) > 0)
			set_object_dir_vel (o, object_dir_vel (o) + TURN_STEP);
		else
			set_object_dir_vel (o, object_dir_vel (o) - TURN_STEP);

		/*
		 * Let's move slowly, since I'm not necessarily looking
		 *  at you.
		 */
		if (distance > DISTANCE_NORM)
			set_object_pos_vel (o, object_pos_vel (o) + MOVE_HALF_STEP);
		else if (distance < DISTANCE_NEAR)
			set_object_pos_vel (o, object_pos_vel (o) - MOVE_HALF_STEP);
	}
	else if (turn_angle > TURN_STEP)
	{
		/*
		 * I'm almost looking at you, so I'll turn more slowly now.
		 */
		if (vector_y (cross) > 0)
			set_object_dir_vel (o, object_dir_vel (o) + TURN_HALF_STEP);
		else
			set_object_dir_vel (o, object_dir_vel (o) - TURN_HALF_STEP);

		/*
		 * Let's move a little faster now if you are far away.
		 */
		if (distance > DISTANCE_FAR)
			set_object_pos_vel (o, object_pos_vel (o) + MOVE_STEP);
		else if (distance > DISTANCE_NORM)
			set_object_pos_vel (o, object_pos_vel (o) + MOVE_HALF_STEP);
		else if (distance < DISTANCE_NEAR)
			set_object_pos_vel (o, object_pos_vel (o) - MOVE_HALF_STEP);
	}
	else
	{
		/*
		 * Well, I'm basically looking at you, so we'll move
		 *  as fast as possible if you are far and at normal
		 *  speed to keep up if you are closer. (Hey, don't
		 *  get to close).
		 */
		if (distance > DISTANCE_FAR)
			set_object_pos_vel (o, object_pos_vel (o) + MOVE_FAST);
		else if (distance > DISTANCE_NORM)
			set_object_pos_vel (o, object_pos_vel (o) + MOVE_STEP);
		else if (distance < DISTANCE_NEAR)
			set_object_pos_vel (o, object_pos_vel (o) - MOVE_STEP);
	}
}
