/**********************************************************************\

File:		prefs.c

Purpose:	This module handles creating/opening/closing/updating
			the preference file, and copying the preference file
			data into our globals (and back).


Shutdown FX -=- graphic effects on shutdown
Copyright (C) 1993-4, Mark Pilgrim & Dave Blumenthal

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

#include "init.h"
#include "prefs.h"
#include "main.h"
#include "Folders.h"

#define		PREFS_FILE_NAME			"\pShutdown FX prefs"
#define		PREFS_TYPE				'pref'
#define		CREATOR					'¤fx¤'
#define		PREFS_HEADER_VERSION	1

typedef struct
{
	unsigned char	onrestart;
	unsigned char	onshutdown;
	unsigned char	sequential;
	unsigned char	isvirgin;
	int				whichwipe;
} PrefStruct;

Boolean			gHasFSSpecs;

/* internal globals for use in prefs.c only */
Boolean			gCanSavePrefs;
PrefStruct		thePrefs;
long			gPrefsFilePos;

/*-----------------------------------------------------------------------------------*/
/* internal stuff for prefs.c                                                        */

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

void SaveThePrefs(void)
/* standard procedure callable from anywhere to save prefs to disk (if possible) */
{
	int			prefsFileID;
	
	if (gCanSavePrefs)		/* if we had no errors in PreferencesInit() */
	{
		OpenPrefsFile(&prefsFileID);	/* open the prefs file */
		CopyGlobalsToPrefs();			/* copy global variables to prefs struct */
		SavePrefs(prefsFileID);			/* save prefs to disk */
		ClosePrefsFile(prefsFileID);	/* close prefs file */
	}
}

int PreferencesInit(void)
{
	int				prefsFileID;
	int				err;
	
	gCanSavePrefs=FALSE;	/* assume the worst and maybe you'll be pleasantly surprised */
	err=OpenPrefsFile(&prefsFileID);	/* open prefs file (or create new one) */
	if (err!=prefs_allsWell)
	{
		if ((err==prefs_diskReadErr) || (err==prefs_diskWriteErr) || (err==prefs_virginErr))
			ClosePrefsFile(prefsFileID);	/* close & abort if error or if new prefs */
		return err;
	}
	
	err=CheckVersion(prefsFileID);		/* check prefs version */
	if (err!=prefs_allsWell)
	{
		ClosePrefsFile(prefsFileID);
		return err;
	}
	
	GetFPos(prefsFileID, &gPrefsFilePos);
	err=GetNextPrefs(prefsFileID);		/* get prefs struct from file */
	if (err==prefs_noMorePrefsErr)		/* or not */
		return (Virgin(prefsFileID));	/* can't find our prefs struct */
		
	if (err!=prefs_allsWell)			/* any other error, just abort */
	{
		ClosePrefsFile(prefsFileID);
		return err;
	}
	
	CopyPrefsToGlobals();				/* copy prefs struct to program globals */
	ClosePrefsFile(prefsFileID);		/* close prefs file */
	gCanSavePrefs=TRUE;
	return prefs_allsWell;				/* piece o' cake */
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
	/* find vRefNum and dirID of preferences folder, creating it if necessary */
	isHuman=FindFolder(kOnSystemDisk, 'pref', kCreateFolder, &vRefNum, &dirID);
	
	if (isHuman!=noErr)		/* screwed up already?!? */
		return prefs_cantOpenPrefsErr;
	if (gHasFSSpecs)
	{
		isHuman=FSMakeFSSpec(vRefNum, dirID, name, &prefsFile);	/* make FSSpec out of it */
		if (isHuman!=noErr)
		{
			if (isHuman==fnfErr)	/* FSSpec is valid, but prefs file does not exist */
			{
				isHuman=FSpCreate(&prefsFile, CREATOR, PREFS_TYPE, 0);	/* so create it */
				if (isHuman!=noErr)										/* or not */
					return prefs_cantCreatePrefsErr;
				newPrefs=TRUE;		/* signal that prefs file is new */
			}
			else return prefs_cantOpenPrefsErr;
		}
		isHuman=FSpOpenDF(&prefsFile, fsRdWrPerm, &thisFile);	/* open prefs file */
		*prefsFileID=thisFile;		/* store file reference number */
		if (isHuman!=noErr)
			return prefs_cantOpenPrefsErr;
	}
	else
	{
		/* try to open prefs file */
		isHuman=HOpen(vRefNum, dirID, name, fsRdWrPerm, &thisFile);
		*prefsFileID=thisFile;
		if (isHuman!=noErr)
		{
			if (isHuman==fnfErr)	/* prefs file does not exist */
			{
				/* ...so create it */
				if (HCreate(vRefNum, dirID, name, CREATOR, PREFS_TYPE)!=noErr)
					return prefs_cantCreatePrefsErr;
				prefsInfo.fdType=PREFS_TYPE;
				prefsInfo.fdCreator=CREATOR;
				prefsInfo.fdFlags=0;
				prefsInfo.fdLocation.h=prefsInfo.fdLocation.v=0;
				prefsInfo.fdFldr=0;
				
				/* set file info of newly created prefs file */
				if (HSetFInfo(vRefNum, dirID, name, &prefsInfo)!=noErr)
					return prefs_cantCreatePrefsErr;
				
				/* NOW open the prefs file */
				isHuman=HOpen(vRefNum, dirID, name, fsRdWrPerm, &thisFile);
				*prefsFileID=thisFile;		/* store file reference number */
				if (isHuman!=noErr)
					return prefs_cantOpenPrefsErr;
				newPrefs=TRUE;				/* signal that prefs file is new */
			}
			else return prefs_cantOpenPrefsErr;
		}
	}
	if (newPrefs)
		return SetupNewPrefsFile(*prefsFileID);		/* needs initial setup if new */
	
	return prefs_allsWell;
}

