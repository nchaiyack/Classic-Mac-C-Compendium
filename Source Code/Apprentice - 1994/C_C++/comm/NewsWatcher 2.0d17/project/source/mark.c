/*----------------------------------------------------------------------------

	mark.c

	This handles marking articles read and unread.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>

#include "glob.h"
#include "child.h"
#include "mark.h"
#include "nntp.h"
#include "util.h"
#include "wind.h"


/*	FindParentCell locates the parent cell in a list window given an
	index in a group or subject array for the window. */
	
Boolean FindParentCell (WindowPtr wind, short index, Cell *theCell)
{
	TWindow **info;
	ListHandle theList;
	short numCells, i;
	short *pCells;

	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	numCells = (**theList).dataBounds.bottom;
	pCells = (short*)*((**theList).cells);
	for (i = 0; i < numCells; i++) {
		if (*pCells == index) {
			SetPt(theCell, 0, i);
			return true;
		}
		pCells++;
	}
	return false;
}


/*	AppendUnreadRange appends a range of unread messages to the end of the 
	unread list for a group. */

void AppendUnreadRange (long first, long last, TGroup *theGroup)
{
	TUnread **unread, **lastRec, **nextRec;

	lastRec = theGroup->unread;
	if (lastRec != nil) {
		while (true) {
			nextRec = (**lastRec).next;
			if (nextRec == nil) break;
			lastRec = nextRec;
		}
	}
	
	if (lastRec != nil && (**lastRec).lastUnread + 1 == first) {
		(**lastRec).lastUnread = last;
	} else {
		unread = (TUnread**)MyNewHandle(sizeof(TUnread));
		(**unread).firstUnread = first;
		(**unread).lastUnread = last;
		(**unread).next = nil;
		if (lastRec == nil) {
			theGroup->unread = unread;
		} else {
			(**lastRec).next = unread;
		}
	}
	theGroup->numUnread += last - first + 1;
}



/*----------------------------------------------------------------------------
 * AdjustUnreadList adjusts the unread list for a group. It trims the unread
 * list to include just articles in the range [firstMess, lastMess].
 * It also recomputes numUnread.
 *
 * Entry:	theGroup = pointer to group record.
 */
 
void AdjustUnreadList (TGroup *theGroup)
{
	TUnread **prev, **cur;
	
	/* Delete all unread ranges which come before firstMess. */
	
	cur = theGroup->unread;
	while (cur && (**cur).lastUnread < theGroup->firstMess) {
		prev = cur;
		cur = (**cur).next;
		MyDisposHandle((Handle)prev);
	}
	theGroup->unread = cur;
	
	/* If necessary, trim the first unread range to begin at firstMess. */
	
	if (cur && (**cur).firstUnread < theGroup->firstMess) 
		(**cur).firstUnread = theGroup->firstMess;
	
	/* Leave alone all unread ranges which come before lastMess. */	
	
	prev = nil;
	while (cur && (**cur).firstUnread <= theGroup->lastMess) {
		prev = cur;
		cur = (**cur).next; 
	}
	
	/* If necessary, trim the last unread range to end at lastMess. */
	
	if (prev && (**prev).lastUnread > theGroup->lastMess)
		(**prev).lastUnread = theGroup->lastMess;
	if (prev) {
		(**prev).next = nil;
	} else {
		theGroup->unread = nil;
	}
		
	/* Delete the remaining unread ranges, which all come after lastMess. */
		
	while (cur) {
		prev = cur;
		cur = (**cur).next;
		MyDisposHandle((Handle)prev);
	}
	
	/* Recalculate numUnread. */
	
	theGroup->numUnread = 0;
	cur = theGroup->unread;
	while (cur) {
		theGroup->numUnread += (**cur).lastUnread - (**cur).firstUnread + 1;
		cur = (**cur).next;
	}
}


/*	UpdateUnreadList is called to update the unread message list for a group
	in a user group list which has an open subject window.
	
	Entry:	wind = pointer to subject window for group.
*/

