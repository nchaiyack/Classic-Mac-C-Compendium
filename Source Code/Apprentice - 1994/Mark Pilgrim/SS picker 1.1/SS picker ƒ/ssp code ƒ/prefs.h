/**********************************************************************\

File:		prefs.c

Purpose:	This is the header file for prefs.c


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

#include "Folders.h"

#define		PREFS_FILE_NAME			"\pStartupScreen Picker Prefs"
#define		PREFS_TYPE				'PREF'
#define		CREATOR					'SsPk'

typedef struct
{
	unsigned char		name[32];
} PrefStruct;

int PreferencesInit(void);
int OpenPrefsFile(int *prefsFileID);
void ClosePrefsFile(int prefsFileID);
int GetNextPrefs(int prefsFileID);
int SavePrefs(int prefsFileID);
void DefaultPrefs(void);
void CopyGlobalsToPrefs(void);
void CopyPrefsToGlobals(void);
void SaveThePrefs(void);

enum
{
	prefs_allsWell=0,
	prefs_diskReadErr,
	prefs_diskWriteErr,
	prefs_cantOpenPrefsErr,
	prefs_cantCreatePrefsErr,
	prefs_virginErr
};
