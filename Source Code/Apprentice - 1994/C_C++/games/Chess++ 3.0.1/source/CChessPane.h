/****
 * CChessPane.h
 *
 *	Class representing the Chess Board pane and its contents.
 *
 *	Copyright © 1993 Steven J. Bushell. All rights reserved.
 *
 ****/

#pragma once

#include <CPane.h>
#include "CChessPiece.h"

struct CChessPane : public CPane {

	CChessBoard	*itsChessBoard;

									/** Contruction/Destruction **/
	void		IChessPane(CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing);

									/** Drawing **/
	void		Draw(Rect *area);
	void		DrawSelectedSquare(void);
	void		ShowSelectedSquare(void);
	void		ShowMove(short rank, short file);

									/** Mouse **/
	void		DoClick(Point hitPt, short modifierKeys, long when);
	void 		RegisterMove(short rank, short file);
	Boolean		HitSamePart(Point pointA, Point pointB);
	
									/** Cursor **/
	void		AdjustCursor(Point where, RgnHandle mouseRgn);
};

pascal	void	TrackPiece(void);	// routine to track piece while dragging