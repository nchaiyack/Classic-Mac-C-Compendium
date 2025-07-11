/**********************************************************************\

File:		fix.c

Purpose:	This module handles program initialization and the main
			procedure for checking menu IDs.


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

#include "fix.h"
#include "fix meat.h"
#include "fix generic open.h"
#include "fix files.h"
#include "fix structs.h"
#include "msg environment.h"
#include "msg graphics.h"
#include "msg menus.h"
#include "msg dialogs.h"

FSSpec			editFile;

extern	unsigned long ts:0x09d2;

void InitProgram(void)
{
	int			i,j,k;
	AppFile		myFile;
	
	editFile.name[0]=0x00;
	
	CountAppFiles(&i, &j);
	if ((j>0) && (i==0))
	{
		GetAppFiles(1, &myFile);
		MyMakeFSSpec(myFile.vRefNum, 0, myFile.fName, &editFile);
		GenericOpen(&editFile);
	}
}

void OpenTheFile(void)
{
	int				thisRefNum;
	int				oldResFile;
	int				numTotalMenus, numFixed, numIgnored;
	Str255			tempStr1, tempStr2, tempStr3;
	int				resultCode;
	unsigned long	oldTopMapHndl;
	Boolean			resFileAlreadyOpen;
	int				theResError;
	
	if (editFile.name[0]==0x00)
	{
		GetSourceFile(&editFile, FALSE);
		if (editFile.name[0]==0x00)
			return;
	}
	
	oldResFile=CurResFile();
	oldTopMapHndl=(unsigned long)TopMapHndl;
	thisRefNum=HOpenResFile(editFile.vRefNum, editFile.parID, editFile.name, fsRdWrPerm);
	theResError=ResError();
	resFileAlreadyOpen=(oldTopMapHndl==(unsigned long)TopMapHndl);
	
	if (theResError!=0)
	{
		NumToString(theResError, tempStr1);
		switch (theResError)
		{
			case -39:
				ParamText(editFile.name, "\pthis file does not have a resource fork",
					tempStr1,"\p");
				break;
			case -49:
				ParamText(editFile.name, "\pthis file is already open in another application",
					tempStr1,"\p");
				break;
			default:
				ParamText(editFile.name, "\pan unexpected error occurred",tempStr1,
					"\p");
				break;
		}
		PositionDialog('ALRT', resErrorAlert);
		StopAlert(resErrorAlert, 0L);
	}
	else
	{
		UseResFile(thisRefNum);
		gNumTotalMenus=gNumFixedMenus=gNumBadMenus=0;
		resultCode=MenuFixer(editFile, thisRefNum, resFileAlreadyOpen);
		UseResFile(oldResFile);
		if (!resFileAlreadyOpen)
			CloseResFile(thisRefNum);
		FlushVol(0L, editFile.vRefNum);
		if (resultCode!=allsWell)
		{
			ParamText(editFile.name, "\p", "\p", "\p");
			PositionDialog('ALRT', noMenusAlert);
			StopAlert(noMenusAlert, 0L);
		}
		else
		{
			NumToString(gNumTotalMenus, tempStr1);
			NumToString(gNumBadMenus, tempStr2);
			NumToString(gNumFixedMenus, tempStr3);
			ParamText(editFile.name, tempStr1, tempStr2, tempStr3);
			PositionDialog('ALRT', resultsAlert);
			Alert(resultsAlert, 0L);
		}
	}
	editFile.name[0]=0x00;
}
