/*----------------------------------------------------------------------------

	full.c

	This module handles tasks involving the full group list.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>

#include "glob.h"
#include "full.h"
#include "util.h"
#include "qsort.h"
#include "dlgutil.h"
#include "open.h"
#include "resize.h"
#include "wind.h"
#include "close.h"
#include "nntp.h"



#define kDeletedGroupsDlg				142			/* Deleted groups dialog */
#define kDeletedGroupsTheInfoItem		3



static short	gNumAllocGroups;	/* number of groups allocated in full group list */
static long		gGroupNamesSize;	/* current size of gGroupNames full group array */
static long		gGroupNamesNext;	/* next available offset in gGroupNames */

static TGroup	**gNewGroups;		/* handle to new group array */
static short	gNumNew = 0;		/* number of new groups */
static short 	gNumNewAllocated;	/* number of new groups allocated in gNewGroups */

static Handle	gDeleted;			/* handle to list of deleted groups */
static long		gDeletedLen;		/* length of list of deleted groups */
static long		gDeletedAllocated;	/* number of bytes allocated for gDeleted */



/*----------------------------------------------------------------------------
	FindGroupIndex 
	
	Finds the index of a group in the full group array gGroupArray.

	Entry:	name = group name.
			gGroupArray sorted in increasing order by group name.

	Exit:	function result = index in gGroupArray of group, or -1 if not found.
----------------------------------------------------------------------------*/
 
short FindGroupIndex (const char *name)
{
	long low = 0;
	long high = gNumGroups-1;
	long mid;
	short compare;
	
	while (low <= high) {
		mid = (low + high) >> 1;
		compare = strcmp(name, *gGroupNames + (*gGroupArray)[mid].nameOffset);
		if (compare == 0) {
			return mid;
		} else if (compare < 0) {
			high = mid-1;
		} else {
			low = mid+1;
		}
	}
	return -1;		
}



/*----------------------------------------------------------------------------
	StoreOneGroup 
	
	Appends a new group to the end of a group array.

	Entry:	group = group name.
			groupArray = handle to group array.
			*numGroups = number of groups in group array.
			*numGroupsAllocated = number of groups allocated in group array.
----------------------------------------------------------------------------*/
 
static void StoreOneGroup (const char *group, TGroup **groupArray,
	short *numGroups, short *numGroupsAllocated)	
{
	TGroup newGroup;
	short nameLen;

	/* Set most fields of the group record to default values */
	
	newGroup.firstMess = newGroup.lastMess = 0;
	newGroup.numUnread = 0;
	newGroup.status = 'y';
	newGroup.unread = nil;
	newGroup.onlyRedrawCount = false;

	/* Append the new group's name to the end of the global groupname list */
	
	nameLen = strlen(group) + 1;
	if (gGroupNamesNext + nameLen > gGroupNamesSize) {
		gGroupNamesSize += 10000;
		MySetHandleSize(gGroupNames, gGroupNamesSize);
	}
	BlockMove(group, *gGroupNames + gGroupNamesNext, nameLen);
	newGroup.nameOffset = gGroupNamesNext;
	if (nameLen > 256) {
		nameLen = 256;
		*(*gGroupNames + gGroupNamesNext + 255) = 0;
	}
	gGroupNamesNext += nameLen;

	/* Store the new group record in the group array */
	
	if (*numGroups >= *numGroupsAllocated) {
		(*numGroupsAllocated) += 500;
		MySetHandleSize((Handle)groupArray, (*numGroupsAllocated)*sizeof(TGroup));
	}
	BlockMove(&newGroup, *groupArray + *numGroups, sizeof(TGroup));
	(*numGroups)++;
}



/*----------------------------------------------------------------------------
	GroupCompare 
	
	A comparison routine used in the calls to fastqsort() in 
	SortFullGroupArray and CheckForNewGroups below. It does a 
	simple string compare and gives time to background applications.
----------------------------------------------------------------------------*/
 
static short GroupCompare (void *one, void *two)
{
	static short Counter = 0;

	if((++Counter & 0x1f) == 0) {
		GiveTime();
		Counter = 0;
	}

	return strcmp(*gGroupNames + ((TGroup*)one)->nameOffset, 
		*gGroupNames + ((TGroup*)two)->nameOffset);
}



