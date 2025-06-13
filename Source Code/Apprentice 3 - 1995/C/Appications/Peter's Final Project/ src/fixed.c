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

#include "fixed.h"

/*
 * This is Ron Hunsinger's integer square root algorithm adapted for fixed
 * point numbers. This adaption is simple. Since a fixed point number can
 * be represented as p*q, where p is some constant value (65536) in this
 * case. Then the sqrt(q) represented as a fixed point number is p*sqrt(q).
 * The algorithm below will spit out sqrt(p*q), so all that is needed is to
 * multiply by sqrt(p). Since sqrt(p) is constant (256) and is a multiple of
 * 2, this operation can be done with a left shift of 8 bits.
 * 
 * -Peter Mattis
 */

FIX_NUM fix_sqrt(FIX_NUM n)
{
#define lsqrt_max4pow (1UL << 30)
	/* lsqrt_max4pow is the (machine-specific) largest power of 4 that can
	 * be represented in an unsigned long.
	 *
	 * Compute the integer square root of the integer argument n
	 * Method is to divide n by x computing the quotient x and remainder r
	 * Notice that the divisor x is changing as the quotient x changes
	 * 
	 * Instead of shifting the dividend/remainder left, we shift the
	 * quotient/divisor right. The binary point starts at the extreme
	 * left, and shifts two bits at a time to the extreme right.
	 * 
	 * The residue contains n-x^2. (Within these comments, the ^ operator
	 * signifies exponentiation rather than exclusive or. Also, the /
	 * operator returns fractions, rather than truncating, so 1/4 means
	 * one fourth, not zero.)
	 * 
	 * Since (x + 1/2)^2 == x^2 + x + 1/4,
	 * n - (x + 1/2)^2 == (n - x^2) - (x + 1/4)
	 * Thus, we can increase x by 1/2 if we decrease (n-x^2) by (x+1/4)
	 */
	 
	unsigned long residue;    /* n - x^2  */
	unsigned long root;       /* x + 1/4  */
	unsigned long half;       /* 1/2      */

	residue = n;              /* n - (x = 0)^2, with suitable alignment */

	/*
	 * if the correct answer fits in two bits, pull it out of a magic hat
	 */
	if (residue <= 12)
		return (0x03FFEA94 >> (residue *= 2)) & 3;

	root = lsqrt_max4pow; /* x + 1/4, shifted all the way left */
	/* half = root + root; 1/2, shifted likewise */

	/* 
	 * Unwind iterations corresponding to leading zero bits 
	 */
	while (root > residue) root >>= 2;

	/*
	 * Unwind the iteration corresponding to the first one bit
	 * Operations have been rearranged and combined for efficiency
	 * Initialization of half is folded into this iteration
	 */
	residue -= root;    /* Decrease (n-x^2) by (0+1/4)             */
	half = root >> 2;   /* 1/4, with binary point shifted right 2  */
	root += half;       /* x=1. (root is now (x=1)+1/4.)           */
	half += half;       /* 1/2, properly aligned                   */

	/*
	 * Normal loop (there is at least one iteration remaining)
	 */
	do {
		if (root <= residue)  /* Whenever we can,                          */
		{
			residue -= root;  /* decrease (n-x^2) by (x+1/4)               */
			root += half;     /* increase x by 1/2                         */
		}
		half >>= 2;          /* Shift binary point 2 places right          */
		root -= half;        /* x{ +1/2 } +1/4 - 1/8 == x { +1/2 } 1/8     */
		root >>= 1;          /* 2x{ +1 } +1/4, shifted right 2 places      */
	} while (half);          /* When 1/2 == 0, bin. point is at far right  */

	/* 
	 * round up if (x+1/2)^2 < n
	 */
	if (root < residue)
		++root;

	/*
	 * Multiply the result by 256. (A left shift by 8 will do
	 *  the job quite quickly).
	 *
	 * -Peter Mattis
	 */
	root <<= 8;

	/* 
	 * Guaranteed to be correctly rounded (or truncated)
	 */
		return root;
}
