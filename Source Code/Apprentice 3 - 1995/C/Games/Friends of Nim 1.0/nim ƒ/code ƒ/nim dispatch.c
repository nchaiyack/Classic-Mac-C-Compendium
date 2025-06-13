#include "nim dispatch.h"
#include "sound layer.h"
#include "nim globals.h"
#include "nim meat.h"
#include "prime meat.h"
#include "columns meat.h"
#include "turn meat.h"
#include "silver meat.h"
#include "queen meat.h"

void InitGameDispatch(short game)
{
	switch (game)
	{
		case kNim:
			InitNim();
			break;
		case kPrimeNim:
			InitPrime();
			break;
		case kColumnsNim:
			InitColumns();
			break;
		case kTurnablock:
			InitTurn();
			break;
		case kSilver:
			InitSilver();
			break;
		case kCornerTheQueen:
		case kCornerTheKing:
		case kCornerTheSuperqueen:
		case kCornerTheSuperking:
			InitCorner();
			break;
	}
}

void NewGameDispatch(short game, WindowRef theWindow)
{
	switch (game)
	{
		case kNim:
			InitNimOneGame();
			break;
		case kPrimeNim:
			InitPrimeOneGame();
			break;
		case kColumnsNim:
			InitColumnsOneGame();
			break;
		case kTurnablock:
			InitTurnOneGame();
			break;
		case kSilver:
			InitSilverOneGame();
			break;
		case kCornerTheQueen:
		case kCornerTheKing:
		case kCornerTheSuperqueen:
		case kCornerTheSuperking:
			InitCornerOneGame(theWindow);
			break;
	}
}

void NimDrawDispatch(short game, WindowRef theWindow, short theDepth)
{
	switch (game)
	{
		case kNim:
			NimDrawWindow(theWindow, theDepth);
			break;
		case kPrimeNim:
			PrimeDrawWindow(theWindow, theDepth);
			break;
		case kColumnsNim:
			ColumnsDrawWindow(theWindow, theDepth);
			break;
		case kTurnablock:
			TurnDrawWindow(theWindow, theDepth);
			break;
		case kSilver:
			SilverDrawWindow(theWindow, theDepth);
			break;
		case kCornerTheQueen:
		case kCornerTheKing:
		case kCornerTheSuperqueen:
		case kCornerTheSuperking:
			CornerDrawWindow(theWindow, theDepth);
			break;
	}
}

void NimClickDispatch(short game, WindowRef theWindow, Point thePoint)
{
	switch (game)
	{
		case kNim:
			NimClick(theWindow, thePoint);
			break;
		case kPrimeNim:
			PrimeClick(theWindow, thePoint);
			break;
		case kColumnsNim:
			ColumnsClick(theWindow, thePoint);
			break;
		case kTurnablock:
			TurnClick(theWindow, thePoint);
			break;
		case kSilver:
			SilverClick(theWindow, thePoint);
			break;
		case kCornerTheQueen:
		case kCornerTheKing:
		case kCornerTheSuperqueen:
		case kCornerTheSuperking:
			CornerClick(theWindow, thePoint);
			break;
	}
}

void NimIdleDispatch(short game, WindowRef theWindow)
{
	switch (game)
	{
		case kNim:
			NimIdle(theWindow);
			break;
		case kPrimeNim:
			PrimeIdle(theWindow);
			break;
		case kColumnsNim:
			ColumnsIdle(theWindow);
			break;
		case kTurnablock:
			TurnIdle(theWindow);
			break;
		case kSilver:
			SilverIdle(theWindow);
			break;
		case kCornerTheQueen:
		case kCornerTheKing:
		case kCornerTheSuperqueen:
		case kCornerTheSuperking:
			CornerIdle(theWindow);
			break;
	}
}
