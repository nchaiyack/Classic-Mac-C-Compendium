#include "nim endgame.h"
#include "nim globals.h"
#include "turbo peano.h"
#include "offscreen layer.h"
#include "sound layer.h"
#include "window layer.h"
#include "hgr.h"

#define kWinnerStringID		302		/* res ID of winner strings */

void EndGame(WindowRef theWindow)
{
	MyOffscreenPtr	offscreenWorldPtr;
	short			theDepth;
	Boolean			computerWins;
	
	theDepth=GetWindowDepth(theWindow);
	offscreenWorldPtr=GetWindowPermanentOffscreenWorld(theWindow);
	SetPortToOffscreenWindow(offscreenWorldPtr);
	
	DrawPeano(GetWindowDepth(theWindow));
	SetPortToOnscreenWindow(theWindow, offscreenWorldPtr);
	if (gUseAnimation)
	{
		HGRFade(&(theWindow->portRect));
		HGRReversed(offscreenWorldPtr->offscreenPtr, (GrafPtr)theWindow, &(theWindow->portRect));
	}
	gPlayer=!gPlayer;
	computerWins=IsComputersTurnQQ()^gMisere;
	DoSound(((gPlayerMode==kMacMacMode) || (gPlayerMode==kHumanHumanMode)) ? sound_wingame :
			computerWins ? sound_losegame : sound_wingame, TRUE);
	gGameStatus=kShowingEndGame;
	SetWindowOffscreenNeedsUpdate(theWindow, TRUE);
	UpdateTheWindow(theWindow);
}

Boolean IsComputersTurnQQ(void)
{
	return (gPlayerMode==kMacMacMode) || ((gPlayerMode==kHumanMacMode) && (gPlayer==kSecondPlayer))
		|| ((gPlayerMode==kMacHumanMode) && (gPlayer==kFirstPlayer));
}

void GetWinnerString(Str255 theStr)
{
	if (gMisere)
		GetIndString(theStr, kWinnerStringID,
			(gPlayerMode==kMacMacMode) ? ((gPlayer==kFirstPlayer) ? 2 : 1) :
			(gPlayerMode==kHumanHumanMode) ? ((gPlayer==kFirstPlayer) ? 5 : 4) :
			(IsComputersTurnQQ()) ? 6 : 3);
	else
		GetIndString(theStr, kWinnerStringID,
			(gPlayerMode==kMacMacMode) ? ((gPlayer==kFirstPlayer) ? 1 : 2) :
			(IsComputersTurnQQ()) ? 3 :
			(gPlayerMode==kHumanHumanMode) ? ((gPlayer==kFirstPlayer) ? 4 : 5) : 6);
}
