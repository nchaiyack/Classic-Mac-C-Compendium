/*----------------------------------------------------------------------------

	activate.c

	This module handles window activate/deactivate events.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include "glob.h"
#include "activate.h"
#include "util.h"


/*	HandleActivate activates/deactivates windows */

void HandleActivate (WindowPtr wind, Boolean actFlag)
{
	TWindow **info;
	GrafPtr savePort;
	Rect growBoxRect;
	short fontNum;
	ControlHandle theControl;
	
	if (IsAppWindow(wind)) {
		info = (TWindow**)GetWRefCon(wind);
		switch ((**info).kind) {
			case kFullGroup:
			case kNewGroup:
			case kUserGroup:
			case kSubject:
				LActivate(actFlag,(**info).theList);
				break;
			case kArticle:
			case kMiscArticle:
			case kPostMessage:
			case kMailMessage:
				theControl = ((WindowPeek) wind)->controlList;
				if (actFlag) {
					ShowControl(theControl);
					theControl = (**theControl).nextControl;
					if (theControl != nil) ShowControl(theControl);
					TEActivate((**info).theTE);
				}
				else {
					HideControl(theControl);
					theControl = (**theControl).nextControl;
					if (theControl != nil) HideControl(theControl);
					TEDeactivate((**info).theTE);
				}
				break;
			case kStatus:
				break;
		}
		GetPort(&savePort);
		SetPort(wind);
		SetRect(&growBoxRect,wind->portRect.right-15,wind->portRect.top,
			wind->portRect.right,wind->portRect.bottom);
		InvalRect(&growBoxRect);
		SetPort(savePort);
		ShowCursor();
	}
}


/* HandleSuspendResume: Handles suspend/resume events- activates main window */

void HandleSuspendResume (long message)
{
	SetCursor(&qd.arrow);
	if ((message >> 24) == 1) {
		if ((message & 1) != 0) {
			gInBackground = false;
			HandleActivate(FrontWindow(),true);
		} else {
			gInBackground = true;
			HandleActivate(FrontWindow(),false);
		}
	}
}