/*----------------------------------------------------------------------------
	SortFullGroupArray 
	
	Sorts the full group list alphabetically by group name.
	
	Exit:	function result = true if list sorted, false if canceled.
----------------------------------------------------------------------------*/
 
static Boolean SortFullGroupArray()
{
	Boolean result = true;

	if (gNumGroups) {
		StatusWindow("Sorting full group list.");
		HLock((Handle)gGroupArray);
		result = FastQSort(*gGroupArray, gNumGroups, sizeof(TGroup), GroupCompare);
		HUnlock((Handle)gGroupArray);
	}
	return result;
}



/*----------------------------------------------------------------------------
	ReadGroupsFromPrefs 
	
	Reads in the full group list stored in the preferences file.
	
	Entry:	fRefNum = refnum of opened prefs file.

	Exit:	function result = true if no errors, false if error or canceled.
----------------------------------------------------------------------------*/
 
Boolean ReadGroupsFromPrefs (short fRefNum)
{
	OSErr err;
	long logEOF;
	Boolean needSort = false;
	char *p, *pEnd;
	char *prevName = nil;
	char *curName;
	TGroup *g;

	StatusWindow("Reading full group list.");	

	/* Read the saved group names. */
	
	if ((err = GetEOF(fRefNum, &logEOF)) != noErr) goto exit;
	gGroupNamesSize = gGroupNamesNext = logEOF - sizeof(TPrefRec);
	gGroupNames = MyNewHandle(gGroupNamesSize);
	HLock(gGroupNames);
	if ((err = FSRead(fRefNum, &gGroupNamesSize, *gGroupNames)) != noErr) goto exit;
	HUnlock(gGroupNames);
	
	/* Check to make certain the last group name ends in CR. If not, strip
	   any trailing junk. */
	   
	p = *gGroupNames + gGroupNamesSize - 1;
	while (p >= *gGroupNames && *p != CR) p--;
	p++;
	if (p < *gGroupNames + gGroupNamesSize) {
		gGroupNamesSize = gGroupNamesNext = p - *gGroupNames;
		MySetHandleSize(gGroupNames, gGroupNamesSize);
		if (p == *gGroupNames) return true;
	}
	
	/* Walk through the gGroupNames buffer. Count the number of groups. 
	   Change all CR to 0. Check to see if the groups are alread sorted
	   (they should be). */
	   
	HLock(gGroupNames);
	p = *gGroupNames;
	pEnd = p + gGroupNamesSize;
	while (p < pEnd) {
		curName = p;
		while (*p != CR) p++;
		*p++ = 0;
		gNumGroups++;
		needSort = needSort || (prevName != nil && strcmp(prevName, curName) > 0);
		prevName = curName;
		if ((gNumGroups & 0x1f) == 0 && !GiveTime()) {
			err = -1;
			goto exit;
		}
	}
	HUnlock(gGroupNames);
	
	/* Allocate and initialize the full group array. */
	
	gGroupArray = (TGroup**)MyNewHandle(gNumGroups*sizeof(TGroup));
	p = *gGroupNames;
	pEnd = p + gGroupNamesSize;
	g = *gGroupArray;
	while (p < pEnd) {
		g->nameOffset = p - *gGroupNames;
		while (*p != 0) p++;
		p++;
		g++;
	}
	
	/* If necessary, sort the full group array. */
	
	if (needSort && !SortFullGroupArray()) goto exit2;
	
	return true;
	
exit:

	UnexpectedErrorMessage(err);
	
exit2:

	if (gGroupNames != nil) HUnlock(gGroupNames);
	gNumGroups = 0;
	return false;
}



/*----------------------------------------------------------------------------
	MakeGroupList 
	
	Creates a List Manager list for a group window.

	Entry:	numGroups = number of groups in list.
			theList = handle to list record.

	Exit:	Any previous list deleted. A new list is created with numGroups
			cells. The cell data is initialized to 0, 1, 2, ..., numGroups-1.
----------------------------------------------------------------------------*/
 
