/****
 * CPawn.h
 *
 *	Class representing the pawn.
 *
 *	Copyright � 1993 Steven J. Bushell. All rights reserved.
 *
 ****/

#define	_H_CPawn			/* Include this file only once */
#include "CObject.h" 

class CChessBoard;

class CPawn : public CChessPiece {
public:
	void		IPawn(Boolean);
	void		Draw(short rank, short file);
	CIconHandle	GetCicnHandle(void);
	short		BoardLocationValue(CChessBoard *aBoard, short rank, short file);
	Boolean		IsValidMove(CChessBoard *aBoard, short newRank, short newFile);
	void		RegisterMove(short rank, short file);
};