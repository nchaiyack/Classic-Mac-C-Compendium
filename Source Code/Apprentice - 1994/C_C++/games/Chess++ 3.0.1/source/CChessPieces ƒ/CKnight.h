/****
 * CKnight.h
 *
 *	Class representing the Knight.
 *
 *	Copyright � 1993 Steven J. Bushell. All rights reserved.
 *
 ****/

#define	_H_CKnight			/* Include this file only once */
#include "CObject.h" 

class CKnight : public CChessPiece {
public:
	void		IKnight(Boolean);
	void		Draw(short rank, short file);
	CIconHandle	GetCicnHandle(void);
	short		BoardLocationValue(CChessBoard *aBoard, short rank, short file);
	Boolean		IsValidMove(CChessBoard *aBoard, short newRank, short newFile);
};