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

#ifndef __MY_MATH_H__
#define __MY_MATH_H__

/*
 * This header file is used to quickly convert from using
 *  fix point math to using floating point math. It contains
 *  defines for doing multiplies, divides, square roots,
 *  conversions, and other stuff that are representation
 *  dependent. Using the defines allows a quick conversion
 *  from using one type to using the other. This can
 *  be useful since on the 68k architecture using fix point
 *  will be faster, but on the PowerPC, using floating point
 *  will probably be faster. (I'm not sure about the PowerPC
 *  since I've never had the opportunity to test this, but
 *  I at least know the two versions are about the same speed
 *  on my IIsi w/ FPU).
 *
 * To use floating point math simply change the 1 in the '#if'
 *  below to a 0.
 */

#if 1

#include "fixed.h"

typedef FIX_NUM NUM;

#define my_mul(x, y)        fix_mul((x), (y))
#define my_div(x, y)        fix_div((x), (y))
#define my_sqrt(x)          fix_sqrt(x)
#define my_frac(x)          fix_frac(x)
#define my_floor(x)         fix_floor(x)
#define my_ceil(x)          fix_ceil(x)

#define my_num_to_float(x)  fix_to_float(x)
#define my_num_to_int(x)    fix_to_int(x)
#define my_float_to_num(x)  float_to_fix(x)
#define my_int_to_num(x)    int_to_fix(x)
#define my_num_to_fix(x)    (x)
#define my_fix_to_num(x)    (x)

#define NUM_ZERO            0
#define NUM_ONE             65536
#define NUM_ONE_HALF        32768
#define NUM_EPSILON         255

#else

#include <math.h>

typedef double NUM;

#define my_mul(x, y)        ((x) * (y))
#define my_div(x, y)        ((x) / (y))
#define my_sqrt(x)          ((NUM) sqrt((double) (x)))
#define my_frac(x)          (((x) - (NUM) floor((double) (x))))
#define my_floor(x)         ((NUM) floor((double) (x)))
#define my_ceil(x)          ((NUM) ceil((double) (x)))

#define my_num_to_float(x)  (x)
#define my_num_to_int(x)    (x)
#define my_float_to_num(x)  (x)
#define my_int_to_num(x)    (x)
#define my_num_to_fix(x)    float_to_fix(x)
#define my_fix_to_num(x)    fix_to_float(x)

#define NUM_ZERO            0.0
#define NUM_ONE             1.0
#define NUM_ONE_HALF        0.5
#define NUM_EPSILON         0.000000001

#endif

typedef short ANGLE;

void my_math_init (void);
void my_math_quit (void);
ANGLE my_normalize_angle (ANGLE);
NUM my_sin (ANGLE);
NUM my_cos (ANGLE);

#endif /* __MY_MATH_H__ */