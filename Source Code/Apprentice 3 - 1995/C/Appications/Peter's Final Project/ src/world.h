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

#ifndef __WORLD_H__
#define __WORLD_H__

#include "list.h"
#include "type.defs.h"

WORLD make_world (void);
void free_world (WORLD);

void world_from_maze (WORLD, MAZE, char *, char *, char *);

#define world_points(w)       ((w)->points)
#define world_faces(w)        ((w)->faces)

#define world_point(w, i)     (*(world_points(w) + i))
#define world_face(w, i)      ((FACE) list_nth((LIST) world_faces(w), i))

#define set_world_points(w, k)    (world_points(w) = (k))
#define set_world_faces(w, k)     (world_faces(w) = (k))

#endif /* __WORLD_H__ */
