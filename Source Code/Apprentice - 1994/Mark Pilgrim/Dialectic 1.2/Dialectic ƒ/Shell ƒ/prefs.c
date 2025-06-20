/**********************************************************************\

File:		prefs.c

Purpose:	This module handles creating/opening/closing/updating
			the preference file, and copying the preference file
			data into application globals (and back).

\**********************************************************************/

#include "prefs.h"
#include "dialogs.h"
#include "environment.h"
#include "util.h"
#include "program globals.h"
#include "Folders.h"

#define		PREFS_FILE_NAME			"\pDialectic prefs"
#define		PREFS_TYPE				'PREF'
// CREATOR is #defined in "program globals.h"
#define		PREFS_HEADER_VERSION	2

Str255			gMyName;
Str255			gMyOrg;

typedef struct
{
	char			regname[40];
	char			regorg[40];
	unsigned char	showsave;
	unsigned char	addsuffix;
	unsigned char	showprogress;
	unsigned char	dialect;
	unsigned char	useRTF;
	unsigned char	unused;
	long			fileID;
} PrefStruct;

enum		/* possible error codes */
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

/* internal globals for use in prefs.c only */
static long			gFileID;
static Boolean		gCanSavePrefs;
static PrefStruct	thePrefs;
static long			gPrefsFilePos;

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
void GetRegistration(void);

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

void PrefsError(int err)
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

int GetFileID(void)
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

int CheckFileID(void)
{
	/* compare file ID in current prefs struct to application's file ID */
	return (thePrefs.fileID==gFileID) ? prefs_allsWell : prefs_IDNotMatchErr;
}

int Virgin(int prefsFileID)
{
	int			err;
	
	DefaultPrefs();
	CopyGlobalsToPrefs();
	err=SavePrefs(prefsFileID);
	if (err!=prefs_allsWell)
		return err;
	GetRegistration();
	CopyGlobalsToPrefs();
	err=SavePrefs(prefsFileID);
	
	return (err==prefs_allsWell) ? prefs_virginErr : err;
}

void DefaultPrefs(void)
{
	StuffHex(gMyName, "\p03426F62");	/* Bob */
	gMyOrg[0]=gWhichDialect=0x00;
	gUseRTF=gShowSaveDialog=gAddSuffix=gShowProgress=0xFF;
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
	thePrefs.showsave=gShowSaveDialog;
	thePrefs.addsuffix=gAddSuffix;
	thePrefs.showprogress=gShowProgress;
	thePrefs.dialect=gWhichDialect;
	thePrefs.useRTF=gUseRTF;
	thePrefs.unused=0x00;
	thePrefs.fileID=gFileID;
}

void CopyPrefsToGlobals(void)
{
	Mymemcpy(gMyName, thePrefs.regname, thePrefs.regname[0]+1);
	Mymemcpy(gMyOrg, thePrefs.regorg, thePrefs.regorg[0]+1);
	gShowSaveDialog=thePrefs.showsave;
	gAddSuffix=thePrefs.addsuffix;
	gShowProgress=thePrefs.showprogress;
	gWhichDialect=thePrefs.dialect;
	gUseRTF=thePrefs.useRTF;
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
	SetDItem(theDlog, 8, userItem, OutlineDefaultButton, &box);
	ParamText(APPLICATION_NAME, "\p", "\p", "\p");
	
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

	if (gMyName[0]==0x00)
		DefaultPrefs();

	HideWindow(theDlog);
	DisposeDialog(theDlog);
}
