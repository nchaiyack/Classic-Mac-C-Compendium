/**********************************************************************\

File:		msg menus.h

Purpose:	This is the header file for msg menus.c


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

enum
{
	helpMenu=200,
	
	appleMenu = 400,
	fileMenu,
	editMenu,
	optionsMenu,
	
	aboutItem = 1,
	aboutMSGItem,
	
	newItem = 1,
	openItem,
	closeItem,
	saveItem,
	saveAsItem,
	quitItem = 7,
	
	undoItem = 1,
	cutItem = 3,
	copyItem = 4,
	pasteItem = 5,
	clearItem = 6,
	
	dupToggle = 1,
	nonWordsToggle,
	useFiveLetter = 4,
	useSixLetter,
	soundToggle = 7,
	animationToggle
};

extern	Boolean			gMenuEnabled;
extern	MenuHandle		gAppleMenu;
extern	MenuHandle		gFileMenu;
extern	MenuHandle		gEditMenu;
extern	MenuHandle		gOptionsMenu;
extern	MenuHandle		gHelpMenu;

void AdjustMenus(void);
void HandleMenu(long);
void HandleAppleMenu(int);
void HandleFileMenu(int);
void HandleEditMenu(int);
void HandleOptionsMenu(int);
void HandleHelpMenu(int);
