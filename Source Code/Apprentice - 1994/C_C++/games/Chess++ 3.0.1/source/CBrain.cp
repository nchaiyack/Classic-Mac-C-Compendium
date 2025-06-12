////////////
//
//	CBrain.cp
//
//	The Brain Class ( or The Virtual Opponent )
//
//	Class which represents all thought processes, heuristics, and logic,
//	which are used in calculating, and responding to the real player's move.
//
//	SUPERCLASS = CBureaucrat
//	Copyright © 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <Global.h>
#include <Commands.h>
#include "ChessCommands.h"
#include "CBrain.h"
#include "CChessDoc.h"
#include <CBartender.h>
#include <CApplication.h>

#include <stdio.h>

// Global Variables
extern	CApplication		*gApplication;	/* Application object		*/
extern	CBartender		*gBartender;	/* The menu handling object */
extern	CBureaucrat		*gGopher;		/* First in line to get commands	*/
extern	CChessBoard		*gChessBoard;


void	CBrain::IBrain(CBureaucrat	*aSupervisor)
{
	isBrainsMove = false;
	isThinking = false;
	searchDepth = 1;
	soundOnBetterMoves = false;
	showContemplatedMoves = false;
	backgroundTimeInterval = 500;
										/* Initialize superclass	*/
	CBureaucrat::IBureaucrat(aSupervisor);
}


// When the boolean flag 'isBrainsMove' is true, the Brain will start processing
// the best responding move, occasionally giving time to the rest of the program.
// Once it has found the best move, it executes the move, and sets the flag
// 'isBrainsMove' back to false, and returns control back to the other player.
void	CBrain::Dawdle(long	*maxSleep)
{
	if (isBrainsMove)
	{	
		// if this is our first time through for this move, initialize
		// all Brain class variables and find best response
		if (!isThinking)
		{
			ClearThoughts();
			Think();
		}
	}
}


void	CBrain::Think(void)
{
	CChessPiece	*testPiece;
	CChessBoard	*theRealBoardCopy;
	short		testPieceColor;
	Boolean		showSelectedMoves;
	Rect			aRect;
	short		savedBSCR ,savedBSCF;

	theRealBoardCopy = (CChessBoard *)theRealBoard->Copy();

	for(testFile=1;testFile<=8;testFile++)
		for(testRank=1;testRank<=8;testRank++) 
		{
			showSelectedMoves = showContemplatedMoves;
			
			if (!isThinking)
				break;
			
			testPiece = theRealBoard->theBoard[testRank][testFile];
			testPieceColor = (testPiece) ? testPiece->itsColor : NoColor;

			if (testPieceColor == theBrainsColor) {
				register short	secondTestRank, secondTestFile;
				long			totalMoveValue;
				CChessPiece	*targetPiece;
				short		targetPieceColor;

				if (showSelectedMoves) {
					theRealBoard->itsChessPane->Prepare();
					SetRect(&aRect,(testRank-1)<<5,(testFile-1)<<5,testRank<<5,testFile<<5);
					InvertRect(&aRect);
				}

				theRealBoardCopy->firstClickRank = testRank;
				theRealBoardCopy->firstClickFile = testFile;
		
				for(secondTestFile=1;secondTestFile<=8;secondTestFile++)
					for(secondTestRank=1;secondTestRank<=8;secondTestRank++)
					{
						totalMoveValue = 0;
		
						targetPiece = theRealBoardCopy->theBoard[secondTestRank][secondTestFile];
						targetPieceColor = (targetPiece) ? targetPiece->itsColor: NoColor;

						if ((targetPieceColor != theBrainsColor) &&
							!((testRank == secondTestRank) && (testFile == secondTestFile)) &&
							(testPiece->IsValidMove(theRealBoardCopy,secondTestRank,secondTestFile)))
						{
							// find out what this particular move would be worth
							totalMoveValue = FindMoveValue(theRealBoardCopy,secondTestRank,secondTestFile);

							if (totalMoveValue > bestMoveValue)
							{
								if (soundOnBetterMoves)
									SysBeep(0);
								bestMoveValue = totalMoveValue;
								bestFirstClickRank = testRank;
								bestFirstClickFile = testFile;
								bestSecondClickRank = secondTestRank;
								bestSecondClickFile = secondTestFile;
							}
						}
					}
						
					if (showSelectedMoves)
					{
						theRealBoard->itsChessPane->Prepare();
						InvertRect(&aRect);
						InvalRect(&aRect);
					}
			}
		}

	if (!bestFirstClickRank)	// if no valid moves were found, the Brain will resign
	{
		InitCursor();
		NoteAlert(ResignNoteAlert,0L);
		theRealBoard->gameOver = true;
		goto EXIT;
	}

	// register Brain's move on the real board
	theRealBoard->firstClickRank = bestFirstClickRank;
	theRealBoard->firstClickFile = bestFirstClickFile;
	theRealBoard->RegisterMove(bestSecondClickRank,bestSecondClickFile);

	savedBSCR = bestSecondClickRank;
	savedBSCF = bestSecondClickFile;

	//	check to see if the Brain has won the game
	if (CheckForMate())
	{
		theRealBoard->gameOver = true;
		InitCursor();
		if (CheckForStalemate())
			NoteAlert(StalemateNoteAlert,0L);
		else
			NoteAlert(CheckmateNoteAlert,0L);
	}

	theRealBoard->itsChessPane->Prepare();
	theRealBoard->itsChessPane->ShowMove(savedBSCR,savedBSCF);

EXIT:
	// return control to the other player
	isBrainsMove = false;
	isThinking = false;
	theRealBoard->itsChessPane->BecomeGopher(true);
	theRealBoardCopy->Dispose();
	
	// Return application to 'normal' idling speed
	gApplication->cMaxSleepTime = 2;

	// if we're swapping players, just leave the board alone and get out
	if (abortMove)
		return;
}


