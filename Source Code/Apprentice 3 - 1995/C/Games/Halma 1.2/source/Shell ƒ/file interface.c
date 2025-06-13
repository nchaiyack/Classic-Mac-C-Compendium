/**********************************************************************\

File:		file interface.c

Purpose:	This module handles the standard file package (open & save
			dialogs) and returns an FSSpec to deal with.

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
#include "file interface.h"
#include "environment.h"
#include "util.h"
#include "program globals.h"

Boolean GetSourceFile(FSSpec *editFile, Boolean isText, Boolean isApplication)
/* a standard procedure which shows an open dialog box and returns the FSSpec of
   the file you selected (or returns FALSE if you cancelled) */
{
	Point				where;
	OSErr				isHuman;
	StandardFileReply	reply;
	SFReply				oldReply;
	FInfo				fndrInfo;
	long				procID;
	short				i;
	SFTypeList			theTypes;
	
	theTypes[0]=isApplication ? 'APPL' : isText ? 'TEXT' : SAVE_TYPE;
	theTypes[1]='????';
	isHuman=FALSE;			/* to err is human... */
	if (gStandardFile58)	/* in system 7 or later, all the work is done for us */
		StandardGetFile(0L, (isApplication || isText) ? 2 : 1, theTypes, &reply);	/* reply's got an FSSpec in it afterwards */
	else	/* pre-system-7, we have to do a bit of work manually */
	{
		where.h = (screenBits.bounds.right - 348)/2;	/* center horizontally */
		where.v = (screenBits.bounds.bottom - 200)/3;	/* 1:3 vertically */
		SFGetFile(where, "\p", 0L, (isApplication || isText) ? 2 : 1, theTypes, 0L, &oldReply);	/* old routine */
		
        reply.sfGood = oldReply.good;
		if (reply.sfGood)	/* if user selected a file */
		{
			reply.sfType = oldReply.fType;	/* type of file */
			isHuman=GetWDInfo(oldReply.vRefNum, &reply.sfFile.vRefNum,	/* get vRefNum */
								&reply.sfFile.parID, &procID);			/* and parID */
			if (isHuman!=noErr)
			{
				reply.sfFile.vRefNum = oldReply.vRefNum;	/* default to old method */
				reply.sfFile.parID = 0;						/* & 0 parID -- let the */
			}												/* system figure it out */
			
			/* get the name */
			Mymemcpy((Ptr)(reply.sfFile.name), (Ptr)(oldReply.fName), oldReply.fName[0]+1);
			
			/* get the finder flags of the selected file -- I've never used them,
			   but it's just for completeness; StandardGetFile() will retrieve
			   this info, so we should fake it for consistency */
			reply.sfScript=smSystemScript;
			isHuman=HGetFInfo(reply.sfFile.vRefNum, reply.sfFile.parID,
								reply.sfFile.name, &fndrInfo);
			reply.sfFlags=(isHuman==noErr) ? fndrInfo.fdFlags : 0;
			
			reply.sfIsFolder=FALSE;
			reply.sfIsVolume=FALSE;
		}
	}

	if ((reply.sfGood) && (!isHuman))	/* make the actual FSSpec */
		MyMakeFSSpec(reply.sfFile.vRefNum, reply.sfFile.parID, reply.sfFile.name,
							editFile);
		
	return ((reply.sfGood) && (!isHuman));	/* TRUE if we have a valid file selected */
}

Boolean GetDestFile(FSSpec *destFS, Boolean *deleteTheThing, Str255 theTitle)
/* a standard save dialog box -- given a title (for the prompt), it returns the
   file's FSSpec in destFS and whether a file of that name already exists in
   deleteTheThing (TRUE if file already exists) */
/* details are pretty much the same as GetSourceFile(), see above */
{
	Str255				otherName;
	StandardFileReply	reply;
	SFReply				oldReply;
	Point				where;
	FInfo				fndrInfo;
	long				procID;
	OSErr				isHuman, err;
	short				i;
		
	isHuman=FALSE;
	if (gStandardFile58)
		StandardPutFile(theTitle, destFS->name, &reply);
	else
	{
		where.h = (screenBits.bounds.right - 304)/2;
		where.v = (screenBits.bounds.bottom - 184)/3;
		SFPutFile(where, theTitle, destFS->name, 0, &oldReply);
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
			
			Mymemcpy((Ptr)reply.sfFile.name, (Ptr)oldReply.fName, oldReply.fName[0]+1);
			
			reply.sfScript = smSystemScript;
			err=HGetFInfo(reply.sfFile.vRefNum, reply.sfFile.parID,
								reply.sfFile.name, &fndrInfo);
			reply.sfReplacing=(err!=fnfErr);
			
			reply.sfIsFolder=FALSE;
			reply.sfIsVolume=FALSE;
		}
	}

	if ((reply.sfGood) && (!isHuman))
	{
		*deleteTheThing=reply.sfReplacing;	
		MyMakeFSSpec(reply.sfFile.vRefNum, reply.sfFile.parID, reply.sfFile.name,
						destFS);
	}
	
	return ((reply.sfGood) && (!isHuman));
}

pascal OSErr MyMakeFSSpec(short vRefNum, long parID, Str255 fileName,
	FSSpecPtr myFSS)
/* a standard function for creating an FSSpec out of its three component parts.
   if the system supports FSSpecs, we let it do the work; otherwise we have to
   do it manually. */
{
	if (gHasFSSpecs)
		return FSMakeFSSpec(vRefNum, parID, fileName, myFSS);
	else
	{
		myFSS->vRefNum=vRefNum;
		myFSS->parID=parID;
		Mymemcpy((Ptr)(myFSS->name), (Ptr)fileName, fileName[0]+1);
	}
	
	return noErr;
}
