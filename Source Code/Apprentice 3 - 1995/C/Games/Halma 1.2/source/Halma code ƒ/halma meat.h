/**********************************************************************\

File:		halma meat.h

Purpose:	This is the header file for halma meat.c.

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

extern	short			gStickyButtonRow, gStickyButtonColumn;

enum MoveTypes
{
	kValidJump=0,			/* good move of already-selected piece */
	kValidFirstMove,		/* nothing else selected, select any piece */
	kValidSingleMove,		/* move piece single space */
	kValidSingleMoveNewMove,/* move piece single space after registering previous jumps
								as separate move */
	kOffBoard,				/* click off board */
	kCantJumpThere,			/* from where you are, must move (2,0), (0,2), or (2,2) */
	kMustJumpOverPiece,		/* tried to jump over empty square */
	kCantSelectNonPiece,	/* nothing selected yet, must select piece */
	kSpaceNotEmpty,			/* must jump into empty space */
	kAlreadyHighlighted		/* space already highlighted */
};

void DoValidJump(WindowDataHandle theData, short theRow, short theColumn);
void DoValidFirstMove(WindowDataHandle theData, short theRow, short theColumn,
	Boolean drawPiece);
void DoValidSingleMove(WindowDataHandle theData, short theRow, short theColumn);
void DoValidSingleMoveNewMove(WindowDataHandle theData, short theRow, short theColumn);
void DoOffBoard(WindowDataHandle theData);
void DoCantJumpThere(WindowDataHandle theData);
void DoMustJumpOverPiece(WindowDataHandle theData);
void DoCantSelectNonPiece(WindowDataHandle theData);
void DoSpaceNotEmpty(WindowDataHandle theData, short theRow, short theColumn);
void DoAlreadyHighlighted(WindowDataHandle theData, short theRow, short theColumn);
enum MoveTypes GetMoveType(short theRow, short theColumn);
void SaveJumpChain(void);
void UndoOneJump(WindowDataHandle theData);
void UndoWholeMove(WindowDataHandle theData);
short BestSolution(void);