// Initialize the Brain to start planning a new move.
void	CBrain::ClearThoughts(void)
{
	theRealBoard = gChessBoard;
	theBrainsColor = theRealBoard->theOtherPlayersColor;
	bestMoveValue = -0x1000;
	testRank = 1;
	testFile = 1;
	bestFirstClickRank = 0;
	bestFirstClickFile = 0;
	bestSecondClickRank = 0;
	bestSecondClickFile = 0;
	backgroundTimeIntervalTimer = 0L;
	isThinking = true;
	abortMove = false;
	
	// We want the Brain to think fast while we're waiting
	gApplication->cMaxSleepTime = 0;
}


// Begin recursion to find total worth of this particular subjunctive move
short	CBrain::FindMoveValue(CChessBoard *theRealBoard, short secondTestRank, short secondTestFile)
{
	CChessPiece 	*thePieceToMove = theRealBoard->
						theBoard[theRealBoard->firstClickRank][theRealBoard->firstClickFile],
				*theTargetPiece = theRealBoard->theBoard[secondTestRank][secondTestFile];
	short		theTargetPieceValue = 0;
	long			tempMoveValue = -0x0000;
	CChessBoard	*firstThoughtBoard;
	
	firstThoughtBoard = theRealBoard;

	if (theTargetPiece)
		theTargetPieceValue = theTargetPiece->itsValue;

	tempMoveValue += theTargetPieceValue;
		
	tempMoveValue += thePieceToMove->BoardLocationValue(firstThoughtBoard, secondTestRank,secondTestFile);

	tempMoveValue -= FindBestVirtualResponse(firstThoughtBoard,firstThoughtBoard->myColor,
										secondTestRank,secondTestFile,searchDepth);

	return tempMoveValue;
}


