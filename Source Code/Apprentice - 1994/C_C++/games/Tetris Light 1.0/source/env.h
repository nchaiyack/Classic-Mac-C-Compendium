/**********************************************************************\

File:		env.h

Purpose:	This module provides routines to determine the facilities
			available on the running machine.
			

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

\**********************************************************************/

#ifndef env_H
#define env_H

/*--------------------------------------------------------------------*/

extern Boolean env_trap_available(INTEGER trap_number);

extern Boolean env_WaitNextEvent_available(void);
extern Boolean env_FindFolder_available(void);
extern Boolean env_SndPlay_available(void);

/*--------------------------------------------------------------------*/

#endif

