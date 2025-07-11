////////////
//
//	CChessPiece.cp
//
//	Abstract Chess Piece methods.
//
//	Copyright � 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <stdlib.h>
#include "CChessPiece.h"
#include "CChessBoard.h"

extern CIconHandle gWhitePawnCicnHandle;

void CChessPiece::IChessPiece(short aColor)
{
	itsColor = aColor;
}


void CChessPiece::Draw(short rank, short file)
{
	// Null method -- must be overridden
}


Boolean	CChessPiece::IsValidMove(CChessBoard *aBoard, short newRank, short newFile)
{
	// this is the default method -- it returns false just in case.
	return false;
}


short	CChessPiece::BoardLocationValue(CChessBoard *aBoard, short rank, short file)
{
	short	theKingRank,theKingFile;
	short	rankDist,fileDist;	

	if (itsColor == White)
	{
		theKingRank = aBoard->blackKingRank;
		theKingFile = aBoard->blackKingFile;
	}
	else
	{
		theKingRank = aBoard->whiteKingRank;
		theKingFile = aBoard->whiteKingFile;
	}
	
	rankDist = abs(theKingRank-rank);
	fileDist = abs(theKingFile-file);
	
	return ((9 - rankDist) + (9 - fileDist)) * 2;
}


CIconHandle	CChessPiece::GetCicnHandle(void)
{
	// this is the default method which returns the pawn color icon handle --
	// this method must be overridden
	return gWhitePawnCicnHandle;
}

void	CChessPiece::RegisterMove(short rank, short file)
{
	// this is the default method in case a piece has nothing to register.
}