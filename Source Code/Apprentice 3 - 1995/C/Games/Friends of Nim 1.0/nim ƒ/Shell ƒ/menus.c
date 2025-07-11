#include "menus.h"
#include "program globals.h"
#include "key layer.h"
#include "help.h"
#include "environment.h"
#if USE_SOUNDS
#include "sound layer.h"
#endif
#if USE_MUSIC
#include "music layer.h"
#endif
#if USE_PRINTING
#include "printing layer.h"
#endif
#include "program init.h"
#include "resource layer.h"
#include "graphics dispatch.h"
#include "memory layer.h"
#include "window layer.h"
#if USE_WINDOWS_MENU
#include "windows menu.h"
#endif
#if USE_SAVE
#include "save dialog.h"
#include "nim load-save.h"
#endif
#include "nim.h"
#include "nim endgame.h"
#include "nim dialogs.h"
#include "nim globals.h"

static	MenuHandle		gAppleMenu=0L;
static	MenuHandle		gFileMenu=0L;
static	MenuHandle		gEditMenu=0L;
static	MenuHandle		gOptionsMenu=0L;
static	MenuHandle		gGameMenu=0L;
static	MenuHandle		gSetupMenu=0L;
#if USE_WINDOWS_MENU
		MenuHandle		gWindowsMenu=0L;
#endif

enum
{
	mApple = 400, mFile, mEdit, mOptions, mGame, mSetup,
#if USE_WINDOWS_MENU
		mWindows,
#endif
	
	iAbout = 1, iOtherProducts, iHelp,
	
#if USE_SAVE
	iNew = 1, iOpen, file_unused1, iClose, iSave, iSaveAs, file_unused2, iPageSetup, iPrint,
		file_unused3, iQuit,
#else
	iNew = 1, iClose, file_unused1, iPageSetup, iPrint, file_unused2, iQuit,
#endif
	
	iUndo = 1, edit_unused1, iCut, iCopy, iPaste, iClear, edit_unused2, iSelectAll,
	
	iSound = 1, iMusic, iMessages, iAnimation, options_unused1, iMisere, options_unused2,
		iHumanHumanMode, iMacHumanMode, iHumanMacMode, iMacMacMode,
	
	iNim = 1, iPrimeNim, iColumnsNim, game_unused1, iTurnablock,
		iSilver, game_unused2, iCornerTheQueen, iCornerTheKing, iCornerTheSuperqueen,
		iCornerTheSuperking,
	
#if USE_WINDOWS_MENU
	iArrange=1, iZoom, iSendToBack
#else
	enumDummy
#endif
};

/*-----------------------------------------------------------------------------------*/
/* internal stuff for menus.c                                                        */

static	void HandleAppleMenu(short menuItem);
static	void HandleFileMenu(short menuItem);
static	void HandleEditMenu(short menuItem, Boolean alreadyPassedThrough);
static	void HandleOptionsMenu(short menuItem);
static	void HandleGameMenu(short menuItem);
static	void HandleSetupMenu(short menuItem);
#if USE_WINDOWS_MENU
static	void HandleWindowsMenu(short menuItem);
#endif
static	void EDItem(MenuHandle theMenu, short theItem, Boolean theCondition);

Boolean InitTheMenus(void)
{
	Handle			MBARHandle;
	
	if ((MBARHandle=GetNewMBar(400))==0L)		/* sez which menus are in menu bar. */
		return FALSE;
	SetMenuBar(MBARHandle);						/* set this to be THE menu bar to use. */
	
	if ((gAppleMenu=GetMHandle(mApple))==0L)	/* GetNewMBar also got menu handles of */
		return FALSE;
	if ((gFileMenu=GetMHandle(mFile))==0L)	/* every menu it includes, so just */
		return FALSE;
	if ((gEditMenu=GetMHandle(mEdit))==0L)	/* grab these handles and assign them */
		return FALSE;
	if ((gOptionsMenu=GetMHandle(mOptions))==0L)
		return FALSE;
	if ((gGameMenu=GetMHandle(mGame))==0L)
		return FALSE;
	if ((gSetupMenu=GetMHandle(mSetup))==0L)
		return FALSE;
#if USE_WINDOWS_MENU
	if ((gWindowsMenu=GetMHandle(mWindows))==0L)
		return FALSE;
#endif
	
	AddResMenu(gAppleMenu, 'DRVR');				/* adds control panels to apple menu */
	
	AdjustMenus();								/* dim/enable/check/mark menus/items */
	DrawMenuBar();								/* draws the actual menu bar */
	
	return TRUE;
}

