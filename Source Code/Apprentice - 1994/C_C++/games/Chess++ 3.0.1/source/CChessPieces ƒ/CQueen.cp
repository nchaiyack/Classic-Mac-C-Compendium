////////////
//
//	CQueen.cp
//
//	Chess Piece methods for implementing a Queen.
//
//	Copyright © 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <stdlib.h>
#include <string.h>

#include "CChessBoard.h"
#include "CChessPiece.h"
#include "CQueen.h"

extern	CIconHandle	gWhiteQueenCicnHandle;
extern	CIconHandle	gBlackQueenCicnHandle;

void CQueen::IQueen(Boolean aColor)
{
	inherited::IChessPiece(aColor);
	itsValue = 0x0800;
}

void CQueen::Draw(short rank, short file)
{
	CIconHandle cicnHandle;
	Rect aRect;
	short rankQD = (rank - 1) << 5, fileQD = (file - 1) << 5;
	
	if (itsColor == White)
		cicnHandle = gWhiteQueenCicnHandle;
	else
		cicnHandle = gBlackQueenCicnHandle;

	aRect.left = rankQD;
	aRect.right = rankQD+32;
	aRect.top = fileQD;
	aRect.bottom = fileQD+32;
	PlotCIcon(&aRect,cicnHandle);
}

CIconHandle	CQueen::GetCicnHandle(void)
{
	return gWhiteQueenCicnHandle;
}

Boolean	CQueen::IsValidMove(CChessBoard *aBoard, short newRank, short newFile)
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
	
	if (oldRank == newRank)
	{
		iDelta = abs(newFile-oldFile)/(newFile-oldFile);
		for (i=oldFile+iDelta;i!=newFile;i+=iDelta)
			if (aBoard->theBoard[oldRank][i])
				return false;
		return true;
	}
	if (oldFile == newFile)
	{
		iDelta = abs(newRank-oldRank)/(newRank-oldRank);
		for (i=oldRank+iDelta;i!=newRank;i+=iDelta)
			if (aBoard->theBoard[i][oldFile])
				return false;
		return true;
	}
	
	return false;	
}