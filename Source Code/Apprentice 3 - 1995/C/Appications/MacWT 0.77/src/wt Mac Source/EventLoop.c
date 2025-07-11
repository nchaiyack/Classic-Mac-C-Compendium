


int GetAndProcessEvent(void)
{
	WindowPtr		whichWindow;
	long			menuResult;
	short			partCode;
	int				result = gotNoEvent;
	char			ch;
#define Polite 1

#if Polite
	if (WaitNextEvent(everyEvent, &gTheEvent, gInBackground ? kBackTime : kFrontTime, nil)) {
#else
	if (GetNextEvent(everyEvent, &gTheEvent)) {
#endif
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
				if (TrackGoAway(whichWindow, gTheEvent.where)) {
					ExitToShell();
				}
				break;

			case inZoomIn:
			case inZoomOut:
				if (TrackBox(whichWindow, gTheEvent.where, partCode))
					{
					SetPort(whichWindow);
					EraseRect(&whichWindow->portRect);
					ZoomWindow(whichWindow, partCode, whichWindow == FrontWindow());
					InvalRect(&whichWindow->portRect);
					}
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
				if (ch == kTab || ch == kEsc)	// tab/esc ==> pause game
				TogglePause();

			// FALL THRU

		case keyUp:				// if we ever switch these on...
			result = gotKeyEvent;
			break;


		case updateEvt:
			whichWindow = (WindowPtr)gTheEvent.message;
			BeginUpdate(whichWindow);

			if (whichWindow == gMainWindow)
				RefreshWTWindow();

			EndUpdate(whichWindow);
			break;

		case diskEvt:
			break;

		case activateEvt:
			break;

		case app4Evt:
			if ((gTheEvent.message << 31) == 0) {	// suspend event
				if (!gPaused)
					TogglePause();
				gInBackground = true;
				}
			else
				{
				gInBackground = false;
				SetPort(gMainWindow);
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
