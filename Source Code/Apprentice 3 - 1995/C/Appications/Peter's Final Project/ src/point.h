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

#ifndef __POINT_H__
#define __POINT_H__

#include "list.h"
#include "type.defs.h"
#include "matrix.vector.h"

POINT make_point (void);
void free_point (POINT);
POINT point_clone (POINT);

POINTS points_last (POINTS);
POINTS points_append_point (POINTS, POINT);
POINTS points_prepend_point (POINTS, POINT);
void free_points (POINTS);

long point_mem_usage (void);

#define point_coord(p)      ((p)->coord)
#define point_intensity(p)  ((p)->intensity)

#define point_x(p)      (vector_x(point_coord(p)))
#define point_y(p)      (vector_y(point_coord(p)))
#define point_z(p)      (vector_z(point_coord(p)))
#define point_w(p)      (vector_w(point_coord(p)))

#define set_point_intensity(p, k)  (point_intensity(p) = (k))

#define set_point_x(p, k)   (point_x(p) = (k))
#define set_point_y(p, k)   (point_y(p) = (k))
#define set_point_z(p, k)   (point_z(p) = (k))
#define set_point_w(p, k)   (point_w(p) = (k))

#define points_first(p)   ((POINT) list_datum((LIST) p))
#define points_rest(p)    ((POINTS) list_next((LIST) p))
#define points_prev(p)    ((POINTS) list_prev((LIST) p))

#define set_points_first(p, k)  (set_list_datum((LIST) p, k))
#define set_points_rest(p, k)   (set_list_next((LIST) p, k))
#define set_points_prev(p, k)   (set_list_prev((LIST) p, k))

#define points_add_point  points_prepend_point

#endif /* __POINT_H__ */
