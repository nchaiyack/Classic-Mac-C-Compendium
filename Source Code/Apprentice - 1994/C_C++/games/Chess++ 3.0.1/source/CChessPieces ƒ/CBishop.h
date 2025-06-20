/****
 * CBishop.h
 *
 *	Class representing the Bishop.
 *
 *	Copyright � 1993 Steven J. Bushell. All rights reserved.
 *
 ****/

#define	_H_CBishop			/* Include this file only once */
#include "CObject.h" 

class CBishop : public CChessPiece {
public:
	void		IBishop(Boolean);
	void		Draw(short rank, short file);
	CIconHandle	GetCicnHandle(void);
	Boolean		IsValidMove(CChessBoard *aBoard, short newRank, short newFile);
};