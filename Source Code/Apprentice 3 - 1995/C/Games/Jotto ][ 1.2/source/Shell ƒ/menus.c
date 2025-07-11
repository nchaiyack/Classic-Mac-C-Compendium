#include "menus.h"
#include "graphics.h"
#include "help.h"
#include "environment.h"
#include "print meat.h"
#include "sounds.h"
#include "jotto load-save.h"
#include "jotto.h"
#include "jotto environment.h"
#include "text twiddling.h"
#include "graphics dispatch.h"
#include "window layer.h"
#include "program globals.h"

static	MenuHandle		gAppleMenu=0L;
static	MenuHandle		gFileMenu=0L;
static	MenuHandle		gEditMenu=0L;
static	MenuHandle		gOptionsMenu=0L;

enum
{
	appleMenu = 400, fileMenu, editMenu, optionsMenu,
	
	aboutItem = 1, aboutMSGItem, otherMSGItem, helpPointerItem,
	
	newItem = 1, openItem, file_unused1, closeItem, saveItem, saveAsItem, file_unused2,
		pageSetupItem, printItem, file_unused3, quitItem,
	
	undoItem = 1, edit_unused0, cutItem, copyItem, pasteItem, clearItem, edit_unused1, selectAllItem,
	
	useFiveLetter = 1, useSixLetter, options_unused1, dupToggle, nonWordsToggle,
		soundToggle, animationToggle, options_unused2, showNotePad, boardFrontItem
};

/*-----------------------------------------------------------------------------------*/
/* internal stuff for menus.c                                                        */

static	void HandleAppleMenu(short menuItem);
static	void HandleFileMenu(short menuItem);
static	void HandleEditMenu(short menuItem, Boolean alreadyPassedThrough);
static	void HandleOptionsMenu(short menuItem);
static	void EDItem(MenuHandle theMenu, short theItem, Boolean theCondition);

Boolean InitTheMenus(void)
{
	Handle			MBARHandle;
	
	if ((MBARHandle=GetNewMBar(400))==0L)		/* sez which menus are in menu bar. */
		return FALSE;
	SetMenuBar(MBARHandle);						/* set this to be THE menu bar to use. */
	
	if ((gAppleMenu=GetMHandle(appleMenu))==0L)	/* GetNewMBar also got menu handles of */
		return FALSE;
	if ((gFileMenu=GetMHandle(fileMenu))==0L)	/* every menu it includes, so just */
		return FALSE;
	if ((gEditMenu=GetMHandle(editMenu))==0L)	/* grab these handles and assign them */
		return FALSE;
	if ((gOptionsMenu=GetMHandle(optionsMenu))==0L)
		return FALSE;
	
	AddResMenu(gAppleMenu, 'DRVR');				/* adds control panels to apple menu */
	
	AdjustMenus();								/* dim/enable/check/mark menus/items */
	DrawMenuBar();								/* draws the actual menu bar */
	
	return TRUE;
}

void ShutDownTheMenus(void)
{
	if (gAppleMenu!=0L)
		ReleaseResource((Handle)gAppleMenu);
	if (gFileMenu!=0L)
		ReleaseResource((Handle)gFileMenu);
	if (gEditMenu!=0L)
		ReleaseResource((Handle)gEditMenu);
	if (gOptionsMenu!=0L)
		ReleaseResource((Handle)gOptionsMenu);
}

void AdjustMenus(void)
{
	short			kind;
	WindowPtr		front, frontDoc, mainWindow, notePad;
	
	front=FrontWindow();
	kind=front ? ((WindowPeek)front)->windowKind : 0;
	frontDoc=GetFrontDocumentWindow();
	mainWindow=GetIndWindowPtr(kMainWindow);
	notePad=GetIndWindowPtr(kNotePad);
	
	EDItem(gAppleMenu, 0, TRUE);
	EDItem(gAppleMenu, aboutItem, TRUE);
	EDItem(gAppleMenu, aboutMSGItem, TRUE);
	EDItem(gAppleMenu, otherMSGItem, TRUE);
	EDItem(gAppleMenu, helpPointerItem, TRUE);
	
	EDItem(gFileMenu, 0, TRUE);
	EDItem(gFileMenu, newItem, TRUE);
	EDItem(gFileMenu, openItem, TRUE);
	EDItem(gFileMenu, closeItem, front!=0L);
	EDItem(gFileMenu, saveItem, (mainWindow!=0L) && (WindowHasLayer(mainWindow)) &&
		(WindowIsModifiedQQ(mainWindow)));
	EDItem(gFileMenu, saveAsItem, mainWindow!=0L);
	EDItem(gFileMenu, pageSetupItem, TRUE);
	EDItem(gFileMenu, printItem, (frontDoc!=0L) && (WindowHasLayer(frontDoc)) &&
		(GetWindowTE(frontDoc)!=0L));
	EDItem(gFileMenu, quitItem, TRUE);
	
	EDItem(gEditMenu, 0, TRUE);
	EDItem(gEditMenu, undoItem, kind<0);
	EDItem(gEditMenu, cutItem, (frontDoc!=0L) && (frontDoc==notePad) && (AnyHighlightedQQ(frontDoc)));
	EDItem(gEditMenu, copyItem, (frontDoc!=0L) && (frontDoc==notePad) && (AnyHighlightedQQ(frontDoc)));
	EDItem(gEditMenu, pasteItem, (frontDoc!=0L) && (frontDoc==notePad) && (AnyTextInScrapQQ()));
	EDItem(gEditMenu, clearItem, (frontDoc!=0L) && (frontDoc==notePad) && (AnyHighlightedQQ(frontDoc)));
	EDItem(gEditMenu, selectAllItem, (frontDoc!=0L) && (frontDoc==notePad) && (AnyTextQQ(frontDoc)));
	
	EDItem(gOptionsMenu, 0, TRUE);
	EDItem(gOptionsMenu, useFiveLetter, (FiveLetterOKQQ()) && (mainWindow==0L));
	EDItem(gOptionsMenu, useSixLetter, (SixLetterOKQQ() && (mainWindow==0L)));
	EDItem(gOptionsMenu, dupToggle, mainWindow==0L);
	EDItem(gOptionsMenu, nonWordsToggle, mainWindow==0L);
	EDItem(gOptionsMenu, soundToggle, gSoundAvailable);
	EDItem(gOptionsMenu, animationToggle, TRUE);
	EDItem(gOptionsMenu, showNotePad, TRUE);
	EDItem(gOptionsMenu, boardFrontItem, (frontDoc!=0L) &&
		(((mainWindow!=0L) && (mainWindow!=frontDoc)) ||
		 ((notePad!=0L) && (notePad!=frontDoc))));
	
	if ((frontDoc!=0L) &&
		(((mainWindow==0L) || (mainWindow==frontDoc)) && (notePad!=0L)))
	{
		SetItem(gOptionsMenu, boardFrontItem, "\pBring note pad to front");
	}
	else
	{
		SetItem(gOptionsMenu, boardFrontItem, "\pBring board to front");
	}
	
	CheckItem(gOptionsMenu, dupToggle, gAllowDup);
	CheckItem(gOptionsMenu, nonWordsToggle, gNonWordsCount);
	CheckItem(gOptionsMenu, animationToggle, gAnimation);
	SetItemMark(gOptionsMenu, useFiveLetter, (gNumLetters==5) ? '�' : noMark);
	SetItemMark(gOptionsMenu, useSixLetter, (gNumLetters==6) ? '�' : noMark);
	CheckItem(gOptionsMenu, showNotePad, (notePad!=0L));
	CheckItem(gOptionsMenu, soundToggle, gSoundToggle&&gSoundAvailable);
}

void HandleMenu(long mSelect)
{
	short			menuID = HiWord(mSelect);
	short			menuItem = LoWord(mSelect);
	
	switch (menuID)
	{
		case appleMenu:
			HandleAppleMenu(menuItem);
			break;
		case fileMenu:
			HandleFileMenu(menuItem);
			break;	
		case editMenu:
			HandleEditMenu(menuItem, FALSE);
			break;
		case optionsMenu:
			HandleOptionsMenu(menuItem);
			break;
	}
}

void DoTheCloseThing(WindowPeek theWindow)
/* a standard close procedure, called when "close" is chosen from File menu and when
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
		if (WindowHasLayer((WindowPtr)theWindow))
			CloseTheWindow((WindowPtr)theWindow);
		else
			DisposeWindow((WindowPtr)theWindow);
		
		AdjustMenus();
	}
}

void HandleAppleMenu(short menuItem)
{
	GrafPtr		savePort;
	Str255		name;
	
	switch (menuItem)
	{
		case aboutItem:
			if (!IndWindowExistsQQ(kAboutWindow))
				OpenTheIndWindow(kAboutWindow);
			else
				MySelectWindow(GetIndWindowPtr(kAboutWindow));
			break;
		case aboutMSGItem:
			if (!IndWindowExistsQQ(kAboutMSGWindow))
				OpenTheIndWindow(kAboutMSGWindow);
			else
				MySelectWindow(GetIndWindowPtr(kAboutMSGWindow));
			break;
		case otherMSGItem:
			if (!IndWindowExistsQQ(kOtherMSGWindow))
				OpenTheIndWindow(kOtherMSGWindow);
			else
				MySelectWindow(GetIndWindowPtr(kOtherMSGWindow));
			break;
		case helpPointerItem:
			if (!IndWindowExistsQQ(kHelpWindow))
				OpenTheIndWindow(kHelpWindow);
			else
				MySelectWindow(GetIndWindowPtr(kHelpWindow));
			break;
		default:
			if (menuItem > helpPointerItem+1)
			{
				GetPort(&savePort);
				GetItem(gAppleMenu, menuItem, name);
				OpenDeskAcc(name);
				SetPort(savePort);
			}
			break;
	}
}

void HandleFileMenu(short menuItem)
{
	WindowPtr			theWindow;
	TEHandle			hTE;
	
	switch (menuItem)
	{
		case newItem:
			NewGame();
			break;
		case openItem:
			LoadSaveDispatch(TRUE, FALSE, 0L);
			break;
		case closeItem:
			if ((theWindow=GetFrontDocumentWindow())!=0L)
				DoTheCloseThing((WindowPeek)theWindow);
			else
				DoTheCloseThing((WindowPeek)FrontWindow());
			break;
		case saveItem:
			LoadSaveDispatch(FALSE, TRUE, 0L);
			break;
		case saveAsItem:
			LoadSaveDispatch(FALSE, FALSE, 0L);
			break;
		case pageSetupItem:
			RemoveHilitePatch();
			DoThePageSetup();
			InstallHilitePatch();
			break;
		case printItem:
			theWindow=GetFrontDocumentWindow();
			if (WindowHasLayer(theWindow))
			{
				hTE=GetWindowTE(theWindow);
				if (hTE!=0L)
				{
					RemoveHilitePatch();
					PrintText(hTE);
					InstallHilitePatch();
				}
			}
			else SysBeep(7);
			break;
		case quitItem:
			gDone=ShutDownTheProgram();
			break;
	}
}

void HandleEditMenu(short menuItem, Boolean alreadyPassedThrough)
{
	short			index;
	WindowPtr		frontDoc;
	enum DispatchError	resultCode;
	
	if (!alreadyPassedThrough)
		index=(gFrontWindowIsOurs) ? gFrontWindowIndex : -1;
	else
	{
		frontDoc=GetFrontDocumentWindow();
		index=(frontDoc!=0L) ? GetWindowIndex(frontDoc) : -1;
	}
	
	if (index>=0)
	{
		switch (menuItem)
		{
			case undoItem:		resultCode=UndoDispatch(index);			break;
			case cutItem:		resultCode=CutDispatch(index);			break;
			case copyItem:		resultCode=CopyDispatch(index);			break;
			case pasteItem:		resultCode=PasteDispatch(index);		break;
			case clearItem:		resultCode=ClearDispatch(index);		break;
			case selectAllItem:	resultCode=SelectAllDispatch(index);	break;
			default:			resultCode=kSuccess;					break;
		}
		
		if ((resultCode==kPassThrough) && (!alreadyPassedThrough))
			HandleEditMenu(menuItem, TRUE);
	}
	else SystemEdit(menuItem-1);
}

void HandleOptionsMenu(short menuItem)
{
	WindowPtr			mainWindow, notePad;
	
	mainWindow=GetIndWindowPtr(kMainWindow);
	notePad=GetIndWindowPtr(kNotePad);
	
	switch (menuItem)
	{
		case dupToggle:
		case nonWordsToggle:
		case useFiveLetter:
		case useSixLetter:
		case animationToggle:
		case soundToggle:
			switch (menuItem)
			{
				case dupToggle:
					gAllowDup=!gAllowDup;
					break;
				case nonWordsToggle:
					gNonWordsCount=!gNonWordsCount;
					break;
				case useFiveLetter:
					gNumLetters=0x05;
					break;
				case useSixLetter:
					gNumLetters=0x06;
					break;
				case animationToggle:
					gAnimation=!gAnimation;
					break;
				case soundToggle:
					gSoundToggle=!gSoundToggle;
					DoSound(sound_on, TRUE);
					break;
			}
			break;
		case showNotePad:
			if (notePad!=0L)
			{
				MySelectWindow(notePad);
				CloseTheWindow(notePad);
			}
			else
			{
				OpenTheIndWindow(kNotePad);
			}
			break;
		case boardFrontItem:
			if (FrontWindow()!=0L)
			{
				if ((mainWindow!=0L) && (mainWindow!=GetFrontDocumentWindow()))
				{
					MySelectWindow(mainWindow);
				}
				else if (notePad!=0L)
				{
					MySelectWindow(notePad);
				}
				else DoSound(sound_fluff, TRUE);
			}
			else DoSound(sound_fluff, TRUE);
	}
}

static	void EDItem(MenuHandle theMenu, short theItem, Boolean theCondition)
{
	if (theCondition)
		EnableItem(theMenu, theItem);
	else
		DisableItem(theMenu, theItem);
}