int SetupNewPrefsFile(int prefsFileID)
/* this writes the prefs version number to the newly created prefs file, so we can
   tell if the prefs file was created by a later version of the program and is
   therefore in a format that we don't support -- forward compatability!  what
   a concept! */
{
	long			count;
	int				temp;
	
	gPrefsFilePos=2L;
	if (SetEOF(prefsFileID, 2L)!=noErr)	/* set length of prefs file to 2 */
		return prefs_diskWriteErr;
	
	SetFPos(prefsFileID, 1, 0L);
	temp=PREFS_HEADER_VERSION;			/* get the prefs version (hardcoded) */
	count=2L;
	if (FSWrite(prefsFileID, &count, &temp)!=noErr)		/* write prefs version */
		return prefs_diskWriteErr;		
	
	return Virgin(prefsFileID);			/* be gentle; it's our first time */
}

void ClosePrefsFile(int prefsFileID)
{
	FSClose(prefsFileID);				/* close file on disk */
	FlushVol(0L, kOnSystemDisk);		/* flush volume to write out new info */
}

int GetNextPrefs(int prefsFileID)
{
	OSErr		isHuman;
	long		count;
	
	count=sizeof(thePrefs);
	isHuman=FSRead(prefsFileID, &count, &thePrefs);		/* get next prefs struct */
	if (isHuman==eofErr)	/* no more left */
		return prefs_noMorePrefsErr;
	if (isHuman!=noErr)		/* some other error */
		return prefs_diskReadErr;
	
	return prefs_allsWell;
}

int SavePrefs(int prefsFileID)
{
	long		oldEOF;
	long		count;
	
	GetEOF(prefsFileID, &oldEOF);
	if (gPrefsFilePos>=oldEOF)		/* add new prefs struct onto end of prefs file */
	{
		if (SetEOF(prefsFileID, oldEOF+sizeof(thePrefs))!=noErr)
			return prefs_diskWriteErr;
	}
	
	SetFPos(prefsFileID, 1, gPrefsFilePos);		/* set position inside prefs file */
	count=sizeof(thePrefs);
	/* write prefs struct and return appropriate error code */
	return (FSWrite(prefsFileID, &count, &thePrefs)!=noErr) ?
		prefs_diskWriteErr : prefs_allsWell;
}

int CheckVersion(int prefsFileID)
{
	OSErr		isHuman;
	long		count;
	int			temp;
	
	count=2L;
	isHuman=FSRead(prefsFileID, &count, &temp);		/* get prefs version */
	if (isHuman!=noErr)
		return prefs_diskReadErr;
	if (temp>PREFS_HEADER_VERSION)					/* too new */
		return prefs_versionNotSupportedErr;
	if (temp<PREFS_HEADER_VERSION)					/* old; overwrite */
		return SetupNewPrefsFile(prefsFileID);
	
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
	gIsVirgin=gOnShutdown=0xFF;
	gSequential=gOnRestart=0x00;
	gWhichWipe=0;
}

void CopyGlobalsToPrefs(void)
{
	thePrefs.onrestart=gOnRestart;
	thePrefs.onshutdown=gOnShutdown;
	thePrefs.sequential=gSequential;
	thePrefs.whichwipe=gWhichWipe;
	thePrefs.isvirgin=gIsVirgin;
}

void CopyPrefsToGlobals(void)
{
	gOnRestart=thePrefs.onrestart;
	gOnShutdown=thePrefs.onshutdown;
	gSequential=thePrefs.sequential;
	gWhichWipe=thePrefs.whichwipe;
	gIsVirgin=thePrefs.isvirgin;
}
