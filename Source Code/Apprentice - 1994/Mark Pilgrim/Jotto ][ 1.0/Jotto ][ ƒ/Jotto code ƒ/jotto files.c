/**********************************************************************\

File:		jotto files.c

Purpose:	This module handles standard file dialogs for loading
			and saving games on disk.


Jotto ][ -=- a simple word game, revisited
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

#include "Script.h"
#include "jotto files.h"
#include "jotto load-save.h"
#include "jotto globals.h"
#include "msg graphics.h"
#include "msg environment.h"

Boolean GetSourceFile(FSSpec *gameFile, Boolean useOldGame)
{
	Point				where;
	OSErr				isHuman;
	StandardFileReply	reply;
	SFReply				oldReply;
	unsigned int		count;
	SFTypeList			typeList;
	FInfo				fndrInfo;
	long				procID;
	int					i;
	
	typeList[0]=SAVE_TYPE;
	isHuman=FALSE;
	if ((gStandardFile58) && (!useOldGame))
		StandardGetFile(0L, 1, typeList, &reply);
	else
	{
		where.h = (gMainScreenBounds.right - 348)/2;
		where.v = (gMainScreenBounds.bottom - 200)/3;
		SFGetFile(where, "\p", 0L, 1, typeList, 0L, &oldReply);
		
        reply.sfGood = oldReply.good;
		if (reply.sfGood)
		{
			reply.sfType = oldReply.fType;
			isHuman=GetWDInfo(oldReply.vRefNum, &reply.sfFile.vRefNum,
								&reply.sfFile.parID, &procID);
			if (isHuman!=noErr)
			{
				reply.sfFile.vRefNum = oldReply.vRefNum;
				reply.sfFile.parID = 0;
			}
			count=oldReply.fName[0];
			for (i=0; i<=count; i++)
				reply.sfFile.name[i]=oldReply.fName[i];
			
			reply.sfScript=smSystemScript;
			isHuman=HGetFInfo(reply.sfFile.vRefNum, reply.sfFile.parID,
								reply.sfFile.name, &fndrInfo);
			reply.sfFlags=(isHuman==noErr) ? fndrInfo.fdFlags : 0;
			
			reply.sfIsFolder=FALSE;
			reply.sfIsVolume=FALSE;
		}
	}

	if ((reply.sfGood) && (!isHuman) && (!useOldGame))
		MyMakeFSSpec(reply.sfFile.vRefNum, reply.sfFile.parID, reply.sfFile.name,
							gameFile);
		
	return ((reply.sfGood) && (!isHuman));
}

Boolean GetDestFile(FSSpec *gameFile, Boolean *deleteTheThing)
{
	unsigned int		count;
	Str255				otherName;
	StandardFileReply	reply;
	SFReply				oldReply;
	Point				where;
	FInfo				fndrInfo;
	long				procID;
	OSErr				isHuman;
	int					i;
		
	if (gStandardFile58)
		StandardPutFile("\pSave Jotto ][ game as...", "\p", &reply);
	else
	{
		where.h = (gMainScreenBounds.right - 304)/2;
		where.v = (gMainScreenBounds.bottom - 184)/3;
		SFPutFile(where, "\pSave Jotto ][ game as...", "\p", 0, &oldReply);
		reply.sfGood = oldReply.good;
		if (reply.sfGood)
        {
			isHuman=GetWDInfo(oldReply.vRefNum, &reply.sfFile.vRefNum,
								&reply.sfFile.parID, &procID);
			if (isHuman!=noErr)
			{
				reply.sfFile.vRefNum = oldReply.vRefNum;
				reply.sfFile.parID = 0;
			}			
			count=oldReply.fName[0];
			for (i=0; i<=count; i++)
				reply.sfFile.name[i]=oldReply.fName[i];
			
			reply.sfScript = smSystemScript;
			isHuman=HGetFInfo(reply.sfFile.vRefNum, reply.sfFile.parID,
								reply.sfFile.name, &fndrInfo);
			reply.sfReplacing=(isHuman!=fnfErr);
			
			reply.sfIsFolder=FALSE;
			reply.sfIsVolume=FALSE;
		}
	}

	if (reply.sfGood)
	{
		*deleteTheThing=reply.sfReplacing;	
		MyMakeFSSpec(reply.sfFile.vRefNum, reply.sfFile.parID, reply.sfFile.name,
						gameFile);
	}
	
	return (reply.sfGood);
}

pascal OSErr MyMakeFSSpec(short vRefNum, long parID, ConstStr255Param fileName,
	FSSpecPtr myFSS)
{
	int			i;
	
	if (gHasFSSpecs)
		FSMakeFSSpec(vRefNum, parID, fileName, myFSS);
	else
	{
		myFSS->vRefNum=vRefNum;
		myFSS->parID=parID;
		for (i=fileName[0]; i>=0; i--)
			myFSS->name[i]=fileName[i];
	}
}
