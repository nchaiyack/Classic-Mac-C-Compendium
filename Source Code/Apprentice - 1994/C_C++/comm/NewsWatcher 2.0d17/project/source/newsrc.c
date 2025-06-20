/*----------------------------------------------------------------------------

	newsrc.c

	This module handles newsrc-format files, including opening
	and saving user group lists from and to disk in newsrc format,
	and getting and sending newsrc files from and to remote hosts. 
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "dlgutil.h"
#include "glob.h"
#include "newsrc.h"
#include "util.h"
#include "prefs.h"
#include "ftp.h"
#include "open.h"
#include "full.h"
#include "resize.h"
#include "mark.h"
#include "nntp.h"
#include "wind.h"
#include "sfutil.h"
#include "log.h"



#define kHostDlg			129			/* Remote host dialog */
#define kRemoteHost			4
#define kRemoteLogin		6
#define kRemotePassword		8
#define kRemotePath			10
#define kAutoGetPut			11
#define kSavePassword		12



#define kCheckSaveID		133			/* Save confirm dialog */



#define kDeletedGroupsDlg				142			/* Deleted groups dialog */
#define kDeletedGroupsTheInfoItem		3




/* The following globals are used when parsing newsrc lists. */

static TGroup 	**gUserGroupArray;			/* handle to user group array under construction */
static short 	gNumUserGroups;				/* number of user groups */
static short 	gNumUserGroupsAllocated;	/* number of user groups allocated */
static Handle 	gUnsubscribed;				/* handle to list of unsubscribed newsrc lines */
static long 	gUnsubscribedLen;			/* length of list of unsubscribed newsrc lines */
static long 	gUnsubscribedAllocated;		/* number of bytes allocated for gUnsubscribed */
static Handle	gDeleted;					/* handle to list of deleted groups */
static long		gDeletedLen;				/* length of list of deleted groups */
static long		gDeletedAllocated;			/* number of bytes allocated for gDeleted */
static char 	*gPos;						/* current parsing position in newsrc list */
static TGroup 	gTheGroup;					/* current group under construction */

/* The following global variables are used when constructing newsrc lists. */

static Handle	gNewsrc;					/* newsrc list under construction */
static long		gNewsrcLength;				/* length of newsrc */
static long		gNewsrcAllocated;			/* number of bytes allocated in newsrc */



/*----------------------------------------------------------------------------
	DoHostDialog 
	
	Presents the remote host dialog.
	
	Exit:	function result = true if OK clicked, false if Cancel clicked.
----------------------------------------------------------------------------*/

static Boolean DoHostDialog (void)
{
	DialogPtr dlg;
	short item;
	CStr255 tempStr;
	short len;
	CStr255 host;
	CStr255 name;
	char path[32];
	char password[32];
	
	dlg = MyGetNewDialog(kHostDlg);
	strcpy(host, gPrefs.host);
	DlgSetCString(dlg, kRemoteHost, host);
	SetItemMaxLength(dlg, kRemoteHost, 255);
	strcpy(name, gPrefs.name);
	DlgSetCString(dlg, kRemoteLogin, name);
	SetItemMaxLength(dlg, kRemoteLogin, 255);
	strcpy(password, gPrefs.remotePassword);
	len = strlen(password);
	memset(tempStr, '�', len);
	tempStr[len] = 0;
	DlgSetCString(dlg, kRemotePassword, tempStr);
	SetItemPassword(dlg, kRemotePassword, password);
	SetItemMaxLength(dlg, kRemotePassword, 31);
	strcpy(path, gPrefs.remotePath);
	DlgSetCString(dlg, kRemotePath, path);
	SetItemMaxLength(dlg, kRemotePath, 31);
	DlgSetCheck(dlg, kAutoGetPut, gPrefs.autoFetchnewsrc);
	DlgSetCheck(dlg, kSavePassword, gPrefs.savePassword);
	if (*host == 0) {
		SelIText(dlg, kRemoteHost, 0, 0);
	} else if (*name == 0) {
		SelIText(dlg, kRemoteLogin, 0, 0);
	} else if (*password == 0) {
		SelIText(dlg, kRemotePassword, 0, 0);
	} else if (*path == 0) {
		SelIText(dlg, kRemotePath, 0, 0);
	} else {
		SelIText(dlg, kRemoteHost, 0, 0x7fff);
	}
	
	do {
		DlgEnableItem(dlg, ok, *host != 0 && *name != 0 && *password != 0 && *path != 0);
		MyModalDialog(DialogFilter, &item, true, true);
		switch (item) {
			case kRemoteHost:
				DlgGetCString(dlg, item, host);
				break;
			case kRemoteLogin:
				DlgGetCString(dlg, item, name);
				break;
			case kRemotePath:
				DlgGetCString(dlg, item, path);
				break;
			case kAutoGetPut:
			case kSavePassword:
				DlgToggleCheck(dlg, item);
				break;
		}
	} while (item != ok && item != cancel);

	if (item == ok) {
		strcpy(gPrefs.host, host);
		strcpy(gPrefs.name, name);
		strcpy(gPrefs.remotePassword, password);
		strcpy(gPrefs.remotePath, path);
		gPrefs.autoFetchnewsrc = DlgGetCheck(dlg, kAutoGetPut);
		gPrefs.savePassword = DlgGetCheck(dlg, kSavePassword);
	}
	MyDisposDialog(dlg);
	return item == ok;
}



