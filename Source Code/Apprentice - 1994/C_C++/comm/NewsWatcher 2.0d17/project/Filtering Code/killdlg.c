/*----------------------------------------------------------
#
#	NewsWatcher	- Macintosh NNTP Client Application
#
#-----------------------------------------------------------
#
#	killdlg.c
#
#	This module handles the dialogs for editing the killfile
#	for a group.
#
#	John Werner (werner@soe.berkeley.edu)
#	May, 1993
#
#-----------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "dlgutil.h"
#include "glob.h"
#include "header.h"
#include "killdata.h"
#include "killdlg.h"
#include "menus.h"
#include "popup.h"
#include "strutil.h"
#include "util.h"


static Boolean gFirstCall;

/* Utility functions */
static OSErr AddKillEntry(TGroupKillHdl, TKillEntry *, short index, ListHandle theList);

/* Dialog IDs */
#define kGroupKillDlg	150
#define kKillEntryDlg	151

/*===================================================================================
 * GroupKillDialog and friends
 * These functions handle the dialog box for editing a group's killfile entries.
 *
 * The dialog has a list box that shows all the entries and New, Edit, and Delete
 * buttons that can be used to change them.  Items in the list can also
 * be reordered by dragging.
 *
 * First come lots of constants and data types....
 */

/*
 * The data associated with the dialog box.
 * A handle to this is stored in the refCon field.
 */
typedef struct {
	ListHandle		entryList;		/* List to display the entries */
	TGroupKillHdl	kill;			/* Handle to the actual entries */
	short			numEntries;		/* # of them */
} TGroupKillDlg;

static TGroupKillDlg gGroupDlgData;

/* Prototypes for utility functions */
Boolean			EditKillEntry(TKillEntry *);
void	  		KillEntryToList(TKillEntry *, short index, ListHandle aList);

/* Dialog item and filter functions */
static pascal void		KillListItem(WindowPtr dlg, short theItem);
static pascal Boolean	KillListClikLoop();
static pascal Boolean	GroupKillFilter(DialogPtr, EventRecord *, short *);

#define	kKillList		3			/* Listbox showing all the killfile entries */
#define	kListEdit		4			/* Pushbuttons for editing list */
#define	kListDelete		5
#define	kListNew		6
#define	kNoMatchKeep	7			/* What to do for articles that don't match */
#define kNoMatchKill	8
#define kGroupName		9			/* static text item displaying the group name */

#define kKillListDblClick	20		/* Fake item ID for double-click in list */

#define kKillDlgStrings		1002	/* String list for the kill dialogs */
#define		kKillListStr		1

/*
 * The text style used in the list box.
 * It could go in TGroupKillDlg, but it's easier to make it global
 */
static TextStyle gKillListStyle;

/*--------------------------------------------------------------------------------
 * GroupKillDialog
 * This is the main function for newsgroup killfile editing.
 *
 * The group in question is determined from 'wind'; if it's a group window
 * that group is used; if it's an article or subject window we look up the
 * tree of parent windows for a group window.
 */
