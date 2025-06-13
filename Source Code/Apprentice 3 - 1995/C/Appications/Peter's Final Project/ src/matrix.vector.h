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

#ifndef __MATRIX_VECTOR_H__
#define __MATRIX_VECTOR_H__

#include "type.defs.h"

void matrix_describe (MATRIX);

void matrix_clear (MATRIX);
void matrix_copy (MATRIX, MATRIX);
void matrix_multiply (MATRIX, NUM);
void matrix_translate (MATRIX, NUM, NUM, NUM);
void matrix_scale (MATRIX, NUM, NUM, NUM);
void matrix_rotate_x (MATRIX, NUM);
void matrix_rotate_y (MATRIX, NUM);
void matrix_rotate_z (MATRIX, NUM);
void matrix_projection (MATRIX, NUM);
void matrix_compose (MATRIX, MATRIX, MATRIX);
void matrix_vector (MATRIX, VECTOR, VECTOR);

void matrix_transpose (MATRIX, MATRIX);
void matrix_inverse (MATRIX, MATRIX);
void matrix_adjoint (MATRIX, MATRIX);
NUM matrix_determinant4x4 (MATRIX);
NUM matrix_determinant3x3 (
			    NUM, NUM, NUM,
			    NUM, NUM, NUM,
			    NUM, NUM, NUM);
NUM matrix_determinant2x2 (
			    NUM, NUM,
			    NUM, NUM);

void vector_clear (VECTOR);
void vector_add (VECTOR, VECTOR, VECTOR);
void vector_sub (VECTOR, VECTOR, VECTOR);
void vector_mul (VECTOR, NUM, VECTOR);
void vector_copy (VECTOR, VECTOR);
NUM vector_dot (VECTOR, VECTOR);
void vector_cross (VECTOR, VECTOR, VECTOR);
NUM vector_magnitude (VECTOR);
void vector_normalize (VECTOR);
void vector_homogenize (VECTOR);

#define matrix_element(m, i, j)          ((m)[i][j])
#define set_matrix_element(m, i, j, k)   (matrix_element(m, i, j) = (k))

#define vector_x(v)            ((v)[0])
#define vector_y(v)            ((v)[1])
#define vector_z(v)            ((v)[2])
#define vector_w(v)            ((v)[3])

#define set_vector_x(v, k)     (vector_x(v) = (k))
#define set_vector_y(v, k)     (vector_y(v) = (k))
#define set_vector_z(v, k)     (vector_z(v) = (k))
#define set_vector_w(v, k)     (vector_w(v) = (k))

#endif /* __MATRIX_VECTOR_H__ */
