////////////
//
//	CChessPane.cp
//
//	Pane methods for a chess board.
//
//	Copyright � 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <CBartender.h>
#include <CWindow.h>
#include <string.h>
#include <oops.h>
#include <global.h>
#include "CChessDoc.h"
#include "CChessPane.h"
#include "CChessBoard.h"
#include "CPawn.h"
#include "CKnight.h"
#include "CBishop.h"
#include "CRook.h"
#include "CQueen.h"
#include "CKing.h"
#include "CBrain.h"
#include "ChessCommands.h"

extern	CBureaucrat *gGopher;
extern	CChessBoard	*gChessBoard;
extern	CBrain		*gBrain;
extern	CBartender	*gBartender;
extern	RgnHandle	gUtilRgn;
extern	RgnHandle	gChessBoardRgnHandle;
extern	Cursor		gChessCursor,gScopeCursor;
extern	CursHandle	gWatchCursor;

void CChessPane::IChessPane(CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing)
{
	register rank,file;
	
	CPane::IPane(anEnclosure, aSupervisor, aWidth, aHeight,
							aHEncl, aVEncl, aHSizing, aVSizing);

	itsChessBoard = new CChessBoard;
	itsChessBoard->IChessBoard(this);
	gChessBoard = itsChessBoard;
		
	SetWantsClicks(true);
}


/***
 * Draw
 *
 *	In this method, you draw whatever you need to display in
 *	your pane. The area parameter gives the portion of the 
 *	pane that needs to be redrawn. Area is in frame coordinates.
 *
 ***/

void CChessPane::Draw(Rect *area)
{
	register	rank,file;
	RGBColor	color;
	Rect		aRect;
	CChessPiece	*thePiece;
	
	// Draw Chess board pattern
	color.blue = color.green = 0;
	color.red = -1;
	RGBForeColor(&color);
	PenMode(srcCopy);
	PaintRgn(gChessBoardRgnHandle);
	
	// have fun with board & pieces
	ForeColor(blackColor);
	for (rank=1;rank<=8;rank++)
		for(file=1;file<=8;file++) {
			thePiece = itsChessBoard->theBoard[rank][file];
			if (thePiece) {
				thePiece->Draw(rank,file);
			}
		}
	if (!itsChessBoard->awaitingFirstClick)
		DrawSelectedSquare();
}

void CChessPane::DrawSelectedSquare(void)
{
	register	rankQD = (itsChessBoard->firstClickRank-1) << 5,
				fileQD = (itsChessBoard->firstClickFile-1) << 5;
	Rect	aRect;

	// invert square to highlight selection
	SetRect(&aRect, rankQD, fileQD, rankQD+32, fileQD+32);
	InvertRect(&aRect);
}


void CChessPane::ShowSelectedSquare(void)
{
	register	rankQD = (itsChessBoard->firstClickRank-1) << 5,
				fileQD = (itsChessBoard->firstClickFile-1) << 5;
	Rect	aRect;

	// invalidate square to force redraw
	SetRect(&aRect, rankQD, fileQD, rankQD+32, fileQD+32);
	InvalRect(&aRect);
}

	
void CChessPane::ShowMove(short rank, short file)
{
#define DELAY 6
	register	i,
				rankQD = (itsChessBoard->firstClickRank-1) << 5,
				fileQD = (itsChessBoard->firstClickFile-1) << 5;
	Rect		aRect;
	long		dummy;

	// invalidate first square to force redraw
	SetRect(&aRect, rankQD, fileQD, rankQD+32, fileQD+32);
	for (i=0;i<6;i++)
	{
		InvertRect(&aRect);
		Delay(DELAY,&dummy);
	}
	InvalRect(&aRect);

	// invalidate second square to force redraw
	rankQD = (rank-1) << 5;
	fileQD = (file-1) << 5;
	SetRect(&aRect, rankQD, fileQD, rankQD+32, fileQD+32);
	for (i=0;i<6;i++)
	{
		InvertRect(&aRect);
		Delay(DELAY,&dummy);
	}
	InvalRect(&aRect);	
}


