/**********************************************************************\

File:		highscore.h

Purpose:	Header file for the high score module.
			

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

#ifndef highscore_H
#define highscore_H

/*--------------------------------------------------------------------*/

extern Boolean highscore_init(void);
extern void highscore_term(void);

extern void highscore_start(void);
extern void highscore_add(unsigned int score);

extern Boolean high_score_load(void);
extern OSErr high_score_save(INTEGER pref_file);

extern OSErr highscore_save_location(INTEGER pref_file);
extern void highscore_load_location(void);

/*--------------------------------------------------------------------*/

#endif
