/**********************************************************************\

File:		msg prefs.h

Purpose:	This is the header file for msg prefs.c


Pentominoes - a 2-D geometry board game
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

#include "Folders.h"

#define		PREFS_FILE_NAME			"\pPentominoes prefs"
#define		PREFS_TYPE				'PREF'
#define		CREATOR					'PNT5'
#define		PREFS_HEADER_VERSION	2

extern	Boolean		gCanSavePrefs;
extern	Str255		gMyName;
extern	Str255		gMyOrg;

typedef struct
{
	char			regname[40];
	char			regorg[40];
	short			sound;
	short			whichboard;
	long			fileID;
} PrefStruct;

int PreferencesInit(void);
void PrefsError(int err);
int OpenPrefsFile(int *prefsFileID);
int SetupNewPrefsFile(int prefsFileID);
void ClosePrefsFile(int prefsFileID);
int GetNextPrefs(int prefsFileID);
int SavePrefs(int prefsFileID);
int CheckVersion(int prefsFileID);
int GetFileID(void);
int CheckFileID(void);
int Virgin(int prefsFileID);
void DefaultPrefs(void);
void CopyGlobalsToPrefs(void);
void CopyPrefsToGlobals(void);
void GetRegistration(void);
void SaveThePrefs(void);

enum
{
	allsWell=0,
	diskReadErr=-1,
	diskWriteErr=-2,
	cantOpenPrefsErr=-3,
	cantCreatePrefsErr=-4,
	noMorePrefsErr=-5,
	versionNotSupportedErr=-6,
	virginErr=-7,
	IDNotMatchErr=-8
};
