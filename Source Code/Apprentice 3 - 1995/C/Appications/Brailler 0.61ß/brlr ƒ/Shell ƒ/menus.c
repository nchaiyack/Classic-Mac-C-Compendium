#include "menus.h"
#include "graphics.h"
#include "help.h"
#include "environment.h"
#include "print meat.h"
#include "brlr load-save.h"
#include "program init.h"
#include "text twiddling.h"
#include "resource utilities.h"
#include "graphics dispatch.h"
#include "window layer.h"
#include "program globals.h"

static	MenuHandle		gAppleMenu;
static	MenuHandle		gFileMenu;
static	MenuHandle		gEditMenu;
static	MenuHandle		gOptionsMenu;

enum
{
	appleMenu = 400, fileMenu, editMenu, optionsMenu,
	
	aboutItem = 1, otherProductsItem, helpPointerItem,
	
	newItem = 1, openItem, file_unused1, closeItem, saveItem, saveAsItem, file_unused2,
		pageSetupItem, printItem, file_unused4, quitItem,
	
	undoItem = 1, edit_unused0, cutItem, copyItem, pasteItem, clearItem, edit_unused1, selectAllItem,
	
	useBrailleItem = 1, useLettersItem, useGrade2Item, options_unused1, dynamicScrollItem
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
	gAppleMenu=SafeDisposeMenu(gAppleMenu);
	gFileMenu=SafeDisposeMenu(gFileMenu);
	gEditMenu=SafeDisposeMenu(gEditMenu);
	gOptionsMenu=SafeDisposeMenu(gOptionsMenu);
}

void AdjustMenus(void)
{
	short			kind;
	WindowPtr		front, frontDoc, mainWindow;
	
	front=FrontWindow();
	kind=front ? ((WindowPeek)front)->windowKind : 0;
	frontDoc=GetFrontDocumentWindow();
	mainWindow=GetIndWindowPtr(kMainWindow);
	
	EDItem(gAppleMenu, 0, TRUE);
	EDItem(gAppleMenu, aboutItem, TRUE);
	EDItem(gAppleMenu, otherProductsItem, TRUE);
	EDItem(gAppleMenu, helpPointerItem, TRUE);
	
	EDItem(gFileMenu, 0, TRUE);
	EDItem(gFileMenu, newItem, mainWindow==0L);
	EDItem(gFileMenu, openItem, mainWindow==0L);
	EDItem(gFileMenu, closeItem, front!=0L);
	EDItem(gFileMenu, saveItem, (mainWindow!=0L) && (WindowHasLayer(mainWindow)) &&
		(WindowIsModifiedQQ(mainWindow)));
	EDItem(gFileMenu, saveAsItem, mainWindow!=0L);
	EDItem(gFileMenu, pageSetupItem, TRUE);
	EDItem(gFileMenu, printItem, (frontDoc!=0L) && (WindowHasLayer(frontDoc)) &&
		(WindowIsPrintableQQ(frontDoc)));
	EDItem(gFileMenu, quitItem, TRUE);
	
	EDItem(gEditMenu, 0, TRUE);
	EDItem(gEditMenu, undoItem, kind<0);
	EDItem(gEditMenu, cutItem, (frontDoc!=0L) && (frontDoc==mainWindow) && (AnyHighlightedQQ(frontDoc)));
	EDItem(gEditMenu, copyItem, (frontDoc!=0L) && (frontDoc==mainWindow) && (AnyHighlightedQQ(frontDoc)));
	EDItem(gEditMenu, pasteItem, (frontDoc!=0L) && (frontDoc==mainWindow) && (AnyTextInScrapQQ()));
	EDItem(gEditMenu, clearItem, (frontDoc!=0L) && (frontDoc==mainWindow) && (AnyHighlightedQQ(frontDoc)));
	EDItem(gEditMenu, selectAllItem, (frontDoc!=0L) && (frontDoc==mainWindow) && (AnyTextQQ(frontDoc)));
	
	EDItem(gOptionsMenu, 0, TRUE);
	EDItem(gOptionsMenu, useBrailleItem, mainWindow!=0L);
	EDItem(gOptionsMenu, useLettersItem, mainWindow!=0L);
	EDItem(gOptionsMenu, useGrade2Item, mainWindow!=0L);
	EDItem(gOptionsMenu, dynamicScrollItem, TRUE);
	
	SetItemMark(gOptionsMenu, useBrailleItem, (gGrade==0) ? '�' : noMark);
	SetItemMark(gOptionsMenu, useLettersItem, (gGrade==1) ? '�' : noMark);
	SetItemMark(gOptionsMenu, useGrade2Item, (gGrade==2) ? '�' : noMark);
	CheckItem(gOptionsMenu, dynamicScrollItem, (gDynamicScroll) ? TRUE : FALSE);
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

static	void HandleAppleMenu(short menuItem)
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
		case otherProductsItem:
			if (!IndWindowExistsQQ(kOtherProductsWindow))
				OpenTheIndWindow(kOtherProductsWindow);
			else
				MySelectWindow(GetIndWindowPtr(kOtherProductsWindow));
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

static	void HandleFileMenu(short menuItem)
{
	WindowPtr			theWindow;
	TEHandle			hTE;
	
	switch (menuItem)
	{
		case newItem:
			OpenTheIndWindow(kMainWindow);
			break;
		case openItem:
			LoadSaveDispatch(TRUE, FALSE);
			break;
		case closeItem:
			if ((theWindow=GetFrontDocumentWindow())!=0L)
				DoTheCloseThing((WindowPeek)theWindow);
			else
				DoTheCloseThing((WindowPeek)FrontWindow());
			break;
		case saveItem:
			LoadSaveDispatch(FALSE, TRUE);
			break;
		case saveAsItem:
			LoadSaveDispatch(FALSE, FALSE);
			break;
		case pageSetupItem:
			DeactivateFloatersAndFirstDocumentWindow();
			DoThePageSetup();
			ActivateFloatersAndFirstDocumentWindow();
			break;
		case printItem:
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
		case quitItem:
			gDone=ShutDownTheProgram();
			break;
	}
}

static	void HandleEditMenu(short menuItem, Boolean alreadyPassedThrough)
{
	short			index;
	WindowPtr		frontDoc;
	enum DispatchError	resultCode;
	
	if (!alreadyPassedThrough)
		index=gFrontWindowIndex;
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

static	void HandleOptionsMenu(short menuItem)
{
	switch (menuItem)
	{
		case useBrailleItem:
			gGrade=0;
			if (IndWindowExistsQQ(kFloatingWindow))
				CloseTheWindow(GetIndWindowPtr(kFloatingWindow));
			break;
		case useLettersItem:
			gGrade=1;
			if (IndWindowExistsQQ(kFloatingWindow))
				CloseTheWindow(GetIndWindowPtr(kFloatingWindow));
			break;
		case useGrade2Item:
			gGrade=2;
			if (!IndWindowExistsQQ(kFloatingWindow))
				OpenTheIndWindow(kFloatingWindow);
			break;
		case dynamicScrollItem:
			gDynamicScroll=!gDynamicScroll;
			break;
	}
	
	AdjustMenus();
}

static	void EDItem(MenuHandle theMenu, short theItem, Boolean theCondition)
{
	if (theCondition)
		EnableItem(theMenu, theItem);
	else
		DisableItem(theMenu, theItem);
}
