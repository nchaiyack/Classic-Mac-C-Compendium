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

#include "clip.h"
#include "matrix.vector.h"
#include "sys.stuff.h"

/*
 * Define some cases for clipping
 */

#define TRIVIAL_ACCEPT 1
#define TRIVIAL_REJECT 2
#define NON_TRIVIAL    3

/*
 * Define some function types. We need a comparison
 *  function and an intersection function.
 */
 
typedef short (*CLIP_COMPARE) (POINT);
typedef short (*CLIP_INTERSECT) (POINT, POINT, POINT *);

/*
 * Declare the functions private to this module.
 */
 
static short clip_calculate_case (POINTS);
static POINTS clip_plane (POINTS, CLIP_COMPARE, CLIP_INTERSECT);

static short clip_compare_all (POINT);
static short clip_compare_left (POINT);
static short clip_compare_right (POINT);
static short clip_compare_top (POINT);
static short clip_compare_bottom (POINT);
static short clip_compare_front (POINT);

static short clip_left (POINT, POINT, POINT *);
static short clip_right (POINT, POINT, POINT *);
static short clip_top (POINT, POINT, POINT *);
static short clip_bottom (POINT, POINT, POINT *);
static short clip_front (POINT, POINT, POINT *);

/*
 * We need to know what the front clipping plane
 *  distance is. Make sure the value is valid even
 *  if it is never set by the program.
 */
static NUM z_min = NUM_ONE * 0.1;

/*
 * Sets the 'z_min' clip value.
 */
 
void
clip_set_z_min (new_z_min)
	NUM new_z_min;
{
	z_min = new_z_min;
}

/*
 * Clips the polygon defined by 'points' to the clip rectangle
 *  defined by 'x_min', 'x_max', 'y_min', and 'y_max'.
 * It returns the clipped polygon.
 * Note: This function may possibly destroy the points passed
 *       into it. So be sure not to pass in any points that you
 *       don't want destroyed. (i.e. Make a copy first).
 * 
 * I do some hanky, panky #define magic in this function. Read
 *  up in K&R to see what the '##' does in defines. It's really
 *  quite neat.
 */
 
POINTS
clip_polygon (points)
	POINTS points;
{
#define CLIP(plane)   clip_compare_##plane, clip_##plane

	switch (clip_calculate_case (points))
	{
	case TRIVIAL_ACCEPT:
		return points;
		break;
	case TRIVIAL_REJECT:
		return NULL;
		break;
	}

	/*
	 * Clip to the left, right, bottom, top, front and back planes using a
	 *  generic 'clip_plane' routine.
	 * Only continue to clip if the polygon has not degenerated.
	 */

	if ((points = clip_plane (points, CLIP (front))) != NULL)
		if ((points = clip_plane (points, CLIP (left))) != NULL)
			if ((points = clip_plane (points, CLIP (right))) != NULL)
				if ((points = clip_plane (points, CLIP (bottom))) != NULL)
					points = clip_plane (points, CLIP (top));

	return points;
}

/*
 * Determine if the polygon to be clipped is a trivial case.
 */
 
static short
clip_calculate_case (points)
	POINTS points;
{
	POINT point;

	if (points)
	{
		while (points)
		{
			point = points_first (points);
			points = points_rest (points);

			if (!clip_compare_all (point))
				return NON_TRIVIAL;
		}

		return TRIVIAL_ACCEPT;
	}

	return TRIVIAL_REJECT;
}

/*
 * Clips the polygon defined by 'points' to a given edge.
 * Note: This is basically the Sutherland-Hodgeman clipping
 *       algorithm given in the book. The 'inside' and 'intersect'
 *       functions are present so that the same routine can be
 *       used to clip against each of the four clip edges.
 */
 
