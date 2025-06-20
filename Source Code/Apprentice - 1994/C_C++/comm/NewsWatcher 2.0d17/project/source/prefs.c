/*----------------------------------------------------------------------------

	prefs.c

	This module handles locating, reading, and writing the prefs file, and
	initialization of the preferences if no prefs file exists or an old
	version is encountered.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <Packages.h>
#include <Folders.h>
#include <string.h>
#include <stdio.h>

#include "glob.h"
#include "prefs.h"
#include "dlgutil.h"
#include "util.h"
#include "full.h"



#define kServerDlg			132			/* Server dialog */
#define kNewsServer			6
#define kMailServer			8

#define kPersonalDlg		138			/* Personal info dialog */
#define kFullName			6
#define kOrganization		8
#define kAddress			10

#define kCustAlert			144			/* resource id of customization confirmation
									       alert */




static Str255 gNewsServerAtStartup;		/* news server address at startup */



/*----------------------------------------------------------------------------
	GetDefaultTextDir
	
	Return a reasonable place for storing text files when the
	user hasn't selected one yet. Uses the root directory of the 
	volume where NewsWatcher lives.
	
	Exit:	volName = default volume name for saved text files.
			dirID = default dirID for saved text files.
----------------------------------------------------------------------------*/
 
static void GetDefaultTextDir (Str31 volName, long *dirID)
{
	FCBPBRec pbFcb;
	HParamBlockRec pbVol;
	OSErr err = noErr;

	/* Find the volume that we're running on */
	
	pbFcb.ioNamePtr = nil;
	pbFcb.ioRefNum = CurResFile();
	pbFcb.ioFCBIndx = 0;
	err = PBGetFCBInfo(&pbFcb, false);
	
	/* Now find the volume name. */
	
	if (err == noErr) {
		pbVol.volumeParam.ioNamePtr = volName;
		pbVol.volumeParam.ioVRefNum = pbFcb.ioFCBVRefNum;
		pbVol.volumeParam.ioVolIndex = 0;
		err = PBHGetVInfo(&pbVol, false);
	}

	if (err != noErr) *volName = 0;

	*dirID = fsRtDirID;
}



/*----------------------------------------------------------------------------
	DoServerDialog 
	
	Presents the server dialog.
	
	Entry:	startup = true if initialization call.
	
	Exit:	function result = true if OK clicked, false if Cancel clicked.
			gPrefs.newsServerName = news server name.
			gPrefs.mailServerName = mail server name.
----------------------------------------------------------------------------*/

static Boolean DoServerDialog (void)
{
	DialogPtr dlg;
	short item;
	Str255 news;
	Str255 mail;
	
	dlg = MyGetNewDialog(kServerDlg);
	pstrcpy(news, gPrefs.newsServerName);
	DlgSetPString(dlg, kNewsServer, news);
	SetItemMaxLength(dlg, kNewsServer, 255);
	pstrcpy(mail, gPrefs.mailServerName);
	DlgSetPString(dlg, kMailServer, mail);
	SetItemMaxLength(dlg, kMailServer, 255);
	if (*news == 0) {
		SelIText(dlg, kNewsServer, 0, 0);
	} else if (*mail == 0) {
		SelIText(dlg, kMailServer, 0, 0);
	} else {
		SelIText(dlg, kNewsServer, 0, 0x7fff);
	}
	
	do {
		DlgEnableItem(dlg, ok, *news != 0 && *mail != 0);
		MyModalDialog(DialogFilter, &item, true, true);
		switch (item) {
			case kNewsServer:
				DlgGetPString(dlg, item, news);
				break;
			case kMailServer:
				DlgGetPString(dlg, item, mail);
				break;
		}
	} while (item != ok && item != cancel);

	MyDisposDialog(dlg);
	if (item == ok) {
		pstrcpy(gPrefs.newsServerName, news);
		pstrcpy(gPrefs.mailServerName, mail);
	}
	return item == ok;
}



/*----------------------------------------------------------------------------
	DoPersonalDialog 
	
	Presents the personal information dialog.
	
	Exit:	function result = true if OK clicked, false if Cancel clicked.
			gPrefs.fullName = user's full name.
			gPrefs.organization = user's organization.
			gPrefs.address = user's email address
----------------------------------------------------------------------------*/

