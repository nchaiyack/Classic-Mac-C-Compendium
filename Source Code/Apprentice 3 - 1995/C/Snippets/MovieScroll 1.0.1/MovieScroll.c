//¥ --------------------------------------------------------------- ¥//
//¥ A public domain demo of "movie" type scrolling,	courtesy of		¥//
//¥ Kenneth A. Long (kenlong@netcom.com).							¥//
//¥ An itty bitty bytesª production, for the benefit of anyone		¥//
//¥ who can use it.													¥//
//¥ This is basically just "Bullseye" with some things dumped and	¥//
//¥ two routines added, and a resource file to back the additions.	¥//

//¥ If you want the text drawn over black, like in the movies, 		¥//
//¥ then some other "girations" are necessary.  This just shows 	¥//
//¥ the scrolling, with style.										¥//
 
//¥ Enjoy!															¥//

//¥ --------------------------------------------------------------- ¥//

//¥ BullShell.c

void InitMacintosh (void);
void SetUpMenus (void);
void HandleEvent (void);
void HandleMouseDown (EventRecord *theEvent);
void AdjustMenus (void);
static void enable (MenuHandle menu, short item, short ok);
void HandleMenu (long mSelect);
void main (void);
pascal void OutlineDefault (WindowPtr myDialog, short itemNo);
void DoAbout (void);

#define aboutID 128
TEHandle textHand;

MenuHandle	appleMenu, fileMenu;

enum	{
	appleID = 1,
	fileID
};

enum	{
	quitItem = 1
};

WindowPtr	shell_window;

void InitMacintosh(void)
{
	MaxApplZone();
	
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();

}

void SetUpMenus (void)
{
	InsertMenu (appleMenu = NewMenu (appleID, "\p\024"), 0);
	InsertMenu (fileMenu = NewMenu (fileID, "\pFile"), 0);
	DrawMenuBar ();
	AppendMenu (appleMenu, "\pAbout item");
	AddResMenu (appleMenu, 'DRVR');
	AppendMenu (fileMenu, "\pQuit/Q");
}

void HandleEvent(void)
{
	int			ok;
	EventRecord	theEvent;

	HiliteMenu(0);
	SystemTask ();		//¥ Handle desk accessories.
	
	ok = GetNextEvent (everyEvent, &theEvent);
	if (ok)
		switch (theEvent.what)
		{
			case mouseDown:
				HandleMouseDown(&theEvent);
			break;
			
			case keyDown: 
			case autoKey:
			if ((theEvent.modifiers & cmdKey) != 0)
			{
		   		AdjustMenus();
				HandleMenu(MenuKey((char) (theEvent.message & charCodeMask)));
			}
			break;
			
			case updateEvt:
				BeginUpdate(shell_window);
				EndUpdate(shell_window);
			break;
			
			case activateEvt:
				InvalRect(&shell_window->portRect);
			break;
		}
}

void HandleMouseDown (EventRecord	*theEvent)
{
	WindowPtr	theWindow;
	int			windowCode = FindWindow (theEvent->where, &theWindow);
	
	switch (windowCode)
	{
		case inSysWindow: 
			SystemClick (theEvent, theWindow);
		break;
		
		case inMenuBar:
	  		AdjustMenus();		//¥ Left over - not really needed.
			HandleMenu(MenuSelect(theEvent->where));
		break;
	  	  
		case inContent:
			if (theWindow == shell_window)
			{
				if (theWindow != FrontWindow())
					SelectWindow(shell_window);
				else
					InvalRect(&shell_window->portRect);
			}
		break;
	  	
		case inGoAway:
		if (theWindow == shell_window && 
			TrackGoAway(shell_window, theEvent->where))
			HideWindow(shell_window);
		break;
	}
}

void AdjustMenus(void)
{
	register WindowPeek wp = (WindowPeek) FrontWindow();
	short kind = wp ? wp->windowKind : 0;
	Boolean DA = kind < 0;
		
}


static void enable(MenuHandle menu, short item, short ok)
{
	if (ok)
		EnableItem(menu, item);
	else
		DisableItem(menu, item);
}

void HandleMenu (long mSelect)
{
	int			menuID = HiWord(mSelect);
	int			menuItem = LoWord(mSelect);
	Str255		name;
	GrafPtr		savePort;
	WindowPeek	frontWindow;
	
	switch (menuID)
	{
		case	appleID:
			if (menuItem == 1)
				DoAbout ();
			else
				{
					GetPort (&savePort);
					GetItem (appleMenu, menuItem, name);
					OpenDeskAcc (name);
					SetPort (savePort);
			}
		break;
	
		case	fileID:
			switch (menuItem)
			{			  
				if (frontWindow->windowKind < 0)
					CloseDeskAcc(frontWindow->windowKind);
				else 
					if ((frontWindow = (WindowPeek) shell_window) != NULL)
						HideWindow(shell_window);
  				break;
  				  	
				case	quitItem:
					ExitToShell();
				break;
			}
		break;
	}
}


