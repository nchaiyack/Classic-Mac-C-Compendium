/**********************************************************************\

File:		cdev gui.c

Purpose:	This module handles all the DITL stuff in the cdev --
			checking and unchecking controls, etc.
			

Go Sit In The Corner -=- not you, just the cursor
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

#include "cdev globals.h"
#include "cdev gui.h"
#include "GestaltEQU.h"

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

void CheckCorner(DialogPtr theDlog, int numItems, unsigned char whichCorner)
{
	int					itemType;
	Handle				itemH;
	Rect				box;
	unsigned char		i;
	
	for (i=0x00; i<0x04; i++)
	{
		GetDItem(theDlog, kButtonTopLeft+numItems+i, &itemType, &itemH, &box);
		SetCtlValue((ControlHandle)itemH, (i==whichCorner) ? 1 : 0);
	}
}

void CheckTime(DialogPtr theDlog, int numItems, unsigned long theTime)
{
	int					i;
	Handle				itemH;
	Rect				box;
	int					itemType;
	int					index;
	
	switch (theTime)
	{
		case 900:	index=0;	break;
		case 1800:	index=1;	break;
		case 3600:	index=2;	break;
		case 7200:	index=3;	break;
		case 18000:	index=4;	break;
	}
	
	for (i=0; i<5; i++)
	{
		GetDItem(theDlog, kButtonFirstTime+numItems+i, &itemType, &itemH, &box);
		SetCtlValue((ControlHandle)itemH, (i==index) ? 1 : 0);
	}
}

void CheckAlways(DialogPtr theDlog, int numItems, unsigned char always)
{
	int					itemType;
	Handle				itemH;
	Rect				box;
	
	GetDItem(theDlog, kButtonAlways+numItems, &itemType, &itemH, &box);
	SetCtlValue((ControlHandle)itemH, always ? 1 : 0);
	GetDItem(theDlog, kButtonOnlyIf+numItems, &itemType, &itemH, &box);
	SetCtlValue((ControlHandle)itemH, always ? 0 : 1);
}
