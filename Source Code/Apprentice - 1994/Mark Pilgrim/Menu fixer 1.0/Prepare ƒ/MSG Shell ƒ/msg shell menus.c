/**********************************************************************\

File:		msg shell menus.c

Purpose:	This module handles menu selections.


MSG Prepare 1.0 -- minimal integrity check preparation program
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

#include "msg menus.h"
#include "MSG Prepare.h"

MenuHandle		gAppleMenu;
MenuHandle		gFileMenu;
MenuHandle		gEditMenu;

void AdjustMenus(void)
{
	WindowPeek	theWindow;
	int			kind;
	int			i;
	
	theWindow = (WindowPeek)FrontWindow();
	kind = theWindow ? theWindow->windowKind : 0;
	
	if(kind < 0) EnableItem(gEditMenu, 0);
	else DisableItem(gEditMenu, 0);
	
	if(theWindow)
		EnableItem(gFileMenu, closeItem);
	else
		DisableItem(gFileMenu, closeItem);
	
	DrawMenuBar();
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
	}
}

void HandleAppleMenu(int menuItem)
{
	GrafPtr		savePort;
	Str255		name;
	
	if(menuItem == 1)
		Alert(aboutPrepare, 0L);
	else
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
	SFReply			reply;
	
	switch (menuItem)
	{
		case openItem:
			where.h=40;
			where.v=40;
			SFGetFile(where,"\p",0L,-1,0L,0L,&reply);
			if (reply.good)
				OpenFile(reply.fName, reply.vRefNum);
			break;
		case closeItem:
			DisposeWindow(FrontWindow());
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
