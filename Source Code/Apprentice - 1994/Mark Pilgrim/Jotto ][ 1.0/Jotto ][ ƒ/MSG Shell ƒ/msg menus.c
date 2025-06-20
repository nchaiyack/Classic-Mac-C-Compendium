/**********************************************************************\

File:		msg menus.c

Purpose:	This module handles menu selections, including selection
			of dimmed menu items (hehe).


Jotto ][ -=- a simple word game, revisited
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
#include "jotto globals.h"
#include "jotto load-save.h"
#include "jotto.h"

Boolean			gMenuEnabled;
MenuHandle		gAppleMenu;
MenuHandle		gFileMenu;
MenuHandle		gEditMenu;
MenuHandle		gOptionsMenu;
MenuHandle		gHelpMenu;

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
	
	if (gMainWindow)
	{
		EnableItem(gFileMenu, saveItem);
		EnableItem(gFileMenu, saveAsItem);
		DisableItem(gFileMenu, newItem);
		DisableItem(gFileMenu, openItem);	
		DisableItem(gOptionsMenu, dupToggle);
		DisableItem(gOptionsMenu, nonWordsToggle);
		DisableItem(gOptionsMenu, useFiveLetter);
		DisableItem(gOptionsMenu, useSixLetter);
	}
	else
	{
		DisableItem(gFileMenu, saveItem);
		DisableItem(gFileMenu, saveAsItem);
		EnableItem(gFileMenu, newItem);
		EnableItem(gFileMenu, openItem);
		EnableItem(gOptionsMenu, dupToggle);
		EnableItem(gOptionsMenu, nonWordsToggle);
		if (gFiveLetterOK)
			EnableItem(gOptionsMenu, useFiveLetter);
		else
			DisableItem(gOptionsMenu, useFiveLetter);
		
		if (gSixLetterOK)
			EnableItem(gOptionsMenu, useSixLetter);
		else
			DisableItem(gOptionsMenu, useSixLetter);
	}
	
	CheckItem(gOptionsMenu, dupToggle, gAllowDup);
	CheckItem(gOptionsMenu, nonWordsToggle, gNonWordsCount);
	CheckItem(gOptionsMenu, animationToggle, gAnimation);
	CheckItem(gOptionsMenu, useFiveLetter, (gNumLetters==5));
	CheckItem(gOptionsMenu, useSixLetter, (gNumLetters==6));
	
	CheckItem(gOptionsMenu, soundToggle, gSoundToggle&&gSoundAvailable);
	if (gSoundAvailable)
		EnableItem(gOptionsMenu, soundToggle);
	else
		DisableItem(gOptionsMenu, soundToggle);
	
//	DrawMenuBar();
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
			else DoSound(sound_fluff);
			break;
		case openItem:
			if (gMenuEnabled)
				LoadSaveDispatch(TRUE, FALSE);
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
		if (gMenuEnabled)
		{
			if(!SystemEdit(menuItem - 1))
				if(menuItem == undoItem)
					GameUndo();
		}
		else DoSound(sound_fluff);
}

void HandleOptionsMenu(int menuItem)
{
	switch (menuItem)
	{
		case dupToggle:
		case nonWordsToggle:
		case useFiveLetter:
		case useSixLetter:
		case animationToggle:
		case soundToggle:
			if (gMenuEnabled)
			{
				switch (menuItem)
				{
					case dupToggle:			gAllowDup=!gAllowDup; break;
					case nonWordsToggle:	gNonWordsCount=!gNonWordsCount; break;
					case useFiveLetter:		gNumLetters=0x05; break;
					case useSixLetter:		gNumLetters=0x06; break;
					case animationToggle:	gAnimation=!gAnimation; break;
					case soundToggle:		gSoundToggle=!gSoundToggle; DoSound(sound_on); break;
				}
				SaveThePrefs();
			}
			else DoSound(sound_fluff);
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
