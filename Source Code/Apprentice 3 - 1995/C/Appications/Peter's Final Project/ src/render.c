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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "clip.h"
#include "face.h"
#include "object.h"
#include "point.h"
#include "render.h"
#include "scan.h"
#include "sector.h"
#include "sys.stuff.h"

/*
 * Declare the functions private to this module.
 */

static void render_reset (CAMERA);
static void render_sectors (SECTOR);
static short render_sector (SECTOR);
static void render_object (OBJECT);
static void render_sector_face (FACE);
static void render_object_face (FACE, MATRIX, NUM, NUM, NUM);
static POINTS render_transform (POINTS, MATRIX, short);
static void render_shade (POINTS, VECTOR);

/*
 * We keep track of the frame number here (as well as in "engine.c").
 * For convience, store the current camera and its position at the
 *  top level.
 */
static long frame_num = 0;
static CAMERA cur_camera;
static NUM x_pos, y_pos, z_pos;

/*
 * Render a world from an objects view.
 */
 
void
do_render_world (o)
	OBJECT o;
{
	scan_reset ();
	render_reset (object_camera (o));
	render_sectors (object_sector (o));
	scan_finish ();
}

/*
 * Reset the renderer.
 */

static void
render_reset (c)
	CAMERA c;
{
	cur_camera = c;

	x_pos = camera_pos_x (c);
	y_pos = camera_pos_y (c);
	z_pos = camera_pos_z (c);

	frame_num++;
}

/*
 * Render the sectors. (Here's where it all happens).
 */

static void
render_sectors (s)
	SECTOR s;
{
	SECTORS sector_queue;
	SECTORS neighbors;
	SECTOR temp_sector;
	LIST temp_list;

	/*
	 * The method used is to render the current sector,
	 *  then render all its neighbors, then render all
	 *  the neighbors neighbors, etc. We make sure to
	 *  not render a sector twice by setting its val to
	 *  the frame number. If a sectors val is less than
	 *  the frame number, then it needs to be rendered.
	 * Basically, we traverse the graph of sectors starting
	 *  from the current sector in a breadth first search.
	 *  To this end, we'll use a queue. A sector will only
	 *  get placed on the queue if it is to be drawn. When
	 *  the sector is drawn, we place its neighbors on the
	 *  stack.
	 */
	
	/*
	 * Always draw the start sector. So lets place it on
	 *  the queue.
	 */
	set_sector_val (s, frame_num);
	sector_queue = sectors_add_sector ((SECTORS) NULL, s);

	/*
	 * While there are sectors in the queue...
	 */
	while (sector_queue)
	{
		/*
		 * Get the next sector on the queue and draw it.
		 * "render_sector" will return true if the screen
		 *  is filled. (Which means we should quit).
		 */
		s = sectors_first (sector_queue);
		if (render_sector (s))
			break;

		/*
		 * Take that first link on the sector queue and
		 *  delete it.
		 */
		temp_list = sector_queue;
		sector_queue = sectors_rest (sector_queue);

		free_list (temp_list);

		/*
		 * Add all of the sectors neighbors to the queue,
		 *  but only if they haven't been drawn already.
		 */
		neighbors = sector_neighbors (s);
		while (neighbors)
		{
			temp_sector = sectors_first (neighbors);
			neighbors = sectors_rest (neighbors);

			if (sector_val (temp_sector) < frame_num)
			{
				set_sector_val (temp_sector, frame_num);
				sector_queue = sectors_append_sector (sector_queue, temp_sector);
			}
		}
	}

	/*
	 * The sector queue might still have items in it if
	 *  rendering ended early. (Which, hopefully it did).
	 *  So we need to destroy the queue.
	 */
	while (sector_queue)
	{
		temp_list = sector_queue;
		sector_queue = sectors_rest (sector_queue);
		free_list (temp_list);
	}
}

/*
 * Render a single sector.
 */

static short
render_sector (s)
	SECTOR s;
{
	OBJECTS objects;
	OBJECT object;
	FACES faces;
	FACE face;

	/*
	 * First render the objects in this sector. I really
	 *  don't handle this correctly. I should depth sort
	 *  the objects and draw them from front to back.
	 */
	objects = sector_objects (s);
	while (objects)
	{
		object = objects_first (objects);
		objects = objects_rest (objects);

		render_object (object);
	}

	/*
	 * Next render the faces in this sector.
	 */
	faces = sector_faces (s);
	while (faces)
	{
		face = faces_first (faces);
		faces = faces_rest (faces);

		render_sector_face (face);
	}

	/*
	 * Return whether we are finished yet, or not.
	 */
	return scan_complete ();
}

/*
 * Render an object.
 */

