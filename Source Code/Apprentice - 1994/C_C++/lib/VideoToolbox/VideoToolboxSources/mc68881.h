/*
mc68881.h 
by Denis Pelli, 1992
Defines MPW C's mc68881 and mc68020 preprocessor symbols for THINK C, so that
your programs can use them in "if" statements without worrying about which compiler
you're using. The symbols are 1 (i.e. true) if the compiled code requires the chip 
(or better) and 0 otherwise.

CAUTION: Do not #include this file before FixMath.h
*/
#pragma once

#if THINK_C==1						/* THINK C 4 */
	#define mc68881 _MC68881_
	#define mc68020 _MC68020_
#endif
#if THINK_C>1						/* THINK C 5 */
	#if __option(mc68881)
		#define mc68881 1
	#else
		#define mc68881 0
	#endif
	#if __option(mc68020)
		#define mc68020 1
	#else
		#define mc68020 0
	#endif
#endif
