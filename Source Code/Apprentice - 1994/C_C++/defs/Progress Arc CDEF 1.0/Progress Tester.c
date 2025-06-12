/*
 * Progress Tester.c
 *
 * By Eddy J. Gurney
 *
 * Simple program to test my Progress CDEF's
 *
 */

void ToolBoxInit()
{
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	MaxApplZone();
	InitCursor();
	
	SetDAFont(applFont);
}

void DoDialogUpdate(DialogPtr theDialog)
{
	GrafPtr	savePort;
	
	GetPort(&savePort);
	SetPort(theDialog);
	
	BeginUpdate(theDialog);
	UpdtDialog(theDialog, theDialog->visRgn);
	EndUpdate(theDialog);
	SetPort(savePort);
}

void DoDialogActivate(DialogPtr theDialog, Boolean active)
{
	SetPort(theDialog);
}

pascal Boolean DialogFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem)
{
	short		iType;
	Handle		iHandle;
	Rect			iRect;
	char			theChar;
	Boolean		result = FALSE;
	WindowPtr	theWindow;
	long			endTicks;
	static long		nextUpdate = 0;

	theWindow = (WindowPtr)(theEvent->message);
	switch (theEvent->what) {
		case updateEvt:
			if (theWindow == theDialog) {
				DoDialogUpdate(theDialog);
				result = TRUE;
				*theItem = 0;
			}
			break;
		case activateEvt:
			if (theWindow == theDialog) {
				DoDialogActivate(theDialog, (theEvent->modifiers & activeFlag) != 0);
				*theItem = 0;
			}
			break;
		case keyDown:
		case autoKey:
			theChar = theEvent->message & charCodeMask;
			if (theChar == 0x0d || theChar == 0x03) {	/* Return/Enter key? */
				GetDItem(theDialog, ok, &iType, &iHandle, &iRect);
				HiliteControl((ControlHandle)iHandle, 1);
				Delay(8, &endTicks);
				HiliteControl((ControlHandle)iHandle, 0);
				*theItem = ok;
				return TRUE;
			}
			break;
		case nullEvent:
			if (TickCount() > nextUpdate) {
				short value;

				/* Update the progress bar */
				GetDItem(theDialog, 2, &iType, &iHandle, &iRect);
				if ((value = GetCtlValue((ControlHandle)iHandle)) >= GetCtlMax((ControlHandle)iHandle))
					value = GetCtlMin((ControlHandle)iHandle);
				else
					value++;
				SetCtlValue((ControlHandle)iHandle, value);
				
				/* Update the progress arc */
				GetDItem(theDialog, 3, &iType, &iHandle, &iRect);
				if ((value = GetCtlValue((ControlHandle)iHandle)) >= GetCtlMax((ControlHandle)iHandle))
					value = GetCtlMin((ControlHandle)iHandle);
				else
					value++;
				SetCtlValue((ControlHandle)iHandle, value);
				
				/* Item 4 is the thermometer PICT */
				
				/* Update the vertical progress bar */
				GetDItem(theDialog, 5, &iType, &iHandle, &iRect);
				if ((value = GetCtlValue((ControlHandle)iHandle)) >= GetCtlMax((ControlHandle)iHandle))
					value = GetCtlMin((ControlHandle)iHandle);
				else
					value++;
				SetCtlValue((ControlHandle)iHandle, value);

				nextUpdate = TickCount() + 3;			
			}
			break;
	}
	return result;
}

main()
{
	GrafPtr		savePort;
	DialogPtr		theDialog;
	short		itemHit, iType;
	Handle		iHandle, rsrcHandle;
	Boolean		dialogDone = FALSE;
	Rect			iRect;
	Str255		theString;
	long			theValue;
	
	ToolBoxInit();
	
	if ((theDialog = GetNewDialog(400, nil, (WindowPtr)-1)) == nil) {
		SysBeep(1);
		ExitToShell();
	}
	
	GetPort(&savePort);	
	SelectWindow(theDialog);
  	SetPort(theDialog);

	ShowWindow(theDialog);

	while (!dialogDone) {
		ModalDialog(DialogFilter, &itemHit);
		switch (itemHit) {
			case ok:
				dialogDone = TRUE;
				break;
		}
	}
	DisposDialog(theDialog);
  	SetPort(savePort);
  	ExitToShell();
}
