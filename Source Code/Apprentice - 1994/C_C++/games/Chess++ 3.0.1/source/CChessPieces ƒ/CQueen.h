/****
 * CQueen.h
 *
 *	Class representing the Queen.
 *
 *	Copyright � 1993 Steven J. Bushell. All rights reserved.
 *
 ****/

#define	_H_CQueen			/* Include this file only once */
#include "CObject.h" 

class CQueen : public CChessPiece {
public:
	void		IQueen(Boolean);
	void		Draw(short rank, short file);
	CIconHandle	GetCicnHandle(void);
	Boolean		IsValidMove(CChessBoard *aBoard, short newRank, short newFile);
};