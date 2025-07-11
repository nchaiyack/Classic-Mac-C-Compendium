////////////
//
//	CKnight.cp
//
//	Chess Piece methods for implementing a Knight.
//
//	Copyright � 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <stdlib.h>
#include <string.h>

#include "CChessBoard.h"
#include "CChessPiece.h"
#include "CKnight.h"

extern	CIconHandle	gWhiteKnightCicnHandle;
extern	CIconHandle	gBlackKnightCicnHandle;

void CKnight::IKnight(Boolean aColor)
{
	inherited::IChessPiece(aColor);
	itsValue = 0x0300;
}

void CKnight::Draw(short rank, short file)
{
	CIconHandle cicnHandle;
	Rect aRect;
	short rankQD = (rank - 1) << 5, fileQD = (file - 1) << 5;
	
	if (itsColor == White)
		cicnHandle = gWhiteKnightCicnHandle;
	else
		cicnHandle = gBlackKnightCicnHandle;

	aRect.left = rankQD;
	aRect.right = rankQD+32;
	aRect.top = fileQD;
	aRect.bottom = fileQD+32;
	PlotCIcon(&aRect,cicnHandle);
}

CIconHandle	CKnight::GetCicnHandle(void)
{
	return gWhiteKnightCicnHandle;
}

Boolean	CKnight::IsValidMove(CChessBoard *aBoard, short newRank, short newFile)
{
	short	oldRank = aBoard->firstClickRank, oldFile = aBoard->firstClickFile;
	
	if ((abs(newRank-oldRank) == 2) && (abs(newFile-oldFile) == 1))
		return true;
	if ((abs(newRank-oldRank) == 1) && (abs(newFile-oldFile) == 2))
		return true;
	return false;
}


static CChessPiece *GetPiece(CChessBoard *board,short rank,short file)
{
	if (rank<1)
		return NULL;
	if (rank>8)
		return NULL;
	if (file<1)
		return NULL;
	if (file>8)
		return NULL;
		
	return board->theBoard[rank][file];
}


short	CKnight::BoardLocationValue(CChessBoard *aBoard, short rank, short file)
{
	register short	thisPieceColor = itsColor;
	CChessPiece		*target;
	short			totalValue;
	long			pieceValues = 0;

	totalValue = inherited::BoardLocationValue(aBoard,rank,file);

	target = GetPiece(aBoard,rank-1,file-2);
	if (target)
		if (thisPieceColor == target->itsColor)
			pieceValues += (target->itsValue == kKingValue) ? 0 : target->itsValue;

	target = GetPiece(aBoard,rank-1,file+2);
	if (target)
		if (thisPieceColor == target->itsColor)
			pieceValues += (target->itsValue == kKingValue) ? 0 : target->itsValue;

	target = GetPiece(aBoard,rank+1,file-2);
	if (target)
		if (thisPieceColor == target->itsColor)
			pieceValues += (target->itsValue == kKingValue) ? 0 : target->itsValue;

	target = GetPiece(aBoard,rank+1,file+2);
	if (target)
		if (thisPieceColor == target->itsColor)
			pieceValues += (target->itsValue == kKingValue) ? 0 : target->itsValue;

	target = GetPiece(aBoard,rank-2,file-1);
	if (target)
		if (thisPieceColor == target->itsColor)
			pieceValues += (target->itsValue == kKingValue) ? 0 : target->itsValue;

	target = GetPiece(aBoard,rank-2,file+1);
	if (target)
		if (thisPieceColor == target->itsColor)
			pieceValues += (target->itsValue == kKingValue) ? 0 : target->itsValue;

	target = GetPiece(aBoard,rank+2,file-1);
	if (target)
		if (thisPieceColor == target->itsColor)
			pieceValues += (target->itsValue == kKingValue) ? 0 : target->itsValue;

	target = GetPiece(aBoard,rank+2,file+1);
	if (target)
		if (thisPieceColor == target->itsColor)
			pieceValues += (target->itsValue == kKingValue) ? 0 : target->itsValue;

	return totalValue + (pieceValues >> 10);
}