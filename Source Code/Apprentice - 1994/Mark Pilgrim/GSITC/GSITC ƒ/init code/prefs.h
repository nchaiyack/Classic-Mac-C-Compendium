/**********************************************************************\

File:		prefs.h

Purpose:	This is the header file for prefs.c


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

#define		PREFS_FILE_NAME			"\pGo Sit In The Corner prefs"
#define		PREFS_TYPE				'PREF'
#define		CREATOR					'GsIc'

int PreferencesInit(PrefHandle prefsHandle);
int OpenPrefsFile(int *prefsFileID, PrefHandle prefsHandle);
void ClosePrefsFile(int prefsFileID);
int GetNextPrefs(int prefsFileID, PrefHandle prefsHandle);
int SavePrefs(int prefsFileID, PrefHandle prefsHandle);
int Virgin(int prefsFileID, PrefHandle prefsHandle);
void DefaultPrefs(PrefHandle prefsHandle);
void SaveThePrefs(PrefHandle prefsHandle);
