/**********************************************************************\

File:		jotto dictionary.c

Purpose:	This module handles opening/closing/maintaining the
			dictionary files (6 of 'em).  This includes getting
			words from all dictionaries and saving new words to the
			custom dictionaries.


Jotto ][ -=- a simple word game, revisited
Copyright (C) 1993 Mark Pilgrim

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "jotto globals.h"
#include "jotto dictionary.h"
#include "jotto files.h"
#include "jotto error.h"
#include "msg environment.h"
#include "util.h"

#define kFiveLetterDict		0
#define kSixLetterDict		1
#define kFiveLetterCustom	2
#define kSixLetterCustom	3

int				gComputerFile[2];
int				gHumanFile[4];
Boolean			gFiveLetterOK;			/* we have valid 5-letter dictionaries */
Boolean			gSixLetterOK;			/* we have valid 6-letter dictionaries */
Boolean			gFiveLetterCustomOK;	/* we have valid 5-letter custom dict */
Boolean			gSixLetterCustomOK;		/* we have valid 6-letter custom dict */
Boolean			gFiveLetterCustomSaveOK;
Boolean			gSixLetterCustomSaveOK;

int GetComputerWord(void)
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
	int			i,j;
	
	for (i=0; i<gNumLetters; i++)
		for (j=i+1; j<gNumLetters; j++)
			if (gComputerWord[i]==gComputerWord[j])
				return FALSE;

	return TRUE;
}

int GetHumanWord(char *thisWord, unsigned long index)
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
			kCantGetSixLetterCustomWord);
	
	return TRUE;
}

void SaveCustomWordToDisk(char *thisWord)
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
			gFiveLetterCustomSaveOK=FALSE;
			HandleError(kCantSaveFiveLetterCustomWord);
			return;
		}
		else
		{
			gSixLetterCustomSaveOK=FALSE;
			HandleError(kCantSaveSixLetterCustomWord);
			return;
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
			gFiveLetterCustomSaveOK=FALSE;
			HandleError(kCantSaveFiveLetterCustomWord);
		}
		else
		{
			gSixLetterCustomSaveOK=FALSE;
			HandleError(kCantSaveSixLetterCustomWord);
		}
	}
}

