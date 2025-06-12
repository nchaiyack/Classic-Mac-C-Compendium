////////////
//
//	CBishop.cp
//
//	Abstract Chess Piece methods.
//
//	Copyright © 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <stdlib.h>

#include "CChessBoard.h"
#include "CChessPiece.h"
#include "CBishop.h"

extern	CIconHandle	gWhiteBishopCicnHandle;
extern	CIconHandle	gBlackBishopCicnHandle;

void CBishop::IBishop(Boolean aColor)
{
	inherited::IChessPiece(aColor);
	itsValue = 0x0300;
}

void CBishop::Draw(short rank, short file)
{
	CIconHandle cicnHandle;
	Rect aRect;
	short rankQD = (rank - 1) << 5, fileQD = (file - 1) << 5;
	
	if (itsColor == White)
		cicnHandle = gWhiteBishopCicnHandle;
	else
		cicnHandle = gBlackBishopCicnHandle;

	aRect.left = rankQD;
	aRect.right = rankQD+32;
	aRect.top = fileQD;
	aRect.bottom = fileQD+32;
	PlotCIcon(&aRect,cicnHandle);
}

CIconHandle	CBishop::GetCicnHandle(void)
{
	return gWhiteBishopCicnHandle;
}

Boolean	CBishop::IsValidMove(CChessBoard *aBoard, short newRank, short newFile)
{
	short	oldRank = aBoard->firstClickRank, oldFile = aBoard->firstClickFile;
	register i,j;
	short iDelta,rankDelta = newRank-oldRank,fileDelta = newFile-oldFile;
	
	if (abs(rankDelta) == abs(fileDelta))
	{
		rankDelta = abs(rankDelta)/rankDelta;
		fileDelta = abs(fileDelta)/fileDelta;
		for(i=oldRank+rankDelta,j=oldFile+fileDelta;i!=newRank;
			i+=rankDelta,j+=fileDelta)
				if (aBoard->theBoard[i][j])
					return false;
		return true;
	}

	return false;	
}