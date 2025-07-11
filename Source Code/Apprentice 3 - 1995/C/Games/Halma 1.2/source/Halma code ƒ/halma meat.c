/**********************************************************************\

File:		halma meat.c

Purpose:	This module handles all the blood and guts of Halma play;
			what constitutes a valid jump, etc.

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

#include "halma meat.h"
#include "halma main window.h"
#include "error.h"
#include "buttons.h"
#include "program globals.h"
#include "util.h"
#include "sounds.h"

short			gStickyButtonRow, gStickyButtonColumn;

void DoValidJump(WindowDataHandle theData, short theRow, short theColumn)
{
	if (gThisJumpString[0]==0xFF)
	{
		DoSound(sound_buzz, TRUE);
		HandleError(kTooManyJumpsError, FALSE);
		return;
	}
	
	Board[gStickyButtonRow][gStickyButtonColumn]=kNoPiece;
	Draw3DButton(&gButtonRect[gStickyButtonRow*9+gStickyButtonColumn], 0L, 0L,
		(**theData).windowDepth, FALSE, FALSE);

	Board[theRow][theColumn]=kHighlightedPiece;
	Draw3DButton(&gButtonRect[theRow*9+theColumn], 0L, (**theData).windowDepth>2 ?
		(Handle)gPieceIconColor : gPieceIconBW, (**theData).windowDepth, TRUE, FALSE);
	DoSound(sound_click, TRUE);
	gStickyButtonRow=theRow;
	gStickyButtonColumn=theColumn;
	gThisJumpString[++gThisJumpString[0]]=theRow*9+theColumn+'A';
	gNumJumps++;
	(**theData).offscreenNeedsUpdate=TRUE;
}

void DoValidFirstMove(WindowDataHandle theData, short theRow, short theColumn,
	Boolean drawPiece)
{
	Board[theRow][theColumn]=kHighlightedPiece;
	if (drawPiece)
		Draw3DButton(&gButtonRect[theRow*9+theColumn], 0L, (**theData).windowDepth>2 ?
			(Handle)gPieceIconColor : gPieceIconBW, (**theData).windowDepth, TRUE, FALSE);
	gStickyButtonRow=gThisStartRow=theRow;
	gStickyButtonColumn=gThisStartColumn=theColumn;
	(**theData).offscreenNeedsUpdate=TRUE;
	gNumJumps=0;
	gThisJumpString[0]=0x01;
	gThisJumpString[1]='A'+theRow*9+theColumn;
}

void DoValidSingleMove(WindowDataHandle theData, short theRow, short theColumn)
{
	DoValidJump(theData, theRow, theColumn);
	SaveJumpChain();
	DoValidFirstMove(theData, theRow, theColumn, FALSE);
	DrawScoreAndStuff(theData, (**theData).windowDepth);
}

void DoValidSingleMoveNewMove(WindowDataHandle theData, short theRow, short theColumn)
{
	SaveJumpChain();
	DoValidFirstMove(theData, gStickyButtonRow, gStickyButtonColumn, FALSE);
	DoValidSingleMove(theData, theRow, theColumn);
}

void DoOffBoard(WindowDataHandle theData)
{
	if ((gStickyButtonRow!=-1) && (gStickyButtonColumn!=-1))
		DoAlreadyHighlighted(theData, gStickyButtonRow, gStickyButtonColumn);
}

void DoCantJumpThere(WindowDataHandle theData)
{
	DoSound(sound_buzz, TRUE);
	HandleError(kCantGetThereFromHereError, FALSE);
}

void DoMustJumpOverPiece(WindowDataHandle theData)
{
	DoSound(sound_buzz, TRUE);
	HandleError(kJumpOverNonPieceError, FALSE);
}

void DoCantSelectNonPiece(WindowDataHandle theData)
{
	DoSound(sound_buzz, TRUE);
	HandleError(kNonPieceError, FALSE);
}

void DoSpaceNotEmpty(WindowDataHandle theData, short theRow, short theColumn)
{
	DoAlreadyHighlighted(theData, gStickyButtonRow, gStickyButtonColumn);
	DoValidFirstMove(theData, theRow, theColumn, TRUE);
}

void DoAlreadyHighlighted(WindowDataHandle theData, short theRow, short theColumn)
{
	Board[theRow][theColumn]=kPiece;
	Draw3DButton(&gButtonRect[theRow*9+theColumn], 0L, 
		(**theData).windowDepth>2 ? (Handle)gPieceIconColor : gPieceIconBW,
		(**theData).windowDepth, FALSE, FALSE);
	gStickyButtonRow=gStickyButtonColumn=-1;

	if (((theRow!=gThisStartRow) || (theColumn!=gThisStartColumn)) && (gNumJumps>0))
		SaveJumpChain();
	
	gNumJumps=0;
	gThisJumpString[0]=0x00;
	DrawScoreAndStuff(theData, (**theData).windowDepth);
}

enum MoveTypes GetMoveType(short theRow, short theColumn)
{
	short			hdif, vdif;
	short			vave, have;
	
	if ((theRow<0) || (theRow>=gNumRows) || (theColumn<0) || (theColumn>=gNumColumns))
		return kOffBoard;
	
	if ((gStickyButtonRow==-1) && (gStickyButtonColumn==-1))
		return (Board[theRow][theColumn]==kPiece) ? kValidFirstMove : kCantSelectNonPiece;
	
	if ((theRow==gStickyButtonRow) && (theColumn==gStickyButtonColumn))
		return kAlreadyHighlighted;
	
	if (Board[theRow][theColumn]!=kNoPiece)
		return kSpaceNotEmpty;
	
	vdif=theRow-gStickyButtonRow;
	if (vdif<0) vdif=-vdif;
	hdif=theColumn-gStickyButtonColumn;
	if (hdif<0) hdif=-hdif;

	if (((vdif==2) && (hdif==2)) || ((vdif==2) && (hdif==0)) ||
		((vdif==0) && (hdif==2)))
	{
		vave=(theRow+gStickyButtonRow)/2;
		have=(theColumn+gStickyButtonColumn)/2;
		return (Board[vave][have]==kPiece) ? kValidJump : kMustJumpOverPiece;
	}
	else if ((vdif==1) && (hdif==1) || (vdif==1) && (hdif==0) ||
			(vdif==0) && (hdif==1))
		return (gNumJumps==0) ? kValidSingleMove : kValidSingleMoveNewMove;
	else
		return kCantJumpThere;
}

void SaveJumpChain(void)
{
	unsigned long		theSize;
	
	theSize=GetHandleSize((Handle)gTheFullJumpHandle);
	SetHandleSize((Handle)gTheFullJumpHandle, theSize+gThisJumpString[0]+1);
	Mymemcpy((Ptr)(*gTheFullJumpHandle+gThisJumpString[0]+1), (Ptr)(*gTheFullJumpHandle),
		theSize);
	Mymemcpy((Ptr)(*gTheFullJumpHandle), (Ptr)gThisJumpString, gThisJumpString[0]+1);
	gThisJumpString[0]=0x00;
	gNumJumps=0;
	gNumMoves++;
}

void UndoOneJump(WindowDataHandle theData)
{
	unsigned char	thisChar;
	short			theRow, theColumn;
	
	if (gThisJumpString[0]>0x01)	/* back up one in jump chain */
	{
		Board[gStickyButtonRow][gStickyButtonColumn]=kNoPiece;
		Draw3DButton(&gButtonRect[gStickyButtonRow*9+gStickyButtonColumn], 0L, 0L,
			(**theData).windowDepth, FALSE, FALSE);
		gThisJumpString[0]--;
		gNumJumps--;
		thisChar=gThisJumpString[gThisJumpString[0]]-'A';
		gStickyButtonColumn=theColumn=thisChar%9;
		gStickyButtonRow=theRow=(thisChar-theColumn)/9;
		Board[theRow][theColumn]=kHighlightedPiece;
		Draw3DButton(&gButtonRect[theRow*9+theColumn], 0L, (**theData).windowDepth>2 ?
			(Handle) gPieceIconColor : gPieceIconBW, (**theData).windowDepth, TRUE, FALSE);
		DoSound(sound_click, TRUE);
	}
	else	/* piece highlighted but no jumps yet --> no piece highlighted */
	{
		Board[gStickyButtonRow][gStickyButtonColumn]=kPiece;
		Draw3DButton(&gButtonRect[gStickyButtonRow*9+gStickyButtonColumn], 0L, 
			(**theData).windowDepth>2 ? (Handle)gPieceIconColor : gPieceIconBW,
			(**theData).windowDepth, FALSE, FALSE);
		gStickyButtonRow=gStickyButtonColumn=-1;
		gThisJumpString[0]=0x00;
	}
	
	DrawScoreAndStuff(theData, (**theData).windowDepth);
}

