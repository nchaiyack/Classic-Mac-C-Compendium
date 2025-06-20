/**********************************************************************\

File:		msg menus.c

Purpose:	This module handles menu selections, including selection
			of dimmed menu items (hehe).


Devil�s Cubes -- a simple cubes puzzle
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
#include "msg sounds.h"
#include "msg environment.h"
#include "msg prefs.h"
#include "cube.h"
#include "cube load-save.h"
#include "cube meat.h"

Boolean				gMenuEnabled;
MenuHandle			gAppleMenu;
MenuHandle			gFileMenu;
MenuHandle			gEditMenu;
MenuHandle			gOptionsMenu;
MenuHandle			gHelpMenu;

extern	long		menuDisable : 0x0b54;

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
		DisableItem(gFileMenu, newItem);
		DisableItem(gFileMenu, openItem);
		EnableItem(gFileMenu, saveItem);
		EnableItem(gFileMenu, saveAsItem);
	}
	else
	{
		EnableItem(gFileMenu, newItem);
		EnableItem(gFileMenu, openItem);
		DisableItem(gFileMenu, saveItem);
		DisableItem(gFileMenu, saveAsItem);
	}
	
	CheckItem(gOptionsMenu, mirrorItem, gUseMirror);
	CheckItem(gOptionsMenu, showItem, gShowAll);
	
	CheckItem(gOptionsMenu, soundToggle, gSoundToggle&&gSoundAvailable);
	if (gSoundAvailable)
		EnableItem(gOptionsMenu, soundToggle);
	else
		DisableItem(gOptionsMenu, soundToggle);
}

void HandleMenu(long mSelect)
{
	int			menuID = HiWord(mSelect);
	int			menuItem = LoWord(mSelect);
	
	if (menuID==0)
	{
		menuID=HiWord(menuDisable);
		menuItem=LoWord(menuDisable);
		gMenuEnabled=FALSE;
	}
	else gMenuEnabled=TRUE;
	menuDisable=0L;

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
	int				i;
	
	switch (menuItem)
	{
		case newItem:
			if (gMenuEnabled)
				NewGame();
			else DoSound(sound_fluff);
			break;
		case openItem:
			if (gMenuEnabled)
				LoadSaveDispatch(TRUE, TRUE);
			else DoSound(sound_fluff);
			break;
		case closeItem:
			if (gMenuEnabled)
			{
				theWindow=FrontWindow();
				for (i=0; i<NUM_HELP; i++)
					if (theWindow == gHelp[i])
						gHelp[i]=0L;
				
				if(theWindow == gMainWindow)
					CloseMainWindow();
				else
					DisposeWindow(theWindow);
				
				AdjustMenus();
			}
			else DoSound(sound_fluff);
			break;
		case saveItem:
			if (gMenuEnabled)
				LoadSaveDispatch(FALSE, TRUE);
			else DoSound(sound_fluff);
			break;
		case saveAsItem:
			if (gMenuEnabled)
				LoadSaveDispatch(FALSE, FALSE);
			else DoSound(sound_fluff);
			break;
		case quitItem:
			gDone = TRUE;
			break;
	}
}

void HandleEditMenu(int menuItem)
{
	if ((menuItem>0) && (menuItem!=2))
	{
		if (gMenuEnabled)
			SystemEdit(menuItem - 1);
		else DoSound(sound_fluff);
	}
}

void HandleOptionsMenu(int menuItem)
{
	switch (menuItem)
	{
		case mirrorItem:
			gUseMirror=!gUseMirror;
			SaveThePrefs();
			AdjustMenus();
			if (gMainWindow)
				UpdateBoard();
			break;
		case showItem:
			gShowAll=!gShowAll;
			SaveThePrefs();
			AdjustMenus();
			if (gMainWindow)
				UpdateBoard();
			break;
		case soundToggle:
			if (gMenuEnabled)
			{
				gSoundToggle=!gSoundToggle;
				SaveThePrefs();
				DoSound(sound_on);
			}
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
