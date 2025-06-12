/**********************************************************************\

File:		a4 setup.h

Purpose:	This is an alternate header file instead of using "SetUpA4.h"
			since that causes some technical problems in certain
			circumstances.  The problems are solved by using this
			shortened version and putting __GetA4() in the initialization
			.c file instead.
			
Note:		This file was not written by the authors of Sleep Deprivation
			and is not subject to the terms of the GNU General Public
			License.


Sleep Deprivation -- graphic effects on sleep
Copyright (C) 1993 Mark Pilgrim & Dave Blumenthal

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

/*
 *  SetUpA4.h
 *
 *  Copyright (c) 1991 Symantec Corporation.  All rights reserved.
 *
 */

#define RememberA4()	do { __GetA4(); asm { move.l a4,(a1) } } while (0)
#define RememberA0()	do { __GetA4(); asm { move.l a0,(a1) } } while (0)

#define SetUpA4()		do { asm { move.l a4,-(sp) } __GetA4(); asm { move.l (a1),a4 } } while (0)
#define RestoreA4()		do { asm { move.l (sp)+,a4 } } while (0)