// Recurse virtual boards CBrain::searchDepth times, returning all best possible responses 
short	CBrain::FindBestVirtualResponse(CChessBoard *theWorkingBoard,
					short theColor, short secondClickRank,
					short secondClickFile, short theSearchDepth)
{
	CChessBoard	*aVirtualBoard;
	register		virtualTestRank,virtualTestFile,virtualSecondTestRank,virtualSecondTestFile;
	long			thisVirtualResponse,bestVirtualResponse = -0x2000;
	short		thePieceToMoveColor,theTargetPieceColor;
	CChessPiece 	*thePieceToMove,*theTargetPiece;
	CChessPiece	*savedTestPiece,*savedTargetPiece;
	short		savedFirstClickRank,savedFirstClickFile;
	short		savedLastFirstClickRank,savedLastFirstClickFile;
	short		savedLastSecondClickRank,savedLastSecondClickFile;
	short		savedMyColor,savedTheOtherPlayersColor;
	short		savedWhiteKingRank,savedWhiteKingFile,
				savedBlackKingRank,savedBlackKingFile;

	if (!theSearchDepth || abortMove)
	{
		if (++backgroundTimeIntervalTimer>backgroundTimeInterval)
		{
			backgroundTimeIntervalTimer = 0;
			gApplication->Process1Event();
		}
		return 0;
	}

	aVirtualBoard = theWorkingBoard;
	savedTestPiece = aVirtualBoard->
				theBoard[aVirtualBoard->firstClickRank][aVirtualBoard->firstClickFile];
	savedTargetPiece = aVirtualBoard->
				theBoard[secondClickRank][secondClickFile];
	savedFirstClickRank = aVirtualBoard->firstClickRank;
	savedFirstClickFile = aVirtualBoard->firstClickFile;
	savedLastFirstClickRank = aVirtualBoard->lastFirstClickRank;
	savedLastFirstClickFile = aVirtualBoard->lastFirstClickFile;
	savedLastSecondClickRank = aVirtualBoard->lastSecondClickRank;
	savedLastSecondClickFile = aVirtualBoard->lastSecondClickFile;
	savedMyColor = aVirtualBoard->myColor;
	savedTheOtherPlayersColor = aVirtualBoard->theOtherPlayersColor;
	savedWhiteKingRank = aVirtualBoard->whiteKingRank;
	savedWhiteKingFile = aVirtualBoard->whiteKingFile;
	savedBlackKingRank = aVirtualBoard->blackKingRank;
	savedBlackKingFile = aVirtualBoard->blackKingFile;

	aVirtualBoard->RegisterMove(secondClickRank,secondClickFile);

	// swap colors on virtual board to think like opponent
	aVirtualBoard-> theOtherPlayersColor = aVirtualBoard->myColor;
	aVirtualBoard->myColor = (aVirtualBoard->myColor == White) ? Black : White;

	// pick a piece to move 
	for (virtualTestFile=1;virtualTestFile<=8;virtualTestFile++)
		for (virtualTestRank=1;virtualTestRank<=8;virtualTestRank++)
		{
			thePieceToMove = aVirtualBoard->theBoard[virtualTestRank][virtualTestFile];
			if (thePieceToMove)
				thePieceToMoveColor = thePieceToMove->itsColor;
			else
				thePieceToMoveColor = NoColor;

			if (thePieceToMoveColor == aVirtualBoard->theOtherPlayersColor)
			{
				aVirtualBoard->firstClickRank = virtualTestRank;
				aVirtualBoard->firstClickFile = virtualTestFile;
				
				// pick a target piece or square to move to
				for (virtualSecondTestFile=1;virtualSecondTestFile<=8;virtualSecondTestFile++)
					for (virtualSecondTestRank=1;virtualSecondTestRank<=8;virtualSecondTestRank++)
					{
						theTargetPiece = aVirtualBoard->theBoard[virtualSecondTestRank][virtualSecondTestFile];
						theTargetPieceColor = (theTargetPiece) ? theTargetPiece->itsColor : NoColor;
						
						if((virtualTestRank == virtualSecondTestRank) && (virtualTestFile == virtualSecondTestFile))
							continue;
						
						thisVirtualResponse = (theTargetPiece) ? theTargetPiece->itsValue : 0;
						
						// if it's one of my pieces, add on its value as protection points
						if (theTargetPieceColor == aVirtualBoard->theOtherPlayersColor)
							;
						else
						// is this a valid piece/square to move to?
						if (thePieceToMove->IsValidMove(aVirtualBoard,virtualSecondTestRank,virtualSecondTestFile))
						{
							thisVirtualResponse += thePieceToMove->
										BoardLocationValue(aVirtualBoard,
											virtualSecondTestRank,virtualSecondTestFile);
					
							// the value of this response is the captured piece/square value
							// minus the responding move's value
							//	NOTE: this line is the single most important one in this entire
							//	program.  It recursively handles all of the subjunctive-move
							//	lookup calculations.  Amazing, isn't it?
							thisVirtualResponse  -= FindBestVirtualResponse(aVirtualBoard,theColor,
										virtualSecondTestRank,virtualSecondTestFile,theSearchDepth-1);
							
							if (thisVirtualResponse > bestVirtualResponse)
								bestVirtualResponse = thisVirtualResponse;
						}
					}
			}
		}

	aVirtualBoard->theBoard[savedFirstClickRank][savedFirstClickFile] = savedTestPiece;
	aVirtualBoard->theBoard[secondClickRank][secondClickFile] = savedTargetPiece;
	aVirtualBoard->firstClickRank = savedFirstClickRank;
	aVirtualBoard->firstClickFile = savedFirstClickFile;
	aVirtualBoard->lastFirstClickRank = savedLastFirstClickRank;
	aVirtualBoard->lastFirstClickFile = savedLastFirstClickFile;
	aVirtualBoard->lastSecondClickRank = savedLastSecondClickRank;
	aVirtualBoard->lastSecondClickFile = savedLastSecondClickFile;
	aVirtualBoard->myColor = savedMyColor;
	aVirtualBoard->theOtherPlayersColor = savedTheOtherPlayersColor;
	aVirtualBoard->whiteKingRank = savedWhiteKingRank;
	aVirtualBoard->whiteKingFile = savedWhiteKingFile;
	aVirtualBoard->blackKingRank = savedBlackKingRank;
	aVirtualBoard->blackKingFile = savedBlackKingFile;

	return bestVirtualResponse;
}