static Boolean DoPersonalDialog (void)
{
	DialogPtr dlg;
	short item;
	CStr255 fullName;
	CStr255	organization;
	CStr255 address;
	
	dlg = MyGetNewDialog(kPersonalDlg);
	strcpy(fullName, gPrefs.fullName);
	DlgSetCString(dlg, kFullName, fullName);
	SetItemMaxLength(dlg, kFullName, 255);
	strcpy(organization, gPrefs.organization);
	DlgSetCString(dlg, kOrganization, organization);
	SetItemMaxLength(dlg, kOrganization, 255);
	strcpy(address, gPrefs.address);
	DlgSetCString(dlg, kAddress, address);
	SetItemMaxLength(dlg, kAddress, 255);
	if (*fullName == 0) {
		SelIText(dlg, kFullName, 0, 0);
	} else if (*organization == 0) {
		SelIText(dlg, kOrganization, 0, 0);
	} else if (*address == 0) {
		SelIText(dlg, kAddress, 0, 0);
	} else {
		SelIText(dlg, kFullName, 0, 0x7fff);
	}
	
	do {
		DlgEnableItem(dlg, ok, *address != 0);
		MyModalDialog(DialogFilter, &item, true, true);
		switch (item) {
			case kFullName:
				DlgGetCString(dlg, item, fullName);
				break;
			case kOrganization:
				DlgGetCString(dlg, item, organization);
				break;
			case kAddress:
				DlgGetCString(dlg, item, address);
				break;
		}
	} while (item != ok && item != cancel);

	MyDisposDialog(dlg);
	if (item == ok) {
		strcpy(gPrefs.fullName, fullName);
		strcpy(gPrefs.organization, organization);
		strcpy(gPrefs.address, address);
	}
	return item == ok;
}



/*----------------------------------------------------------------------------
	LocatePrefsFile
	
	Locates the preferences file. It also takes care of locating, moving, 
	and renaming any old prefs file from previous versions, if necessary. 
	Note that this function does not open or read the file, or create it 
	if it doesn't already exist.
	
	Entry:	docList = initial document list.
			numDocs = number of documents in list.

	Exit:	gPrefsFile = FSSpec recording location of prefs file.
----------------------------------------------------------------------------*/