void GroupKillDialog(WindowPtr wind)
{
	OSErr			err = noErr;
	DialogPtr		dlg = nil;
	short			itemHit, selIndex, dummy;
	Boolean			finished, anySelected;
	CStr255			groupName;
	Cell			theCell;
	TGroupKillHdl	kill = NULL;
	TextStyle		saveStyle;

	/* Figure out which group we're dealing with */
	FindGroupName(wind, groupName);
	if (!*groupName)
		return;

	/*
	 * Find the current killfile entry for this group and make a copy to edit.
	 * If there isn't one already, make a new one.
	 */
	kill = FindGroupKill(groupName, &dummy);
	if (kill) {
		kill = CloneGroupKill(kill);
	} else {
		kill = NewGroupKill(groupName);
	}
	FailNIL(kill, cleanup);
	
	/* Initialize the TextStyle font we're going to use in the list box */
	gKillListStyle.tsFont = applFont;
	gKillListStyle.tsFace = 0;
	gKillListStyle.tsSize = 12;
	gKillListStyle.tsColor.red = gKillListStyle.tsColor.green =
		gKillListStyle.tsColor.blue = 0;

	dlg = MyGetNewDialog(kGroupKillDlg);
	SetPort(dlg);

	gGroupDlgData.entryList = nil;
	gGroupDlgData.kill = kill;
	gGroupDlgData.numEntries = (**kill).numEntries;

	GetPortTextStyle(&saveStyle);

	/* Initialize the two radio buttons */
	DlgSetCheck(dlg, kNoMatchKill, (**kill).noMatchKill);
	DlgSetCheck(dlg, kNoMatchKeep, !(**kill).noMatchKill);

	/* And the user item for the group name */
	DlgSetCString(dlg, kGroupName, groupName);

	/* Create and initialize the list for displaying kill entries */
	{
		Handle itemHandle;
		Rect itemRect, cellsRect;
		short iType, i, numEntries;
		Point cellSize;

		GetDItem(dlg, kKillList, &iType, &itemHandle, &itemRect);
		SetDItem(dlg, kKillList, iType, (Handle)KillListItem, &itemRect);
		itemRect.right -= 15;	/* Room for scrollbar */
		SetRect(&cellsRect, 0, 0, 1, (**kill).numEntries);
		SetPt(&cellSize, 0, 0);
		SetPortTextStyle(&gKillListStyle);
		gGroupDlgData.entryList = LNew(&itemRect, &cellsRect, cellSize, 0,
								dlg, FALSE, FALSE, FALSE, TRUE);
		FailNIL(gGroupDlgData.entryList, cleanup);

		(**gGroupDlgData.entryList).selFlags = lOnlyOne;
		(**gGroupDlgData.entryList).lClikLoop = KillListClikLoop;

		/* Stuff the kill entries into the list */
		HLock((Handle)(**kill).entries);
		numEntries = (**kill).numEntries;
		for (i = 0; i < numEntries; i++) {
			KillEntryToList(*(**kill).entries + i, i, gGroupDlgData.entryList);
		}
		HUnlock((Handle)(**kill).entries);
		LDoDraw(TRUE, gGroupDlgData.entryList);
	}
	
	ShowWindow(dlg);

	do {
		/* See if any cell in the entry list are selected */
		SetPt(&theCell, 0, 0);
		anySelected = LGetSelect(TRUE, &theCell, gGroupDlgData.entryList);
		selIndex = theCell.v;

		/* Enable/Disable buttons based on list */
		DlgEnableItem(dlg, kListEdit, anySelected != 0);
		DlgEnableItem(dlg, kListDelete, anySelected != 0);

		SetPortTextStyle(&saveStyle);
		MyModalDialog(GroupKillFilter, &itemHit, TRUE, TRUE);

		switch (itemHit) {
		  case ok:
		  case cancel:
		  	finished = TRUE;
		  	break;

		  case kNoMatchKill:
		  case kNoMatchKeep:
		  	/* Radio buttons to toggle default action for articles */
		  	(**kill).noMatchKill = (itemHit == kNoMatchKill);
			DlgSetCheck(dlg, kNoMatchKill, (**kill).noMatchKill);
			DlgSetCheck(dlg, kNoMatchKeep, !(**kill).noMatchKill);
		  	break;

		  case kKillListDblClick:	/* Double-click on list item */
		  case kListEdit:			/* or Edit button */
		  	{						/* Edit this killfile entry */
		  		HLockHi((Handle)(**kill).entries);
			  	if (EditKillEntry(*(**kill).entries + selIndex)) {
			  		KillEntryToList(*(**kill).entries + selIndex, selIndex,
			  						gGroupDlgData.entryList);
			  	}
			  	HUnlock((Handle)(**kill).entries);
		  	}
		  	break;

		  case kListDelete:			/* Delete item (or Clear key) */
		  	{
				SetPortTextStyle(&gKillListStyle);
		  		LDelRow(1, selIndex, gGroupDlgData.entryList);

		  		HLock((Handle)(**kill).entries);
		  		BlockMove(*(**kill).entries + selIndex + 1,
		  					*(**kill).entries + selIndex,
		  					((**kill).numEntries - selIndex - 1) * sizeof(TKillEntry));
		  		HUnlock((Handle)(**kill).entries);
		  		(**kill).numEntries--;
		  	}
		  	break;

		  case kListNew:			/* Add a new kill entry (at the end) */
		  	{
			  	TKillEntry newEntry;
			  	InitKillEntry(&newEntry, "");
			  	if (EditKillEntry(&newEntry)) {
			  		short index = (**kill).numEntries;
			  		SetPortTextStyle(&gKillListStyle);
			  		AddKillEntry(kill, &newEntry, index, gGroupDlgData.entryList);
			  	} else {
			  		DelKillEntry(&newEntry);
			  	}
		  	}
		  	break;
		}
	} while (itemHit != ok && itemHit != cancel);

	if (itemHit == ok) {
		StoreGroupKill(kill);
	} else {
		DelGroupKill(kill);
	}
	kill = nil;

cleanup:
	if (gGroupDlgData.entryList) {
		LDispose(gGroupDlgData.entryList);
		gGroupDlgData.entryList = NULL;
	}
	if (kill)
		DelGroupKill(kill);
	if (dlg)
		MyDisposDialog(dlg);
}


