/*----------------------------------------------------------------------------

	key.c

	This module handles key down events.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <packages.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "glob.h"
#include "close.h"
#include "cmd.h"
#include "collapse.h"
#include "key.h"
#include "mark.h"
#include "next.h"
#include "scroll.h"
#include "util.h"
#include "wind.h"
#include "subject.h"
#include "article.h"
#include "dlgutil.h"


/*	CompareGroups is used by HandleListKey below to compare a typed keyboard
	navigation string to a group name. Periods in the keyboard navigation string
	match everything up to and including the next period in the group name. Thus,
	for example, "c.s.m.a" matches "comp.sys.mac.announce".
*/

static short CompareGroups (char *a, char *b)
{
	char aup, bup;

	while (*a != 0) {
		if (*a == '.') {
			while (*b != 0 && *b != '.') b++;
			if (*b == 0) {
				return 1;
			} else {
				b++;
			}
		} else {
			aup = toupper(*a);
			bup = toupper(*b);
			if (aup < bup) return -1;
			if (aup > bup) return 1;
			b++;
		}
		a++;
	}
	return 0;
}



/*	DoListArrow handles the up and down arrow keys in list windows. */

static void DoListArrow (WindowPtr wind, Boolean up)
{
	TWindow **info;
	ListHandle theList;
	Cell tmpCell;

	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	SetPt(&tmpCell,0,0);
	while (LGetSelect(true,&tmpCell,theList)) {
		LSetSelect(false,tmpCell,theList);
		tmpCell.v++;
	}
	if (up) {
		tmpCell.v -= 2;
		if (tmpCell.v < 0) tmpCell.v = 0;
	} else { /* down */
		if (tmpCell.v >= (**theList).dataBounds.bottom) tmpCell.v--;
	}
	LSetSelect(true,tmpCell,theList);
	LAutoScroll(theList);
}		


/*	OpenSelectedCells opens all selected items in a list. The items are opened
	in reverse order, so that the window for the earliest one appears on top.
*/

void OpenSelectedCells (WindowPtr wind)
{
	Cell theCell;
	TWindow **theInfo;
	ListHandle theList;
	Boolean error=false;
	EWindowKind kind;
	short *p,*pBegin,result,numSelected=0,numOpened=0;
	CStr255 msg;
	
	theInfo = (TWindow**) GetWRefCon(wind);
	theList = (**theInfo).theList;
	kind = (**theInfo).kind;
	
	SetPt(&theCell,0,(**theList).dataBounds.bottom-1);
	while (theCell.v >= 0) {
		pBegin = (**theList).cellArray;
		p = pBegin + theCell.v;
		while (*p >= 0 && p >= pBegin) p--;
		theCell.v = p - pBegin;
		if (p >= pBegin) {
			switch (kind) {
				case kFullGroup:
				case kNewGroup:
				case kUserGroup:
					result = OpenGroupCell(wind, theCell);
					break;
				case kSubject:
					result = OpenSubjectCell(wind, theCell, 1);
					break;
			}
			numSelected++;
			if (result == 2) return;
			if (result == 0) numOpened++;
		}
		theCell.v--;
	}
	if (numOpened < numSelected) {
		if (kind == kSubject) {
			if (numSelected == 1) {
				strcpy(msg,"The selected article could not be opened");
				strcat(msg,", because it no longer exists on the news server.");
			} else {
				if (numOpened == 0) {
					strcpy(msg,"None of the selected articles could be opened");
				} else {
					strcpy(msg,"Some of the selected articles could not be opened");
				}
				strcat(msg,", because they no longer exist on the news server.");
			}
		} else {
			if (numSelected == 1) {
				strcpy(msg,"The selected group was not opened");
				if (kind == kUserGroup) {
					strcat(msg,", because it contains no unread articles.");
				} else {
					strcat(msg,", because it contains no articles.");
				}
			} else {
				if (numOpened == 0) {
					strcpy(msg,"None of the selected groups were opened");
					if (kind == kUserGroup) {
						strcat(msg,", because they contain no unread articles.");
					} else {
						strcat(msg,", because they contain no articles.");
					}
				} else {
					*msg = 0;
				}
			}
		}
		if (*msg != 0) ErrorMessage(msg);
	}
}


/*	HandleListKey handles keydown events in list manager windows.
*/

