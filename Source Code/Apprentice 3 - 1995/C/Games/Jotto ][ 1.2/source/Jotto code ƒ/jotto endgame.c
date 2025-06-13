#include "jotto endgame.h"
#include "jotto graphics.h"
#include "jotto environment.h"
#include "sounds.h"
#include "dialogs.h"
#include "util.h"
#include "graphics.h"
#include "program globals.h"

#define loseAlert	201

void WinGame(WindowPtr theWindow)
{
	
	HighlightChar(theWindow);
	SetEndGame(TRUE);
	UpdateTheWindow(theWindow);
	DoSound(sound_wingame, TRUE);
	SetGameInProgress(FALSE);
}

void LoseGame(WindowPtr theWindow)
{
	Str255			tempStr;
	
	HighlightChar(theWindow);
	DoSound(sound_losegame, TRUE);
	PositionDialog('ALRT', loseAlert);
	Mymemcpy((Ptr)gHumanWord[gNumTries], gComputerWord, gNumLetters);
	Mymemcpy((Ptr)((long)tempStr+1), (Ptr)gComputerWord, gNumLetters);
	tempStr[0]=gNumLetters;
	ParamText(tempStr,"\p","\p","\p");
	Alert(loseAlert, 0L);
	SetGameInProgress(FALSE);
	UpdateTheWindow(theWindow);
}