/*----------------------------------------------------------------------------
	Skip 
	
	Skips white space in a newsrc line.
----------------------------------------------------------------------------*/
 
static void Skip (void)
{
	while (*gPos == ' ' || *gPos == '\t') gPos++;
}



/*----------------------------------------------------------------------------
 	GetNumber 
 	
 	Parses a number from a newsrc line. Leading white space is skipped.

	Exit:	function result = parsed number, or -1 if syntax error.
----------------------------------------------------------------------------*/
 
static long GetNumber (void)
{
	long num;
	char *endp;
	
	errno = 0;
	num = strtol(gPos, &endp, 10);
	if (num <= 0 || errno != 0) return -1;
	gPos = endp;
	return num;
}



/*----------------------------------------------------------------------------
	GetUnreadList 
	
	Parses the list of read article ranges in a newsrc line and
	converts it to a linked list of unread article ranges.

	Exit:	function result = true if list parsed, false if error.
			gTheGroup.unread = handle to unread list.
			gTheGroup.lastMess = highest article number read, or 0 if 
				unread list is empty.
----------------------------------------------------------------------------*/
 
static Boolean GetUnreadList (void)
{
	long firstUnread, firstRead, lastRead;
	TUnread **prev, **cur;

	gTheGroup.unread = nil;
	gTheGroup.numUnread = 0;
	
	firstUnread = 1;
	lastRead = 0;
	Skip();
	while (*gPos && *gPos != CR) {
		firstRead = GetNumber();
		if (firstRead < firstUnread) goto exit1;
		Skip();
		if (*gPos == '-') {
			gPos++;
			lastRead = GetNumber();
			if (lastRead < firstRead) goto exit1;
		} else {
			lastRead = firstRead;
		}
		if (firstUnread < firstRead) AppendUnreadRange(firstUnread, firstRead-1, &gTheGroup);
		firstUnread = lastRead+1;
		Skip();
		if (*gPos == ',') {
			gPos++;
			Skip();
		}
	}
	AppendUnreadRange(firstUnread, 0x7fffffff, &gTheGroup);
	gTheGroup.lastMess = lastRead;
	
	if (*gPos == CR) gPos++;
	return true;

exit1:
	cur = gTheGroup.unread;
	while (cur != nil) {
		prev = cur;
		cur = (**cur).next;
		MyDisposHandle((Handle)prev);
	}
	return false;
}



/*----------------------------------------------------------------------------
	RecordDeletedGroup 
	
	Records a user error message for a group which was deleted while
	parsing a newsrc file.
	
	Entry:	groupName = the group name.
			reason = the reason the group was deleted:
				1: Syntax error in newsrc line.
				2: Group not in full group list.
				3: Group deleted on server.
----------------------------------------------------------------------------*/
 
