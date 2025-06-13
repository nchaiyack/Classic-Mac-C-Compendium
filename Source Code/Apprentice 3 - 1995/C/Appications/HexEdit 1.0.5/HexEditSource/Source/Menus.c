/*********************************************************************
 * Menus.c
 *
 * HexEdit, a simple hex editor
 * copyright 1993, Jim Bumgardner
 *********************************************************************/
#include "HexEdit.h"

// Menu Handles
MenuHandle	gAppleMenu, gFileMenu, gEditMenu, gFindMenu, gOptionsMenu;



// Menu Initialization code.

void MySetUpMenus(void)
{
	Handle	myMenuBar;
	myMenuBar = GetNewMBar(MenuBaseID);
	SetMenuBar(myMenuBar);

	gAppleMenu = GetMHandle(AppleMENU);
	gFileMenu = GetMHandle(FileMENU);
	gEditMenu = GetMHandle(EditMENU);
	gFindMenu = GetMHandle(FindMENU);
	gOptionsMenu = GetMHandle(OptionsMENU);

	AddResMenu(gAppleMenu, 'DRVR');

	DrawMenuBar();
}


//	Enable or disable the items in the Edit menu if a DA window
//	comes up or goes away. Our application doesn't do anything with
//	the Edit menu.

int MyEnableMenuItem (MenuHandle menu, short item, short ok);

void MyAdjustMenus(void)
{
	register WindowPeek wp;
	short 				windowKind;
	Boolean 			isDA,isObjectWindow,selection,scrapExists,undoExists;
	EditWindowPtr		dWin;

	wp = (WindowPeek) FrontWindow();
	dWin = (EditWindowPtr) wp;
	windowKind = wp ? wp->windowKind : 0;
	isDA = (windowKind < 0);
	isObjectWindow = (wp? wp->refCon == MyWindowID : false);

	selection = (isObjectWindow && 
			((EditWindowPtr) wp)->endSel > ((EditWindowPtr) wp)->startSel);

	scrapExists = (isObjectWindow && gScrapChunk != NULL);

	undoExists = (gUndoRec.type != 0);

	MyEnableMenuItem(gEditMenu, 0, wp != NULL);
	MyEnableMenuItem(gEditMenu, EM_Undo, isDA || undoExists);
	MyEnableMenuItem(gEditMenu, EM_Cut, isDA || selection);
	MyEnableMenuItem(gEditMenu, EM_Copy, isDA || selection);
	MyEnableMenuItem(gEditMenu, EM_Paste, isDA || scrapExists);
	MyEnableMenuItem(gEditMenu, EM_Clear, isDA || selection);

	MyEnableMenuItem(gEditMenu, EM_SelectAll, isDA || isObjectWindow);

	// MyEnableMenuItem(gFileMenu, FM_Open, true);
	if (isObjectWindow && dWin->startSel < dWin->endSel)
		SetItem(gFileMenu, FM_Print, "\pPrint SelectionÉ");
	else
		SetItem(gFileMenu, FM_Print, "\pPrintÉ");

	MyEnableMenuItem(gFileMenu, FM_Print, isObjectWindow);
	MyEnableMenuItem(gFileMenu, FM_Close, isDA || isObjectWindow);
	MyEnableMenuItem(gFileMenu, FM_Save, isObjectWindow && dWin->dirtyFlag);
	MyEnableMenuItem(gFileMenu, FM_SaveAs, isObjectWindow);
	MyEnableMenuItem(gFileMenu, FM_Revert, isObjectWindow && dWin->refNum &&
										dWin->dirtyFlag);

	MyEnableMenuItem(gFindMenu, 0, isObjectWindow);
	MyEnableMenuItem(gFindMenu, SM_Find, isObjectWindow);
	MyEnableMenuItem(gFindMenu, SM_FindForward, isObjectWindow);
	MyEnableMenuItem(gFindMenu, SM_FindBackward, isObjectWindow);
	MyEnableMenuItem(gFindMenu, SM_GotoAddress, isObjectWindow);

	CheckItem(gOptionsMenu, OM_HiAscii, gPrefs.asciiMode == AM_Hi);
	CheckItem(gOptionsMenu, OM_DecimalAddr, gPrefs.decimalAddr);
	CheckItem(gOptionsMenu, OM_Backups, gPrefs.backupFlag);
	CheckItem(gOptionsMenu, OM_Overwrite, gOverwrite);
}


// Code to simplify enabling/disabling menu items.
//
static MyEnableMenuItem(MenuHandle menu, short item, short ok)
{
	if (ok)
		EnableItem(menu, item);
	else
		DisableItem(menu, item);
	if (item == 0)
		DrawMenuBar();
}


//	Handle the menu selection. mSelect is what MenuSelect() and
//	MenuKey() return: the high word is the menu ID, the low word
//	is the menu item
//
void MyHandleMenu (long mSelect)

