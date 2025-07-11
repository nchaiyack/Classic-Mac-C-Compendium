/**********************************************************************\

File:		msg prefs.c

Purpose:	This module handles creating/opening/closing/updating
			the preference file, and copying the preference file
			data into application globals (and back).


Devil�s Cubes -- a simple cubes puzzle
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
#include "msg sounds.h"
#include "msg graphics.h"
#include "msg environment.h"
#include "util.h"
#include "cube files.h"
#include "cube.h"

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
	if (err!=allsWell)
		return err;
	
	err=OpenPrefsFile(&prefsFileID);
	if (err!=allsWell)
	{
		if ((err==diskReadErr) || (err==diskWriteErr) || (err==virginErr))
			ClosePrefsFile(prefsFileID);
		return err;
	}
	
	err=CheckVersion(prefsFileID);
	if (err!=allsWell)
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
		if (err==noMorePrefsErr)
			return (Virgin(prefsFileID));
		
		if (err!=allsWell)
		{
			ClosePrefsFile(prefsFileID);
			return err;
		}
		
		err=CheckFileID();
	}
	while (err==IDNotMatchErr);
	
	CopyPrefsToGlobals();
	ClosePrefsFile(prefsFileID);
	
	return allsWell;
}

void PrefsError(int err)
{
	switch (err)
	{
		case diskReadErr:
			ParamText("\pThe Devil�s Cubes preferences file could not be accessed, because a \
disk read error occurred.  Default preferences will be used for now, but the file may \
be damaged, and you should probably throw it in the trash.", "\p", "\p", "\p");
			break;
		case diskWriteErr:
			ParamText("\pThe Devil�s Cubes preferences file could not be accessed, because a \
disk write error occurred.  The file may be damaged, and you should probably throw it \
in the trash.","\p","\p","\p");
			break;
		case cantCreatePrefsErr:
			ParamText("\pThe Devil�s Cubes preferences file could not be created.  The \
disk may be locked or full.  Default preferences will be used for now.","\p","\p","\p");
			break;
		case cantOpenPrefsErr:
			ParamText("\pThe Devil�s Cubes preferences file could not be accessed.  The \
file may be damaged, and you should probably throw it in the trash.  Default preferences \
will be used for now.","\p","\p","\p");
			break;
		case versionNotSupportedErr:
			ParamText("\pThe preferences file you have in your system folder was created \
by a later version of Devil�s Cubes than this copy.  If you want to be able to save your \
preferences with this copy, you will have to throw the current preferences file in the \
trash.","\p","\p","\p");
			break;
	}
	switch (err)
	{
		case diskReadErr:
		case diskWriteErr:
		case cantCreatePrefsErr:
		case cantOpenPrefsErr:
		case versionNotSupportedErr:
			gCanSavePrefs=FALSE;
			PositionDialog('ALRT', prefsErrorAlert);
			StopAlert(prefsErrorAlert, 0L);
			break;
		default:
		{
			gCanSavePrefs=TRUE;
			if (err!=virginErr)
				ShowSplashScreen();
		}
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
		return cantOpenPrefsErr;
	if (gHasFSSpecs)
	{
		isHuman=FSMakeFSSpec(vRefNum, dirID, name, &prefsFile);
		if (isHuman!=noErr)
		{
			if (isHuman==fnfErr)
			{
				isHuman=FSpCreate(&prefsFile, CREATOR, PREFS_TYPE, 0);
				if (isHuman!=noErr)
					return cantCreatePrefsErr;
				newPrefs=TRUE;
			}
			else return cantOpenPrefsErr;
		}
		isHuman=FSpOpenDF(&prefsFile, fsRdWrPerm, &thisFile);
		*prefsFileID=thisFile;
		if (isHuman!=noErr)
			return cantOpenPrefsErr;
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
					return cantCreatePrefsErr;
				prefsInfo.fdType=PREFS_TYPE;
				prefsInfo.fdCreator=CREATOR;
				prefsInfo.fdFlags=0;
				prefsInfo.fdLocation.h=prefsInfo.fdLocation.v=0;
				prefsInfo.fdFldr=0;
				isHuman=HSetFInfo(vRefNum, dirID, name, &prefsInfo);
				if (isHuman!=noErr)
					return cantCreatePrefsErr;
				isHuman=HOpen(vRefNum, dirID, name, fsRdWrPerm, &thisFile);
				*prefsFileID=thisFile;
				if (isHuman!=noErr)
					return cantOpenPrefsErr;
				newPrefs=TRUE;
			}
			else return cantOpenPrefsErr;
		}
	}
	if (newPrefs)
		return SetupNewPrefsFile(*prefsFileID);
	
	return allsWell;
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
		return diskWriteErr;
	SetFPos(prefsFileID, 1, 0L);
	temp=PREFS_HEADER_VERSION;
	count=2L;
	isHuman=FSWrite(prefsFileID, &count, &temp);
	if (isHuman!=noErr)
		return diskWriteErr;		
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
		return noMorePrefsErr;
	if (isHuman!=noErr)
		return diskReadErr;
	
	return allsWell;
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
			return diskWriteErr;
	}
	SetFPos(prefsFileID, 1, gPrefsFilePos);
	count=sizeof(thePrefs);
	isHuman=FSWrite(prefsFileID, &count, &thePrefs);
	if (isHuman!=noErr)
		return diskWriteErr;
	
	return allsWell;
}

int CheckVersion(int prefsFileID)
{
	OSErr		isHuman;
	long		count;
	int			temp;
	
	count=2L;
	isHuman=FSRead(prefsFileID, &count, &temp);
	if (isHuman!=noErr)
		return diskReadErr;
	if (temp>PREFS_HEADER_VERSION)
		return versionNotSupportedErr;
	if (temp<PREFS_HEADER_VERSION)
		return SetupNewPrefsFile(prefsFileID);
	
	return allsWell;
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
		return diskReadErr;
	
	gFileID=pb.hFileInfo.ioDirID;
	
	return allsWell;
}

int CheckFileID(void)
{
	return (thePrefs.fileID==gFileID) ? allsWell : IDNotMatchErr;
}

int Virgin(int prefsFileID)
{
	int			err;
	
	DefaultPrefs();
	CopyGlobalsToPrefs();
	err=SavePrefs(prefsFileID);
	if (err!=allsWell)
		return err;
	DoSound(sound_virgin);
	GetRegistration();
	CopyGlobalsToPrefs();
	err=SavePrefs(prefsFileID);
	
	return (err==allsWell) ? virginErr : err;
}

void DefaultPrefs(void)
{
	gMyName[0]=0x03;
	gMyName[1]='B';
	gMyName[2]='o';
	gMyName[3]='b';
	gMyOrg[0]=0x00;
	gSoundToggle=0xFF;
	gUseMirror=0xFF;
	gShowAll=0xFF;
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
	thePrefs.usemirror=gUseMirror;
	thePrefs.showall=gShowAll;
	thePrefs.unused=0x00;
	thePrefs.fileID=gFileID;
}

void CopyPrefsToGlobals(void)
{
	Mymemcpy(gMyName, thePrefs.regname, thePrefs.regname[0]+1);
	Mymemcpy(gMyOrg, thePrefs.regorg, thePrefs.regorg[0]+1);
	gSoundToggle=thePrefs.sound;
	gUseMirror=thePrefs.usemirror;
	gShowAll=thePrefs.showall;
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
