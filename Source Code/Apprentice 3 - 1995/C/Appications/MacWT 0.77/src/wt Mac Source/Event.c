/*
** File:		Event.c
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright © 1995 Nikol Software
** All rights reserved.
*/



#include "MacWT.h"
#include "Constants.h"
#include "Menu.h"


static Boolean		gInBackground = false;



/*****************************************************************************/




short GetAndProcessEvent(void)
{
	WindowPtr		whichWindow;
	long			menuResult;
	short			partCode;
	short			result = gotNoEvent;
	char			ch;


	if (WaitNextEvent(everyEvent, &gTheEvent, gInBackground ? kBackTime : kFrontTime, nil))
		{
		result = gotOtherEvent;

		switch (gTheEvent.what) {

		case mouseDown:

			switch (partCode = FindWindow(gTheEvent.where, &whichWindow)) {

			case inSysWindow:
				SystemClick(&gTheEvent, whichWindow);
				break;

			case inMenuBar:
				DoAdjustMenus();
				DoMenuCommand(MenuSelect(gTheEvent.where));
				break;

			case inDrag:
				SelectWindow(whichWindow);
				DragWindow(whichWindow, gTheEvent.where, &qd.screenBits.bounds);
				break;

			case inContent:
				if (whichWindow != FrontWindow())
					SelectWindow(whichWindow);
				break;

			case inGoAway:
				if (TrackGoAway(whichWindow, gTheEvent.where))
					ExitToShell();
				break;

			default:
				break;

			}								// switch (FindWindow)
			break;


		case keyDown:
			ObscureCursor();
			// FALL THRU

		case autoKey:
			ch = gTheEvent.message;			// automagic "& charCodeMask" :)
			if (gTheEvent.modifiers & cmdKey)
				{
				DoAdjustMenus();
				menuResult = MenuKey(ch);
				if (menuResult & 0xFFFF0000)
					{
					DoMenuCommand(menuResult);
					break;		// out of this switch if it was a menu command
					}
				}
			else
				if (ch == kTab)	// tab/esc ==> pause game
					TogglePause();

			// FALL THRU

		case keyUp:				// if we ever switch these on...
			result = gotKeyEvent;
			break;


		case updateEvt:
			whichWindow = (WindowPtr)gTheEvent.message;
			BeginUpdate(whichWindow);

			if (whichWindow == gWindow)
				{
				UpdtDialog(gWindow, gWindow->visRgn);
				RefreshWTWindow();
				if (gPaused)
					ShowIfPaused();
				else if (!gGameOn)
					MacAttractMode();
				}

			EndUpdate(whichWindow);
			break;

		case diskEvt:
		case activateEvt:
			break;

		case app4Evt:
			if ((gTheEvent.message << 31) == 0)		// suspend event
				{
				if (!gPaused)
					TogglePause();
				gInBackground = true;
				}
			else
				{
				gInBackground = false;
				SetPort(gWindow);
				}
			break;

		case kHighLevelEvent:
			AEProcessAppleEvent(&gTheEvent);
			break;

		default:
			break;
		}										// switch (gTheEvent.what)
	}

	return result;
}


/**/



void TogglePause(void)
{
	Str255	aStr;

	if (gGameOn)
		{
		MenuHandle menu = GetMHandle(mFile);
		gPaused = !gPaused;
		
		if (gPaused)
			ShowIfPaused();
	
		GetIndString(aStr, rUtilityStrs, (gPaused) ? rStrContinue : rStrPause);
		SetItem(menu, kPause, aStr);
		}
}

