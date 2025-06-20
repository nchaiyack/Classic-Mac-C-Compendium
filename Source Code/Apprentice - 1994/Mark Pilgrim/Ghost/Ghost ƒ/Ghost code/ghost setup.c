/**********************************************************************\

File:		ghost setup.c

Purpose:	This module handles initial game setup -- choosing random
			computer players, getting human player information, and
			mixing up the order of all the players.


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

#include "ghost globals.h"
#include "ghost setup.h"
#include "msg graphics.h"
#include "msg dialogs.h"
#include "msg error.h"
#include "msg prefs.h"

void MakeComputerPlayers(void)
{
	int			i,j;
	
	if (gNumComputerPlayers)
	{
		for (i=0; i<gNumComputerPlayers; i++)
		{
			gComputerIconIndex[i]=24;
			while (gComputerIconIndex[i]==24)
			{
				gComputerIconIndex[i]=(Random()%13)+12;
				j=0;
				while (j<i)
				{
					if (gComputerIconIndex[j++]==gComputerIconIndex[i])
					{
						gComputerIconIndex[i]=24;
						j=i;
					}
				}
				j=0;
				while (j<gNumHumanPlayers)
				{
					if (gComputerIconIndex[i]==gHumanIconIndex[j++])
					{
						gComputerIconIndex[i]=24;
						j=gNumHumanPlayers;
					}
				}
			}
			gComputerPlayerScore[i]=0;
		}
	}
}

Boolean GetHumanPlayers(void)
{
	int			i;
	DialogPtr	theDlog;
	DialogPtr	iconDlog;
	int			itemSelected;
	int			iconSelected;
	int			newleft;
	int			newtop;
	int			dlogID;
	Boolean		done;
	Boolean		play;
	int			itemtype;
	Handle		item;
	Rect		box;
	Str255		tempStr;

	gNumHumanPlayers=0;

	done=FALSE;
	while (!done)
	{
		PositionDialog('DLOG', addPlayerDialog);
		theDlog = GetNewDialog(addPlayerDialog, 0L, (WindowPtr)-1L);
		
		tempStr[0]=0x0f;
		tempStr[1]='P';	tempStr[2]='l';	tempStr[3]='a';	tempStr[4]='y';
		tempStr[5]='e';	tempStr[6]='r';	tempStr[7]=tempStr[10]=' ';
		tempStr[8]='#';
		tempStr[9]=(char)(49+(gNumHumanPlayers%0xff));

		tempStr[11]='s';	tempStr[12]='e';	tempStr[13]='t';
		tempStr[14]='u';	tempStr[15]='p';		
		
		SetWTitle(theDlog, tempStr);
	
		GetDItem(theDlog, 5, &itemtype, &item, &box);
	
		if (gHumanName[gNumHumanPlayers]!=0L)
		{
			for (i=((char*)(*(gHumanName[gNumHumanPlayers])))[0]; i>=0; i--)
				tempStr[i]=((char*)(*(gHumanName[gNumHumanPlayers])))[i];
			SetIText(item, tempStr);
		}
		else
			SetIText(item, gMyName);
		SelIText(theDlog, 5, 0, 32767);
		ShowWindow(theDlog);
		itemSelected=0;
		while ((itemSelected!=1) && (itemSelected!=2) && (itemSelected!=3))
		{
			ModalDialog(0L, &itemSelected);
		}
		GetIText(item, tempStr);
		HideWindow(theDlog);
		DisposeDialog(theDlog);
		
		if (itemSelected==1) /* accept, choose icon */
		{
			if (tempStr[0]!=0x00)
			{
				PositionDialog('DLOG', chooseIconDialog);
				iconDlog=GetNewDialog(chooseIconDialog, 0L, (WindowPtr)-1L);
				
				if (gHumanName[gNumHumanPlayers]!=0L)
					DisposeHandle(gHumanName[gNumHumanPlayers]);
				gHumanName[gNumHumanPlayers]=NewString(tempStr);
				SetWTitle(iconDlog, tempStr);
				ShowWindow(iconDlog);
				iconSelected=0;
				while ((iconSelected<1) || (iconSelected>26))
					ModalDialog(0L, &iconSelected);
				
				if (iconSelected==1) /* random */
				{
					gHumanIconIndex[gNumHumanPlayers]=24;
					while (gHumanIconIndex[gNumHumanPlayers]==24)
					{
						gHumanIconIndex[gNumHumanPlayers]=(Random()%13)+12;
						i=0;
						while (i<gNumHumanPlayers)
						{
							if (gHumanIconIndex[gNumHumanPlayers]==gHumanIconIndex[i++])
							{
								gHumanIconIndex[gNumHumanPlayers]=24;
								i=gNumHumanPlayers;
							}
						}
					}
				}
				else if (iconSelected>2)
					gHumanIconIndex[gNumHumanPlayers]=iconSelected-3;
				
				if (iconSelected!=2)
				{
					gHumanPlayerScore[gNumHumanPlayers]=0;
					gNumHumanPlayers++;
				}
				
				HideWindow(iconDlog);
				DisposeDialog(iconDlog);
			}
			else itemSelected=2;
		}
		done=((itemSelected!=1) || (gNumHumanPlayers==5));
		play=(itemSelected!=3);
	}
	
	return play;
}

void OrderPlayers(void)
{
	Boolean		used[10];
	int			PlayerNumber;
	int			i;
		
	gNumPlayers=gNumComputerPlayers+gNumHumanPlayers;
	for (i=0; i<gNumPlayers; i++)
		used[i]=FALSE;
	for (i=0; i<gNumPlayers; i++)
	{
		PlayerNumber=-1;
		while (PlayerNumber<0)
		{
			while (PlayerNumber<0) PlayerNumber=Random()%gNumPlayers;
			if (used[PlayerNumber])
				PlayerNumber=-1;
		}
		gPlayOrderIndex[i]=PlayerNumber;
		used[PlayerNumber]=TRUE;
	}
}

