#include "kant event class dispatch.h"
#include "kant parser dispatch.h"
#include "kant main window.h"
#include "kant build files.h"
#include "kant load-save.h"
#include "error.h"
#include "music layer.h"
#include "menus.h"
#include "main.h"
#include "cancel.h"
#include "window layer.h"
#include "text layer.h"
#include "speech layer.h"
#include "environment.h"
#include "file utilities.h"
#include "prefs.h"
#include "cursor layer.h"
#include "program globals.h"

void KGPResolve(void)
{
	WindowRef		mainWindow;
	
	mainWindow=GetIndWindowRef(kMainWindow);
	
	if (GetFrontDocumentWindow()!=mainWindow)
		MySelectWindow(mainWindow);
	
	HandleError(ParserDispatch(mainWindow), FALSE, TRUE);
}

void KGPInsert(Str255 theStr)
{
	WindowRef		mainWindow;
	
	mainWindow=GetIndWindowRef(kMainWindow);
	if (mainWindow==0L)
	{
		OpenTheIndWindow(kMainWindow, kAlwaysOpenNew);
		mainWindow=GetIndWindowRef(kMainWindow);
	}
	
	if (GetFrontDocumentWindow()!=mainWindow)
		MySelectWindow(mainWindow);
	
	InsertAfterEnd(mainWindow, theStr);
	
	if (gAlwaysResolve)
		HandleError(ParserDispatch(GetIndWindowRef(kMainWindow)), FALSE, TRUE);
}

void KGPNew(void)
{
	SetMainWindowTitle("\p");
	OpenTheIndWindow(kMainWindow, kAlwaysOpenNew);
	gNeedToOpenWindow=FALSE;
}

DescType KGPUseModule(Str255 theStr, Boolean tryFullPath)
{
	FSSpec			theFS;
	
	if (theStr[0]==0x00)
	{
		theFS.name[0]=0x00;
		UseTheModule(&theFS, TRUE);
	}
	else
	{
		if (GetNamedModuleFS(&theFS, theStr)==noErr)
		{
			UseTheModule(&theFS, FALSE);
		}
		else
		{
			if (tryFullPath && (FSpLocationFromFullPath(theStr[0], &theStr[1], &theFS)==noErr))
			{
				UseTheModule(&theFS, FALSE);
			}
			else
			{
				if (!tryFullPath) /* if tryFullPath, this is from a script, so only return error */
					HandleError(kModuleNotFound, FALSE, FALSE);
				return kResultFailure;
			}
		}
	}
	
	return kResultSuccess;
}

void KGPGetText(Ptr *theTextPtr, unsigned short *theTextLength)
{
	WindowRef		mainWindow;
	TEHandle		hTE;
	
	*theTextLength=0;
	if ((mainWindow=GetIndWindowRef(kMainWindow))!=0L)
	{
		hTE=GetWindowTE(mainWindow);
		if (hTE!=0L)
		{
			*theTextPtr=*((**hTE).hText);
			*theTextLength=(**hTE).teLength;
		}
	}
}

DescType KGPCloseDocument(DescType saveType)
{
	WindowRef		theWindow;
	FSSpec			fs;
	
	if ((theWindow=GetIndWindowRef(kMainWindow))==0L)
		return kResultFailure;
	
	switch (saveType)
	{
		case kSaveYes:
			fs=GetWindowFS(theWindow);
			if (WindowIsModifiedQQ(theWindow))
				LoadSaveDispatch(&fs, FALSE, TRUE);
			if (WindowIsModifiedQQ(theWindow))	/* save unsuccessful or cancelled */
				return kResultFailure;
			break;
		case kSaveNo:
			SetWindowIsModified(theWindow, FALSE);
			break;
		case kSaveAsk:
			SetWindowIsModified(theWindow, TRUE);
			break;
	}
	
	return CloseTheWindow(theWindow) ? kResultSuccess : kResultCancel;
}

Boolean IsOkayOptionTypeQQ(DescType optionType)
{
	if ((optionType==kOptionAlwaysResolve) || (optionType==kOptionShowStages) ||
		(optionType==kOptionListInteresting) || (optionType==kOptionDynamicScrolling) ||
		(optionType==kOptionMenuBarIcons))
	{
		return TRUE;
	}
	
	return FALSE;
}

