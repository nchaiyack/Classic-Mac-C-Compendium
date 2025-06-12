/*******************************************************************************
 ***
 *** ctypes.h
 ***
 *** Globally useful data types
 *** Macintosh THINK C 5.0
 ***
 ***/


#ifndef __ctypes__
#define __ctypes__

#define private		static

#define elementsof(array)	(sizeof(array) / sizeof((array)[0]))


/**
 ** DATA TYPES
 **/

/*
 *	8-bit integers
 */
typedef		/*signed*/ char	int8;
typedef		unsigned char	uns8;
typedef		uns8			byte;

/*
 *	16-bit integers
 */
typedef		/*signed*/ int	int16;
typedef		unsigned int	uns16;

/*
 *	32-bit integers
 */
typedef		/*signed*/ long	int32;
typedef		unsigned long	uns32;

/*
 * IEEE floating point
 */
typedef		float			float32;	/* SANE SINGLE */
typedef		short double	float64;	/* SANE DOUBLE */
typedef		double			float80;	/* SANE EXTENDED */
typedef		double			float96;	/* if 68881 option on */

typedef		float32			floats;		/* smallest float */
typedef		float96			floatb;		/* biggest float */
typedef		float80			floatf;		/* fastest float */


#endif	/* ifndef __ctypes__ */
