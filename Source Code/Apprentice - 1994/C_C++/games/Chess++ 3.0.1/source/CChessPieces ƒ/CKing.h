/****
 * CKing.h
 *
 *	Class representing the King.
 *
 *	Copyright � 1993 Steven J. Bushell. All rights reserved.
 *
 ****/

#define	_H_CBishop			/* Include this file only once */
#include "CObject.h" 

class CKing : public CChessPiece {
public:
	Boolean		canCastle;

	void		IKing(Boolean);
	void		Draw(short rank, short file);
	CIconHandle	GetCicnHandle(void);
	Boolean		IsValidMove(CChessBoard *aBoard, short newRank, short newFile);
	void		RegisterMove(short rank, short file);
};