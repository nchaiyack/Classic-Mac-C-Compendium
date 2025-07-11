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
#include "face.h"
#include "matrix.vector.h"
#include "object.h"
#include "point.h"
#include "sys.stuff.h"
#include "texture.h"

/*
 * Keep a list of free objects for fast allocation.
 * Keep track of the memory used by objects.
 */

static OBJECTS free_objects_list = NULL;
static long object_mem = 0;

/*
 * Make an object and initialize its values.
 */

OBJECT
make_object ()
{
	OBJECTS objects;
	OBJECT object;

	if (free_objects_list)
	{
		objects = free_objects_list;
		object = objects_first (objects);
		free_objects_list = objects_rest (free_objects_list);
		free_list (objects);
	}
	else
	{
		object_mem += sizeof (_OBJECT);

		object = (OBJECT) ALLOC (sizeof (_OBJECT));
	}

	set_object_id (object, 0);
	set_object_val (object, 0);

	set_object_radius (object, 0);

	set_object_pos_x (object, 0);
	set_object_pos_y (object, 0);
	set_object_pos_z (object, 0);
	set_object_pos_w (object, NUM_ONE);

	set_object_dir_x (object, 0);
	set_object_dir_y (object, 0);
	set_object_dir_z (object, 0);
	set_object_dir_w (object, 0);

	set_object_pos_vel (object, 0);
	set_object_dir_vel (object, 0);
	set_object_pos_drag (object, my_float_to_num (1.9));
	set_object_dir_drag (object, my_float_to_num (1.9));

	set_object_camera (object, make_camera ());
	set_object_move (object, NULL);
	set_object_data (object, NULL);

	set_object_faces (object, NULL);
	object_init (object);

	set_object_red (object, NUM_ZERO);
	set_object_green (object, NUM_ZERO);
	set_object_blue (object, NUM_ONE);

	set_object_sector (object, NULL);
	set_object_sectors (object, NULL);

	return object;
}

/*
 * Free an object by placing it on the free list.
 */

void
free_object (o)
	OBJECT o;
{
	LIST temp;

	temp = make_list ();
	set_list_datum (temp, o);
	set_list_next (temp, free_objects_list);
	free_objects_list = temp;

	free_camera (object_camera (o));
}

/*
 * Initialize an objects faces and matrix.
 * Currently, objects are cubes.
 */

void
object_init (o)
	OBJECT o;
{
	POINT pts;
	POINT norms;
	FACE face;
	short i;

	pts = (POINT) ALLOC (sizeof (_POINT) * 8);
	assert (pts != NULL);

	norms = (POINT) ALLOC (sizeof (_POINT) * 6);
	assert (norms != NULL);

	set_point_x (&pts[0], NUM_ONE);
	set_point_y (&pts[0], NUM_ONE);
	set_point_z (&pts[0], NUM_ONE);
	set_point_w (&pts[0], NUM_ONE);

	set_point_x (&pts[1], NUM_ONE);
	set_point_y (&pts[1], -NUM_ONE);
	set_point_z (&pts[1], NUM_ONE);
	set_point_w (&pts[1], NUM_ONE);

	set_point_x (&pts[2], -NUM_ONE);
	set_point_y (&pts[2], -NUM_ONE);
	set_point_z (&pts[2], NUM_ONE);
	set_point_w (&pts[2], NUM_ONE);

	set_point_x (&pts[3], -NUM_ONE);
	set_point_y (&pts[3], NUM_ONE);
	set_point_z (&pts[3], NUM_ONE);
	set_point_w (&pts[3], NUM_ONE);

	set_point_x (&pts[4], NUM_ONE);
	set_point_y (&pts[4], NUM_ONE);
	set_point_z (&pts[4], -NUM_ONE);
	set_point_w (&pts[4], NUM_ONE);

	set_point_x (&pts[5], NUM_ONE);
	set_point_y (&pts[5], -NUM_ONE);
	set_point_z (&pts[5], -NUM_ONE);
	set_point_w (&pts[5], NUM_ONE);

	set_point_x (&pts[6], -NUM_ONE);
	set_point_y (&pts[6], -NUM_ONE);
	set_point_z (&pts[6], -NUM_ONE);
	set_point_w (&pts[6], NUM_ONE);

	set_point_x (&pts[7], -NUM_ONE);
	set_point_y (&pts[7], NUM_ONE);
	set_point_z (&pts[7], -NUM_ONE);
	set_point_w (&pts[7], NUM_ONE);

	set_point_x (&norms[0], NUM_ZERO);
	set_point_y (&norms[0], NUM_ZERO);
	set_point_z (&norms[0], NUM_ONE);
	set_point_w (&norms[0], NUM_ZERO);

	set_point_x (&norms[1], NUM_ZERO);
	set_point_y (&norms[1], NUM_ZERO);
	set_point_z (&norms[1], -NUM_ONE);
	set_point_w (&norms[1], NUM_ZERO);

	set_point_x (&norms[2], NUM_ONE);
	set_point_y (&norms[2], NUM_ZERO);
	set_point_z (&norms[2], NUM_ZERO);
	set_point_w (&norms[2], NUM_ZERO);

	set_point_x (&norms[3], -NUM_ONE);
	set_point_y (&norms[3], NUM_ZERO);
	set_point_z (&norms[3], NUM_ZERO);
	set_point_w (&norms[3], NUM_ZERO);

	set_point_x (&norms[4], NUM_ZERO);
	set_point_y (&norms[4], NUM_ONE);
	set_point_z (&norms[4], NUM_ZERO);
	set_point_w (&norms[4], NUM_ZERO);

	set_point_x (&norms[5], NUM_ZERO);
	set_point_y (&norms[5], -NUM_ONE);
	set_point_z (&norms[5], NUM_ZERO);
	set_point_w (&norms[5], NUM_ZERO);

	face = make_face ();
	set_face_normal (face, &norms[0]);
	face_points (face) = points_add_point (face_points (face), &pts[0]);
	face_points (face) = points_add_point (face_points (face), &pts[1]);
	face_points (face) = points_add_point (face_points (face), &pts[2]);
	face_points (face) = points_add_point (face_points (face), &pts[3]);
	object_faces (o) = faces_add_face (object_faces (o), face);

	face = make_face ();
	set_face_normal (face, &norms[1]);
	face_points (face) = points_add_point (face_points (face), &pts[4]);
	face_points (face) = points_add_point (face_points (face), &pts[5]);
	face_points (face) = points_add_point (face_points (face), &pts[6]);
	face_points (face) = points_add_point (face_points (face), &pts[7]);
	object_faces (o) = faces_add_face (object_faces (o), face);

	face = make_face ();
	set_face_normal (face, &norms[2]);
	face_points (face) = points_add_point (face_points (face), &pts[0]);
	face_points (face) = points_add_point (face_points (face), &pts[1]);
	face_points (face) = points_add_point (face_points (face), &pts[5]);
	face_points (face) = points_add_point (face_points (face), &pts[4]);
	object_faces (o) = faces_add_face (object_faces (o), face);

	face = make_face ();
	set_face_normal (face, &norms[3]);
	face_points (face) = points_add_point (face_points (face), &pts[7]);
	face_points (face) = points_add_point (face_points (face), &pts[6]);
	face_points (face) = points_add_point (face_points (face), &pts[2]);
	face_points (face) = points_add_point (face_points (face), &pts[3]);
	object_faces (o) = faces_add_face (object_faces (o), face);

	face = make_face ();
	set_face_normal (face, &norms[4]);
	face_points (face) = points_add_point (face_points (face), &pts[7]);
	face_points (face) = points_add_point (face_points (face), &pts[3]);
	face_points (face) = points_add_point (face_points (face), &pts[0]);
	face_points (face) = points_add_point (face_points (face), &pts[4]);
	object_faces (o) = faces_add_face (object_faces (o), face);

	face = make_face ();
	set_face_normal (face, &norms[5]);
	face_points (face) = points_add_point (face_points (face), &pts[6]);
	face_points (face) = points_add_point (face_points (face), &pts[2]);
	face_points (face) = points_add_point (face_points (face), &pts[1]);
	face_points (face) = points_add_point (face_points (face), &pts[5]);
	object_faces (o) = faces_add_face (object_faces (o), face);

	{
		MATRIX m1, m2, m3, m4;

		matrix_scale (m1, my_float_to_num (0.25), 
		my_float_to_num (0.25), 
		my_float_to_num (0.25));
		matrix_rotate_x (m2, my_float_to_num (0.7854));
		matrix_rotate_y (m3, my_float_to_num (0.7854));

		matrix_compose (m2, m3, m4);
		matrix_compose (m1, m4, object_matrix (o));
	}
}

