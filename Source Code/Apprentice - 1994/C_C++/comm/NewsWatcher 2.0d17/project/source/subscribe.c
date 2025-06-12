/*----------------------------------------------------------------------------

	subscribe.c

	This module handles subscribing and unsubscribing to groups.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>

#include "glob.h"
#include "dlgutil.h"
#include "child.h"
#include "close.h"
#include "resize.h"
#include "subscribe.h"
#include "util.h"
#include "nntp.h"
#include "wind.h"



/*----------------------------------------------------------------------------
	AddNewGroup 
	
	Adds a new group to a user group window.
	
	Entry:	nameOffset = offset in gGroupNames of group name to add.
			wind = pointer to user group window.
			pos = position of new user group list entry (row number
				of new cell). Pass 0x7fff to add at end of list.

	Exit:	function result = 
				0 if new group added.
				1 if group already present in window.
				2 if some other error.
----------------------------------------------------------------------------*/

static short AddNewGroup (long nameOffset, WindowPtr wind, short pos)
{
	ListHandle theList;
	Cell theCell;
	TWindow **info;
	TGroup **groupArray, newGroup;
	short numCells, cellData, cellDataLen, newGroupIndex;
	Handle strings;
	TUnread **unread;
	CStr255 groupName;

	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	numCells = (**theList).dataBounds.bottom;
	groupArray = (**info).groupArray;
	strings = (**info).strings;
	strcpy(groupName, *strings + nameOffset);

	theCell.h = 0;
	for (theCell.v = 0; theCell.v < numCells; theCell.v++) {
		cellDataLen = 2;
		LGetCell(&cellData, &cellDataLen, theCell, theList);
		if (strcmp(groupName, *strings + (*groupArray)[cellData].nameOffset) == 0) return 1;
	}
	
	newGroup.nameOffset = nameOffset;
	newGroup.firstMess = newGroup.lastMess = newGroup.numUnread = 0;
	newGroup.onlyRedrawCount = false;
	if (GetGroupArticleRange(&newGroup) == 2) return 2;
	if (newGroup.firstMess <= newGroup.lastMess) {
		unread = (TUnread**)MyNewHandle(sizeof(TUnread));
		(**unread).firstUnread = newGroup.firstMess;
		(**unread).lastUnread = newGroup.lastMess;
		(**unread).next = nil;
		newGroup.unread = unread;
	} else {
		newGroup.unread = nil;
	}

	MySetHandleSize((Handle)groupArray, GetHandleSize((Handle)groupArray) + sizeof(TGroup));
	newGroupIndex = (**info).numGroups;
	(**info).numGroups++;
	(*groupArray)[newGroupIndex] = newGroup;

	LDoDraw(false, theList);
	pos = LAddRow(1, pos, theList);
	SetPt(&theCell, 0, pos);
	LSetCell(&newGroupIndex, 2, theCell, theList);
	LDoDraw(true, theList);

	SetPort(wind);
	InvalRect(&wind->portRect);
	
	(**info).changed = true;
	return 0;
}



/*----------------------------------------------------------------------------
	SubscribeSelected 
	
	Subscribes to all selected newsgroups.
	
	Entry:	srcWindow = pointer to source window.
			destWindow = pointer to destination window.
			pos = position of new user group list entries in destination
				window (starting row number of new cells). 
				Pass 0x7fff to add at end of list.
----------------------------------------------------------------------------*/

void SubscribeSelected (WindowPtr srcWindow, WindowPtr destWindow, short pos)
{
	Cell srcCell;
	TWindow **srcInfo;
	TGroup **srcGroupArray;
	ListHandle srcList;
	short cellData, cellDataLen;
	short result;
	short numSelected=0, numSubscribed=0;
	
	srcInfo = (TWindow**)GetWRefCon(srcWindow);
	srcList = (**srcInfo).theList;
	srcGroupArray = (**srcInfo).groupArray;
	SetPt(&srcCell,0,0);
	while (LGetSelect(true, &srcCell, srcList)) {
		numSelected++;
		cellDataLen = 2;
		LGetCell(&cellData, &cellDataLen, srcCell, srcList);
		result = AddNewGroup((*srcGroupArray)[cellData].nameOffset , destWindow, pos);
		if (result == 2) return;
		if (result == 0) numSubscribed++;
		srcCell.v++;
		pos++;
	}
	if (numSubscribed > 0) {
		if (gPrefs.zoomWindows) {
			DoZoom(destWindow, inZoomOut);
		} else {
			SetWindowNeedsZooming(destWindow);
		}
	}
	if (numSelected == numSubscribed) return;
	if (numSelected == 1) {
		ErrorMessage("That group has already been subscribed to.");
	} else if (numSubscribed > 0) {
		ErrorMessage("One or more of those groups have already been subscribed to.");
	} else {
		ErrorMessage("All of those groups have already been subscribed to.");
	}
	return;
}



/*----------------------------------------------------------------------------
	DoSubscribe 
	
	Handles the Subscribe command.
	
	Entry:	wind = pointer to group list window.
----------------------------------------------------------------------------*/

void DoSubscribe (WindowPtr wind)
{
	TWindow **info, **destInfo;
	WindowPtr destWindow;
	
	info = (TWindow**)GetWRefCon(wind);
		
	destWindow = (WindowPtr) ((WindowPeek)wind)->nextWindow;
	while (destWindow != nil) {
		destInfo = (TWindow**)GetWRefCon(destWindow);
		if ((**destInfo).kind == kUserGroup) break;
		destWindow = (WindowPtr)((WindowPeek)destWindow)->nextWindow;
	}
	if (!destWindow) return;
	
	SubscribeSelected(wind, destWindow, 0x7fff);
}



/*----------------------------------------------------------------------------
	DoUnsubscribe 
	
	Handles the Unsubscribe command.
	
	Entry:	wind = pointer to user group list window.
----------------------------------------------------------------------------*/

void DoUnsubscribe (WindowPtr wind)
{
	WindowPtr child;
	TWindow **info;
	ListHandle theList;
	Cell theCell;
	short numDel = 0;
	
	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	
	SetPt(&theCell,0,0);
	while (LGetSelect(true, &theCell, theList)) {
		if ((child = FindChild(wind, theCell)) != nil) DoCloseWindow(child);
		LDelRow(1, theCell.v, theList);
		(**info).changed = true;
		numDel++;
	}
	if (numDel) SetWindowNeedsZooming(wind);
}