static void RecordDeletedGroup (char *groupName, short reason)
{
	char *reasonStr;
	char msg[512];
	short len;

	switch (reason) {
		case 1:
			reasonStr = "Syntax error.";
			break;
		case 2:
			reasonStr = "Group not in full group list.";
			break;
		case 3:
			reasonStr = "Group deleted on news server.";
			break;
	}
	sprintf(msg, "%s: %s\r", groupName, reasonStr);
	len = strlen(msg);
	if (gDeletedLen + len > gDeletedAllocated) {
		gDeletedAllocated += 1000;
		MySetHandleSize(gDeleted, gDeletedAllocated);
	}
	BlockMove(msg, *gDeleted + gDeletedLen, len);
	gDeletedLen += len;
}



/*----------------------------------------------------------------------------
	ProcessOneNewsrcLine 
	
	Parses and processes one line from a newsrc list.

	Entry:	gPos = pointer to beginning of newsrc line.

	Exit:	gPos = pointer to beginning of next newsrc line.

	There are four possible kinds of newsrc lines:

	1. Lines with syntax errors. These lines are skipped.

	2. Lines for deleted groups (group name not in full group list). 
       These lines are skipped.

	3. Subscribed lines. These lines are parsed and appended to the end of the
       gUserGroupArray, with the following TGroup fields initialized:

	   nameOffset = offset in gGroupNames of group name.
	   firstMess = 1.
       lastMess = highest article number read on newsrc line, or 0 if read article
          list is empty.
       unread = unread list built assuming the range of articles in the group
	     is [1,maxlong]. This will be adjusted later when we learn the real
         range of articles in the group.
	   status = 'x'. This indicates that we need to learn the range of articles
         for the group.
       onlyRedrawCount = false.

       The firstMess, lastMess, and numUnread fields are reset later when
	   we learn the real range of articles in the group.

	4. Unsubscribed lines. These lines are copied as is to the end of the
	   gUnsubscribed memory block. NewsWatcher doesn't do anything with these
       lines except append them to the end of the newsrc file when it is later
       sent to a remote host. This is for compatibility with UNIX newsreaders.
----------------------------------------------------------------------------*/
 
static void ProcessOneNewsrcLine(void)
{
	CStr255 groupName;
	char *groupNameStart, *lineStart;
	short index;
	long len;

	lineStart = gPos;
	Skip();
	groupNameStart = gPos;
	while (*gPos && *gPos != ':' && *gPos != '!' && *gPos != CR) gPos++;
	len = gPos - groupNameStart;
	if (len > 255) {
		BlockMove(groupNameStart, groupName, 255);
		groupName[255] = 0;
		RecordDeletedGroup(groupName, 1);
		if (gPrefs.logActionsToFile) LogNewsrcGroupNameTooLong(groupName);
		goto exit;
	}
	BlockMove(groupNameStart, groupName, len);
	groupName[len] = 0;
	if (len == 0) {
		RecordDeletedGroup("Missing group name", 1);
		if (gPrefs.logActionsToFile) LogNewsrcNoGroupName();
		goto exit;
	}
	
	if (*gPos == ':') {
	
		/* subscribed line - parse it and append the new group to the end
		   of gUserGroupArray. */
		   
		gPos++;
		index = FindGroupIndex(groupName);
		if (index == -1) {
			RecordDeletedGroup(groupName, 2);
			if (gPrefs.logActionsToFile) LogNewsrcGroupNotInFullGroupList(groupName);
			goto exit;
		}
		gTheGroup.nameOffset = (*gGroupArray)[index].nameOffset;
		gTheGroup.firstMess = 1;
		if (!GetUnreadList()) {
			RecordDeletedGroup(groupName, 1);
			if (gPrefs.logActionsToFile) LogNewsrcReadListSyntaxError(groupName);
			goto exit;
		}
		gTheGroup.status = 'x';
		gTheGroup.onlyRedrawCount = false;
		if (gNumUserGroups >= gNumUserGroupsAllocated) {
			gNumUserGroupsAllocated += 50;
			MySetHandleSize((Handle)gUserGroupArray, sizeof(TGroup)*gNumUserGroupsAllocated);
		}
		(*gUserGroupArray)[gNumUserGroups] = gTheGroup;
		gNumUserGroups++;
		if (gPrefs.logActionsToFile) LogNewsrcSubscribedOK(groupName);
		return;
				
	} else if (*gPos == '!') {
	
		/* unsubscribed line - append the line as is to the end of the
		   gUnsubscribed memory block. */
		   
		while (*gPos && *gPos != CR) gPos++;
		if (*gPos == CR) *gPos++;
		len = gPos - lineStart;
		if (gUnsubscribedLen + len > gUnsubscribedAllocated) {
			gUnsubscribedAllocated += 10000;
			MySetHandleSize(gUnsubscribed, gUnsubscribedAllocated);
		}
		BlockMove(lineStart, *gUnsubscribed + gUnsubscribedLen, len);
		gUnsubscribedLen += len;
		if (gPrefs.logActionsToFile) LogNewsrcUnsubscribedOK(groupName);
		return;
		
	} else {
	
		RecordDeletedGroup(groupName, 1);
		if (gPrefs.logActionsToFile) LogNewsrcNoColonOrBang(groupName);
	
	}

exit:

	/* syntax error - skip this line. */

	while (*gPos && *gPos != CR) gPos++;
	if (*gPos == CR) gPos++;
}