void KGPSetOption(DescType optionType, DescType saveType)
{
	Boolean			isOn;
	
	isOn=(saveType==kSettingOn);
	switch (optionType)
	{
		case kOptionAlwaysResolve:
			gAlwaysResolve=isOn;
			break;
		case kOptionShowStages:
			gFastResolve=!isOn;
			break;
		case kOptionListInteresting:
			gShowAllRefs=isOn;
			RebuildInsertMenu();
			break;
		case kOptionDynamicScrolling:
			gDynamicScroll=isOn;
			break;
		case kOptionMenuBarIcons:
			gIconifyMenus=isOn;
			IconifyMenus(isOn);
			break;
	}
}

void KGPSpeed(DescType speedType)
{
	switch (speedType)
	{
		case kSpeedNoDelay:
			gSpeedDelay=0;
			break;
		case kSpeedOneTenthSecondDelay:
			gSpeedDelay=6;
			break;
		case kSpeedOneFifthSecondDelay:
			gSpeedDelay=12;
			break;
		case kSpeedOneHalfSecondDelay:
			gSpeedDelay=30;
			break;
		case kSpeedOneSecondDelay:
			gSpeedDelay=60;
			break;
	}
}

void KGPMusic(DescType musicType)
{
	switch (musicType)
	{
		case kMusicOptionAlways:
			gMusicStatus=kMusicAlways;
			if (!MusicIsPlayingQQ())
				StartTheMusic();
			break;
		case kMusicOptionOnly:
			gMusicStatus=kMusicGeneratingOnly;
			if (gInProgress)
			{
				if (!MusicIsPlayingQQ())
					StartTheMusic();
			}
			else
			{
				if (MusicIsPlayingQQ())
					CloseTheMusicChannel();
			}
			break;
		case kMusicOptionNever:
			gMusicStatus=kMusicNever;
			if (MusicIsPlayingQQ())
				CloseTheMusicChannel();
			break;
	}
}

DescType KGPSave(FSSpec *fsPtr, Boolean fsIsValid)
{
	WindowRef		theWindow;
	ErrorTypes		saveError;
	
	if ((theWindow=GetIndWindowRef(kMainWindow))==0L)
		return kResultFailure;
	
	if (!fsIsValid)
		*fsPtr=GetWindowFS(theWindow);
	
	saveError=LoadSaveDispatch(fsPtr, FALSE, TRUE);
	
	switch (saveError)
	{
		case allsWell:
			return kResultSuccess;
		case userCancelErr:
			return kResultCancel;
		default:
			return kResultFailure;
	}
}

void KGPRemember(void)
{
	RememberOptions();
}

DescType KGPRestore(void)
{
	return RestoreOptions() ? kResultSuccess : kResultFailure;
}

DescType KGPSpeech(Boolean speakAll)
{
	WindowRef		theWindow;
	TEHandle		hTE;
	short			len, start, end;
	SpeechChannel	speechChannel;
	
	if (!SpeechIsAvailableQQ())
		return kResultFailure;
	if ((theWindow=GetIndWindowRef(kMainWindow))==0L)
		return kResultFailure;
	if ((hTE=GetWindowTE(theWindow))==0L)
		return kResultFailure;
	if ((len=(**hTE).teLength)==0)
		return kResultSuccess;
	
	if (!speakAll)
	{
		start=(**hTE).selStart;
		end=(**hTE).selEnd;
	}
	else
	{
		start=0;
		end=len;
	}
	
	gInProgress=TRUE;
	gCustomCursor=TRUE;
	AdjustMenus();
	DrawMenuBar();
	speechChannel=StartTheSpeech((**hTE).hText, start, end);
	while (gInProgress)
	{
		AnimateTheCursor();
		if (!DealWithOtherPeople())
			gInProgress=FALSE;
		if ((theWindow=GetIndWindowRef(kMainWindow))==0L)
			gInProgress=FALSE;
		if (SpeechBusy()==0)
			gInProgress=FALSE;
	}
	StopTheSpeech(speechChannel);
	if ((theWindow=GetIndWindowRef(kMainWindow))!=0L)
		if ((hTE=GetWindowTE(theWindow))!=0L)
			HUnlock((**hTE).hText);
	gInProgress=FALSE;
	gCustomCursor=FALSE;
	AdjustMenus();
	DrawMenuBar();
	
	return kResultSuccess;
}

DescType KGPSetVoice(Str255 theVoiceStr)
{
	return SetCurrentVoiceFromName(theVoiceStr) ? kResultSuccess : kResultFailure;
}