void UpdateUnreadList (WindowPtr wind)
{
	long tmpFirst,tmpLast,i;
	TGroup theGroup,**groupArray;
	TSubject **subjectArray;
	TWindow **info,**parentInfo;
	short index, numSubjects;
	WindowPtr parentWindow;
	long firstFetched;
	TUnread **unread, **prevUnread=nil, **nextUnread;
	
	info = (TWindow**)GetWRefCon(wind);
	parentWindow = (**info).parentWindow;
	parentInfo = (TWindow**)GetWRefCon(parentWindow);
	if ((**parentInfo).kind != kUserGroup) return;
	groupArray = (**parentInfo).groupArray;
	index = (**info).parentGroup;
	theGroup = (*groupArray)[index];
	
	/* Truncate the unread list for the group so that the unread list
	   includes only articles which were NOT included in the subject
	   window (because there were more than gPrefs.maxFetch unread
	   articles when the subject window was first opened). */
	
	firstFetched = (**info).firstFetched;
	unread = theGroup.unread;
	theGroup.numUnread = 0;
	while (unread != nil) {
		if (firstFetched <= (**unread).lastUnread) break;
		theGroup.numUnread += (**unread).lastUnread - (**unread).firstUnread + 1;
		prevUnread = unread;
		unread = (**unread).next;
	}
	if (unread != nil) {
		if ((**unread).firstUnread < firstFetched) {
			(**unread).lastUnread = firstFetched-1;
			theGroup.numUnread += firstFetched - (**unread).firstUnread;
			prevUnread = unread;
			unread = (**unread).next;
			(**prevUnread).next = nil;
		} else {
			if (prevUnread == nil) {
				theGroup.unread = nil;
			} else {
				(**prevUnread).next = nil;
			}
		}
		while (unread != nil) {
			nextUnread = (**unread).next;
			MyDisposHandle((Handle)unread);
			unread = nextUnread;
		}
	}
	
	(**parentInfo).changed = true;

	/* Walk the subject array for the subject window and append new unread
	   article ranges to the end of the unread list for the group. */
	
	subjectArray = (**info).subjectArray;
	numSubjects = (**info).numSubjects;
	
	tmpFirst = -1;
		
	for (i=0; i<numSubjects; i++) {			
		if (!(*subjectArray)[i].read && 
			i < numSubjects-1 && 
			(*subjectArray)[i+1].number-(*subjectArray)[i].number > 1) 
		{
			if (tmpFirst == -1) tmpFirst = (*subjectArray)[i].number;
			AppendUnreadRange(tmpFirst, (*subjectArray)[i].number, &theGroup);
			tmpFirst = -1;
		}
		else if (tmpFirst == -1 && !(*subjectArray)[i].read)
			tmpFirst = tmpLast = (*subjectArray)[i].number;
		else if (tmpFirst != -1 && !(*subjectArray)[i].read)
			tmpLast = (*subjectArray)[i].number;
		else if (tmpFirst != -1 && (*subjectArray)[i].read) {
			AppendUnreadRange(tmpFirst, tmpLast, &theGroup);
			tmpFirst = -1;
		}
	}
	if (tmpFirst != -1) AppendUnreadRange(tmpFirst, tmpLast, &theGroup);
		
	(*groupArray)[index] = theGroup;
}


/*	LoMarkGroup adjusts the number of unread articles in a group list window
	when one or more subjects in a child subject list window have
	been marked read or unread. 
*/
	
static void LoMarkGroup (WindowPtr child, short deltaUnread)
{
	WindowPtr parent;
	TWindow **childInfo, **parentInfo;
	TGroup **parentGroupArray;
	ListHandle parentList;
	long numUnread;
	short index;
	Cell theCell;
	
	if (deltaUnread == 0) return;
	childInfo = (TWindow**)GetWRefCon(child);
	parent = (**childInfo).parentWindow;
	parentInfo = (TWindow**)GetWRefCon(parent);
	if ((**parentInfo).kind != kUserGroup) return;
	parentGroupArray = (**parentInfo).groupArray;
	parentList = (**parentInfo).theList;
	index = (**childInfo).parentGroup;
	if (FindParentCell(parent, index, &theCell)) {
		numUnread = (*parentGroupArray)[index].numUnread;
		numUnread += deltaUnread;
		if (numUnread < 0) numUnread = 0;
		(*parentGroupArray)[index].numUnread = numUnread;
		(*parentGroupArray)[index].onlyRedrawCount = true;
		LDraw(theCell, parentList);
		(*parentGroupArray)[index].onlyRedrawCount = false;
	}
}


/*	LoMarkArticle marks an article window read or unread. */

