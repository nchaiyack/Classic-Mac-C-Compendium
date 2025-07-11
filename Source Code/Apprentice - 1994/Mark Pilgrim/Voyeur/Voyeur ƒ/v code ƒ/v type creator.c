/**********************************************************************\

File:		v type creator.c

Purpose:	This module handles displaying and modifying the file
			type and creator.


Voyeur -- a no-frills file viewer
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

#include "v type creator.h"
#include "v structs.h"
#include "v error.h"
#include "msg dialogs.h"

void ChangeTypeCreator(void)
{
	DialogPtr		theDlog;
	int				itemSelected;
	Handle			itemH;
	short			item,itemType;
	Rect			box;
	Str255			tempStr, tempStr2;
	unsigned long	theType, theCreator;
	int				resultCode;
	
	resultCode=TypeCreatorDispatch(TRUE, TRUE, &theType, editFile);
	if (resultCode!=allsWell)
	{
		HandleError(resultCode);
		return;
	}
	resultCode=TypeCreatorDispatch(FALSE, TRUE, &theCreator, editFile);
	if (resultCode!=allsWell)
	{
		HandleError(resultCode);
		return;
	}

	ParamText(editFile.name, "\p", "\p", "\p");
	
	PositionDialog('DLOG', typeCreatorDialog);
	theDlog = GetNewDialog(typeCreatorDialog, 0L, (WindowPtr)-1L);

	GetDItem(theDlog, 3, &itemType, &itemH, &box);
	InsetRect(&box, -4, -4);
	SetDItem(theDlog, 3, itemType, (Handle)OutlineDefaultButton, &box);
	
	GetDItem(theDlog,7,&itemType,&itemH,&box);
	LongToString(theType, tempStr);
	SetIText(itemH, tempStr);
	GetDItem(theDlog,8,&itemType,&itemH,&box);
	LongToString(theCreator, tempStr);
	SetIText(itemH, tempStr);
	SelIText(theDlog, 7, 0, 32767);
	SetWTitle((WindowPtr)theDlog, "\pType & creator");
	
	ShowWindow(theDlog);
	
	itemSelected=0;
	while ((itemSelected!=1) && (itemSelected!=2))
	{
		ModalDialog(0L, &itemSelected);
	}
	
	if (itemSelected==1)
	{
		GetDItem(theDlog,7,&itemType,&itemH,&box);
		GetIText(itemH, tempStr);
		GetDItem(theDlog,8,&itemType,&itemH,&box);
		GetIText(itemH, tempStr2);
	}

	HideWindow(theDlog);
	DisposeDialog(theDlog);

	if (itemSelected==1)
	{
		if ((tempStr[0]!=0x04) || (tempStr2[0]!=0x04))
		{
			HandleError(badTypeCreatorErr);
			return;
		}
		theType=StringToLong(tempStr);
		theCreator=StringToLong(tempStr2);
		
		resultCode=TypeCreatorDispatch(TRUE, FALSE, &theType, editFile);
		if (resultCode!=allsWell)
		{
			HandleError(resultCode);
			return;
		}
		resultCode=TypeCreatorDispatch(FALSE, FALSE, &theCreator, editFile);
		if (resultCode!=allsWell)
		{
			HandleError(resultCode);
			return;
		}
	}
}

int TypeCreatorDispatch(Boolean isType, Boolean justGetValue, unsigned long *theLong,
	FSSpec editFile)
{
	HParamBlockRec		pb;
	OSErr				isHuman;
	
	pb.fileParam.ioCompletion=0L;
	pb.fileParam.ioNamePtr=editFile.name;
	pb.fileParam.ioFDirIndex=0;
	pb.fileParam.ioVRefNum=editFile.vRefNum;
	pb.fileParam.ioDirID=editFile.parID;
	isHuman=PBHGetFInfo(&pb, FALSE);
	if (isHuman!=allsWell)
		return diskReadErr;
	
	if (justGetValue)
	{
		*theLong=isType ? pb.fileParam.ioFlFndrInfo.fdType :
			pb.fileParam.ioFlFndrInfo.fdCreator;
		return allsWell;
	}
	else
	{
		pb.fileParam.ioNamePtr=editFile.name;
		pb.fileParam.ioDirID=editFile.parID;
		if (isType)
			pb.fileParam.ioFlFndrInfo.fdType=*theLong;
		else
			pb.fileParam.ioFlFndrInfo.fdCreator=*theLong;
		isHuman=PBHSetFInfo(&pb, FALSE);
		return (isHuman==noErr) ? allsWell : diskWriteErr;
	}
}

void LongToString(unsigned long input, Str255 tempStr)
{
	tempStr[0]=0x04;
	tempStr[1]=(input>>24)&0xff;
	tempStr[2]=(input>>16)&0xff;
	tempStr[3]=(input>>8)&0xff;
	tempStr[4]=input&0xff;
}

unsigned long StringToLong(Str255 tempStr)
{
	unsigned long	temp;
	
	temp=tempStr[1];
	temp<<=8;
	temp|=tempStr[2];
	temp<<=8;
	temp|=tempStr[3];
	temp<<=8;
	temp|=tempStr[4];
	return temp;
}