static POINTS
clip_plane (points, inside, intersect)
	POINTS points;
	CLIP_COMPARE inside;
	CLIP_INTERSECT intersect;
{
/*
 * Use some defines to simplify calling the compare function
 *  and to make output of polygons look nicer. (Makes the code
 *  more readable.
 */
#define OUTPUT(set, p)         (set) = points_add_point((set), (p))
#define INSIDE(p)              ((*inside) (p))
#define INTERSECT(p1, p2, p3)  ((*intersect) (p1, p2, p3))
#define CLONE_POINT(p)         (point_clone(p))

	POINTS out_points;
	POINTS local_points;
	POINT current_point;
	POINT next_point;
	POINT new_point;

	out_points = NULL;
	local_points = points;
	current_point = points_first (points_last (local_points));

	/*
	 * While there are still points in the polygon...
	 */
	while (local_points)
	{
		/*
		 * Set 'next_point' to the first of the remaining points to clip.
		 */
		next_point = points_first (local_points);

		/*
		 * If the point is inside (the current clip boundary)...
		 */
		if (INSIDE (current_point))
		{
			/*
			 * ...then output 'current_point'...
			 */

			OUTPUT (out_points, CLONE_POINT (current_point));

			/*
			 * ...and if 'next_point' is outside the current clip boundary
			 *  then clip the line to the clip boundary and output the new
			 *  point.
			 */
			if (!INSIDE (next_point))
			{
				INTERSECT (current_point, next_point, &new_point);
				OUTPUT (out_points, new_point);
			}
		}
		else if (INSIDE (next_point))
		{
			/*
			 * ...else if 'next_point' is inside the clip boundary,
			 *  then clip the line to the clip boundary and output the
			 *  new point.
			 */
			INTERSECT (current_point, next_point, &new_point);
			OUTPUT (out_points, new_point);
		}

		/*
		 * Increment to the next point.
		 */
		local_points = points_rest (local_points);
		current_point = next_point;
	}

	/*
	 * Free up the points passed in.
	 */
	free_points (points);

	/*
	 * Return the clipped points.
	 */
	return out_points;
}

/*
 * Compare the point to all clip planes.
 */
 
static short
clip_compare_all (p)
	POINT p;
{
	return (clip_compare_left (p) & clip_compare_right (p) &
		clip_compare_top (p) & clip_compare_bottom (p) &
		clip_compare_front (p));
}

/*
 * Compare the point to the left clip plane.
 */

static short
clip_compare_left (p)
	POINT p;
{
	return (point_x (p) >= point_z (p));
}

/*
 * Compare the point to the right clip plane.
 */

static short
clip_compare_right (p)
	POINT p;
{
	return (point_x (p) <= -point_z (p));
}

/*
 * Compare the point to the top clip plane.
 */

static short
clip_compare_top (p)
	POINT p;
{
	return (point_y (p) <= -point_z (p));
}

/*
 * Compare the point to the bottom clip plane.
 */

static short
clip_compare_bottom (p)
	POINT p;
{
	return (point_y (p) >= point_z (p));
}

/*
 * Compare the point to the front clip plane.
 */

static short
clip_compare_front (p)
	POINT p;
{
	return (point_z (p) <= z_min);
}

/*
 * Clip the segment to the left clip plane.
 */

static short
clip_left (start, end, p)
	POINT start, end, *p;
{
	NUM t, dx, dy, dz, di;
	short val;

	dx = point_x (end) - point_x (start);
	dy = point_y (end) - point_y (start);
	dz = point_z (end) - point_z (start);
	di = point_intensity (end) - point_intensity (start);

	t = my_div (-point_x (start) + point_z (start), dx - dz);
	if ((t == NUM_ZERO) || (t == NUM_ONE))
		val = 0;
	else
		val = 1;

	*p = make_point ();
	set_point_x (*p, point_x (start) + my_mul (dx, t));
	set_point_y (*p, point_y (start) + my_mul (dy, t));
	set_point_z (*p, point_z (start) + my_mul (dz, t));
	set_point_w (*p, NUM_ONE);

	set_point_intensity (*p, point_intensity (start) + my_mul (di, t));

	return val;
}

