/*----------------------------------------------------------------------------

	subject.c

	This module handles the creation of subject windows.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "glob.h"
#include "child.h"
#include "header.h"
#include "kill.h"
#include "mark.h"
#include "nntp.h"
#include "open.h"
#include "qsort.h"
#include "resize.h"
#include "subject.h"
#include "util.h"
#include "dlgutil.h"


static Handle gSubjectStrings;	 /* handle to subject strings for sort comparison function */



/*----------------------------------------------------------------------------
	SubjectCompare 
	
	Compares two subject strings, ignoring any "Re:" prefixes.
	
	Entry:	aPtr = pointer to first subject string.
			bPtr = pointer to second subject string.
			
	Exit:	function result:
				< 0 if first string < second string.
				= 0 if first string == second string.
				> 0 if first string > second string.
----------------------------------------------------------------------------*/

static short SubjectCompare (char *aPtr, char *bPtr)
{	
	/* Skip spaces. */
	
	while (*aPtr == ' ') aPtr++;
	while (*bPtr == ' ') bPtr++;
		
	/* Remove reply indicators. */

	if (strncasecmp(aPtr, "Re:", 3) == 0) aPtr += 3;
	if (strncasecmp(bPtr, "Re:", 3) == 0) bPtr += 3;
	
	/* Skip spaces. */
	
	while (*aPtr == ' ') aPtr++;
	while (*bPtr == ' ') bPtr++;
	
	/* Compare strings. */
	
	return strcasecmp(aPtr,bPtr);
}



/*----------------------------------------------------------------------------
	GetSubjects
	
	Gets a range of subject headers from the news server and stores them in
	a subject array.
	
	Entry:	newsGroup = name of the group.
			first = first article number to get.
			last = last article number to get.
			subjectArray = handle to subject array.
			index = index in subject array to store first subject.
			strings = handle to strings block.
			*nextStringOffset = offset of next available location in
				strings block.
	
	Exit:	function result = true if no error, false if error.
			*numFetched = number of subjects fetched.
			*nextStringOffset updated.
	
	The caller must preallocate memory in the subject array for at least
	(last-first+1) new subjects.
	
	If the group does not exist the function result is true and 
	*numFetched=0.
----------------------------------------------------------------------------*/

static Boolean GetSubjects (char *newsGroup, long first, long last, 
	TSubject **subjectArray, short index,
	Handle strings, long *nextStringOffset,
	short *numFetched)
{
	short result;
	THeader **headers = nil; 
	short numHeaders;
	TSubject theSubject;
	THeader *pHeader, *pHeaderEnd;
	TSubject *pSubject;

	result = GetHeaders(newsGroup, "SUBJECT", first, last, strings, nextStringOffset,
		nil, 255, &headers, &numHeaders);
	if (result == 2) return false;
	if (result == 1) {
		*numFetched = 0;
		return true;
	}
	
	pHeaderEnd = *headers + numHeaders;
	pSubject = *subjectArray + index;
	for (pHeader = *headers; pHeader < pHeaderEnd; pHeader++, pSubject++, index++) {
		theSubject.subjectOffset = pHeader->offset;
		theSubject.authorOffset = -1;
		theSubject.number = pHeader->number;
		theSubject.myIndex = index;
		theSubject.collapsed = gPrefs.showThreadsCollapsed;
		theSubject.read = false;
		theSubject.drawTriangleFilled = false;
		theSubject.onlyRedrawTriangle = false;
		theSubject.onlyRedrawCheck = false;
		theSubject.highlight = 0;
		*pSubject = theSubject;
	}

	MyDisposHandle((Handle)headers);
	*numFetched = numHeaders;
	return true;
}



