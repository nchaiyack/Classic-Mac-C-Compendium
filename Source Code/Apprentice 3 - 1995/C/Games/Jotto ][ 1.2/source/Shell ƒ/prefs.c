#include "program globals.h"
#include "prefs.h"
#include "help.h"
#include "dialogs.h"
#include "environment.h"
#include "sounds.h"
#include "util.h"
#include "window layer.h"
#include "Folders.h"

#define		PREFS_FILE_NAME			"\pJotto ][ prefs"
#define		PREFS_TYPE				'PREF'
// CREATOR is #defined in "program globals.h"
#define		PREFS_HEADER_VERSION	2

Str255			gMyName;
Str255			gMyOrg;

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
	short			lastwipe;
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
	pb.fileParam.ioNamePtr=LMGetCurApName();
	pb.fileParam.ioVRefNum=0;
	pb.fileParam.ioFVersNum=0;
	pb.fileParam.ioFDirIndex=0;
	if (PBGetFInfo(&pb, FALSE)!=noErr)
		return prefs_diskReadErr;
	
	gFileID=pb.fileParam.ioFlNum;
	
	return prefs_allsWell;
}

enum PrefErrorTypes CheckFileID(void)
{
	/* compare file ID in current prefs struct to application's file ID */
	return (thePrefs.fileID==gFileID) ? prefs_allsWell : prefs_IDNotMatchErr;
}

enum PrefErrorTypes Virgin(short prefsFileID)
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

void DefaultPrefs(void)
{
	unsigned char		*bob="\pBob";
	
	Mymemcpy((Ptr)gMyName, (Ptr)bob, bob[0]+1);
	gMyOrg[0]=0x00;
	gMainTopicShowing=gSubTopicShowing=0;
	gSoundToggle=0xFF;
	gAllowDup=0x00;
	gNonWordsCount=0xFF;
	gAnimation=0xFF;
	gNumLetters=5;
	gLastWipe=0;
}

void CopyGlobalsToPrefs(void)
{
	Mymemset((Ptr)&thePrefs, 0, sizeof(thePrefs));
	if (gMyName[0]>0x27)
		gMyName[0]=0x27;
	if (gMyOrg[0]>0x27)
		gMyOrg[0]=0x27;
	Mymemcpy(thePrefs.regname, gMyName, gMyName[0]+1);
	Mymemcpy(thePrefs.regorg, gMyOrg, gMyOrg[0]+1);
	thePrefs.maintopic=gMainTopicShowing;
	thePrefs.subtopic=gSubTopicShowing;
	thePrefs.sound=gSoundToggle;
	thePrefs.dup=gAllowDup;
	thePrefs.nonwords=gNonWordsCount;
	thePrefs.animation=gAnimation;
	thePrefs.numletters=gNumLetters;
	thePrefs.unused=0x00;
	thePrefs.lastwipe=gLastWipe;
	thePrefs.fileID=gFileID;
}

void CopyPrefsToGlobals(void)
{
	Mymemcpy(gMyName, thePrefs.regname, thePrefs.regname[0]+1);
	Mymemcpy(gMyOrg, thePrefs.regorg, thePrefs.regorg[0]+1);
	gMainTopicShowing=thePrefs.maintopic;
	gSubTopicShowing=thePrefs.subtopic;
	gSoundToggle=thePrefs.sound;
	gAllowDup=thePrefs.dup;
	gNonWordsCount=thePrefs.nonwords;
	gAnimation=thePrefs.animation;
	gNumLetters=thePrefs.numletters;
	if ((gNumLetters<5) || (gNumLetters>6))
		gNumLetters=5;
	gLastWipe=thePrefs.lastwipe;
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
	
	RemoveHilitePatch();
	
	theDlog = GetNewDialog(131, 0L, (WindowPtr)-1L);
	newleft = qd.screenBits.bounds.left + (((qd.screenBits.bounds.right -
				qd.screenBits.bounds.left) - (theDlog->portRect.right -
				theDlog->portRect.left)) / 2);
	newtop = qd.screenBits.bounds.top + (((qd.screenBits.bounds.bottom -
				qd.screenBits.bounds.top) - (theDlog->portRect.bottom -
				theDlog->portRect.top)) / 2);
	if(newtop < 15)
		newtop = 15;
	GetDItem(theDlog, 1, &itemType, &item, &box);
	InsetRect(&box, -4, -4);
	SetDItem(theDlog, 8, userItem, (Handle)OutlineDefaultButton, &box);
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
	
	InstallHilitePatch();
}
