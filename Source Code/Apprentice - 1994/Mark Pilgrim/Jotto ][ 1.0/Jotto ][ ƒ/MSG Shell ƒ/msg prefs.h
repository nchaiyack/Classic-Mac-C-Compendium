/**********************************************************************\

File:		msg prefs.h

Purpose:	This is the header file for msg prefs.c


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

#include "Folders.h"

#define		PREFS_FILE_NAME			"\pJotto ][ Prefs"
#define		PREFS_TYPE				'PREF'
// CREATOR is #defined in "jotto globals.h"
#define		PREFS_HEADER_VERSION	1

extern	Boolean		gCanSavePrefs;
extern	Str255		gMyName;
extern	Str255		gMyOrg;

typedef struct
{
	char			regname[40];
	char			regorg[40];
	unsigned char	sound;
	unsigned char	dup;
	unsigned char	nonwords;
	unsigned char	animation;
	unsigned char	numletters;
	unsigned char	unused;
	int				lastwipe;
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
	prefs_allsWell=0,
	prefs_diskReadErr=-1,
	prefs_diskWriteErr=-2,
	prefs_cantOpenPrefsErr=-3,
	prefs_cantCreatePrefsErr=-4,
	prefs_noMorePrefsErr=-5,
	prefs_versionNotSupportedErr=-6,
	prefs_virginErr=-7,
	prefs_IDNotMatchErr=-8
};