void UndoWholeMove(WindowDataHandle theData)
{
	unsigned char	lengthOfLastJump;
	unsigned long	newFullJumpHandleLength;
	unsigned char	thisChar;
	
	lengthOfLastJump=**gTheFullJumpHandle;
	newFullJumpHandleLength=GetHandleSize((Handle)gTheFullJumpHandle)-lengthOfLastJump-1;
	Mymemcpy((Ptr)gThisJumpString, (Ptr)(*gTheFullJumpHandle), lengthOfLastJump+1);
	Mymemcpy((Ptr)*gTheFullJumpHandle, (Ptr)*gTheFullJumpHandle+lengthOfLastJump+1,
		newFullJumpHandleLength);
	SetHandleSize((Handle)gTheFullJumpHandle, newFullJumpHandleLength);
	gNumJumps=gThisJumpString[0]-1;
	gNumMoves--;
	thisChar=gThisJumpString[gThisJumpString[0]]-'A';
	gStickyButtonColumn=thisChar%9;
	gStickyButtonRow=(thisChar-gStickyButtonColumn)/9;
	UndoOneJump(theData);
}

short BestSolution(void)
{
// this could be compressed a lot, but it's in this expanded for readability
	switch (gNumRows)
	{
		case 4:
			switch (gNumColumns)
			{
				case 4:		return 6;
				case 5:		return 8;
				case 6:		return 10;
				case 7:		return 12;
				case 8:		return 16;
				default:	return 0;
			}
			break;
		case 5:
			switch (gNumColumns)
			{
				case 4:		return 8;
				case 5:		return 9;
				case 6:		return 12;
				default:	return 0;
			}
			break;
		case 6:
			switch (gNumColumns)
			{
				case 4:		return 10;
				case 5:		return 12;
				case 6:		return 12;
				default:	return 0;
			}
			break;
		case 7:
			switch (gNumColumns)
			{
				case 4:		return 12;
				case 7:		return 13;
				default:	return 0;
			}
			break;
		case 8:
			switch (gNumColumns)
			{
				case 4:		return 16;
				case 8:		return 15;
				default:	return 0;
			}
			break;
		case 9:
			switch (gNumColumns)
			{
				case 9:		return 16;
				default:	return 0;
			}
			break;
	}
	
	return 0;
}
