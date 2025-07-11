/*
**  wt -- a 3d game engine
**
**  Copyright (C) 1994 by Chris Laurel
**  email:  claurel@mr.net
**  snail mail:  Chris Laurel, 5700 W Lake St #208,  St. Louis Park, MN  55416
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* directory and file configuration */
#define TEXTURE_PATH ":textures"
#define DEFAULT_WORLD_FILE "\0"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif


#define True TRUE
#define False FALSE

/* MBW - Another Mac name conflict solved.  Just don't include
        this _before_ any MacOS headers!
*/
#define Region wt_Region

#define	_exit(x)	fatal_error("_exit(%d) called", x)

#define	PATH_MAX		80


#if !defined(__GNUC__) && !defined(__MWERKS__)
#define inline
#endif

/* For compilers without NULL defined in stdlib.h */
#ifndef NULL
#include <stdio.h>
#endif

/* The following types are needed by the drawing functions, where the exact
**   number of bits in a pixel is important.
*/
typedef unsigned char  int8;
typedef unsigned short int16;


void WTMain(char *WorldFile, short width, short height, Boolean blocky);