void MakeGroupList (short numGroups, ListHandle theList)
{
	short i;
	short *pCells;
	short *pCellArray;
	short offset;
	
	LDoDraw(false,theList);
	LDelRow(0, 0, theList);
	LAddRow(numGroups,0,theList);

	MySetHandleSize((**theList).cells, 2*numGroups);
	pCells = (short*)*((**theList).cells);
	pCellArray = (**theList).cellArray;
	offset = 0;

	for (i=0; i<numGroups; i++) {
		*pCellArray++ = offset;
		*pCells++ = i;
		offset += 2;
	}
	*pCellArray = offset;
	LDoDraw(true,theList);
}



/*----------------------------------------------------------------------------
	CreateNewGroupListWindow 
	
	Creates a New Groups window containing the names of the new newsgroups.
----------------------------------------------------------------------------*/

void CreateNewGroupListWindow (void)
{
	TWindow **info;
	WindowPtr theWind;
	GrafPtr savePort;
	Cell theCell;
	Point firstOffset;
	
	if (gNumNew == 0) return;
	
	SetPt(&firstOffset,kOffLeft,kOffTop);
	
	info = (TWindow**) GetWRefCon(theWind = 
		MakeNewWindow(kNewGroup,firstOffset,"\pNew Groups"));
	
	(**info).numGroups = gNumNew;
	(**info).groupArray = gNewGroups;
	
	MakeGroupList(gNumNew, (**info).theList);
	
	SetPt(&theCell,0,0);
	LSetSelect(true,theCell,(**info).theList);
	
	DoZoom(theWind, inZoomOut);
	ShowWindow(theWind);
	
	GetPort(&savePort);
	SetPort(theWind);
	InvalRect(&theWind->portRect);
	SetPort(savePort);
}



/*----------------------------------------------------------------------------
	CreateFullGroupListWindow 
	
	Creates the full group list window.

	Exit:	function result = true if window created, false if error.
----------------------------------------------------------------------------*/
 
Boolean CreateFullGroupListWindow (void)
{
	WindowPtr wind;
	TWindow **info;
	GrafPtr savePort;
	Point firstOffset;
	Point thePt;
	WStateData **stateHndl;

	SetPt(&firstOffset,kOffLeft,kOffTop);
	info = (TWindow**) GetWRefCon(gFullGroupWindow = wind =
				MakeNewWindow(kFullGroup,firstOffset,"\pFull Group List"));

	(**info).numGroups = gNumGroups;
	(**info).groupArray = gGroupArray;
	
	MakeGroupList(gNumGroups, (**info).theList);
	
	SetPt(&thePt,0,0);
	LSetSelect(true,thePt,(**info).theList);
	
	GetPort(&savePort);
	SetPort(wind);

	SizeWindow(wind,
		gPrefs.groupWindowRect.right-gPrefs.groupWindowRect.left,
		gPrefs.groupWindowRect.bottom-gPrefs.groupWindowRect.top,
		true);
	SizeContents(wind);
	MoveWindow(wind,
		gPrefs.groupWindowRect.left,
		gPrefs.groupWindowRect.top,
		false);
	stateHndl = (WStateData **) ((WindowPeek)wind)->dataHandle;
	(**stateHndl).userState = gPrefs.groupWindowRect;

	if (!gPrefs.groupWindowVisible) {
		gPrefs.groupWindowVisible = true;
		ShowHideGroups();
		gMustDoZoomOnShowFullGroupList = true;
	} else {
		if (!DoZoom(wind,inZoomOut)) return false;
		ShowWindow(wind);
		gMustDoZoomOnShowFullGroupList = false;
	}

	InvalRect(&wind->portRect);
	SetPort(savePort);
	
	return true;
}	



/*----------------------------------------------------------------------------
	AdjustFullGroupListChildWindows

	This function must be called whenever the full group list changes. It locates
	all the open child subject list windows. If a group has been deleted, any
	associated open child subject list window is closed. If a group still exists,
	the "parentGroup" backpointer in the child window's TWindow info is adjusted
	to point to the new location of the parent group in the full group array
	gGroupArray.
----------------------------------------------------------------------------*/
 
