/**********************************************************************\

File:		msg menus.h

Purpose:	This is the header file for msg menus.c


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

enum
{
	helpMenu=200,
	computerMenu,
	intelligenceMenu,
	speedMenu,
	dictionaryMenu,
	
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
	quitItem=7,
	
	undoItem = 1,
	cutItem = 3,
	copyItem,
	pasteItem,
	clearItem,
	
	saveOptions = 1,
	computerPtr = 3,
	intelligencePtr,
	dictionaryPtr,
	speedPtr,
	showMessage = 8,
	soundToggle,
	challengeWord = 11
};

extern	Boolean			gMenuEnabled;
extern	MenuHandle		gAppleMenu;
extern	MenuHandle		gFileMenu;
extern	MenuHandle		gEditMenu;
extern	MenuHandle		gOptionsMenu;
extern	MenuHandle		gHelpMenu;
extern	MenuHandle		gComputerMenu;
extern	MenuHandle		gIntelligenceMenu;
extern	MenuHandle		gSpeedMenu;
extern	MenuHandle		gDictionaryMenu;

void AdjustMenus(void);
void HandleMenu(long);
void HandleAppleMenu(int);
void HandleFileMenu(int);
void HandleEditMenu(int);
void HandleOptionsMenu(int);
void HandleHelpMenu(int);
void HandleComputerMenu(int);
void HandleIntelligenceMenu(int);
void HandleSpeedMenu(int);
void HandleDictionaryMenu(int);
