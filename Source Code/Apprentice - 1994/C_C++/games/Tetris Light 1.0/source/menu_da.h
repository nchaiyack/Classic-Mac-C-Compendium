/**********************************************************************\

File:		menu_da.h

Purpose:	Header file for the menu and desk accessory handling routines.
			

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

#ifndef menu_da_H
#define menu_da_H

/*--------------------------------------------------------------------*/

extern Boolean menu_init(void);
extern Boolean menu_choice(LONGINT choice);
extern void update_filter_menu(INTEGER old, INTEGER new_value);
extern void adjust_menus(void);
extern Boolean is_DA_window(WindowPtr w_ptr);

/*--------------------------------------------------------------------*/

#endif
