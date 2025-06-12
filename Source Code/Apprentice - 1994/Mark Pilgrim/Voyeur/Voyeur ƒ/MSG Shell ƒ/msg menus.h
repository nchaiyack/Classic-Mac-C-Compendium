/**********************************************************************\

File:		msg menus.h

Purpose:	This is the header file for msg menus.c


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

enum
{
	helpMenu = 200,
	
	appleMenu = 400,
	fileMenu,
	editMenu,
	optionsMenu,
	dangerousMenu,
	
	aboutItem = 1,
	aboutMSGItem,
	
	openItem = 1,
	closeItem,
	quitItem = 4,
	
	undoItem = 1,
	cutItem = 3,
	copyItem = 4,
	pasteItem = 5,
	clearItem = 6,
	
	dataForkItem = 1,
	resourceForkItem,
	forwardItem = 4,
	backItem,
	beginningItem,
	endItem,
	offsetItem,
	findItem = 10,
	findAgainItem,
	
	typeCreatorItem = 1,
	eofItem
};

extern	MenuHandle		gAppleMenu;
extern	MenuHandle		gFileMenu;
extern	MenuHandle		gEditMenu;
extern	MenuHandle		gOptionsMenu;
extern	MenuHandle		gDangerousMenu;
extern	MenuHandle		gHelpMenu;

void AdjustMenus(void);
void HandleMenu(long);
void HandleAppleMenu(int);
void HandleFileMenu(int);
void HandleEditMenu(int);
void HandleOptionsMenu(int);
void HandleDangerousMenu(int);
void HandleHelpMenu(int);
