#include "program globals.h"
#include "jotto.h"
#include "jotto keys.h"
#include "jotto meat.h"
#include "jotto load-save.h"
#include "jotto dictionary.h"
#include "jotto environment.h"
#include "error.h"
#include "graphics.h"
#include "window layer.h"

#define NUMWIPES 54

char			gComputerWord[6];		/* what it really is */
char			gHumanWord[MAX_TRIES+1][6];	/* n-th word guess */
char			gNumRight[MAX_TRIES];	/* # right in n-th try */
unsigned char	gNumTries;				/* # tries so far */
short			gNumComputerWords[2];	/* # total words in computer (5- or 6-letter) dict */
short			gNumHumanWords[2];		/* # total words in human (5- or 6-letter) dict */
unsigned char	gWhichChar;				/* which char in current word the cursor is at */
unsigned char	gNumLetters;			/* 5 or 6 */
char			gAllowDup;				/* allow duplicate letters */
char			gNonWordsCount;			/* non-words count against you */
char			gAnimation;				/* animation? */

void InitTheProgram(void)
{
	gWhichWipe=0;
	SetEndGame(FALSE);
	HandleError(OpenTheFiles(), TRUE);
}

void NewGame(void)
{
	enum ErrorTypes	resultCode;
	short			i;
	
	gNumTries=0;
	NewWord();
	for (i=0; i<MAX_TRIES; i++)
		gNumRight[i]=0;
	
	resultCode=GetComputerWord();
	if (resultCode!=allsWell)
		HandleError(resultCode, FALSE);
	else
	{
		SetGameInProgress(TRUE);
		StartGame();
	}
}

void StartGame(void)
{
	gWhichWipe=gLastWipe+1;
	if (gWhichWipe>NUMWIPES)
		gWhichWipe=1;
	SetEndGame(FALSE);
	OpenTheIndWindow(kMainWindow);
	ObscureCursor();
}

Boolean ShutDownTheProgram(void)
{
	WindowPtr		theWindow;
	
	while ((theWindow=GetFrontDocumentWindow())!=0L)
		if (!CloseTheWindow(theWindow))
			return FALSE;
	
	while ((theWindow=FrontWindow())!=0L)
		if (!CloseTheWindow(theWindow))
			return FALSE;
	
	CloseTheFiles();
	
	return TRUE;
}
