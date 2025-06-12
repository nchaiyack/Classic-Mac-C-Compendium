////////////
//
//	CChessBoard.cp
//
//	Methods for implementing a chess board.
//
//  Copyright © 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <CBartender.h>
#include <CWindow.h>
#include <string.h>
#include <oops.h>
#include <global.h>
#include "CChessDoc.h"
#include "CChessBoard.h"
#include "CPawn.h"
#include "CKnight.h"
#include "CBishop.h"
#include "CRook.h"
#include "CQueen.h"
#include "CKing.h"
#include "CBrain.h"
#include "ChessCommands.h"

Rect	gRectToInvert;

extern	CBureaucrat *gGopher;
extern	CChessBoard	*gChessBoard;
extern	CBrain		*gBrain;
extern	CBartender	*gBartender;
extern	RgnHandle	gUtilRgn;
extern	RgnHandle	gChessBoardRgnHandle;
extern	Cursor		gChessCursor,gScopeCursor;
extern	CursHandle	gWatchCursor;

void CChessBoard::IChessBoard(CChessPane *theChessPane)
{
	register rank,file;

	itsChessPane = theChessPane;
	gameOver = false;
	awaitingFirstClick = true;
	myColor = White;
	theOtherPlayersColor = Black;

	for (rank=1;rank<=8;rank++)
		for(file=1;file<=8;file++)
		theBoard[rank][file] = (CChessPiece *)0l;
		
	for (rank=1;rank<=8;rank++)
	{
		theBoard[rank][2] = new CPawn;
			((CPawn *)theBoard[rank][2])->IPawn(Black);
		theBoard[rank][7] = new CPawn;
			((CPawn *)theBoard[rank][7])->IPawn(White);
	}

	theBoard[1][1] = new CRook;
	((CRook *)theBoard[1][1])->IRook(Black);
	theBoard[8][1] = new CRook;
	((CRook *)theBoard[8][1])->IRook(Black);
	theBoard[1][8] = new CRook;
	((CRook *)theBoard[1][8])->IRook(White);
	theBoard[8][8] = new CRook;
	((CRook *)theBoard[8][8])->IRook(White);

	theBoard[2][1] = new CKnight;
	((CKnight *)theBoard[2][1])->IKnight(Black);
	theBoard[7][1] = new CKnight;
	((CKnight *)theBoard[7][1])->IKnight(Black);
	theBoard[2][8] = new CKnight;
	((CKnight *)theBoard[2][8])->IKnight(White);
	theBoard[7][8] = new CKnight;
	((CKnight *)theBoard[7][8])->IKnight(White);

	theBoard[3][1] = new CBishop;	
	((CBishop *)theBoard[3][1])->IBishop(Black);
	theBoard[6][1] = new CBishop;
	((CBishop *)theBoard[6][1])->IBishop(Black);
	theBoard[3][8] = new CBishop;
	((CBishop *)theBoard[3][8])->IBishop(White);
	theBoard[6][8] = new CBishop;
	((CBishop *)theBoard[6][8])->IBishop(White);


	theBoard[4][1] = new CQueen;
	((CQueen *)theBoard[4][1])->IQueen(Black);
	theBoard[4][8] = new CQueen;
	((CQueen *)theBoard[4][8])->IQueen(White);
	theBoard[5][1] = new CKing;
	((CKing *)theBoard[5][1])->IKing(Black);
	RegisterKingLocation(Black,5,1);
	theBoard[5][8] = new CKing;
	((CKing *)theBoard[5][8])->IKing(White);
	RegisterKingLocation(White,5,8);
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

void CChessBoard::DoClick(Point hitPt, short modifierKeys, long when)

{
	register rank = (hitPt.h >> 5) + 1, file = (hitPt.v >> 5) + 1;
	short	theColor;
	
	if (gameOver)
	{
		SysBeep(0);
		return;
	}
	
	if (theBoard[rank][file])
		theColor = theBoard[rank][file]->itsColor;
	else
		theColor = NoColor;

	if (awaitingFirstClick)  // first click to select piece to move
	{
		if (theColor == myColor) // make sure it's one of ours
		{
doFirst:
			firstClickRank = rank; // save its location
			firstClickFile = file;
			itsChessPane->ShowSelectedSquare();
			awaitingFirstClick = false;  // now we're awaiting the second click
			gBartender->DisableCmd(cmdSwapPlayers);
			if (StillDown())
			{
				Rect	chessLimitRect,chessSlopRect;
				CIconHandle CicnHandle;
				long	deltaPt;
				
				itsChessPane->Prepare();
				if (modifierKeys & optionKey)
				{
					ShowPossibleMoves();
					awaitingFirstClick = true;
					itsChessPane->ShowSelectedSquare();
					gBartender->EnableCmd(cmdSwapPlayers);
					return;
				}
				SetRect(&chessLimitRect,0,0,256,256);
				SetRect(&chessSlopRect,0,0,256,256);
				CicnHandle = theBoard[rank][file]->GetCicnHandle();
				BitMapToRegion(gUtilRgn,&(*CicnHandle)->iconMask);
				OffsetRgn(gUtilRgn,(rank-1) << 5,(file-1) << 5);
				SetRect(&gRectToInvert,0,0,0,0);
				deltaPt = DragGrayRgn(gUtilRgn,hitPt,&chessLimitRect,&chessSlopRect,0,&TrackPiece);
				InvertRect(&gRectToInvert);
				if (deltaPt == 0x80008000)
				{
					// move aborted by dragging off chess board
					awaitingFirstClick = true;
					itsChessPane->ShowSelectedSquare();
					gBartender->EnableCmd(cmdSwapPlayers);
					return;
				}
				else
				{
					rank = ((hitPt.h + LoShort(deltaPt)) >> 5) + 1;
					file = ((hitPt.v + HiShort(deltaPt)) >> 5) + 1;
					if ((rank != firstClickRank) || (file != firstClickFile))
					{
						theColor = theBoard[rank][file] ? theBoard[rank][file]->itsColor : NoColor;
						if ((theColor != myColor) &&
							(theBoard[firstClickRank][firstClickFile]->
							IsValidMove(this,rank,file)))
						{
							if (CheckForCheck(rank,file))
							{
								InitCursor();
								NoteAlert(CheckNoteAlert,0);
							}
							else
							{
								RegisterMove(rank,file);
								itsChessPane->ShowMove(rank,file);
								awaitingFirstClick = true;
								gBartender->EnableCmd(cmdSwapPlayers);
								gBrain->isBrainsMove = true;
								((CDocument *)itsChessPane->itsSupervisor)->dirty = true;
								gBrain->BecomeGopher(true);
							}
						}
					}
					else
						;
				}
			}
		}
	}
	else // second click to select square to move to, and/or piece to capture
	{
		if ((rank == firstClickRank) && (file == firstClickFile))
		{
			// same square -- reset selection
			awaitingFirstClick = true;
			itsChessPane->ShowSelectedSquare();
			gBartender->EnableCmd(cmdSwapPlayers);
			goto doFirst;
			return;
		}
		if ((theColor != myColor) &&
			(theBoard[firstClickRank][firstClickFile]->
			IsValidMove(this,rank,file)))
		{
			if (CheckForCheck(rank,file))
			{
				InitCursor();
				NoteAlert(CheckNoteAlert,0);
			}
			else
			{
				RegisterMove(rank,file);
				itsChessPane->ShowMove(rank,file);
				awaitingFirstClick = true;
				gBartender->EnableCmd(cmdSwapPlayers);
				gBrain->isBrainsMove = true;
				((CDocument *)itsChessPane->itsSupervisor)->dirty = true;
				gBrain->BecomeGopher(true);
			}
		}
		if ((theColor == myColor) &&
			((rank != firstClickRank) || (file != firstClickFile)))
		{
			itsChessPane->DrawSelectedSquare();
		 goto doFirst;
		}
	}
}


void CChessBoard::ShowPossibleMoves(void)
{
	register possibleRank,possibleFile,theColor;
	Rect aRect;

	for(possibleFile=1;possibleFile<9;possibleFile++)
		for(possibleRank=1;possibleRank<9;possibleRank++)
			if (theBoard[firstClickRank][firstClickFile])
				if ((possibleFile != firstClickFile) || (possibleRank != firstClickRank)) {
					theColor = theBoard[possibleRank][possibleFile] ?
									theBoard[possibleRank][possibleFile]->itsColor :
									NoColor;
					if ((theColor != myColor) &&
						(theBoard[firstClickRank][firstClickFile]->
						IsValidMove(this,possibleRank,possibleFile)))
					{
						SetRect(&aRect,(possibleRank-1) << 5, (possibleFile-1) << 5,
								possibleRank << 5, possibleFile << 5);
								
						InvertRect(&aRect);
					}
				}

	while(StillDown())
		;

	for(possibleFile=1;possibleFile<9;possibleFile++)
		for(possibleRank=1;possibleRank<9;possibleRank++)
			if (theBoard[firstClickRank][firstClickFile])
				if ((possibleFile != firstClickFile) || (possibleRank != firstClickRank)) {
					theColor = theBoard[possibleRank][possibleFile] ?
									theBoard[possibleRank][possibleFile]->itsColor :
									NoColor;
					if ((theColor != myColor) &&
						(theBoard[firstClickRank][firstClickFile]->
						IsValidMove(this,possibleRank,possibleFile)))
					{
						SetRect(&aRect,(possibleRank-1) << 5, (possibleFile-1) << 5,
								possibleRank << 5, possibleFile << 5);
								
						InvertRect(&aRect);
					}
				}
}


void CChessBoard::RegisterMove(short rank, short file)
{
	CChessPiece *aPiece = theBoard[rank][file],
				*myPiece = theBoard[firstClickRank][firstClickFile];
	theBoard[rank][file] = myPiece;
	theBoard[firstClickRank][firstClickFile] = (CChessPiece *)0L;
	
	if (myPiece->itsValue == kKingValue)
		RegisterKingLocation(myPiece->itsColor,rank,file);
	
	lastFirstClickRank = firstClickRank;
	lastFirstClickFile = firstClickFile;
	lastSecondClickRank = rank;
	lastSecondClickFile = file;
}


void CChessBoard::RegisterKingLocation(short theColor, short rank, short file)
{
	if (theColor == White)
	{
		whiteKingRank = rank;
		whiteKingFile = file;
	}
	else
	{
		blackKingRank = rank;
		blackKingFile = file;
	}
}


pascal void TrackPiece(void)
{
	Point	thePt;
	register rank,file;
	short	pieceColor=NoColor,thisFirstClickRank,thisFirstClickFile;
	static short	oldHiliteRank,oldHiliteFile;

	GetMouse(&thePt);
	rank = (thePt.h >> 5) + 1;
	file = (thePt.v >> 5) + 1;
	
	if ((rank<1) || (file<1))	// escape if we're off the board while tracking
	{
		SetCursor(&arrow);
		InvertRect(&gRectToInvert);
		SetRect(&gRectToInvert,0,0,0,0);
		return;
	}

	if ((rank>8) || (file>8))
	{
		SetCursor(&arrow);
		InvertRect(&gRectToInvert);
		SetRect(&gRectToInvert,0,0,0,0);
		return;
	}
	
	if (EmptyRect(&gRectToInvert))
	{
		oldHiliteRank = 0;
		oldHiliteFile = 0;
	}
	
	if (gChessBoard->theBoard[rank][file])
		pieceColor = gChessBoard->theBoard[rank][file]->itsColor;
	
	thisFirstClickRank = gChessBoard->firstClickRank;
	thisFirstClickFile = gChessBoard->firstClickFile;
	if ((pieceColor != gChessBoard->myColor) &&
		(gChessBoard->theBoard[thisFirstClickRank][thisFirstClickFile]->
		IsValidMove(gChessBoard,rank,file)))
	{
		SetCursor(&gScopeCursor);
		if ((oldHiliteRank != rank) || (oldHiliteFile != file))
		{
			InvertRect(&gRectToInvert);
			SetRect(&gRectToInvert,(rank-1) << 5,(file-1) << 5,rank << 5,file << 5);
			InvertRect(&gRectToInvert);
			oldHiliteRank = rank;
			oldHiliteFile = file;
		}
	}
	else
	{
		SetCursor(&gChessCursor);
		InvertRect(&gRectToInvert);
		SetRect(&gRectToInvert,0,0,0,0);
	}
}

Boolean	CChessBoard::CheckForCheck(short secondClickRank, short secondClickFile)
{
	Boolean	check = false;
	short	testFile,testRank,
			savedFirstClickRank = firstClickRank,
			savedFirstClickFile = firstClickFile;
	CChessPiece	*testPiece,*savedPiece;
	short		testPieceColor;

	savedPiece = theBoard[secondClickRank][secondClickFile];
	theBoard[secondClickRank][secondClickFile] = theBoard[firstClickRank][firstClickFile];
	theBoard[firstClickRank][firstClickFile] = NULL;

	// swap colors on virtual board to think like opponent
	theOtherPlayersColor = myColor;
	myColor = (myColor == White) ? Black : White;

	for(testFile=1;testFile<=8;testFile++)
		for(testRank=1;testRank<=8;testRank++) 
		{
			testPiece = theBoard[testRank][testFile];
			testPieceColor = (testPiece) ? testPiece->itsColor : NoColor;

			if (testPieceColor == myColor)
			{
				register short	secondTestRank, secondTestFile;
				CChessPiece		*targetPiece;
				short			targetPieceColor;

				firstClickRank = testRank;
				firstClickFile = testFile;
		
				for(secondTestFile=1;secondTestFile<=8;secondTestFile++)
					for(secondTestRank=1;secondTestRank<=8;secondTestRank++)
					{
						targetPiece = theBoard[secondTestRank][secondTestFile];
						if (!targetPiece)
							continue;

						targetPieceColor = targetPiece->itsColor;

						if ((targetPieceColor == theOtherPlayersColor) &&
							!((testRank == secondTestRank) && (testFile == secondTestFile)) &&
							(testPiece->IsValidMove(this,secondTestRank,secondTestFile)))
						{
							if (targetPiece->itsValue == kKingValue)
							{
								check = true;
								goto EXIT;
							}
						}
					}
			}
		}

EXIT:
	theOtherPlayersColor = myColor;
	myColor = (myColor == White) ? Black : White;

	firstClickRank = savedFirstClickRank;
	firstClickFile = savedFirstClickFile;
	
	theBoard[firstClickRank][firstClickFile] = theBoard[secondClickRank][secondClickFile];
	theBoard[secondClickRank][secondClickFile] = savedPiece;
	
	return check;
}