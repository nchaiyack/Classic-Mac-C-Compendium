#include "kant load-save.h"
#include "kant main window.h"
#include "file utilities.h"
#include "error.h"
#include "file interface.h"
#include "text layer.h"
#include "dialogs.h"
#include "environment.h"
#include "memory layer.h"
#include "window layer.h"
#include "windows menu.h"

ErrorTypes LoadSaveDispatch(FSSpec *fs, Boolean isLoad, Boolean useOldFile)
/* handles errors, but returns error code for your convenience anyway */
{
	Boolean			alreadyExists;
	ErrorTypes		theError;
	WindowRef		mainWindowRef;
	
	theError=allsWell;
	
	if ((!isLoad) && ((mainWindowRef=GetIndWindowRef(kMainWindow))==0L))
		return userCancelErr;
	
	if (isLoad)
	{
		if (GetSourceFile(fs, SAVE_TYPE))
		{
			if ((theError=GetTheFile(fs))!=allsWell)
				HandleError(theError, FALSE, FALSE);
		}
	}
	else
	{
		useOldFile=useOldFile&&((fs->name)[0]!=0x00);
		alreadyExists=(FileExists(fs)==noErr);
		if (!useOldFile)
		{
			if (!GetDestFile(fs, &alreadyExists, "\pSave document as..."))
				return userCancelErr;
		}
		
		if ((theError=SaveTheFile(*fs, alreadyExists))!=allsWell)
			HandleError(theError, FALSE, FALSE);
	}
	
	return theError;
}

ErrorTypes SaveTheFile(FSSpec saveFile, Boolean alreadyExists)
{
	OSErr			theError;
	short			thisFile;
	long			count;
	TEHandle		hTE;
	WindowRef		theWindow;
	
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

	theWindow=GetIndWindowRef(kMainWindow);
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
	ModifyFromWindowsMenu(theWindow);
	
	return allsWell;
}

ErrorTypes GetTheFile(FSSpec *saveFile)
{
	short			thisFile;
	long			count;
	OSErr			theError;
	Ptr				data;
	WindowRef		theWindow;
	
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
	
	SetMainWindowTitle(saveFile->name);
	OpenTheIndWindow(kMainWindow, kAlwaysOpenNew);
	theWindow=GetIndWindowRef(kMainWindow);
	SetTheText(theWindow, data, count);
	SetWindowFS(theWindow, *saveFile);
	SetWindowIsModified(theWindow, FALSE);
	DisposePtr(data);
	
	return allsWell;
}
