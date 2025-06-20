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
#include "point.h"
#include "sys.stuff.h"

/*
 * Keep a list of free points for fast allocation.
 * Keep track of the memory used by points.
 */

static POINTS free_points_list = NULL;
static long point_mem = 0;

/*
 * Make a point and initialize its values.
 */

POINT
make_point ()
{
	POINTS points;
	POINT point;

	if (free_points_list)
	{
		points = free_points_list;
		point = points_first (points);
		free_points_list = points_rest (free_points_list);
		free_list (points);
	}
	else
	{
		point_mem += sizeof (_POINT);

		point = (POINT) ALLOC (sizeof (_POINT));
	}

	set_point_intensity (point, NUM_ZERO);

	set_point_x (point, NUM_ZERO);
	set_point_y (point, NUM_ZERO);
	set_point_z (point, NUM_ZERO);
	set_point_w (point, NUM_ONE);

	return point;
}

/*
 * Free a point by placing it on the free list.
 */

void
free_point (p)
	POINT p;
{
	LIST temp;

	temp = make_list ();
	set_list_datum (temp, p);
	set_list_next (temp, free_points_list);
	free_points_list = temp;
}

/*
 * Clone a point. That is, create a new point and
 *  copy all the values from the old point into the
 *  new point. (Used in clipping).
 */

POINT
point_clone (p)
	POINT p;
{
	POINT new_point = make_point ();

	vector_copy (point_coord (p), point_coord (new_point));
	set_point_intensity (new_point, point_intensity (p));

	return new_point;
}

/*
 * Return the last point in a list of points.
 * (Actually, the list containing the last point).
 */

POINTS
points_last (points)
	POINTS points;
{
	if (points)
	{
		while (points_rest (points))
			points = points_rest (points);

		return points;
	}

	return NULL;
}

/*
 * Append a point to a list of points.
 */

POINTS
points_append_point (set, p)
	POINTS set;
	POINT p;
{
	LIST n;

	n = make_list ();
	set_list_datum (n, p);

	return ((POINTS) list_append_list ((LIST) set, n));
}

/*
 * Prepend a point to a list of points.
 */

POINTS
points_prepend_point (set, p)
	POINTS set;
	POINT p;
{
	LIST n;

	n = make_list ();
	set_list_datum (n, p);

	return ((POINTS) list_prepend_list ((LIST) set, n));
}

/*
 * Free a list of points.
 */

void
free_points (set)
	POINTS set;
{
	if (set == NULL)
		return;

	free_points (points_rest (set));

	set_points_rest (set, free_points_list);
	free_points_list = set;
}

/*
 * Return the point memory usage.
 */

long
point_mem_usage ()
{
	return point_mem;
}