static void AdjustFullGroupListChildWindows (void)
{
	TWindow		**info, **childInfo;
	TChild		**childList, **prevChildList;
	WindowPtr	childWindow;
	CStr255		groupName;
	short		index;
	
	gFullGroupListDirty = true;
	if (gFullGroupWindow == nil) return;
	info = (TWindow**)GetWRefCon(gFullGroupWindow);
	childList = (**info).childList;
	prevChildList = nil;
	while (childList != nil) {
		childWindow = (**childList).childWindow;
		childInfo = (TWindow**)GetWRefCon(childWindow);
		strcpy(groupName, *gGroupNames + (**childInfo).groupNameOffset);
		index = FindGroupIndex(groupName);
		if (index == -1) {
			/* Group has been deleted. Close the child window. */
			DoCloseWindow(childWindow);
			if (prevChildList == nil) {
				childList = (**info).childList;
			} else {
				childList = (**prevChildList).next;
			}
		} else {
			/* Group still exists. Update the parentGroup backpointer. */
			(**childInfo).parentGroup = index;
			prevChildList = childList;
			childList = (**childList).next;
		}
	}
}


/*----------------------------------------------------------------------------
	UpdateFullGroupWindow 
	
	Makes sure that the Full Group List window corresponds to the changed 
	full group list. It must be called whenever the full group list changes.
----------------------------------------------------------------------------*/
 
static void UpdateFullGroupWindow (void)
{
	TWindow **info;
	Point thePt;
	GrafPtr savePort;

	AdjustFullGroupListChildWindows();
	if (gFullGroupWindow) {
		info = (TWindow**)GetWRefCon(gFullGroupWindow);
		(**info).groupArray = gGroupArray;
		(**info).numGroups = gNumGroups;
		MakeGroupList(gNumGroups, (**info).theList);
		SetPt(&thePt,0,0);
		LSetSelect(true,thePt,(**info).theList);
		GetPort(&savePort);
		SetPort(gFullGroupWindow);
		SizeContents(gFullGroupWindow);
		InvalRect(&gFullGroupWindow->portRect);
		SetPort(savePort);
	}
}



/*----------------------------------------------------------------------------
	NewGroupsFunc

	When we're checking for new groups, GetGroupNames calls this function
	once for every group that the server returns. The new groups are
	accumulated in the gNewGroups group array.

	Entry:	name = the name of the potential new group.

	Exit:	function result = true if no errors, false if error or cancelled.
----------------------------------------------------------------------------*/
 
static Boolean NewGroupsFunc(const char *name)
{
	short nameWidth;

	if (!GiveTime()) return false;
	if (FindGroupIndex(name) == -1) {
		StoreOneGroup(name, gNewGroups, &gNumNew, &gNumNewAllocated);
		if (gPrefs.maxGroupNameWidth > 0) {
			nameWidth = TextWidth(name, 0, strlen(name));
			if (nameWidth > gPrefs.maxGroupNameWidth) gPrefs.maxGroupNameWidth = nameWidth;
		}
	}
	return true;
}



/*----------------------------------------------------------------------------
	MergeNewGroupsIntoFullGroupList 
	
	Merges new groups into the full group list. Both lists must be sorted on 
	entry. The full group list remains sorted on exit.
----------------------------------------------------------------------------*/

static void MergeNewGroupsIntoFullGroupList (void)
{
	short numLeftToInsert, numToMoveUp;
	TGroup *fullListPtr, *newListPtr;
	char *newName;

	gNumGroups += gNumNew;
	gNumAllocGroups = gNumGroups;
	MySetHandleSize((Handle)gGroupArray, gNumAllocGroups*sizeof(TGroup));
	
	numLeftToInsert = gNumNew;
	fullListPtr = *gGroupArray + gNumGroups - gNumNew - 1;
	newListPtr = *gNewGroups + gNumNew - 1;
	while (numLeftToInsert > 0) {
		newName = *gGroupNames + newListPtr->nameOffset;
		numToMoveUp = 0;
		while (fullListPtr >= *gGroupArray &&
			strcmp(newName, *gGroupNames + fullListPtr->nameOffset) <= 0) 
		{
			fullListPtr--;
			numToMoveUp++;
		}
		if (numToMoveUp > 0)
			BlockMove(fullListPtr + 1, fullListPtr + numLeftToInsert + 1, 
				numToMoveUp*sizeof(TGroup));
		BlockMove(newListPtr, fullListPtr + numLeftToInsert, sizeof(TGroup));
		newListPtr--;
		numLeftToInsert--;
	}
}



