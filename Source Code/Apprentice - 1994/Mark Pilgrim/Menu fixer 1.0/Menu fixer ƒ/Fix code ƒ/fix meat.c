/**********************************************************************\

File:		fix meat.c

Purpose:	This module handles actually checking menu resource IDs
			and internal menuIDs and alerting the user of discrepancies.


Menu Fixer -=- synchronize menu IDs and menu resource IDs
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

#include "fix structs.h"
#include "fix meat.h"
#include "msg dialogs.h"

int			gNumTotalMenus;
int			gNumFixedMenus;
int			gNumBadMenus;

int MenuFixer(FSSpec myFSS, int thisRefNum, Boolean resFileAlreadyOpen)
{
	MenuHandle		thisMenu;
	ResType			thisResType;
	unsigned int	thisID;
	Str255			thisName;
	int				i;
	
	gNumTotalMenus=Count1Resources('MENU');
	if (gNumTotalMenus==0)
		return noMenusErr;
	
	for (i=1; i<=gNumTotalMenus; i++)
	{
		thisMenu=GetIndResource('MENU', i);
		if (*thisMenu==0L)
			LoadResource(thisMenu);
		GetResInfo(thisMenu, &thisID, &thisResType, thisName);
		if (thisID!=**((unsigned int**)thisMenu))
		{
			BadMenu(myFSS, thisRefNum, thisMenu, thisID, thisName);
			gNumBadMenus++;
		}
		if (!resFileAlreadyOpen)
			ReleaseResource(thisMenu);
	}
	
	return allsWell;
}

void BadMenu(FSSpec myFSS, int thisRefNum, MenuHandle thisMenu, unsigned int thisID,
	Str255 thisName)
{
	Str255			tempStr, tempStr2;
	int				alertResult;
	
	NumToString(thisID, tempStr);
	NumToString(**((unsigned int**)thisMenu), tempStr2);
	ParamText(myFSS.name, thisName, tempStr, tempStr2);
	PositionDialog('ALRT', badMenuAlert);
	alertResult=Alert(badMenuAlert, 0L);
	if (alertResult==2)	/* correct the problem */
		CorrectMenu(myFSS, thisRefNum, thisMenu, thisID, thisName);
}

void CorrectMenu(FSSpec myFSS, int thisRefNum, MenuHandle thisMenu, unsigned int thisID,
	Str255 thisName)
{
	**((unsigned int**)thisMenu)=thisID;
	ChangedResource(thisMenu);
	WriteResource(thisMenu);
	UpdateResFile(thisRefNum);
	gNumFixedMenus++;
}
