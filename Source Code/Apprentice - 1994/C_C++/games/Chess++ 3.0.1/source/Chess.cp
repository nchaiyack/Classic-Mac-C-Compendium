////////////
//
//	Chess.cp
//
//	This is the main entry point for the Chess++ application.
//
//	Copyright © 1993 Steven J. Bushell. All rights reserved.
//
////////////

 
#include "CChessApp.h"
#include <TCLUtilities.h>

void main()

{
	CChessApp	*ChessApp;					

	//	There's inherently a lot of recursion when searching for
	//	good moves in a chess application, so we bump up the stack
	SetMinimumStack(64*1024);

	ChessApp = new CChessApp;
	
	ChessApp->IChessApp();
	ChessApp->Run();
	ChessApp->Exit();
}