{
	int			menuID = HiWord(mSelect);
	int			menuItem = LoWord(mSelect);
	Str255		name;
	GrafPtr		savePort;
	WindowPeek	frontWindow;
	EditWindowPtr	dWin;

	switch (menuID) {
	case AppleMENU:
		if (menuItem == AM_About) {
			HexEditAboutBox();
			// ErrorAlert(ES_Message,"HexEdit v1.0.2\r\r©1993 Jim Bumgardner\rjbum@netcom.com");
		}
		else {
			GetPort(&savePort);
			GetItem(gAppleMenu, menuItem, name);
			OpenDeskAcc(name);
			SetPort(savePort);
		}
		break;
	case FileMENU:
		switch (menuItem) {
		case FM_New:
			NewEditWindow();
			break;
		case FM_Open:
			AskEditWindow();
			break;
		case FM_Save:
			if ((frontWindow = (WindowPeek) FrontWindow()) == 0L)
				break;
			if (frontWindow->refCon == MyWindowID &&
				((ObjectWindowPtr) frontWindow)->Save) {
				((ObjectWindowPtr) frontWindow)->Save((WindowPtr) frontWindow);
			}
			break;
		case FM_SaveAs:
			if ((frontWindow = (WindowPeek) FrontWindow()) == 0L)
				break;
			if (frontWindow->refCon == MyWindowID &&
				((ObjectWindowPtr) frontWindow)->SaveAs) {
				((ObjectWindowPtr) frontWindow)->SaveAs((WindowPtr) frontWindow);
			}
			break;
		case FM_Revert:
			if ((frontWindow = (WindowPeek) FrontWindow()) == 0L)
				break;
			if (frontWindow->refCon == MyWindowID &&
				((ObjectWindowPtr) frontWindow)->Revert) {
				((ObjectWindowPtr) frontWindow)->Revert((WindowPtr) frontWindow);
			}
			break;

		case FM_Close:
			if ((frontWindow = (WindowPeek) FrontWindow()) == 0L)
				break;
 			if (frontWindow->windowKind < 0)
			 	CloseDeskAcc(frontWindow->windowKind);
			else if (frontWindow->refCon == MyWindowID) {
				CloseEditWindow((WindowPtr) frontWindow);
			}
			else if ((WindowPtr) frontWindow == gSearchWin) {
				DisposDialog(gSearchWin);
				gSearchWin = NULL;
			}

  			break;

		case FM_Quit:
			if (CloseAllEditWindows())
				gQuitFlag = true;
			break;
		case FM_PageSetup:
			PrOpen();
			PrStlDialog(gHPrint);
			PrClose();
			break;
		case FM_Print:
			dWin = (EditWindowPtr) FrontWindow();
			PrintWindow(dWin);
			break;
		}
		break;
	case EditMENU:
		if (!SystemEdit(menuItem-1)) {
			dWin = (EditWindowPtr) FrontWindow();
			switch (menuItem) {
			case EM_Undo:
				UndoOperation();
				break;
			case EM_Cut:
				CutSelection(dWin);				
				break;
			case EM_Copy:	
				CopySelection(dWin);	
				break;
			case EM_Paste:	
				PasteSelection(dWin);	
				break;
			case EM_Clear:
				ClearSelection(dWin);			
				break;
			case EM_SelectAll:
				dWin = (EditWindowPtr) FrontWindow();
				dWin->startSel = 0;
				dWin->endSel = dWin->fileSize;
				UpdateOnscreen((WindowPtr) dWin);
				break;
			}
		}
		break;
	case FindMENU:
		dWin = (EditWindowPtr) FrontWindow();
		if (((WindowPeek) dWin)->refCon == MyWindowID) {
			switch (menuItem) {
			case SM_Find:
				OpenSearchDialog(dWin);
				break;
			case SM_FindForward:
				gSearchDir = 0;
				PerformTextSearch(dWin);
				break;
			case SM_FindBackward:
				gSearchDir = 1;
				PerformTextSearch(dWin);
				break;
			case SM_GotoAddress:
				GotoAddress(dWin);
				break;
			}
		}
		break;
	case OptionsMENU:
		switch (menuItem) {
		case OM_HiAscii:
			gPrefs.asciiMode = !gPrefs.asciiMode;
			if (gPrefs.asciiMode)
				gHighChar = 255;
			else
				gHighChar = '~';
			dWin = (EditWindowPtr) FrontWindow();
			if (((WindowPeek) dWin)->refCon == MyWindowID) {
				DrawPage(dWin);
				UpdateOnscreen((WindowPtr) dWin);
			}
			break;
		case OM_DecimalAddr:
			gPrefs.decimalAddr = !gPrefs.decimalAddr;
			dWin = (EditWindowPtr) FrontWindow();
			if (((WindowPeek) dWin)->refCon == MyWindowID) {
				DrawPage(dWin);
				UpdateOnscreen((WindowPtr) dWin);
			}
			break;
		case OM_Backups:
			gPrefs.backupFlag = !gPrefs.backupFlag;
			break;
		case OM_Overwrite:
			gOverwrite = !gOverwrite;
			break;
		}
		break;
	}
	HiliteMenu(0);
	MyAdjustMenus();
}

