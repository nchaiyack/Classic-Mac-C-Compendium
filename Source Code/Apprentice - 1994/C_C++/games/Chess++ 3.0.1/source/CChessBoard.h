/****
 * CChessBoard.h
 *
 *	Class representing the Chess Board and its contents.
 *
 *	Copyright © 1993 Steven J. Bushell. All rights reserved.
 *
 ****/

//#define	_H_CChessBoard			/* Include this file only once */
#pragma once

#include "CChessPane.h"

enum { NoColor, Black, White };

class CChessBoard : public CObject {

	CChessPane	*itsChessPane;
	CChessPiece *theBoard[10][10];
	Boolean		gameOver;
	Boolean		awaitingFirstClick;
	short		firstClickRank,firstClickFile;
	short		lastFirstClickRank,lastFirstClickFile;
	short		lastSecondClickRank,lastSecondClickFile;
	short		myColor,theOtherPlayersColor;
	short		whiteKingRank,whiteKingFile,blackKingRank,blackKingFile;

									/** Contruction/Destruction **/
	void			IChessBoard(CChessPane *theChessPane);

									/** Mouse **/
	void			DoClick(Point hitPt, short modifierKeys, long when);
	void			ShowPossibleMoves(void);
	virtual void 	RegisterMove(short rank, short file);
	void			RegisterKingLocation(short theColor, short rank, short file);
	Boolean			CheckForCheck(short rank, short file);
};

pascal	void	TrackPiece(void);	// routine to track piece while dragging