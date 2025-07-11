/**********************************************************************\

File:		alert.h

Purpose:	This module provides routines to display various messages
			to the user in alerts.
			

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

#ifndef alert_H
#define alert_H

/*--------------------------------------------------------------------*/

extern void alert_caution(INTEGER msg_id);
extern void alert_erc(INTEGER msg_id, OSErr err);
extern void alert_fatal(INTEGER msg_id);

/*--------------------------------------------------------------------*/

#endif
