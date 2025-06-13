/**********************************************************************\

File:		cdev meat.c

Purpose:	This module handles stuff internal to the cdev -- checking
			the right items and handling buttons.  Also installing &
			de-installing the actual mouse VBL.
			

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
#include "cdev meat.h"
#include "cdev prefs.h"
#include "cdev.h"
#include "Retrace.h"
#include "Folders.h"
#include "GestaltEQU.h"

#define MODULE_TYPE		'MMdl'
#define MODULE_CREATOR	'MBrk'
#define MODULE_FOLDER	"\pMouse Modules"

void RemoveTheMouseVBL(PrefHandle cdevStorage)
{
	if (((**cdevStorage).moduleIndex!=0) &&
		((**cdevStorage).mouseVBLPtr>=(unsigned long)RAMBase) &&
		((**cdevStorage).mouseVBLPtr<(unsigned long)MemTop) &&
		((**cdevStorage).mouseCodePtr>=(unsigned long)RAMBase) &&
		((**cdevStorage).mouseCodePtr<(unsigned long)MemTop) &&
		(GetPtrSize((Ptr)((**cdevStorage).mouseVBLPtr))==0x0e) &&
		(VRemove((VBLTask*)((**cdevStorage).mouseVBLPtr))==noErr))
	{
		DisposePtr((Ptr)((**cdevStorage).mouseVBLPtr));
		DisposeHandle((Handle)((**cdevStorage).mouseCodePtr));
	}
}

int InstallTheMouseVBL(PrefHandle cdevStorage)
{
	VBLTask			*vblPtr;
	Handle			ourCode;
	int				oldResFile;
	
	oldResFile=CurResFile();
	UseResFile((**cdevStorage).moduleRefNum);
	ourCode=0L;
	ourCode=GetResource('vbl ', 668);
	UseResFile(oldResFile);
	if (ourCode!=0L)
	{
		if (*ourCode==0L)
			LoadResource(ourCode);
		if (*ourCode==0L)
			return kCantGetResource;
		
		HLock(ourCode);
		DetachResource(ourCode);
		
		vblPtr = (VBLTask*) NewPtrSys(sizeof(VBLTask));
		vblPtr->qType = vType;
		vblPtr->vblAddr = (ProcPtr) *ourCode;
		vblPtr->vblCount = 1;
		vblPtr->vblPhase = 0;
		VInstall(vblPtr);
		(**cdevStorage).mouseCodePtr=(unsigned long)ourCode;
		(**cdevStorage).mouseVBLPtr=(unsigned long)vblPtr;
	}
	else return kCantGetResource;

	return prefs_allsWell;
}

int GetModuleInfo(PrefHandle cdevStorage)
{
	OSErr			isHuman;
	int				vRefNum;
	long			dirID;
	CInfoPBRec		pb_dir;
	
	if ((**cdevStorage).moduleIndex==0)
		return GetModuleInfoFromIndex(cdevStorage, 1);
	
	isHuman=FindFolder(kOnSystemDisk, kSystemFolderType, kDontCreateFolder, &vRefNum,
				&dirID);					/* find system folder */
	if (isHuman!=noErr)
		return kCantFindSystemFolder;
	
	pb_dir.dirInfo.ioCompletion=0L;
	pb_dir.dirInfo.ioNamePtr=MODULE_FOLDER;
	pb_dir.dirInfo.ioVRefNum=vRefNum;
	pb_dir.dirInfo.ioFDirIndex=0; 			/* very important */
	pb_dir.dirInfo.ioDrDirID=dirID;
	isHuman=PBGetCatInfo(&pb_dir, FALSE);	/* get info on "Mouse Modules" dir */
	if (isHuman!=noErr)
		return kCantOpenModuleFolder;

	if (!AtLeastOneModule(vRefNum, pb_dir.dirInfo.ioDrDirID))
		return kNoModules;					/* check if there are any modules at all */
	
	return FindCurrentModule(cdevStorage, vRefNum, pb_dir.dirInfo.ioDrDirID,
			pb_dir.dirInfo.ioDrNmFls);
}

