/**********************************************************************\

File:		prefs.c

Purpose:	This module handles the preferences file, which contains
			the real filename of the current startup screen.
			

Startup Screen Picker -=- pick a random startup screen
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

#include "prefs.h"
#include "init.h"
#include "environment.h"
#include "main.h"

static PrefStruct	thePrefs;
static long			gPrefsFilePos;

int PreferencesInit(void)
{
	int			prefsFileID;
	int			err;
	
	err=OpenPrefsFile(&prefsFileID);
	if (err!=prefs_allsWell)
	{
		if ((err==prefs_diskReadErr) || (err==prefs_diskWriteErr) || (err==prefs_virginErr))
			ClosePrefsFile(prefsFileID);
		return err;
	}
	
	GetNextPrefs(prefsFileID);
	if (err!=prefs_allsWell)
	{
		ClosePrefsFile(prefsFileID);
		return err;
	}
	
	CopyPrefsToGlobals();
	ClosePrefsFile(prefsFileID);
	
	return prefs_allsWell;
}

int OpenPrefsFile(int *prefsFileID)
{
	int				thisFile;
	OSErr			isHuman;
	int				vRefNum;
	long			dirID;
	FSSpec			prefsFile;
	FInfo			prefsInfo;
	int				temp;
	long			count;
	Boolean			newPrefs;
	int				err;
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
	{
		return Virgin(*prefsFileID);
	}
	
	return prefs_allsWell;
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
	if (isHuman!=noErr)
		return prefs_diskReadErr;
	
	return prefs_allsWell;
}

int SavePrefs(int prefsFileID)
{
	long		oldEOF;
	OSErr		isHuman;
	long		count;
	
	isHuman=SetEOF(prefsFileID, sizeof(thePrefs));
	if (isHuman!=noErr)
		return prefs_diskWriteErr;

	SetFPos(prefsFileID, 1, 0L);
	count=sizeof(thePrefs);
	isHuman=FSWrite(prefsFileID, &count, &thePrefs);
	if (isHuman!=noErr)
		return prefs_diskWriteErr;
	
	return prefs_allsWell;
}

int Virgin(int prefsFileID)
{
	int			err;
	
	DefaultPrefs();
	CopyGlobalsToPrefs();
	err=SavePrefs(prefsFileID);
	
	return (err==prefs_allsWell) ? prefs_virginErr : err;
}

void DefaultPrefs(void)
{
	unsigned char	*temp="\pOld StartupScreen";
	int				i;
	
	for (i=temp[0]; i>=0; i--)
		gLastName[i]=temp[i];
}

void CopyGlobalsToPrefs(void)
{
	int				i;
	
	for (i=0; i<32; i++)
		thePrefs.name[i]=0x00;
	for (i=gLastName[0]; i>=0; i--)
		thePrefs.name[i]=gLastName[i];
}

void CopyPrefsToGlobals(void)
{
	int				i;
	
	for (i=thePrefs.name[0]; i>=0; i--)
		gLastName[i]=thePrefs.name[i];
}

void SaveThePrefs(void)
{
	int			prefsFileID;
	
	OpenPrefsFile(&prefsFileID);
	CopyGlobalsToPrefs();
	SavePrefs(prefsFileID);
	ClosePrefsFile(prefsFileID);
}
