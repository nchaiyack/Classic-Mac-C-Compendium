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

#ifndef __FIXED_H__
#define __FIXED_H__

typedef long FIX_NUM;

#define fix_to_float(x)		((float) (x) / 65536.0)
#define fix_to_int(x)		((long) ((x) >> 16))
#define float_to_fix(x)		((FIX_NUM) ((x) * 65536))
#define int_to_fix(x)		((FIX_NUM) (((long) x) << 16))

#define fix_frac(x)         ((x) & 0x0000FFFF)
#define fix_floor(x)        ((x) & 0xFFFF0000)
#define fix_ceil(x)         (fix_frac(x) ? (fix_floor(x) + 0x00010000) : fix_floor(x))

#if 1

#pragma parameter __d2 fix_mul(__d0,__d2)
pascal FIX_NUM fix_mul(FIX_NUM a, FIX_NUM b) =
{
	0x4C00, 0x2C01,     /* MULS.L    D0,D1:D2 */
	0x3401,             /* MOVE.W    D1,D2    */
	0x4842              /* SWAP      D2       */
};

#pragma parameter __d0 fix_div(__d2,__d1)
pascal FIX_NUM fix_div(FIX_NUM a, FIX_NUM b) = 
{
	0x3002,             /* MOVE.W    D2,D0    */
	0x4840,             /* SWAP      D0       */
	0x4240,             /* CLR.W     D0       */
	0x4842,             /* SWAP      D2       */
	0x48C2,             /* EXT.L     D2       */
	0x4C41, 0x0C02      /* DIVS.L    D1,D2:D0 */
};

#else

/*
 * Define some macros for fixed point multiply and divide.
 * These should be portable across platforms, but assembly
 *  versions that do the same will be much faster.
 */

#define fix_mul(a, b)   ((long) ((double) (a) * (double) (b) / 65536.0))
#define fix_div(a, b)   ((long) ((double) (a) * 65536.0 / (double) (b)))

#endif

FIX_NUM fix_sqrt(FIX_NUM x);

#endif /* __FIXED_H__ */