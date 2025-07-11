/*
==============================================================================
Project:	POV-Ray

Version:	2.2

File Name:	Stdio_p2w.h

Description:
	General-purpose printf-capturing routines that allow a console-like
	output window for c programs that otherwise prefer to use printf/fprintf.
	This code was "inspired heavily" from sources such as MacDTS'es TESample,
	MacApp's Transcript window, and previous code of mine.  It is fairly well
	self-contained, and works in MPW C 3.2 and Think C 5.0.

	This file contains global definitions used by any source files that would
	otherwise include <stdio.h>.. that is, non-Macintosh-toolbox-aware.  It is
	_REQUIRED_ that those files be modified to include "stdio_p2w.h" instead of
	<stdio.h>, since this header will include <stdio.h> for them.
	NOTE: If stdio.h is included after this header, problems will occur.

Related Files:
	Stdio_p2w.h		- generic header for sources that would otherwise use <stdio.h>
	Printf2Window.h	- Mac-specific header for p2w routines
	Printf2Window.c	- the main source for the p2w routines
------------------------------------------------------------------------------
Author:
	Eduard [esp] Schwan
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
	920326	[esp]	Created.
	920330	[esp]	Updated file header with copyright & related files info
	920424  (jln)   removed putc definition to avoid problems writing Targa format.
	920621	[esp]	Changed file header definition to STDIOP2W_H for ANSI consistency
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
==============================================================================
*/

#if !defined (STDIOP2W_H)
#define STDIOP2W_H


#include <stdio.h>


// ==== <stdio.h> replacement macros (Forces std C i/o to call us instead)

#undef	fflush
#undef	fprintf
#undef	fputc
#undef	fputs
#undef	printf
// #undef	putc	-- let it pass for now [esp]
#undef	putchar
#undef	puts

#define fflush		p2w_fflush
#define fprintf		p2w_fprintf
#define fputc		p2w_fputc
#define fputs		p2w_fputs
#define printf		p2w_printf
// #define putc		p2w_putc  -yeah right! not until we get pass-thru to system for non-console.
#define putchar		p2w_putchar
#define puts		p2w_puts

// ==== Std C library replacement routines

int p2w_fflush(FILE *stream);
int p2w_fprintf(FILE *stream, const char *format, ...);
int p2w_fputc(int theChar, FILE *stream);
int p2w_fputs(const char *theString, FILE *stream);
int p2w_printf(const char * format, ...);
int p2w_putc(int theChar, FILE *stream);
int p2w_putchar(const char theChar);
int p2w_puts(const char *theString);


#endif // STDIOP2W_H