/***
 * DoClick
 *
 *	The mouse went down in the pane.
 *	In this method you do whatever is appropriate for your
 *	application. HitPt is given in frame coordinates. The other
 *	parameters, modiferKeys and when, are taken from the event
 *	record.
 *
 *	If you want to implement mouse tracking, this is the method
 *	to do it in. You need to create a subclass of CMouseTask and
 *	pass it in a TrackMouse() message to the pane.
 *
 ***/ 

void CChessPane::DoClick(Point hitPt, short modifierKeys, long when)

{
	if (gBrain->isBrainsMove) 	// we shouldn't do anything while
		 return;			// the computer is thinking
	else
		itsChessBoard->DoClick(hitPt, modifierKeys, when);
}


void CChessPane::RegisterMove(short rank, short file)
{
	short		firstClickRank = itsChessBoard->firstClickRank,
				firstClickFile = itsChessBoard->firstClickFile;
	CChessPiece *aPiece = itsChessBoard->theBoard[rank][file],
				*myPiece = itsChessBoard->theBoard[firstClickRank][firstClickFile];
				
	if (aPiece)
		aPiece->Dispose();
	itsChessBoard->theBoard[rank][file] = myPiece;
	itsChessBoard->theBoard[firstClickRank][firstClickFile] = (CChessPiece *)0L;
	
	itsChessBoard->lastFirstClickRank = firstClickRank;
	itsChessBoard->lastFirstClickFile = firstClickFile;
	itsChessBoard->lastSecondClickRank = rank;
	itsChessBoard->lastSecondClickFile = file;
	
	myPiece->RegisterMove(rank,file);
}

/***
 * HitSamePart
 *
 *	Test whether pointA and pointB are in the same part.
 *	"The same part" means different things for different applications.
 *	In the default method, "the same part" means "in the same pane."
 *	If you want a different behavior, override this method. For instance,
 *	two points might be in the same part if they're within n pixels
 *  of each other.
 *
 *	PointA and pointB are both in frame coordinates.
 *
 ***/

Boolean	 CChessPane::HitSamePart(Point pointA, Point pointB)

{
	return inherited::HitSamePart(pointA, pointB);
}


/***
 * AdjustCursor
 *
 *	If you want the cursor to have a different shape in your pane,
 *	do it in this method. If you want a different cursor for different
 *	parts of the same pane, you'll need to change the mouseRgn like this:
 *		1. Create a region for the "special area" of your pane.
 *		2. Convert this region to global coordinates
 *		3. Set the mouseRgn to the intersection of this region
 *		   and the original mouseRgn: SectRgn(mouseRgn, myRgn, mouseRgn);
 *
 *	The default method just sets the cursor to the arrow. If this is fine
 *	for you, don't override this method.
 *
 ***/

void CChessPane::AdjustCursor(Point where, RgnHandle mouseRgn)

{
	register	rank,file;
	short		pieceColor = NoColor,myColor = itsChessBoard->myColor;
	
	// if the Brain is thinking, set the cursor to a watch and get out
	if (gBrain->isBrainsMove)
	{
		SetCursor(*gWatchCursor);
		return;
	}
	
	rank = (where.h >> 5) + 1;
	file = (where.v >> 5) + 1;
	
	if (itsChessBoard->theBoard[rank][file])
		pieceColor = itsChessBoard->theBoard[rank][file]->itsColor;

	if (itsChessBoard->awaitingFirstClick)
	{
		if (pieceColor == myColor)
			SetCursor(&gChessCursor);
		else
			inherited::AdjustCursor(where, mouseRgn);
	}
	else
	{
		if ((pieceColor != myColor) &&
			(itsChessBoard->
				theBoard[itsChessBoard->firstClickRank][itsChessBoard->firstClickFile]->
					IsValidMove(itsChessBoard,rank,file)))
				SetCursor(&gScopeCursor);
		else
			inherited::AdjustCursor(where, mouseRgn);
	}
}