/******************************************************************************\

File:		pstring.c

Purpose:	This module contains implementations of the familar string
			manipulation routines from the standard C libraries, but
			written to operate on Pascal strings.
			

``Tetris Light'' - a simple implementation of a Tetris game.
Copyright (C) 1993 Hoylen Sue

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the
Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

\******************************************************************************/

#include "local.h"
#include "pstring.h"

/*--------------------------------------------------------------------*/

void pstrcpy(Str255 dest, const Str255 src)
/* Copies the Pascal string from `src' to `dest'.  It is assumed that
   there is enough space in `dest' to hold the string. */
{
	register short count = src[0];
	
	do {
		*(dest++) = *(src++);
	} while (count-- > 0);
}

/*--------------------------------------------------------------------*/
