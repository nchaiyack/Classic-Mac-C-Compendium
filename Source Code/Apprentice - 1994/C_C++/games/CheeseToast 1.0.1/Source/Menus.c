/************************************************************************************
 * Menus.c
 *
 *	Routines for CheeseToast menus.
 *
 * Menus are unused in this game, except in Attract Mode
 ************************************************************************************/
#include "ObjectWindow.h"

extern Boolean	gDoneFlag;

// Menu Handles
MenuHandle	gAppleMenu, gFileMenu, gEditMenu;

#define MenuBaseID	128

// Menu Resource IDs
enum	{
	AppleMENU = 128, FileMENU, EditMENU
	};

// Menu Item Numbers
enum	{FM_Open = 1, FM_Close,	FM_Quit = 4};
enum 	{EM_Undo = 1, EM_Cut = 3, EM_Copy,
		 EM_Paste, EM_Clear};

// Menu Initialization code.

void MySetUpMenus(void)
{
	Handle	myMenuBar;
	myMenuBar = GetNewMBar(MenuBaseID);
	SetMenuBar(myMenuBar);

	gAppleMenu = GetMHandle(AppleMENU);
	gFileMenu = GetMHandle(FileMENU);
	gEditMenu = GetMHandle(EditMENU);

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
	short windowKind;
	Boolean isDA,isObjectWindow;

	wp = (WindowPeek) FrontWindow();
	windowKind = wp ? wp->windowKind : 0;
	isDA = (windowKind < 0);
	isObjectWindow = (wp->refCon == MyWindowID);

	MyEnableMenuItem(gEditMenu, EM_Undo, isDA);
	MyEnableMenuItem(gEditMenu, EM_Cut, isDA);
	MyEnableMenuItem(gEditMenu, EM_Copy, isDA);
	MyEnableMenuItem(gEditMenu, EM_Paste, isDA);
	MyEnableMenuItem(gEditMenu, EM_Clear, isDA);

	MyEnableMenuItem(gFileMenu, FM_Open, true);
	MyEnableMenuItem(gFileMenu, FM_Close, isDA || isObjectWindow);
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
	
	switch (menuID) {
	case AppleMENU:
		GetPort(&savePort);
		GetItem(gAppleMenu, menuItem, name);
		OpenDeskAcc(name);
		SetPort(savePort);
		break;
	
	case FileMENU:
		switch (menuItem) {
  		case FM_Close:
			if ((frontWindow = (WindowPeek) FrontWindow()) == 0L)
				break;
			  
			if (frontWindow->windowKind < 0)
			 	CloseDeskAcc(frontWindow->windowKind);
			else if (frontWindow->refCon == MyWindowID)
				((ObjectWindowPtr) frontWindow)->Dispose((WindowPtr) frontWindow);
  			break;

		case FM_Quit:
			gDoneFlag = true;
			break;
		}
		break;
	case EditMENU:
		if (!SystemEdit(menuItem-1))
		  SysBeep(5);
		break;
	}
	HiliteMenu(0);
}

