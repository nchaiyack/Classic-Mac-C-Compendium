/**********************************************************************\

File:		msg menus.c

Purpose:	This module handles menu selections, including selection
			of dimmed menu items (hehe).


Pentominoes - a 2-D geometry board game
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
#include "pent.h"
#include "pent load-save.h"
#include "pent meat.h"

Boolean				gMenuEnabled;
MenuHandle			gAppleMenu;
MenuHandle			gFileMenu;
MenuHandle			gEditMenu;
MenuHandle			gBoardMenu;
MenuHandle			gFluffMenu;
MenuHandle			gSub1;
MenuHandle			gSub2;
MenuHandle			gSub3;
MenuHandle			gSub4;
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
	{
		EnableItem(gEditMenu, 0);
	}
	else
	{
		DisableItem(gEditMenu, cutItem);
		DisableItem(gEditMenu, copyItem);
		DisableItem(gEditMenu, pasteItem);
		DisableItem(gEditMenu, clearItem);
		if ((gMainWindow) && ((gNumPlayed) || (gNumHilited)))
			EnableItem(gEditMenu, undoItem);
		else
			DisableItem(gEditMenu, undoItem);
	}
	
	if(theWindow)
		EnableItem(gFileMenu, closeItem);
	else
		DisableItem(gFileMenu, closeItem);
	
	if(gMainWindow)
	{
		DisableItem(gFileMenu, newItem);
		DisableItem(gFileMenu, openItem);
		DisableItem(gFileMenu, loadSolutionItem);
		EnableItem(gFileMenu, saveItem);
		EnableItem(gFileMenu, saveAsItem);
		DisableItem(gBoardMenu, 0);
		if (gNumPlayed==12)
			EnableItem(gFileMenu, saveSolutionItem);
		else
			DisableItem(gFileMenu, saveSolutionItem);
	}
	else
	{
		EnableItem(gFileMenu, newItem);
		EnableItem(gFileMenu, openItem);
		EnableItem(gFileMenu, loadSolutionItem);
		DisableItem(gFileMenu, saveItem);
		DisableItem(gFileMenu, saveAsItem);
		DisableItem(gFileMenu, saveSolutionItem);
		EnableItem(gBoardMenu, 0);
	}
	
	if ((gMainWindow) && (gNumHilited==5) && (gCurrentColor==-2))
		EnableItem(gFluffMenu, placeItem);
	else
		DisableItem(gFluffMenu, placeItem);
	
	if ((gMainWindow) && (gCurrentColor!=-2))
		EnableItem(gFluffMenu, removeItem);
	else
		DisableItem(gFluffMenu, removeItem);
	
	CheckAppropriate();
	
	CheckItem(gFluffMenu, soundToggle, gSoundToggle&&gSoundAvailable);
	if (gSoundAvailable)
		EnableItem(gFluffMenu, soundToggle);
	else
		DisableItem(gFluffMenu, soundToggle);
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
		case boardMenu:
			HandleBoardMenu(menuItem);
			break;
		case fluffMenu:
			HandleFluffMenu(menuItem);
			break;
		case sub1:
			HandleSub1(menuItem);
			break;
		case sub2:
			HandleSub2(menuItem);
			break;
		case sub3:
			HandleSub3(menuItem);
			break;
		case sub4:
			HandleSub4(menuItem);
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
				LoadSaveDispatch(TRUE, TRUE, TRUE);
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
				LoadSaveDispatch(FALSE, TRUE, TRUE);
			else DoSound(sound_fluff);
			break;
		case saveAsItem:
			if (gMenuEnabled)
				LoadSaveDispatch(FALSE, TRUE, FALSE);
			else DoSound(sound_fluff);
			break;
		case loadSolutionItem:
			if (gMenuEnabled)
				LoadSaveDispatch(TRUE, FALSE, FALSE);
			else DoSound(sound_fluff);
			break;
		case saveSolutionItem:
			if (gMenuEnabled)
				LoadSaveDispatch(FALSE, FALSE, FALSE);
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

void HandleBoardMenu(int menuItem)
{
	if (menuItem>0)
		if ((!gMenuEnabled) && (gMainWindow))
			DoSound(sound_fluff);
}

void HandleFluffMenu(int menuItem)
{
	switch (menuItem)
	{
		case placeItem:
			if (gMenuEnabled)
				PlaceUnknownPiece();
			else
				DoSound(sound_fluff);
			break;
		case removeItem:
			if (gMenuEnabled)
				RemovePiece();
			else
				DoSound(sound_fluff);
			break;
		case soundToggle:
			if (gMenuEnabled)
			{
				gSoundToggle=!gSoundToggle;
				SaveThePrefs();
				DoSound(sound_on);
			}
			else
				DoSound(sound_fluff);
			break;
	}
}

void HandleSub1(int menuItem)
{
	if (gMenuEnabled)
	{
		switch (menuItem)
		{
			case 1: gWhichBoard=1; break;
			case 2: gWhichBoard=2; break;
			case 3: gWhichBoard=10; break;
			case 4: gWhichBoard=3; break;
			case 5: gWhichBoard=11; break;
			case 7: gWhichBoard=4; break;
			case 8: gWhichBoard=5; break;
			case 9: gWhichBoard=12; break;
			case 10: gWhichBoard=13; break;
		}
		SaveThePrefs();
		NewGame();
	}
}

void HandleSub2(int menuItem)
{
	if (gMenuEnabled)
	{
		gWhichBoard=menuItem+5;
		SaveThePrefs();
		NewGame();
	}
}

void HandleSub3(int menuItem)
{
	if (gMenuEnabled)
	{
		gWhichBoard=menuItem+13;
		SaveThePrefs();
		NewGame();
	}
}

void HandleSub4(int menuItem)
{
	if (gMenuEnabled)
	{
		gWhichBoard=menuItem+25;
		SaveThePrefs();
		NewGame();
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

void CheckAppropriate(void)
{
	int				i;
	
	for (i=1; i<=10; i++)
		CheckItem(gSub1, i, FALSE);
	for (i=1; i<=4; i++)
		CheckItem(gSub2, i, FALSE);
	for (i=1; i<=12; i++)
		CheckItem(gSub3, i, FALSE);
	for (i=1; i<=6; i++)
		CheckItem(gSub4, i, FALSE);

	switch (gWhichBoard)
	{
		case 1: CheckItem(gSub1, 1, TRUE); break;
		case 2: CheckItem(gSub1, 2, TRUE); break;
		case 3: CheckItem(gSub1, 4, TRUE); break;
		case 4: CheckItem(gSub1, 7, TRUE); break;
		case 5: CheckItem(gSub1, 8, TRUE); break;
		case 10: CheckItem(gSub1, 3, TRUE); break;
		case 11: CheckItem(gSub1, 5, TRUE); break;
		case 12: CheckItem(gSub1, 9, TRUE); break;
		case 13: CheckItem(gSub1, 10, TRUE); break;
	}
	if ((gWhichBoard>=6) && (gWhichBoard<=9))
		CheckItem(gSub2, gWhichBoard-5, TRUE);
	else if ((gWhichBoard>=14) && (gWhichBoard<=25))
		CheckItem(gSub3, gWhichBoard-13, TRUE);
	else if (gWhichBoard>=26)
		CheckItem(gSub4, gWhichBoard-25, TRUE);
}