/*----------------------------------------------------------------------------
	GetArticleRangeInfo 
	
	Queries the NNTP server to get current article range info for each group 
	in the new user group array.

	Exit:	function result = true if info retrieved, false if error.
----------------------------------------------------------------------------*/
 
static Boolean GetArticleRangeInfo (void)
{
	short i = 0;
	short numDeleted = 0;
	TUnread **prev, **cur;
	char *groupName;
	
	StatusWindow("Checking for new articles.");

	if (!GetGroupArrayArticleRanges(gUserGroupArray, gNumUserGroups))
		return false;

	while (i < gNumUserGroups) {
		gTheGroup = (*gUserGroupArray)[i];
		if (gTheGroup.status == 'x') {
			AdjustUnreadList(&gTheGroup);
			gTheGroup.status = ' ';
			(*gUserGroupArray)[i] = gTheGroup;
			i++;
		} else if (gTheGroup.status == 'd') {
			cur = gTheGroup.unread;
			while (cur != nil) {
				prev = cur;
				cur = (**cur).next;
				MyDisposHandle((Handle)prev);
			}
			gNumUserGroups--;
			if (i < gNumUserGroups) {
				BlockMove(*gUserGroupArray+i+1, *gUserGroupArray+i,
					sizeof(TGroup)*(gNumUserGroups-i));
			}
			numDeleted++;
			HLock(gGroupNames);
			groupName = *gGroupNames + gTheGroup.nameOffset;
			RecordDeletedGroup(groupName, 3);
			if (gPrefs.logActionsToFile) 
				LogNewsrcGroupNotOnServer(groupName);
			HUnlock(gGroupNames);
		}
	}
	
	if (numDeleted > 0)
		MySetHandleSize((Handle)gUserGroupArray, sizeof(TGroup)*gNumUserGroups);
	
	return true;
}



/*----------------------------------------------------------------------------
	MakeUserGroupArrayFromNewsrc 
	
	Creates a new user group array from a newsrc-format list.

	Entry:	newsrc = handle to newsrc-format list, 0-terminated.

	Exit:	function result = true if no error.
			*groupArray = handle to new user group array.
			*numGroups = number of groups in array.
			*unsubscribed = handle to list of unsubscribed groups in newsrc list.
			*deleted = handle to list of deleted groups.
----------------------------------------------------------------------------*/
 
