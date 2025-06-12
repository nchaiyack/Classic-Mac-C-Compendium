/*----------------------------------------------------------------------------

	cmd.c

	This module handles command dispatching.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <ctype.h>
#include <string.h>

#include "glob.h"
#include "article.h"
#include "close.h"
#include "cmd.h"
#include "dlgutil.h"
#include "full.h"
#include "header.h"
#include "killdlg.h"
#include "mark.h"
#include "message.h"
#include "menus.h"
#include "newart.h"
#include "newsrc.h"
#include "next.h"
#include "nntp.h"
#include "open.h"
#include "prefsdlog.h"
#include "print.h"
#include "save.h"
#include "scroll.h"
#include "search.h"
#include "send.h"
#include "subject.h"
#include "subscribe.h"
#include "util.h"
#include "wind.h"
#include "about.h"


/*	DoSelectAll is called in response to a Select-All menu command.
	If in a list manager window, all cells are hilited.  If in a 
	textedit window, the entire range of a message is hilited.
*/

void DoSelectAll (WindowPtr wind)
{
	TWindow **info;
	Cell theCell;
	ListHandle theList;
	EWindowKind kind;
	
	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	if (kind == kFullGroup || kind == kNewGroup || kind == kUserGroup || kind == kSubject) {
		theList = (**info).theList;
		SetPt(&theCell,0,0);
		do {
			LSetSelect(true,theCell,theList);
		} while (LNextCell(false,true,&theCell,theList));
	} else {
		TESetSelect(0,32767,(**info).theTE);
	}
}


/*	DoRot is called in response to the Do Rot-13 menu command.  It
	encrypts/decrypts an entire message, or just a selection using
	the Rot-13 method common on USENET news groups.
*/

static void DoRot (WindowPtr wind)
{
	TWindow **info;
	TEHandle theTE;
	GrafPtr savePort;
	short i,sStart,sEnd;
	
	info = (TWindow**)GetWRefCon(wind);
	theTE = (**info).theTE;
	HLock((**theTE).hText);
	
	sStart = (**theTE).selStart;
	sEnd = (**theTE).selEnd;
	if (sStart == sEnd) {
		sStart = 0;
		sEnd = (**theTE).teLength;
	}
	
	for (i=sStart; i<= sEnd; i++)
		if (isalpha((*(**theTE).hText)[i]))
			(*(**theTE).hText)[i] += (toupper((*(**theTE).hText)[i]) > 'M') ? -13 : 13;		
			
	HUnlock((**theTE).hText);
	GetPort(&savePort);
	SetPort(wind);
	InvalRect(&wind->portRect);
	SetPort(savePort);
}


/* DoCommand: Process pull-down menu requests */

