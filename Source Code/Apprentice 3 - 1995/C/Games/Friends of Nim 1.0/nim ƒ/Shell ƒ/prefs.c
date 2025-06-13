#include "prefs.h"
#include "program globals.h"
#include "help.h"
#include "dialog layer.h"
#include "environment.h"
#include "memory layer.h"
#include "window layer.h"
#include "music layer.h"
#include "sound layer.h"
#include "nim globals.h"
#include <Folders.h>

#define	PREFS_FILE_NAME				"\pFriends of Nim prefs"
#define	PREFS_TYPE					'pref'
#define	PREFS_HEADER_VERSION		1
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
	Str255			registeredName;
	Str255			registeredOrg;
	
	Boolean			soundToggle;
	Boolean			musicToggle;
	Boolean			displayMessages;
	Boolean			useAnimation;
	Boolean			miserePlay;
	Boolean			unused;
	
	short			gameType;
	short			playerMode;
	
	short			nimNumRows;
	short			nimMaxPerRow;
	short			nimDistribution;
	
	short			primeNumRows;
	short			primeMaxPerRow;
	short			primeDistribution;
	
	short			columnsNumRows;
	short			columnsNumColumns;
	short			columnsDistribution;
	
	short			turnNumRows;
	short			turnNumColumns;
	short			turnDistribution;
	
	short			silverNumPlaces;
	short			silverNumTiles;
	short			silverDistribution;
	
	GameStuffRec	cornerInfo[4];
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
	short			i;
	
	gMainTopicShowing=gSubTopicShowing=0;
	
	Mymemcpy((Ptr)gMyName, (Ptr)bob, bob[0]+1);
	Mymemset((Ptr)gMyOrg, 0, 256);
	
	gSoundToggle=TRUE;
	gMusicToggle=FALSE;
	gDisplayMessages=TRUE;
	gUseAnimation=TRUE;
	gMisere=FALSE;
	
	gGameType=kNim;
	gPlayerMode=kHumanMacMode;
	
	gNimNumRows=kNimNumRowsMax;
	gNimMaxPerRow=kNimMaxPerRowMax;
	gNimDistribution=kNimDistRandom;
	
	gPrimeNumRows=kNimNumRowsMax;
	gPrimeMaxPerRow=kNimMaxPerRowMax;
	gPrimeDistribution=kNimDistRandom;
	
	gColumnsNumRows=kColumnsNumRowsMax;
	gColumnsNumColumns=kColumnsNumColumnsMax;
	gColumnsDistribution=kNimDistRandom;
	
	gTurnNumRows=kTurnNumRowsMax;
	gTurnNumColumns=kTurnNumColumnsMax;
	gTurnDistribution=kTurnDistCheckerboard;
	
	gSilverNumPlaces=kSilverNumPlacesMax;
	gSilverNumTiles=kSilverNumTilesMin;
	gSilverDistribution=kNimDistRandom;
	
	for (i=0; i<4; i++)
	{
		gCornerInfo[i].numRows=kCornerNumRowsMax;
		gCornerInfo[i].numColumns=kCornerNumColumnsMax;
		gCornerInfo[i].start=kCornerStartRandom;
	}
}

static	void CopyGlobalsToPrefs(void)
{
	Mymemset((Ptr)gThePrefsPtr, 0, sizeof(MyPrefsRec));
	Mymemcpy((Ptr)gThePrefsPtr->registeredName, (Ptr)gMyName, gMyName[0]+1);
	Mymemcpy((Ptr)gThePrefsPtr->registeredOrg, (Ptr)gMyOrg, gMyOrg[0]+1);
	gThePrefsPtr->maintopic=gMainTopicShowing;
	gThePrefsPtr->subtopic=gSubTopicShowing;
	gThePrefsPtr->fileID=gFileID;
	
	gThePrefsPtr->soundToggle=gSoundToggle;
	gThePrefsPtr->musicToggle=gMusicToggle;
	gThePrefsPtr->displayMessages=gDisplayMessages;
	gThePrefsPtr->useAnimation=gUseAnimation;
	gThePrefsPtr->miserePlay=gMisere;
	
	gThePrefsPtr->gameType=gGameType;
	gThePrefsPtr->playerMode=gPlayerMode;
	
	gThePrefsPtr->nimNumRows=gNimNumRows;
	gThePrefsPtr->nimMaxPerRow=gNimMaxPerRow;
	gThePrefsPtr->nimDistribution=gNimDistribution;
	
	gThePrefsPtr->primeNumRows=gPrimeNumRows;
	gThePrefsPtr->primeMaxPerRow=gPrimeMaxPerRow;
	gThePrefsPtr->primeDistribution=gPrimeDistribution;
	
	gThePrefsPtr->columnsNumRows=gColumnsNumRows;
	gThePrefsPtr->columnsNumColumns=gColumnsNumColumns;
	gThePrefsPtr->columnsDistribution=gColumnsDistribution;
	
	gThePrefsPtr->turnNumRows=gTurnNumRows;
	gThePrefsPtr->turnNumColumns=gTurnNumColumns;
	gThePrefsPtr->turnDistribution=gTurnDistribution;
	
	gThePrefsPtr->silverNumPlaces=gSilverNumPlaces;
	gThePrefsPtr->silverNumTiles=gSilverNumTiles;
	gThePrefsPtr->silverDistribution=gSilverDistribution;
	
	Mymemcpy((Ptr)gThePrefsPtr->cornerInfo, (Ptr)gCornerInfo, sizeof(GameStuffRec)*4);
}

