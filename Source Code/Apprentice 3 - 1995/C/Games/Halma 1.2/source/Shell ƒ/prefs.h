/**********************************************************************\

File:		prefs.h

Purpose:	This is the header file for prefs.c

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

extern	Str255			gMyName;
extern	Str255			gMyOrg;

enum PrefErrorTypes		/* possible error codes */
{
	prefs_allsWell=0,				/* no error */
	prefs_diskReadErr,				/* error reading prefs file */
	prefs_diskWriteErr,				/* error writing prefs file */
	prefs_cantOpenPrefsErr,			/* error trying to open prefs file */
	prefs_cantCreatePrefsErr,		/* error trying to create new prefs file */
	prefs_noMorePrefsErr,			/* no more prefs found in prefs file */
	prefs_versionNotSupportedErr,	/* prefs file created by later version of program */
	prefs_virginErr,				/* prefs file didn't exist -- first time running */
	prefs_IDNotMatchErr				/* file ID in current prefs doesn't match program's ID */
};

void SaveThePrefs(void);
enum PrefErrorTypes PreferencesInit(void);
void PrefsError(enum PrefErrorTypes err);
