/**********************************************************************\

File:		windows.c

Purpose:	This module implements a dispatch table mechanism which is
			used to handle window events.
			

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

#include "local.h"
#include "windows.h"

/*--------------------------------------------------------------------*/

void window_mouseDown(WindowPtr wind, Point where)
/* Sends a mouseDown event to the given window. */
{
	register Wind_table *table = (Wind_table *) GetWRefCon(wind);
	
	if (table->mouseDown_handler != 0)
		table->mouseDown_handler(where);
}

/*--------------------------------------------------------------------*/

void window_key(WindowPtr wind, unsigned char code, unsigned char ascii)
/* Sends a key press event to the given window. */
{
	register Wind_table *table = (Wind_table *) GetWRefCon(wind);
	
	if (table->key_handler != 0)
		table->key_handler(code, ascii);
}

/*--------------------------------------------------------------------*/

void window_update(WindowPtr wind)
/* Sends an update event to the given window. */
{
	register Wind_table *table = (Wind_table *) GetWRefCon(wind);

	BeginUpdate(wind);
	if (table->update_handler != 0)
		table->update_handler();
	EndUpdate(wind);
}

/*--------------------------------------------------------------------*/

void window_activate(WindowPtr wind)
/* Sends an activate event to the given window. */
{
	register Wind_table *table = (Wind_table *) GetWRefCon(wind);
	
	if (table->activate_handler != 0)
		table->activate_handler();
}

/*--------------------------------------------------------------------*/

void window_deactivate(WindowPtr wind)
/* Sends a deactivate event to the given window. */
{
	register Wind_table *table = (Wind_table *) GetWRefCon(wind);
	
	if (table->deactivate_handler != 0)
		table->deactivate_handler();
}

/*--------------------------------------------------------------------*/
