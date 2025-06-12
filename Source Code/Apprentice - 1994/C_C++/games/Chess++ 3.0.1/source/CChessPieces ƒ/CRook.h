/****
 * CRook.h
 *
 *	Class representing the Rook.
 *
 *	Copyright © 1993 Steven J. Bushell. All rights reserved.
 *
 ****/

#define	_H_CRook			/* Include this file only once */
#include "CObject.h" 

class CRook : public CChessPiece {
public:
	Boolean		canCastle;

	void		IRook(Boolean);
	void		Draw(short rank, short file);
	CIconHandle	GetCicnHandle(void);
	Boolean		IsValidMove(CChessBoard *aBoard, short newRank, short newFile);
	void		RegisterMove(short rank, short file);
};