void LocatePrefsFile (FSSpec **docList, short numDocs)
{
	OSErr err;
	FInfo fndrInfo;
	FSSpec oldFile;
	FSSpec theFile;
	short i;
	FCBPBRec pBlock;
	FSSpec prefsFolder;
	CInfoPBRec pb;
	
	/* Check to see if a NewsWatcher prefs file is an element of
	   the document list. If there is, use the first one. */

	for (i = 0; i < numDocs; i++) {
		theFile = (*docList)[i];
		err = FSpGetFInfo(&theFile, &fndrInfo);
		if (err == noErr && fndrInfo.fdCreator == kFCreator && fndrInfo.fdType == kPrefType) {
			gPrefsFile = theFile;
			return;
		}
	}
	
	/* Check to see if a NewsWatcher user group list file is an element
	   of the document list. If it is, check to see if a "NewsWatcher
	   Preferences" file exists in the same folder as the first one. If 
	   there is such a prefs file, use that one. */
	   
	for (i  = 0; i < numDocs; i++) {
		theFile = (*docList)[i];
		err = FSpGetFInfo(&theFile, &fndrInfo);
		if (err == noErr && fndrInfo.fdCreator == kFCreator && fndrInfo.fdType == kFType) {
			err = FSMakeFSSpec(theFile.vRefNum, theFile.parID, kPrefName, &gPrefsFile);
			if (err == noErr) {
				err = FSpGetFInfo(&gPrefsFile, &fndrInfo);
				if (err == noErr) goto exit;
			}
		}
	}
	
	/* If the doc list is empty, check to see if a "NewsWatcher Preferences" file 
	   exists in the same folder as the NewsWatcher application. If there is such a 
	   prefs file, use that one. */
	   
	if (numDocs == 0) {
		pBlock.ioNamePtr = nil;
		pBlock.ioVRefNum = 0;
		pBlock.ioRefNum = CurApRefNum;
		pBlock.ioFCBIndx = 0;
		err = PBGetFCBInfo(&pBlock, false);
		if (err == noErr) {
			err = FSMakeFSSpec(pBlock.ioFCBVRefNum, pBlock.ioFCBParID, kPrefName, &gPrefsFile);
			if (err == noErr) {
				err = FSpGetFInfo(&gPrefsFile, &fndrInfo);
				if (err == noErr) goto exit;
			}
		}
	}
	
	/* Construct gPrefsFile = FSSpec for "NewsWatcher Preferences" file
	   in Preferences folder. */

	err = FindFolder(kOnSystemDisk, kPreferencesFolderType,
		kCreateFolder, &gPrefsFile.vRefNum, &gPrefsFile.parID);
	if (err != noErr) {
		ErrorMessage("Internal error - FindFolder failed.");
		ExitToShell();
	}
	pstrcpy(gPrefsFile.name, kPrefName);
	
	/* Check to see if "NewsWatcher Preferences" exists in the Preferences
	   folder. If it does exist, use that one. */
	   
	err = FSpGetFInfo(&gPrefsFile, &fndrInfo);
	if (err == noErr) goto exit;
	
	/* If "NewsWatcher Preferences" does not exist in the Preferences
	   folder, next try "News Prefs" in the Preferences folder. If that
	   exists, rename it "NewsWatcher Preferences". */
	   
	err = FSMakeFSSpec(gPrefsFile.vRefNum, gPrefsFile.parID, kOldPrefName, &oldFile);
	if (err == noErr) {
		FSpRename(&oldFile, kPrefName);
		return;
	}
	
	/* If this fails, next try "News Prefs" in the System folder. If that
	   exists, move it to the Preferences folder and rename it "NewsWatcher
	   Preferences". */
	   
	err = FindFolder(kOnSystemDisk, kSystemFolderType,
		kCreateFolder, &oldFile.vRefNum, &oldFile.parID);
	if (err != noErr) return;
	pb.dirInfo.ioNamePtr = prefsFolder.name;
	pb.dirInfo.ioVRefNum = gPrefsFile.vRefNum;
	pb.dirInfo.ioFDirIndex = -1;
	pb.dirInfo.ioDrDirID = gPrefsFile.parID;
	err = PBGetCatInfo(&pb, false);
	if (err != noErr) return;
	prefsFolder.vRefNum = pb.dirInfo.ioVRefNum;
	prefsFolder.parID = pb.dirInfo.ioDrParID;
	err = FSpCatMove(&oldFile, &prefsFolder);
	if (err != noErr) return;
	oldFile.vRefNum = gPrefsFile.vRefNum;
	oldFile.parID = gPrefsFile.parID;
	err = FSpRename(&oldFile, kPrefName);
	if (err != noErr) return;
	err = FSpGetFInfo(&gPrefsFile, &fndrInfo);
	if (err != noErr) return;
	fndrInfo.fdFlags &= ~(1 << 8);    /* clear hasBeenInited to force Finder to
										 assign new icon location */
	FSpSetFInfo(&gPrefsFile, &fndrInfo);
	return;
	
exit:

	if (fndrInfo.fdCreator == kFCreator && fndrInfo.fdType == kPrefType) return;
	ErrorMessage("The owners of this shared Macintosh insist that you open NewsWatcher using a personal preferences file. Ask them for help, or see the NewsWatcher document.");
	ExitToShell();

}



/*----------------------------------------------------------------------------
	ScramblePW
	
	Scrambles (and unscrambles) saved passwords. This is not really secure,
	just something to foil people browsing using disk editors.
	
	Entry:	pw = the password.
			len = length of password.
			
	Exit:	Each byte nibble-swapped and bit-flipped.
----------------------------------------------------------------------------*/

static void ScramblePW (char *pw, short len)
{
	char *p, *pEnd;
	
	pEnd = pw + len;
	for (p = pw; p < pEnd; p++) *p = (((*p >> 4) & 0x0f) | ((*p & 0x0f) << 4)) ^ 0xff;
}



/*----------------------------------------------------------------------------
	ReadPrefs 
	
	Reads the preferences file.
	
	Exit:	function result = true if no error, false if error.
----------------------------------------------------------------------------*/

