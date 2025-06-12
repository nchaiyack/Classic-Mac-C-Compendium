////////////
//
//	CRook.cp
//
//	Chess Piece methods for implementing a Rook.
//
//	Copyright © 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <stdlib.h>
#include <string.h>

#include "CChessBoard.h"
#include "CChessPiece.h"
#include "CRook.h"

extern	CIconHandle	gWhiteRookCicnHandle;
extern	CIconHandle	gBlackRookCicnHandle;

void CRook::IRook(Boolean aColor)
{
	inherited::IChessPiece(aColor);
	itsValue = 0x0500;
	canCastle = true;
}

void CRook::Draw(short rank, short file)
{
	CIconHandle cicnHandle;
	Rect aRect;
	short rankQD = (rank - 1) << 5, fileQD = (file - 1) << 5;
	
	if (itsColor == White)
		cicnHandle = gWhiteRookCicnHandle;
	else
		cicnHandle = gBlackRookCicnHandle;

	aRect.left = rankQD;
	aRect.right = rankQD+32;
	aRect.top = fileQD;
	aRect.bottom = fileQD+32;
	PlotCIcon(&aRect,cicnHandle);
}

CIconHandle	CRook::GetCicnHandle(void)
{
	return gWhiteRookCicnHandle;
}

Boolean	CRook::IsValidMove(CChessBoard *aBoard, short newRank, short newFile)
{
	short	oldRank = aBoard->firstClickRank, oldFile = aBoard->firstClickFile;
	register i;
	short iDelta;
	
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

void CRook::RegisterMove(short rank, short file)
{
	canCastle = false;
	inherited::RegisterMove(rank,file);
}