/*
 * Clip the segment to the right clip plane.
 */

static short
clip_right (start, end, p)
	POINT start, end, *p;
{
	NUM t, dx, dy, dz, di;
	short val;

	dx = point_x (end) - point_x (start);
	dy = point_y (end) - point_y (start);
	dz = point_z (end) - point_z (start);
	di = point_intensity (end) - point_intensity (start);

	t = my_div (point_x (start) + point_z (start), -dx - dz);
	if ((t == NUM_ZERO) || (t == NUM_ONE))
		val = 0;
	else
		val = 1;

	*p = make_point ();
	set_point_x (*p, point_x (start) + my_mul (dx, t));
	set_point_y (*p, point_y (start) + my_mul (dy, t));
	set_point_z (*p, point_z (start) + my_mul (dz, t));
	set_point_w (*p, NUM_ONE);

	set_point_intensity (*p, point_intensity (start) + my_mul (di, t));

	return val;
}

/*
 * Clip the segment to the top clip plane.
 */

static short
clip_top (start, end, p)
	POINT start, end, *p;
{
	NUM t, dx, dy, dz, di;
	short val;

	dx = point_x (end) - point_x (start);
	dy = point_y (end) - point_y (start);
	dz = point_z (end) - point_z (start);
	di = point_intensity (end) - point_intensity (start);

	t = my_div (point_y (start) + point_z (start), -dy - dz);
	if ((t == NUM_ZERO) || (t == NUM_ONE))
		val = 0;
	else
		val = 1;

	*p = make_point ();
	set_point_x (*p, point_x (start) + my_mul (dx, t));
	set_point_y (*p, point_y (start) + my_mul (dy, t));
	set_point_z (*p, point_z (start) + my_mul (dz, t));
	set_point_w (*p, NUM_ONE);

	set_point_intensity (*p, point_intensity (start) + my_mul (di, t));

	return val;
}

/*
 * Clip the segment to the bottom clip plane.
 */

static short
clip_bottom (start, end, p)
	POINT start, end, *p;
{
	NUM t, dx, dy, dz, di;
	short val;

	dx = point_x (end) - point_x (start);
	dy = point_y (end) - point_y (start);
	dz = point_z (end) - point_z (start);
	di = point_intensity (end) - point_intensity (start);

	t = my_div (-point_y (start) + point_z (start), dy - dz);
	if ((t == NUM_ZERO) || (t == NUM_ONE))
		val = 0;
	else
		val = 1;

	*p = make_point ();
	set_point_x (*p, point_x (start) + my_mul (dx, t));
	set_point_y (*p, point_y (start) + my_mul (dy, t));
	set_point_z (*p, point_z (start) + my_mul (dz, t));
	set_point_w (*p, NUM_ONE);

	set_point_intensity (*p, point_intensity (start) + my_mul (di, t));

	return val;
}

/*
 * Clip the segment to the front clip plane.
 */

static short
clip_front (start, end, p)
	POINT start, end, *p;
{
	NUM t, dx, dy, dz, di;
	short val;

	dx = point_x (end) - point_x (start);
	dy = point_y (end) - point_y (start);
	dz = point_z (end) - point_z (start);
	di = point_intensity (end) - point_intensity (start);

	t = my_div (point_z (start) - z_min, -dz);
	if ((t == NUM_ZERO) || (t == NUM_ONE))
		val = 0;
	else
		val = 1;

	*p = make_point ();
	set_point_x (*p, point_x (start) + my_mul (dx, t));
	set_point_y (*p, point_y (start) + my_mul (dy, t));
	set_point_z (*p, point_z (start) + my_mul (dz, t));
	set_point_w (*p, NUM_ONE);

	set_point_intensity (*p, point_intensity (start) + my_mul (di, t));

	return val;
}
