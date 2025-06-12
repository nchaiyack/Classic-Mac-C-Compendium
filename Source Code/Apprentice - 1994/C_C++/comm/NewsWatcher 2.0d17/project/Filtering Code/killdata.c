/*----------------------------------------------------------
#
#	NewsWatcher	- Macintosh NNTP Client Application
#
#-----------------------------------------------------------
#
#	kill.c
#
#	Routines for manipulating all the data structures that are
#	used to store a killfile in memory.
#
#	John Werner (werner@soe.berkeley.edu)
#	May, 1993
#
#-----------------------------------------------------------*/

#include <string.h>

#include "glob.h"
#include "killdata.h"
#include "util.h"

/* gKillFile
 * This holds all of the killfile info at run time.
 * It is a handle to an array which in turn contains
 * handles to the individual TGroupKill objects.
 */
TGroupKillHdl **gKillFile = nil;


/*---------------------------------------------------------------------------------
 * InitKillEntry
 * This is the "constructor" for the TKillEntry type
 * It initializes an already-allocated TKillEntry, since these are used as fields
 * or automatic objects, not heap objects.
 */
OSErr
InitKillEntry(
	TKillEntry*	entry, const char*	str)
{
	entry->string = MyNewHandle(strlen(str) + 1);
	if (!entry->string) return MyMemErr();

	HLock(entry->string);
	strcpy(*entry->string, str);
	HUnlock(entry->string);

	entry->header = kKillSubject;
	entry->match = kContains;
	entry->action = kKill;
	entry->expires = 0;
	entry->highlight = 0;
	entry->ignoreCase = true;

	return noErr;
}

/*---------------------------------------------------------------------------------
 * CopyKillEntry
 * This is the "copy constructor" for the TKillEntry type
 * Copy one TKillEntry to another.
 */
OSErr
CopyKillEntry(const TKillEntry *from, TKillEntry *to)
{
	OSErr err = noErr;

	*to = *from;		/* Takes care of everything but the string */

	err = MyHandToHand(&(to->string));
	if (err != noErr)
		to->string = nil;

	return err;
}


/*---------------------------------------------------------------------------------
 * DelKillEntry
 * "destructor" for TKillEntry.  Frees memory the entry refers to, but does not
 * free the entry itself.
 */
void
DelKillEntry(TKillEntry *entry)
{
	if (entry->string)
		DisposHandle(entry->string);
	entry->string = nil;
}

/*---------------------------------------------------------------------------------
 * NewGroupKill
 * This is the "constructor" for the TGroupKill type
 * Returns a handle to the new TGroupKill, or nil if out of memory.
 */
TGroupKillHdl
NewGroupKill(char *groupName)
{
	OSErr err;

	TGroupKillHdl aKill = (TGroupKillHdl) MyNewHandle(sizeof(TGroupKill));
	FailNIL(aKill, error);
	
	(**aKill).groupName = nil;		/* Make it safe for disposal*/
	(**aKill).entries = nil;
	(**aKill).numEntries = 0;
	(**aKill).noMatchKill = false;

	(**aKill).groupName = MyNewHandle(strlen(groupName) + 1);
	FailNIL((**aKill).groupName, error);
	strcpy(*(**aKill).groupName, groupName);
	
	(**aKill).entries = (TKillEntry**)NewHandle(0);
	FailNIL((**aKill).entries, error);
	
	return aKill;

error:
	if (aKill)
		DelGroupKill(aKill);
	return nil;
}

TGroupKillHdl
CloneGroupKill(TGroupKillHdl oldKill)
{
	TGroupKillHdl	newKill = nil;
	TKillEntry		**newEntries, **oldEntries;
	short			num = 0;
	Handle			groupName = nil;
	OSErr			err;
	
	newKill = oldKill;						/* First clone the TGroupKill itself */
	FailErr(MyHandToHand((Handle*)&newKill), error);

	groupName = (**newKill).groupName;		/* Clone the group name */
	(**newKill).groupName = nil;
	FailErr(MyHandToHand(&groupName), error);
	(**newKill).groupName = groupName;

	(**newKill).entries = (TKillEntry **)
							MyNewHandle((**newKill).numEntries*sizeof(TKillEntry));
	FailNIL((**newKill).entries, error);

	/* Clone the entry strings */
	newEntries = (**newKill).entries;	HLockHi((Handle)newEntries);
	oldEntries = (**oldKill).entries;	HLockHi((Handle)oldEntries);
	for (num = 0;  num < (**newKill).numEntries; num++) {
		FailErr(CopyKillEntry(*oldEntries + num, *newEntries + num), error);
	}
	HUnlock((Handle)newEntries);
	HUnlock((Handle)oldEntries);

	return newKill;
	
error:
	HUnlock((Handle)(**oldKill).entries);
	if (newKill) {
		(**newKill).numEntries = num;
		DelGroupKill(newKill);				/* Delete the cloned TGroupKill */
	}
	return nil;
}

