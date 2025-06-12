/**********************************************************************\

File:		msg menus.h

Purpose:	This is the header file for msg menus.c


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

enum
{
	sub1 = 200,
	sub2,
	sub3,
	sub4,
	helpMenu,
	
	appleMenu = 400,
	fileMenu,
	editMenu,
	boardMenu,
	fluffMenu,
	
	aboutItem = 1,
	aboutMSGItem,
	helpItem,
	
	newItem = 1,
	openItem,
	closeItem,
	saveItem,
	saveAsItem,
	loadSolutionItem=7,
	saveSolutionItem,
	quitItem = 10,
	
	undoItem = 1,
	cutItem = 3,
	copyItem = 4,
	pasteItem = 5,
	clearItem = 6,
	
	placeItem = 1,
	removeItem,
	soundToggle = 4
};

extern	Boolean			gMenusActive;
extern	Boolean			gMenuEnabled;
extern	MenuHandle		gAppleMenu;
extern	MenuHandle		gFileMenu;
extern	MenuHandle		gEditMenu;
extern	MenuHandle		gBoardMenu;
extern	MenuHandle		gFluffMenu;
extern	MenuHandle		gSub1;
extern	MenuHandle		gSub2;
extern	MenuHandle		gSub3;
extern	MenuHandle		gSub4;
extern	MenuHandle		gHelpMenu;
void AdjustMenus(void);
void HandleMenu(long);
void HandleAppleMenu(int);
void HandleFileMenu(int);
void HandleEditMenu(int);
void HandleBoardMenu(int);
void HandleSub1(int);
void HandleSub2(int);
void HandleSub3(int);
void HandleSub4(int);
void HandleHelpMenu(int);
void HandleFluffMenu(int);
void CheckAppropriate(void);
