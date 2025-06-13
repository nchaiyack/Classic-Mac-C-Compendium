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

#include <assert.h>
#include "camera.h"
#include "sys.stuff.h"

/*
 * Make a camera object and initialize its values to something decent.
 */

CAMERA
make_camera ()
{
	CAMERA camera;

	camera = (CAMERA) ALLOC (sizeof (_CAMERA));

	set_camera_pos_x (camera, NUM_ZERO);
	set_camera_pos_y (camera, NUM_ZERO);
	set_camera_pos_z (camera, NUM_ZERO);
	set_camera_pos_z (camera, NUM_ONE);

	set_camera_vpn_x (camera, NUM_ZERO);
	set_camera_vpn_y (camera, NUM_ZERO);
	set_camera_vpn_z (camera, NUM_ZERO);
	set_camera_vpn_z (camera, NUM_ONE);

	set_camera_vup_x (camera, NUM_ZERO);
	set_camera_vup_y (camera, NUM_ZERO);
	set_camera_vup_z (camera, NUM_ZERO);
	set_camera_vup_z (camera, NUM_ONE);

	set_camera_u_min (camera, NUM_ZERO);
	set_camera_u_max (camera, NUM_ZERO);
	set_camera_v_min (camera, NUM_ZERO);
	set_camera_v_max (camera, NUM_ZERO);

	return camera;
}

/*
 * Free a camera object.
 */

void
free_camera (c)
	CAMERA c;
{
	FREE (c);
}

/*
 * Determine a camera's matrices from its "pos", "vpn" and "vup".
 * (This is the meat and potatoes of the camera.)
 */

void
camera_determine_matrices (c)
	CAMERA c;
{
	MATRIX trans_pos;
	MATRIX rotate_uvn;
	MATRIX scale;
	MATRIX projection;
	MATRIX map_trans;
	MATRIX map_scale;
	MATRIX matrix_temp;
	VECTOR pos, vpn, vup;
	VECTOR u, v;

	/*
	 * Copy the "pos", "vpn" and "vup".
	 */
	vector_copy (camera_pos (c), pos);
	vector_copy (camera_vpn (c), vpn);
	vector_copy (camera_vup (c), vup);

	/*
	 * Normalize the "vpn" and "vup" so we have unit vectors.
	 */
	vector_normalize (vpn);
	vector_normalize (vup);

	/*
	 * First we need to translate to the origin.
	 */
	matrix_translate (trans_pos, -vector_x (pos), -vector_y (pos), -vector_z (pos));

	/*
	 * Then we need to determine the "u", "v", "n" coordinate system
	 *  of the camera. The "vpn" is the "n" of the coordinate system.
	 *  Basically, this is the direction we are looking. The "u" is a
	 *  vector pointing to our right. The "v" is a vector pointing up.
	 *
	 * To find "u" and "v" we do two cross products.
	 */
	vector_cross (vup, vpn, u);
	vector_cross (vpn, u, v);

	/*
	 * Since these are directional vectors, set the "w" coordinate to 0.
	 */
	set_vector_w (vpn, NUM_ZERO);
	set_vector_w (u, NUM_ZERO);
	set_vector_w (v, NUM_ZERO);

	/*
	 * If "u", "v" and "n" define a coordinate system, then a simple
	 *  way to create a matrix that transforms a vector to that
	 *  coordinate system is to create the matrix that contain "u",
	 *  "v" and "n" as the first 3 rows of that matrix (in the proper
	 *  order, of course).
	 */
	matrix_clear (rotate_uvn);
	vector_copy (u, rotate_uvn[0]);
	vector_copy (v, rotate_uvn[1]);
	vector_copy (vpn, rotate_uvn[2]);

	/*
	 * Next we scale the view volume is the canonical view volume.
	 * Note: This works, but messes up the psuedo-Gourad shading.
	 *       So for now we are stuck with a 90 degrees viewing
	 *       angle.
	 */
	matrix_scale (scale, NUM_ONE, NUM_ONE, NUM_ONE);

	/*
	 * Compose the matrices to form the "orientation" matrix.
	 * Remember, the order of composition is important.
	 */
	matrix_compose (rotate_uvn, trans_pos, matrix_temp);
	matrix_compose (scale, matrix_temp, camera_matrix_orientation (c));

	/*
	 * Create the "projection" matrix.
	 * We know the projection plane distance is 1 at this point,
	 *  so we can easily set up the matrix.
	 */
	matrix_clear (projection);
	set_matrix_element (projection, 3, 2, -NUM_ONE);
	set_matrix_element (projection, 3, 3, 0);

	/*
	 * Translate the view so that all coordinates lie between
	 *  0 and 2 in both x and y.
	 */
	matrix_translate (map_trans, NUM_ONE, NUM_ONE, 0);

	/*
	 * Scale the view so that x values range between 0 and "u_max"
	 *  and y values range between 0 and "v_max".
	 * The divide by two is present because previously values ranged
	 *  between 0 and 2.
	 * We want to keep the z value so don't scale in that direction.
	 */
	matrix_scale (map_scale,
		(camera_u_max (c) - camera_u_min (c)) / 2,
		(camera_v_max (c) - camera_v_min (c)) / 2,
		NUM_ONE);

	/*
	 * Compose the matrices to form the "mapping" matrix.
	 */
	matrix_compose (map_trans, projection, matrix_temp);
	matrix_compose (map_scale, matrix_temp, camera_matrix_mapping (c));
}
