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
#include <math.h>
#include "my_math.h"
#include "sys.stuff.h"

/*
 * Define the trig table size.
 */
#define TRIG_TABLE_SIZE	360

/*
 * Declare the sine and cosine tables.
 */
static NUM *sin_table;
static NUM *cos_table;

/*
 * Declare the pi constant.
 */
static const double pi = 3.1459265359;

/*
 * Initialize the trig tables used for fast trig calculations.
 *  Not the most memory efficient versions, but easy to program
 *  and simple to understand.
 */
void 
my_math_init ()
{
	register short i;
	double angle;
	double increment;

	sin_table = (NUM*) ALLOC (sizeof(NUM) * TRIG_TABLE_SIZE);
	cos_table = (NUM*) ALLOC (sizeof(NUM) * TRIG_TABLE_SIZE);

	angle = 0.0;
	increment = 2.0 * pi /TRIG_TABLE_SIZE;
	for (i = 0; i < TRIG_TABLE_SIZE; i++)
	{
		sin_table[i] = my_float_to_num (sin (angle));
		cos_table[i] = my_float_to_num (cos (angle));

		angle += increment;
	}
}

/*
 * Delete the trig tables.
 */
void 
my_math_quit ()
{
	FREE (sin_table);
	FREE (cos_table);
}

/*
 * Normalize "theta" to be between 0 and
 *  TRIG_TABLE_SIZE.
 */

ANGLE 
my_normalize_angle (theta)
	ANGLE theta;
{
	while (theta < 0)
		theta += TRIG_TABLE_SIZE;
	while (theta >= TRIG_TABLE_SIZE)
		theta -= TRIG_TABLE_SIZE;

	return theta;
}

/*
 * Return the sin of "theta".
 */

NUM 
my_sin (theta)
	ANGLE theta;
{
	return sin_table[my_normalize_angle (theta)];
}

/*
 * Return the cos of "theta".
 */

NUM 
my_cos (theta)
	ANGLE theta;
{
	return cos_table[my_normalize_angle (theta)];
}
