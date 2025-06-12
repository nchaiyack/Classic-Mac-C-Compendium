/*----------------------------------------------------------------------------

	close.c

	This module handles window closing.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include "glob.h"
#include "activate.h"
#include "child.h"
#include "close.h"
#include "mark.h"
#include "newsrc.h"
#include "send.h"
#include "util.h"
#include "wind.h"


/*	DoCloseWindow removes a window from the screen and disposes of all the window's
	associated data structures. 
*/

Boolean DoCloseWindow (WindowPtr wind)
{
	TWindow **info;
	TGroup **groupArray,theGroup;
	TUnread **pUnreadRec,**qUnreadRec;
	EWindowKind kind;
	short i,numGroups;
	Point offPt;
	GrafPtr savePort;

	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	
	/* Save window if necessary. */
	
	if (kind == kUserGroup) {
		if ((**info).changed && (**info).autoFetched && gPrefs.autoFetchnewsrc)
			DoSendGroupListToHost(wind, gAutoFetchHost, gAutoFetchName,
				gAutoFetchPass, gAutoFetchPath);
		if ((**info).changed && !(CheckForSave(wind))) return false;
	} else if (kind == kPostMessage || kind == kMailMessage) {
		if ((**info).changed && !CheckForSend(wind)) return false;
	}
	
	/* Close all child windows and remove this window from the child
	   list of its parent window. */
	
	while ((**info).childList != nil) DoCloseWindow((**(**info).childList).childWindow);
	RemoveChild((**info).parentWindow, wind);
	
	/* Deactivate the window. */
	
	HandleActivate(wind,false);
	
	/* Remember size and position of full group list window. */
	
	if (kind == kFullGroup) {
		gPrefs.groupWindowRect = wind->portRect;
		SetPt(&offPt,0,0);
		GetPort(&savePort);
		SetPort(wind);
		LocalToGlobal(&offPt);
		SetPort(savePort);
		OffsetRect(&gPrefs.groupWindowRect,offPt.h,offPt.v);
	}
	
	/* If closing a subject window, update the unread list for the parent
	   group. */
	   
	if (kind == kSubject) UpdateUnreadList(wind);
	
	/* Dispose all of the window's associated data structures. */

	if (kind == kUserGroup) {
		groupArray = (**info).groupArray;
		numGroups = (**info).numGroups;
		for (i = 0; i < numGroups; i++) {
			theGroup = (*groupArray)[i];
			pUnreadRec = theGroup.unread;
			while (pUnreadRec != nil) {
				qUnreadRec = (**pUnreadRec).next;
				MyDisposHandle((Handle)pUnreadRec);
				pUnreadRec = qUnreadRec;
			}
		}
	}
	
	if ((**info).theList != nil) LDispose((**info).theList);
	if ((**info).theTE != nil) TEDispose((**info).theTE);
	if ((**info).groupArray != nil && kind != kFullGroup)
		MyDisposHandle((Handle)(**info).groupArray);
	MyDisposHandle((Handle)(**info).subjectArray);
	if (kind == kSubject) MyDisposHandle((**info).strings);
	MyDisposHandle((**info).fullText);
	MyDisposHandle((**info).headerText);
	MyDisposHandle((Handle)(**info).sectionBreaks);
	MyDisposHandle((Handle)(**info).msgId);
	MyDisposHandle((**info).unsubscribed);
	if ((**info).collapseTriangle != nil) KillPoly((**info).collapseTriangle);
	if ((**info).expandTriangle != nil) KillPoly((**info).expandTriangle);
	MyDisposHandle((Handle)info);
	
	/* Remove window title from Windows menu. */

	RemoveWindMenu(wind);
	
	/* Finally, dispose the window record. */
	
	DisposeWindow(wind);
	
	return true;
}