static Boolean MakeUserGroupArrayFromNewsrc(Handle newsrc, TGroup ***groupArray, 
	short *numGroups, Handle *unsubscribed, Handle *deleted)
{	
	gUserGroupArray = (TGroup**)NewHandle(50*sizeof(TGroup));
	gNumUserGroups = 0;
	gNumUserGroupsAllocated = 50;
	gUnsubscribed = NewHandle(10000);
	gUnsubscribedLen = 0;
	gUnsubscribedAllocated = 10000;
	gDeleted = NewHandle(0);
	gDeletedLen = 0;
	gDeletedAllocated = 0;
	
	if (gPrefs.logActionsToFile) LogNewsrcParseBegin(newsrc);
	MoveHHi(newsrc);
	HLock(newsrc);
	gPos = *newsrc;
	while (*gPos) ProcessOneNewsrcLine();
	HUnlock(newsrc);
	
	MySetHandleSize((Handle)gUserGroupArray, gNumUserGroups*sizeof(TGroup));
	MySetHandleSize(gUnsubscribed, gUnsubscribedLen);

	if (!GetArticleRangeInfo()) goto exit1;
	if (gPrefs.logActionsToFile) LogNewsrcParseEnd(gUserGroupArray, gNumUserGroups, gUnsubscribed);
	*groupArray = gUserGroupArray;
	*numGroups = gNumUserGroups;
	*unsubscribed = gUnsubscribed;
	MySetHandleSize(gDeleted, gDeletedLen);
	*deleted = gDeleted;
	return true;
	
exit1:
	HUnlock(newsrc);
	MyDisposHandle((Handle)gUserGroupArray);
	MyDisposHandle(gUnsubscribed);
	MyDisposHandle(gDeleted);
	*groupArray = nil;
	*numGroups = 0;
	*unsubscribed = nil;
	*deleted = nil;
	return false;
}


/*----------------------------------------------------------------------------
	MakeUserGroupWindowFromNewsrc 
	
	Creates a new user group list window from a newsrc-format list.

	Entry:	newsrc = handle to newsrc-format list, 0-terminated.
			title = window title.
			autoFetched = true if autofetched from host.
			theFile = pointer to disk file FSSpec, or nil if fetched from host.
			saved = true if from disk file, false if from host.
----------------------------------------------------------------------------*/
 
static void MakeUserGroupWindowFromNewsrc (Handle newsrc, StringPtr title,
	Boolean autoFetched, FSSpec *theFile, Boolean saved)
{
	TGroup **groupArray;
	Handle unsubscribed;
	Handle deleted;
	WindowPtr wind;
	TWindow **info;
	short numGroups;
	DialogPtr dlg;
	short fontNum;
	short item;

	if (!MakeUserGroupArrayFromNewsrc(newsrc, &groupArray, 
		&numGroups, &unsubscribed, &deleted)) return;
	
	wind = NewUserGroupWindow(title, groupArray, numGroups);
	info = (TWindow**)GetWRefCon(wind);		
	(**info).unsubscribed = unsubscribed;
	(**info).autoFetched = autoFetched;
	if (theFile) (**info).theFile = *theFile;
	(**info).saved = saved;
	ShowWindow(wind);

	if (GetHandleSize(deleted) != 0) {
		(**info).changed = true;
		dlg = MyGetNewDialog(kDeletedGroupsDlg);
		GetFNum("\pMonaco", &fontNum);
		MySetHandleSize(deleted, GetHandleSize(deleted) - 1);
		SetItemReadOnly(dlg, kDeletedGroupsTheInfoItem, deleted, fontNum, 9);
		MyModalDialog(DialogFilter, &item, false, true);
		MyDisposDialog(dlg);
	}
	MyDisposHandle(deleted);
}



/*----------------------------------------------------------------------------
	OpenFile 
	
	Opens a user group list from a disk file.

	Entry:	theFile = the file to be opened.
----------------------------------------------------------------------------*/
 
void OpenFile (FSSpec *theFile)
{
	OSErr err;
	short fRefNum = 0;
	long length;
	Handle newsrc = nil;
	CStr255 msg;
	FInfo fndrInfo;
	WindowPtr wind;
	TWindow **info;
	FSSpec theWindowFile;
	
	/* Check to make sure the file has the proper creator and type. */

	err = FSpGetFInfo(theFile, &fndrInfo);
	if (err != noErr) goto exit1;
	if (fndrInfo.fdType != kFType  || 
		fndrInfo.fdCreator != kFCreator ) return;

	/* Check to see if the file is already open. If it is, bring its
	   window to the front. */
	
	for (wind = FrontWindow(); wind != nil; wind = (WindowPtr)((WindowPeek)wind)->nextWindow) {
		if (IsAppWindow(wind)) {
			info = (TWindow**)GetWRefCon(wind);
			if ((**info).kind == kUserGroup) {
				theWindowFile = (**info).theFile;
				if (IsEqualFSSpec(&theWindowFile, theFile)) {
					SelectWindow(wind);
					return; 
				}
			}
		}
	}
		   	
	/* Open the file in a new window. */	
	
	err = FSpOpenDF(theFile, fsRdPerm, &fRefNum);
	if (err != noErr) goto exit1;
	err = GetEOF(fRefNum, &length);
	if (err != noErr) goto exit1;
	newsrc = MyNewHandle(length+1);
	HLock(newsrc);
	err = FSRead(fRefNum, &length, *newsrc);
	HUnlock(newsrc);
	if (err != noErr) goto exit1;
	(*newsrc)[length] = 0;
	
	MakeUserGroupWindowFromNewsrc(newsrc, theFile->name, false, theFile, true);
	
exit:
	MyDisposHandle(newsrc);
	if (fRefNum != 0) FSClose(fRefNum);
	return;
	
exit1:
	p2cstr(theFile->name);
	sprintf(msg, "Unexpected error %d while trying to open file �%s�", 
		err, theFile->name);
	c2pstr((char*)theFile->name);
	ErrorMessage(msg);
	goto exit;
}



