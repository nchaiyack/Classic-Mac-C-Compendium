////////////
//
//	ChessGlobals.cp
//
//
//	Global variables specific to Chess
//
////////////

#include "CBrain.h"
 
CChessBoard	*gChessBoard;
CBrain		*gBrain;

RgnHandle	gChessBoardRgnHandle;

CIconHandle	gWhitePawnCicnHandle;
CIconHandle	gWhiteKnightCicnHandle;
CIconHandle	gWhiteBishopCicnHandle;
CIconHandle	gWhiteRookCicnHandle;
CIconHandle	gWhiteQueenCicnHandle;
CIconHandle	gWhiteKingCicnHandle;
CIconHandle	gBlackPawnCicnHandle;
CIconHandle	gBlackKnightCicnHandle;
CIconHandle	gBlackBishopCicnHandle;
CIconHandle	gBlackRookCicnHandle;
CIconHandle	gBlackQueenCicnHandle;
CIconHandle	gBlackKingCicnHandle;

			// cursor for selecting piece to move
Cursor		gChessCursor = { 0x0000, 0x0000, 0x0000, 0x0000,
							0x0100, 0x0100, 0x0100, 0x0fe0,
							0x0100, 0x0100, 0x0100, 0x0000,
							0x0000, 0x0000, 0x0000, 0x0000,
							0x0000, 0x0000, 0x0000, 0x0100,
							0x0380, 0x0380, 0x0fe0, 0x1ff0,
							0x0fe0, 0x0380, 0x0380, 0x0100,
							0x0000, 0x0000, 0x0000, 0x0000,
							0x0007, 0x0007},

			// cursor for selecting square in which to move
			gScopeCursor = { 0x0100, 0x07c0, 0x1930, 0x2108,
							0x2108, 0x4104, 0x4104, 0xfffe,
							0x4104, 0x4104, 0x2108, 0x2108,
							0x1930, 0x07c0, 0x0100, 0x0000,
							0x0100, 0x07c0, 0x1930, 0x2108,
							0x2108, 0x4104, 0x4104, 0xfffe,
							0x4104, 0x4104, 0x2108, 0x2108,
							0x1930, 0x07c0, 0x0100, 0x0000,
							0x0007, 0x0007};