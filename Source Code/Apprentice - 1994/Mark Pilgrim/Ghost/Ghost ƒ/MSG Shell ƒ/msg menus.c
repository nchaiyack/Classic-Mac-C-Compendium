/**********************************************************************\

File:		msg menus.c

Purpose:	This module handles menu selections, including selection
			of dimmed menu items (hehe).


Ghost -=- a classic word-building challenge
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
#include "msg prefs.h"
#include "msg environment.h"
#include "ghost globals.h"
#include "ghost load-save.h"
#include "ghost.h"
#include "ghost dictionary.h"
#include "ghost challenge.h"

Boolean			gMenuEnabled;
MenuHandle		gAppleMenu;
MenuHandle		gFileMenu;
MenuHandle		gEditMenu;
MenuHandle		gOptionsMenu;
MenuHandle		gHelpMenu;
MenuHandle		gComputerMenu;
MenuHandle		gIntelligenceMenu;
MenuHandle		gSpeedMenu;
MenuHandle		gDictionaryMenu;

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
	
	if (gInProgress)
	{
		EnableItem(gFileMenu, saveItem);
		EnableItem(gFileMenu, saveAsItem);
		DisableItem(gFileMenu, newItem);
		DisableItem(gFileMenu, openItem);	
		DisableItem(gComputerMenu, 0);
		DisableItem(gDictionaryMenu, 0);
		DisableItem(gIntelligenceMenu, 0);
		if (gTheWord[0]>0x01)
			EnableItem(gOptionsMenu, challengeWord);
		else
			DisableItem(gOptionsMenu, challengeWord);
	}
	else
	{
		DisableItem(gFileMenu, saveItem);
		DisableItem(gFileMenu, saveAsItem);
		EnableItem(gFileMenu, newItem);
		EnableItem(gFileMenu, openItem);
		EnableItem(gComputerMenu, 0);
		EnableItem(gDictionaryMenu, 0);
		EnableItem(gIntelligenceMenu, 0);
		DisableItem(gOptionsMenu, challengeWord);
	}
	
	for (i=1; i<=6; i++)
		CheckItem(gComputerMenu, i, (gNumComputerPlayers==i-1));
	for (i=1; i<=3; i++)
		CheckItem(gIntelligenceMenu, i, (gComputerIntelligence==i));
	for (i=1; i<=5; i++)
		CheckItem(gSpeedMenu, i, (gGameSpeed==5-i));
	
	CheckItem(gDictionaryMenu, 1, !gUseFullDictionary);
	CheckItem(gDictionaryMenu, 2, gUseFullDictionary);
	
	CheckItem(gOptionsMenu, soundToggle, gSoundToggle&&gSoundAvailable);
	CheckItem(gOptionsMenu, showMessage, gShowMessageBox);
	
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
		case computerMenu:
			HandleComputerMenu(menuItem);
			break;
		case intelligenceMenu:
			HandleIntelligenceMenu(menuItem);
			break;
		case speedMenu:
			HandleSpeedMenu(menuItem);
			break;
		case dictionaryMenu:
			HandleDictionaryMenu(menuItem);
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
	else if(menuItem > 3)
	{
		GetPort(&savePort);
		GetItem(gAppleMenu, menuItem, name);
		OpenDeskAcc(name);
		SetPort(savePort);
	}
}

void HandleFileMenu(int menuItem)
{
	Point			where;
	SFTypeList		typeList;
	SFReply			reply;
	WindowPtr		theWindow;
	int				i;
	
	switch (menuItem)
	{
		case newItem:
			if (gMenuEnabled)
				NewGame();
			else DoSound(sound_fluff, TRUE);
			break;
		case openItem:
			if (gMenuEnabled)
				LoadSaveDispatch(TRUE, FALSE);
			else DoSound(sound_fluff, TRUE);
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
			else DoSound(sound_fluff, TRUE);
			break;
		case saveItem:
			if (gMenuEnabled)
				LoadSaveDispatch(FALSE, TRUE);
			else DoSound(sound_fluff, TRUE);
			break;
		case saveAsItem:
			if (gMenuEnabled)
				LoadSaveDispatch(FALSE, FALSE);
			else DoSound(sound_fluff, TRUE);
			break;
		case quitItem:
			gDone = TRUE;
			break;
	}
}

void HandleEditMenu(int menuItem)
{
	if ((menuItem>0) && (menuItem!=2))
		if (gMenuEnabled)
		{
			if(!SystemEdit(menuItem - 1))
				if(menuItem == undoItem)
					GameUndo();
		}
		else DoSound(sound_fluff, TRUE);
}

void HandleOptionsMenu(int menuItem)
{
	switch (menuItem)
	{
		case saveOptions:
			SaveThePrefs();
			break;
		case soundToggle:
			gSoundToggle=!gSoundToggle;
			DoSound(sound_on, TRUE);
			break;
		case showMessage:
			gShowMessageBox=!gShowMessageBox;
			if (gMainWindow)
				UpdateBoard();
			break;
		case challengeWord:
			if (gMenuEnabled)
				ChallengeWord();
			else DoSound(sound_fluff, TRUE);
			break;
	}
}

void HandleComputerMenu(int menuItem)
{
	if ((menuItem>0) && (menuItem<=6))
	{
		if (gMenuEnabled)
			gNumComputerPlayers=menuItem-1;
		else DoSound(sound_fluff, TRUE);
	}
}

void HandleIntelligenceMenu(int menuItem)
{
	if ((menuItem>0) && (menuItem<=3))
	{
		if (gMenuEnabled)
			gComputerIntelligence=menuItem;
		else DoSound(sound_fluff, TRUE);
	}
}

void HandleSpeedMenu(int menuItem)
{
	if ((menuItem>0) && (menuItem<=5))
	{
		if (gMenuEnabled)
			gGameSpeed=5-menuItem;
		else DoSound(sound_fluff, TRUE);
	}
}

void HandleDictionaryMenu(int menuItem)
{
	if ((menuItem>0) && (menuItem<=2))
	{
		if (gMenuEnabled)
			gUseFullDictionary=(menuItem==2);
		else DoSound(sound_fluff, TRUE);
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
