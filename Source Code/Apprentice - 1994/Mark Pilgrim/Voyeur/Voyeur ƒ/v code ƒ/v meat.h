/**********************************************************************\

File:		v meat.h

Purpose:	This is the header file for v meat.c


Voyeur -- a no-frills file viewer
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

void ChangeEOF(void);
void DoDataFork(void);
void DoResourceFork(void);
void GoForward(void);
void GoBack(void);
void GoBeginning(void);
void GoEnd(void);
void GoOffset(void);
