#include <Script.h>
#include "error.h"
#include "jotto load-save.h"
#include "jotto.h"
#include "jotto environment.h"
#include "file interface.h"
#include "util.h"
#include "graphics.h"
#include "window layer.h"
#include "program globals.h"

typedef struct
{
	unsigned short	version;					/* for forward compatibility */
	unsigned char	numLetters;					/* 5 or 6 */
	unsigned char	computerWord[6];			/* encrypted! */
	unsigned char	numRight[MAX_TRIES];		/* # letters right in n-th guess */
	unsigned char	humanWord[MAX_TRIES][6];	/* n-th guess */
	unsigned char	numTries;					/* # guesses so far */
	unsigned char	whichChar;					/* which char 1-6 of current word */
	unsigned char	checksum;					/* checksum of previous data */
} SaveStruct;

void LoadSaveDispatch(Boolean isLoad, Boolean useOldFile, FSSpec *useThisFS)
{
	FSSpec			saveFile;
	Boolean			alreadyExists;
	enum ErrorTypes	theError;
	WindowPtr		mainWindowPtr;
	Boolean			goon;
	
	if ((!isLoad) && ((mainWindowPtr=GetIndWindowPtr(kMainWindow))==0L))
		return;
	
	if (isLoad)
	{
		if (useOldFile)
		{
			goon=TRUE;
			saveFile=*useThisFS;
		}
		else
		{
			goon=GetSourceFile(&saveFile);
		}
		
		if (goon)
		{
			theError=GetTheFile(&saveFile);
			if (theError!=noErr)
				HandleError(theError, FALSE);
		}
	}
	else
	{
		saveFile=GetWindowFS(mainWindowPtr);
		useOldFile=useOldFile&&(saveFile.name[0]!=0x00);
		if (!useOldFile)
		{
			if (!GetDestFile(&saveFile, &alreadyExists, "\pSave Jotto ][ game as..."))
				return;
		}
		
		theError=SaveTheFile(&saveFile, alreadyExists);
		if (theError!=noErr)
			HandleError(theError, FALSE);
	}
}

enum ErrorTypes SaveTheFile(FSSpec *saveFile, Boolean alreadyExists)
{
	OSErr			theError;
	short			thisFile;
	long			count;
	SaveStruct		data;
	unsigned char	checksum;
	short			i,j;
	WindowPtr		theWindow;
	
	theWindow=GetIndWindowPtr(kMainWindow);
	
	data.version=SAVE_VERSION;
	data.numLetters=gNumLetters;
	for (i=0; i<6; i++)
		data.computerWord[i]=gComputerWord[i]^0x42;
	for (i=0; i<MAX_TRIES; i++)
		data.numRight[i]=gNumRight[i];
	for (i=0; i<MAX_TRIES; i++)
		for (j=0; j<6; j++)
			data.humanWord[i][j]=gHumanWord[i][j];
	data.numTries=gNumTries;
	data.whichChar=gWhichChar;	
	checksum=0;
	for (i=0; i<sizeof(SaveStruct)-2; i++)
		checksum+=*((unsigned char*)((long)&data+i));
	data.checksum=checksum;
	
	theError=noErr;
	
	if (!alreadyExists)
	{
		theError=FSpCreate(saveFile, CREATOR, SAVE_TYPE, smSystemScript);
		FlushVol(0L, saveFile->vRefNum);
	}
	
	if (theError!=noErr)
		return kCantCreateFile;
	
	FSpCreate(saveFile, CREATOR, SAVE_TYPE, smSystemScript);
	theError=FSpOpenDF(saveFile, fsRdWrPerm, &thisFile);
	
	if (theError!=noErr)
		return kCantOpenFileToSave;

	count=sizeof(SaveStruct);
	theError=SetEOF(thisFile, count);
	if (theError!=noErr)
	{
		FSClose(thisFile);
		return kDiskFull;
	}
	
	SetFPos(thisFile, 1, 0L);
	theError=FSWrite(thisFile, &count, &data);
	
	FSClose(thisFile);
	FlushVol(0L, saveFile->vRefNum);

	if (theError!=noErr)
	{
		saveFile->name[0]=0x00;
		SetWindowFS(theWindow, *saveFile);
		return kCantWriteFile;
	}
	
	SetWindowFS(theWindow, *saveFile);
	SetWindowIsModified(theWindow, FALSE);
	
	return allsWell;
}

enum ErrorTypes GetTheFile(FSSpec *saveFile)
{
	short			thisFile;
	unsigned char	checksum;
	long			count;
	short			i,j;
	SaveStruct		data;
	OSErr			theError;
	WindowPtr		theWindow;
	
	theError=FSpOpenDF(saveFile, fsRdPerm, &thisFile);
	if (theError!=noErr)
		return kCantOpenFileToLoad;
	
	count=sizeof(SaveStruct);
	SetFPos(thisFile, 1, 0L);
	theError=FSRead(thisFile, &count, &data);
	FSClose(thisFile);
	
	if (theError!=noErr)
		return kCantLoadFile;
	
	if (data.version!=SAVE_VERSION)
		return kSaveVersionNotSupported;
	
	checksum=0;
	for (i=0; i<sizeof(SaveStruct)-2; i++)
			checksum+=*((unsigned char*)((long)&data+i));

	if (checksum!=data.checksum)
	{
		saveFile->name[0]=0x00;
		return kBadChecksum;
	}

	gNumLetters=data.numLetters;
	for (i=0; i<6; i++)
		gComputerWord[i]=data.computerWord[i]^0x42;
	for (i=0; i<15; i++)
		gNumRight[i]=data.numRight[i];
	for (i=0; i<15; i++)
		for (j=0; j<6; j++)
			gHumanWord[i][j]=data.humanWord[i][j];
	gNumTries=data.numTries;
	gWhichChar=data.whichChar;
	if (gNumTries==MAX_TRIES)
		Mymemcpy((Ptr)gHumanWord[gNumTries], (Ptr)gComputerWord, 6);
	
	SetGameInProgress((gNumRight[gNumTries]!=gNumLetters) && (gNumTries<MAX_TRIES));
	StartGame();
	
	theWindow=GetIndWindowPtr(kMainWindow);
	SetWindowFS(theWindow, *saveFile);
	SetWindowIsModified(theWindow, FALSE);
	
	return allsWell;
}