int OpenTheFiles(void)
{
	long			thisDirID;
	OSErr			isHuman;
	long			thisEOF;
	FSSpec			fs;
	int				i;
	
	for (i=0; i<4; i++)
		gComputerFile[i]=gHumanFile[i]=0;
	
	thisDirID=GetApplicationParID();
	if (thisDirID==-1L)
		return kNoDictionaries;
	
	MyMakeFSSpec(0, thisDirID, "\p:Five-letter custom dict", &fs);
	if (gHasFSSpecs)
		isHuman=FSpCreate(&fs, CREATOR, CUSTOM_TYPE, 0);
	else
		isHuman=HCreate(0, thisDirID, fs.name, CREATOR, CUSTOM_TYPE);
	gFiveLetterCustomOK=((isHuman==noErr) || (isHuman==dupFNErr));
	
	if (gFiveLetterCustomOK)
	{
		if (gHasFSSpecs)
			isHuman=FSpOpenDF(&fs, fsRdWrPerm, &gHumanFile[kFiveLetterCustom]);
		else
			isHuman=HOpen(0, thisDirID, fs.name, fsRdWrPerm, &gHumanFile[kFiveLetterCustom]);
		if (isHuman!=noErr)
			gFiveLetterCustomOK=FALSE;
	}
	
	MyMakeFSSpec(0, thisDirID, "\p:Six-letter custom dict", &fs);
	if (gHasFSSpecs)
		isHuman=FSpCreate(&fs, CREATOR, CUSTOM_TYPE, 0);
	else
		isHuman=HCreate(0, thisDirID, fs.name, CREATOR, CUSTOM_TYPE);
	gSixLetterCustomOK=((isHuman==noErr) || (isHuman==dupFNErr));
	
	if (gSixLetterCustomOK)
	{
		if (gHasFSSpecs)
			isHuman=FSpOpenDF(&fs, fsRdWrPerm, &gHumanFile[kSixLetterCustom]);
		else
			isHuman=HOpen(0, thisDirID, fs.name, fsRdWrPerm, &gHumanFile[kSixLetterCustom]);
		if (isHuman!=noErr)
			gSixLetterCustomOK=FALSE;
	}
	
	gFiveLetterCustomSaveOK=gFiveLetterCustomOK;
	gSixLetterCustomSaveOK=gSixLetterCustomOK;
	
	if (!gFiveLetterCustomOK)
	{
		if (!gSixLetterCustomOK)
			HandleError(kNoCustomAtAll);
		else
			HandleError(kNoFiveLetterCustom);
	}
	else
		if (!gSixLetterCustomOK)
			HandleError(kNoSixLetterCustom);
	
	MyMakeFSSpec(0, thisDirID, "\p:Five-letter computer dict", &fs);
	if (gHasFSSpecs)
		isHuman=FSpOpenDF(&fs, fsRdPerm, &gComputerFile[kFiveLetterDict]);
	else
		isHuman=HOpen(0, thisDirID, fs.name, fsRdPerm, &gComputerFile[kFiveLetterDict]);
	gFiveLetterOK=(isHuman==noErr);
	
	if (gFiveLetterOK)
	{
		MyMakeFSSpec(0, thisDirID, "\p:Five-letter human dict", &fs);
		if (gHasFSSpecs)
			isHuman=FSpOpenDF(&fs, fsRdPerm, &gHumanFile[kFiveLetterDict]);
		else
			isHuman=HOpen(0, thisDirID, fs.name, fsRdPerm, &gHumanFile[kFiveLetterDict]);
		gFiveLetterOK=(isHuman==noErr);
	}
	
	if (gFiveLetterOK)
	{
		GetEOF(gComputerFile[0], &thisEOF);
		gNumComputerWords[kFiveLetterDict]=thisEOF/6;
		
		GetEOF(gHumanFile[kFiveLetterDict], &thisEOF);
		gNumHumanWords[kFiveLetterDict]=thisEOF/6;
	}
	
	MyMakeFSSpec(0, thisDirID, "\p:Six-letter computer dict", &fs);
	if (gHasFSSpecs)
		isHuman=FSpOpenDF(&fs, fsRdPerm, &gComputerFile[kSixLetterDict]);
	else
		isHuman=HOpen(0, thisDirID, fs.name, fsRdPerm, &gComputerFile[kSixLetterDict]);
	gSixLetterOK=(isHuman==noErr);
	
	if (gSixLetterOK)
	{
		MyMakeFSSpec(0, thisDirID, "\p:Six-letter human dict", &fs);
		if (gHasFSSpecs)
			isHuman=FSpOpenDF(&fs, fsRdPerm, &gHumanFile[kSixLetterDict]);
		else
			isHuman=HOpen(0, thisDirID, fs.name, fsRdPerm, &gHumanFile[kSixLetterDict]);
		gSixLetterOK=(isHuman==noErr);
	}
	
	if (gSixLetterOK)
	{
		GetEOF(gComputerFile[kSixLetterDict], &thisEOF);
		gNumComputerWords[kSixLetterDict]=thisEOF/7;
		
		GetEOF(gHumanFile[kSixLetterDict], &thisEOF);
		gNumHumanWords[kSixLetterDict]=thisEOF/7;
	}
	
	if (!gFiveLetterOK)
		gNumLetters=6;
	
	return ((gFiveLetterOK) || (gSixLetterOK)) ? allsWell : kNoDictionaries;
}

void CloseTheFiles(void)
{
	if (gFiveLetterOK)
	{
		FSClose(gComputerFile[kFiveLetterDict]);
		FSClose(gHumanFile[kFiveLetterDict]);
	}
	
	if (gSixLetterOK)
	{
		FSClose(gComputerFile[kSixLetterDict]);
		FSClose(gHumanFile[kSixLetterDict]);
	}
	
	if (gFiveLetterCustomOK)
		FSClose(gHumanFile[kFiveLetterCustom]);
	
	if (gSixLetterCustomOK)
		FSClose(gHumanFile[kSixLetterCustom]);
}

long GetApplicationParID(void)
{
	CInfoPBRec		pb;
	int				err;
	
	pb.hFileInfo.ioCompletion=0L;
	pb.hFileInfo.ioNamePtr=CurApName;
	pb.hFileInfo.ioVRefNum=0;
	pb.hFileInfo.ioFDirIndex=0;
	pb.hFileInfo.ioDirID=0;
	err=PBGetCatInfo(&pb, FALSE);
	if (err!=noErr)
		return -1L;
	
	return pb.hFileInfo.ioFlParID;
}
