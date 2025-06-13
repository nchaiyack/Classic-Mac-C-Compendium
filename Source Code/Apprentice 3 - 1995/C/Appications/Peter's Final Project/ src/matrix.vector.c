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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "matrix.vector.h"
#include "utils.h"

/*
 * Print a matrix.
 */

void
matrix_describe (m)
	MATRIX m;
{
	printf ("( ");
	printf ("%7.3f ", matrix_element (m, 0, 0));
	printf ("%7.3f ", matrix_element (m, 0, 1));
	printf ("%7.3f ", matrix_element (m, 0, 2));
	printf ("%7.3f ", matrix_element (m, 0, 3));
	printf ("\n");

	printf ("  ");
	printf ("%7.3f ", matrix_element (m, 1, 0));
	printf ("%7.3f ", matrix_element (m, 1, 1));
	printf ("%7.3f ", matrix_element (m, 1, 2));
	printf ("%7.3f ", matrix_element (m, 1, 3));
	printf ("\n");

	printf ("  ");
	printf ("%7.3f ", matrix_element (m, 2, 0));
	printf ("%7.3f ", matrix_element (m, 2, 1));
	printf ("%7.3f ", matrix_element (m, 2, 2));
	printf ("%7.3f ", matrix_element (m, 2, 3));
	printf ("\n");

	printf ("  ");
	printf ("%7.3f ", matrix_element (m, 3, 0));
	printf ("%7.3f ", matrix_element (m, 3, 1));
	printf ("%7.3f ", matrix_element (m, 3, 2));
	printf ("%7.3f ", matrix_element (m, 3, 3));
	printf (")\n");
}

/*
 * Clear a matrix to the identity matrix.
 */

void
matrix_clear (m)
	MATRIX m;
{
	register short i, j;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
			set_matrix_element (m, i, j, NUM_ZERO);
		set_matrix_element (m, i, i, NUM_ONE);
	}
}

/*
 * Copy a matrix.
 */

void
matrix_copy (src, dest)
	MATRIX src, dest;
{
	register short i, j;

	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			set_matrix_element (dest, i, j, matrix_element (src, i, j));
}

/*
 * Multiply a matrix by a scalar.
 */

void
matrix_multiply (m, k)
	MATRIX m;
	NUM k;
{
	register short i, j;

	if (k != 0)
	{
		for (i = 0; i < 4; i++)
			for (j = 0; j < 4; j++)
				set_matrix_element (m, i, j, my_mul (matrix_element (m, i, j), k));
	}
}

/*
 * Construct a translation matrix.
 */

void
matrix_translate (m, tx, ty, tz)
	MATRIX m;
	NUM tx, ty, tz;
{
	matrix_clear (m);
	set_matrix_element (m, 0, 3, tx);
	set_matrix_element (m, 1, 3, ty);
	set_matrix_element (m, 2, 3, tz);
}

/*
 * Construct a scale matrix.
 */

void
matrix_scale (m, sx, sy, sz)
	MATRIX m;
	NUM sx, sy, sz;
{
	matrix_clear (m);
	set_matrix_element (m, 0, 0, sx);
	set_matrix_element (m, 1, 1, sy);
	set_matrix_element (m, 2, 2, sz);
}

/*
 * Construct an x-axis rotation matrix.
 */

void
matrix_rotate_x (m, theta)
	MATRIX m;
	NUM theta;
{
	NUM sine, cosine;

	sine = my_float_to_num (sin (my_num_to_float (theta)));
	cosine = my_float_to_num (cos (my_num_to_float (theta)));

	matrix_clear (m);
	set_matrix_element (m, 1, 1, cosine);
	set_matrix_element (m, 1, 2, -sine);
	set_matrix_element (m, 2, 1, sine);
	set_matrix_element (m, 2, 2, cosine);
}

/*
 * Construct an y-axis rotation matrix.
 */

void
matrix_rotate_y (m, theta)
	MATRIX m;
	NUM theta;
{
	NUM sine, cosine;

	sine = my_float_to_num (sin (my_num_to_float (theta)));
	cosine = my_float_to_num (cos (my_num_to_float (theta)));

	matrix_clear (m);
	set_matrix_element (m, 0, 0, cosine);
	set_matrix_element (m, 0, 2, sine);
	set_matrix_element (m, 2, 0, -sine);
	set_matrix_element (m, 2, 2, cosine);
}

