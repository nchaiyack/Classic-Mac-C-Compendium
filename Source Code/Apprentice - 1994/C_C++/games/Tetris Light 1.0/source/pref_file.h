/**********************************************************************\

File:		pref_file.h

Purpose:	Header file for the preference file management module.
			

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

#ifndef pref_file_H
#define pref_file_H

/*--------------------------------------------------------------------*/

extern OSErr pref_open(INTEGER *ref, INTEGER pref_name_str_resid,
					   OSType creator, OSType type,
					   Boolean create_if_not_found);

/*--------------------------------------------------------------------*/

#endif