/*----------------------------------------------------------------------------
	CheckForNewGroups 
	
	Asks the NNTP server for any new groups created since last time we checked.

	Exit:	function result = true if no errors, false if error or canceled.
----------------------------------------------------------------------------*/
 
Boolean CheckForNewGroups (void)
{
	StatusWindow("Checking for new groups.");

	gNewGroups = (TGroup**)MyNewHandle(100*sizeof(TGroup));
	gNumNewAllocated = 100;
	gNumNew = 0;

	/* Ask NNTP server for the new group names. */
	
	SetPort(gFullGroupWindow);
	if (!GetGroupNames(gPrefs.groupCheckTime, NewGroupsFunc)) goto exit;

	if (gNumNew > 0) {
	
		/* Shrink the gNewGroups and gGroupNames arrays. */
		
		MySetHandleSize((Handle)gNewGroups, gNumNew*sizeof(TGroup));
		gGroupNamesSize = gGroupNamesNext;
		MySetHandleSize(gGroupNames, gGroupNamesNext);
		
		/* Sort the gNewGroups array. */
		
		HLock((Handle)gNewGroups);
		if (!FastQSort(*gNewGroups, gNumNew, sizeof(TGroup), GroupCompare)) goto exit;
		HUnlock((Handle)gNewGroups);
		
		/* Merge the new groups into the full group list. */

		MergeNewGroupsIntoFullGroupList();
		UpdateFullGroupWindow();
		
	} else {
	
		MyDisposHandle((Handle)gNewGroups);
		
	}

	/* Record the time new groups were checked for */
	
	GetDateTime(&gPrefs.groupCheckTime);

	return true;
	
exit:
	MyDisposHandle((Handle)gNewGroups);
	gNumNew = 0;
	return false;
}



/*----------------------------------------------------------------------------
	DelGroupsFunc

	When we're checking for deleted groups, GetGroupNames calls this function
	once for every group in the server's full group list. It changes the group's 
	status to ' ' to mark it as still present.

	Entry:	name = group name.

	Exit:	function result = true if no errors, false if error or canceled.
----------------------------------------------------------------------------*/
 
static Boolean DelGroupsFunc (const char *name)
{
	short index;

	if (!GiveTime()) return false;
	index = FindGroupIndex(name);
	if (index != -1) (*gGroupArray)[index].status = ' ';
	return true;
}



/*----------------------------------------------------------------------------
	CheckForDeletedGroups 
	
	Fetches the entire group list from the server and uses it to see if any 
	of the groups in the full group list have been deleted
----------------------------------------------------------------------------*/
 