static void
render_object (o)
	OBJECT o;
{
	FACES faces;
	FACE face;
	MATRIX m1, m2, m3, m4;
	VECTOR u, v, n;

	/*
	 * Only render the object if it hasn't been rendered
	 *  already. An object may be rendered multiple times
	 *  because it can overlap multiple sectors (getting
	 *  drawn by each sector). 
	 */
	if (object_val (o) < frame_num)
	{
		set_object_val (o, frame_num);

		/*
		 * Set up a matrix that rotates the object. Without
		 *  this, an object would not appear to rotate as
		 *  it turned.
		 */
		set_vector_x (n, object_dir_x (o));
		set_vector_y (n, object_dir_y (o));
		set_vector_z (n, object_dir_z (o));
		set_vector_w (n, NUM_ZERO);

		set_vector_x (v, NUM_ZERO);
		set_vector_y (v, -NUM_ONE);
		set_vector_z (v, NUM_ZERO);
		set_vector_w (v, NUM_ZERO);

		vector_cross (v, n, u);
		set_vector_w (u, NUM_ZERO);

		matrix_clear (m1);
		vector_copy (u, m1[0]);
		vector_copy (v, m1[1]);
		vector_copy (n, m1[2]);

		/*
		 * Set up a matrix to translate the object. Without
		 *  this, an object would not appear to move.
		 */
		matrix_translate (m2, object_pos_x (o), object_pos_y (o), object_pos_z (o));

		/*
		 * Compose the two matrices with the objects own matrix.
		 */
		matrix_compose (m1, object_matrix (o), m3);
		matrix_compose (m2, m3, m4);

		/*
		 * Then simply render every face in the object with
		 *  the composited matrix.
		 */
		faces = object_faces (o);
		while (faces)
		{
			face = faces_first (faces);
			faces = faces_rest (faces);

			render_object_face (face, m4, object_red (o), object_green (o), object_blue (o));
		}
	}
}

/*
 * Render a face of a sector. (They're textured).
 */

static void
render_sector_face (f)
	FACE f;
{
	POINTS t_pts;
	POINTS c_pts;
	POINTS p_pts;
	POINT p;
	VECTOR v1, v2, n;
	VECTOR to, tu, tv;
	NUM dot;

	/*
	 * Determine if this face is front facing or not.
	 *  (ie Should it be backface culled).
	 */
	p = points_first (face_points (f));
	vector_copy (point_coord (p), v1);
	vector_copy (point_coord (face_normal (f)), n);
	vector_sub (camera_pos (cur_camera), v1, v2);

	dot = vector_dot (v2, n);
	if (dot > 0)
	{
		/*
		 * Transform the face by the orientation matrix.
		 */
		t_pts = render_transform (face_points (f), camera_matrix_orientation (cur_camera), 0);
		
		/*
		 * Clip the face to the view volume.
		 */
		c_pts = clip_polygon (t_pts);

		/*
		 * If something remains after clipping...
		 */
		if (c_pts)
		{
			/*
			 * Transform what remains by the mapping matrix.
			 * We also set the "screenify" flag which tells "render_transform"
			 *  to homogenize the new points and to round there x and y values.
			 *  The homogenization accomplishes the perspective division. The
			 *  rounding takes care of truncation artefacts.
			 */
			p_pts = render_transform (c_pts, camera_matrix_mapping (cur_camera), 1);

			/*
			 * Transform the texture points by the orientation matrix.
			 */
			matrix_vector (camera_matrix_orientation (cur_camera), point_coord (face_texture_o (f)), to);
			matrix_vector (camera_matrix_orientation (cur_camera), point_coord (face_texture_u (f)), v1);
			matrix_vector (camera_matrix_orientation (cur_camera), point_coord (face_texture_v (f)), v2);
			
			/*
			 * Calculate the texture vectors from the transform texture points.
			 */
			vector_sub (v1, to, tu);
			vector_sub (v2, to, tv);

			/*
			 * Transform the normal by the orientation matrix.
			 */
			matrix_vector (camera_matrix_orientation (cur_camera), point_coord (face_normal (f)), n);

			/*
			 * Use the texture mapping scan conversion function.
			 */
			scan_texture (p_pts, n, to, tu, tv, face_texture (f));

			/*
			 * Free the clipped and project points.
			 * (The clipping function free'd the transformed points).
			 */
			free_points (c_pts);
			free_points (p_pts);
		}
	}
}

/*
 * Render an object of a sector. (They're Gourad shaded).
 */

