/**********************************************************************\

File:		prefs.c

Purpose:	This module handles getting stuff from the prefs file,
			like the current module name, and then saving other stuff,
			like where the current module is installed in memory.


Go Sit In The Corner -=- not you, just the cursor
Copyright �1994, Mark Pilgrim

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

#include "structs.h"
#include "prefs.h"
#include "GestaltEQU.h"
#include "Folders.h"

int PreferencesInit(PrefHandle prefsHandle)
{
	int			prefsFileID;
	int			err;
	
	err=OpenPrefsFile(&prefsFileID, prefsHandle);
	if (err!=prefs_allsWell)
	{
		if ((err==prefs_diskReadErr) || (err==prefs_diskWriteErr) || (err==prefs_virginErr))
			ClosePrefsFile(prefsFileID);
		return err;
	}
	
	GetNextPrefs(prefsFileID, prefsHandle);
	if (err!=prefs_allsWell)
	{
		ClosePrefsFile(prefsFileID);
		return err;
	}
	
	ClosePrefsFile(prefsFileID);
	
	return prefs_allsWell;
}

int OpenPrefsFile(int *prefsFileID, PrefHandle prefsHandle)
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
	long			gestalt_temp;
	Boolean			gHasFSSpecs;
	
	isHuman = Gestalt(gestaltFSAttr, &gestalt_temp);
	gHasFSSpecs=((isHuman==noErr) && (gestalt_temp & (1 << gestaltHasFSSpecCalls)));
	
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
		return Virgin(*prefsFileID, prefsHandle);
	}
	
	return prefs_allsWell;
}

void ClosePrefsFile(int prefsFileID)
{
	FSClose(prefsFileID);
	FlushVol(0L, kOnSystemDisk);
}

int GetNextPrefs(int prefsFileID, PrefHandle prefsHandle)
{
	OSErr		isHuman;
	long		count;
	
	count=GetHandleSize(prefsHandle);
	isHuman=FSRead(prefsFileID, &count, *prefsHandle);
	if (isHuman!=noErr)
		return prefs_diskReadErr;
	
	return prefs_allsWell;
}

int SavePrefs(int prefsFileID, PrefHandle prefsHandle)
{
	long		oldEOF;
	OSErr		isHuman;
	long		count;
	
	isHuman=SetEOF(prefsFileID, GetHandleSize(prefsHandle));
	if (isHuman!=noErr)
		return prefs_diskWriteErr;

	SetFPos(prefsFileID, 1, 0L);
	count=GetHandleSize(prefsHandle);
	isHuman=FSWrite(prefsFileID, &count, *prefsHandle);
	if (isHuman!=noErr)
		return prefs_diskWriteErr;
	
	return prefs_allsWell;
}

int Virgin(int prefsFileID, PrefHandle prefsHandle)
{
	int			err;
	
	DefaultPrefs(prefsHandle);
	err=SavePrefs(prefsFileID, prefsHandle);
	
	return (err==prefs_allsWell) ? prefs_virginErr : err;
}

void DefaultPrefs(PrefHandle prefsHandle)
{
	int				i;
	
	(**prefsHandle).showIcon=0xFF;
	(**prefsHandle).isOn=0xFF;
	(**prefsHandle).ourCodePtr=0L;
	(**prefsHandle).ourVBLPtr=0L;
	(**prefsHandle).whichCorner=0x00;	/* top-left corner */
	(**prefsHandle).always=0x00;		/* only if no mouse movement */
}

void SaveThePrefs(PrefHandle prefsHandle)
{
	int				prefsFileID;
	
	OpenPrefsFile(&prefsFileID, prefsHandle);
	SavePrefs(prefsFileID, prefsHandle);
	ClosePrefsFile(prefsFileID);
}
