void ToolBoxInit(void);
pascal Boolean	myDlgFilter( DialogPtr theDlg, EventRecord *theEvent, short *itemHit );
pascal void Track(ControlHandle theControl, short part);
unsigned long	gLastTime;

#define WIND_WIDTH 	236
#define MIN_HEIGHT	130
#define MAX_HEIGHT	188
void main()
{
	DialogPtr	aDialog;
	Boolean		done = FALSE;
	short		itemHit;
	short		value;
	short		iType;
	ControlHandle		iHandle;
	Rect		iRect;
	ToolBoxInit();
	aDialog = GetNewDialog(128, 0L, (WindowPtr)-1L);
	SizeWindow(aDialog, WIND_WIDTH, MIN_HEIGHT, FALSE);
	SetPort(aDialog);
	ShowWindow(aDialog);
	GetDItem(aDialog, 2, &iType, (Handle *)&iHandle, &iRect);
	while(!done)
	{
		ModalDialog(myDlgFilter, &itemHit);
		if (itemHit == 1) done = TRUE;
		else if (itemHit == 2)
		{
			value = GetCtlValue(iHandle);
			if (value == 0) SizeWindow(aDialog,WIND_WIDTH,MAX_HEIGHT,TRUE);
			else SizeWindow(aDialog, WIND_WIDTH, MIN_HEIGHT, FALSE);
			SetCtlValue(iHandle, !value);
		}
	}
	DisposDialog(aDialog);
	
}
void ToolBoxInit(void)
{
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
}

pascal Boolean	myDlgFilter( DialogPtr theDlg, EventRecord
			*theEvent, short *itemHit )
{
	short			iWinPart;
	WindowPtr		whichWin;
	short			iCtlPart;
	ControlHandle	whichCtl, upLever, sideLever;
	short			iType;
	Rect			iRect;
	short			value;
	Point			where;
	where = theEvent->where;
	switch (theEvent->what)
	{
		case mouseDown:
			iWinPart = FindWindow(where, &whichWin);
			if (iWinPart == inContent && whichWin == theDlg)
			{
				GlobalToLocal(&where);
				iCtlPart = FindControl(where, whichWin, &whichCtl);
				if (iCtlPart && iCtlPart != inThumb)
				{
					GetDItem(theDlg, 3, &iType, (Handle *)&upLever, &iRect);
					GetDItem(theDlg, 4, &iType, (Handle *)&sideLever, &iRect);
					if (whichCtl == upLever || whichCtl == sideLever)
					{
						gLastTime = TickCount();
						iCtlPart = TrackControl(whichCtl, where, Track);
						value = GetCtlValue(whichCtl);
						switch (iCtlPart)
						{
							case inPageUp:
								SetCtlValue(whichCtl, value + 5);
							break;
							case inPageDown:
								SetCtlValue(whichCtl, value - 5);
							break;
						}
						return (TRUE);
					}
				}
			}
		break;
	}
	return( FALSE );		/* or FALSE to pass it on */
}

pascal void Track(ControlHandle theControl, short part)
{
	unsigned long	time;
	short			value;
	
	if (part == inPageUp || part == inPageDown)
	{
		time = TickCount();
		
		if (time >= gLastTime)
		{
			value = GetCtlValue(theControl);
			switch(part)
			{
				case inPageUp:
					SetCtlValue(theControl, value + 5);
				break;
				case inPageDown:
					SetCtlValue(theControl, value - 5);
				break;
			}
			gLastTime = time + 5L;
		}
	}
}
