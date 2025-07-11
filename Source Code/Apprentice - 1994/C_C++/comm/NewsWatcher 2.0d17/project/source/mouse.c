/*----------------------------------------------------------------------------

	mouse.c

	This module handles mousedown events.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include "glob.h"
#include "article.h"
#include "close.h"
#include "cmd.h"
#include "key.h"
#include "collapse.h"
#include "draw.h"
#include "mouse.h"
#include "resize.h"
#include "scroll.h"
#include "send.h"
#include "util.h"
#include "wind.h"
#include "drag.h"


/*	DoDrag handles window drags */

static void DoDrag (WindowPtr wind, Point globMouse)
{
	Rect oldRect, newRect;
	GrafPtr savePort;
	WStateData **stateHndl;
	short deltaH, deltaV;

	GetPort(&savePort);
	SetPort(wind);
	stateHndl = (WStateData**)((WindowPeek)wind)->dataHandle;
	oldRect = wind->portRect;
	LocalToGlobal((Point*)&oldRect.top);
	DragWindow(wind, globMouse, &gDragRect);
	newRect = wind->portRect;
	LocalToGlobal((Point*)&newRect.top);
	deltaH = newRect.left - oldRect.left;
	deltaV = newRect.top - oldRect.top;
	OffsetRect(&(**stateHndl).stdState, deltaH, deltaV);
	OffsetRect(&(**stateHndl).userState, deltaH, deltaV);
	SetPort(savePort);
}


/* HandleContent: Handles mouse-downs within content of window */

static void HandleContent (WindowPtr wind, EventRecord *ev)
{
	GrafPtr savePort;
	TWindow **info;
	short part;
	Boolean extendClick;
	TEHandle theTE;
	ControlHandle control;
	EWindowKind kind;
		
	if (wind != FrontWindow()) {
		SelectWindow(wind);
		return;
	}
	
	GetPort(&savePort);
	SetPort(wind);
	GlobalToLocal(&ev->where);

	info = (TWindow**) GetWRefCon(wind);
	kind = (**info).kind;
	
	switch (kind) {
		case kFullGroup:
		case kNewGroup:
		case kUserGroup:
		case kSubject:
			if (kind == kSubject && TriangleClick(wind, ev->where)) break;
			if (kind != kSubject) BeginGroupListClick();
			if (LClick(ev->where, ev->modifiers, (**info).theList))
				OpenSelectedCells(wind);
			break;
		case kArticle:
		case kMiscArticle:
		case kPostMessage:
		case kMailMessage:
			theTE = (**info).theTE;
			if ((part = FindControl(ev->where, wind, &control)) != 0) {
				if (GetCRefCon(control) == kSendButton) {
					if (TrackControl(control, ev->where, nil) != 0) {
						if (DoSendMsg(wind)) DoCloseWindow(wind);
					}
				} else {
					DoScrollers(control, part, ev->where);
				}
			}
			else {
				if (PtInRect(ev->where, &((**theTE).viewRect))) {
					extendClick = ((ev->modifiers & shiftKey) != 0);
					TEClick(ev->where,extendClick, theTE);
					if ((ev->modifiers & optionKey) != 0) 
						OpenSelectedReference(wind);
				}
			}
			break;
		case kStatus:
			if ((part = FindControl(ev->where, wind, &control)) != 0 &&
				TrackControl(control, ev->where, nil) != 0)
				gCancel = true;
			break;
	}
	SetPort(savePort);
}


/* HandleMouseDown handles mouse down events */

void HandleMouseDown (EventRecord *ev)
{
	WindowPtr wind;
	TWindow **info;
	short part;
	
	part = FindWindow(ev->where, &wind);
	
	if (IsStatusWindow(FrontWindow()) && !IsStatusWindow(wind)) return;
	
	switch (part) {
		case inMenuBar:
			if (!IsStatusWindow(FrontWindow()))
				DoCommand(MenuSelect(ev->where));
			break;
		case inSysWindow:
			SystemClick(ev, wind);
			break;
		case inDrag:
			DoDrag(wind, ev->where);
			break;
		case inGrow:
			DoGrow(wind, ev->where);
			break;
		case inGoAway:
			if (TrackGoAway(wind, ev->where)) {
				info = (TWindow**)GetWRefCon(wind);
				if ((**info).kind == kFullGroup) {
					ShowHideGroups();
				} else {
					DoCloseWindow(wind);
				}
			}
			break;
		case inZoomIn:
		case inZoomOut:
			if (TrackBox(wind, ev->where, part)) DoZoom(wind, part);
			break;
		case inContent:
			HandleContent(wind, ev);
			break;
	}
}