void
DelGroupKill(TGroupKillHdl aKill)
{
	if ((**aKill).groupName)
		DisposHandle((**aKill).groupName);

	if ((**aKill).entries) {
		short i;
		for (i = 0; i < (**aKill).numEntries; i++) {
			DelKillEntry(*(**aKill).entries + i);
		}
		DisposHandle((Handle)(**aKill).entries);
	}

	DisposHandle((Handle)aKill);
}

/*---------------------------------------------------------------------------------
 * FindGroupKill
 * Find the TGroupKill info corresponding to a given group name.
 * Returns nil if there isn't one.
 */
TGroupKillHdl				/* Return: found TGroupKill or nil */
FindGroupKill(
	const char	*groupName,		/* Name of group to find */
	short		*index)			/* Return: index of group in gKillFile, or -1 */
{
	TGroupKillHdl result = nil;
	long numKill, i;

	*index = -1;

	if (gKillFile) {
		numKill = GetHandleSize((Handle)gKillFile) / sizeof(**gKillFile);
		for (i = 0; i < numKill && !result; i++) {
			TGroupKillHdl thisGroup = (*gKillFile)[i];
			Handle thisGroupName = (**thisGroup).groupName;
			HLock(thisGroupName);
			if (!strcasecmp(groupName, *thisGroupName)) {
				result = thisGroup;
				*index = i;
			}
			HUnlock(thisGroupName);
		}
	}

	return result;
}


/*---------------------------------------------------------------------------------
 * StoreGroupKill
 * Save the TGroupKill info for a group.
 * The group name is taken from the TGroupKill object we're passed.
 */
OSErr
StoreGroupKill(TGroupKillHdl newKill)
{
	CStr255			groupName;
	TGroupKillHdl	oldKill;
	short			index;

	HLock((**newKill).groupName);
	strcpy(groupName, *(**newKill).groupName);
	HUnlock((**newKill).groupName);

	if (!gKillFile) {
		gKillFile = (TGroupKillHdl **) NewHandle(0);
		if (!gKillFile)
			return MemError();
	}

	oldKill = FindGroupKill(groupName, &index);

	if (oldKill) {
		/* This group already had a kill entry.  Replace it */
		DelGroupKill(oldKill);
		(*gKillFile)[index] = newKill;
	} else {
		/* Need to add a new kill entry for this group */
		long numKill = GetHandleSize((Handle)gKillFile) / sizeof(**gKillFile);
		SetHandleSize((Handle)gKillFile, (numKill+1) * sizeof(**gKillFile));
		if (MemError())
			return MemError();
		(*gKillFile)[numKill] = newKill;
	}

	return noErr;
}

/*
 * Routines for converting from header name to a # and back
 */

static char *gKillHeaders[] = {
	"subject",
	"from",
	"keywords",
	"summary",
	"date",
	"organization",
	"references",
	"distribution",
	0
};

short KillHeaderNum(const char *name)
{
	short i, num = -1, len;
	
	len = strlen(name);
	if (len && name[len-1] == ':')
		len--;
	for (i = 0; gKillHeaders[i]; i++) {
		if (strncasecmp(name, gKillHeaders[i], len) == 0) {
			num = i;
			break;
		}
	}
	return num;
}

char * KillHeaderName(short num)
{
	char *result = nil;
	if (num >= 0 && num < sizeof(gKillHeaders)/sizeof(*gKillHeaders) - 1)
		result = gKillHeaders[num];
	return result;
}

/*---------------------------------------------------------------------------------
 * FindGroupName
 * A utility function to find the group name for a given window.
 * The window can be either a group window, a subject window or an article window.
 * If there's an error, "name" is set to an empty string.
 */
void
FindGroupName(
	WindowPtr	wind,
	CStr255		name)
{
	TWindow		**info, **parentInfo;
	TGroup		**groupArray, theGroup;
	Cell 		theCell;
	short		cellData, cellDataLen;
	ListHandle	groupList;

	*name = 0;

	if (wind == NULL)
		return;

	info = (TWindow**)GetWRefCon(wind);
	if ((**info).kind == kArticle) {
		/* We want the info for the subject window, not the article */
		wind = (**info).parentWindow;
		info = (TWindow**)GetWRefCon(wind);
	}

	if ((**info).kind == kSubject) {
		/* For Subject windows, use the parentGroup field to find the group name */
		parentInfo = (TWindow**)GetWRefCon((**info).parentWindow);
		groupArray = (**parentInfo).groupArray;
		theGroup = (*groupArray)[(**info).parentGroup];
		strcpy(name, *gGroupNames + theGroup.nameOffset);
	} else  if ((**info).kind == kUserGroup || (**info).kind == kFullGroup
									||(**info).kind == kNewGroup) {
		/* For group windows, use the name of the first selected group */
		groupList = (**info).theList;
		groupArray = (**info).groupArray;
		SetPt(&theCell, 0, 0);
		if (LGetSelect(true, &theCell, groupList)) {
			cellDataLen = 2;
			LGetCell(&cellData, &cellDataLen, theCell, groupList);
			theGroup = (*groupArray)[cellData];
			strcpy(name, *gGroupNames + theGroup.nameOffset);
		}
	}
}