void CheckForDeletedGroups (void)
{
	short i, numDel, numToMoveDown;
	TGroup *prev, *cur, *curEnd;
	char *name;
	short nameWidth;
	long len;
	DialogPtr dlg;
	short fontNum;
	short item;

	StatusWindow("Checking for deleted groups.");

	/* Mark all groups as deleted, then see which ones really exist */

	for (i = 0; i < gNumGroups; i++) (*gGroupArray)[i].status = 'x';
	if (!GetGroupNames(0, DelGroupsFunc)) return;

	/* Delete any groups still marked as deleted. */
	
	gDeleted = MyNewHandle(1000);
	gDeletedLen = 0;
	gDeletedAllocated = 1000;
	
	SetPort(gFullGroupWindow);
	numDel = 0;
	HLock((Handle)gGroupArray);
	HLock(gGroupNames);
	prev = *gGroupArray;
	cur = *gGroupArray;
	curEnd = *gGroupArray + gNumGroups;
	while (cur < curEnd) {
		numToMoveDown = 0;
		while (cur < curEnd && cur->status != 'x') {
			cur++;
			numToMoveDown++;
		}
		if (numDel > 0 && numToMoveDown > 0)
			BlockMove(prev + numDel, prev, numToMoveDown*sizeof(TGroup));
		prev += numToMoveDown;
		if (cur < curEnd) {
			numDel++;
			name = *gGroupNames + cur->nameOffset;
			len = strlen(name) + 1;
			if (gDeletedLen + len > gDeletedAllocated) {
				gDeletedAllocated += 1000;
				MySetHandleSize(gDeleted, gDeletedAllocated);
			}
			BlockMove(name, *gDeleted + gDeletedLen, len);
			gDeletedLen += len;
			(*gDeleted)[gDeletedLen - 1] = CR;
			if (gPrefs.maxGroupNameWidth > 0) {
				nameWidth = TextWidth(name, 0, strlen(name));
				if (nameWidth >= gPrefs.maxGroupNameWidth) gPrefs.maxGroupNameWidth = 0;
			}
		}
		cur++;
	}
	HUnlock((Handle)gGroupArray);
	HUnlock(gGroupNames);

	if (numDel > 0) {
		gNumGroups -= numDel;
		gNumAllocGroups = gNumGroups;
		MySetHandleSize((Handle)gGroupArray, gNumGroups*sizeof(TGroup));
		UpdateFullGroupWindow();
		dlg = MyGetNewDialog(kDeletedGroupsDlg);
		GetFNum("\pMonaco", &fontNum);
		MySetHandleSize(gDeleted, gDeletedLen - 1);
		SetItemReadOnly(dlg, kDeletedGroupsTheInfoItem, gDeleted, fontNum, 9);
		MyModalDialog(DialogFilter, &item, false, true);
		MyDisposDialog(dlg);
	} else {
		ErrorMessage("No deleted groups.");
	}
	MyDisposHandle(gDeleted);
}



/*----------------------------------------------------------------------------
	ReadGroupsFunc

	When we're reading the full group list from the server, GetGroupNames calls 
	this function once for every group that the server returns. The new groups
	are appended to the end of the full group array gGroupArray.

	Entry:	name = group name.

	Exit:	function result = true if no errors, false if error or canceled.
----------------------------------------------------------------------------*/
 
static Boolean ReadGroupsFunc (const char *name)
{
	if (!GiveTime()) return false;
	StoreOneGroup(name, gGroupArray, &gNumGroups, &gNumAllocGroups);
	return true;
}



/*----------------------------------------------------------------------------
	ReadGroupsFromServer 
	
	Fetches the entire group list from the server the very first time the 
	program is run (no Prefs file found). It also processes the "Rebuild 
	Full Group List" command.

	Exit:	function result = true if no errors, false if error or cancelled.
----------------------------------------------------------------------------*/
 
Boolean ReadGroupsFromServer (void)
{
	TGroup** savedGroupArray = nil;
	short savedNumGroups;
	short savedNumAllocGroups;

	StatusWindow("Getting full group list from server.");

	if (gGroupArray != nil) {
		savedGroupArray = gGroupArray;
		savedNumGroups = gNumGroups;
		savedNumAllocGroups = gNumAllocGroups;
	}
	gNumGroups = 0;
	gGroupArray = (TGroup**)MyNewHandle(2000*sizeof(TGroup));
	gNumAllocGroups = 2000;
	if (gGroupNames == nil) {
		gGroupNames = MyNewHandle(40000);
		gGroupNamesSize = 40000;
		gGroupNamesNext = 0;
	}
	if (!GetGroupNames(0, ReadGroupsFunc)) goto exit;
	gNumAllocGroups = gNumGroups;
	MySetHandleSize((Handle)gGroupArray, gNumGroups*sizeof(TGroup));
	gGroupNamesSize = gGroupNamesNext;
	MySetHandleSize(gGroupNames, gGroupNamesSize);
	if (!SortFullGroupArray()) goto exit;
	gPrefs.maxGroupNameWidth = 0;
	UpdateFullGroupWindow();
	GetDateTime(&gPrefs.groupCheckTime);
	MyDisposHandle((Handle)savedGroupArray);
	return true;
	
exit:
	if (savedGroupArray != nil) {
		MyDisposHandle((Handle)gGroupArray);
		gGroupArray = savedGroupArray;
		gNumGroups = savedNumGroups;
		gNumAllocGroups = savedNumAllocGroups;
	}
	return false;
}