static void LoMarkArticle (WindowPtr wind, Boolean read)
{
	Str255 title;

	GetWTitle(wind,title);
	if (read) {
		if (title[1] == (unsigned char)'Ã') return;
		if (*title == 255) *title == 254;
		BlockMove(title+1, title+2, *title);
		title[1] = 'Ã';
		(*title)++;
	} else {
		if (title[1] != (unsigned char)'Ã') return;
		(*title)--;
		BlockMove(title+2, title+1, *title);
	}
	RemoveWindMenu(wind);
	SetWTitle(wind,title);
	AddWindMenu(wind);
}


/*	LoMarkSubject marks a single subject cell in a subject window read 
	or unread. Any open child window of the cell is also marked. If
	the cell is collapsed, all the articles in the thread are marked, as
	are any open children.
*/

static void LoMarkSubject (WindowPtr wind, Cell theCell, Boolean read,
	short *numMarked)
{
	WindowPtr child;
	TWindow **info;
	ListHandle theList;
	TSubject **subjectArray;
	short cellData, cellDataLen, nextInThread;

	*numMarked = 0;
	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	subjectArray = (**info).subjectArray;
	cellDataLen = 2;
	LGetCell(&cellData, &cellDataLen, theCell, theList);
	if ((*subjectArray)[cellData].read != read) {
		(*subjectArray)[cellData].read = read;
		(*numMarked)++;
	}
	child = FindChildByCellData(wind, cellData);
	if (child != nil) LoMarkArticle(child, read);
	if ((*subjectArray)[cellData].collapsed) {
		nextInThread = (*subjectArray)[cellData].nextInThread;
		while (nextInThread != -1) {
			if ((*subjectArray)[nextInThread].read != read) {
				(*subjectArray)[nextInThread].read = read;
				(*numMarked)++;
			}
			child = FindChildByCellData(wind, nextInThread);
			if (child != nil) LoMarkArticle(child, read);
			nextInThread = (*subjectArray)[nextInThread].nextInThread;
		}
	}
	(*subjectArray)[cellData].onlyRedrawCheck = true;
	LDraw(theCell, theList);
	(*subjectArray)[cellData].onlyRedrawCheck = false;
}


/*	LoMarkOneRead marks a single cross-referenced message in a specified group 
	as being read.
*/

static void LoMarkOneRead (TGroup *theGroup, long number)
{
	TUnread **unread,**newUnread,**prevUnread;
		
	for (unread = prevUnread = theGroup->unread;
		unread && !(number>=(**unread).firstUnread && 
			number<=(**unread).lastUnread);
		prevUnread = unread,unread = (**unread).next);
	
	if (!unread) return; /* already read */
	
	if (number == (**unread).firstUnread || number == (**unread).lastUnread) {
		if (number == (**unread).firstUnread) {
			(**unread).firstUnread++;
		} else {
			(**unread).lastUnread--;
		}
		if ((**unread).firstUnread > (**unread).lastUnread) {
			if (unread != prevUnread) {
				(**prevUnread).next = (**unread).next;
			} else {
				theGroup->unread = (**unread).next;
			}
			MyDisposHandle((Handle)unread);
		}
	} else {
		/* split this one in half */
		newUnread = (TUnread**)MyNewHandle(sizeof(TUnread));
		(**newUnread).next = (**unread).next;
		(**unread).next = newUnread;
		(**newUnread).lastUnread = (**unread).lastUnread;
		(**newUnread).firstUnread = number + 1;
		(**unread).lastUnread = number - 1;
	}
	theGroup->numUnread -= 1;
}


/*	LoMarkOneUnread marks a single cross-referenced message in a specified group 
	as being unread.
*/