/*----------------------------------------------------------------------------
	GetAuthors
	
	Gets a range of author headers from the news server and stores them in
	a subject array.
	
	Entry:	newsGroup = name of the group.
			first = first article number to get.
			last = last article number to get.
			subjectArray = handle to subject array.
			index = starting index in subject array.
			num = number of subject records.
			strings = handle to strings block.
			*nextStringOffset = offset of next available location in
				strings block.
	
	Exit:	function result = true if no error, false if error.
			*nextStringOffset updated.
	
	GetSubjects must be called before GetAuthors to initialize the
	subject array elements.
----------------------------------------------------------------------------*/

static Boolean GetAuthors (char *newsGroup, long first, long last, 
	TSubject **subjectArray, short index, short num,
	Handle strings, long *nextStringOffset)
{
	short result;
	THeader **headers = nil; 
	short numHeaders;
	THeader *pHeader, *pHeaderEnd;
	TSubject *pSubject, *pSubjectEnd;

	result = GetHeaders(newsGroup, "From", first, last, strings, nextStringOffset,
		PrettifyName, 255, &headers, &numHeaders);
	if (result == 2) return false;
	if (result == 1) return true;
	
	pHeader = *headers;
	pHeaderEnd = *headers + numHeaders;
	pSubject = *subjectArray + index;
	pSubjectEnd = pSubject + num;
	while (pHeader < pHeaderEnd && pSubject < pSubjectEnd) {
		if (pHeader->number < pSubject->number) {
			pHeader++;
		} else if (pHeader->number > pSubject->number) {
			pSubject++;
		} else {
			pSubject->authorOffset = pHeader->offset;
			pHeader++;
			pSubject++;
		}
	}
	
	MyDisposHandle((Handle)headers);
	return true;
}



/*----------------------------------------------------------------------------
	GetSubjectsAndAuthorsFromNet gets unread subjects and authors 
	for a group from the NNTP server.
	
	Entry:	groupName = the group name.
			theGroup = pointer to the group record.
			kind = kind of group (kFullGroup, kNewGroup, or kUserGroup).
			
	Exit:	function result = true if subjects fetched, false if error.
			*subjectArray = handle to subject array, with the fields
				subjectOffset, authorOffset, number, read, and myIndex initialized.
			*numSubjects = number of subjects fetched.
			*subjectStrings = handle to subject strings.
			*firstFetched = article number of first article fetched.
			
	If the group does not exist, the function result is true and 
	*numSubjects = 0. In other words, a deleted group is treated the
	same as a group with no unread articles.
----------------------------------------------------------------------------*/

static Boolean GetSubjectsAndAuthorsFromNet (char *groupName, TGroup *theGroup, 
	EWindowKind kind, TSubject ***subjectArray, short *numSubjects, 
	Handle *subjectStrings, long *firstFetched)
{
	TSubject **array = nil;
	short index = 0;
	Handle strings = nil;
	OSErr err;
	long nextStringOffset = 0;
	TUnread **unread;
	long first, last, numToFetch, totNumToFetch;
	short numAllocated;
	short numFetched;
	
	array = (TSubject**)MyNewHandle(100*sizeof(TSubject));
	numAllocated = 100;
	strings = MyNewHandle(20000);
	
	if (kind == kUserGroup) {
		unread = theGroup->unread;
		totNumToFetch = theGroup->numUnread;
		first = (**unread).firstUnread;
		last = (**unread).lastUnread;
		if (totNumToFetch > gPrefs.maxFetch) {
			while (unread != nil) {
				totNumToFetch -= last - first + 1;
				if (totNumToFetch <= gPrefs.maxFetch) {
					first = last - (gPrefs.maxFetch - totNumToFetch) + 1;
					break;
				}
				unread = (**unread).next;
				if (unread != nil) {
					first = (**unread).firstUnread;
					last = (**unread).lastUnread;
				}
			}
		}
		*firstFetched = first;
		while (unread != nil) {
			numToFetch = last - first + 1;
			if (index + numToFetch > numAllocated) {
				numAllocated = index + numToFetch + 100;
				MySetHandleSize((Handle)array, numAllocated*sizeof(TSubject));
			}
			if (!GetSubjects(groupName, first, last, array, index, strings,
				&nextStringOffset, &numFetched)) goto exit;
			if (gPrefs.showAuthors)
				if (!GetAuthors(groupName, first, last, array, index, numFetched, strings,
					&nextStringOffset)) goto exit;
			index += numFetched;
			unread = (**unread).next;
			if (unread != nil) {
				first = (**unread).firstUnread;
				last = (**unread).lastUnread;
			}
		}
	} else {
		first = theGroup->firstMess;
		last = theGroup->lastMess;
		numToFetch = last - first + 1;
		if (numToFetch > gPrefs.maxFetch) {
			numToFetch = gPrefs.maxFetch;
			first = last - numToFetch + 1;
		}
		*firstFetched = first;
		if (numToFetch > numAllocated) {
			numAllocated = numToFetch;
			MySetHandleSize((Handle)array, numAllocated*sizeof(TSubject));
		}
		if (!GetSubjects(groupName, first, last, array, index, strings,
			&nextStringOffset, &numFetched)) goto exit;
		if (gPrefs.showAuthors)
			if (!GetAuthors(groupName, first, last, array, index, numFetched, strings,
				&nextStringOffset)) goto exit;
		index = numFetched;
	}
	
	MySetHandleSize((Handle)array, index*sizeof(TSubject));
	MySetHandleSize(strings, nextStringOffset);

	*subjectArray = array;
	*numSubjects = index;
	*subjectStrings = strings;
	return true;

exit:

	MyDisposHandle((Handle)array);
	MyDisposHandle(strings);
	return false;
}