void DoCommand (long mResult)
{
	short selItem,selMenu,temp;
	Str255	name;
	GrafPtr	tempPort;
	WindowPtr wind;
	TWindow **info;
	EWindowKind kind;
	short editCmd;
	Boolean daEditCmd=false;
	TEHandle theTE;
	
	wind = FrontWindow();
	if (IsStatusWindow(wind)) {
		return;
	} else if (wind != nil && IsAppWindow(wind)) {
		info = (TWindow**)GetWRefCon(wind);
		kind = (**info).kind;
		theTE = (**info).theTE;
	}
		
	selItem = LoWord(mResult);
	selMenu = HiWord(mResult);
	
	switch (selMenu) {
		case kAppleMenu:
			if (selItem > 2) {
				GetPort(&tempPort);
				SetCursor(&qd.arrow);
				GetItem(GetMHandle(kAppleMenu),selItem,name);
				temp = OpenDeskAcc(name);
				SetPort(tempPort);
			} else {
				DoAboutBox();
			}
			break;
			
		case kFileMenu:
			switch (selItem) {
				case kNewItem:
					wind = NewUserGroupWindow("\puntitled", nil, 0);
					ShowWindow(wind);
					break;
				case kOpenItem:
					DoOpenGroupList();
					break;
				case kCloseItem:
					if (kind == kFullGroup) {
						ShowHideGroups();
					} else {
						DoCloseWindow(wind);
					}
					break;
				case kSaveItem:
				case kSaveAsItem:
					switch (kind) {
						case kArticle:
						case kMiscArticle:
						case kPostMessage:
						case kMailMessage:
							DoSaveWindow(wind);
							break;
						case kUserGroup:
							if (selItem == kSaveItem) {
								DoSave(wind);
							} else {
								DoSaveAs(wind);
							}
							break;
					}
					break;
				case kPageSetupItem:
					DoPageSetup();
					break;
				case kPrintItem:
					DoPrint(wind);
					break;
				case kPrintSelItem:
					ErrorMessage("Not yet implemented.");
					break;
				case kGetFromHostItem:
					DoGetGroupListFromHost(false);
					break;
				case kSendToHostItem:
					DoSendGroupListToHost(wind, "", "", "", "");
					break;
				case kPrefsItem:
					DoPrefsDialog();
					break;
				case kQuitItem:
					gDone = true;
					break;
			}
			break;
			
		case kEditMenu:
			if (selItem == kUndoItem || selItem == kCutItem || selItem == kCopyItem ||
				selItem == kPasteItem || selItem == kClearItem)
			{
				editCmd = selItem == kClearItem ? 5 : selItem-1;
				daEditCmd = SystemEdit(editCmd);
			}
			if (!daEditCmd)
				switch (selItem) {
					case kUndoItem:
						break;
					case kCutItem:
						TECut(theTE);
						ZeroScrap();
						TEToScrap();
						AdjustScrollBar(wind);
						CheckInsertion(wind);
						break;
					case kCopyItem:
						TECopy(theTE);
						ZeroScrap();
						TEToScrap();
						break;
					case kPasteItem:
						TEFromScrap();
						TEPaste(theTE);
						AdjustScrollBar(wind);
						CheckInsertion(wind);
						break;
					case kPasteQuoteItem:
						ErrorMessage("Not yet implemented.");
						break;
					case kClearItem:
						TEDelete(theTE);
						AdjustScrollBar(wind);
						CheckInsertion(wind);
						break;
					case kSelectAllItem:
						DoSelectAll(wind);
						break;
					case kShowHideHeaderItem:
						DoShowHideHeader(wind);
						break;
					case kRotItem:
						DoRot(wind);
						break;
				}
			break;

		case kNewsMenu:
			switch (selItem) {
				case kNextArticleItem:
					DoNextArticle(wind, false);
					break;
				case kNextThreadItem:
					DoNextThread(wind);
					break;
				case kNextGroupItem:
					DoNextGroup(wind);
					break;
				case kMarkReadItem:
					DoMarkCommand(wind, true);
					break;
				case kMarkUnreadItem:
					DoMarkCommand(wind, false);
					break;
				case kPostItem:
					MakePost(wind);
					break;
				case kFollowUpItem:
					MakeFollowUp(wind);
					break;
				case kReplyViaEmailItem:
					MakeRespond(wind);
					break;
				case kSendItem:
					if (DoSendMsg(wind)) DoCloseWindow(wind);
					break;
				case kCheckNewArticlesItem:
					DoCheckNewArticles(wind);
					break;
			}
			break;
		
		case kSearchMenu:
			switch (selItem) {
				case kFindItem:
					ErrorMessage("Not yet implemented.");
					break;
				case kFindAgainItem:
					ErrorMessage("Not yet implemented.");
					break;
				case kSearchItem:
					DoSearch(wind);
					break;
				case kGroupKillItem:
					#ifdef FILTERS
						GroupKillDialog(wind);
					#else
						ErrorMessage("Not yet implemented.");
					#endif
					break;
				case kKillAuthorItem:
					#ifdef FILTERS
						KillThisAuthor(wind);
					#else
						ErrorMessage("Not yet implemented.");
					#endif
					break;
				case kKillSubjectItem:
					#ifdef FILTERS
						KillThisSubject(wind);
					#else
						ErrorMessage("Not yet implemented.");
					#endif
					break;
			}
			break;

		case kSpecialMenu:
			switch (selItem) {
				case kSubscribeItem:
					DoSubscribe(wind);
					break;
				case kUnsubscribeItem:
					DoUnsubscribe(wind);
					break;
				case kCheckNewGroupsItem:
					CheckForNewGroups();
					CreateNewGroupListWindow();
					break;
				case kCheckDeletedGroupsItem:
					CheckForDeletedGroups();
					break;
				case kRebuildFullGroupsItem:
					ReadGroupsFromServer();
					break;
				case kOpenSelRefItem:
					OpenSelectedReference(wind);
					break;
				case kOpenAllRefsItem:
					OpenReferences(wind);
					break;
				case kGetServerInfoItem:
					GetServerInfo();
					break;
			}
			break;

		case kWindMenu:
			switch (selItem) {
				case kCycleItem:
					SendBehind(wind,nil);
					break;
				case kZoomItem:
					ToggleZoom(wind);
					break;
				case kShowHideFullItem:
					ShowHideGroups();
					break;
				default:
					SelectWindMenu(selItem);
					break;
			}
 	}
	HiliteMenu(0);
}
