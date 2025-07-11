#include "program globals.h"
#include "file utilities.h"
#include "jotto dictionary.h"
#include "jotto environment.h"
#include "file interface.h"
#include "error.h"
#include "environment.h"
#include "util.h"

#define kFiveLetterDict		0
#define kSixLetterDict		1
#define kFiveLetterCustom	2
#define kSixLetterCustom	3

short			gComputerFile[2];
short			gHumanFile[4];

enum ErrorTypes GetComputerWord(void)
{
	unsigned long	index;
	Boolean			goodword;
	OSErr			isHuman;
	long			count;
	
	goodword=FALSE;
	while (!goodword)
	{
		index=(Random()&0x7fff)%gNumComputerWords[gNumLetters-5];
		SetFPos(gComputerFile[gNumLetters-5], 1, index*(gNumLetters+1));
		count=gNumLetters;
		isHuman=FSRead(gComputerFile[gNumLetters-5], &count, gComputerWord);

		goodword=gAllowDup ? TRUE : CheckDup();
	}
	return (isHuman==noErr) ? allsWell :
		(gNumLetters==5) ? kCantGetFiveLetterComputerWord : kCantGetSixLetterComputerWord;
}

Boolean CheckDup(void)
{
	short			i,j;
	
	for (i=0; i<gNumLetters; i++)
		for (j=i+1; j<gNumLetters; j++)
			if (gComputerWord[i]==gComputerWord[j])
				return FALSE;

	return TRUE;
}

enum ErrorTypes GetHumanWord(char *thisWord, unsigned long index)
{
	long			count;
	OSErr			isHuman;
	
	SetFPos(gHumanFile[gNumLetters-5], 1, index*(gNumLetters+1));
	count=gNumLetters;
	isHuman=FSRead(gHumanFile[gNumLetters-5], &count, thisWord);
	
	return (isHuman==noErr) ? allsWell :
		(gNumLetters==5) ? kCantGetFiveLetterHumanWord : kCantGetSixLetterHumanWord;
}

Boolean GetCustomWord(char *thisWord, unsigned long index)
{
	long			count;
	OSErr			isHuman;
	
	if (SetFPos(gHumanFile[gNumLetters-3], 1, (index+1)*(gNumLetters+1))==eofErr)
		return FALSE;
	
	SetFPos(gHumanFile[gNumLetters-3], 1, index*(gNumLetters+1));
	count=gNumLetters;
	isHuman=FSRead(gHumanFile[gNumLetters-3], &count, thisWord);
	
	if (isHuman!=noErr)
		HandleError((gNumLetters==5) ? kCantGetFiveLetterCustomWord :
			kCantGetSixLetterCustomWord, TRUE);
	
	return TRUE;
}

enum ErrorTypes SaveCustomWordToDisk(char *thisWord)
{
	long			oldEOF;
	unsigned char	temp[7];
	long			count;
	OSErr			isHuman;
	
	GetEOF(gHumanFile[gNumLetters-3], &oldEOF);
	if (SetEOF(gHumanFile[gNumLetters-3], oldEOF+gNumLetters+1)!=noErr)
	{
		if (gNumLetters==5)
		{
			SetFiveLetterCustomSave(FALSE);
			return kCantSaveFiveLetterCustomWord;
		}
		else
		{
			SetSixLetterCustomSave(FALSE);
			return kCantSaveSixLetterCustomWord;
		}
	}
	SetFPos(gHumanFile[gNumLetters-3], 1, oldEOF);
	Mymemcpy((Ptr)temp, (Ptr)thisWord, 6);
	temp[gNumLetters]=0x0d;
	count=gNumLetters+1;
	isHuman=FSWrite(gHumanFile[gNumLetters-3], &count, temp);
	if (isHuman!=noErr)
	{
		if (gNumLetters==5)
		{
			SetFiveLetterCustomSave(FALSE);
			return kCantSaveFiveLetterCustomWord;
		}
		else
		{
			SetSixLetterCustomSave(FALSE);
			return kCantSaveSixLetterCustomWord;
		}
	}
	
	return allsWell;
}

enum ErrorTypes OpenTheFiles(void)
{
	long			thisDirID;
	OSErr			isHuman;
	long			thisEOF;
	FSSpec			fs;
	short			i;
	Boolean			fiveLetterOK;
	Boolean			sixLetterOK;
	Boolean			fiveLetterCustomOK;
	Boolean			sixLetterCustomOK;
	Boolean			fiveLetterCustomSaveOK;
	Boolean			sixLetterCustomSaveOK;
	short			vRefNum;

	for (i=0; i<4; i++)
		gComputerFile[i]=gHumanFile[i]=0;
	
	if (GetApplicationParID(&thisDirID)!=noErr)
		return kNoDictionaries;
	
