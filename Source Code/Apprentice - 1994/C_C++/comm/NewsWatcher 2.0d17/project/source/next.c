/*----------------------------------------------------------------------------

	next.c

	This module handles the next article, thread, and group commands.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include "glob.h"
#include "article.h"
#include "close.h"
#include "mark.h"
#include "next.h"
#include "subject.h"
#include "util.h"


/*	DoNextArticle is called in response to the menu command of the same
	name.  It closes the current message window, and opens the next
	unread message or group.
*/

void DoNextArticle (WindowPtr wind, Boolean continueInParent)
{
	WindowPtr parent;
	TWindow **info;
	Cell theCell,newCell;
	ListHandle theList;
	short cellData,cellDataLen,numCells;
	TGroup **groupArray,theGroup;
	TSubject **subjectArray, theSubject;
	EWindowKind kind;
	short index, threadHeadIndex, result;
	Boolean inCollapsedThread;

	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	theList = (**info).theList;
	
	if (kind == kArticle) {
		continueInParent = true;
	} else if (!continueInParent) {
		SetPt(&theCell, 0, 0);
		if (LGetSelect(true, &theCell, theList)) {
			if (kind == kSubject) {
				cellDataLen = 2;
				LGetCell(&cellData, &cellDataLen, theCell, theList);
			}
		} else if (kind == kSubject) {
			continueInParent = true;
		} else {
			SysBeep(1);
			return;
		}
	}

	if (continueInParent) {
		index = (kind == kSubject) ? (**info).parentGroup : 
			(**info).parentSubject;
		parent = (**info).parentWindow;
		info = (TWindow**)GetWRefCon(parent);
		kind = (**info).kind;
		theList = (**info).theList;
		subjectArray = (**info).subjectArray;
		numCells = (**theList).dataBounds.bottom;
		threadHeadIndex = (kind == kSubject && (*subjectArray)[index].collapsed) ? 
			(*subjectArray)[index].threadHeadIndex : index;
		if (FindParentCell(parent, threadHeadIndex, &theCell)) {
			if (kind == kSubject) {
				if ((*subjectArray)[index].collapsed) {
					cellData = (*subjectArray)[index].nextInThread;
					inCollapsedThread = cellData != -1;
				} else {
					inCollapsedThread = false;
				}
				if (!inCollapsedThread) {
					theCell.v++;
					if (theCell.v < numCells) {
						cellDataLen = 2;
						LGetCell(&cellData, &cellDataLen, theCell, theList);
					}
				}
			} else {
				theCell.v++;
			}
		} else {
			theCell.v = numCells;
		}
		DoCloseWindow(wind);
		BringToFront(parent);
		wind = parent;
		if (theCell.v >= numCells) {
			if (kind == kSubject) {
				DoNextArticle(wind, true);
			} else {
				SysBeep(1);
			}
			return;
		}
	}
	
	if (kind == kSubject) {
		subjectArray = (**info).subjectArray;
		numCells = (**theList).dataBounds.bottom;
		newCell = theCell;
		while (true) {
			theSubject = (*subjectArray)[cellData];
			if (!theSubject.read) {
				SetPt(&theCell, 0, 0);
				while (LGetSelect(true, &theCell, theList)) 
					LSetSelect(false, theCell, theList);
				LSetSelect(true, newCell, theList);
				LAutoScroll(theList);
				result = OpenSubjectCell(wind, newCell, 
					theSubject.collapsed ? theSubject.threadOrdinal : 1);
				if (result != 1) return;
			};
			if (theSubject.collapsed && theSubject.nextInThread != -1) {
				cellData = theSubject.nextInThread;
			} else {
				newCell.v++;
				if (newCell.v >= numCells) {
					DoNextArticle(wind, true);
					return;
				} else {
					cellDataLen = 2;
					LGetCell(&cellData, &cellDataLen, newCell, theList);
				}
			}
		}
	} else { /* group list */
		groupArray = (**info).groupArray;
		numCells = (**theList).dataBounds.bottom;
		for (newCell = theCell; newCell.v < numCells; newCell.v++) {
			cellDataLen = 2;
			LGetCell(&cellData, &cellDataLen, newCell, theList);
			theGroup = (*groupArray)[cellData];
			if ((kind == kUserGroup && theGroup.unread != nil) ||
				(kind != kUserGroup && theGroup.lastMess >= theGroup.firstMess)) 
			{ 
				SetPt(&theCell, 0, 0);
				while (LGetSelect(true, &theCell, theList))
					LSetSelect(false, theCell, theList);
				LSetSelect(true, newCell, theList);
				LAutoScroll(theList);
				result = OpenGroupCell(wind, newCell);
				if (result != 1) return;
			}
		}
		SysBeep(1);
	}
}


/*	DoNextThread is called in response to the menu command of the same
	name. 
*/

void DoNextThread (WindowPtr wind)
{
	WindowPtr parent;
	TWindow **info, **parentInfo;
	ListHandle theList;
	TSubject **subjectArray;
	Cell theCell;
	short cellData, cellDataLen, threadHeadIndex, index;
	
	info = (TWindow**)GetWRefCon(wind);
	
	switch ((**info).kind) {
		case kUserGroup:
		case kFullGroup:
		case kNewGroup:
			DoNextArticle(wind, false);
			break;
		case kSubject:
			theList = (**info).theList;
			subjectArray = (**info).subjectArray;
			SetPt(&theCell, 0, 0);
			if (LGetSelect(true, &theCell, theList)) {
				cellDataLen = 2;
				LGetCell(&cellData, &cellDataLen, theCell, theList);
				threadHeadIndex = (*subjectArray)[cellData].threadHeadIndex;
				if (threadHeadIndex != cellData) 
					MarkThread(wind, threadHeadIndex, true);
				DoNextArticle(wind, false);
			} else {
				SysBeep(1);
			}
			break;
		case kArticle:
			parent = (**info).parentWindow;
			index = (**info).parentSubject;
			parentInfo = (TWindow**)GetWRefCon(parent);
			subjectArray = (**parentInfo).subjectArray;
			threadHeadIndex = (*subjectArray)[index].threadHeadIndex;
			MarkThread(parent, threadHeadIndex, true);
			DoNextArticle(wind, true);
			break;
	}
}


/*	DoNextGroup is called in response to the menu command of the same
	name.  If a subject window is open, it marks all of the subjects
	read, closes the subject window, and then does a Next Message.
	If a user group window is open, it just does a Next Message.
*/

void DoNextGroup (WindowPtr wind)
{
	TWindow **info;
	WindowPtr parentWind;
	
	info = (TWindow**) GetWRefCon(wind);
	
	switch ((**info).kind) {
		case kUserGroup:
		case kFullGroup:
		case kNewGroup:
			DoNextArticle(wind, false);
			break;
		case kSubject:
			MarkAllSubjects(wind, true);
			DoNextArticle(wind, true);
			break;
		case kArticle:
			parentWind = (**info).parentWindow;
			DoCloseWindow(wind);
			BringToFront(parentWind);
			DoNextGroup(parentWind);
			break;
	}
}
