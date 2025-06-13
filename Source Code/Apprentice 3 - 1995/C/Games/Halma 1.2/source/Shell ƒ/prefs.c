/**********************************************************************\

File:		prefs.c

Purpose:	This module handles creating/opening/closing/updating
			the preference file, and copying the preference file
			data into application globals (and back).

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

#include "program globals.h"
#include "prefs.h"
#include "help.h"
#include "dialogs.h"
#include "environment.h"
#include "util.h"
#include "sounds.h"
#include "Folders.h"

#define		PREFS_FILE_NAME			"\pHalma prefs"
#define		PREFS_TYPE				'PREF'
// CREATOR is #defined in "program globals.h"
#define		PREFS_HEADER_VERSION	2

Boolean			gCanSavePrefs;
Str255			gMyName;
Str255			gMyOrg;

typedef struct
{
	char			regname[40];
	char			regorg[40];
	unsigned char	sound;
	unsigned char	doesntmutate;
	unsigned char	snowcolor;
	unsigned char	snowspeed;
	short			numrows;
	short			numcolumns;
	short			maintopic;
	short			subtopic;
	long			fileID;
} PrefStruct;

/* internal globals for use in prefs.c only */
static long			gFileID;
static Boolean		gCanSavePrefs;
static PrefStruct	thePrefs;
static long			gPrefsFilePos;

/*-----------------------------------------------------------------------------------*/
/* internal stuff for prefs.c                                                        */

enum PrefErrorTypes OpenPrefsFile(short *prefsFileID);
enum PrefErrorTypes SetupNewPrefsFile(short prefsFileID);
void ClosePrefsFile(short prefsFileID);
enum PrefErrorTypes GetNextPrefs(short prefsFileID);
enum PrefErrorTypes SavePrefs(short prefsFileID);
enum PrefErrorTypes CheckVersion(short prefsFileID);
enum PrefErrorTypes GetFileID(void);
enum PrefErrorTypes CheckFileID(void);
enum PrefErrorTypes Virgin(short prefsFileID);
void DefaultPrefs(void);
void CopyGlobalsToPrefs(void);
void CopyPrefsToGlobals(void);
void GetRegistration(void);

void SaveThePrefs(void)
/* standard procedure callable from anywhere to save prefs to disk (if possible) */
{
	short			prefsFileID;
	
	if (gCanSavePrefs)		/* if we had no errors in PreferencesInit() */
	{
		OpenPrefsFile(&prefsFileID);	/* open the prefs file */
		CopyGlobalsToPrefs();			/* copy global variables to prefs struct */
		SavePrefs(prefsFileID);			/* save prefs to disk */
		ClosePrefsFile(prefsFileID);	/* close prefs file */
	}
}

enum PrefErrorTypes PreferencesInit(void)
{
	short			prefsFileID;
	enum PrefErrorTypes		err;
	
	gCanSavePrefs=FALSE;	/* assume the worst and maybe you'll be pleasantly surprised */
	err=GetFileID();		/* get application file ID */
	if (err!=prefs_allsWell)	/* screwed up already?!? */
		return err;
	
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
	gPrefsFilePos-=sizeof(thePrefs);
	do
	{
		gPrefsFilePos+=sizeof(thePrefs);
		err=GetNextPrefs(prefsFileID);		/* get prefs struct from file */
		if (err==prefs_noMorePrefsErr)		/* or not */
			return (Virgin(prefsFileID));	/* can't find our file ID, it's our first time */
		
		if (err!=prefs_allsWell)			/* any other error, just abort */
		{
			ClosePrefsFile(prefsFileID);
			return err;
		}
		
		err=CheckFileID();					/* check file ID of current prefs struct */
	}
	while (err==prefs_IDNotMatchErr);
	
	CopyPrefsToGlobals();					/* copy prefs struct to program globals */
	ClosePrefsFile(prefsFileID);			/* close prefs file */
	
	return prefs_allsWell;					/* piece o' cake */
}

void PrefsError(enum PrefErrorTypes err)
{
	Str255			tempStr;
	
	switch (err)
	{
		case prefs_diskReadErr:
		case prefs_diskWriteErr:
		case prefs_cantCreatePrefsErr:
		case prefs_cantOpenPrefsErr:
		case prefs_versionNotSupportedErr:
			DefaultPrefs();					/* use default prefs if error */
			gCanSavePrefs=FALSE;			/* don't bother trying to save prefs later */
			GetIndString(tempStr, 128, err);	/* get error string from .rsrc file */
			ParamText(tempStr, "\p", "\p", "\p");
			PositionDialog('ALRT', largeAlert);
			StopAlert(largeAlert, 0L);		/* display error alert */
			break;
		default:
			gCanSavePrefs=TRUE;				/* can save prefs to disk later if needed */
			break;
	}
}