static void HandleListKey (WindowPtr wind, char theChar, short modifiers)
{
	ListHandle theList;
	static long lastDown = 0;
	static short numChars = 0;
	static CStr255 searchStr;
	Cell theCell,tmpCell;
	TWindow **info;
	Boolean isSorted;
	short cellData,cellDataLen;
	TGroup **groupArray;
	char *theString;
	short low,high,compare;
	Handle strings;
	EWindowKind kind;
	
	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	theList = (**info).theList;

	switch (theChar) {
		case returnKey:
		case enterKey:
			OpenSelectedCells(wind);
			return;
		case upArrow:
			DoListArrow(wind, true);
			break;
		case downArrow:
			DoListArrow(wind, false);
			break;
		case rightArrow:
		case leftArrow:
			if (kind == kSubject && (modifiers & cmdKey))
				ExpandCollapseKey(wind, theChar);
			break;
		default:
			if (kind == kSubject) return;
			if (modifiers & cmdKey) return;
			SetPt(&theCell,0,0);
			if ((TickCount()-lastDown) > (4*GetDblTime()))
				numChars = 0;
			lastDown = TickCount();
			searchStr[numChars++] = theChar;
			searchStr[numChars] = 0;
			isSorted = (kind == kFullGroup) || (kind == kNewGroup);
			strings = (**info).strings;
			HLock(strings);
			if (isSorted) {
				groupArray = (**info).groupArray;
				low = 0;
				high = (**theList).dataBounds.bottom - 1;
				while (low < high) {
					theCell.v = (low + high) >> 1;
					cellDataLen = 2;
					LGetCell(&cellData, &cellDataLen, theCell, theList);
					theString = *strings + (*groupArray)[cellData].nameOffset;
					compare = CompareGroups(searchStr, theString);
					if (compare < 0) {
						high = theCell.v - 1;
					} else if (compare == 0) {
						if (low == theCell.v) {
							high = low;
						} else {
							high = theCell.v - 1;
						}
					} else {
						low = theCell.v + 1;
					}
				}
				if (low != high) break;
				theCell.v = low;
				cellDataLen = 2;
				LGetCell(&cellData, &cellDataLen, theCell, theList);
				theString = *strings + (*groupArray)[cellData].nameOffset;
				compare = CompareGroups(searchStr, theString);
				if (compare != 0) {
					theCell.v++;
					if (theCell.v >= (**theList).dataBounds.bottom) break;
					cellDataLen = 2;
					LGetCell(&cellData, &cellDataLen, theCell, theList);
					theString = *strings + (*groupArray)[cellData].nameOffset;
					compare = CompareGroups(searchStr, theString);
					if (compare != 0) break;
				}
				SetPt(&tmpCell,0,0);
				while (LGetSelect(true,&tmpCell,theList)) {
					LSetSelect(false,tmpCell,theList);
					tmpCell.v++;
				}
				LSetSelect(true,theCell,theList);
				LAutoScroll(theList);
			} else {
				groupArray = (**info).groupArray;
				while (theCell.v < (**theList).dataBounds.bottom) {
					cellDataLen = 2;
					LGetCell(&cellData, &cellDataLen, theCell, theList);
					theString = *strings + (*groupArray)[cellData].nameOffset;
					compare = CompareGroups(searchStr, theString);
					if (compare == 0) {
						SetPt(&tmpCell,0,0);
						while (LGetSelect(true,&tmpCell,theList)) {
							LSetSelect(false,tmpCell,theList);
							tmpCell.v++;
						}
						LSetSelect(true,theCell,theList);
						LAutoScroll(theList);
						break;
					}
					theCell.v++;
				}
			}
			HUnlock(strings);
			break;
	}
}


/*	DoPagingCommand handles the paging commands: 
	
	Entry:	wind = pointer to window to be paged.
			whichCmd =
				0: line up.
				1: line down. 
				2: page up.
				3: page down.
				4: home.
				5: end.
				6: next section.
				7: previous section.
*/

static void DoPagingCommand (WindowPtr wind, short whichCmd)
{
	TWindow **info;
	ListHandle theList;
	short kind, height, numCells;
	
	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	
	if (kind == kFullGroup || kind == kNewGroup || kind == kUserGroup ||
		kind == kSubject) 
	{
		theList = (**info).theList;
		height = (**theList).visible.bottom - (**theList).visible.top;
		numCells = (**theList).dataBounds.bottom;
		switch (whichCmd) {
			case 0:
				DoListArrow(wind, true);
				break;
			case 1:
				DoListArrow(wind, false);
				break;
			case 2:
				LScroll(0, -(height - 1), theList);
				break;
			case 3:
				LScroll(0, height - 1, theList);
				break;
			case 4:
				LScroll(0, -numCells, theList);
				break;
			case 5:
				LScroll(0, numCells, theList);
				break;
		}
	} else { /* text window */
		switch (whichCmd) {
			case 0:
				ScrollTextLineUp(wind);
				break;
			case 1:
				ScrollTextLineDown(wind);
				break;
			case 2:
				ScrollTextPageUp(wind);
				break;
			case 3:
				ScrollTextPageDown(wind);
				break;
			case 4:
				ScrollTextHome(wind);
				break;
			case 5:
				ScrollTextEnd(wind);
				break;
			case 6:
				ScrollTextRight(wind);
				break;
			case 7:
				ScrollTextLeft(wind);
				break;
		}
	}
}


/*	HandleKeypadKey handles numeric keypad command shortcuts, plus the four
	paging keys on the extended keyboard. */