int GetModuleInfoFromIndex(PrefHandle cdevStorage, int direction)
{
	OSErr			isHuman;
	int				vRefNum;
	long			dirID;
	CInfoPBRec		pb_dir;
	HParamBlockRec	pb_basic;
	Str63			theName;
	int				i;
	int				tempIndex;
	int				resultCode;
	
	isHuman=FindFolder(kOnSystemDisk, kSystemFolderType, kDontCreateFolder, &vRefNum,
				&dirID);					/* find system folder */
	if (isHuman!=noErr)
		return kCantFindSystemFolder;
	
	pb_dir.dirInfo.ioCompletion=0L;
	pb_dir.dirInfo.ioNamePtr=MODULE_FOLDER;
	pb_dir.dirInfo.ioVRefNum=vRefNum;
	pb_dir.dirInfo.ioFDirIndex=0; /* very important */
	pb_dir.dirInfo.ioDrDirID=dirID;
	isHuman=PBGetCatInfo(&pb_dir, FALSE);	/* get info on "Mouse Modules" dir */
	if (isHuman!=noErr)
		return kCantOpenModuleFolder;

	if (!AtLeastOneModule(vRefNum, pb_dir.dirInfo.ioDrDirID))
		return kNoModules;					/* check if there are any modules at all */
	
	if ((**cdevStorage).moduleIndex>0)
	{
		resultCode=FindCurrentModule(cdevStorage, vRefNum, pb_dir.dirInfo.ioDrDirID,
					pb_dir.dirInfo.ioDrNmFls);	/* get index # of current module */
		if (resultCode!=allsWell)
			return resultCode;
	}
	
	tempIndex=(**cdevStorage).moduleIndex;
	do
	{
		tempIndex+=direction;
		if (tempIndex==0)
			tempIndex=pb_dir.dirInfo.ioDrNmFls;
		else if (tempIndex>pb_dir.dirInfo.ioDrNmFls)
			tempIndex=1;
		
		pb_basic.fileParam.ioCompletion=0L;
		pb_basic.fileParam.ioNamePtr=theName;
		pb_basic.fileParam.ioVRefNum=vRefNum;
		pb_basic.fileParam.ioFDirIndex=tempIndex;
		pb_basic.fileParam.ioDirID=pb_dir.dirInfo.ioDrDirID;
		isHuman=PBGetCatInfo(&pb_basic, FALSE);	/* get info on previous/next module */
		if (isHuman!=noErr)
			return kCantOpenModule;
	}
	while ((pb_basic.fileParam.ioFlFndrInfo.fdType!=MODULE_TYPE) ||
			(pb_basic.fileParam.ioFlFndrInfo.fdCreator!=MODULE_CREATOR));
	
	(**cdevStorage).moduleIndex=tempIndex;
	(**cdevStorage).moduleFS.vRefNum=vRefNum;
	(**cdevStorage).moduleFS.parID=pb_dir.dirInfo.ioDrDirID;
		for (i=theName[0]; i>=0; i--)
		(**cdevStorage).moduleFS.name[i]=theName[i];
	return allsWell;
}

Boolean AtLeastOneModule(int vRefNum, long dirID)
{
	HParamBlockRec		pb_basic;
	int					dirIter;
	Boolean				foundModule;
	Str63				theName;
	OSErr				isHuman;
	
	dirIter=0;
	do
	{
		dirIter++;
		pb_basic.fileParam.ioCompletion=0L;
		pb_basic.fileParam.ioNamePtr=theName;
		pb_basic.fileParam.ioVRefNum=vRefNum;
		pb_basic.fileParam.ioFDirIndex=dirIter;
		pb_basic.fileParam.ioDirID=dirID;
		isHuman=PBGetCatInfo(&pb_basic, FALSE);
		if (isHuman!=noErr)
			return FALSE;
		foundModule=(pb_basic.fileParam.ioFlFndrInfo.fdType==MODULE_TYPE) &&
			(pb_basic.fileParam.ioFlFndrInfo.fdCreator==MODULE_CREATOR);
	}
	while ((isHuman==noErr) && (!foundModule));
	
	return (isHuman==noErr);
}

