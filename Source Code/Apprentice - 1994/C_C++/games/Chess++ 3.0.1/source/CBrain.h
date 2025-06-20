/******************************************************************************
 CBrain.h

		Interface for the Brain Class
		
		Copyright � 1993 Steven J. Bushell. All rights reserved.

 ******************************************************************************/
 
#define _H_CBrain

#include "CBureaucrat.h"				/* Interface for its superclass		*/

CLASS CChessBoard;
//CLASS CVirtualBoard;

struct CBrain : CBureaucrat {

	Boolean		isBrainsMove;	// true if it's the Brain's turn to move
	Boolean		isThinking;		// true if Brain is thinking
	Boolean		abortMove;		// true if we're going to swap players

	CChessBoard		*theRealBoard;		// the current 'real' board
	CChessBoard		*theVirtualBoard;	// the current virtual board

	// The Brain's characteristics
	short		theBrainsColor;
	short		searchDepth;	// depth of Brain's thinking
	Boolean		soundOnBetterMoves;
	Boolean		showContemplatedMoves;
	Boolean		backPropagation;	//	variable to make options dialog look good
	long		backgroundTimeInterval;
	long		backgroundTimeIntervalTimer;
	
	// instance variables used to reenter Brain multiple times without
	// losing train of thought.
	long		bestMoveValue;
	short		testRank, testFile;
	short		bestFirstClickRank, bestFirstClickFile;
	short		bestSecondClickRank, bestSecondClickFile;

	// The Brain's methods
	void 			IBrain(CBureaucrat	*aSupervisor);
	void			Dawdle(long	*maxSleep);
	void			Think(void);
	void			ClearThoughts(void);
	short			FindMoveValue(CChessBoard *theRealBoard,
					short secondTestRank, short secondTestFile);
	short			FindBestVirtualResponse(CChessBoard *theWorkingBoard,
						short theColor, short secondClickRank,
						short secondClickFile, short theSearchDepth);
	Boolean			CheckForMate(void);
	Boolean			CheckForStalemate(void);
	CChessBoard		*CopyBoard(CChessBoard *theBoardToCopy);
	void			DisposeBoard(CChessBoard *theBoardToDispose);
	void			UpdateMenus(void);
};