static Boolean HandleKeypadKey (WindowPtr wind, EWindowKind kind, EventRecord *ev)
{
	unsigned short theChar;
	unsigned short keyCode;
	Boolean isListWind, isListOrArticleWind, isArticleWind, x;
	
	theChar = ev->message & charCodeMask;
	keyCode = (ev->message & keyCodeMask) >> 8;
	
	isListWind = kind == kFullGroup || kind == kNewGroup ||
		kind == kUserGroup || kind == kSubject;
	isListOrArticleWind = isListWind || kind == kArticle;
	isArticleWind = kind == kArticle || kind == kMiscArticle;
	x = gPrefs.keypadShortcuts;
	
	if (theChar == '0' && keyCode == 0x52 && isListOrArticleWind && x) {
		DoNextArticle(wind, false);
	} else if (theChar == '.' && keyCode == 0x41 && isListOrArticleWind && x) {
		DoNextThread(wind);
	} else if (theChar == enterKey && keyCode == 0x4c && isListOrArticleWind && x) {
		DoNextGroup(wind);
	} else if (theChar == '*' && (keyCode == 0x42 || keyCode == 0x43) && x) {
		if (kind == kFullGroup) {
			ShowHideGroups();
		} else {
			DoCloseWindow(wind);
		}
	} else if (theChar == '+' && (keyCode == 0x46 || keyCode == 0x45) && 
		isListOrArticleWind && x) {
		DoMarkCommand(wind, true);
	} else if (theChar == '-' && keyCode == 0x4e && isListOrArticleWind && x) {
		DoMarkCommand(wind, false);
	} else if (theChar == '=' && (keyCode == 0x48 || keyCode == 0x51) && x) {
		DoSelectAll(wind);
	} else if (theChar == '/' && (keyCode == 0x4d || keyCode == 0x4b) &&
		kind == kSubject && x) {
		DoExpandCollapseSelectedThread(wind);
	} else if (theChar == '8' && keyCode == 0x5b && x) {
		DoPagingCommand(wind, 0);
	} else if (theChar == '2' && keyCode == 0x54 && x) {
		DoPagingCommand(wind, 1);
	} else if (theChar == pageUpKey || (theChar == '9' && keyCode == 0x5c)) {
		DoPagingCommand(wind, 2);
	} else if (theChar == pageDownKey || (theChar == '3' && keyCode == 0x55)) {
		DoPagingCommand(wind, 3);
	} else if (theChar == homeKey || (theChar == '7' && keyCode == 0x59)) {
		DoPagingCommand(wind, 4);
	} else if (theChar == endKey || (theChar == '1' && keyCode == 0x53)) {
		DoPagingCommand(wind, 5);
	} else if (theChar == '6' && keyCode == 0x58 && isArticleWind && x) {
		DoPagingCommand(wind, 6);
	} else if (theChar == '4' && keyCode == 0x56 && isArticleWind && x) {
		DoPagingCommand(wind, 7);
	} else {
		return false;
	}
	
	return true;
}


/* DoForwardDelete handles the forward delete key. */

static void DoForwardDelete (WindowPtr wind)
{
	TWindow **info;
	TEHandle theTE;
	short selStart, selEnd, teLength;
	
	info = (TWindow**)GetWRefCon(wind);
	theTE = (**info).theTE;
	selStart = (**theTE).selStart;
	selEnd = (**theTE).selEnd;
	teLength = (**theTE).teLength;
	if (selStart == selEnd && selEnd < teLength) (**theTE).selEnd = selEnd + 1;
	TEDelete(theTE);
	AdjustScrollBar(wind);
	CheckInsertion(wind);
}


/*	HandleKeyDown handles key down events */

void HandleKeyDown (WindowPtr wind, EventRecord *ev)
{
	unsigned short theChar;
	TWindow **info;
	EWindowKind kind;
	long menucmd = 0;
	
	theChar = ev->message & charCodeMask;
	if ((ev->modifiers & cmdKey) != 0) {
		menucmd = MenuKey(theChar);
		if (HiWord(menucmd))
			DoCommand(MenuKey(theChar));
		else
			menucmd = 0;
	}
	if (!menucmd && IsAppWindow(wind)) {
		ObscureCursor();
		info = (TWindow**)GetWRefCon(wind);
		kind = (**info).kind;
		if (HandleKeypadKey(wind, kind, ev)) return;
		switch (kind) {
			case kFullGroup:
			case kNewGroup:
			case kUserGroup:
			case kSubject:
				HandleListKey(wind, theChar, ev->modifiers);
				break;
			case kArticle:
			case kMiscArticle:
				if (theChar == leftArrow || theChar == rightArrow ||
					theChar == upArrow || theChar == downArrow) {
					TEKey(theChar,(**info).theTE);
					CheckInsertion(wind);
				}
				break;
			case kPostMessage:
			case kMailMessage:
				if (theChar == forwardDelKey) {
					DoForwardDelete(wind);
				} else if (!(ev->modifiers & cmdKey)) {
					TEKey(theChar,(**info).theTE);
					AdjustScrollBar(wind);
					CheckInsertion(wind);
				}
				break;
		}
	}
}