void main( void)
{
	InitMacintosh();
	SetUpMenus();
	DoAbout ();
	for (;;)
		HandleEvent();
}

pascal void OutlineDefault (WindowPtr myDialog, short itemNo)
{
	short x;
	Rect rectangle;
	Handle handle;

	GetDItem((DialogPtr) myDialog, itemNo, &x, &handle, &rectangle);
	PenSize(3, 3);
	InsetRect(&rectangle, -4, -4);
	FrameRoundRect(&rectangle, 16, 16);
	PenSize(1, 1);
}

void DoAbout(void)		//¥ 'About...' dialog box.
{
	#define SCREEN		2		//¥ Text window useritem.
	#define SHOWTIME	3
	short USERITEM = 7;			//¥ Used to outline default button.
	short textID = 128;			//¥ rsrc ID# for text used.
	short styleID = 128;		//¥ rsrc ID# for styl used.

	GrafPtr saveWPtr;			//¥ Holds previous grafPtr.
	DialogPtr aboutPtr;			//¥ Pointer to dialog.
	short theItem;				//¥ Item selected by user.
	Point mLoc;					//¥ Mouse location.
	Rect rectangle;
	short x;
	Handle handle;
	Rect txtRect;				//¥ Used to hold viewRect.
	TEHandle saveTxtHdl;		//¥ Text handle.
	Boolean finished;			//¥ Pushed 'The End' button yet?
	StScrpHandle styleHdl;
	OSErr err;

	GetPort(&saveWPtr);				//¥ Save the old port.
	saveTxtHdl = textHand;			//¥ Save the old text hdl.

	//¥ Get dialog box pointer.
	aboutPtr = GetNewDialog(aboutID, nil, (WindowPtr)-1);	

	//¥ This next makes the static text field font monaco 9.
	SetPort((GrafPtr) aboutPtr);	//¥ Output to dialog.
	TextSize(9);					//¥ Set text size.
	TextFont(monaco);				//¥ Set text font.

	//¥ Get text window rect.
	GetDItem((DialogPtr) aboutPtr, SCREEN, &x, &handle, &txtRect);	
	InsetRect(&txtRect, 5, 1);		//¥ Leave margins for text.

	//¥ Create styled TERecord.
	textHand = TEStylNew(&txtRect, &txtRect);

	//¥ Read the TEXT resource.
	handle = GetResource('TEXT', textID);	
	HLock(handle);					//¥ Lock handle.

	//¥ Get the style handle.
	styleHdl = (StScrpHandle)(Get1Resource('styl', styleID));	
	TEStylInsert(*handle, SizeResource(handle), styleHdl, textHand);	//¥ move text into text record.
	TESetJust (1, textHand);
	HUnlock(handle);					//¥ Unlock handle.

	ShowWindow(aboutPtr);				//¥ Show dialog box now.
	TEUpdate(&txtRect, textHand);		//¥ Draw text in viewRect.
	InsetRect(&txtRect, -5, -1);		//¥ Leave margins for text.
	FrameRect(&txtRect);				//¥ Draw frame around text.
	OutlineDefault(aboutPtr, USERITEM);
	GetDItem((DialogPtr) aboutPtr, USERITEM, &x, &handle, &rectangle);	//¥ outline default button.
	SetDItem(aboutPtr, USERITEM, x, (Handle) OutlineDefault, &rectangle);	//¥ redraw if erased*/

	finished = false;				//¥ Reset flag.
	do {							//¥ Repeat until finished.
		ModalDialog(0L, &theItem);	//¥ Show dialog/get result.
		switch (theItem) {			//¥ Control hit.
			case 1:
				finished = true;	//¥ "The End" button hit / close.
				break;
			case SHOWTIME:
				do
				{
					//¥ scroll up a pixel
					TEScroll(0, -1, textHand);		
				}while (! Button ());
				break;
			default:
				break;				//¥ None of the above, avoid range error.
		}							//¥ End of  case.
	} while (!finished);			//¥ End of  mainloop.

	TEDispose(textHand);			//¥ Reclaim heap space.
	textHand = saveTxtHdl;			//¥ Restore global textHand.
	DisposDialog(aboutPtr);			//¥ Get rid of dialog box.
	SetPort(saveWPtr);				//¥ Restore the old port.
}

