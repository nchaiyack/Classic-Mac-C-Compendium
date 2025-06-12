/**********************************************************************\

File:		msg prefs.c

Purpose:	This module handles creating/opening/closing/updating
			the preference file, and copying the preference file
			data into application globals (and back).


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

#include "msg prefs.h"
#include "msg dialogs.h"
#include "msg graphics.h"
#include "msg environment.h"
#include "msg sounds.h"
#include "util.h"
#include "jotto globals.h"

long				gFileID;
Str255				gMyName;
Str255				gMyOrg;
Boolean				gCanSavePrefs;

static PrefStruct	thePrefs;
static long			gPrefsFilePos;

int PreferencesInit(void)
{
	int			prefsFileID;
	int			err;
	
	gCanSavePrefs=FALSE;
	err=GetFileID();
	if (err!=prefs_allsWell)
		return err;
	
	err=OpenPrefsFile(&prefsFileID);
	if (err!=prefs_allsWell)
	{
		if ((err==prefs_diskReadErr) || (err==prefs_diskWriteErr) || (err==prefs_virginErr))
			ClosePrefsFile(prefsFileID);
		return err;
	}
	
	err=CheckVersion(prefsFileID);
	if (err!=prefs_allsWell)
	{
		ClosePrefsFile(prefsFileID);
		return err;
	}
	
	GetFPos(prefsFileID, &gPrefsFilePos);
	gPrefsFilePos-=sizeof(thePrefs);
	do
	{
		gPrefsFilePos+=sizeof(thePrefs);
		err=GetNextPrefs(prefsFileID);
		if (err==prefs_noMorePrefsErr)
			return (Virgin(prefsFileID));
		
		if (err!=prefs_allsWell)
		{
			ClosePrefsFile(prefsFileID);
			return err;
		}
		
		err=CheckFileID();
	}
	while (err==prefs_IDNotMatchErr);
	
	CopyPrefsToGlobals();
	ClosePrefsFile(prefsFileID);
	
	return prefs_allsWell;
}

void PrefsError(int err)
{
	Str255			tempStr;
	
	switch (err)
	{
		case prefs_diskReadErr:
		case prefs_diskWriteErr:
		case prefs_cantCreatePrefsErr:
		case prefs_cantOpenPrefsErr:
		case prefs_versionNotSupportedErr:
			DefaultPrefs();
			gCanSavePrefs=FALSE;
			GetIndString(tempStr, 128, -err);
			ParamText(tempStr, "\p", "\p", "\p");
			PositionDialog('ALRT', prefsErrorAlert);
			StopAlert(prefsErrorAlert, 0L);
			break;
		default:
			gCanSavePrefs=TRUE;
			if (err!=prefs_virginErr)
				ShowSplashScreen();
			break;
	}
}

int OpenPrefsFile(int *prefsFileID)
{
	int				thisFile;
	OSErr			isHuman;
	int				vRefNum;
	long			dirID;
	FSSpec			prefsFile;
	FInfo			prefsInfo;
	Boolean			newPrefs;
	unsigned char	*name=PREFS_FILE_NAME;
	
	newPrefs=FALSE;
	isHuman=FindFolder(kOnSystemDisk, 'pref', kCreateFolder, &vRefNum, &dirID);
	
	if (isHuman!=noErr)
		return prefs_cantOpenPrefsErr;
	if (gHasFSSpecs)
	{
		isHuman=FSMakeFSSpec(vRefNum, dirID, name, &prefsFile);
		if (isHuman!=noErr)
		{
			if (isHuman==fnfErr)
			{
				isHuman=FSpCreate(&prefsFile, CREATOR, PREFS_TYPE, 0);
				if (isHuman!=noErr)
					return prefs_cantCreatePrefsErr;
				newPrefs=TRUE;
			}
			else return prefs_cantOpenPrefsErr;
		}
		isHuman=FSpOpenDF(&prefsFile, fsRdWrPerm, &thisFile);
		*prefsFileID=thisFile;
		if (isHuman!=noErr)
			return prefs_cantOpenPrefsErr;
	}
	else
	{
		isHuman=HOpen(vRefNum, dirID, name, fsRdWrPerm, &thisFile);
		*prefsFileID=thisFile;
		if (isHuman!=noErr)
		{
			if (isHuman==fnfErr)
			{
				isHuman=HCreate(vRefNum, dirID, name, CREATOR, PREFS_TYPE);
				if (isHuman!=noErr)
					return prefs_cantCreatePrefsErr;
				prefsInfo.fdType=PREFS_TYPE;
				prefsInfo.fdCreator=CREATOR;
				prefsInfo.fdFlags=0;
				prefsInfo.fdLocation.h=prefsInfo.fdLocation.v=0;
				prefsInfo.fdFldr=0;
				isHuman=HSetFInfo(vRefNum, dirID, name, &prefsInfo);
				if (isHuman!=noErr)
					return prefs_cantCreatePrefsErr;
				isHuman=HOpen(vRefNum, dirID, name, fsRdWrPerm, &thisFile);
				*prefsFileID=thisFile;
				if (isHuman!=noErr)
					return prefs_cantOpenPrefsErr;
				newPrefs=TRUE;
			}
			else return prefs_cantOpenPrefsErr;
		}
	}
	if (newPrefs)
		return SetupNewPrefsFile(*prefsFileID);
	
	return prefs_allsWell;
}

int SetupNewPrefsFile(int prefsFileID)
{
	int				err;
	OSErr			isHuman;
	long			count;
	int				temp;
	
	gPrefsFilePos=2L;
	isHuman=SetEOF(prefsFileID, 2L);
	if (isHuman!=noErr)
		return prefs_diskWriteErr;
	SetFPos(prefsFileID, 1, 0L);
	temp=PREFS_HEADER_VERSION;
	count=2L;
	isHuman=FSWrite(prefsFileID, &count, &temp);
	if (isHuman!=noErr)
		return prefs_diskWriteErr;		
	err=Virgin(prefsFileID);
	return err;
}

void ClosePrefsFile(int prefsFileID)
{
	FSClose(prefsFileID);
	FlushVol(0L, kOnSystemDisk);
}

int GetNextPrefs(int prefsFileID)
{
	OSErr		isHuman;
	long		count;
	
	count=sizeof(thePrefs);
	isHuman=FSRead(prefsFileID, &count, &thePrefs);
	if (isHuman==eofErr)
		return prefs_noMorePrefsErr;
	if (isHuman!=noErr)
		return prefs_diskReadErr;
	
	return prefs_allsWell;
}

int SavePrefs(int prefsFileID)
{
	long		oldEOF;
	OSErr		isHuman;
	long		count;
	
	GetEOF(prefsFileID, &oldEOF);
	if (gPrefsFilePos>=oldEOF)
	{
		isHuman=SetEOF(prefsFileID, oldEOF+sizeof(thePrefs));
		if (isHuman!=noErr)
			return prefs_diskWriteErr;
	}
	SetFPos(prefsFileID, 1, gPrefsFilePos);
	count=sizeof(thePrefs);
	isHuman=FSWrite(prefsFileID, &count, &thePrefs);
	if (isHuman!=noErr)
		return prefs_diskWriteErr;
	
	return prefs_allsWell;
}

int CheckVersion(int prefsFileID)
{
	OSErr		isHuman;
	long		count;
	int			temp;
	
	count=2L;
	isHuman=FSRead(prefsFileID, &count, &temp);
	if (isHuman!=noErr)
		return prefs_diskReadErr;
	if (temp>PREFS_HEADER_VERSION)
		return prefs_versionNotSupportedErr;
	if (temp<PREFS_HEADER_VERSION)
		return SetupNewPrefsFile(prefsFileID);
	
	return prefs_allsWell;
}

int GetFileID(void)
{
	CInfoPBRec		pb;
	int				err;
	
	pb.hFileInfo.ioCompletion=0L;
	pb.hFileInfo.ioNamePtr=CurApName;
	pb.hFileInfo.ioVRefNum=0;
	pb.hFileInfo.ioFDirIndex=0;
	pb.hFileInfo.ioDirID=0;
	err=PBGetCatInfo(&pb, FALSE);
	if (err!=noErr)
		return prefs_diskReadErr;
	
	gFileID=pb.hFileInfo.ioDirID;
	
	return prefs_allsWell;
}

int CheckFileID(void)
{
	return (thePrefs.fileID==gFileID) ? prefs_allsWell : prefs_IDNotMatchErr;
}

int Virgin(int prefsFileID)
{
	int			err;
	
	DefaultPrefs();
	CopyGlobalsToPrefs();
	err=SavePrefs(prefsFileID);
	if (err!=prefs_allsWell)
		return err;
	DoSound(sound_virgin);
	GetRegistration();
	CopyGlobalsToPrefs();
	err=SavePrefs(prefsFileID);
	
	return (err==prefs_allsWell) ? prefs_virginErr : err;
}

void DefaultPrefs(void)
{
	gMyName[0]=0x03;
	gMyName[1]='B';
	gMyName[2]='o';
	gMyName[3]='b';
	gMyOrg[0]=0x00;
	gSoundToggle=0xFF;
	gAllowDup=0x00;
	gNonWordsCount=0xFF;
	gAnimation=0xFF;
	gNumLetters=5;
	gLastWipe=0;
}

void CopyGlobalsToPrefs(void)
{
	Mymemset(&thePrefs, 0, sizeof(thePrefs));
	if (gMyName[0]>0x27)
		gMyName[0]=0x27;
	if (gMyOrg[0]>0x27)
		gMyOrg[0]=0x27;
	Mymemcpy(thePrefs.regname, gMyName, gMyName[0]+1);
	Mymemcpy(thePrefs.regorg, gMyOrg, gMyOrg[0]+1);
	thePrefs.sound=gSoundToggle;
	thePrefs.dup=gAllowDup;
	thePrefs.nonwords=gNonWordsCount;
	thePrefs.animation=gAnimation;
	thePrefs.numletters=gNumLetters;
	thePrefs.unused=0x00;
	thePrefs.lastwipe=gLastWipe;
	thePrefs.fileID=gFileID;
}

void CopyPrefsToGlobals(void)
{
	Mymemcpy(gMyName, thePrefs.regname, thePrefs.regname[0]+1);
	Mymemcpy(gMyOrg, thePrefs.regorg, thePrefs.regorg[0]+1);
	gSoundToggle=thePrefs.sound;
	gAllowDup=thePrefs.dup;
	gNonWordsCount=thePrefs.nonwords;
	gAnimation=thePrefs.animation;
	gNumLetters=thePrefs.numletters;
	if ((gNumLetters<5) || (gNumLetters>6))
		gNumLetters=5;
	gLastWipe=thePrefs.lastwipe;
}

void GetRegistration(void)
{
	DialogPtr		theDlog;
	int				itemSelected = 0;
	int				newleft;
	int				newtop;
	int				itemType;
	Handle			item;
	Rect			box;
	int				dlogID;
	
	if(GetWindowDepth() == 1)
		dlogID = bwPersonalDialog;
	else
		dlogID = colorPersonalDialog;
		
	theDlog = GetNewDialog(dlogID, 0L, (WindowPtr)-1L);
	newleft = gMainScreenBounds.left + (((gMainScreenBounds.right -
				gMainScreenBounds.left) - (theDlog->portRect.right -
				theDlog->portRect.left)) / 2);
	newtop = gMainScreenBounds.top + (((gMainScreenBounds.bottom -
				gMainScreenBounds.top) - (theDlog->portRect.bottom -
				theDlog->portRect.top)) / 2);
	if(newtop < 15)
		newtop = 15;
	GetDItem(theDlog, 1, &itemType, &item, &box);
	InsetRect(&box, -4, -4);
	SetDItem(theDlog, 8, userItem, OutlineDefaultButton, &box);

	MoveWindow(theDlog, newleft, newtop, TRUE);
	ShowWindow(theDlog);
	while(itemSelected != 1)
	{
		ModalDialog(0L, &itemSelected);
	}
	GetDItem(theDlog,4,&itemType,&item,&box);
	GetIText(item,&gMyName);
	
	GetDItem(theDlog,5,&itemType,&item,&box);
	GetIText(item,&gMyOrg);

	if (gMyName[0]=='\0')
		DefaultPrefs();

	HideWindow(theDlog);
	DisposeDialog(theDlog);
}

void SaveThePrefs(void)
{
	int			prefsFileID;
	
	if (gCanSavePrefs)
	{
		OpenPrefsFile(&prefsFileID);
		CopyGlobalsToPrefs();
		SavePrefs(prefsFileID);
		ClosePrefsFile(prefsFileID);
	}
}