/*
 * Have the object update its camera.
 */

void
object_update_camera (o)
	OBJECT o;
{
	if (o && object_camera (o))
	{
		set_camera_pos_x (object_camera (o), object_pos_x (o));
		set_camera_pos_y (object_camera (o), object_pos_y (o));
		set_camera_pos_z (object_camera (o), object_pos_z (o));
		set_camera_pos_w (object_camera (o), NUM_ONE);

		set_camera_vpn_x (object_camera (o), -object_dir_x (o));
		set_camera_vpn_y (object_camera (o), -object_dir_y (o));
		set_camera_vpn_z (object_camera (o), -object_dir_z (o));
		set_camera_vpn_w (object_camera (o), NUM_ZERO);

		set_camera_vup_x (object_camera (o), NUM_ZERO);
		set_camera_vup_y (object_camera (o), NUM_ONE);
		set_camera_vup_z (object_camera (o), NUM_ZERO);
		set_camera_vup_w (object_camera (o), NUM_ZERO);

		set_camera_u_min (object_camera (o), NUM_ZERO);
		set_camera_v_min (object_camera (o), NUM_ZERO);
		set_camera_u_max (object_camera (o), my_int_to_num (get_frame_buffer_width ()));
		set_camera_v_max (object_camera (o), my_int_to_num (get_frame_buffer_height ()));

		camera_determine_matrices (object_camera (o));
	}
}

/*
 * Append an object to a list of objects.
 */

OBJECTS
objects_append_object (set, o)
	OBJECTS set;
	OBJECT o;
{
	LIST n;

	n = make_list ();
	set_list_datum (n, o);

	return ((OBJECTS) list_append_list ((LIST) set, n));
}

/*
 * Prepend an object to a list of objects.
 */

OBJECTS
objects_prepend_object (set, o)
	OBJECTS set;
	OBJECT o;
{
	LIST n;

	n = make_list ();
	set_list_datum (n, o);

	return ((OBJECTS) list_prepend_list ((LIST) set, n));
}

/*
 * Remove an object from a list of objects.
 */

OBJECTS
objects_remove_object (set, o)
	OBJECTS set;
	OBJECT o;
{
	return ((OBJECTS) list_remove_list ((LIST) set, (void *) o));
}

/*
 * Free a list of objects.
 */

void
free_objects (set)
	OBJECTS set;
{
	if (set == NULL)
		return;

	free_objects (objects_rest (set));

	set_objects_rest (set, free_objects_list);
	free_objects_list = set;
}

/*
 * Return the object memory usage.
 */

long
object_mem_usage ()
{
	return object_mem;
}