/*
 * Construct an z-axis rotation matrix.
 */

void
matrix_rotate_z (m, theta)
	MATRIX m;
	NUM theta;
{
	NUM sine, cosine;

	sine = my_float_to_num (sin (my_num_to_float (theta)));
	cosine = my_float_to_num (cos (my_num_to_float (theta)));

	matrix_clear (m);
	set_matrix_element (m, 0, 0, cosine);
	set_matrix_element (m, 0, 1, -sine);
	set_matrix_element (m, 1, 0, sine);
	set_matrix_element (m, 1, 1, cosine);
}

/*
 * Construct a projection matrix.
 */

void
matrix_projection (m, d)
	MATRIX m;
	NUM d;
{
	matrix_clear (m);
	set_matrix_element (m, 3, 2, my_div (NUM_ONE, d));
	set_matrix_element (m, 3, 3, 0);
}

/*
 * Compose two matrices. (Matrix multiplication).
 */ 

void
matrix_compose (a, b, ab)
	MATRIX a, b, ab;
{
	register short i, j;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			set_matrix_element (ab, i, j,
				(my_mul (matrix_element (a, i, 0), matrix_element (b, 0, j)) +
				 my_mul (matrix_element (a, i, 1), matrix_element (b, 1, j)) +
				 my_mul (matrix_element (a, i, 2), matrix_element (b, 2, j)) +
				 my_mul (matrix_element (a, i, 3), matrix_element (b, 3, j))));
		}
	}
}

/*
 * Multiply a matrix and a vector.
 */

void
matrix_vector (m, src, dest)
	MATRIX m;
	VECTOR src, dest;
{
	set_vector_x (dest,
		my_mul (matrix_element (m, 0, 0), vector_x (src)) +
		my_mul (matrix_element (m, 0, 1), vector_y (src)) +
		my_mul (matrix_element (m, 0, 2), vector_z (src)) +
		my_mul (matrix_element (m, 0, 3), vector_w (src)));

	set_vector_y (dest,
		my_mul (matrix_element (m, 1, 0), vector_x (src)) +
		my_mul (matrix_element (m, 1, 1), vector_y (src)) +
		my_mul (matrix_element (m, 1, 2), vector_z (src)) +
		my_mul (matrix_element (m, 1, 3), vector_w (src)));

	set_vector_z (dest,
		my_mul (matrix_element (m, 2, 0), vector_x (src)) +
		my_mul (matrix_element (m, 2, 1), vector_y (src)) +
		my_mul (matrix_element (m, 2, 2), vector_z (src)) +
		my_mul (matrix_element (m, 2, 3), vector_w (src)));

	set_vector_w (dest,
		my_mul (matrix_element (m, 3, 0), vector_x (src)) +
		my_mul (matrix_element (m, 3, 1), vector_y (src)) +
		my_mul (matrix_element (m, 3, 2), vector_z (src)) +
		my_mul (matrix_element (m, 3, 3), vector_w (src)));
}

/*
 * Transpose a matrix.
 */

void
matrix_transpose (src, dest)
	MATRIX src, dest;
{
	short i, j;

	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			set_matrix_element (dest, i, j, matrix_element (src, j, i));
}

/*
 * Invert a matrix.
 */

void
matrix_inverse (src, dest)
	MATRIX src, dest;
{
	short i, j;
	NUM det;

	matrix_adjoint (src, dest);
	det = matrix_determinant4x4 (src);

	if (fabs (det) < 1e-12)
		fatal_error ("Matrix is singular");

	matrix_multiply (dest, my_div (NUM_ONE, det));
}

/*
 * Construct the adjoint of a matrix (for use
 *  in determing the inverse of a matrix).
 */

