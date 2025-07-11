/**********************************************************************\

File:		a4 setup.h

Purpose:	This is an alternate header file instead of using "SetUpA4.h"
			since that causes some technical problems in certain
			circumstances.  The problems are solved by using this
			shortened version and putting __GetA4() in the initialization
			.c file instead.
			
Note:		This file was not written by the author of Startup Screen
			Picker and is not subject to the terms of the GNU General
			Public License.


Startup Screen Picker -=- pick a random startup screen
Copyright (C) 1993 Mark Pilgrim

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
 *  This defines "SetUpA4()" and "RestoreA4()" routines that will work
 *  in all A4-based projects.
 *
 *  "RememberA4()" or "RememberA0()" must be called in advance to
 *  store away the value of A4 where it can be found by "SetUpA4()".
 *  The matching calls to "RememberA4()" (or "RememberA0()") and
 *  "SetUpA4()" *MUST* occur in the same file.
 *
 *  Note that "RememberA4()", "RememberA0()" "SetUpA4()", and
 *  "RestoreA4()" are not external.  Each file that uses them must
 *  include its own copy.
 *
 *  If this file is used in the main file of a code resource with
 *  "Custom Headers", be sure to #include it *AFTER* the custom
 *  header!  Otherwise, the code resource will begin with the code
 *  for the function "__GetA4()", defined below.
 *
 */

#define RememberA4()	do { __GetA4(); asm { move.l a4,(a1) } } while (0)
#define RememberA0()	do { __GetA4(); asm { move.l a0,(a1) } } while (0)

#define SetUpA4()		do { asm { move.l a4,-(sp) } __GetA4(); asm { move.l (a1),a4 } } while (0)
#define RestoreA4()		do { asm { move.l (sp)+,a4 } } while (0)
