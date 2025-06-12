////////////
//
//	CPawn.cp
//
//	Chess Piece methods for implementing a Pawn.
//
//	Copyright © 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <stdlib.h>

#include "CChessBoard.h"
#include "CChessPiece.h"
#include "CPawn.h"

extern		CIconHandle	gWhitePawnCicnHandle;
extern		CIconHandle	gBlackPawnCicnHandle;

void CPawn::IPawn(Boolean aColor)
{
	inherited::IChessPiece(aColor);
	itsValue = 0x0100;
}

void CPawn::Draw(short rank, short file)
{
	CIconHandle cicnHandle;
	Rect aRect;
	short rankQD = (rank - 1) << 5, fileQD = (file - 1) << 5;
	
	if (itsColor == White)
		cicnHandle = gWhitePawnCicnHandle;
	else
		cicnHandle = gBlackPawnCicnHandle;

	aRect.left = rankQD;
	aRect.right = rankQD+32;
	aRect.top = fileQD;
	aRect.bottom = fileQD+32;
	PlotCIcon(&aRect,cicnHandle);
}

CIconHandle	CPawn::GetCicnHandle(void)
{
	return gWhitePawnCicnHandle;
}


Boolean	CPawn::IsValidMove(CChessBoard *aBoard, short newRank, short newFile)
{
	short	oldRank = aBoard->firstClickRank, oldFile = aBoard->firstClickFile,
			newColor,
			myColor = aBoard->theBoard[oldRank][oldFile]->itsColor,
			theOtherPlayersColor;

	if (myColor == White)
		theOtherPlayersColor=Black;
	else
		theOtherPlayersColor=White;

	if (aBoard->theBoard[newRank][newFile])
		newColor = aBoard->theBoard[newRank][newFile]->itsColor;
	else
		newColor = NoColor;
	
	if ((oldRank == newRank) && (newColor == NoColor))
	{
		if (((oldFile - newFile) == 1) && (myColor == White))
			return true;
		if (((newFile - oldFile) == 1) && (myColor == Black))
			return true;
		if (((oldFile - newFile) == 2) && (myColor == White) &&
			(oldFile == 7) && (!(aBoard->theBoard[oldRank][6])))
			return true;
		if (((newFile - oldFile) == 2) && (myColor == Black) &&
			(oldFile == 2) && (!(aBoard->theBoard[oldRank][3])))
			return true;
	}
	if ((abs(oldRank-newRank) == 1) && (newColor == theOtherPlayersColor))
	{
		if (((oldFile - newFile) == 1) && (myColor == White))
			return true;
		if (((newFile - oldFile) == 1) && (myColor == Black))
			return true;
	}
	return false;
}


short	CPawn::BoardLocationValue(CChessBoard *aBoard, short rank, short file)
{
	short killFile;
	CChessPiece	*leftTarget,*rightTarget;
	short	totalValue;

	totalValue = inherited::BoardLocationValue(aBoard,rank,file);

	killFile = file + ((itsColor == White) ? -1 : 1);
	
	leftTarget = aBoard->theBoard[rank-1][killFile];
	rightTarget = aBoard->theBoard[rank+1][killFile];

	if (leftTarget)
		if (itsColor == leftTarget->itsColor)
			totalValue += 
				((leftTarget->itsValue == kKingValue) ? 0 : leftTarget->itsValue) >> 5;
	
	if (rightTarget)
		if (itsColor == rightTarget->itsColor)
			totalValue +=
				((rightTarget->itsValue == kKingValue) ? 0 : rightTarget->itsValue) >> 5;

	return totalValue;
}


void CPawn::RegisterMove(short rank, short file)
{
	inherited::RegisterMove(rank,file);
}