////////////
//
//	CKing.cp
//
//	Chess Piece methods for implementing a King.
//
//	Copyright © 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <stdlib.h>
#include <string.h>

#include "CChessBoard.h"
#include "CChessPiece.h"
#include "CKing.h"

extern	CIconHandle	gWhiteKingCicnHandle;
extern	CIconHandle	gBlackKingCicnHandle;

void CKing::IKing(Boolean aColor)
{
	inherited::IChessPiece(aColor);
	itsValue = kKingValue;
	canCastle = true;
}

void CKing::Draw(short rank, short file)
{
	CIconHandle cicnHandle;
	Rect aRect;
	short rankQD = (rank - 1) << 5, fileQD = (file - 1) << 5;
	
	cicnHandle = (itsColor == White) ? gWhiteKingCicnHandle : gBlackKingCicnHandle;

	aRect.left = rankQD;
	aRect.right = rankQD+32;
	aRect.top = fileQD;
	aRect.bottom = fileQD+32;
	PlotCIcon(&aRect,cicnHandle);
}

CIconHandle	CKing::GetCicnHandle(void)
{
	return gWhiteKingCicnHandle;
}

Boolean	CKing::IsValidMove(CChessBoard *aBoard, short newRank, short newFile)
{
	short	oldRank = aBoard->firstClickRank, oldFile = aBoard->firstClickFile;

	if ((abs(oldRank-newRank) <= 1) && (abs(oldFile-newFile) <= 1))
		return true;

	if ((abs(oldRank-newRank) == 2) && (canCastle == true))
	{
		if ( 1 )
			;	//	do castling stuff here

	}

	return false;
}

void CKing::RegisterMove(short rank, short file)
{
	canCastle = false;
	inherited::RegisterMove(rank,file);
}