static	void CopyPrefsToGlobals(void)
{
	Mymemcpy((Ptr)gMyName, (Ptr)gThePrefsPtr->registeredName, gThePrefsPtr->registeredName[0]+1);
	Mymemcpy((Ptr)gMyOrg, (Ptr)gThePrefsPtr->registeredOrg, gThePrefsPtr->registeredOrg[0]+1);
	gMainTopicShowing=gThePrefsPtr->maintopic;
	gSubTopicShowing=gThePrefsPtr->subtopic;
	
	gSoundToggle=gThePrefsPtr->soundToggle;
	gMusicToggle=gThePrefsPtr->musicToggle;
	gDisplayMessages=gThePrefsPtr->displayMessages;
	gUseAnimation=gThePrefsPtr->useAnimation;
	gMisere=gThePrefsPtr->miserePlay;
	
	gGameType=gThePrefsPtr->gameType;
	gPlayerMode=gThePrefsPtr->playerMode;
	
	gNimNumRows=gThePrefsPtr->nimNumRows;
	gNimMaxPerRow=gThePrefsPtr->nimMaxPerRow;
	gNimDistribution=gThePrefsPtr->nimDistribution;
	
	gPrimeNumRows=gThePrefsPtr->primeNumRows;
	gPrimeMaxPerRow=gThePrefsPtr->primeMaxPerRow;
	gPrimeDistribution=gThePrefsPtr->primeDistribution;
	
	gColumnsNumRows=gThePrefsPtr->columnsNumRows;
	gColumnsNumColumns=gThePrefsPtr->columnsNumColumns;
	gColumnsDistribution=gThePrefsPtr->columnsDistribution;
	
	gTurnNumRows=gThePrefsPtr->turnNumRows;
	gTurnNumColumns=gThePrefsPtr->turnNumColumns;
	gTurnDistribution=gThePrefsPtr->turnDistribution;
	
	gSilverNumPlaces=gThePrefsPtr->silverNumPlaces;
	gSilverNumTiles=gThePrefsPtr->silverNumTiles;
	gSilverDistribution=gThePrefsPtr->silverDistribution;
	
	Mymemcpy((Ptr)gCornerInfo, (Ptr)gThePrefsPtr->cornerInfo, sizeof(GameStuffRec)*4);
}

static	void GetRegistration(void)
{
	DialogPtr		theDialog;
	short			itemSelected;
	UniversalProcPtr	modalFilterProc, userItemProc;
	
	theDialog=SetupTheDialog(kPersonalizationDialogID, 8, "\p", (UniversalProcPtr)OneButtonFilter,
		&modalFilterProc, &userItemProc);
	ParamText(APPLICATION_NAME, "\p", "\p", "\p");
	DisplayTheDialog(theDialog, TRUE);
	
	itemSelected=0;
	while (itemSelected != 1)
	{
		itemSelected=GetOneDialogEvent(modalFilterProc);
	}
	
	GetDialogItemString(theDialog, 4, gMyName);
	GetDialogItemString(theDialog, 5, gMyOrg);

	if (gMyName[0]==0x00)
		DefaultPrefs();

	ShutDownTheDialog(theDialog, modalFilterProc, userItemProc);
	
	gIsVirgin=TRUE;
}
