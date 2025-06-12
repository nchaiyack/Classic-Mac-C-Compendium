/**********************************************************************\

File:		menus.c

Purpose:	This module handles menu selections.



Dialectic -=- dialect text conversion extraordinare
Copyright ©1994, Mark Pilgrim

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

#include "graphics.h"
#include "menus.h"
#include "help.h"
#include "prefs.h"
#include "environment.h"
#include "program globals.h"
#include "dialectic.h"
#include "dialectic dispatch.h"
#include "dialectic scrap.h"

MenuHandle		gAppleMenu;
MenuHandle		gFileMenu;
MenuHandle		gEditMenu;
MenuHandle		gOptionsMenu;
MenuHandle		gHelpMenu;
MenuHandle		gDialectMenu;

enum
{
	helpMenu=200,
	appleMenu = 400, fileMenu, editMenu, optionsMenu, dialectMenu,
	
	aboutItem = 1, aboutMSGItem, helpPointerItem,
	openItem=1, closeItem, forceQuitItem = 4, quitItem,
	undoItem = 1, cutItem = 3, copyItem, pasteItem, clearItem, showClipboardItem = 8,
		convertClipboardItem,
	showSaveItem = 1, addSuffixItem, showProgressItem, rtfItem
};

/*-----------------------------------------------------------------------------------*/
/* internal stuff for menus.c                                                        */

void HandleAppleMenu(int menuItem);
void HandleFileMenu(int menuItem);
void HandleEditMenu(int menuItem);
void HandleViewMenu(int menuItem);
void HandleOptionsMenu(int menuItem);
void HandleDialectMenu(int menuItem);
void HandleHelpMenu(int menuItem);


Boolean InitTheMenus(void)
{
	Handle		MBARHandle;
	
	if ((MBARHandle=GetNewMBar(400))==0L)		/* sez which menus are in menu bar. */
		return FALSE;
	SetMenuBar(MBARHandle);						/* set this to be THE menu bar to use. */
	
	if ((gAppleMenu=GetMHandle(appleMenu))==0L)	/* GetNewMBar also got menu handles of */
		return FALSE;
	if ((gFileMenu=GetMHandle(fileMenu))==0L)	/* every menu it includes, so just */
		return FALSE;
	if ((gEditMenu=GetMHandle(editMenu))==0L)	/* grab these handles and assign them */
		return FALSE;
	if ((gOptionsMenu=GetMHandle(optionsMenu))==0L)	/* to the appropriate menus */
		return FALSE;
	if ((gDialectMenu=GetMHandle(dialectMenu))==0L)	/* ...there, was that so hard? */
		return FALSE;
	if ((gHelpMenu = GetMenu(helpMenu))==0L)		/* help menu is not in menu bar, so */
		return FALSE;
	InsertMenu(gHelpMenu, -1);					/* get it manually and insert it */
	
	AddResMenu(gAppleMenu, 'DRVR');				/* adds control panels to apple menu */
	
	AdjustMenus();								/* dim/enable/check/mark menus/items */
	DrawMenuBar();								/* draws the actual menu bar */
	
	return TRUE;
}