Boolean ReadPrefs (void)
{
	OSErr err;
	short fRefNum = 0;
	long count;
	Boolean need102Defaults = true;
	Boolean need12NUDefaults = true;
	Boolean need13d1Defaults = true;
	Boolean need13d4Defaults = true;
	Boolean need20d9Defaults = true;
	Boolean need20d10Defaults = true;
	Boolean need20d11Defaults = true;
	Boolean need20d12Defaults = true;
	Boolean need20d14Defaults = true;
	Boolean need20d16Defaults = false;
	Boolean haveListFont, haveTextFont, differentFonts;
	CStr255 msg;
	short fontNum;
	Handle prefsHandle;
	Boolean havePrefs = false;
	
	err = FSpOpenDF(&gPrefsFile, fsRdPerm, &fRefNum);
	if (err == noErr) {
		count = sizeof(TPrefRec);
		err = FSRead(fRefNum, &count, (Ptr)&gPrefs);
		if (err == noErr && count == sizeof(TPrefRec)) {
			havePrefs = true;
		} else {
			FSClose(fRefNum);
			fRefNum = 0;
		}
	}
	if (fRefNum == 0) {
		prefsHandle = Get1Resource('PREF', 128);
		if (prefsHandle != nil && GetHandleSize(prefsHandle) == sizeof(TPrefRec)) {
			BlockMove(*prefsHandle, &gPrefs, sizeof(TPrefRec));
			havePrefs = true;
			ReleaseResource(prefsHandle);
		}
	}

	if (havePrefs) {
		ScramblePW(gPrefs.remotePassword, sizeof(gPrefs.remotePassword));
		need102Defaults = false;
		if (strcmp(gPrefs.magicCookie, "MagicCookie") == 0) {
			need12NUDefaults = false;
			if (strcmp(gPrefs.version, "1.3d1") == 0) {
				need13d1Defaults = false;
			} else if (strcmp(gPrefs.version, "1.3d4") == 0 ||
				strcmp(gPrefs.version, "1.3d5") == 0 ||
				strcmp(gPrefs.version, "1.3d6") == 0) 
			{
				need13d1Defaults = false;
				need13d4Defaults = false;
			} else if (strcmp(gPrefs.version, "2.0d9") == 0) {
				need13d1Defaults = false;
				need13d4Defaults = false;
				need20d9Defaults = false;
			} else if (strcmp(gPrefs.version, "2.0d10") == 0) {
				need13d1Defaults = false;
				need13d4Defaults = false;
				need20d9Defaults = false;
				need20d10Defaults = false;
			} else if (strcmp(gPrefs.version, "2.0d11") == 0) {
				need13d1Defaults = false;
				need13d4Defaults = false;
				need20d9Defaults = false;
				need20d10Defaults = false;
				need20d11Defaults = false;
			} else if (strcmp(gPrefs.version, "2.0d12") == 0) {
				need13d1Defaults = false;
				need13d4Defaults = false;
				need20d9Defaults = false;
				need20d10Defaults = false;
				need20d11Defaults = false;
				need20d12Defaults = false;
			} else if (strcmp(gPrefs.version, "2.0d14") == 0) {
				need13d1Defaults = false;
				need13d4Defaults = false;
				need20d9Defaults = false;
				need20d10Defaults = false;
				need20d11Defaults = false;
				need20d12Defaults = false;
				need20d14Defaults = false;
			} else if (strcmp(gPrefs.version, "2.0d16") == 0) {
				need13d1Defaults = false;
				need13d4Defaults = false;
				need20d9Defaults = false;
				need20d10Defaults = false;
				need20d11Defaults = false;
				need20d12Defaults = false;
				need20d14Defaults = false;
				need20d16Defaults = false;
			}
		}
	}
	
	if (need102Defaults) {
		*gPrefs.newsServerName = 0;
		*gPrefs.mailServerName = 0;
		*gPrefs.name = 0;
		*gPrefs.host = 0;
		*gPrefs.fullName = 0;
		*gPrefs.organization = 0;
		*gPrefs.signature = 0;
		*gPrefs.address = 0;
		pstrcpy(gPrefs.listFont, "\pMonaco");
		pstrcpy(gPrefs.textFont, "\pMonaco");
		gPrefs.listSize = 9;
		gPrefs.textSize = 9;
		SetRect(&gPrefs.groupWindowRect, 4, 40, 300, 340);
		SetPt(&gPrefs.statusWindowLocn, 48, 100);
		gPrefs.groupWindowVisible = true;
		gPrefs.maxFetch = 400;
	}
	
	if (need12NUDefaults) {
		gPrefs.areYouSureAlert = true;
	}
	
	if (need13d1Defaults) {
		gPrefs.autoFetchnewsrc = false;
		gPrefs.replyCC = false;
	}
	
	if (need13d4Defaults) {
		gPrefs.showHeaders = false;
		gPrefs.showAuthors = true;
		gPrefs.showThreadsCollapsed = true;
	}
	
	if (need20d9Defaults) {
		gPrefs.checkForNewGroups = true;
		gPrefs.showKilledArticles = false;
		gPrefs.groupCheckTime = 0;
		memset(gPrefs.remotePassword, 0, sizeof(gPrefs.remotePassword));
		gPrefs.savePassword = false;
		gPrefs.expandHilited = true;
		gPrefs.textCreator = 'ttxt';
		pstrcpy(gPrefs.textCreatorName, "\pTeachText");
		GetDefaultTextDir(gPrefs.textVolName, &gPrefs.textDirID);
		gPrefs.maxGroupNameWidth = 0;
		gPrefs.useXPAT = false;
	}
	
	if (need20d10Defaults) {
		gPrefs.textDefaultDir = false;
		strcpy(gPrefs.remotePath, ".newsrc");
	}
	
	if (need20d11Defaults) {
		gPrefs.addSigSeparatorLine = true;
	}
	
	if (need20d12Defaults) {
		gPrefs.keypadShortcuts = false;
		gPrefs.logActionsToFile = false;
	}
	
	if (need20d14Defaults) {
		gPrefs.batchedGroupCmds = false;
		gPrefs.noNewConnection = false;
		gPrefs.noModeReader = false;
	}
	
	if (need20d16Defaults) {
		gPrefs.zoomWindows = false;
		gPrefs.autoSaveOnQuit = false;
	}

	strcpy(gPrefs.magicCookie, "MagicCookie");
	strcpy(gPrefs.version, "2.0d16");
	
	if (*gPrefs.newsServerName == 0 && !DoServerDialog()) goto exit;
	if (*gPrefs.address == 0 && !DoPersonalDialog()) goto exit;

	haveListFont = GetFontNumber(gPrefs.listFont, &fontNum);
	haveTextFont = GetFontNumber(gPrefs.textFont, &fontNum);
	differentFonts = !EqualString(gPrefs.listFont, gPrefs.textFont, false, false);
	if (!haveListFont) {
		sprintf(msg, "The font �%#s� does not exist on this system. Monaco will be used instead.",
			gPrefs.listFont);
		ErrorMessage(msg);
		pstrcpy(gPrefs.listFont, "\pMonaco");
	}
	if (!haveTextFont) {
		if (differentFonts) {
			sprintf(msg, "The font �%#s� does not exist on this system. Monaco will be used instead.",
				gPrefs.textFont);
			ErrorMessage(msg);
		}
		pstrcpy(gPrefs.textFont, "\pMonaco");
	}
	
	pstrcpy(gNewsServerAtStartup, gPrefs.newsServerName);

	if (!PtInRect(*(Point*)&gPrefs.groupWindowRect, &gDesktopExtent)) {
		SetRect(&gPrefs.groupWindowRect, 4, 40, 300, 340);
	}

	if (fRefNum == 0) return true;
	
	if (!ReadGroupsFromPrefs(fRefNum)) goto exit;
	
	FSClose(fRefNum);
	
	return true;

exit:

	if (fRefNum != 0) FSClose(fRefNum);
	return false;
}



