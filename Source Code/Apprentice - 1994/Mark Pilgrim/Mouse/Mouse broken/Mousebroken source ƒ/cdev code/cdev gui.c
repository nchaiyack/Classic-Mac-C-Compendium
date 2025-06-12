/**********************************************************************\

File:		cdev gui.c

Purpose:	This module handles all the DITL stuff in the cdev --
			checking and unchecking controls, drawing the module icon.
			

Mousebroken -=- your computer isn't truly broken until it's mousebroken
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

#include "cdev globals.h"
#include "cdev gui.h"
#include "GestaltEQU.h"

static pascal void DrawModuleIcon(DialogPtr theDlog, int theItem);

void CheckShowIcon(DialogPtr theDlog, int numItems, unsigned char showIconQQ)
{
	int					itemType;
	Handle				itemH;
	Rect				box;
	
	GetDItem(theDlog, kButtonShowIcon+numItems, &itemType, &itemH, &box);
	SetCtlValue((ControlHandle)itemH, showIconQQ ? 1 : 0);
}

void CheckOnOff(DialogPtr theDlog, int numItems, Boolean checkOn)
{
	int					itemType;
	Handle				itemH;
	Rect				box;
	
	GetDItem(theDlog, kButtonOn+numItems, &itemType, &itemH, &box);
	SetCtlValue((ControlHandle)itemH, checkOn ? 1 : 0);
	GetDItem(theDlog, kButtonOff+numItems, &itemType, &itemH, &box);
	SetCtlValue((ControlHandle)itemH, checkOn ? 0 : 1);
}

void IconSetup(DialogPtr theDlog, int numItems)
{
	int					itemType;
	Handle				itemH;
	Rect				box;
	
	GetDItem(theDlog, kModuleIcon+numItems, &itemType, &itemH, &box);
	SetDItem(theDlog, kModuleIcon+numItems, userItem+itemDisable, (ProcPtr)DrawModuleIcon, &box);
}

void SetModuleSpecifics(DialogPtr theDlog, int numItems, PrefHandle cdevStorage)
{
	int					itemType;
	Handle				itemH;
	Rect				box;
	Str255				theCopyright, theInfo;
	GrafPtr				oldPort;
	
	GetPort(&oldPort);
	SetPort(theDlog);
	GetDItem(theDlog, kButtonShowIcon+numItems, &itemType, &itemH, &box);
	/* DrawModuleIcon needs access to the icon Handle, but has no access to cdevStorage, */
	/* so we'll store the icon Handle in the refcon of a control for lack of anything cleaner */
	SetCRefCon((ControlHandle)itemH, (unsigned long)(**cdevStorage).moduleIconHandle);
	DrawModuleIcon(theDlog, kModuleIcon+numItems);
	GetIndString(theCopyright, 668, 1);
	GetIndString(theInfo, 668, 2);
	GetDItem(theDlog, kModuleName+numItems, &itemType, &itemH, &box);
	SetIText((ControlHandle)itemH, (**cdevStorage).moduleFS.name);
	if (numItems>0)			/* system 6 needs to be told that it changed */
		InvalRect(&box);
	GetDItem(theDlog, kModuleCopyright+numItems, &itemType, &itemH, &box);
	SetIText((ControlHandle)itemH, theCopyright);
	if (numItems>0)
		InvalRect(&box);
	GetDItem(theDlog, kModuleInfo+numItems, &itemType, &itemH, &box);
	SetIText((ControlHandle)itemH, theInfo);
	if (numItems>0)
		InvalRect(&box);
	SetPort(oldPort);
	
	UseResFile((**cdevStorage).oldRefNum);	/* important in system 6 */
}

static pascal void DrawModuleIcon(DialogPtr theDlog, int theItem)
{
	GrafPtr				oldPort;
	OSErr				isHuman;
	unsigned long		gestalt_temp;
	int					itemType;
	Handle				itemH;
	Rect				box;
	Handle				tempHandle;

	GetPort(&oldPort);
	SetPort(theDlog);
	GetDItem(theDlog, kButtonShowIcon-kModuleIcon+theItem, &itemType, &itemH, &box);
	tempHandle=(Handle)GetCRefCon((ControlHandle)itemH);	/* stored icon handle */
	if (tempHandle!=0L)
	{
		GetDItem(theDlog, theItem, &itemType, &itemH, &box);
		isHuman = Gestalt(gestaltQuickdrawVersion, &gestalt_temp);
		if (isHuman || (gestalt_temp < gestalt8BitQD))
			PlotIcon(&box, tempHandle);
		else
			PlotCIcon(&box, tempHandle);
	}
	
	SetPort(oldPort);
}
