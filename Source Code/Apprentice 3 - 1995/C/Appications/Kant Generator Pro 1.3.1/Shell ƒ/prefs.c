#include "program globals.h"
#include "prefs.h"
#include "help.h"
#include "dialog layer.h"
#include "environment.h"
#include "memory layer.h"
#include "window layer.h"
#include "music layer.h"
#include "speech layer.h"
#include <Folders.h>

#define		PREFS_FILE_NAME			"\pKant Generator Pro prefs"
#define		PREFS_TYPE				'pref'
/* CREATOR is #defined in "program globals.h" */
#define		PREFS_HEADER_VERSION	8
#define	kPersonalizationDialogID	131

Str255			gMyName;
Str255			gMyOrg;

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif
struct MyPrefsRec
{
	long			fileID;
	short			maintopic;
	short			subtopic;
	
	char			regname[40];
	char			regorg[40];
	Boolean			alwaysResolve;
	Boolean			useDefault;
	short			speedDelay;
	MusicStatusTypes	musicStatus;
	OSType			voiceCreator;
	OSType			voiceID;
	Boolean			dynamicScroll;
	Boolean			startFromTop;
	Boolean			ignoreCase;
	Boolean			showAllRefs;
	Boolean			iconifyMenus;
	Boolean			showMessageBox;
	Boolean			showToolbar;
	Boolean			fastResolve;
	Boolean			ignoreErrors;
	Boolean			unused;
	Str255			findStr;
	Str255			replaceStr;
	FSSpec			moduleFS;
};
#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

typedef struct MyPrefsRec			MyPrefsRec, *MyPrefsPtr;

/* internal globals for use in prefs.c only */
static	long			gFileID;
static	Boolean			gCanSavePrefs;
static	MyPrefsPtr		gThePrefsPtr;
static	long			gPrefsFilePos;
static	Boolean			gOptionsRemembered=FALSE;
static	MyPrefsPtr		gRememberedPrefsPtr;

/*-----------------------------------------------------------------------------------*/
/* internal stuff for prefs.c                                                        */

static	enum PrefErrorTypes OpenPrefsFile(short *prefsFileID);
static	enum PrefErrorTypes SetupNewPrefsFile(short prefsFileID);
static	void ClosePrefsFile(short prefsFileID);
static	enum PrefErrorTypes GetNextPrefs(short prefsFileID);
static	enum PrefErrorTypes SavePrefs(short prefsFileID);
static	enum PrefErrorTypes CheckVersion(short prefsFileID);
static	enum PrefErrorTypes GetFileID(void);
static	enum PrefErrorTypes CheckFileID(void);
static	enum PrefErrorTypes Virgin(short prefsFileID);
static	void DefaultPrefs(void);
static	void CopyGlobalsToPrefs(void);
static	void CopyPrefsToGlobals(void);
static	void GetRegistration(void);

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

void RememberOptions(void)
{
	CopyGlobalsToPrefs();
	Mymemcpy((Ptr)gRememberedPrefsPtr, (Ptr)gThePrefsPtr, sizeof(MyPrefsRec));
	gOptionsRemembered=TRUE;
}

Boolean RestoreOptions(void)
{
	if (!gOptionsRemembered)
		return FALSE;
	
	Mymemcpy((Ptr)gThePrefsPtr, (Ptr)gRememberedPrefsPtr, sizeof(MyPrefsRec));
	CopyPrefsToGlobals();
	
	return TRUE;
}

enum PrefErrorTypes PreferencesInit(void)
{
	short			prefsFileID;
	enum PrefErrorTypes		err;
	
	gThePrefsPtr=(MyPrefsPtr)NewPtrClear(sizeof(MyPrefsRec));
	gRememberedPrefsPtr=(MyPrefsPtr)NewPtrClear(sizeof(MyPrefsRec));
	
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
	gPrefsFilePos-=sizeof(MyPrefsRec);
	do
	{
		gPrefsFilePos+=sizeof(MyPrefsRec);
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
			DisplayTheAlert(kStopAlert, kLargeAlertID, tempStr, "\p", "\p", "\p", 0L);
			break;
		default:
			gCanSavePrefs=TRUE;				/* can save prefs to disk later if needed */
			break;
	}
}

static	enum PrefErrorTypes OpenPrefsFile(short *prefsFileID)
{
	short			thisFile;
	Boolean			newPrefs;
	unsigned char	*name=PREFS_FILE_NAME;
	OSErr			isHuman;
	short			vRefNum;
	long			dirID;
	FSSpec			prefsFile;
	