/*----------------------------------------------------------------------------
	DoOpenGroupList 
	
	Handles the "Open Group List" command.
----------------------------------------------------------------------------*/

void DoOpenGroupList (void)
{
	StandardFileReply reply;
	
	MyStandardGetFile(nil, 1, (OSType*)"NEWS", &reply);
	if (reply.sfGood) OpenFile(&reply.sfFile);
}



/*----------------------------------------------------------------------------
	DoGetGroupListFromHost 
	
	Handles the "Get Group List from Host" command.

	Entry:	autoFetch = true if startup call to autoFetch group list from host.
----------------------------------------------------------------------------*/
 
void DoGetGroupListFromHost (Boolean autoFetch)
{
	Handle newsrc = nil;

	/*If this is a startup autofetch and the user has "Save Password" turned on,
	 * don't put up the confirmation dialog unless needed info is missing. */
	 
	if (!(autoFetch && gPrefs.savePassword && *gPrefs.host &&
			*gPrefs.name && *gPrefs.remotePassword && *gPrefs.remotePath)) {
		if (!DoHostDialog()) return;
	}
		
	newsrc = MyNewHandle(0);
	
	StatusWindow("Getting group list from host.");
	if (!FTPGetFile(gPrefs.host, gPrefs.name, gPrefs.remotePassword, 
		gPrefs.remotePath, newsrc))
		goto exit;

	MakeUserGroupWindowFromNewsrc(newsrc, "\p.newsrc", autoFetch, nil, false);

	if (autoFetch) {
		strcpy(gAutoFetchHost,gPrefs.host);
		strcpy(gAutoFetchName,gPrefs.name);
		strcpy(gAutoFetchPass,gPrefs.remotePassword);
		strcpy(gAutoFetchPath,gPrefs.remotePath);
	}

exit:
	MyDisposHandle(newsrc);
}



/*----------------------------------------------------------------------------
	AppendStrToNewsrc 
	
	Appends a string to the newsrc.

	Entry:	str = pointer to string to append to newsrc.
----------------------------------------------------------------------------*/
 
 static void AppendStrToNewsrc (char *str)
 {
 	long len;
 	
 	len = strlen(str);
 	if (gNewsrcLength + len > gNewsrcAllocated) {
 		gNewsrcAllocated += 10000;
 		MySetHandleSize(gNewsrc, gNewsrcAllocated);
 	}
 	BlockMove(str, *gNewsrc + gNewsrcLength, len);
 	gNewsrcLength += len;
 }