/*----------------------------------------------------------------------------
	WriteGroups 
	
	Writes the full group list to the preferences file.
	
	Entry:	fRefNum = refnum of opened prefs file.
----------------------------------------------------------------------------*/

static void WriteGroups (short fRefNum)
{
	OSErr err;
	Handle groupBuf = nil;
	long groupBufNext, groupBufAllocated;
	short i, len;
	char *groupName;
	
	StatusWindow("Writing full group list.");
	
	/* Allocate and build the buffer of group names. */
	
	groupBuf = MyNewHandle(100000);
	groupBufNext = 0;
	groupBufAllocated = 100000;
	for (i = 0; i < gNumGroups; i++) {
		groupName = *gGroupNames + (*gGroupArray)[i].nameOffset;
		len = strlen(groupName) + 1;
		if (groupBufNext + len > groupBufAllocated) {
			groupBufAllocated += 50000;
			MySetHandleSize(groupBuf, groupBufAllocated);
		}
		BlockMove(groupName, *groupBuf + groupBufNext, len);
		groupBufNext += len;
		*(*groupBuf + groupBufNext -1) = CR;
		if ((i & 0x1f) == 0) GiveTime();
	}
	
	/* Write the buffer to the file. */
	
	HLock(groupBuf);
	err = FSWrite(fRefNum, &groupBufNext, *groupBuf);
	if (err != noErr) goto exit;
	err = SetEOF(fRefNum, sizeof(TPrefRec) + groupBufNext);
	if (err != noErr) goto exit;
	
	MyDisposHandle(groupBuf);
	return;

exit:
	UnexpectedErrorMessage(err);
	if (groupBuf) MyDisposHandle(groupBuf);
}