void
matrix_adjoint (src, dest)
	MATRIX src, dest;
{
	MATRIX temp;
	NUM a1, a2, a3, a4;
	NUM b1, b2, b3, b4;
	NUM c1, c2, c3, c4;
	NUM d1, d2, d3, d4;

	a1 = src[0][0];
	a2 = src[0][1];
	a3 = src[0][2];
	a4 = src[0][3];

	b1 = src[1][0];
	b2 = src[1][1];
	b3 = src[1][2];
	b4 = src[1][3];

	c1 = src[2][0];
	c2 = src[2][1];
	c3 = src[2][2];
	c4 = src[2][3];

	d1 = src[3][0];
	d2 = src[3][1];
	d3 = src[3][2];
	d4 = src[3][3];

	temp[0][0] = matrix_determinant3x3 (b2, b3, b4, c2, c3, c4, d2, d3, d4);
	temp[1][0] = -matrix_determinant3x3 (a2, a3, a4, c2, c3, c4, d2, d3, d4);
	temp[2][0] = matrix_determinant3x3 (a2, a3, a4, b2, b3, b4, d2, d3, d4);
	temp[3][0] = -matrix_determinant3x3 (a2, a3, a4, b2, b3, b4, c2, c3, c4);

	temp[0][1] = -matrix_determinant3x3 (b1, b3, b4, c1, c3, c4, d1, d3, d4);
	temp[1][1] = matrix_determinant3x3 (a1, a3, a4, c1, c3, c4, d1, d3, d4);
	temp[2][1] = -matrix_determinant3x3 (a1, a3, a4, b1, b3, b4, d1, d3, d4);
	temp[3][1] = matrix_determinant3x3 (a1, a3, a4, b1, b3, b4, c1, c3, c4);

	temp[0][2] = matrix_determinant3x3 (b1, b2, b4, c1, c2, c4, d1, d2, d4);
	temp[1][2] = -matrix_determinant3x3 (a1, a2, a4, c1, c2, c4, d1, d2, d4);
	temp[2][2] = matrix_determinant3x3 (a1, a2, a4, b1, b2, b4, d1, d2, d4);
	temp[3][2] = -matrix_determinant3x3 (a1, a2, a4, b1, b2, b4, c1, c2, c4);

	temp[0][3] = -matrix_determinant3x3 (b1, b2, b3, c1, c2, c3, d1, d2, d3);
	temp[1][3] = matrix_determinant3x3 (a1, a2, a3, c1, c2, c3, d1, d2, d3);
	temp[2][3] = -matrix_determinant3x3 (a1, a2, a3, b1, b2, b3, d1, d2, d3);
	temp[3][3] = matrix_determinant3x3 (a1, a2, a3, b1, b2, b3, c1, c2, c3);

	matrix_transpose (temp, dest);
}

/*
 * Calculate the determinant of a matrix.
 */

NUM
matrix_determinant4x4 (m)
	MATRIX m;
{
	NUM ans;
	NUM a1, a2, a3, a4;
	NUM b1, b2, b3, b4;
	NUM c1, c2, c3, c4;
	NUM d1, d2, d3, d4;

	a1 = m[0][0];
	a2 = m[0][1];
	a3 = m[0][2];
	a4 = m[0][3];

	b1 = m[1][0];
	b2 = m[1][1];
	b3 = m[1][2];
	b4 = m[1][3];

	c1 = m[2][0];
	c2 = m[2][1];
	c3 = m[2][2];
	c4 = m[2][3];

	d1 = m[3][0];
	d2 = m[3][1];
	d3 = m[3][2];
	d4 = m[3][3];

	ans = (my_mul (a1, matrix_determinant3x3 (b2, b3, b4, c2, c3, c4, d2, d3, d4)) -
		   my_mul (b1, matrix_determinant3x3 (a2, a3, a4, c2, c3, c4, d2, d3, d4)) +
		   my_mul (c1, matrix_determinant3x3 (a2, a3, a4, b2, b3, b4, d2, d3, d4)) -
		   my_mul (d1, matrix_determinant3x3 (a2, a3, a4, b2, b3, b4, c2, c3, c4)));

	return ans;
}

/*
 * Calculate the determinant of a 3x3 matrix.
 */

NUM
matrix_determinant3x3 (a1, a2, a3, b1, b2, b3, c1, c2, c3)
	NUM a1, a2, a3;
	NUM b1, b2, b3;
	NUM c1, c2, c3;
{
	NUM ans;

	ans = (my_mul (a1, matrix_determinant2x2 (b2, b3, c2, c3)) -
		   my_mul (b1, matrix_determinant2x2 (a2, a3, c2, c3)) +
		   my_mul (c1, matrix_determinant2x2 (a2, a3, b2, b3)));

	return ans;
}