void ShutDownTheMenus(void)
{
	gAppleMenu=SafeDisposeMenu(gAppleMenu);
	gFileMenu=SafeDisposeMenu(gFileMenu);
	gEditMenu=SafeDisposeMenu(gEditMenu);
	gOptionsMenu=SafeDisposeMenu(gOptionsMenu);
	gGameMenu=SafeDisposeMenu(gGameMenu);
	gSetupMenu=SafeDisposeMenu(gSetupMenu);
#if USE_WINDOWS_MENU
	gWindowsMenu=SafeDisposeMenu(gWindowsMenu);
#endif
}

void AdjustMenus(void)
{
	short			kind;
	WindowRef		front, frontDoc, mainWindow;
	short			i;
#if USE_WINDOWS_MENU
	short			numOpenWindows;
#endif
	
	front=FrontWindow();
	kind=front ? ((WindowPeek)front)->windowKind : 0;
	frontDoc=GetFrontDocumentWindow();
	mainWindow=GetIndWindowRef(kMainWindow);
#if USE_WINDOWS_MENU
	numOpenWindows=GetNumberOfOpenWindows();
#endif
	
	EDItem(gAppleMenu, 0, TRUE);
	EDItem(gAppleMenu, iAbout, TRUE);
	EDItem(gAppleMenu, iOtherProducts, TRUE);
	EDItem(gAppleMenu, iHelp, TRUE);
	
	EDItem(gFileMenu, 0, TRUE);
	EDItem(gFileMenu, iNew, TRUE);
	EDItem(gFileMenu, iClose, (frontDoc!=0L));
#if USE_SAVE
	EDItem(gFileMenu, iOpen, TRUE);
	EDItem(gFileMenu, iSave, (mainWindow!=0L) && (WindowHasLayer(mainWindow)) &&
		(WindowIsModifiedQQ(mainWindow)));
	EDItem(gFileMenu, iSaveAs, mainWindow!=0L);
#endif
#if USE_PRINTING
	EDItem(gFileMenu, iPageSetup, TRUE);
	EDItem(gFileMenu, iPrint, (frontDoc!=0L) && (WindowHasLayer(frontDoc)) &&
		(WindowIsPrintableQQ(frontDoc)));
#endif
	EDItem(gFileMenu, iQuit, TRUE);
	
	EDItem(gEditMenu, 0, TRUE);
//	EDItem(gEditMenu, iUndo, (kind<0) || ((mainWindow!=0L) && (frontDoc==mainWindow) &&
//		(gGameStatus==kGameInProgress)));
	EDItem(gEditMenu, iUndo, kind<0);
	EDItem(gEditMenu, iCut, kind<0);
	EDItem(gEditMenu, iCopy, kind<0);
	EDItem(gEditMenu, iPaste, kind<0);
	EDItem(gEditMenu, iClear, kind<0);
	EDItem(gEditMenu, iSelectAll, kind<0);
	
	EDItem(gOptionsMenu, 0, TRUE);
	EDItem(gOptionsMenu, iSound, gSoundAvailable);
	EDItem(gOptionsMenu, iMusic, gSoundAvailable);
	EDItem(gOptionsMenu, iMessages, TRUE);
	EDItem(gOptionsMenu, iAnimation, TRUE);
	EDItem(gOptionsMenu, iMisere, (gGameStatus!=kGameInProgress));
	
	CheckItem(gOptionsMenu, iSound, gSoundToggle&&gSoundAvailable);
	CheckItem(gOptionsMenu, iMusic, gMusicToggle&&gSoundAvailable);
	CheckItem(gOptionsMenu, iMessages, gDisplayMessages);
	CheckItem(gOptionsMenu, iAnimation, gUseAnimation);
	CheckItem(gOptionsMenu, iMisere, gMisere);
	SetItemMark(gOptionsMenu, iHumanHumanMode, (gPlayerMode==kHumanHumanMode) ? '�' : noMark);
	SetItemMark(gOptionsMenu, iMacHumanMode, (gPlayerMode==kMacHumanMode) ? '�' : noMark);
	SetItemMark(gOptionsMenu, iHumanMacMode, (gPlayerMode==kHumanMacMode) ? '�' : noMark);
	SetItemMark(gOptionsMenu, iMacMacMode, (gPlayerMode==kMacMacMode) ? '�' : noMark);
	EDItem(gOptionsMenu, iHumanHumanMode, gGameStatus!=kGameInProgress);
	EDItem(gOptionsMenu, iMacHumanMode, gGameStatus!=kGameInProgress);
	EDItem(gOptionsMenu, iHumanMacMode, gGameStatus!=kGameInProgress);
	EDItem(gOptionsMenu, iMacMacMode, gGameStatus!=kGameInProgress);
	
	EDItem(gGameMenu, 0, TRUE);
	EDItem(gSetupMenu, 0, TRUE);
	for (i=1; i<=CountMItems(gGameMenu); i++)
	{
		CheckItem(gGameMenu, i, (gGameType==i-1));
		/* SetItemMark(gGameMenu, i, (gGameType==i-1) ? '�' : noMark); */
		/* EDItem(gGameMenu, i, (mainWindow==0L)); */
	}

#if USE_WINDOWS_MENU
	EDItem(gWindowsMenu, 0, !gInProgress);
	EDItem(gWindowsMenu, iArrange, numOpenWindows>0);
	EDItem(gWindowsMenu, iZoom, (frontDoc!=0L) && (WindowIsZoomableQQ(frontDoc)));
	EDItem(gWindowsMenu, iSendToBack, numOpenWindows>1);
	AdjustWindowsMenu(frontDoc);
#endif
}

