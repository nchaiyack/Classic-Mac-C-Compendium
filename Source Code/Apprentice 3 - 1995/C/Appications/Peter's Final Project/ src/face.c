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
#include "sys.stuff.h"

/*
 * Keep a list of free faces for fast allocation.
 * Keep track of the memory used by faces.
 */

static FACES free_faces_list = NULL;
static long face_mem = 0;

/*
 * Make a face object and initialize its values to something decent.
 */

FACE
make_face ()
{
	FACES faces;
	FACE face;

	if (free_faces_list)
	{
		faces = free_faces_list;
		face = faces_first (faces);
		free_faces_list = faces_rest (free_faces_list);
		free_list (faces);
	}
	else
	{
		face_mem += sizeof (_FACE);

		face = (FACE) ALLOC (sizeof (_FACE));
	}

	set_face_points (face, NULL);
	set_face_normal (face, NULL);
	set_face_texture_o (face, NULL);
	set_face_texture_u (face, NULL);
	set_face_texture_v (face, NULL);
	set_face_intensity (face, 0);
	set_face_texture (face, NULL);
	set_face_obstructs (face, FALSE);

	return face;
}

/*
 * Free a face by placing it on the free list.
 */

void
free_face (f)
	FACE f;
{
	LIST temp;

	temp = make_list ();
	set_list_datum (temp, f);
	set_list_next (temp, free_faces_list);
	free_faces_list = temp;
}

/*
 * Append a face to a list of faces.
 */
 
FACES
faces_append_face (set, f)
	FACES set;
	FACE f;
{
	LIST n;

	n = make_list ();
	set_list_datum (n, f);
	set_list_next (n, NULL);
	
	return (FACES) list_append_list ((LIST) set, n);
}

/*
 * Prepend a face to a list of faces.
 */

FACES
faces_prepend_face (set, f)
	FACES set;
	FACE f;
{
	LIST n;

	n = make_list ();
	set_list_datum (n, f);
	set_list_next (n, NULL);
	
	return (FACES) list_prepend_list ((LIST) set, n);
}

/*
 * Free a list of faces.
 */

void
free_faces (set)
	FACES set;
{
	if (set == NULL)
		return;

	free_faces (faces_rest (set));

	set_faces_rest (set, free_faces_list);
	free_faces_list = set;
}

/*
 * Return the face memory usage.
 */

long
face_mem_usage ()
{
	return face_mem;
}