/*----------------------------------------------------------------------------
	WritePrefs 
	
	Writes the preferences file.
----------------------------------------------------------------------------*/

void WritePrefs (void)
{
	OSErr err;
	short fRefNum = 0;
	long count;

	err = FSpOpenDF(&gPrefsFile, fsRdWrPerm, &fRefNum);
	if (err == fnfErr) {
		FSpCreate(&gPrefsFile, kFCreator, kPrefType, smSystemScript);
		err = FSpOpenDF(&gPrefsFile, fsRdWrPerm, &fRefNum);
	}
	if (err != noErr) goto exit;

	if (!gPrefs.savePassword)
		memset(gPrefs.remotePassword, 0, sizeof(gPrefs.remotePassword));
	ScramblePW(gPrefs.remotePassword, sizeof(gPrefs.remotePassword));

	count = sizeof(TPrefRec);
	err = FSWrite(fRefNum, &count, &gPrefs);
	if (err != noErr) goto exit;
	
	/* If the user changed news servers, save an empty full group list to force the
	   program to read a new full group list the next time it is run. */
	
	if (!EqualString(gPrefs.newsServerName, gNewsServerAtStartup, false, true)) {
		gNumGroups = 0;
		gFullGroupListDirty = true;
	}
	
	if (gStartupOK && gFullGroupListDirty) WriteGroups(fRefNum);
	
	FSClose(fRefNum);
	FlushVol(nil, gPrefsFile.vRefNum);
	return;
	
exit:

	if (fRefNum != 0) FSClose(fRefNum);
	FlushVol(nil, gPrefsFile.vRefNum);
	UnexpectedErrorMessage(err);
}
	


/*----------------------------------------------------------------------------
	CustomizeNewsWatcher 
	
	Adds a 'PREF' id=128 resource to NewsWatcher to record the current 
	preferences settings.
----------------------------------------------------------------------------*/

void CustomizeNewsWatcher (void)
{
	Handle prefsHandle;
	DialogPtr dlg;
	short item;
	TPrefRec *savedPrefs;

	dlg = MyGetNewDialog(kCustAlert);
	MyModalDialog(DialogFilter, &item, true, true);
	MyDisposDialog(dlg);
	if (item != ok) return;
	
	prefsHandle = Get1Resource('PREF', 128);
	if (prefsHandle == nil) {
		prefsHandle = MyNewHandle(sizeof(TPrefRec));
		BlockMove(&gPrefs, *prefsHandle, sizeof(TPrefRec));
		AddResource(prefsHandle, 'PREF', 128, "\pDefault preferences");
	} else {
		SetHandleSize(prefsHandle, sizeof(TPrefRec));
		BlockMove(&gPrefs, *prefsHandle, sizeof(TPrefRec));
		ChangedResource(prefsHandle);
	}

	HLock(prefsHandle);
	savedPrefs = (TPrefRec*)(*prefsHandle);
	savedPrefs->savePassword = false;
	memset(savedPrefs->remotePassword, 0, sizeof(gPrefs.remotePassword));
	ScramblePW(savedPrefs->remotePassword, sizeof(gPrefs.remotePassword));
	HUnlock(prefsHandle);
	WriteResource(prefsHandle);
	ReleaseResource(prefsHandle);
}
