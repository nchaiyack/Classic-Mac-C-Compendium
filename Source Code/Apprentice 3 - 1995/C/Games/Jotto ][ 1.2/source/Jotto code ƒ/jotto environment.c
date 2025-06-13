#include "jotto environment.h"

static	Boolean			gIsEndGame;
static	Boolean			gGameInProgress;
static	Boolean			gFiveLetterOK;
static	Boolean			gSixLetterOK;
static	Boolean			gFiveLetterCustomOK;
static	Boolean			gSixLetterCustomOK;
static	Boolean			gFiveLetterCustomSaveOK;
static	Boolean			gSixLetterCustomSaveOK;

void SetEndGame(Boolean theEnd)
{
	gIsEndGame=theEnd;
}

Boolean EndGameShowingQQ(void)
{
	return gIsEndGame;
}

void SetGameInProgress(Boolean inProgress)
{
	gGameInProgress=inProgress;
}

Boolean GameInProgressQQ(void)
{
	return gGameInProgress;
}

void SetDictionaryFlags(Boolean fiveLetterOK, Boolean sixLetterOK,
	Boolean fiveLetterCustomOK, Boolean sixLetterCustomOK,
	Boolean fiveLetterCustomSaveOK, Boolean sixLetterCustomSaveOK)
{
	gFiveLetterOK=fiveLetterOK;
	gSixLetterOK=sixLetterOK;
	gFiveLetterCustomOK=fiveLetterCustomOK;
	gSixLetterCustomOK=sixLetterCustomOK;
	gFiveLetterCustomSaveOK=fiveLetterCustomSaveOK;
	gSixLetterCustomSaveOK=sixLetterCustomSaveOK;
}

void SetFiveLetterCustomSave(Boolean isOK)
{
	gFiveLetterCustomSaveOK=isOK;
}

void SetSixLetterCustomSave(Boolean isOK)
{
	gSixLetterCustomSaveOK=isOK;
}

Boolean FiveLetterOKQQ(void)
{
	return gFiveLetterOK;
}

Boolean SixLetterOKQQ(void)
{
	return gSixLetterOK;
}

Boolean FiveLetterCustomOKQQ(void)
{
	return gFiveLetterCustomOK;
}

Boolean SixLetterCustomOKQQ(void)
{
	return gSixLetterCustomOK;
}

Boolean FiveLetterCustomSaveOKQQ(void)
{
	return gFiveLetterCustomSaveOK;
}

Boolean SixLetterCustomSaveOKQQ(void)
{
	return gSixLetterCustomSaveOK;
}