static void LoMarkOneUnread (TGroup *theGroup, long number)
{
	TUnread **unread,**newUnread,**prevUnread;
		
	for (unread = prevUnread = theGroup->unread;
		unread && number>(**unread).lastUnread;
		prevUnread = unread,unread = (**unread).next);
	
	if (unread != nil && number>=(**unread).firstUnread) return; /* already unread */
	
	if (unread == nil) {
		/* insert at end of list, after prevUnread. */
		if (prevUnread != nil && (**prevUnread).lastUnread == number-1) {
			(**prevUnread).lastUnread++;
		} else {
			newUnread = (TUnread**)MyNewHandle(sizeof(TUnread));
			(**newUnread).firstUnread = (**newUnread).lastUnread = number;
			(**newUnread).next = nil;
			if (prevUnread == nil) {
				theGroup->unread = newUnread;
			} else {
				(**prevUnread).next = newUnread;
			}
		}
	} else if (unread == prevUnread) {
		/* insert at beginning of list, before unread. */
		if ((**unread).firstUnread == number+1) {
			(**unread).firstUnread--;
		} else {
			newUnread = (TUnread**)MyNewHandle(sizeof(TUnread));
			(**newUnread).firstUnread = (**newUnread).lastUnread = number;
			(**newUnread).next = unread;
			theGroup->unread = newUnread;
		}
	} else {
		/* insert in middle of list, between prevUnread and unread. */
		if ((**prevUnread).lastUnread == number-1 || 
			(**unread).firstUnread == number+1) 
		{
			if ((**prevUnread).lastUnread == number-1) {
				(**prevUnread).lastUnread++;
			} else {
				(**unread).firstUnread--;
			}
			if ((**prevUnread).lastUnread >= (**unread).firstUnread - 1) {
				/* join two blocks together */
				(**prevUnread).lastUnread = (**unread).lastUnread;
				(**prevUnread).next = (**unread).next;
				MyDisposHandle((Handle)unread);
			}
		} else {
			newUnread = (TUnread**)MyNewHandle(sizeof(TUnread));
			(**newUnread).firstUnread = (**newUnread).lastUnread = number;
			(**prevUnread).next = newUnread;
			(**newUnread).next = unread;
		}
	}
	theGroup->numUnread += 1;
}


/*	MarkOne marks a single cross-referenced message in a specified group as 
	being read or unread. */

static void MarkOne (char *groupName, long number, WindowPtr wind, 
	Boolean read)
{
	TWindow **info, **childInfo;
	TGroup theGroup, **groupArray;
	ListHandle parentList, childList;
	TSubject **subjectArray, *pSubject;
	Cell parentCell, childCell;
	short parentCellData, childIndex, cellDataLen, numCells, numSubjects;
	short threadHeadIndex;
	Handle strings;
	WindowPtr child, grandChild;
	
	info = (TWindow**)GetWRefCon(wind);
	parentList = (**info).theList;
	groupArray = (**info).groupArray;
	strings = (**info).strings;
	numCells = (**parentList).dataBounds.bottom;
	parentCell.h = 0;
	for (parentCell.v = 0; parentCell.v < numCells; parentCell.v++) {
		cellDataLen = 2;
		LGetCell(&parentCellData, &cellDataLen, parentCell, parentList);
		if (strcmp(*strings + (*groupArray)[parentCellData].nameOffset, 
			groupName) == 0) break;
	}
	if (parentCell.v >= numCells) return;
	child = FindChild(wind, parentCell);
	if (child != nil) {
		childInfo = (TWindow**)GetWRefCon(child);
		childList = (**childInfo).theList;
		subjectArray = (**childInfo).subjectArray;
		numSubjects = (**childInfo).numSubjects;
		pSubject = *subjectArray;
		for (childIndex = 0; childIndex < numSubjects; childIndex++) {
			if (pSubject->number == number) break;
			pSubject++;
		}
		if (childIndex >= numSubjects) return;
		threadHeadIndex = (*subjectArray)[childIndex].threadHeadIndex;
		if ((*subjectArray)[childIndex].read != read) {
			if (!FindParentCell(child, threadHeadIndex, &childCell)) return;
			(*subjectArray)[childIndex].read = read;
			(*subjectArray)[threadHeadIndex].onlyRedrawCheck = true;
			LDraw(childCell, childList);
			(*subjectArray)[threadHeadIndex].onlyRedrawCheck = false;
			(*groupArray)[parentCellData].numUnread += read ? -1 : 1;
			(*groupArray)[parentCellData].onlyRedrawCount = true;
			LDraw(parentCell, parentList);
			(*groupArray)[parentCellData].onlyRedrawCount = false;
			grandChild = FindChildByCellData(child, childIndex);
			if (grandChild != nil) LoMarkArticle(grandChild, read);
		}
	} else {
		theGroup = (*groupArray)[parentCellData];
		if (read) {
			LoMarkOneRead(&theGroup, number);
		} else {
			LoMarkOneUnread(&theGroup, number);
		}
		(*groupArray)[parentCellData] = theGroup;
		(*groupArray)[parentCellData].onlyRedrawCount = true;
		LDraw(parentCell, parentList);
		(*groupArray)[parentCellData].onlyRedrawCount = false;
	}
}


/*
	MarkXrefs marks articles which appear in multiple newsgroups as read
	or unread in all of the newsgroups.
*/

