/*==============================================================================
Project:	POV-Ray

Version:	2.2

File:	Config.h -- Macintosh

Description:
	This file contains Macintosh-specific defines, types, etc for both the
	Think C 5.0 and MPW 3.2 development environments.
------------------------------------------------------------------------------
Authors:
	Thomas Okken, David Lichtman, Glenn Sugden
	Jim Nitchals, Eduard [esp] Schwan
------------------------------------------------------------------------------
	from Persistence of Vision Raytracer
	Copyright 1993 Persistence of Vision Team
------------------------------------------------------------------------------
	NOTICE: This source code file is provided so that users may experiment
	with enhancements to POV-Ray and to port the software to platforms other 
	than those supported by the POV-Ray Team.  There are strict rules under
	which you are permitted to use this file.  The rules are in the file
	named POVLEGAL.DOC which should be distributed with this file. If 
	POVLEGAL.DOC is not available or for more info please contact the POV-Ray
	Team Coordinator by leaving a message in CompuServe's Graphics Developer's
	Forum.  The latest version of POV-Ray may be found there as well.

	This program is based on the popular DKB raytracer version 2.12.
	DKBTrace was originally written by David K. Buck.
	DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
------------------------------------------------------------------------------
More Info:
	This Macintosh version of POV-Ray was created and compiled by Jim Nitchals
	(Think 5.0) and Eduard Schwan (MPW 3.2), based (loosely) on the original
	port by Thomas Okken and David Lichtman, with some help from Glenn Sugden.

	For bug reports regarding the Macintosh version, you should contact:
	Eduard [esp] Schwan
		CompuServe: 71513,2161
		Internet: jl.tech@applelink.apple.com
		AppleLink: jl.tech
	Jim Nitchals
		Compuserve: 73117,3020
		America Online: JIMN8
		Internet: jimn8@aol.com -or- jimn8@applelink.apple.com
		AppleLink: JIMN8
------------------------------------------------------------------------------
Change History:
	920815	[jln]	version 1.0 Mac released.
	921105	[esp]	version 1.1 beta Mac
	930902	[esp]	version 1.9 beta Mac
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
	931119	[djh]	2.0.1 source conditionally compiles for PPC machines, keyword __powerc
==============================================================================*/

#if !defined(CONFIG_H)
#define CONFIG_H

#include <stdarg.h>
#include <stdlib.h>

#if defined(__powerc)
#include <fp.h>				/* superset of math.h */
#else
#include <math.h>			/* fmod, floor, HUGE_VAL! */
#endif

#include "POVLib.h"

/* Below are several defines needed by the generic parts of POV.  You
*   may redefine them as needed in this config.h file.
*
*   The following constants are needed by various POV modules.  Each has
*   a default (shown below) which is will be defined in frame.h if you don't 
*   define them here.
*
*   #define EPSILON 1.0e-10               - a small value used for POLY
*   #define FILE_NAME_LENGTH 150          - default file name length
*   #define HUGE_VAL 1.0e+17              - a really big number
*   #define DBL_FORMAT_STRING "%lf"       - the string to use for scanfs of doubles
*   #define DEFAULT_OUTPUT_FORMAT  'd'    - the default +f format
*   #define RED_RAW_FILE_EXTENSION ".red" - for +fr, this is the extension for the
*                                           red file
*   #define GREEN_RAW_FILE_EXTENSION ".grn"  - ditto for the green file
*   #define BLUE_RAW_FILE_EXTENSION ".blu"   - ditto again for the blue file
*   #define FILENAME_SEPARATOR "/"        - the character that separates names
*                                           in a path.
*   #define CASE_SENSITIVE_DEFAULT 2     - controls case sensitivity in DAT files
*   #define READ_FILE_STRING "rb"        - special binary (raw, uncooked) 
*   #define WRITE_FILE_STRING "wb"         modes of fopen()
*   #define APPEND_FILE_STRING "ab"
*   #define NORMAL '0'                   - machine-specific PaletteOption settings 
*   #define GREY   'G'
*
*   These routines are required by POV to compute trace times.  The defaults 
*   shown below are defined in most versions of C.  You may redefine them if
*   your compiler doesn't support these.  If time is completely unsupported 
*   define them as 0.
*
*   #define START_TIME time(&tstart);     
*   #define STOP_TIME  time(&tstop);
*   #define TIME_ELAPSED difftime (tstop, tstart);
*
*   Note difftime can be replaced with: #define TIME_ELAPSED (tstop - tstart); 
*   in some cases. 
*
*   These are optional routines that POV calls.  You may define them
*   or if undefined they will be defined as empty in frame.h.
*
*   #define STARTUP_POVRAY             - first executable statement of main
*                                           (useful for initialization)
*   #define PRINT_OTHER_CREDITS           - used for people who extend the machine specific modules
*                                           (put your own printf's here)
*   #define TEST_ABORT                    - code to test for ^C or abort
*                                           (called on each pixel)
*   #define FINISH_POVRAY              - last statement before exiting normally
*   #define COOPERATE                    - called for multi-tasking
*  
*   These items are standard on some C compilers.  If they are not defined
*   on your compiler or are called something different you may define your 
*   own or use the defaults supplied in frame.h as shown below.
*
*   #define DBL double                    - floating point precision
*   #define ACOS acos                     - arc cosine of DBL
*   #define SQRT sqrt                     - square root of DBL
*   #define POW pow                       - x to the y power - x, y DBL
*   #define COS cos                       - cosine of DBL
*   #define SIN sin                       - sine of DBL
*   #define labs(x) (long) ((x<0)?-x:x)   - Returns the absolute value of long integer x
*   #define max (x,y) ((x<y)?y:x)         - greater of x and y
*
*   On some systems "sprintf" returns the string rather than the length
*   of the string.  If the string is returned, define the following:
*   #define STRLN(x) strlen(x)
*
*   #define setvbuf(w,x,y,z)              - some systems don't understand setvbuf.
*                                           If not, just define it away - it's
*                                           not critical to the raytracer.  It
*                                           just buffers disk accesses.
*   This macro controls prototyping and is required by POV.  Choose one
*   of the two options below or if left undefined it will default to ANSI 
*   in frame.h
*
*   #define PARAMS (x) x                  - do ANSI prototyping
*   #define PARAMS (x) ()                 - do K&R non-ANSI prototyping
*
*   These allow command line vs non-command line versions and alternate
*   main routines.  Normally they should be undefined.  Define them 1 if needed.
*   #define ALTMAIN 1
*   #define NOCMDLINE 1
****************************************************/

