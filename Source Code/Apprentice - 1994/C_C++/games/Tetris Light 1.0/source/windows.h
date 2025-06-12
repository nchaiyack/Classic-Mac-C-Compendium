/**********************************************************************\

File:		windows.h

Purpose:	Header file for the window handling module.  This module
			implements a dispatch table mechanism which is used to
			handle window events.


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

#ifndef windows_H
#define windows_H

/*--------------------------------------------------------------------*/

/* This structure is the dispatch table for window events.  Each
   window created has its refCon set to point to one of these tables.
   This method provides an extensible window handling mechanism. */

typedef struct {
	void (* mouseDown_handler)(Point where);
	void (* key_handler)(unsigned char code, unsigned char ascii);
	void (* update_handler)(void);
	void (* activate_handler)(void);
	void (* deactivate_handler)(void);
} Wind_table;

/*--------------------------------------------------------------------*/

void window_mouseDown(WindowPtr wind, Point where);
void window_key(WindowPtr wind, unsigned char code, unsigned char ascii);
void window_update(WindowPtr wind);
void window_activate(WindowPtr wind);
void window_deactivate(WindowPtr wind);

/*--------------------------------------------------------------------*/

#endif