	newPrefs=FALSE;
	/* find vRefNum and dirID of preferences folder, creating it if necessary */
	isHuman=FindFolder(kOnSystemDisk, 'pref', kCreateFolder, &vRefNum, &dirID);
	
	if (isHuman!=noErr)		/* screwed up already?!? */
		return prefs_cantOpenPrefsErr;
	
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
	
	if (newPrefs)
		return SetupNewPrefsFile(*prefsFileID);		/* needs initial setup if new */
	
	return prefs_allsWell;
}

static	enum PrefErrorTypes SetupNewPrefsFile(short prefsFileID)
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

static	void ClosePrefsFile(short prefsFileID)
{
	FSClose(prefsFileID);				/* close file on disk */
	FlushVol(0L, kOnSystemDisk);		/* flush volume to write out new info */
}

static	enum PrefErrorTypes GetNextPrefs(short prefsFileID)
{
	OSErr		isHuman;
	long		count;
	
	count=sizeof(MyPrefsRec);
	isHuman=FSRead(prefsFileID, &count, gThePrefsPtr);		/* get next prefs struct */
	if (isHuman==eofErr)	/* no more left */
		return prefs_noMorePrefsErr;
	if (isHuman!=noErr)		/* some other error */
		return prefs_diskReadErr;
	
	return prefs_allsWell;
}

static	enum PrefErrorTypes SavePrefs(short prefsFileID)
{
	long		oldEOF;
	long		count;
	
	GetEOF(prefsFileID, &oldEOF);
	if (gPrefsFilePos>=oldEOF)		/* add new prefs struct onto end of prefs file */
	{
		if (SetEOF(prefsFileID, oldEOF+sizeof(MyPrefsRec))!=noErr)
			return prefs_diskWriteErr;
	}
	
	SetFPos(prefsFileID, 1, gPrefsFilePos);		/* set position inside prefs file */
	count=sizeof(MyPrefsRec);
	/* write prefs struct and return appropriate error code */
	return (FSWrite(prefsFileID, &count, gThePrefsPtr)!=noErr) ?
		prefs_diskWriteErr : prefs_allsWell;
}

static	enum PrefErrorTypes CheckVersion(short prefsFileID)
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

static	enum PrefErrorTypes GetFileID(void)
{
	ParamBlockRec	pb;
	
	pb.fileParam.ioCompletion=0L;
	pb.fileParam.ioNamePtr=LMGetCurApName();
	pb.fileParam.ioVRefNum=0;
	pb.fileParam.ioFVersNum=0;
	pb.fileParam.ioFDirIndex=0;
	if (PBGetFInfo(&pb, FALSE)!=noErr)
		return prefs_diskReadErr;
	
	gFileID=pb.fileParam.ioFlNum;
	
	return prefs_allsWell;
}

static	enum PrefErrorTypes CheckFileID(void)
{
	/* compare file ID in current prefs struct to application's file ID */
	return (gThePrefsPtr->fileID==gFileID) ? prefs_allsWell : prefs_IDNotMatchErr;
}

