/**********************************************************************\

File:		halma main window.h

Purpose:	This is the header file for halma main window.c.

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

#pragma once

#include "graphics.h"

#define NUM_BUTTONS 81

extern	Rect			gButtonRect[NUM_BUTTONS];
extern	PicHandle		gPieceColorPict;
extern	PicHandle		gPieceBWPict;

short MainWindowDispatch(WindowDataHandle theData, short theMessage,
	unsigned long misc);
void DrawScoreAndStuff(WindowDataHandle theData, short theDepth);
