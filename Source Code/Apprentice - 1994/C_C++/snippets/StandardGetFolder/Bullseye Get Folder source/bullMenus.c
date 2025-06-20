/*****
 * bullMenus.c
 *
 *	Routines for Bullseye demo menus.
 *
 *****/

#include "bullMenus.h"
#include "bullWindow.h"
#include "StandardGetFolder.h"

void DoUpdate (EventRecord *theEvent);


extern	WindowPtr bullseyeWindow;
extern	int	width;


MenuHandle	appleMenu, fileMenu, editMenu, widthMenu;

enum	{
	appleID = 1,
	fileID,
	editID,
	widthID
	};

enum	{
	openItem = 1,
	closeItem,
	quitItem = 4
	};


/****
 * SetUpMenus()
 *
 *	Set up the menus. Normally, we�d use a resource file, but
 *	for this example we�ll supply �hardwired� strings.
 *
 ****/

void SetUpMenus(void)

{
	InsertMenu(appleMenu = NewMenu(appleID, "\p\024"), 0);
	InsertMenu(fileMenu = NewMenu(fileID, "\pFile"), 0);
	InsertMenu(editMenu = NewMenu(editID, "\pEdit"), 0);
	InsertMenu(widthMenu = NewMenu(widthID, "\pWidth"), 0);
	DrawMenuBar();
	AddResMenu(appleMenu, 'DRVR');
	AppendMenu(fileMenu, "\pOpen/O;Close/W;(-;Quit/Q");
	AppendMenu(editMenu, "\pUndo/Z;(-;Cut/X;Copy/C;Paste/V;Clear");
	AppendMenu(widthMenu, "\p1/1;2/2;3/3;4/4;5/5;6/6;7/7;8/8;9/9;10/0;11;12");
}
/* end SetUpMenus */


/****
 *  AdjustMenus()
 *
 *	Enable or disable the items in the Edit menu if a DA window
 *	comes up or goes away. Our application doesn't do anything with
 *	the Edit menu.
 *
 ****/

int enable (MenuHandle menu, short item, short ok);

void AdjustMenus(void)
{
	register WindowPeek wp = (WindowPeek) FrontWindow();
	short kind = wp ? wp->windowKind : 0;
	Boolean DA = kind < 0;
	
	enable(editMenu, 1, DA);
	enable(editMenu, 3, DA);
	enable(editMenu, 4, DA);
	enable(editMenu, 5, DA);
	enable(editMenu, 6, DA);
	
	enable(fileMenu, openItem, true);
	enable(fileMenu, closeItem, DA || ((WindowPeek) bullseyeWindow)->visible);

	CheckItem(widthMenu, width, true);
}


static
enable(MenuHandle menu, short item, short ok)
{
	if (ok)
		EnableItem(menu, item);
	else
		DisableItem(menu, item);
}


/*****
 * HandleMenu(mSelect)
 *
 *	Handle the menu selection. mSelect is what MenuSelect() and
 *	MenuKey() return: the high word is the menu ID, the low word
 *	is the menu item
 *
 *****/

void HandleMenu (long mSelect)

{
	int			menuID = HiWord(mSelect);
	int			menuItem = LoWord(mSelect);
	Str255		name;
	GrafPtr		savePort;
	WindowPeek	frontWindow;
	StandardFolderReply	reply;
	
	switch (menuID)
	  {
	  case	appleID:
		GetPort(&savePort);
		GetItem(appleMenu, menuItem, name);
		OpenDeskAcc(name);
		SetPort(savePort);
		break;
	
	  case	fileID:
		switch (menuItem)
		  {
		  case	openItem:
			ShowWindow(bullseyeWindow);
			SelectWindow(bullseyeWindow);
			StandardGetFolder("\pBullseye Select Folder:",&reply,DoUpdate);
			break;
  				  			
		  case	closeItem:
			if ((frontWindow = (WindowPeek) FrontWindow()) == 0L)
			  break;
			  
			if (frontWindow->windowKind < 0)
			  CloseDeskAcc(frontWindow->windowKind);
			else if (frontWindow = (WindowPeek) bullseyeWindow)
			  HideWindow(bullseyeWindow);
  				  	break;
  				  	
		  case	quitItem:
			ExitToShell();
			break;
		  }
		break;
  				
	  case	editID:
		if (!SystemEdit(menuItem-1))
		  SysBeep(5);
		break;
		
	  case	widthID:
		CheckItem(widthMenu, width, false);
		width = menuItem;
		InvalRect(&bullseyeWindow->portRect);
		break;
	  }
}
/* end HandleMenu */

void DoUpdate (EventRecord *theEvent)
{
	if ((WindowPtr)(theEvent->message) == bullseyeWindow)
		{
		if (theEvent->what == activateEvt)
			{
			SetPort(bullseyeWindow);
			InvalRect(&bullseyeWindow->portRect);
			}
		else if (theEvent->what == updateEvt)
			{
			BeginUpdate(bullseyeWindow);
			DrawBullseye(((WindowPeek) bullseyeWindow)->hilited);
			EndUpdate(bullseyeWindow);
			}
		}
}