/*
 * Calculate the determinant of a 2x2 matrix.
 */

NUM
matrix_determinant2x2 (a, b, c, d)
	NUM a, b;
	NUM c, d;
{
	return (my_mul (a, d) - my_mul (b, c));
}

/*
 * Clear a vector. The w coordinate is 1 by default.
 */
 
void
vector_clear (v)
	VECTOR v;
{
	set_vector_x (v, NUM_ZERO);
	set_vector_y (v, NUM_ZERO);
	set_vector_z (v, NUM_ZERO);
	set_vector_w (v, NUM_ONE);
}

/*
 * Add two vectors.
 */

void
vector_add (a, b, d)
	VECTOR a, b, d;
{
	set_vector_x (d, vector_x (a) + vector_x (b));
	set_vector_y (d, vector_y (a) + vector_y (b));
	set_vector_z (d, vector_z (a) + vector_z (b));
	set_vector_w (d, NUM_ONE);
}

/*
 * Subtract two vectors.
 */

void
vector_sub (a, b, d)
	VECTOR a, b, d;
{
	set_vector_x (d, vector_x (a) - vector_x (b));
	set_vector_y (d, vector_y (a) - vector_y (b));
	set_vector_z (d, vector_z (a) - vector_z (b));
	set_vector_w (d, NUM_ONE);
}

/*
 * Multiply a vector by a scalar.
 */

void
vector_mul (src, k, dest)
	VECTOR src, dest;
	NUM k;
{
	set_vector_x (dest, my_mul (vector_x (src), k));
	set_vector_y (dest, my_mul (vector_y (src), k));
	set_vector_z (dest, my_mul (vector_z (src), k));
	set_vector_w (dest, NUM_ONE);
}

/*
 * Copy a vector.
 */

void
vector_copy (src, dest)
	VECTOR src, dest;
{
	set_vector_x (dest, vector_x (src));
	set_vector_y (dest, vector_y (src));
	set_vector_z (dest, vector_z (src));
	set_vector_w (dest, vector_w (src));
}

/*
 * Calculate the dot product of two vectors.
 */

NUM
vector_dot (a, b)
	VECTOR a, b;
{
	return (my_mul (vector_x (a), vector_x (b)) +
			my_mul (vector_y (a), vector_y (b)) +
			my_mul (vector_z (a), vector_z (b)));
}

/*
 * Calculate the cross product of two vectors.
 */

void
vector_cross (a, b, d)
	VECTOR a, b, d;
{
	set_vector_x (d, 
		my_mul (vector_y (a), vector_z (b)) - 
		my_mul (vector_z (a), vector_y (b)));
	set_vector_y (d, 
		my_mul (vector_z (a), vector_x (b)) - 
		my_mul (vector_x (a), vector_z (b)));
	set_vector_z (d, 
		my_mul (vector_x (a), vector_y (b)) - 
		my_mul (vector_y (a), vector_x (b)));
	set_vector_w (d, NUM_ONE);
}

/*
 * Calculate the magnitude (or length) of a vector).
 */
 
NUM
vector_magnitude (v)
	VECTOR v;
{
	return my_sqrt (vector_dot (v, v));
}

/*
 * Normalize a vector. (That is, make its length 1).
 */

void
vector_normalize (v)
	VECTOR v;
{
	NUM norm;

	norm = vector_magnitude (v);

	if (norm != 0)
	{
		set_vector_x (v, my_div (vector_x (v), norm));
		set_vector_y (v, my_div (vector_y (v), norm));
		set_vector_z (v, my_div (vector_z (v), norm));
		set_vector_w (v, NUM_ONE);
	}
	else
	{
		fatal_error ("Vector normalization of a 0 length vector");
	}
}

/*
 * Homogenize a vector. (That is, divide each coordinate
 *  by the w coordinate).
 */

void
vector_homogenize (v)
	VECTOR v;
{
	if (vector_w (v) != 0)
	{
		set_vector_x (v, my_div (vector_x (v), vector_w (v)));
		set_vector_y (v, my_div (vector_y (v), vector_w (v)));
		set_vector_z (v, my_div (vector_z (v), vector_w (v)));
		set_vector_w (v, NUM_ONE);
	}
	else
	{
		fatal_error ("Vector homogenization of a directional vector");
	}
}
