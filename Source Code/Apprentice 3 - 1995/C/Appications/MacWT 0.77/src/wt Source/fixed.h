/*
**  MacWT -- a 3d game engine for the Macintosh
**  © 1995, Bill Hayden and Nikol Software
**
**  On the Internet:
**  bmoc1@aol.com (my personal address)
**  nikolsw@grove.ufl.edu (my school address)
**	MacWT anonymous FTP site: ftp.circa.ufl.edu/pub/software/ufmug/mirrors/LocalSW/Hayden/
**  http://grove.ufl.edu:80/~nikolsw (my WWW page, containing MacWT info)
**
**  based on wt, by Chris Laurel (claurel@mr.net)
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/


#ifndef FIXED_H_
#define FIXED_H_

#include "limits.h"

/* fixed point conversions */
#define INT_TO_FIXED(i) ((i) << 16)
#define FIXED_TO_INT(f) ((f) >> 16)
#define FIXED_TO_FLOAT(f) (((double) (f)) * 1.52587890625e-5)
#define FLOAT_TO_FIXED(f) ((fixed) ((f) * 65536.0))

/* functions */
#define FIXED_ABS(f) ((f) < 0 ? -(f) : (f))
#define FIXED_TRUNC(f) ((f) & 0xffff0000)
#define FIXED_SIGN(f) ((unsigned int) (f) >> 31)
#define FIXED_PRODUCT_SIGN(f, g) ((unsigned int) ((f) ^ (g)) >> 31)
#define FIXED_HALF(f) ((f) / 2)
#define FIXED_DOUBLE(f) ((f) << 1)

/* perform integer scaling of a fixed point number */
#define FIXED_SCALE(f, i) ((f) * (i))


/* fixed point constants */
#define FIXED_ZERO     (INT_TO_FIXED(0))
#define FIXED_ONE      (INT_TO_FIXED(1))
#define FIXED_ONE_HALF (FIXED_HALF(FIXED_ONE))
#define FIXED_PI       (FLOAT_TO_FIXED(3.14159265))
#define FIXED_2PI      (FLOAT_TO_FIXED(6.28318531))
#define FIXED_HALF_PI  (FLOAT_TO_FIXED(1.57079633))
#define FIXED_MIN      LONG_MIN
#define FIXED_MAX      LONG_MAX

/* we need this for kludges to avoid fixed point division overflow */
#define FIXED_EPSILON  ((fixed) 0x100)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef long fixed;

extern fixed fix_sqrt(fixed);


#if !defined(__powerc)  && !__MC68881__

/******************** MBW - 68k Macintosh only... ****************/

/* toolbox trap versions - slow... */
/* extern pascal fixed fixmul(fixed a, fixed b) = 0xA868; */
/* extern pascal fixed fixmul2_30(fixed x, fixed y) = 0xA84A; */

/* inline versions */
#pragma parameter __D0 fixmul(__D0,__D1)
fixed fixmul(fixed a, fixed b) = 
{
        0x4C01,0x0C01   // muls.L D1,D1:D0
        ,0x3001                 // move.w D1,D0
        ,0x4840                 // swap D0
};

#pragma parameter __D1 fixmul2_30(__D0,__D1)
fixed fixmul2_30(fixed a, fixed b) = 
{
        0x4C01,0x0C01   // muls.L D1,D1:D0
        ,0xE589                 // lsl.L #2, D1
        /* without the following lines, we're dropping the 
           least signifigant 2 bits.  If this is a problem, 
           uncomment them. 
         */
         //,0xE598                       // rol.L #2, D0
         //,0xEFC1, 0x0782       // bfins D0, D1{30,2}
};


/* MBW - one of these days I'll rewrite division too. 
   The toolbox trap works, though.
*/
extern pascal fixed fixdiv(fixed x, fixed y) = 0xA84D;

#else

/****************** Generic fixed point functions ********************/

#define fixmul(a, b) ((fixed) ((double) (a) * ((double) (b)) * 1.52587890625e-5))
#define fixmul2_30(a, b) ((fixed) ((double) (a) * ((double) (b) * 9.313225746154785e-10)))
#define fixdiv(a, b) ((fixed) (((double) (a) / (double) (b)) * 65536.0))

#endif // !powerc && __FPU


#if !defined(__powerc)

#undef INT_TO_FIXED
#undef FIXED_TO_INT
#undef FIXED_TRUNC

#pragma parameter __D0 FIXED_TRUNC(__D0)
fixed FIXED_TRUNC(fixed x) = 
        0x4240;         // clr.W D0

#pragma parameter __D0 FIXED_TO_INT(__D0)
signed short FIXED_TO_INT(fixed x) = 
        0x4840;         // swap D0

#pragma parameter __D0 INT_TO_FIXED(__D0)
fixed INT_TO_FIXED(signed short x) = 
{
        0x4840          // swap D0
        ,0x4240         // clr.w D0
};

#endif	// !powerc


#endif /* _FIXED_H_ */