/*--------------------------------------------------------------------------------
 * KillEntryToList
 * Make a text string corresponding to a killfile entry, and put the string
 * at the specified place in the list.  The list cell must already exist, it
 * is not created here.  This makes this function useful for list refreshes
 * when the data changes.
 */
void
KillEntryToList(
	TKillEntry	*theEntry,		/* The entry to put in the list */
	short		index,			/* Where to put it */
	ListHandle	aList)			/* The list handle */
{
	CStr255		text;
	const char	*fieldPtr;
	Str63		matchStr, actionStr;
	char		headerStr[64];
	Str255		item;
	MenuHandle	mHdl;
	Cell		aCell;

	/* Find the strings for the field name and match type */
	fieldPtr = KillHeaderName(theEntry->header);
	if (!fieldPtr) fieldPtr = "????";
	strcpy(headerStr, fieldPtr);

	mHdl = GetMenu(kKillKeepMenu);				/* What to do with it? */
	GetItem(mHdl, theEntry->action, actionStr);

	HLock(theEntry->string);
	strcpy(text, *theEntry->string);
	HUnlock(theEntry->string);

	GetIndString(item, kKillDlgStrings, kKillListStr);
	ParamString(item, actionStr, c2pstr(headerStr), c2pstr(text), "\p",
				sizeof(item));
	
	SetPortTextStyle(&gKillListStyle);
	SetPt(&aCell, 0, index);
	LSetCell(item+1, *item, aCell, aList);
}

/*--------------------------------------------------------------------------------
 * AddKillEntry
 * Add a new killfile entry to the group killfile and to to the listbox.
 * This creates the list manager cell, unlike KillEntryToList.
 */
OSErr
AddKillEntry(
	TGroupKillHdl	kill,
	TKillEntry		*newEntry,
	short			index,
	ListHandle		theList)
{
	Cell theCell;
	OSErr err = noErr;
	TKillEntry **entries = (**kill).entries;

	if (index < 0) {
		index = 0;
	} else if (index > (**kill).numEntries) {
		index = (**kill).numEntries;
	}

	/* Make room for the new entry */
	MySetHandleSize((Handle)entries,
					((**kill).numEntries + 1) * sizeof(TKillEntry));
	err = MyMemErr();

	if (err == noErr) {
		/* If we're not adding at the end, move other items */
		Size toMove = ((**kill).numEntries - index) * sizeof(TKillEntry);
		if (toMove > 0) {
			BlockMove(*entries + index, *entries + index + 1, toMove);
		}
		/* Put it in the list */
		(**kill).numEntries++;
		(*entries)[index] = *newEntry;

		if (theList) {
			/* Add new cell to list */
			LAddRow(1, index, theList);
			KillEntryToList(newEntry, index, theList);
			/* Clear out old selection */
			SetPt(&theCell, 0, 0);
			while (LGetSelect(true, &theCell, theList)) {
				LSetSelect(false, theCell, theList);
				theCell.v += 1;
			}
			/* Select the new cell and scroll it into view */
			SetPt(&theCell, 0, index);
			LSetSelect(true, theCell, theList);
			LAutoScroll(theList);
		}
	}

	return err;
}