enum PrefErrorTypes OpenPrefsFile(short *prefsFileID)
{
	short			thisFile;
	OSErr			isHuman;
	short			vRefNum;
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

enum PrefErrorTypes SetupNewPrefsFile(short prefsFileID)
/* this writes the prefs version number to the newly created prefs file, so we can
   tell if the prefs file was created by a later version of the program and is
   therefore in a format that we don't support -- forward compatability!  what
   a concept! */
{
	long			count;
	short			temp;
	
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

void ClosePrefsFile(short prefsFileID)
{
	FSClose(prefsFileID);				/* close file on disk */
	FlushVol(0L, kOnSystemDisk);		/* flush volume to write out new info */
}

enum PrefErrorTypes GetNextPrefs(short prefsFileID)
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

enum PrefErrorTypes SavePrefs(short prefsFileID)
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

enum PrefErrorTypes CheckVersion(short prefsFileID)
{
	OSErr		isHuman;
	long		count;
	short			temp;
	
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

enum PrefErrorTypes GetFileID(void)
{
	ParamBlockRec	pb;
	
	pb.fileParam.ioCompletion=0L;
	pb.fileParam.ioNamePtr=CurApName;
	pb.fileParam.ioVRefNum=0;
	pb.fileParam.ioFVersNum=0;
	pb.fileParam.ioFDirIndex=0;
	if (PBGetFInfo(&pb, FALSE)!=noErr)
		return prefs_diskReadErr;
	
	gFileID=pb.fileParam.ioFlNum;
}

enum PrefErrorTypes CheckFileID(void)
{
	/* compare file ID in current prefs struct to application's file ID */
	return (thePrefs.fileID==gFileID) ? prefs_allsWell : prefs_IDNotMatchErr;
}

enum PrefErrorTypes Virgin(short prefsFileID)
{
	short			err;
	
	DefaultPrefs();
	CopyGlobalsToPrefs();
	err=SavePrefs(prefsFileID);
	if (err!=prefs_allsWell)
		return err;
	DoSound(sound_virgin, TRUE);
	GetRegistration();
	CopyGlobalsToPrefs();
	err=SavePrefs(prefsFileID);
	
	return (err==prefs_allsWell) ? prefs_virginErr : err;
}

void DefaultPrefs(void)
{
	unsigned char		*bob="\pBob";
	
	Mymemcpy((Ptr)gMyName, (Ptr)bob, bob[0]+1);
	gMyOrg[0]=0x00;
	gSnowColor=0x07;	/* white */
	gSnowSpeed=0x03;	/* fast */
	gSnowMutates=0xff;	/* true */
	gSoundToggle=0xFF;
	gNumRows=4;
	gNumColumns=6;
	gMainTopicShowing=gSubTopicShowing=0;
}

void CopyGlobalsToPrefs(void)
{
	Mymemset((Ptr)(&thePrefs), 0, sizeof(thePrefs));
	if (gMyName[0]>0x27)
		gMyName[0]=0x27;
	if (gMyOrg[0]>0x27)
		gMyOrg[0]=0x27;
	Mymemcpy((Ptr)thePrefs.regname, (Ptr)gMyName, gMyName[0]+1);
	Mymemcpy((Ptr)thePrefs.regorg, (Ptr)gMyOrg, gMyOrg[0]+1);
	thePrefs.maintopic=gMainTopicShowing;
	thePrefs.subtopic=gSubTopicShowing;
	thePrefs.fileID=gFileID;
	
	thePrefs.sound=gSoundToggle;
	thePrefs.doesntmutate=gSnowMutates^0xff;
	thePrefs.snowcolor=gSnowColor;
	thePrefs.snowspeed=gSnowSpeed;
	thePrefs.numrows=gNumRows;
	thePrefs.numcolumns=gNumColumns;
}

void CopyPrefsToGlobals(void)
{
	Mymemcpy((Ptr)gMyName, (Ptr)thePrefs.regname, thePrefs.regname[0]+1);
	Mymemcpy((Ptr)gMyOrg, (Ptr)thePrefs.regorg, thePrefs.regorg[0]+1);
	gMainTopicShowing=thePrefs.maintopic;
	gSubTopicShowing=thePrefs.subtopic;
	
	gSnowMutates=thePrefs.doesntmutate^0xff;
	gSnowColor=thePrefs.snowcolor;
	gSnowSpeed=thePrefs.snowspeed;
	gSoundToggle=thePrefs.sound;
	gNumRows=thePrefs.numrows;
	gNumColumns=thePrefs.numcolumns;
}

void GetRegistration(void)
{
	DialogPtr		theDlog;
	short			itemSelected = 0;
	short			newleft;
	short			newtop;
	short			itemType;
	Handle			item;
	Rect			box;
	
	theDlog = GetNewDialog(personalDialog, 0L, (WindowPtr)-1L);
	newleft = screenBits.bounds.left + (((screenBits.bounds.right -
				screenBits.bounds.left) - (theDlog->portRect.right -
				theDlog->portRect.left)) / 2);
	newtop = screenBits.bounds.top + (((screenBits.bounds.bottom -
				screenBits.bounds.top) - (theDlog->portRect.bottom -
				theDlog->portRect.top)) / 2);
	if(newtop < 15)
		newtop = 15;
	GetDItem(theDlog, 1, &itemType, &item, &box);
	InsetRect(&box, -4, -4);
	SetDItem(theDlog, 8, userItem, (ProcPtr)OutlineDefaultButton, &box);
	ParamText(APPLICATION_NAME, "\p", "\p", "\p");
	
	MoveWindow(theDlog, newleft, newtop, TRUE);
	ShowWindow(theDlog);
	while(itemSelected != 1)
	{
		ModalDialog(0L, &itemSelected);
	}
	GetDItem(theDlog,4,&itemType,&item,&box);
	GetIText(item,gMyName);
	
	GetDItem(theDlog,5,&itemType,&item,&box);
	GetIText(item,gMyOrg);

	if (gMyName[0]==0x00)
		DefaultPrefs();

	HideWindow(theDlog);
	DisposeDialog(theDlog);
	
	gIsVirgin=TRUE;
}
