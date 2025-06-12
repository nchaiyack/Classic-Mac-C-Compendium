/****
 * CChessPiece.h
 *
 *	Abstract class representing the Chess peices.
 *
 *	Copyright © 1993 Steven J. Bushell. All rights reserved.
 *
 ****/
#pragma once
#include <CObject.h> 

#define kKingValue 0x5000

CLASS CChessBoard;

class CChessPiece : public CObject {
public:
	Boolean	itsColor;
	short	itsValue;

	virtual void		IChessPiece(short aColor);
	virtual void		Draw(short rank, short file); // abstract Draw method -- must be overridden
	virtual Boolean		IsValidMove(CChessBoard *aBoard, short newRank, short newFile);
	virtual short		BoardLocationValue(CChessBoard *aBoard, short rank, short file);
	virtual CIconHandle	GetCicnHandle(void);
	virtual void		RegisterMove(short rank, short file);
};