int FindCurrentModule(PrefHandle cdevStorage, int vRefNum, long dirID, int numFiles)
{
	int				tempIndex;
	Boolean			isCorrectModule;
	HParamBlockRec	pb_basic;
	Str63			theName;
	OSErr			isHuman;
	int				i;
	
	isCorrectModule=FALSE;
	for (tempIndex=1; (tempIndex<=numFiles) && (!isCorrectModule); tempIndex++)
	{
		pb_basic.fileParam.ioCompletion=0L;
		pb_basic.fileParam.ioNamePtr=theName;
		pb_basic.fileParam.ioVRefNum=vRefNum;
		pb_basic.fileParam.ioFDirIndex=tempIndex;
		pb_basic.fileParam.ioDirID=dirID;
		isHuman=PBGetCatInfo(&pb_basic, FALSE);	/* get info on current module */
		if (isHuman!=noErr)
			return kCantOpenModule;
		isCorrectModule=(pb_basic.fileParam.ioFlFndrInfo.fdType==MODULE_TYPE) &&
			(pb_basic.fileParam.ioFlFndrInfo.fdCreator==MODULE_CREATOR);
		for (i=(**cdevStorage).moduleFS.name[0]; (i>=0) && (isCorrectModule); i--)
			isCorrectModule=((**cdevStorage).moduleFS.name[i]==theName[i]);
	}
	
	if (!isCorrectModule)
	{
		(**cdevStorage).moduleIndex=0;
		return GetModuleInfoFromIndex(cdevStorage, 1);
	}

	(**cdevStorage).moduleIndex=tempIndex-1;
	(**cdevStorage).moduleFS.vRefNum=vRefNum;
	(**cdevStorage).moduleFS.parID=dirID;
	
	return allsWell;
}

int OpenTheModule(PrefHandle cdevStorage)
{
	unsigned long		oldTopMapHndl;
	OSErr				theResError;
	OSErr				isHuman;
	unsigned long		gestalt_temp;
	THz					saveZone;
	
	(**cdevStorage).oldRefNum=CurResFile();
	oldTopMapHndl=(unsigned long)TopMapHndl;
	(**cdevStorage).moduleRefNum=HOpenResFile((**cdevStorage).moduleFS.vRefNum,
		(**cdevStorage).moduleFS.parID, (**cdevStorage).moduleFS.name, fsRdPerm);
	theResError=ResError();
	(**cdevStorage).moduleAlreadyOpen=(oldTopMapHndl==(unsigned long)TopMapHndl);
	if (theResError==noErr)
	{
		UseResFile((**cdevStorage).moduleRefNum);
		isHuman = Gestalt(gestaltQuickdrawVersion, &gestalt_temp);
		(**cdevStorage).moduleIconHandle=0L;
		saveZone=GetZone();
		SetZone(SysZone);
		(**cdevStorage).moduleIconHandle=(isHuman || (gestalt_temp < gestalt8BitQD)) ?
			(unsigned long)GetIcon(668) : (unsigned long)GetCIcon(668);
		if ((**cdevStorage).moduleIconHandle!=0L)
		{
			HLock((Handle)(**cdevStorage).moduleIconHandle);
			HNoPurge((Handle)(**cdevStorage).moduleIconHandle);
		}
		SetZone(saveZone);
	}
	
	return (theResError==noErr) ? allsWell : kCantOpenModule;
}

int CloseTheModule(PrefHandle cdevStorage)
{
	OSErr				isHuman;
	unsigned long		gestalt_temp;
	THz					saveZone;
	
	if (!((**cdevStorage).moduleAlreadyOpen))
		CloseResFile((**cdevStorage).moduleRefNum);
	isHuman = Gestalt(gestaltQuickdrawVersion, &gestalt_temp);
	saveZone=GetZone();
	SetZone(SysZone);
	if (isHuman || (gestalt_temp < gestalt8BitQD))
		ReleaseResource((Handle)(**cdevStorage).moduleIconHandle);
	else
		DisposCIcon((CIconHandle)(**cdevStorage).moduleIconHandle);
	SetZone(saveZone);
	UseResFile((**cdevStorage).oldRefNum);
}