Boolean	CBrain::CheckForMate(void)
{
	CChessPiece	*testPiece;
	CChessBoard	*theRealBoardCopy;
	long			savedBackgroundTimeInterval = backgroundTimeInterval;
	short		testPieceColor,
				savedBrainsColor = theBrainsColor,
				savedSearchDepth = searchDepth;
	Boolean		showSelectedMoves;
	Rect			aRect;

	//	do a simplified version of ClearThoughts()
	theBrainsColor = theRealBoard->myColor;
	bestMoveValue = -0x1000;
	testRank = 1;
	testFile = 1;
	bestFirstClickRank = 0;
	bestFirstClickFile = 0;
	bestSecondClickRank = 0;
	bestSecondClickFile = 0;
	backgroundTimeInterval = 0x10000000;

	theRealBoardCopy = (CChessBoard *)theRealBoard->Copy();

	//	swap colors on virtual board to think like opponent
	theRealBoardCopy-> theOtherPlayersColor = theRealBoardCopy->myColor;
	theRealBoardCopy->myColor = (theRealBoardCopy->myColor == White) ? Black : White;

	//	prepare for superficial search for a king capture
	searchDepth = 1;

	for(testFile=1;testFile<=8;testFile++)
		for(testRank=1;testRank<=8;testRank++) 
		{
			showSelectedMoves = showContemplatedMoves;
			
			testPiece = theRealBoard->theBoard[testRank][testFile];
			testPieceColor = (testPiece) ? testPiece->itsColor : NoColor;

			if (testPieceColor == theBrainsColor) {
				register short	secondTestRank, secondTestFile;
				long			totalMoveValue;
				CChessPiece	*targetPiece;
				short		targetPieceColor;

				if (showSelectedMoves) {
					theRealBoard->itsChessPane->Prepare();
					SetRect(&aRect,(testRank-1)<<5,(testFile-1)<<5,testRank<<5,testFile<<5);
					InvertRect(&aRect);
				}

				theRealBoardCopy->firstClickRank = testRank;
				theRealBoardCopy->firstClickFile = testFile;
		
				for(secondTestFile=1;secondTestFile<=8;secondTestFile++)
					for(secondTestRank=1;secondTestRank<=8;secondTestRank++)
					{
						totalMoveValue = 0;
		
						targetPiece = theRealBoardCopy->theBoard[secondTestRank][secondTestFile];
						targetPieceColor = (targetPiece) ? targetPiece->itsColor: NoColor;

						if ((targetPieceColor != theBrainsColor) &&
							!((testRank == secondTestRank) && (testFile == secondTestFile)) &&
							(testPiece->IsValidMove(theRealBoardCopy,secondTestRank,secondTestFile)))
						{
							// find out what this particular move would be worth
							totalMoveValue = FindMoveValue(theRealBoardCopy,secondTestRank,secondTestFile);

							if (totalMoveValue > bestMoveValue)
							{
								if (soundOnBetterMoves)
									SysBeep(0);
								bestMoveValue = totalMoveValue;
								bestFirstClickRank = testRank;
								bestFirstClickFile = testFile;
								bestSecondClickRank = secondTestRank;
								bestSecondClickFile = secondTestFile;
							}
						}
					}
						
					if (showSelectedMoves)
					{
						theRealBoard->itsChessPane->Prepare();
						InvertRect(&aRect);
						InvalRect(&aRect);
					}
			}
		}

	theRealBoardCopy->Dispose();

	//	restore our original settings
	theBrainsColor = savedBrainsColor;
	searchDepth = savedSearchDepth;
	backgroundTimeInterval = savedBackgroundTimeInterval;

	if (!bestFirstClickRank)	// if no valid moves were found, the Brain has checkmated its opponent!
		return true;

	return false;	//	if we've made it this far, the game goes on...
}