/*==== FRAME.H Overrides ====*/

//		Identifier			Mac Value	.. Default Value
//		----------			---------	-- -------------
#define FILE_NAME_LENGTH	255			//	150
#define	FILENAME_SEPARATOR	":"			//	"/"
#define EPSILON				1.0e-10		//	1.0e-10
#if !defined(HUGE_VAL)
#define HUGE_VAL			1.0e+20		//	1.0e+17
#endif // !HUGE_VAL
#define BOUND_HUGE			1.0e30		//	1.0e30
#define MAX_CONSTANTS		1000		//	1000
#define ALTMAIN				1			//	0 (on makes main() a subroutine)
#define MAIN_RETURN_TYPE	int			//	main() return type

// turn on/off USE_LONG_DBL to use long float or regular float
// NOTE: MPW C compiler treats long double as extended, the native type,
// so supposedly there would be less converting happening.

#if !defined (__powerc)
#define USE_LONG_DBL	1
#endif

#if defined(USE_LONG_DBL)
#define DBL long double
#define DBL_FORMAT_STRING "%Lf"
#else
#define DBL double
#define DBL_FORMAT_STRING "%lf"
#endif // USE_LONG_DBL

#if defined (applec)
#define COMPILER_VER	".MACMPW"
#endif // applec

#if defined (THINK_C)
#define COMPILER_VER	".MACTHC"
#endif // THINK_C

#if defined (gcc)
#define COMPILER_VER	".MACGNU"
#endif // gcc

#if !defined(COMPILER_VER)
#define COMPILER_VER	".MAC???"
#endif // COMPILER_VER


#define START_TIME		{tused = 0; tstart = TICKS;}
#define STOP_TIME		tstop = TICKS; 
#define TIME_ELAPSED	((tstop - tstart) / 60L);

#define TEST_ABORT		Cooperate(false);	// if (Stop_Flag) return;

#define STARTUP_POVRAY	int Display_Started = false; int Shadow_Test_Flag = false;

#define COOPERATE		Cooperate(false);

#define PRINT_CREDITS		// called in Pov.c
#define PRINT_OTHER_CREDITS	// called in Pov.c

// Just use stock print_stats routine
// #define PRINT_STATS					// routine to display stats

// we don't do anything special on completion
#define FINISH_POVRAY					// called before exiting engine


/*==== Other Mac support ====*/
/*
The rest of this file is Mac-specific, and has nothing
to do with the generic POV-Ray code.
*/

/* capture any c library calls & do the Mac thing instead */
#define	exit(n)				catch_exit(n)


#if defined (THINK_C)
#define TICKS				Ticks
// typedef int (*__cmp_func)(const void *, const void *);
#define QSORT_FUNCT_RET		int
#define QSORT_FUNCT_PARAM	void *
#endif

#if defined (applec)
#include <sysequ.h>		// for Ticks
#define TICKS				(*(long*)Ticks)
#define QSORT_FUNCT_RET		int
#define QSORT_FUNCT_PARAM	void *
#endif // applec

#if defined (__powerc)
#include <sysequ.h>		// for Ticks
#define TICKS				(volatile *(long*)Ticks)
#define QSORT_FUNCT_RET		int
#define QSORT_FUNCT_PARAM	void *
#endif // applec

/* Mac-specific routines that must now be defined */

void	catch_exit(int n);
void	Cooperate(int doImmediate);


#endif /* CONFIG_H */
