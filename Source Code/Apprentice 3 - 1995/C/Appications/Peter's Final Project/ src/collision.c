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

#include <signal.h>
#include <stdlib.h>

#include "collision.h"
#include "face.h"
#include "matrix.vector.h"
#include "object.h"
#include "sector.h"

/*
 * Declare the functions private to this module.
 */

static void collide_sector (SECTOR, NUM, VECTOR, VECTOR);
static void collide_face (FACE, NUM, VECTOR, VECTOR);
static short collide_check (FACE, VECTOR, NUM);
static short point_in_sector (SECTOR, VECTOR, NUM);

/*
 * Collide the object with its environment. 
 * "start" and "end" are the starting and ending positions 
 *  of its movement. If its movement is obstructed, the 
 *  value of "end" will be modified.
 * Currently, we only look for collisions with the sector the
 *  object is in and its neighboring sectors. This isn't a
 *  problem for us, since the object cannot move more than a
 *  sectors distance at one time. But it is forseeable in a
 *  different environment that we would want to check for collisions
 *  with more distant sectors. 
 * This function also makes sure that all the sectors that
 *  the object overlaps know that the object is in that sector.
 */

void
collision (object, start, end)
	OBJECT object;
	VECTOR start, end;
{
	SECTORS neighbors;
	SECTORS sectors;
	SECTOR cur_sector;
	LIST temp_list;

	/*
	 * First remove the object from all the sectors it
	 *  currently overlaps. These sectors are stored in
	 *  the "sectors" list.
	 */
	sectors = object_sectors (object);
	set_object_sectors (object, NULL);

	while (sectors)
	{
		cur_sector = sectors_first (sectors);

		temp_list = sectors;
		sectors = sectors_rest (sectors);
		free_list (temp_list);

		sector_objects (cur_sector) = objects_remove_object (sector_objects (cur_sector), object);
	}
	
	/*
	 * Next, collide the object with the current sector and
	 *  its neighbors.
	 */
	cur_sector = object_sector (object);
	collide_sector (cur_sector, object_radius (object), start, end);

	neighbors = sector_neighbors (cur_sector);
	while (neighbors)
	{
		cur_sector = sectors_first (neighbors);
		neighbors = sectors_rest (neighbors);

		collide_sector (cur_sector, object_radius (object), start, end);
	}

	/*
	 * Finally, determine which sectors the object overlaps.
	 * If an object overlaps a sector, check to see if that sector
	 *  contains the center of the object. (There is only one such sector).
	 */
	cur_sector = object_sector (object);
	if (point_in_sector (cur_sector, end, object_radius (object)))
	{
		if (point_in_sector (cur_sector, end, NUM_ZERO))
			set_object_sector (object, cur_sector);
		sector_objects (cur_sector) = objects_add_object (sector_objects (cur_sector), object);
		object_sectors (object) = sectors_add_sector (object_sectors (object), cur_sector);
	}

	neighbors = sector_neighbors (cur_sector);
	while (neighbors)
	{
		cur_sector = sectors_first (neighbors);
		neighbors = sectors_rest (neighbors);

		if (point_in_sector (cur_sector, end, object_radius (object)))
		{
			if (point_in_sector (cur_sector, end, NUM_ZERO))
				set_object_sector (object, cur_sector);
			sector_objects (cur_sector) = objects_add_object (sector_objects (cur_sector), object);
			object_sectors (object) = sectors_add_sector (object_sectors (object), cur_sector);
		}
	}
}

/*
 * Collide an object with the given radius with a sector.
 */

static void
collide_sector (sector, radius, start, end)
	SECTOR sector;
	NUM radius;
	VECTOR start, end;
{
	FACES faces;
	FACE face;

	/*
	 * All we have to do is call "collide_face" for every
	 *  face in the sector.
	 */
	faces = sector_faces (sector);
	while (faces)
	{
		face = faces_first (faces);
		faces = faces_rest (faces);

		collide_face (face, radius, start, end);
	}
}

/*
 * Collide an object with the given radius with a face.
 */