static	enum PrefErrorTypes Virgin(short prefsFileID)
{
	enum PrefErrorTypes	err;
	
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

static	void DefaultPrefs(void)
{
	unsigned char		*bob="\pBob";
	
	gMainTopicShowing=gSubTopicShowing=0;
	
	Mymemcpy((Ptr)gMyName, (Ptr)bob, bob[0]+1);
	Mymemset((Ptr)gMyOrg, 0, 256);
	
	gAlwaysResolve=TRUE;
	gSpeedDelay=0;
	gUseDefault=TRUE;
	gModuleFS.name[0]=0x00;
	gDynamicScroll=TRUE;
	gIgnoreCase=TRUE;
	gStartFromTop=TRUE;
	Mymemset((Ptr)gFindString, 0, 256);
	Mymemset((Ptr)gReplaceString, 0, 256);
	gShowAllRefs=TRUE;
	gIconifyMenus=FALSE;
	gShowMessageBox=TRUE;
	gShowToolbar=TRUE;
	gFastResolve=FALSE;
	gIgnoreErrors=FALSE;
	gMusicStatus=kMusicGeneratingOnly;
	gCurrentVoiceCreator='gala';
	gCurrentVoiceID=12;
}

static	void CopyGlobalsToPrefs(void)
{
	Mymemset((Ptr)(gThePrefsPtr), 0, sizeof(MyPrefsRec));
	if (gMyName[0]>0x27)
		gMyName[0]=0x27;
	if (gMyOrg[0]>0x27)
		gMyOrg[0]=0x27;
	Mymemcpy((Ptr)gThePrefsPtr->regname, (Ptr)gMyName, gMyName[0]+1);
	Mymemcpy((Ptr)gThePrefsPtr->regorg, (Ptr)gMyOrg, gMyOrg[0]+1);
	gThePrefsPtr->maintopic=gMainTopicShowing;
	gThePrefsPtr->subtopic=gSubTopicShowing;
	gThePrefsPtr->fileID=gFileID;
	
	gThePrefsPtr->alwaysResolve=gAlwaysResolve;
	gThePrefsPtr->speedDelay=gSpeedDelay;
	gThePrefsPtr->useDefault=gUseDefault;
	gThePrefsPtr->moduleFS=gModuleFS;
	gThePrefsPtr->dynamicScroll=gDynamicScroll;
	gThePrefsPtr->startFromTop=gStartFromTop;
	gThePrefsPtr->ignoreCase=gIgnoreCase;
	Mymemcpy((Ptr)gThePrefsPtr->findStr, gFindString, 256);
	Mymemcpy((Ptr)gThePrefsPtr->replaceStr, gReplaceString, 256);
	gThePrefsPtr->showAllRefs=gShowAllRefs;
	gThePrefsPtr->iconifyMenus=gIconifyMenus;
	gThePrefsPtr->showMessageBox=gShowMessageBox;
	gThePrefsPtr->showToolbar=gShowToolbar;
	gThePrefsPtr->fastResolve=gFastResolve;
	gThePrefsPtr->ignoreErrors=gIgnoreErrors;
	gThePrefsPtr->musicStatus=gMusicStatus;
	gThePrefsPtr->voiceCreator=gCurrentVoiceCreator;
	gThePrefsPtr->voiceID=gCurrentVoiceID;
}

static	void CopyPrefsToGlobals(void)
{
	Mymemcpy((Ptr)gMyName, (Ptr)gThePrefsPtr->regname, gThePrefsPtr->regname[0]+1);
	Mymemcpy((Ptr)gMyOrg, (Ptr)gThePrefsPtr->regorg, gThePrefsPtr->regorg[0]+1);
	gMainTopicShowing=gThePrefsPtr->maintopic;
	gSubTopicShowing=gThePrefsPtr->subtopic;
	
	gAlwaysResolve=gThePrefsPtr->alwaysResolve;
	gSpeedDelay=gThePrefsPtr->speedDelay;
	gUseDefault=gThePrefsPtr->useDefault;
	gModuleFS=gThePrefsPtr->moduleFS;
	gDynamicScroll=gThePrefsPtr->dynamicScroll;
	gStartFromTop=gThePrefsPtr->startFromTop;
	gIgnoreCase=gThePrefsPtr->ignoreCase;
	gShowAllRefs=gThePrefsPtr->showAllRefs;
	Mymemcpy((Ptr)gFindString, (Ptr)gThePrefsPtr->findStr, 256);
	Mymemcpy((Ptr)gReplaceString, (Ptr)gThePrefsPtr->replaceStr, 256);
	gIconifyMenus=gThePrefsPtr->iconifyMenus;
	gShowMessageBox=gThePrefsPtr->showMessageBox;
	gShowToolbar=gThePrefsPtr->showToolbar;
	gFastResolve=gThePrefsPtr->fastResolve;
	gIgnoreErrors=gThePrefsPtr->ignoreErrors;
	gMusicStatus=gThePrefsPtr->musicStatus;
	gCurrentVoiceCreator=gThePrefsPtr->voiceCreator;
	gCurrentVoiceID=gThePrefsPtr->voiceID;
}

static	void GetRegistration(void)
{
	DialogPtr		theDlog;
	short			itemSelected;
	UniversalProcPtr	modalFilterProc, userItemProc;
	
	theDlog=SetupTheDialog(kPersonalizationDialogID, 8, "\p", (UniversalProcPtr)OneButtonFilter,
		&modalFilterProc, &userItemProc);
	ParamText(APPLICATION_NAME, "\p", "\p", "\p");
	DisplayTheDialog(theDlog, TRUE);
	
	itemSelected=0;
	while (itemSelected != 1)
	{
		itemSelected=GetOneDialogEvent(theDlog, modalFilterProc);
	}
	
	GetDialogItemString(theDlog, 4, gMyName);
	GetDialogItemString(theDlog, 5, gMyOrg);

	if (gMyName[0]==0x00)
		DefaultPrefs();

	ShutDownTheDialog(theDlog, modalFilterProc, userItemProc);
	
	gIsVirgin=TRUE;
}
