/*----------------------------------------------------------------------------

	newart.c

	This module handles checking for new articles.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>

#include "glob.h"
#include "newart.h"
#include "nntp.h"
#include "mark.h"
#include "util.h"
#include "close.h"



/*----------------------------------------------------------------------------
	DoCheckNewArticles 
	
	Checks to see if there are any new articles on the server for all of the 
	groups in a user group list.

	Entry:	wind = pointer to user group list window.
----------------------------------------------------------------------------*/
 
void DoCheckNewArticles(WindowPtr wind)
{
	TWindow **info;
	ListHandle theList;
	TGroup **groupArray;
	short numGroups, numCells, cellDataLen, groupIndex;
	Cell theCell;
	TGroup theGroup;
	long numUnread;
	Boolean haveSelectedGroup;
	TUnread **cur, **prev;
	
	StatusWindow("Checking for new articles.");
	
	info = (TWindow**)GetWRefCon(wind);
	groupArray = (**info).groupArray;
	numGroups = (**info).numGroups;
	theList = (**info).theList;
	numCells = (**theList).dataBounds.bottom;
	theCell.h = 0;
	
	/* Close all child windows. */
	
	while ((**info).childList != nil) DoCloseWindow((**(**info).childList).childWindow);
	
	/* Add [lastMess+1, maxlong] to the end of each group unread list. 
	   Mark each group in the list for an article range update. */
	
	for (theCell.v = 0; theCell.v < numCells; theCell.v++) {
		cellDataLen = 2;
		LGetCell(&groupIndex, &cellDataLen, theCell, theList);
		theGroup = (*groupArray)[groupIndex];
		numUnread = theGroup.numUnread;
		AppendUnreadRange(theGroup.lastMess+1, 0x7fffffff, &theGroup);
		theGroup.numUnread = numUnread;
		theGroup.status = 'x';
		(*groupArray)[groupIndex] = theGroup;
	}
	
	/* Get new group article ranges from server. */

	GetGroupArrayArticleRanges(groupArray, numGroups);
	
	/* Adjust unread lists and redraw unread article counts. Select the first
	   group with unread articles, if any. */
	
	haveSelectedGroup = false;
	for (theCell.v = 0; theCell.v < numCells; theCell.v++) {
		cellDataLen = 2;
		LGetCell(&groupIndex, &cellDataLen, theCell, theList);
		theGroup = (*groupArray)[groupIndex];
		if (theGroup.status == 'x') {
			AdjustUnreadList(&theGroup);
			theGroup.onlyRedrawCount = true;
			(*groupArray)[groupIndex] = theGroup;
			LDraw(theCell, theList);
			(*groupArray)[groupIndex].onlyRedrawCount = false;
			if (haveSelectedGroup || theGroup.numUnread == 0) {
				LSetSelect(false, theCell, theList);
			} else {
				LSetSelect(true, theCell, theList);
				haveSelectedGroup = true;
			}
		} else {
			theGroup.firstMess = 1;
			theGroup.lastMess = 0;
			theGroup.numUnread = 0;
			cur = theGroup.unread;
			while (cur != nil) {
				prev = cur;
				cur = (**cur).next;
				MyDisposHandle((Handle)prev);
			}
			theGroup.onlyRedrawCount = true;
			(*groupArray)[groupIndex] = theGroup;
			LDraw(theCell, theList);
			(*groupArray)[groupIndex].onlyRedrawCount = false;
		}
	}
	if (haveSelectedGroup) LAutoScroll(theList);
}

