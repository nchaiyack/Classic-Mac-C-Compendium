/**********************************************************************\

File:		msg environment.c

Purpose:	This module handles initializing the environment and
			checking for various environmental characteristics.


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

#include "GestaltEqu.h"
#include "Traps.h"
#include "msg environment.h"
#include "msg apple events.h"
#include "msg graphics.h"
#include "msg menus.h"

Boolean			gHasPowerManager;
Boolean			gHasAppleEvents;
Boolean			gHasFSSpecs;
Boolean			gStandardFile58;
Boolean			gHasColorQD;

Boolean			gInProgress;
Boolean			gDone;

Rect			gDragRect;

void CheckEnvironment(void)
{
	long	gestalt_temp;
	OSErr	isHuman;
	
	isHuman = Gestalt(gestaltQuickdrawVersion, &gestalt_temp);
	gHasColorQD = !(isHuman || (gestalt_temp < gestalt8BitQD));
	
	GetMainScreenBounds();
	
	isHuman = Gestalt(gestaltFSAttr, &gestalt_temp);
	gHasFSSpecs=((isHuman==noErr) && (gestalt_temp & (1 << gestaltHasFSSpecCalls)));

	isHuman = Gestalt(gestaltStandardFileAttr, &gestalt_temp);
	gStandardFile58=((isHuman==noErr) && (gestalt_temp & (1 << gestaltStandardFile58)));

	gHasAppleEvents=0;
	isHuman = Gestalt(gestaltAppleEventsAttr, &gestalt_temp);
	if (!isHuman)
	{
		gHasAppleEvents = 1;
		SetUpAppleEvents();
	}
}

void InitEnvironment(void)
{
	Handle		MBARHandle;
	
	MBARHandle = GetNewMBar(400);
	SetMenuBar(MBARHandle);
	gAppleMenu = GetMHandle(appleMenu);
	gFileMenu = GetMHandle(fileMenu);
	gEditMenu = GetMHandle(editMenu);
	gOptionsMenu = GetMHandle(optionsMenu);
	
	gHelpMenu = GetMenu(helpMenu);
	InsertMenu(gHelpMenu, -1);
	
	gComputerMenu = GetMenu(computerMenu);
	InsertMenu(gComputerMenu, -1);
	
	gIntelligenceMenu = GetMenu(intelligenceMenu);
	InsertMenu(gIntelligenceMenu, -1);
	
	gSpeedMenu = GetMenu(speedMenu);
	InsertMenu(gSpeedMenu, -1);
	
	gDictionaryMenu = GetMenu(dictionaryMenu);
	InsertMenu(gDictionaryMenu, -1);
	
	AddResMenu(gAppleMenu, 'DRVR');
	
	AdjustMenus();
	DrawMenuBar();
	
	if(gHasColorQD)
	{
		gDragRect = (*GetGrayRgn())->rgnBBox;
	}
	else
	{
		gDragRect = screenBits.bounds;
	}
	
	gDragRect.top += 4;
	gDragRect.left += 4;
	gDragRect.right -= 4;
	gDragRect.bottom -= 4;
	
	gInProgress=FALSE;
	gDone=FALSE;
}