/*----------------------------------------------------------------------------
	SortSubjectArrayCompare1 is the comparison function used to sort
	an array of pointers to subject records into increasing order by
	subject (ignoring any "Re:" prefixes).
	
	Entry:	p = pointer to pointer to TSubject record.
			q = pointer to pointer to TSubject record.
			gSubjectStrings = locked handle to subject strings.
			
	Exit:	function result:
				< 0 if first subject < second subject.
				= 0 if first subject == second subject.
				> 0 if first subject > second subject.
				
	Article numbers are used as a secondary sort key. 
----------------------------------------------------------------------------*/

static short SortSubjectArrayCompare1 (TSubject **p, TSubject **q)
{
	short result;
	static short Counter = 0;

	if ((++Counter & 0x1f) == 0) {
		GiveTime();
		Counter = 0;
	}
	
	result = SubjectCompare(*gSubjectStrings + (**p).subjectOffset, 
		*gSubjectStrings + (**q).subjectOffset);
	if (result != 0 ) return result;
	return (**p).number < (**q).number ? -1 : +1;
}



/*----------------------------------------------------------------------------
	SortSubjectArrayCompare2 is the comparison function used to sort
	an array of pointers to subject records into thread order.
	
	Entry:	p = pointer to pointer to TSubject record.
			q = pointer to pointer to TSubject record.
			
	Exit:	function result:
				< 0 if first subject < second subject.
				= 0 if first subject == second subject.
				> 0 if first subject > second subject.
----------------------------------------------------------------------------*/

static short SortSubjectArrayCompare2 (TSubject **p, TSubject **q)
{
	static short Counter = 0;

	if ((++Counter & 0x1f) == 0) {
		GiveTime();
		Counter = 0;
	}
	
	if ((**p).threadHeadNumber == (**q).threadHeadNumber)
		return (**p).number < (**q).number ? -1 : +1;
	return (**p).threadHeadNumber < (**q).threadHeadNumber ? -1 : +1;
}