/*--------------------------------------------------------------------------------
 * GroupKillFilter
 * Dialog filter function for the newsgroup killfile editor.
 * The main work here is to handle clicks in the listbox
 */
static pascal Boolean
GroupKillFilter(
	DialogPtr	dlg,
	EventRecord	*theEvent, 
	short		*itemHit)
{
	Boolean			result = FALSE;
	GrafPtr			savePort;
	TextStyle		saveStyle;
	char			theChar;
	Handle			iHndl;
	short			iType;
	Rect			iRect;

	GetPort(&savePort);
	SetPort(dlg);	/* TODO: necessary? */

	if (theEvent->what == mouseDown && gGroupDlgData.entryList != NULL) {
		Point where = theEvent->where;
		GlobalToLocal(&where);
		if (FindDItem(dlg, where) == kKillList - 1) {
			GetPortTextStyle(&saveStyle);
			SetPortTextStyle(&gKillListStyle);
			gFirstCall = true;
			if (LClick(where, theEvent->modifiers, gGroupDlgData.entryList)) {
				/* Double-click, return fake item ID */
				*itemHit = kKillListDblClick;
				result = TRUE;
			} else {
				/* Single-click; return normal item ID */
				*itemHit = kKillList;
				result = TRUE;
			}
			SetPortTextStyle(&saveStyle);
		}
	} else if (theEvent->what == keyDown || theEvent->what == autoKey) {
		char theChar = theEvent->message & charCodeMask;
		char theKey = (theEvent->message & keyCodeMask) >> 8;
		/* Clear and Delete do the same thing as the delete button */
		if (theChar == deleteKey || theKey == clearKeyCode) {
			GetDItem(dlg, kListDelete, &iType, &iHndl, &iRect);
			if (theEvent->what == keyDown && !(iType & itemDisable)) {
				*itemHit = kListDelete;
				result = true;
			} else {
				theEvent->what = nullEvent;		/* ignore autoKey */
			}
		}
	}

	SetPort(savePort);

	result = result || DialogFilter(dlg, theEvent, itemHit);

	return result;
}

/*--------------------------------------------------------------------------------
 * KillListItem
 * The userItem procedure for the list box; just draws it when necessary
 */
static pascal void
KillListItem(
	WindowPtr	dlg,
	short		theItem)
{
	TextStyle		saveStyle;
	Rect			frame;

	if (gGroupDlgData.entryList) {
		SetPort(dlg);
		frame = (**gGroupDlgData.entryList).rView;
		InsetRect(&frame, -1, -1);
		PenNormal();
		FrameRect(&frame);
		GetPortTextStyle(&saveStyle);
		SetPortTextStyle(&gKillListStyle);
		LUpdate(dlg->visRgn, gGroupDlgData.entryList);
		SetPortTextStyle(&saveStyle);
	}
}

/*--------------------------------------------------------------------------------
 * KillListClikLoop
 * This big ugly mess is the clikLoop procedure for the list box.
 *
 * As long as the mouse stays in the cell where it was clicked, it just
 * returns true to tell the List Manager to keep going.
 *
 * Once the mouse leaves the cell, this routine totally takes over and
 * lets the user drag a dotted outline of the cell around in the list
 * so she can rearrange cells.
 */
