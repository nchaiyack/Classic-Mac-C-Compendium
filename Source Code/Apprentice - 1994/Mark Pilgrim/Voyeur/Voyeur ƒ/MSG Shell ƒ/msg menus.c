/**********************************************************************\

File:		msg menus.c

Purpose:	This module handles menu selections.


Voyeur -- a no-frills file viewer
Copyright (C) 1993 Mark Pilgrim

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "msg graphics.h"
#include "msg menus.h"
#include "msg prefs.h"
#include "msg environment.h"
#include "v.h"
#include "v meat.h"
#include "v find.h"
#include "v structs.h"
#include "v error.h"
#include "v type creator.h"
#include "v help.h"

MenuHandle		gAppleMenu;
MenuHandle		gFileMenu;
MenuHandle		gEditMenu;
MenuHandle		gOptionsMenu;
MenuHandle		gDangerousMenu;
MenuHandle		gHelpMenu;

void AdjustMenus(void)
{
	WindowPeek	theWindow;
	int			kind;
	int			i;
	
	theWindow = (WindowPeek)FrontWindow();
	kind = theWindow ? theWindow->windowKind : 0;
	
	if(kind < 0)
		EnableItem(gEditMenu, 0);
	else
		DisableItem(gEditMenu, 0);
	
	if(theWindow)
		EnableItem(gFileMenu, closeItem);
	else
		DisableItem(gFileMenu, closeItem);
	
	if(gMainWindow)
	{
		DisableItem(gFileMenu, openItem);
		EnableItem(gOptionsMenu, 0);
		EnableItem(gDangerousMenu, 0);
		CheckItem(gOptionsMenu, dataForkItem, (forknum==0));
		CheckItem(gOptionsMenu, resourceForkItem, (forknum==1));
		if (findString[0]==0x00)
			DisableItem(gOptionsMenu, findAgainItem);
		else
			EnableItem(gOptionsMenu, findAgainItem);
	}
	else
	{
		EnableItem(gFileMenu, openItem);
		CheckItem(gOptionsMenu, dataForkItem, FALSE);
		CheckItem(gOptionsMenu, resourceForkItem, FALSE);
		DisableItem(gOptionsMenu, 0);
		DisableItem(gDangerousMenu, 0);
	}
}

void HandleMenu(long mSelect)
{
	int			menuID = HiWord(mSelect);
	int			menuItem = LoWord(mSelect);
	
	switch (menuID)
	{
		case appleMenu:
			HandleAppleMenu(menuItem);
			break;
		case fileMenu:
			HandleFileMenu(menuItem);
			break;	
		case editMenu:
			HandleEditMenu(menuItem);
			break;
		case optionsMenu:
			HandleOptionsMenu(menuItem);
			break;
		case dangerousMenu:
			HandleDangerousMenu(menuItem);
			break;
		case helpMenu:
			HandleHelpMenu(menuItem);
			break;
	  }
}

void HandleAppleMenu(int menuItem)
{
	GrafPtr		savePort;
	Str255		name;
	
	if(menuItem == 1)
		ShowInformation();
	if (menuItem == 2)
		ShowSplashScreen();
	else if(menuItem > 4)
	{
		GetPort(&savePort);
		GetItem(gAppleMenu, menuItem, name);
		OpenDeskAcc(name);
		SetPort(savePort);
	}
}

void HandleFileMenu(int menuItem)
{
	WindowPtr		theWindow;
	
	switch (menuItem)
	{
		case openItem:
			HandleError(NewEditWindow());
			break;
		case closeItem:
			theWindow=FrontWindow();
			if (theWindow == gMainWindow)
				CloseMainWindow();
			else
				DisposeWindow(theWindow);
			
			AdjustMenus();
			break;
		case quitItem:
			gDone = TRUE;
			break;
	}
}

void HandleEditMenu(int menuItem)
{
	if ((menuItem>0) && (menuItem!=2))
		if(!SystemEdit(menuItem - 1))
			if(menuItem == undoItem)
				ProgramUndo();
}

void HandleOptionsMenu(int menuItem)
{
	switch (menuItem)
	{
		case dataForkItem:
			DoDataFork();
			break;
		case resourceForkItem:
			DoResourceFork();
			break;
		case forwardItem:
			GoForward();
			break;
		case backItem:
			GoBack();
			break;
		case beginningItem:
			GoBeginning();
			break;
		case endItem:
			GoEnd();
			break;
		case offsetItem:
			GoOffset();
			break;
		case findItem:
			DoFind();
			break;
		case findAgainItem:
			DoFindAgain();
			break;
	}
}

void HandleDangerousMenu(int menuItem)
{
	switch (menuItem)
	{
		case typeCreatorItem:
			ChangeTypeCreator();
			break;
		case eofItem:
			ChangeEOF();
			break;
	}
}

void HandleHelpMenu(int menuItem)
{
	if ((menuItem>0) && (menuItem<=NUM_HELP))
	{
		OpenHelpWindow(menuItem-1);
		SelectWindow(gHelp[menuItem-1]);
	}
}