/*----------------------------------------------------------------------------
	SortSubjectArray sorts a subject array into thread order. 

	The array itself is not sorted into thread order. Rather, an array of 
	pointers to the subject records is sorted into thread order. This array
	of pointers is then used to build the List Manager list for the subject
	window, at which point the array is discarded. There are two reasons 
	for doing this: 
	
	1. The subject array must be in increasing order by article number. 
	   This property is required by the function UpdateUnreadList in mark.c.
	   
	2. Sorting 4 byte pointers to the TSubject records is faster than
	   sorting the records themselves.
	
	Entry:	subjectArray = handle to locked subject array.
			subjectArrayPointers = preallocated nonrelocatable block
				of size 4 * numSubjects.
			numSubjects = number of subjects.
			subjectStrings = handle to subject strings.
			
	Exit:	function result = true if no error, false if error. 
			subjectArrayPointers = array of pointers to elements of
				subjectArray, sorted into thread order.
			fields threadOrdinal, threadLength, nextInThread, and
				threadHeadIndex initialized in subjectArray.
----------------------------------------------------------------------------*/

static Boolean SortSubjectArray (TSubject **subjectArray, 
	TSubject **subjectArrayPointers, short numSubjects, 
	Handle subjectStrings)
{
	TSubject *pSubject;
	TSubject **pPointer, **qPointer, **threadHeadPointer=nil;
	char *threadHeadSubject=0;
	long threadHeadNumber;
	Boolean newThread;
	short threadOrdinal=0;
	short i;
	short threadHeadIndex;
	Boolean result;
			
	/*	Initialize the pointer array. */
	
	pPointer = subjectArrayPointers;
	pSubject = *subjectArray;
	for (i = 0; i < numSubjects; i++) {
		*pPointer = pSubject;
		pPointer++;
		pSubject++;
	}
	
	/*	Sort the pointer array into increasing order by subject (ignoring
		any "Re:" prefixes). This brings threads together, although the
		threads are not in the right order yet. The article numbers are
		used as a secondary sort key to kep the articles within a thread
		in the correct order. */
		
	gSubjectStrings = subjectStrings;
	HLock(gSubjectStrings);
	result = FastQSort(subjectArrayPointers, numSubjects, sizeof(Ptr),
		(short(*)(void*,void*))SortSubjectArrayCompare1);
	HUnlock(gSubjectStrings);
	if (!result) return false;
	
	/*	Set the following fields in each element of the subject array:
	
		threadOrdinal = index of article in thread (1,2,3,...).
		threadLength = total number of articles in thread.
		nextInThread = index in subject array of next article in thread,
			or -1 if none.
		threadHeadIndex = index in subject array of first article in
			thread.
		threadHeadNumber = article number of first article in thread. */
	
	pPointer = subjectArrayPointers;
	for (i = 0; i < numSubjects; i++) {
		newThread = i == 0 ? true :
			SubjectCompare(*subjectStrings + (**pPointer).subjectOffset,
				threadHeadSubject) != 0;
		if (newThread) {
			if (i != 0) {
				for (qPointer = threadHeadPointer; qPointer < pPointer;
					qPointer++)
				{
					(**qPointer).threadLength = threadOrdinal;
					(**qPointer).nextInThread = (qPointer < pPointer-1) ? 
						(**(qPointer+1)).myIndex : -1;
				}
			}
			threadHeadPointer = pPointer;
			threadHeadNumber = (**pPointer).number;
			threadHeadSubject = *subjectStrings + (**pPointer).subjectOffset;
			threadOrdinal = 0;
			threadHeadIndex = (**pPointer).myIndex;
		}
		threadOrdinal++;
		(**pPointer).threadOrdinal = threadOrdinal;
		(**pPointer).threadHeadNumber = threadHeadNumber;
		(**pPointer).threadHeadIndex = threadHeadIndex;
		pPointer++;
	}
	for (qPointer = threadHeadPointer; qPointer < pPointer; qPointer++) {
		(**qPointer).threadLength = threadOrdinal;
		(**qPointer).nextInThread = (qPointer < pPointer-1) ? 
			(**(qPointer+1)).myIndex : -1;
	}
	
	/*	Sort the pointer array into final thread order. The primary
		sort key is threadHeadNumber. The secondary sort key is number.
		This final sort sorts the threads into proper chronological
		order, keeping the articles within the threads together. */
		
	return FastQSort(subjectArrayPointers, numSubjects, sizeof(Ptr),
		(short(*)(void*,void*))SortSubjectArrayCompare2);
}



