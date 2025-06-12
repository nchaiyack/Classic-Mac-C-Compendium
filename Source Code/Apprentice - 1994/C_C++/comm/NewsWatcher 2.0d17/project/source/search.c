/*----------------------------------------------------------------------------

	search.c

	This module handles the "Search Selected Groups" command.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "dlgutil.h"
#include "glob.h"
#include "close.h"
#include "mark.h"
#include "menus.h"
#include "nntp.h"
#include "open.h"
#include "popup.h"
#include "resize.h"
#include "search.h"
#include "subscribe.h"
#include "util.h"



#define kSearchDlg			140			/* Search dialog */
#define kSearchHeader		3
#define	kSearchPattern		4
#define kSearchPopup		5



/*----------------------------------------------------------------------------
	DoSearchDialog
	
	Presents the search dialog.
			
	Exit:	function result = true if OK clicked, false if Cancel clicked.
			header = header name.
			pattern = search string.
----------------------------------------------------------------------------*/

static Boolean DoSearchDialog (CStr255 header, CStr255 pattern)
{
	static CStr255 headerSave = "Subject";
	static CStr255 patternSave = "";
	DialogPtr dlg;
	short item;
	
	dlg = MyGetNewDialog(kSearchDlg);
	
	strcpy(header, headerSave);
	strcpy(pattern, patternSave);

	DlgSetCString(dlg, kSearchHeader, header);
	SetItemMaxLength(dlg, kSearchHeader, 255);
	DlgSetCString(dlg, kSearchPattern, pattern);
	SetItemMaxLength(dlg, kSearchPattern, 255);
	SelIText(dlg, kSearchPattern, 0, 255);
	SetItemPopupTypeinItem(dlg, kSearchPopup, kSearchHeader);
	
	do {
		DlgEnableItem(dlg, ok, *header != 0 && *pattern != 0);
		MyModalDialog(DialogFilter, &item, true, true);
		switch (item) {
			case kSearchPopup:
				GetPopupCString(DlgGetControl(dlg, item), kCurrentPopupItem, header);
				DlgSetCString(dlg, kSearchHeader, header);
				SelIText(dlg, kSearchHeader, 0, 0x7fff);
				break;
			case kSearchHeader:
				DlgGetCString(dlg, kSearchHeader, header);
				break;
			case kSearchPattern:
				DlgGetCString(dlg, kSearchPattern, pattern);
				break;
		}
	} while (item != ok && item != cancel);
	
	MyDisposDialog(dlg);
	
	if (item == cancel) return false;
	
	strcpy(headerSave, header);
	strcpy(patternSave, pattern);
	return true;
}



/*----------------------------------------------------------------------------
	SearchOneGroup 
	
	Searches a single group.
	
	Entry:	header = name of the header to be searched.
			pattern = search string.
			*theGroup = group record.
			
	Exit:	function result = true if no error, false if error.
			theGroup->numUnread = number of matched articles.
			theGroup->unread = handle to unread list.
----------------------------------------------------------------------------*/

static Boolean SearchOneGroup (char *header, char *pattern, TGroup *theGroup)
{
	CStr255	groupName, statusStr;
	short result;
	THeader **headers = nil;
	short numHeaders;
	THeader *pHeader, *pHeaderEnd;
	long firstUnread, lastUnread;
	long number;

	strcpy(groupName, *gGroupNames + theGroup->nameOffset);

	strcpy(statusStr,"Searching group: ");
	strcat(statusStr,groupName);
	StatusWindow(statusStr);
	
	result = GetGroupArticleRange(theGroup);
	if (result == 1) return true;
	if (result == 2) return false;
	
	theGroup->numUnread = 0;
	result = SearchHeaders(groupName, header, theGroup->firstMess,
		theGroup->lastMess, pattern, &headers, &numHeaders);
	if (result == 1) return true;
	if (result == 2) return false;

	HLock((Handle)headers);
	pHeaderEnd = *headers + numHeaders;
	firstUnread = 0;
	for (pHeader = *headers; pHeader < pHeaderEnd; pHeader++) {
		number = pHeader->number;
		if (firstUnread == 0) {
			firstUnread = lastUnread = number;
		} else if (pHeader->number == lastUnread+1) {
			lastUnread = number;
		} else {
			AppendUnreadRange(firstUnread, lastUnread, theGroup);
			firstUnread = lastUnread = number;
		}
	}
	if (firstUnread != 0) AppendUnreadRange(firstUnread, lastUnread, theGroup);
	MyDisposHandle((Handle)headers);
	return true;
}