static pascal Boolean KillListClikLoop()
{
	WindowPtr		wind = FrontWindow();
	ListHandle		theList = gGroupDlgData.entryList;
	Point			mousePt, lastPt;
	Cell			clickCell, destCell;
	Rect			cellRect, limitRect, dragRect;
	short			lineCoord, lastCoord;
	RgnHandle		saveClip;
	static Point	firstPt;
	long			needsScroll, scrollTime;

	/* On the first call just return, because LLastClick doesn't work yet */
	if (gFirstCall) {
		gFirstCall = false;
		GetMouse(&firstPt);
		return true;
	}

	/* If the mouse is still in the original cell, keep tracking */
	clickCell = LLastClick(theList);
	LRect(&cellRect, clickCell, theList);
	GetMouse(&mousePt);
	if (clickCell.v > (**theList).dataBounds.bottom
			|| PtInRect(mousePt, &cellRect)) {
		return true;
	}

	/* Set up clipping to keep drawing confined to the list */
	saveClip = NewRgn();
	GetClip(saveClip);
	limitRect = (**theList).rView;
	ClipRect(&limitRect);

	/* Set up for drawing the dotted outline of a cell */
	PenMode(patXor);
	PenPat(qd.gray);
	lastPt = mousePt;					/* Rect for dragged cell */
	dragRect = cellRect;
	OffsetRect(&dragRect, 0, mousePt.v - firstPt.v);
	FrameRect(&dragRect);

	/* And for a bold dividing line to indicate where the cell would go */
	lineCoord = lastCoord = cellRect.top;
	PenSize(2, 2);						/* Line showing destination */
	PenPat(qd.black);
	MoveTo(cellRect.left, lineCoord);
	LineTo(cellRect.right, lineCoord);
	
	scrollTime = TickCount();
	while (StillDown()) {
		GetMouse(&mousePt);
		needsScroll = 0;
		/* If the mouse is outside the list rectangle, autoscroll */
		if (TickCount() - needsScroll > 30) {
			if (mousePt.v < limitRect.top)
				needsScroll = -1;
			else if (mousePt.v > limitRect.bottom)
				needsScroll = 1;
		}
		if (!EqualPt(mousePt, lastPt) || needsScroll) {
			/* Erase the old dragging rectangle and draw the new one */
			PenPat(qd.gray);
			PenSize(1,1);
			FrameRect(&dragRect);	/* Erase old one first */
			/* Do scrolling if we need to */
			if (needsScroll) {
				PenPat(qd.black);
				PenSize(2,2);
				MoveTo(cellRect.left, lineCoord);
				LineTo(cellRect.right, lineCoord);
				PenNormal();
				LScroll(0, needsScroll, theList);
				scrollTime = TickCount();
				PenMode(patXor);
				PenSize(2,2);
				MoveTo(cellRect.left, lineCoord);
				LineTo(cellRect.right, lineCoord);
			}
			/* Now draw the new dragging rectangle */
			dragRect = cellRect;
			OffsetRect(&dragRect, 0, mousePt.v - firstPt.v);
			if (dragRect.top < limitRect.top) {
				OffsetRect(&dragRect, 0, limitRect.top - dragRect.top);
			} else if (dragRect.bottom > limitRect.bottom) {
				OffsetRect(&dragRect, 0, limitRect.bottom - dragRect.bottom);
			}
			PenPat(qd.gray);
			PenSize(1,1);
			FrameRect(&dragRect);
			lastPt = mousePt;
			
			/* Figure out which cell the mouse is in now */
			lineCoord = (mousePt.v - (**theList).rView.top) / (**theList).cellSize.v;
			if (lineCoord < 0)
				lineCoord = 0;
			if (lineCoord > (**theList).dataBounds.bottom)
				lineCoord = (**theList).dataBounds.bottom;
			SetPt(&destCell, 0, lineCoord + (**theList).visible.top);
			lineCoord = lineCoord * (**theList).cellSize.v + (**theList).rView.top - 1;
		}
		if (lineCoord != lastCoord) {
			/* If the destination has moved, redraw the black line that indicates it */
			PenPat(qd.black);
			PenSize(2,2);
			MoveTo(cellRect.left, lastCoord);
			LineTo(cellRect.right, lastCoord);
			MoveTo(cellRect.left, lineCoord);
			LineTo(cellRect.right, lineCoord);
			lastCoord = lineCoord;
		}
	}
	PenSize(1,1);
	PenPat(qd.gray);
	FrameRect(&dragRect);				/* Erase last rect */
	PenSize(2,2);

	PenPat(qd.black);
	MoveTo(cellRect.left, lineCoord);	/* Erase last line */
	LineTo(cellRect.right, lineCoord);
	PenNormal();

	SetClip(saveClip);
	DisposeRgn(saveClip);

	if (!EqualPt(destCell, clickCell)) {
		/*
		 * The item was dragged to another cell.  Rearrange the 
		 * kill entry list for this group, then refresh the affected
		 * cells in the list.  It's too hard to describe what "up"
		 * does here, but it's fairly obvious if you draw a picture.
		 */
		TKillEntry**	entries = (**gGroupDlgData.kill).entries;
		TKillEntry		temp;
		short			num = (**gGroupDlgData.kill).numEntries;
		short			first, last, i;
		Boolean			up;
		
		if (clickCell.v < destCell.v) {
			first = clickCell.v; last = destCell.v;
			up = true;
		} else {
			first = destCell.v; last = clickCell.v;
			up = false;
		}
		/* Move the selected item to its new spot in the list */
		temp = (*entries)[clickCell.v];
		BlockMove(*entries + first + (up ? 1 : 0),
					*entries + first + (up ? 0 : 1),
					(last - first - (up ? 1 : 0)) * sizeof(**entries));

		(*entries)[destCell.v - (up ? 1 : 0)] = temp;
	
		/* Put the new text in all of the affected list cells */
		LDoDraw(false, theList);
		HLock((Handle)entries);
		if (last > num - 1)
			last = num - 1;
		for (i = first; i <= last; i++) {
			KillEntryToList(*entries + i, i, theList);
		}
		HUnlock((Handle)entries);
		LDoDraw(true, theList);
		/* And force the list to be redrawn */
		limitRect = (**theList).rView;
		InvalRect(&limitRect);
	}

	return false;	/* No more tracking */
}

