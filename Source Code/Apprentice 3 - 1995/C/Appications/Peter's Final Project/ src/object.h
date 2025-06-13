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

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "camera.h"
#include "list.h"
#include "matrix.vector.h"
#include "type.defs.h"

OBJECT make_object (void);
void free_object (OBJECT);

void object_init (OBJECT);
void object_update_camera (OBJECT);

OBJECTS objects_append_object (OBJECTS, OBJECT);
OBJECTS objects_prepend_object (OBJECTS, OBJECT);
OBJECTS objects_remove_object (OBJECTS, OBJECT);
void free_objects (OBJECTS);

long object_mem_usage (void);

#define object_id(o)        ((o)->id)
#define object_val(o)       ((o)->val)
#define object_faces(o)     ((o)->faces)
#define object_matrix(o)    ((o)->matrix)
#define object_red(o)       ((o)->red)
#define object_green(o)     ((o)->green)
#define object_blue(o)      ((o)->blue)
#define object_radius(o)    ((o)->radius)
#define object_pos(o)       ((o)->pos)
#define object_dir(o)       ((o)->dir)
#define object_pos_vel(o)   ((o)->pos_vel)
#define object_dir_vel(o)   ((o)->dir_vel)
#define object_pos_drag(o)  ((o)->pos_drag)
#define object_dir_drag(o)  ((o)->dir_drag)
#define object_bounce(o)    ((o)->bounce)
#define object_camera(o)    ((o)->camera)
#define object_sector(o)    ((o)->sector)
#define object_sectors(o)   ((o)->sectors)
#define object_move(o)      ((o)->move_func)
#define object_data(o)      ((o)->data)

#define object_pos_x(o)   (vector_x(object_pos(o)))
#define object_pos_y(o)   (vector_y(object_pos(o)))
#define object_pos_z(o)   (vector_z(object_pos(o)))
#define object_pos_w(o)   (vector_w(object_pos(o)))

#define object_dir_x(o)   (vector_x(object_dir(o)))
#define object_dir_y(o)   (vector_y(object_dir(o)))
#define object_dir_z(o)   (vector_z(object_dir(o)))
#define object_dir_w(o)   (vector_w(object_dir(o)))

#define set_object_id(o, k)       (object_id(o) = (k))
#define set_object_val(o, k)      (object_val(o) = (k))
#define set_object_faces(o, k)    (object_faces(o) = (k))
#define set_object_red(o, k)      (object_red(o) = (k))
#define set_object_green(o, k)    (object_green(o) = (k))
#define set_object_blue(o, k)     (object_blue(o) = (k))
#define set_object_pos_vel(o, k)  (object_pos_vel(o) = (k))
#define set_object_dir_vel(o, k)  (object_dir_vel(o) = (k))
#define set_object_pos_drag(o, k) (object_pos_drag(o) = (k))
#define set_object_dir_drag(o, k) (object_dir_drag(o) = (k))
#define set_object_bounce(o, k)   (object_bounce(o) = (k))
#define set_object_matrix(o, k)   (object_matrix(o) = (k))
#define set_object_radius(o, k)   (object_radius(o) = (k))
#define set_object_camera(o, k)   (object_camera(o) = (k))
#define set_object_sector(o, k)   (object_sector(o) = (k))
#define set_object_sectors(o, k)  (object_sectors(o) = (k))
#define set_object_move(o, k)     (object_move(o) = (k))
#define set_object_data(o, k)     (object_data(o) = (k))

#define set_object_pos_x(o, k)    (object_pos_x(o) = (k))
#define set_object_pos_y(o, k)    (object_pos_y(o) = (k))
#define set_object_pos_z(o, k)    (object_pos_z(o) = (k))
#define set_object_pos_w(o, k)    (object_pos_w(o) = (k))

#define set_object_dir_x(o, k)    (object_dir_x(o) = (k))
#define set_object_dir_y(o, k)    (object_dir_y(o) = (k))
#define set_object_dir_z(o, k)    (object_dir_z(o) = (k))
#define set_object_dir_w(o, k)    (object_dir_w(o) = (k))

#define objects_first(o)     ((OBJECT) list_datum((LIST) o))
#define objects_rest(o)      ((OBJECTS) list_next((LIST) o))

#define set_objects_first(o, k)  (set_list_datum((LIST) o, k))
#define set_objects_rest(o, k)   (set_list_next((LIST) o, k))

#define objects_add_object  objects_prepend_object

#endif /* __OBJECT_H__ */