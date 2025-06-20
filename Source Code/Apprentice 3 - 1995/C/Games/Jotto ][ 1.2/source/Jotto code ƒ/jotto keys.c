#include "program globals.h"
#include "jotto keys.h"
#include "jotto meat.h"
#include "jotto endgame.h"
#include "jotto dictionary.h"
#include "jotto graphics.h"
#include "jotto environment.h"
#include "sounds.h"
#include "dialogs.h"
#include "error.h"
#include "util.h"
#include "main.h"
#include "graphics.h"

#define badWordAlert			202
#define badWordNoCustomAlert	203

static pascal Boolean LearnFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem);

void UserHitReturn(WindowPtr theWindow)
{
	short			i;
	
	for (i=0; i<gNumLetters; i++)
		if ((gHumanWord[gNumTries][i]<'A') || (gHumanWord[gNumTries][i]>'Z'))
			return;

	if (ValidWord(gHumanWord[gNumTries]))
		UserGuessedValidWord(theWindow);
	else
		UserGuessedInvalidWord(theWindow);
}

void UserGuessedValidWord(WindowPtr theWindow)
{
	CalculateNumRight();
	DrawWordInList(theWindow);
	if (gNumRight[gNumTries]==gNumLetters)
		WinGame(theWindow);
	else
	{
		gNumTries++;
		if (gNumTries!=MAX_TRIES)
		{
			DoSound(sound_playword_good, TRUE);
			NewWord();
			SetPort(theWindow);
			DrawCurrentWord(theWindow);
			HighlightChar(theWindow);
		}
		else LoseGame(theWindow);
	}
}

void UserGuessedInvalidWord(WindowPtr theWindow)
{
	short		alertResult;
	Str255		tempStr;
	ModalFilterUPP	procFilter = NewModalFilterProc(LearnFilter);
	
	if ((gNumLetters==5) ? (FiveLetterCustomSaveOKQQ()) : (SixLetterCustomSaveOKQQ()))
		DoSound(sound_badword, TRUE);
	Mymemcpy((Ptr)((long)tempStr+1), (Ptr)gHumanWord[gNumTries], gNumLetters);
	tempStr[0]=gNumLetters;
	ParamText(tempStr, "\p", "\p", "\p");
	if ((gNumLetters==5) ? (FiveLetterCustomSaveOKQQ()) : (SixLetterCustomSaveOKQQ()))
	{
		PositionDialog('ALRT', badWordAlert);
		alertResult=NoteAlert(badWordAlert, procFilter);
		DisposeRoutineDescriptor(procFilter);
		if (alertResult==1)
		{
			DoSound(sound_bluffing, TRUE);
			AdmitBluffing(theWindow);
		}
		else
			LearnWord(theWindow);
	}
	else
	{
		DoSound(sound_bluffing, TRUE);
		PositionDialog('ALRT', badWordNoCustomAlert);
		NoteAlert(badWordNoCustomAlert, OneButtonFilter);
		AdmitBluffing(theWindow);
	}
}

void AdmitBluffing(WindowPtr theWindow)
{
	if (gNonWordsCount)
		gNumRight[gNumTries]=-1;
	
	UpdateTheWindow(theWindow);
	
	if (gNonWordsCount)
	{
		DrawWordInList(theWindow);
		gNumTries++;
	}
	
	if (gNumTries!=MAX_TRIES)
	{
		NewWord();
		SetPort(theWindow);
		DrawCurrentWord(theWindow);
		HighlightChar(theWindow);
	}
	else
		LoseGame(theWindow);
}

void LearnWord(WindowPtr theWindow)
{
	HandleError(SaveCustomWordToDisk(gHumanWord[gNumTries]), FALSE);
	UpdateTheWindow(theWindow);
	UserGuessedValidWord(theWindow);
}

void UserHitLeftArrow(WindowPtr theWindow, char charPressed)
{
	HighlightChar(theWindow);
	if (gWhichChar!=0x00)
		gWhichChar--;
	else if (charPressed==0x1c)
		gWhichChar=gNumLetters-1;
	HighlightChar(theWindow);
}

void UserHitRightArrow(WindowPtr theWindow)
{
	HighlightChar(theWindow);
	gWhichChar++;
	if (gWhichChar==gNumLetters)
		gWhichChar=0x00;
	HighlightChar(theWindow);
}

void UserHitLetter(WindowPtr theWindow, char charPressed)
{
	DoSound(sound_keyclick, TRUE);
	gHumanWord[gNumTries][gWhichChar]=charPressed&0xdf;
	DrawOneChar(theWindow);
	if ((gWhichChar!=gNumLetters-1) || (charPressed==' '))
	{
		gWhichChar++;
		if (gWhichChar==gNumLetters)
			gWhichChar=0x00;
	}
	HighlightChar(theWindow);
}

static pascal Boolean LearnFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem)
{
	unsigned char	theChar;
	
	switch (theEvent->what)	/* examine event record */
	{
		case keyDown:	/* keypress */
		case autoKey:
			theChar=theEvent->message & charCodeMask;	/* get ascii char value */
			if ((theChar==0x0d) || (theChar==0x03))		/* RETURN or ENTER */
			{
				*theItem=FakeSelect(theDialog, 1);
				return TRUE;
			}
			if (theEvent->modifiers&cmdKey)
			{
				switch (theChar)
				{
					case 'l':
					case 'L':
						*theItem=FakeSelect(theDialog, 2);
						return TRUE;
						break;
					case 'a':
					case 'A':
						*theItem=FakeSelect(theDialog, 1);
						return TRUE;
						break;
				}
			}
			break;
		case updateEvt:
			if ((theEvent->message)!=(unsigned long)theDialog)
				DispatchEvents(*theEvent, FALSE);
			else
				OutlineDefaultButton(theDialog, 1);
			break;
	}
	
	return FALSE;	/* no faking, proceed as planned */
}