static void
collide_face (face, radius, start, end)
	FACE face;
	NUM radius;
	VECTOR start, end;
{
	POINT normal;
	POINT point;
	VECTOR new_normal;
	VECTOR new_point;
	VECTOR diff;
	NUM num, denom;
	NUM d, t;

	/*
	 * Only perform the collision if this is an obstructing face.
	 */
	if (face_obstructs (face))
	{
		/*
		 * The "start" and "end" points define a line segment.
		 * What we'll do is intersect the line and the plane.
		 */
		
		normal = face_normal (face);
		point = points_first (face_points (face));

		/*
		 * To handle the fact that an object has some radius,
		 *  I offset the point on the face a distance of the radius
		 *  out along the direction of the normal. This has the
		 *  effect of moving the plane defining the face out a
		 *  distance of the radius along the normal, which is exactly
		 *  what we want.
		 */
		vector_mul (point_coord (normal), radius, new_normal);
		vector_add (point_coord (point), new_normal, new_point);

		vector_sub (end, new_point, diff);
		if (vector_dot (point_coord (normal), diff) > NUM_ZERO)
			return;

		d = vector_dot (point_coord (normal), new_point);
		vector_sub (end, start, diff);

		num = -vector_dot (point_coord (normal), start) + d;

		/*
		 * If "denom" is equal to 0, then the line and plane
		 *  are parallel and no intersection can occur.
		 */
		denom = vector_dot (point_coord (normal), diff);
		if (denom == 0)
			return;

		/*
		 * "t" is the parameterized distance along the line
		 *  from the "start". If it lies between 0 and 1, then
		 *  then line segment intersects the plane.
		 */
		t = my_div (num, denom);
		if ((t < NUM_ZERO) || (t > NUM_ONE))
			return;

		/*
		 * Calculate the point of intersection.
		 */
		set_vector_x (new_point, vector_x (start) + my_mul (vector_x (diff), t));
		set_vector_y (new_point, vector_y (start) + my_mul (vector_y (diff), t));
		set_vector_z (new_point, vector_z (start) + my_mul (vector_z (diff), t));
		set_vector_w (new_point, NUM_ONE);

		/*
		 * If the line segment intersects the plane, then we
		 *  next want to know if the point if intersection is
		 *  within the face.
		 */
		if (collide_check (face, new_point, radius))
		{
			/*
			 * An collision occurred, so slide the object
			 *  along the face. This is a bit of hack, because
			 *  I make use of the fact that we're dealing with
			 *  a simple environment.
			 */
			vector_sub (end, new_point, diff);
			vector_copy (new_point, end);

			if (point_x (normal) == NUM_ONE)
				set_vector_z (end, vector_z (end) + vector_z (diff));
			else if (point_x (normal) == -NUM_ONE)
				set_vector_z (end, vector_z (end) + vector_z (diff));
			else if (point_z (normal) == NUM_ONE)
				set_vector_x (end, vector_x (end) + vector_x (diff));
			else if (point_z (normal) == -NUM_ONE)
				set_vector_x (end, vector_x (end) + vector_x (diff));
		}
	}
}

/*
 * Determine if the point "v" is within the face.
 * I'm not going to comment on this function except to
 *  say I don't handle the true 3d case. I assume that
 *  the face is 2d and aligned with the coordinate axes.
 *  This simplifies the problem, to checking to see if
 *  a value lies within some bounds. I have an idea of
 *  how to do the true 3d check, though its a little 
 *  more work.
 */

static short
collide_check (f, v, r)
	FACE f;
	VECTOR v;
	NUM r;
{
	POINTS pts;
	POINT pt;
	NUM min, max;

	min = 100000000;
	max = -min;
	pts = face_points (f);

	if (point_x (face_normal (f)) != 0)
	{
		while (pts)
		{
			pt = points_first (pts);
			pts = points_rest (pts);

			if ((point_z (pt) - r) < min)
				min = point_z (pt) - r;
			if ((point_z (pt) + r) > max)
				max = point_z (pt) + r;
		}

		return ((vector_z (v) >= min) && (vector_z (v) <= max));
	}
	else if (point_z (face_normal (f)) != 0)
	{
		while (pts)
		{
			pt = points_first (pts);
			pts = points_rest (pts);

			if ((point_x (pt) - r) < min)
				min = point_x (pt) - r;
			if ((point_x (pt) + r) > max)
				max = point_x (pt) + r;
		}

		return ((vector_x (v) >= min) && (vector_x (v) <= max));
	}
	else
	{
		return 0;
	}
}

/*
 * Determine if a point lies within sector, (with
 *  some overlap).
 */

static short
point_in_sector (s, v, r)
	SECTOR s;
	VECTOR v;
	NUM r;
{
	FACES faces;
	FACE face;
	POINT temp_point;
	VECTOR temp_vector;
	NUM dot;

	/*
	 * Simply check to see if the point lies on the
	 *  correct side of every face.
	 * The dest against "-r" allows the point to lie
	 *  a distance "r" outside a face and still be
	 *  considered to lie inside. This allows use to
	 *  check to whether an object overlaps a sector.
	 */
	faces = sector_faces (s);
	while (faces)
	{
		face = faces_first (faces);
		faces = faces_rest (faces);

		temp_point = points_first (face_points (face));
		vector_sub (v, point_coord (temp_point), temp_vector);

		dot = vector_dot (temp_vector, point_coord (face_normal (face)));
		if (dot < -r)
			return 0;
	}

	return 1;
}
