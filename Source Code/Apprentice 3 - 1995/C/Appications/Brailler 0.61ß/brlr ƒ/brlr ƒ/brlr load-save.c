#include <Script.h>
#include "error.h"
#include "brlr load-save.h"
#include "brlr main window.h"
#include "brlr grade 1 meat.h"
#include "brlr grade 2 meat.h"
#include "file interface.h"
#include "file utilities.h"
#include "text twiddling.h"
#include "environment.h"
#include "util.h"
#include "graphics.h"
#include "dialog layer.h"
#include "window layer.h"
#include "program globals.h"

#define kImportDialogID		200

short			gOldImportGrade;

static	Boolean DoImportDialog(short *newGrade);

void LoadSaveDispatch(Boolean isLoad, Boolean useOldFile)
{
	FSSpec			saveFile;
	Boolean			alreadyExists;
	enum ErrorTypes	theError;
	WindowPtr		mainWindowPtr;
	
	if ((!isLoad) && ((mainWindowPtr=GetIndWindowPtr(kMainWindow))==0L))
		return;
	
	if (isLoad)
	{
		if (GetSourceFile(&saveFile, SAVE_TYPE))
		{
			theError=GetTheFile(&saveFile);
			if (theError!=allsWell)
				HandleError(theError, FALSE, FALSE);
		}
	}
	else
	{
		saveFile=GetWindowFS(mainWindowPtr);
		useOldFile=useOldFile&&(saveFile.name[0]!=0x00);
		if (!useOldFile)
		{
			if (!GetDestFile(&saveFile, &alreadyExists, "\pSave document as..."))
				return;
		}
		
		theError=SaveTheFile(saveFile, alreadyExists);
		if (theError!=noErr)
			HandleError(theError, FALSE, FALSE);
	}
}

enum ErrorTypes SaveTheFile(FSSpec saveFile, Boolean alreadyExists)
{
	OSErr			theError;
	short			thisFile;
	long			count;
	TEHandle		hTE;
	WindowPtr		theWindow;
	
	theError=noErr;
	
	if (!alreadyExists)
	{
		theError=FSpCreate(&saveFile, CREATOR, SAVE_TYPE, smSystemScript);
		FlushVol(0L, saveFile.vRefNum);
	}
	
	if (theError!=noErr)
		return kCantCreateFile;
	
	FSpCreate(&saveFile, CREATOR, SAVE_TYPE, smSystemScript);
	theError=FSpOpenDF(&saveFile, fsRdWrPerm, &thisFile);
	
	if (theError!=noErr)
		return kCantOpenFileToSave;

	theWindow=GetIndWindowPtr(kMainWindow);
	hTE=GetWindowTE(theWindow);
	count=(**hTE).teLength;
	theError=SetEOF(thisFile, count);
	if (theError!=noErr)
	{
		FSClose(thisFile);
		return kDiskFull;
	}
	
	SetFPos(thisFile, 1, 0L);
	theError=FSWrite(thisFile, &count, *((**hTE).hText));
	
	FSClose(thisFile);
	FlushVol(0L, saveFile.vRefNum);

	if (theError!=noErr)
	{
		saveFile.name[0]=0x00;
		return kCantWriteFile;
	}
	
	SetWTitle(theWindow, saveFile.name);
	SetWindowTitle(theWindow, saveFile.name);
	SetWindowFS(theWindow, saveFile);
	SetWindowIsModified(theWindow, FALSE);
	
	return allsWell;
}

enum ErrorTypes GetTheFile(FSSpec *saveFile)
{
	short			thisFile;
	long			count;
	OSErr			theError;
	Ptr				data;
	WindowPtr		theWindow;
	short			newGrade;
	Boolean			goon;
	
	theError=FSpOpenDF(saveFile, fsRdPerm, &thisFile);
	if (theError!=noErr)
		return kCantOpenFileToLoad;
	
	GetEOF(thisFile, &count);
	data=NewPtr(count);
	SetFPos(thisFile, 1, 0L);
	theError=FSRead(thisFile, &count, data);
	FSClose(thisFile);
	
	if (theError!=noErr)
		return kCantLoadFile;
	
	if (GetFileCreator(saveFile)!=CREATOR)
	{
		goon=DoImportDialog(&newGrade);
	}
	else
	{
		goon=TRUE;
		newGrade=0;
	}
	
	if (goon)
	{
		if (newGrade==0)
			SetMainWindowTitle(saveFile->name);
		OpenTheIndWindow(kMainWindow);
		theWindow=GetIndWindowPtr(kMainWindow);
		switch (newGrade)
		{
			case 0:
				SetTheText(theWindow, data, count);
				break;
			case 1:
				DealWithGrade1Text(theWindow, (unsigned char*)data, count);
				break;
			case 2:
				DealWithGrade2Text(theWindow, (unsigned char*)data, count);
				break;
		}
			
		if (newGrade==0)
			SetWindowFS(theWindow, *saveFile);
		SetWindowIsModified(theWindow, (newGrade!=0));
	}
	
	DisposePtr(data);
	
	return allsWell;
}

static	Boolean DoImportDialog(short *newGrade)
{
	DialogPtr		theDialog;
	UniversalProcPtr	modalFilterProc;
	short			itemSelected, i;
	
	theDialog=SetupTheDialog(kImportDialogID, 3, "\pDocument conversion",
		(UniversalProcPtr)TwoButtonFilter, &modalFilterProc);
	for (i=5; i<=7; i++)
		SetCheckboxState(theDialog, i, gOldImportGrade==i-5);
	DisplayTheDialog(theDialog, TRUE);
	itemSelected=0;
	while ((itemSelected!=1) && (itemSelected!=2))
	{
		itemSelected=GetOneDialogEvent(theDialog, modalFilterProc);
		if ((itemSelected>=5) && (itemSelected<=7))
		{
			*newGrade=itemSelected-5;
			for (i=5; i<=7; i++)
				SetCheckboxState(theDialog, i, ((*newGrade)==i-5));
		}
	}
	
	if (itemSelected==1)
	{
		for (i=5; i<=7; i++)
			if (CheckboxIsCheckedQQ(theDialog, i))
				*newGrade=i-5;
		gOldImportGrade=*newGrade;
	}
	
	ShutDownTheDialog(theDialog, modalFilterProc);
	return (itemSelected==1);
}