/*----------------------------------------------------------------------------
	MakeNewsrcFromUserGroupListWindow 
	
	Makes a newsrc list from a user group list window.

	Entry:	wind = pointer to user group list window.

	Exit:	function result = true if no error, false if error.
			*newsrc = handle to newsrc list.
			*length = length of newsrc list.
----------------------------------------------------------------------------*/
 
 static Boolean MakeNewsrcFromUserGroupListWindow (WindowPtr wind, 
 	Handle *newsrc, long *length)
 {
 	TWindow **info;
 	ListHandle theList;
 	TGroup **groupArray;
 	Handle strings;
 	Cell theCell;
 	short numCells, cellData, cellDataLen;
 	TGroup theGroup;
 	long first, last;
 	TUnread **unread;
 	CStr255 tmpStr;
 	TChild **childListEl;
 	Handle unsubscribed;
 	Boolean firstRange;
 	
 	gNewsrc = MyNewHandle(10000);
 	gNewsrcLength = 0;
 	gNewsrcAllocated = 10000;
 	
 	info = (TWindow**)GetWRefCon(wind);
 	theList = (**info).theList;
 	groupArray = (**info).groupArray;
 	strings = (**info).strings;
 	
 	if (gPrefs.logActionsToFile) LogNewsrcWriteBegin(groupArray, (**info).numGroups, 
 		(**info).unsubscribed);
 	
 	for (childListEl = (**info).childList; childListEl != nil; 
 		childListEl = (**childListEl).next) UpdateUnreadList((**childListEl).childWindow);
 	
 	theCell.h = 0;
 	numCells = (**theList).dataBounds.bottom;
 	HLock(strings);
 	for (theCell.v = 0; theCell.v < numCells; theCell.v++) {
 		cellDataLen = 2;
 		LGetCell(&cellData, &cellDataLen, theCell, theList);
 		theGroup = (*groupArray)[cellData];
 		AppendStrToNewsrc(*strings + theGroup.nameOffset);
 		AppendStrToNewsrc(": ");
 		first = 1;
 		firstRange = true;
 		for (unread = theGroup.unread; unread != nil; unread = (**unread).next) {
 			last = (**unread).firstUnread-1;
 			if (first <= last) {
 				if (first < last) {
 					sprintf(tmpStr, "%lu-%lu", first, last);
 				} else {
 					sprintf(tmpStr, "%lu", first);
 				}
 				if (!firstRange) AppendStrToNewsrc(",");
 				AppendStrToNewsrc(tmpStr);
 				firstRange = false;
 			}
 			first = (**unread).lastUnread + 1;
 		}
 		last = theGroup.lastMess;
 		if (first <= last) {
 			if (first < last) {
 				sprintf(tmpStr, "%lu-%lu", first, last);
 			} else {
 				sprintf(tmpStr, "%lu", first);
 			}
 			if (!firstRange) AppendStrToNewsrc(",");
 			AppendStrToNewsrc(tmpStr);
 		}
 		AppendStrToNewsrc(CRSTR);
 	}
 	HUnlock(strings);
 	 	
 	MySetHandleSize(gNewsrc, gNewsrcLength);
 	
 	if (gPrefs.logActionsToFile) LogNewsrcWriteEnd(gNewsrc);
 
 	*newsrc = gNewsrc;
 	*length = gNewsrcLength;
 	return true;
 }



/*----------------------------------------------------------------------------
	SaveFile 
	
	Saves a user group list window to a disk file.

	Entry:	wind = pointer to user group list window.
			theFile = the file.
			scriptTag = script code.

	Exit:	function result = true if file saved, false if error.
----------------------------------------------------------------------------*/
 
static Boolean SaveFile (WindowPtr wind, FSSpec *theFile, ScriptCode scriptTag)
{
	Handle newsrc = nil;
	long length;
	short fRefNum = 0;
	OSErr err;
	Boolean result;
	CStr255 msg;

	if (!MakeNewsrcFromUserGroupListWindow(wind, &newsrc, &length)) 
		return false;
	
	err = FSpOpenDF(theFile, fsRdWrPerm, &fRefNum);
	if (err == noErr) {
		err = SetEOF(fRefNum, 0);
		if (err != noErr) goto exit1;
	} else if (err = fnfErr) {
		err = FSpCreate(theFile, kFCreator, kFType, scriptTag);
		if (err != noErr) goto exit1;
		err = FSpOpenDF(theFile, fsRdWrPerm, &fRefNum);
		if (err != noErr) goto exit1;
	} else {
		goto exit1;
	}
	
	HLock(newsrc);
	err = FSWrite(fRefNum, &length, *newsrc);
	HUnlock(newsrc);
	result = true;
	
exit:
	MyDisposHandle(newsrc);
	if (fRefNum != 0) FSClose(fRefNum);
	return result;
	
exit1:
	sprintf(msg, "Unexpected error %d while trying to save file �%#s�", 
		err, theFile->name);
	ErrorMessage(msg);
	result = false;
	goto exit;
}