void HandleMenu(long mSelect)
{
	short			menuID = HiWord(mSelect);
	short			menuItem = LoWord(mSelect);
	
	switch (menuID)
	{
		case mApple:
			HandleAppleMenu(menuItem);
			break;
		case mFile:
			HandleFileMenu(menuItem);
			break;	
		case mEdit:
			HandleEditMenu(menuItem, FALSE);
			break;
		case mOptions:
			HandleOptionsMenu(menuItem);
			break;
		case mGame:
			HandleGameMenu(menuItem);
			break;
		case mSetup:
			HandleSetupMenu(menuItem);
			break;
#if USE_WINDOWS_MENU
		case mWindows:
			HandleWindowsMenu(menuItem);
			break;
#endif
	}
}

void DoTheCloseThing(WindowPeek theWindow)
/* a standard close procedure, called when "close" is chosen from File menu or when
   a window is closed through its close box */
{
	short			kind;
	
	if (theWindow==0L)
		return;
	
	kind = theWindow ? theWindow->windowKind : 0;
	if (kind<0)		/* DA window or other system window */
		CloseDeskAcc(kind);
	else
	{
		if (WindowHasLayer((WindowRef)theWindow))
		{
			if (OptionKeyWasDown())
			{
				while ((theWindow=(WindowPeek)GetFrontDocumentWindow())!=0L)
					CloseTheWindow((WindowRef)theWindow);
			}
			else
			{
				CloseTheWindow((WindowRef)theWindow);
			}
		}
		else
		{
			DisposeWindow((WindowRef)theWindow);
		}
		
		AdjustMenus();
	}
}

static	void HandleAppleMenu(short menuItem)
{
	GrafPtr		savePort;
	Str255		name;
	
	switch (menuItem)
	{
		case iAbout:
			if (!IndWindowExistsQQ(kAboutWindow))
				OpenTheIndWindow(kAboutWindow, kOpenOldIfPossible);
			else
				MySelectWindow(GetIndWindowRef(kAboutWindow));
			break;
		case iOtherProducts:
			if (!IndWindowExistsQQ(kOtherProductsWindow))
				OpenTheIndWindow(kOtherProductsWindow, kOpenOldIfPossible);
			else
				MySelectWindow(GetIndWindowRef(kOtherProductsWindow));
			break;
		case iHelp:
			if (!IndWindowExistsQQ(kHelpWindow))
				OpenTheIndWindow(kHelpWindow, kOpenOldIfPossible);
			else
				MySelectWindow(GetIndWindowRef(kHelpWindow));
			break;
		default:
			if (menuItem > iHelp+1)
			{
				GetPort(&savePort);
				GetItem(gAppleMenu, menuItem, name);
				OpenDeskAcc(name);
				SetPort(savePort);
			}
			break;
	}
}