/*===================================================================================
 * EditKillEntry
 * This function manages the dialog box for editing an individual killfile entry
 */
/* Items in the kill entry editing dialog... */
#define	kKillText		3		/* Text field for string to match */
#define	kKillField		4		/* Popup for header to match */
#define kActionPopup	7		/* Popup to decide what to do with matches */
#define kHilitePopup	8		/* Popup for highlight color */

/*----------------------------------------------------------------------------
 * EditKillEntry
 * This function manages the dialog box for editing an individual killfile entry
 */
Boolean	
EditKillEntry(
	TKillEntry *killEntry)
{
	DialogPtr		dlg;
	GrafPtr			savePort;
	ControlHandle	fieldPop;
	short			itemHit, temp;
	Boolean			ignore;
	CStr255			text;
	const char *	header;
	OSErr			err = noErr;

	/* Since this can be called from another dialog, save/restore the port */
	GetPort(&savePort);

	dlg = MyGetNewDialog(kKillEntryDlg);

	/* Initialize the dialog to the current values */
	fieldPop = DlgGetControl(dlg, kKillField);
	header = KillHeaderName(killEntry->header);
	if (header) {
		strcpy(text, header);
		SetPopupValue(fieldPop, c2pstr(text), false);
	}

	DlgSetCtlValue(dlg, kActionPopup, killEntry->action);
	DlgSetCtlValue(dlg, kHilitePopup, killEntry->highlight+1);

	/* The highlight color does't apply if action is kill */
	DlgEnableItem(dlg, kHilitePopup, killEntry->action != kKill);

	if (killEntry->string) {
		HLock(killEntry->string);
		DlgSetCString(dlg, kKillText, *killEntry->string);
		HUnlock(killEntry->string);
	}
	DlgEnableItem(dlg, ok, killEntry->string && **(killEntry->string) != 0);

	ShowWindow(dlg);
	do {
		MyModalDialog(DialogFilter, &itemHit, TRUE, TRUE);
		switch (itemHit) {
		  case kKillText:
		  	DlgGetCString(dlg, kKillText, text);
		  	DlgEnableItem(dlg, ok, *text != 0);
		  	break;

		  case kActionPopup:
		  	temp = DlgGetCtlValue(dlg, kActionPopup);
		  	DlgEnableItem(dlg, kHilitePopup, temp != kKill);
		  	break;
		}
	} while (itemHit != ok && itemHit != cancel);

	if (itemHit == ok) {
	  	GetPopupCString(fieldPop, kCurrentPopupItem, text);
	  	killEntry->header = KillHeaderNum(text);
	  	killEntry->action = DlgGetCtlValue(dlg, kActionPopup);
	  	killEntry->highlight = DlgGetCtlValue(dlg, kHilitePopup) - 1;
	  	DlgGetCString(dlg, kKillText, text);
	  	if (killEntry->string) {
	  		/* Replacing existing string handle */
	  		err = PtrToXHand(text, killEntry->string, strlen(text) + 1);
	  	} else {
	  		/* New string handle */
	  		err = PtrToHand(text, &(killEntry->string), strlen(text) + 1);
	  	}
	}

exit:
	if (dlg)
		MyDisposDialog(dlg);
	if (err)
		UnexpectedErrorMessage(err);

	SetPort(savePort);

	return (itemHit == ok && err == noErr);
}

