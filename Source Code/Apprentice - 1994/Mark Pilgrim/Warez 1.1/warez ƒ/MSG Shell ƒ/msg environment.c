/**********************************************************************\

File:		msg environment.c

Purpose:	This module handles initializing the environment and
			checking for various environmental characteristics.


WAREZ -=- nostalgia isn't what it used to be
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

#include "GestaltEqu.h"
#include "msg environment.h"
#include "msg apple events.h"
#include "msg graphics.h"
#include "msg menus.h"
#include "program globals.h"

Boolean			gHasColorQD;
Boolean			gHasAppleEvents;
Boolean			gHasFSSpecs;
Boolean			gStandardFile58;
Boolean			gHasPowerManager;

Boolean			gIsInBackground;
Boolean			gInProgress;
Boolean			gDone;
Boolean			gDisableQuit;

Rect			gDragRect;
Rect			gSizeRect;

void CheckEnvironment(void)
{
	long	gestalt_temp;
	OSErr	isHuman;
	
	isHuman = Gestalt(gestaltQuickdrawVersion, &gestalt_temp);
	gHasColorQD = !(isHuman || (gestalt_temp < gestalt8BitQD));
	
	GetMainScreenBounds();
	
	isHuman = Gestalt(gestaltFSAttr, &gestalt_temp);
	gHasFSSpecs=((isHuman==noErr) && (gestalt_temp & (1 << gestaltHasFSSpecCalls)));

	isHuman=Gestalt(gestaltPowerMgrAttr, &gestalt_temp);
	gHasPowerManager=((!isHuman) && (gestalt_temp&(1<<gestaltPMgrCPUIdle)));
	
	gHasAppleEvents=0;
	isHuman = Gestalt(gestaltAppleEventsAttr, &gestalt_temp);
	if(!isHuman)
	{
		gHasAppleEvents = 1;
		SetUpAppleEvents();
	}
}

void InitEnvironment(void)
{
	Handle		MBARHandle;
	int			i;
	
	MBARHandle = GetNewMBar(400);
	SetMenuBar(MBARHandle);
	gAppleMenu = GetMHandle(appleMenu);
	gFileMenu = GetMHandle(fileMenu);
	gEditMenu = GetMHandle(editMenu);
	gOptionsMenu = GetMHandle(optionsMenu);
	
	gHelpMenu = GetMenu(helpMenu);
	InsertMenu(gHelpMenu, -1);
	
	AddResMenu(gAppleMenu, 'DRVR');
	
	gInProgress=FALSE;
	
	AdjustMenus();
	DrawMenuBar();
	
	gNumHelp=CountMItems(gHelpMenu);
	
	gDragRect = screenBits.bounds;
	gDragRect.top += 4;
	gDragRect.left += 4;
	gDragRect.right -= 4;
	gDragRect.bottom -= 4;
	
	gDone=FALSE;
	gIsInBackground=FALSE;
}