static void MarkXrefs (WindowPtr wind, Boolean read)
{
	TWindow **info,**parentInfo,**grandParentInfo;
	WindowPtr parent,grandParent;
	long offset,endHeader,endLine;
	Handle theText;
	char *current,*store;
	CStr255 xrefGroup, xrefNumber, mungeText;
	long number;
	
	info = (TWindow**)GetWRefCon(wind);
	if ((**info).kind != kArticle) return;
	parent = (**info).parentWindow;
	if (parent == nil) return;
	parentInfo = (TWindow**)GetWRefCon(parent);
	if ((**parentInfo).kind != kSubject) return;
	grandParent = (**parentInfo).parentWindow;
	if (grandParent == nil) return;
	grandParentInfo = (TWindow**)GetWRefCon(grandParent);
	if ((**grandParentInfo).kind != kUserGroup) return;
	
	theText = (**info).fullText;
	
	strcpy(mungeText,CRSTR);
	strcat(mungeText,"Xref:");

	offset = Munger(theText,0,mungeText,6,nil,0);
	if (offset < 0) {
		offset = Munger(theText,0,"Xref:",5,nil,0);
		if (offset != 0) offset = -1;
	}
	if (offset < 0) return;
	
	endHeader = Munger(theText,0,CRCR,2,nil,0);
	if (offset > endHeader) return;
	
	endLine = Munger(theText,offset+1,CRSTR,1,nil,0);
	
	HLock(theText);
	current = (*theText)+offset;
	
	/* skip over site name */
	
	current += 6;
	while (*current == ' ' && *current != CR)
		current++;
	while (*current != ' ' && *current != CR)
		current++;
	
	/* parse xrefed groups */
	
	while (current < (*theText)+endLine) {
		store = xrefGroup;
		while (*current == ' ' && *current != CR)
			current++;
		while (*current != ':' && *current != ' ' && *current != CR) {
			*store++ = *current++;
		}
		current++;
		*store = 0;
		
		store = xrefNumber;
		while (*current != ' ' && *current != ' ' && *current != CR) 
			*store++ = *current++;
		*store = 0;
		c2pstr(xrefNumber);
		StringToNum((StringPtr)xrefNumber,&number);
		MarkOne(xrefGroup,number,grandParent,read);
	}
	
	HUnlock(theText);
}


/*	MarkSelectedSubjects marks all selected subjects in a subject window 
	read or unread.
*/

static void MarkSelectedSubjects (WindowPtr wind, Boolean read)
{
	TWindow **info;
	Cell theCell;
	short totMarked = 0, numMarked;
	
	info = (TWindow**)GetWRefCon(wind);
	if ((**info).kind != kSubject) return;

	SetPt(&theCell, 0, 0);
	while (LGetSelect(true, &theCell, (**info).theList)) {
		LoMarkSubject(wind, theCell, read, &numMarked);
		theCell.v++;
		totMarked += numMarked;
	}
	LoMarkGroup(wind, read ? -totMarked : totMarked);
}


/*	MarkAllSubjects marks all subjects in a subject window read or unread.
*/

void MarkAllSubjects (WindowPtr wind, Boolean read)
{
	TWindow **info;
	Cell theCell;
	ListHandle theList;
	short numCells;
	short totMarked = 0, numMarked;
	
	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	numCells = (**theList).dataBounds.bottom;
	theCell.h = 0;
	for (theCell.v = 0; theCell.v < numCells; theCell.v++) {
		LoMarkSubject(wind, theCell, read, &numMarked);
		totMarked += numMarked;
	}
	LoMarkGroup(wind, read ? -totMarked : totMarked);
}


/*	MarkArticle marks an article window read or unread.
*/

void MarkArticle (WindowPtr wind, Boolean read)
{
	WindowPtr parent;
	TWindow **info,**parentInfo;
	Cell theCell;
	ListHandle parentList;
	TSubject **subjectArray;
	short index, threadHeadIndex;
	
	info = (TWindow**)GetWRefCon(wind);
	index = (**info).parentSubject;
	parent = (**info).parentWindow;
	parentInfo = (TWindow**)GetWRefCon(parent);
	subjectArray = (**parentInfo).subjectArray;
	parentList = (**parentInfo).theList;
		
	LoMarkArticle(wind, read);
	
	if ((*subjectArray)[index].read != read) {
		(*subjectArray)[index].read = read;
		threadHeadIndex = (*subjectArray)[index].threadHeadIndex;
		if (FindParentCell(parent, threadHeadIndex, &theCell)) {
			(*subjectArray)[threadHeadIndex].onlyRedrawCheck = true;
			LDraw(theCell, parentList);
			(*subjectArray)[threadHeadIndex].onlyRedrawCheck = false;
		}
		LoMarkGroup(parent, read ? -1 : +1);
	}
	
	MarkXrefs(wind, read);
}


