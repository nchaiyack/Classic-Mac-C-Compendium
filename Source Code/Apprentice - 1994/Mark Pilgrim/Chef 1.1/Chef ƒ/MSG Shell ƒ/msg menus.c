/**********************************************************************\

File:		msg menus.c

Purpose:	This module handles menu selections.



Chef -=- convert text to Swedish chef talk
Copyright �1994, Mark Pilgrim

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
#include "msg help.h"
#include "msg prefs.h"
#include "msg environment.h"
#include "program globals.h"
#include "chef.h"

MenuHandle		gAppleMenu;
MenuHandle		gFileMenu;
MenuHandle		gEditMenu;
MenuHandle		gOptionsMenu;
MenuHandle		gHelpMenu;

void AdjustMenus(void)
{
	WindowPeek		theWindow;
	int				kind;
	int				i;
	unsigned char	iter;
	
	if (!gInProgress)
	{
		EnableItem(gAppleMenu, aboutItem);
		EnableItem(gAppleMenu, aboutMSGItem);
		EnableItem(gAppleMenu, helpItem);
		EnableItem(gFileMenu, 0);
		EnableItem(gEditMenu, 0);
		EnableItem(gOptionsMenu, 0);
	}
	else
	{
		DisableItem(gAppleMenu, aboutItem);
		DisableItem(gAppleMenu, aboutMSGItem);
		DisableItem(gAppleMenu, helpItem);
		DisableItem(gFileMenu, 0);
		DisableItem(gEditMenu, 0);
		DisableItem(gOptionsMenu, 0);
	}
	
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
	
	CheckItem(gOptionsMenu, showSaveItem, gShowSaveDialog);
	CheckItem(gOptionsMenu, addSuffixItem, gAddSuffix);
	CheckItem(gOptionsMenu, showProgressItem, gShowProgress);
	CheckItem(gOptionsMenu, rtfItem, gUseRTF);
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
		OpenTheWindow(kAbout);
	if (menuItem == 2)
		OpenTheWindow(kAboutMSG);
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
	WindowPtr			theWindow;
	int					i;
	Boolean				gotone;
	
	switch (menuItem)
	{
		case openItem:
			NewConvert();
			break;
		case closeItem:
			theWindow=FrontWindow();
			gotone=FALSE;
			for (i=0; (i<NUM_WINDOWS) && (!gotone); i++)
				gotone=(theWindow==gTheWindow[i]);
				
			if (gotone)
				CloseTheWindow(i-1);
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
	SystemEdit(menuItem - 1);
}

void HandleHelpMenu(int menuItem)
{
	gWhichHelp=menuItem;
	UpdateHelpWindow();
}

void HandleOptionsMenu(int menuItem)
{
	switch (menuItem)
	{
		case showSaveItem:
			gShowSaveDialog=!gShowSaveDialog;
			SaveThePrefs();
			break;
		case addSuffixItem:
			gAddSuffix=!gAddSuffix;
			SaveThePrefs();
			break;
		case showProgressItem:
			gShowProgress=!gShowProgress;
			SaveThePrefs();
			break;
		case rtfItem:
			gUseRTF=!gUseRTF;
			SaveThePrefs();
			break;
	}
}