Boolean	CBrain::CheckForStalemate(void)
{
	CChessPiece	*testPiece;
	CChessBoard	*theRealBoardCopy;
	long			savedBackgroundTimeInterval = backgroundTimeInterval;
	short		testPieceColor,
				savedBrainsColor = theBrainsColor,
				savedSearchDepth = searchDepth;
	Boolean		stalemate = true;

	//	do a simplified version of ClearThoughts()
	backgroundTimeInterval = 0x10000000;

	theRealBoardCopy = (CChessBoard *)theRealBoard->Copy();

	for(testFile=1;testFile<=8;testFile++)
		for(testRank=1;testRank<=8;testRank++) 
		{
			testPiece = theRealBoard->theBoard[testRank][testFile];
			testPieceColor = (testPiece) ? testPiece->itsColor : NoColor;

			if (testPieceColor == theBrainsColor) {
				register short	secondTestRank, secondTestFile;
				CChessPiece	*targetPiece;
				short		targetPieceColor;

				theRealBoardCopy->firstClickRank = testRank;
				theRealBoardCopy->firstClickFile = testFile;
		
				for(secondTestFile=1;secondTestFile<=8;secondTestFile++)
					for(secondTestRank=1;secondTestRank<=8;secondTestRank++)
					{
						targetPiece = theRealBoardCopy->theBoard[secondTestRank][secondTestFile];
						if (!targetPiece)
							continue;
						targetPieceColor = targetPiece->itsColor;

						if ((targetPieceColor != theBrainsColor) &&
							!((testRank == secondTestRank) && (testFile == secondTestFile)) &&
							(testPiece->IsValidMove(theRealBoardCopy,secondTestRank,secondTestFile)))
						{
							stalemate = false;
							goto EXIT;
						}
					}
			}
		}

	theRealBoardCopy->Dispose();

	//	restore our original settings
	backgroundTimeInterval = savedBackgroundTimeInterval;
EXIT:
	return stalemate;	//	if we've made it this far, it must have been a stalemate...
}


//	the next two functions are somewhat superfluous, but left
//	around for historical reasons
CChessBoard *CBrain::CopyBoard(CChessBoard *theBoardToCopy)
{
	CChessBoard	*theVirtualBoardCopy;

	theVirtualBoardCopy = (CChessBoard *)theBoardToCopy->Copy();
	
	return theVirtualBoardCopy;
}


void	CBrain::DisposeBoard(CChessBoard *theBoardToDispose)
{
	theBoardToDispose->Dispose();
}


void	CBrain::UpdateMenus(void)
{
	inherited::UpdateMenus();

	// If the Brain is thinking, we don't want the user interrupting
	// with anything but StopThinking, Swap Players, or the Options dialog
	gBartender->EnableCmd(cmdSwapPlayers);
	gBartender->EnableCmd(cmdStopThinking);
	gBartender->EnableCmd(cmdOptions);
}