/*	MarkSelectedGroups marks all selected groups in a group window read or 
	unread.
*/

static void MarkSelectedGroups (WindowPtr wind, Boolean read)
{
	WindowPtr child;
	TWindow **info;
	Cell theCell;
	short cellData, cellDataLen;
	TGroup **groupArray,theGroup;
	ListHandle theList;
	TUnread **pUnreadRec,**qUnreadRec;
	
	info = (TWindow**)GetWRefCon(wind);
	if ((**info).kind != kUserGroup) return;
	theList = (**info).theList;
	groupArray = (**info).groupArray;
		
	SetPt(&theCell,0,0);
	while (LGetSelect(true, &theCell, theList)) {
	
		cellDataLen = 2;
		LGetCell(&cellData, &cellDataLen, theCell, theList);
		
		theGroup = (*groupArray)[cellData];
		
		pUnreadRec = theGroup.unread;
		while (pUnreadRec) {
			qUnreadRec = (**pUnreadRec).next;
			MyDisposHandle((Handle)pUnreadRec);
			pUnreadRec = qUnreadRec;
		}
		if (read) {
			theGroup.unread = nil;
			theGroup.numUnread = 0;
		} else {
			pUnreadRec = (TUnread**)MyNewHandle(sizeof(TUnread));
			(**pUnreadRec).firstUnread = theGroup.firstMess;
			(**pUnreadRec).lastUnread = theGroup.lastMess;
			(**pUnreadRec).next = nil;
			theGroup.unread = pUnreadRec;
			theGroup.numUnread = theGroup.lastMess - theGroup.firstMess + 1;
		}
		
		(*groupArray)[cellData] = theGroup;
		
		(*groupArray)[cellData].onlyRedrawCount = true;
		LDraw(theCell, theList);
		(*groupArray)[cellData].onlyRedrawCount = false;

		child = FindChild(wind, theCell);
		if (child != nil) MarkAllSubjects(child, read);
		
		(**info).changed = true;
		
		theCell.v++;
	}
}


/*	MarkThread marks all the articles in a thread read or unread.
*/

void MarkThread (WindowPtr wind, short threadHeadIndex, Boolean read)
{
	TWindow **info;
	TSubject **subjectArray;
	WindowPtr child;
	short index, numMarked = 0;
	Boolean collapsed;
	Cell theCell;
	ListHandle theList;
	
	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	subjectArray = (**info).subjectArray;
	collapsed = (*subjectArray)[threadHeadIndex].collapsed;
	if (!FindParentCell(wind, threadHeadIndex, &theCell)) return;
	index = threadHeadIndex;
	while (index != -1) {
		if ((*subjectArray)[index].read != read) {
			(*subjectArray)[index].read = read;
			numMarked++;
			child = FindChildByCellData(wind, index);
			if (child != nil) LoMarkArticle(child, read);
			if (!collapsed) {
				(*subjectArray)[index].onlyRedrawCheck = true;
				LDraw(theCell, theList);
				(*subjectArray)[index].onlyRedrawCheck = false;
				theCell.v++;
			}
		}
		index = (*subjectArray)[index].nextInThread;
	}
	if (collapsed) {
		(*subjectArray)[threadHeadIndex].onlyRedrawCheck = true;
		LDraw(theCell, theList);
		(*subjectArray)[threadHeadIndex].onlyRedrawCheck = false;
	}

	LoMarkGroup(wind, read ? -numMarked : numMarked);
}


/*	DoMarkCommand handles the Mark Read and Mark Unread commands. */

void DoMarkCommand (WindowPtr wind, Boolean read)
{
	TWindow **info;
	
	info = (TWindow**)GetWRefCon(wind);

	switch ((**info).kind) {
		case kUserGroup:
			MarkSelectedGroups(wind, read);
			break;
		case kSubject:
			MarkSelectedSubjects(wind, read);
			break;
		case kArticle:
			MarkArticle(wind, read);
			break;
	}
}