	if (GetApplicationVRefNum(&vRefNum)!=noErr)
		return kNoDictionaries;
	
	FSMakeFSSpec(vRefNum, thisDirID, "\p:Five-letter custom dict", &fs);
	isHuman=FSpCreate(&fs, CREATOR, CUSTOM_TYPE, 0);
	fiveLetterCustomOK=((isHuman==noErr) || (isHuman==dupFNErr));
	
	if (fiveLetterCustomOK)
	{
		isHuman=FSpOpenDF(&fs, fsRdWrPerm, &gHumanFile[kFiveLetterCustom]);
		if (isHuman!=noErr)
			fiveLetterCustomOK=FALSE;
	}
	
	FSMakeFSSpec(vRefNum, thisDirID, "\p:Six-letter custom dict", &fs);
	isHuman=FSpCreate(&fs, CREATOR, CUSTOM_TYPE, 0);
	sixLetterCustomOK=((isHuman==noErr) || (isHuman==dupFNErr));
	
	if (sixLetterCustomOK)
	{
		isHuman=FSpOpenDF(&fs, fsRdWrPerm, &gHumanFile[kSixLetterCustom]);
		if (isHuman!=noErr)
			sixLetterCustomOK=FALSE;
	}
	
	fiveLetterCustomSaveOK=fiveLetterCustomOK;
	sixLetterCustomSaveOK=sixLetterCustomOK;
	
	if (!fiveLetterCustomOK)
	{
		if (!sixLetterCustomOK)
			HandleError(kNoCustomAtAll, FALSE);
		else
			HandleError(kNoFiveLetterCustom, FALSE);
	}
	else
		if (!sixLetterCustomOK)
			HandleError(kNoSixLetterCustom, FALSE);
	
	FSMakeFSSpec(vRefNum, thisDirID, "\p:Five-letter computer dict", &fs);
	isHuman=FSpOpenDF(&fs, fsRdPerm, &gComputerFile[kFiveLetterDict]);
	fiveLetterOK=(isHuman==noErr);
	
	if (fiveLetterOK)
	{
		FSMakeFSSpec(vRefNum, thisDirID, "\p:Five-letter human dict", &fs);
		isHuman=FSpOpenDF(&fs, fsRdPerm, &gHumanFile[kFiveLetterDict]);
		fiveLetterOK=(isHuman==noErr);
	}
	
	if (fiveLetterOK)
	{
		GetEOF(gComputerFile[0], &thisEOF);
		gNumComputerWords[kFiveLetterDict]=thisEOF/6;
		
		GetEOF(gHumanFile[kFiveLetterDict], &thisEOF);
		gNumHumanWords[kFiveLetterDict]=thisEOF/6;
	}
	
	FSMakeFSSpec(vRefNum, thisDirID, "\p:Six-letter computer dict", &fs);
	isHuman=FSpOpenDF(&fs, fsRdPerm, &gComputerFile[kSixLetterDict]);
	sixLetterOK=(isHuman==noErr);
	
	if (sixLetterOK)
	{
		FSMakeFSSpec(vRefNum, thisDirID, "\p:Six-letter human dict", &fs);
		isHuman=FSpOpenDF(&fs, fsRdPerm, &gHumanFile[kSixLetterDict]);
		sixLetterOK=(isHuman==noErr);
	}
	
	if (sixLetterOK)
	{
		GetEOF(gComputerFile[kSixLetterDict], &thisEOF);
		gNumComputerWords[kSixLetterDict]=thisEOF/7;
		
		GetEOF(gHumanFile[kSixLetterDict], &thisEOF);
		gNumHumanWords[kSixLetterDict]=thisEOF/7;
	}
	
	if (!fiveLetterOK)
		gNumLetters=6;
	
	SetDictionaryFlags(fiveLetterOK, sixLetterOK, fiveLetterCustomOK, sixLetterCustomOK,
		fiveLetterCustomSaveOK, sixLetterCustomSaveOK);
	
	return ((fiveLetterOK) || (sixLetterOK)) ? allsWell : kNoDictionaries;
}

void CloseTheFiles(void)
{
	if (FiveLetterOKQQ())
	{
		FSClose(gComputerFile[kFiveLetterDict]);
		FSClose(gHumanFile[kFiveLetterDict]);
	}
	
	if (SixLetterOKQQ())
	{
		FSClose(gComputerFile[kSixLetterDict]);
		FSClose(gHumanFile[kSixLetterDict]);
	}
	
	if (FiveLetterCustomOKQQ())
		FSClose(gHumanFile[kFiveLetterCustom]);
	
	if (SixLetterCustomOKQQ())
		FSClose(gHumanFile[kSixLetterCustom]);
}