static void
render_object_face (f, m, r, g, b)
	FACE f;
	MATRIX m;
	NUM r, g, b;
{
	POINTS t_pts;
	POINTS c_pts;
	POINTS p_pts;
	POINT p;
	VECTOR v1, v2, n;
	NUM dot;
	MATRIX mat;

	/*
	 * Determine if this face is front facing or not.
	 *  (ie Should it be backface culled).
	 * This is a little bit more complicated than with
	 *  sector faces since we must multiply the vectors
	 *  by the passed in matrix.
	 */
	p = points_first (face_points (f));
	matrix_vector (m, point_coord (p), v1);
	matrix_vector (m, point_coord (face_normal (f)), n);
	vector_sub (camera_pos (cur_camera), v1, v2);

	dot = vector_dot (v2, n);
	if (dot > 0)
	{
		/*
		 * Compose the new orientation matrix.
		 */
		matrix_compose (camera_matrix_orientation (cur_camera), m, mat);
		
		/*
		 * Transform the face by the new orientation matrix.
		 */
		t_pts = render_transform (face_points (f), mat, 0);

		/*
		 * At this point the face is shaded. Intensity values
		 *  are calculated for each point. (The transformed normal
		 *  to the face is needed for this calculation).
		 */
		matrix_vector (mat, point_coord (face_normal (f)), n);
		vector_normalize (n);
		render_shade (t_pts, n);

		/*
		 * Clip the face to the view volume.
		 */
		c_pts = clip_polygon (t_pts);

		/*
		 * If something remains after clipping...
		 */
		if (c_pts)
		{
			/*
			 * Transform what remains by the mapping matrix.
			 * We also set the "screenify" flag which tells "render_transform"
			 *  to homogenize the new points and to round there x and y values.
			 *  The homogenization accomplishes the perspective division. The
			 *  rounding takes care of truncation artefacts.
			 */
			p_pts = render_transform (c_pts, camera_matrix_mapping (cur_camera), 1);

			/*
			 * Use the shading scan conversion function.
			 */
			scan_shade (p_pts, r, g, b);

			/*
			 * Free the clipped and project points.
			 * (The clipping function free'd the transformed points).
			 */
			free_points (c_pts);
			free_points (p_pts);
		}
	}
}

/*
 * Transform a list of points by a matrix and return
 *  a new list of the transformed points.
 */
 
static POINTS
render_transform (pts, m, screenify)
	POINTS pts;
	MATRIX m;
	short screenify;
{
	POINTS new_pts = NULL;
	POINT pt, new_pt;

	/*
	 * Simply apply the matrix to every point in the list,
	 *  building up a new list as we go.
	 * Notice that the order of the points in the new list
	 *  may be reversed, but we don't care. (Actually, at 
	 *  present, the points are reversed).
	 */
	while (pts)
	{
		pt = points_first (pts);
		pts = points_rest (pts);

		new_pt = make_point ();
		new_pts = points_add_point (new_pts, new_pt);

		matrix_vector (m, point_coord (pt), point_coord (new_pt));
		set_point_intensity (new_pt, point_intensity (pt));

		/*
		 * Homogenize and round if desired.
		 */
		if (screenify)
		{
			vector_homogenize (point_coord (new_pt));
			set_point_x (new_pt, my_num_to_int (ROUND (point_x (new_pt))));
			set_point_y (new_pt, my_num_to_int (ROUND (point_y (new_pt))));
			set_point_z (new_pt, point_z (pt));
		}
	}

	return new_pts;
}

/*
 * Shade the list of points.
 */
 
static void
render_shade (pts, normal)
	POINTS pts;
	VECTOR normal;
{
	POINT pt;
	VECTOR light;
	NUM dot;
	NUM dist;
	NUM intensity;

	while (pts)
	{
		pt = points_first (pts);
		pts = points_rest (pts);

		/*
		 * The light vector is the vector from the point to
		 * the light source (the viewer...at the origin).
		 */
		set_vector_x (light, -point_x (pt));
		set_vector_y (light, -point_y (pt));
		set_vector_z (light, -point_z (pt));
		
		/*
		 * Normalize the light vector so that dot product
		 *  calculations relate to the angle between vectors.
		 */
		vector_normalize (light);

		/*
		 * Calculate the cosine of the angle between the
		 *  normal and the light vector.
		 * Also calculate the distance to the point.
		 */
		dot = vector_dot (normal, light);
		dist = vector_magnitude (point_coord (pt));
		if (dist < NUM_ONE)
			dist = NUM_ONE;

		/*
		 * The lighting of the point depends partially on ambient
		 *  light (0.3) and partially on diffuse reflect light (0.7).
		 */
		intensity = my_mul (my_float_to_num (0.7), dot) + my_float_to_num (0.3);
		
		/*
		 * Divide the intensity by the points distance and clamp
		 *  the intensity between 0 and 1.
		 */
		intensity = my_div (intensity, my_mul (dist, NUM_ONE));
		if (intensity > NUM_ONE)
			intensity = NUM_ONE;
		if (intensity < NUM_ZERO)
			intensity = NUM_ZERO;

		/*
		 * Set the points intensity.
		 */
		set_point_intensity (pt, intensity);
	}
}