/*----------------------------------------------------------------------------
	OpenGroupCell opens one subject window for a cell in a group 
	list window.
	
	Entry:	wind = pointer to group list window.
			theCell = the cell in the group list window to be opened.
			
	Exit:	function result = 
				0 if subject window opened.
				1 if no articles in group or group does not exist.
				2 if some other error.
----------------------------------------------------------------------------*/

short OpenGroupCell (WindowPtr wind, Cell theCell)
{
	WindowPtr child;
	TWindow **info, **childInfo;
	ListHandle theList, childList;
	TGroup **groupArray, theGroup;
	TSubject **subjectArray=nil;
	TSubject **subjectArrayPointers=nil, **p;
	short numSubjects = 0;
	Handle subjectStrings=nil;
	Handle strings;
	CStr255 groupName;
	short cellDataLen, cellData;
	EWindowKind kind;
	Point firstOffset;
	GrafPtr savePort;
	short *pCells;
	short *pCellArray;
	short offset;
	Cell childCell;
	OSErr err;
	short i;
	short numCells;
	char statusMsg[300];
	long firstFetched, lastFetched;
	TUnread **unread, **nextUnread;
	short result;
	
	/* Check to see if the subject window is already open. Bring it
	   to the front if it is. */

	if ((child = FindChild(wind, theCell)) != nil) {
		SelectWindow(child);
		return 0;
	}
	
	/* Initialize. */
	
	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	theList = (**info).theList;
	groupArray = (**info).groupArray;
	strings = (**info).strings;
	cellDataLen = 2;
	LGetCell(&cellData, &cellDataLen, theCell, theList);
	theGroup = (*groupArray)[cellData];
	strcpy(groupName, *strings + theGroup.nameOffset);
	if (kind == kUserGroup && theGroup.numUnread == 0) goto exit1;
	
	if (kind == kUserGroup) (**info).changed = true;
	
	/* Display the status window. */	
	
	strcpy(statusMsg, gPrefs.showAuthors ?
		"Getting subjects and authors: " :
		"Getting subjects: ");
	strcat(statusMsg, groupName);
	statusMsg[255] = 0;
	StatusWindow(statusMsg);
	
	/* If we are opening a cell in the full group list window or the new groups
	   window, first call GetGroupArticleRange to get the [low,high] article range
	   for the group. */

	if (kind != kUserGroup) {
		result = GetGroupArticleRange(&theGroup);
		if (result == 1) goto exit1;
		if (result == 2) goto exit2;
		if (theGroup.numUnread == 0) goto exit1;
	}
	
	/* Get the subjects and authors from the server. */
	
	if (!GetSubjectsAndAuthorsFromNet(groupName, &theGroup, kind,
		&subjectArray, &numSubjects, &subjectStrings, &firstFetched)) goto exit2;
	if (numSubjects == 0) goto exit1;
	
	/* Apply filters. */

	#ifdef FILTERS
		if (kind == kUserGroup) {
			KillArticles(groupName, subjectArray, &numSubjects, subjectStrings);
		}
		if (numSubjects == 0) goto exit1;
	#endif
	
	/* Lock the subject array. */

	HLock((Handle)subjectArray);

	/* Sort the list into thread order. */
	
	subjectArrayPointers = (TSubject**)MyNewPtr(numSubjects * sizeof(Ptr));
	if (!SortSubjectArray(subjectArray, subjectArrayPointers,
		numSubjects, subjectStrings)) goto exit2;
	
	/* Create the subject window. */
	
	SetPt(&firstOffset, 0, 0);
	GetPort(&savePort);
	SetPort(wind);
	LocalToGlobal(&firstOffset);
	SetPort(savePort);
	
	c2pstr(groupName);
	child = MakeNewWindow(kSubject, firstOffset, (StringPtr)groupName);
	childInfo = (TWindow**)GetWRefCon(child);
	
	(**childInfo).subjectArray = subjectArray;
	(**childInfo).numSubjects = numSubjects;
	(**childInfo).firstFetched = firstFetched;
	(**childInfo).strings = subjectStrings;
	(**childInfo).parentWindow = wind;
	(**childInfo).parentGroup = cellData;
	(**childInfo).groupNameOffset = theGroup.nameOffset;

	/* Expand threads with highlighted articles if desired */

	#ifdef FILTERS
		if (gPrefs.showThreadsCollapsed && gPrefs.expandHilited) {
			TSubject **threadStart, **q, **end;
			threadStart = subjectArrayPointers;
			end = subjectArrayPointers + numSubjects;
			for (p = subjectArrayPointers; p < end; p++) {
				if ((**p).threadOrdinal == 1)
					threadStart = p;
				if ((**p).collapsed && (**p).highlight) {
					short thread = (**threadStart).threadHeadIndex;
					for (q = threadStart; q < end && (**q).threadHeadIndex == thread;
						q++) {
						(**q).collapsed = false;
						p = q;
					}
				}
			}
		}
	#endif

	/* Create the List Manager cell list. */
	
	childList = (**childInfo).theList;
	p = subjectArrayPointers;
	numCells = numSubjects;
	for (i = 0; i < numSubjects; i++) {
		if ((**p).collapsed && (**p).threadOrdinal != 1) numCells--;
		p++;
	}
	
	LDoDraw(false, childList);
	LAddRow(numCells, 0, childList);
	MySetHandleSize((**childList).cells, 2*numCells);
	pCells = (short*)*((**childList).cells);
	pCellArray = (**childList).cellArray;
	offset = 0;

	p = subjectArrayPointers;
	for (i = 0; i < numSubjects; i++) {
		if (!(**p).collapsed || (**p).threadOrdinal == 1) {
			*pCellArray++ = offset;
			*pCells++ = (**p).myIndex;
			offset += 2;
		}
		p++;
	}
	*pCellArray = offset;
	LDoDraw(true,childList);
	
	MyDisposPtr((Ptr)subjectArrayPointers);
	
	/* Unlock the subject array. */
	
	HUnlock((Handle)subjectArray);
	
	/* Finish initializing the new subject window. */

	AddChild(wind, child);
	SetPt(&childCell, 0, 0);
	LSetSelect(true, childCell, childList);
	DoZoom(child, inZoomOut);
	
	/* For a user group list, update the unread list and the number
	   of unread articles counter. */
	
	if (kind == kUserGroup) {
		UpdateUnreadList(child);
		(*groupArray)[cellData].onlyRedrawCount = true;
		LDraw(theCell, theList);
		(*groupArray)[cellData].onlyRedrawCount = false;
	}
	
	/* Show the new subject window. */
	
	ShowWindow(child);
		
	return 0;
	
exit1:

	MyDisposHandle((Handle)subjectArray);
	MyDisposHandle(subjectStrings);
	MyDisposPtr((Ptr)subjectArrayPointers);
	if (kind == kUserGroup) {
		unread = theGroup.unread;
		while (unread != nil) {
			nextUnread = (**unread).next;
			MyDisposHandle((Handle)unread);
			unread = nextUnread;
		}
		theGroup.unread = nil;
		theGroup.numUnread = 0;
		(*groupArray)[cellData] = theGroup;
		(*groupArray)[cellData].onlyRedrawCount = true;
		LDraw(theCell, theList);
		(*groupArray)[cellData].onlyRedrawCount = false;
	}
	return 1;
	
exit2:

	MyDisposHandle((Handle)subjectArray);
	MyDisposHandle(subjectStrings);
	MyDisposPtr((Ptr)subjectArrayPointers);
	return 2;
}