static	void HandleFileMenu(short menuItem)
{
	WindowRef		theWindow;
	TEHandle		hTE;
	
	theWindow=GetIndWindowRef(kMainWindow);
	switch (menuItem)
	{
		case iNew:
			NewGame(TRUE);
			break;
#if USE_SAVE
		case iOpen:
			LoadSaveDispatch(TRUE, FALSE);
			break;
#endif
		case iClose:
			if ((theWindow=GetFrontDocumentWindow())!=0L)
				DoTheCloseThing((WindowPeek)theWindow);
			else
				DoTheCloseThing((WindowPeek)FrontWindow());
			break;
#if USE_SAVE
		case iSave:
			LoadSaveDispatch(FALSE, TRUE);
			break;
		case iSaveAs:
			LoadSaveDispatch(FALSE, FALSE);
			break;
#endif
#if USE_PRINTING
		case iPageSetup:
			DeactivateFloatersAndFirstDocumentWindow();
			DoThePageSetup();
			ActivateFloatersAndFirstDocumentWindow();
			break;
		case iPrint:
			theWindow=GetFrontDocumentWindow();
			if ((WindowHasLayer(theWindow)) && (WindowIsPrintableQQ(theWindow)))
			{
				hTE=GetWindowTE(theWindow);
				if (hTE!=0L)
				{
					DeactivateFloatersAndFirstDocumentWindow();
					PrintText(hTE);
					ActivateFloatersAndFirstDocumentWindow();
				}
			}
			else SysBeep(7);
			break;
#endif
		case iQuit:
			gDone=ShutDownTheProgram();
			break;
	}
}

static	void HandleEditMenu(short menuItem, Boolean alreadyPassedThrough)
{
	WindowRef		theWindow;
	DispatchError	resultCode;
	
	if (!alreadyPassedThrough)
		theWindow=FrontWindow();
	else
	{
		theWindow=GetFrontDocumentWindow();
	}
	
	if (theWindow!=0L)
	{
		switch (menuItem)
		{
			case iUndo:			resultCode=UndoDispatch(theWindow);			break;
			case iCut:			resultCode=CutDispatch(theWindow);			break;
			case iCopy:			resultCode=CopyDispatch(theWindow);			break;
			case iPaste:		resultCode=PasteDispatch(theWindow);		break;
			case iClear:		resultCode=ClearDispatch(theWindow);		break;
			case iSelectAll:	resultCode=SelectAllDispatch(theWindow);	break;
			default:			resultCode=kSuccess;						break;
		}
		
		if ((resultCode==kPassThrough) && (!alreadyPassedThrough))
			HandleEditMenu(menuItem, TRUE);
	}
	else SystemEdit(menuItem-1);
}

static	void HandleOptionsMenu(short menuItem)
{
	switch (menuItem)
	{
		case iSound:
			gSoundToggle=!gSoundToggle;
			DoSound(sound_on, TRUE);
			break;
		case iMusic:
			gMusicToggle=!gMusicToggle;
			if (gMusicToggle)
				StartTheMusic();
			else
				CloseTheMusicChannel();
			break;
		case iMessages:
			gDisplayMessages=!gDisplayMessages;
			break;
		case iAnimation:
			gUseAnimation=!gUseAnimation;
			break;
		case iMisere:
			gMisere=!gMisere;
			break;
		case iHumanHumanMode:
			gPlayerMode=kHumanHumanMode;
			break;
		case iMacHumanMode:
			gPlayerMode=kMacHumanMode;
			break;
		case iHumanMacMode:
			gPlayerMode=kHumanMacMode;
			break;
		case iMacMacMode:
			gPlayerMode=kMacMacMode;
			break;
	}
}

static	void HandleGameMenu(short menuItem)
{
	Boolean			isSameGame;
	
	isSameGame=(gGameType==menuItem-1);
	gGameType=menuItem-1;
	NewGame(isSameGame);
}

static	void HandleSetupMenu(short menuItem)
{
	short			oldGameType;
	
	switch (menuItem)
	{
		case iNim:
			DoNimSetup();
			break;
		case iPrimeNim:
			DoPrimeSetup();
			break;
		case iColumnsNim:
			DoColumnsSetup();
			break;
		case iTurnablock:
			DoTurnSetup();
			break;
		case iSilver:
			DoSilverSetup();
			break;
		case iCornerTheQueen:
		case iCornerTheKing:
		case iCornerTheSuperqueen:
		case iCornerTheSuperking:
			oldGameType=gGameType;
			gGameType=menuItem-iCornerTheQueen+kCornerTheQueen;
			DoCornerSetup();
			gGameType=oldGameType;
			break;
	}
}

#if USE_WINDOWS_MENU
static	void HandleWindowsMenu(short menuItem)
{
	switch (menuItem)
	{
		case iArrange:
			ArrangeWindows();
			break;
		case iZoom:
			MyZoomWindow(GetFrontDocumentWindow(), -1);
			break;
		case iSendToBack:
			MySendBehind(GetFrontDocumentWindow(), 0L);
			break;
		default:
			SelectWindowsMenuItem(menuItem);
			break;
	}
}
#endif

static	void EDItem(MenuHandle theMenu, short theItem, Boolean theCondition)
{
	if (theCondition)
		EnableItem(theMenu, theItem);
	else
		DisableItem(theMenu, theItem);
}
