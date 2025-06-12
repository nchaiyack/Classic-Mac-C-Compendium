/* "stdwconf.h" -- mess with predefined symbols.
   This file is supposed to be portable between all OSes.
   It is also reentrant.
*/
   
/* MPW defines "macintosh"; THINK C defines "THINK_C".
   Some portable code tests for "macintosh" to detect any Mac implementation;
   the distinction between MPW and THINK C is made by checking for
   "MPW" or "THINK_C".
*/

/* If "macintosh" defined but not "THINK_C", it must be MPW */
/* XXX (I hope I remember this right -- I haven't used MPW for years) */
#ifdef macintosh
#ifndef THINK_C
#define MPW
#endif
#endif

/* Under THINK C, turn on "macintosh" if not already on */
#ifdef THINK_C
#ifndef macintosh
#define macintosh
#endif
#if THINK_C < 5
#define THINK_C_PRE_5_0
#endif
#endif

/* For Think C 3.0, you must manually define THINK_C and THINK_C_3_0 */

#ifdef _AIX
#define HAVE_STDLIB
#endif
