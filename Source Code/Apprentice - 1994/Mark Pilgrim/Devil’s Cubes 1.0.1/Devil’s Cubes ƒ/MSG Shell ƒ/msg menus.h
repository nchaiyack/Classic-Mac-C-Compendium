/**********************************************************************\

File:		msg menus.h

Purpose:	This is the header file for msg menus.c


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

enum
{
	helpMenu = 200,
	
	appleMenu = 400,
	fileMenu,
	editMenu,
	optionsMenu,
	
	aboutItem = 1,
	aboutMSGItem,
	helpItem,
	
	newItem = 1,
	openItem,
	closeItem,
	saveItem,
	saveAsItem,
	quitItem = 7,
	
	undoItem = 1,
	cutItem = 3,
	copyItem,
	pasteItem,
	clearItem,
	
	mirrorItem = 1,
	showItem,
	soundToggle = 4
};

extern	Boolean			gMenuEnabled;
extern	MenuHandle		gAppleMenu;
extern	MenuHandle		gFileMenu;
extern	MenuHandle		gEditMenu;
extern	MenuHandle		gHelpMenu;
extern	MenuHandle		gOptionsMenu;

void AdjustMenus(void);
void HandleMenu(long);
void HandleAppleMenu(int);
void HandleFileMenu(int);
void HandleEditMenu(int);
void HandleHelpMenu(int);
void HandleOptionsMenu(int);
