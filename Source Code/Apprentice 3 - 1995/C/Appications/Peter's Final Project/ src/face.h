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

#ifndef __FACE_H__
#define __FACE_H__

#include "list.h"
#include "point.h"
#include "texture.h"
#include "type.defs.h"

FACE make_face (void);
void free_face (FACE);

FACES faces_append_face (FACES, FACE);
FACES faces_prepend_face (FACES, FACE);
void free_faces (FACES);

long face_mem_usage (void);

#define face_points(f)      ((f)->points)
#define face_normal(f)      ((f)->normal)
#define face_texture_o(f)   ((f)->texture_o)
#define face_texture_u(f)   ((f)->texture_u)
#define face_texture_v(f)   ((f)->texture_v)
#define face_intensity(f)   ((f)->intensity)
#define face_texture(f)     ((f)->texture)
#define face_obstructs(f)   ((f)->obstructs)

#define set_face_points(f, k)     (face_points(f) = (k))
#define set_face_normal(f, k)     (face_normal(f) = (k))
#define set_face_texture_o(f, k)  (face_texture_o(f) = (k))
#define set_face_texture_u(f, k)  (face_texture_u(f) = (k))
#define set_face_texture_v(f, k)  (face_texture_v(f) = (k))
#define set_face_intensity(f, k)  (face_intensity(f) = (k))
#define set_face_texture(f, k)    (face_texture(f) = (k))
#define set_face_obstructs(f, k)  (face_obstructs(f) = (k))

#define faces_first(f)      ((FACE) list_datum((LIST) f))
#define faces_rest(f)       ((FACES) list_next((LIST) f))

#define set_faces_first(f, k)   (set_list_datum((LIST) f, k))
#define set_faces_rest(f, k)    (set_list_next((LIST) f, k))

#define faces_add_face  faces_prepend_face

#endif /* __FACE_H__ */