/*----------------------------------------------------------------------------
	DoSaveAs 
	
	Handles the "Save As" command.

	Entry:	wind = pointer to user group list window.

	Exit:	function result = true if file saved, false if canceled or error.
----------------------------------------------------------------------------*/
 
Boolean DoSaveAs (WindowPtr wind)
{
	StandardFileReply reply;
	TWindow **info;
	Boolean good;
	Str255 fName;
	
	info = (TWindow**)GetWRefCon(wind);
	
	GetWTitle(wind, fName);
	MyStandardPutFile("\pSave group list as:", fName, &reply);
    if (!reply.sfGood) return false;
		
	if (good = SaveFile(wind, &reply.sfFile, reply.sfScript)) {
		RemoveWindMenu(wind);
		SetWTitle(wind, reply.sfFile.name);
		AddWindMenu(wind);
		(**info).theFile = reply.sfFile;
		(**info).changed = false;
		(**info).saved = true;
	}
	return good;
}



/*----------------------------------------------------------------------------
	DoSave 
	
	Handles the "Save" command.

	Entry:	wind = pointer to user group list window.

	Exit:	function result = true if file saved, false if canceled or error.
----------------------------------------------------------------------------*/
 
Boolean DoSave (WindowPtr wind)
{
	TWindow **info;
	FSSpec theFile;
	Boolean result;
	
	info = (TWindow**)GetWRefCon(wind);
	if ((**info).saved) {
		theFile = (**info).theFile;
		if (result = SaveFile(wind, &theFile, (**info).scriptTag))
			(**info).changed = false;
		return result;
	} else {
		return DoSaveAs(wind);
	}
}



/*----------------------------------------------------------------------------
	CheckForSave 
	
	Asks the user if he wishes to save a user group list window, and saves it 
	if the user says yes.

	Entry:	wind = pointer to user group list window.

	Exit:	function result = true if file saved or user clicked the 
				"Don't Save" button or okToCloseIfChanged flag is set.
			function result = false if save operation failed or user
				clicked the "Cancel" button.
----------------------------------------------------------------------------*/
 

Boolean CheckForSave (WindowPtr wind)
{
	TWindow **info;
	DialogPtr dlg;
	short item;
	Str255 fName;
	
	info = (TWindow**)GetWRefCon(wind);
	if ((**info).okToCloseIfChanged) return true;
	if (gDone && gPrefs.autoSaveOnQuit && (**info).saved) return DoSave(wind);
	GetWTitle(wind, fName);
	ParamText(fName, "\p", "\p", "\p");
	dlg = MyGetNewDialog(kCheckSaveID);
	SetItemKeyEquivalent(dlg, 3, 'D');
	MyModalDialog(DialogFilter, &item, true, true);
	MyDisposDialog(dlg);
	switch (item) {
		case 1: /* save */
			return (DoSave(wind));
			break;
		case 2: /* cancel */
			return false;
			break;
		case 3: /* don't save */
			return true;
			break;
	}
}



/*----------------------------------------------------------------------------
	DoSendGroupListToHost handles the "Send Group List to Host" command.

	Entry:	wind = pointer to window.
			host = host name.
			name = username on host.
			pass = password.
*/
 
void DoSendGroupListToHost (WindowPtr wind, char *host, char *name, char *pass, 
	char *path)
{
	TWindow **info;
	Handle newsrc = nil;
	long length;
	
	info = (TWindow**)GetWRefCon(wind);

	if (*host == 0) {
		if (!DoHostDialog()) return;
		host = gPrefs.host;
		name = gPrefs.name;
		pass = gPrefs.remotePassword;
		path = gPrefs.remotePath;
	}

	StatusWindow("Sending group list to host.");
	
	if (!MakeNewsrcFromUserGroupListWindow(wind, &newsrc, &length)) 
		return;
		
	HLock((**info).unsubscribed);
	HandAndHand((**info).unsubscribed, newsrc);
	HUnlock((**info).unsubscribed);
	HLock(newsrc);
	if (FTPPutFile(host, name, pass, path, *newsrc, GetHandleSize(newsrc))) 
		(**info).changed = false;
	MyDisposHandle(newsrc);
}