/*===================================================================================
 * KillThisAuthor & KillThisSubject
 * These get called as shortcuts if the user wants to filter the subject or author
 * from the current article.  KillThisOne does the actual work.
 */

static void KillThisOne(WindowPtr wind, const char*	header);
	
void
KillThisAuthor(WindowPtr wind)
{
	KillThisOne(wind, "from:");
}

void
KillThisSubject(WindowPtr wind)
{
	KillThisOne(wind, "subject:");
}

static void
KillThisOne(
	WindowPtr	wind,
	const char*	header)
{
	TWindow			**info;
	Handle			text = nil;
	CStr255			groupName;
	char			killText[64];
	TGroupKillHdl	groupKill;
	short			groupIndex;
	OSErr			err = noErr;
	TKillEntry		newEntry;

	FindGroupName(wind, groupName);
	if (*groupName == 0)
		return;

	/* Find the killfile entry for this group */
	groupKill = FindGroupKill(groupName, &groupIndex);
	if (groupKill == NULL) {
		/*  There is no killfile entry for this group yet.  Create one */
		groupKill = NewGroupKill(groupName);
		FailNIL(groupKill, error);
		FailErr(StoreGroupKill(groupKill), error);
	}

	/* Find the text of the appropriate header field */
	*killText = 0;
	info = (TWindow**)GetWRefCon(wind);
	if ((**info).kind == kArticle || (**info).kind == kMiscArticle) {
		/* We have the whole article; just grab the header out of it */
		text = (**info).fullText;
		if (!FindHeader(text, header, killText, sizeof(killText))) {
			FailErr(-1, error);
		}
	} else if ((**info).kind == kSubject && KillHeaderNum(header) == kKillSubject) {
		/* This is a subject window, so all we can do is get a subject */
		ListHandle	theList = (**info).theList;
		Handle		strings = (**info).strings;
		Cell		tmpCell;
		short		cellData, len;
		TSubject	theSubject;
		SetPt(&tmpCell,0,0);
		if (LGetSelect(true,&tmpCell,theList)) {
			len = sizeof(cellData);
			LGetCell(&cellData, &len, tmpCell, theList);
			theSubject = (*(**info).subjectArray)[cellData];
			strcpy(killText, *strings + theSubject.subjectOffset);
		}
	}
	if (!*killText)
		return;

	/* Make a new killfile entry */
	InitKillEntry(&newEntry, killText);
	newEntry.header = KillHeaderNum(header);

	/* And let the user edit it */
	if (EditKillEntry(&newEntry)) {
  		short index = (**groupKill).numEntries;
  		/* This is a perverted use of AddKillEntry; it will ignore the nil list */
  		AddKillEntry(groupKill, &newEntry, index, nil);
	} else {
		DelKillEntry(&newEntry);
	}
		
	return;

error:
	UnexpectedErrorMessage(err);
}