void AdjustMenus(void)
{
	WindowPeek		theWindow;
	int				kind;
	int				i;
	unsigned char	iter;
	unsigned long	dummy;
	
	if (gInProgress)
	{
		DisableItem(gAppleMenu, aboutItem);
		DisableItem(gAppleMenu, aboutMSGItem);
		DisableItem(gAppleMenu, helpPointerItem);
		if (gSystemSevenOrLater)
		{
			DisableItem(gFileMenu, openItem);
			DisableItem(gFileMenu, closeItem);
			DisableItem(gFileMenu, quitItem);
		}
		else DisableItem(gFileMenu, 0);
		DisableItem(gEditMenu, 0);
		DisableItem(gOptionsMenu, 0);
		DisableItem(gDialectMenu, 0);
	}
	else
	{
		EnableItem(gAppleMenu, aboutItem);
		EnableItem(gAppleMenu, aboutMSGItem);
		EnableItem(gAppleMenu, helpPointerItem);
		if (gSystemSevenOrLater)
		{
			EnableItem(gFileMenu, openItem);
			EnableItem(gFileMenu, closeItem);
			EnableItem(gFileMenu, quitItem);
		}
		EnableItem(gFileMenu, 0);
		EnableItem(gEditMenu, 0);
		EnableItem(gOptionsMenu, 0);
		EnableItem(gDialectMenu, 0);
		
		LoadScrap();
		if (GetScrap(0L, 'TEXT', &dummy)==noTypeErr)
		{
			DisableItem(gEditMenu, showClipboardItem);
			DisableItem(gEditMenu, convertClipboardItem);
		}
		else
		{
			EnableItem(gEditMenu, showClipboardItem);
			EnableItem(gEditMenu, convertClipboardItem);
		}
		
		theWindow = (WindowPeek)FrontWindow();
		kind = theWindow ? theWindow->windowKind : 0;
		
		if (kind>=0)
		{
			DisableItem(gEditMenu, undoItem);
			DisableItem(gEditMenu, cutItem);
			DisableItem(gEditMenu, copyItem);
			DisableItem(gEditMenu, pasteItem);
			DisableItem(gEditMenu, clearItem);
		}
		else
		{
			EnableItem(gEditMenu, undoItem);
			EnableItem(gEditMenu, cutItem);
			EnableItem(gEditMenu, copyItem);
			EnableItem(gEditMenu, pasteItem);
			EnableItem(gEditMenu, clearItem);
		}
		
		if (theWindow)
			EnableItem(gFileMenu, closeItem);
		else
			DisableItem(gFileMenu, closeItem);
	}
	
	if (gSystemSevenOrLater)
		EnableItem(gFileMenu, forceQuitItem);
	else
		DisableItem(gFileMenu, forceQuitItem);
	
	CheckItem(gOptionsMenu, showSaveItem, gShowSaveDialog);
	CheckItem(gOptionsMenu, addSuffixItem, gAddSuffix);
	CheckItem(gOptionsMenu, showProgressItem, gShowProgress);
	CheckItem(gOptionsMenu, rtfItem, gUseRTF);
	SetSuffixMenuItem(gOptionsMenu, addSuffixItem);
	for (iter=0; iter<CountMItems(gDialectMenu); iter++)
		CheckItem(gDialectMenu, iter+1, (iter==gWhichDialect));
	HiliteMenu(0);
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
		case dialectMenu:
			HandleDialectMenu(menuItem);
			break;
	  }
}

void DoTheCloseThing(WindowPeek theWindow)
/* a standard close procedure, called when "close" is chosen from File menu and when
   a window is closed through its close box */
{
	Boolean			gotone;
	int				i;
	int				kind;
	
	kind = theWindow ? theWindow->windowKind : 0;
	if (kind<0)		/* DA window or other system window */
		CloseDeskAcc(kind);
	else
	{
		gotone=FALSE;
		/* see if it's one of ours */
		for (i=0; (i<NUM_WINDOWS) && (!gotone); i++)
			gotone=(theWindow==gTheWindow[i]);
		
		if (gotone)		/* if it's one of ours...  see graphics.c */
			CloseTheWindow(gTheWindowData[i-1]);	/* this may return FALSE = not closed */
		else
			DisposeWindow(theWindow);		/* not one of ours, so just close it */
	
		AdjustMenus();	/* may affect which menu items or menus are available, etc */
	}
}

void HandleAppleMenu(int menuItem)
{
	GrafPtr		savePort;
	Str255		name;
	
	if (menuItem == aboutItem)
		OpenTheWindow(kAbout);
	if (menuItem == aboutMSGItem)
		OpenTheWindow(kAboutMSG);
	else if (menuItem > helpPointerItem+1)
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
			DoTheCloseThing(FrontWindow());
			break;
		case forceQuitItem:
			SysError(0x4e22);
			break;
		case quitItem:
			gDone = TRUE;
			break;
	}
}

void HandleEditMenu(int menuItem)
{
	switch (menuItem)
	{
		case showClipboardItem:
			OpenTheWindow(kClipboard);
			break;
		case convertClipboardItem:
			NewScrapConvert();
			break;
		default:
			SystemEdit(menuItem-1);
			break;
	}
}

void HandleHelpMenu(int menuItem)
{
	gLastHelp=gWhichHelp;
	gWhichHelp=menuItem;
	OpenTheWindow(kHelp);
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

void HandleDialectMenu(int menuItem)
{
	gWhichDialect=menuItem-1;
	SaveThePrefs();
	AdjustMenus();
}
