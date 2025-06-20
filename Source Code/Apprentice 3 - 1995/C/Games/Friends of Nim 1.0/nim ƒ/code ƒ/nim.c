#include "nim.h"
#include "nim globals.h"
#include "nim dispatch.h"
#include "nim main window.h"
#include "sound layer.h"
#include "window layer.h"
#include "error.h"
#include "program globals.h"
#include "environment.h"
#include <Icons.h>

Boolean			gDisplayMessages, gUseAnimation, gMisere;
short			gGameStatus, gGameType;

Boolean			gPlayer;
short			gPlayerMode;

Handle			gNimBitIconHandle, gNimBitFirstSelectedHandle, gNimBitSecondSelectedHandle;

short			gNimNumRows, gNimMaxPerRow, gNimDistribution;
short			gPrimeNumRows, gPrimeMaxPerRow, gPrimeDistribution;
short			gColumnsNumRows, gColumnsNumColumns, gColumnsDistribution;
short			gTurnNumRows, gTurnNumColumns, gTurnDistribution;
short			gSilverNumPlaces, gSilverNumTiles, gSilverDistribution;
GameStuffRec	gCornerInfo[4];

void InitGame(void)
{
	short			i;
	
	GetIconSuite(&gNimBitIconHandle, kNimBitIconID, svAllSmallData);
	GetIconSuite(&gNimBitFirstSelectedHandle, kNimBitFirstSelectedID, svAllAvailableData);
	GetIconSuite(&gNimBitSecondSelectedHandle, kNimBitSecondSelectedID, svAllAvailableData);
	for (i=0; i<kNumGames; i++)
		InitGameDispatch(i);
}

void NewGame(Boolean isSameGame)
{
	WindowRef		theWindow;
	
	theWindow=GetIndWindowRef(kMainWindow);
	if (!isSameGame)
	{
		if (theWindow!=0L)
			CloseTheWindow(theWindow);
	}
	else
	{
		if (theWindow!=GetFrontDocumentWindow())
			MySelectWindow(theWindow);
		SetWindowOffscreenNeedsUpdate(theWindow, TRUE);
	}
	gGameStatus=kGameInProgress;
	gPlayer=kFirstPlayer;
	OpenTheIndWindow(kMainWindow, kOpenOldIfPossible);
	if ((theWindow=GetIndWindowRef(kMainWindow))==0L)
		HandleError(kNoMemory, FALSE, FALSE);
	else
	{
		NewGameDispatch(gGameType, theWindow);
		UpdateTheWindow(theWindow);
		DoSound(sound_startgame, TRUE);
	}
}