/*----------------------------------------------------------------------------
	SearchGroups
	
	Searches all the selected groups and builds a new group array containing
	the matching groups and article lists.
	
	Entry:	wind = pointer to group window record.
			header = header name.
			pattern = search string.
	
	Exit:	function result = true if no error, false if error.
			*newGroupArray = array of matching group records.
			*newNumGroups = number of matching group records.
----------------------------------------------------------------------------*/

static Boolean SearchGroups (WindowPtr wind, char *header, char *pattern,
	TGroup ***newGroupArray, short *newNumGroups)
{
	TWindow **info;
	ListHandle theList;
	Cell theCell;
	short cellData, cellDataLen;
	TGroup **groupArray;
	TGroup theGroup; 
	TGroup **theNewGroupArray = nil;
	short theNewNumGroups;
	short numAllocated;
	short i;
	TUnread **unread, **nextUnread;

	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	groupArray = (**info).groupArray;
	theNewGroupArray = (TGroup**)MyNewHandle(100*sizeof(TGroup));
	numAllocated = 100;
	theNewNumGroups = 0;
	SetPt(&theCell, 0, 0);
	
	while (LGetSelect(true, &theCell, theList)) {
		cellDataLen = 2;
		LGetCell(&cellData, &cellDataLen, theCell, theList);
		theGroup.nameOffset = (*groupArray)[cellData].nameOffset;
		theGroup.firstMess = theGroup.lastMess = theGroup.numUnread = 0;
		theGroup.unread = nil;
		theGroup.onlyRedrawCount = false;
		if(!SearchOneGroup(header, pattern, &theGroup)) goto exit;
		if (theGroup.numUnread != 0) {
			if (theNewNumGroups >= numAllocated) {
				numAllocated += 100;
				MySetHandleSize((Handle)theNewGroupArray, numAllocated*sizeof(TGroup));
			}
			(*theNewGroupArray)[theNewNumGroups] = theGroup;
			theNewNumGroups++;
		}
		theCell.v++;
	}
	
	MySetHandleSize((Handle)theNewGroupArray, theNewNumGroups*sizeof(TGroup));
	*newGroupArray = theNewGroupArray;
	*newNumGroups = theNewNumGroups;
	return true;
	
exit:

	for (i = 0; i < theNewNumGroups; i++) {
		unread = (*theNewGroupArray)[i].unread;
		while (unread != nil) {
			nextUnread = (**unread).next;
			MyDisposHandle((Handle)unread);
			unread = nextUnread;
		}
	}
	MyDisposHandle((Handle)theNewGroupArray);
	return;
}



/*----------------------------------------------------------------------------
	DoSearch
	
	Handle the "Search Selected Groups" command.
	
	Entry:	wind = pointer to group window record.
----------------------------------------------------------------------------*/

void DoSearch (WindowPtr wind)
{
	CStr255	header,pattern;
	TGroup **groupArray;
	short numGroups;
	WindowPtr newWind;
	TWindow **info;
	
	/* Present the search dialog. */
	
	if (!DoSearchDialog(header, pattern)) return;
	
	/* Search the groups. */
	
	if (!SearchGroups (wind, header, pattern, &groupArray, &numGroups)) return;
	if (numGroups == 0) {
		ErrorMessage("No matching articles were found.");
		return;
	}
	
	/* Create the window. */
	
	newWind = NewUserGroupWindow("\pSearch Groups", groupArray, numGroups);
	info = (TWindow**)GetWRefCon(newWind);
	(**info).okToCloseIfChanged = true;
	ShowWindow(newWind);
	return;
}
