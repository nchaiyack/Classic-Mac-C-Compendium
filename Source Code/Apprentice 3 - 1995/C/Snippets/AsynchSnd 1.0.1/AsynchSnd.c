//¥ BullShell.c

#include <Sound.h>		//¥ Added by K.A.L for addition of sounds.

MenuHandle	appleMenu, fileMenu, editMenu, widthMenu;

enum {
	appleID = 1,
	fileID,
	editID,
	widthID
};

enum {
	openItem = 1,
	closeItem,
	quitItem = 4
};

#define over qd.screenBits.bounds.right
#define down qd.screenBits.bounds.bottom

WindowPtr	shellWindow;
Rect		dragRect;
Rect		windowBounds;
Rect		circleStart = {2, 2, 296, 296};
int			theWidth = 2;

SndChannelPtr channelPtr;

//¥ Prototypes:
void SetUpWindow (void);
void Center (char *str);
void SetText (short font, short size, short style);
void DoAbout (void);
void DrawBullseye (short active);
void DoTheSound (short whichID, Boolean asynch);
void HandleKeyHits (EventRecord *theEvent);
void SetUpMenus (void);
void AdjustMenus (void);
int enable (MenuHandle menu, short item, short ok);
void HandleMenu (long mSelect);
void InitMacintosh (void);
void HandleMouseDown (EventRecord *theEvent);
void HandleEvent (void);
void main (void);

//¥ Routines:

void SetUpWindow(void)
{
	dragRect = qd.screenBits.bounds;
	
	SetRect (&windowBounds, over / 2 - 150,
							down / 2 - 150,
							over / 2 + 150,
							down / 2 + 150);
							
	shellWindow = NewWindow(0L, &windowBounds, "\pKen's Shell", true, noGrowDocProc, (WindowPtr) -1L, true, 0);
	SetPort(shellWindow);
}

void Center (char *str)
{	
	Move ((((circleStart.right - 2) / 2) -
			(StringWidth ((StringPtr) str)) / 2), 0);
	DrawString ((StringPtr) str);
	Move (-(shellWindow->pnLoc.h), (shellWindow->txSize) + 2);
}

void SetText (short font, short size, short style)
{
	TextFont (font);
	TextSize (size);
	TextFace (style);
}

void DoAbout ()
{
	EraseRect (&circleStart);
	MoveTo (0, 0);
	Move (0, 30);
	SetText (newYork, 18, italic);
	Center ((char*) "\pAn itty bitty bytesª demo");
	Move (0, 5);
	SetText (newYork, 12, italic);
	Center ((char*) "\pof asynchronous sound play.");
	Move (0, 5);
	SetText (times, 12, outline);
	Center ((char*) "\pPublic Domain (P) November 1994,");
	Move (0, 5);
	Center ((char*) "\pKenneth A. Long.  No rights reserved");
	
	Move (0, 15);
	SetText (0, 12, 0);
	Center ((char*) "\pClick the mouse for some more bull!");
	Move (0, 5);
	Center ((char*) "\pHit number keys as fast as you can.");
	Move (0, 5);
	Center ((char*) "\pHold keys down for continuous sound.");
}
	
void DrawBullseye(short active)
{
	Rect	myRect;
	int		color = true;
	
	SetPort(shellWindow);
	EraseRect(&circleStart);
	myRect = circleStart;
	
	while(myRect.left < myRect.right)
	{
		FrameOval(&myRect);//, color ? (active ? black : gray) : white);
//¥ 	FillOval(&myRect, color ? (active ? black : gray) : white);
		InsetRect(&myRect, theWidth, theWidth);
//¥		color = !color;
	} 
}

//¥ The following routine was added by K.A.L.  It was from the Stella 
//¥ Obscura source, by John Calhoun and ported to C by Ken Long.

void DoTheSound (short whichID, Boolean asynch)
{
	Handle theSnd;
	OSErr err;
	Boolean soundActive;
	
	soundActive = true;	
	
	if ((soundActive))
	{
		theSnd = GetResource ('snd ', whichID);
		
		if ((theSnd != 0L) && (ResError () == noErr))
		{
			if ((channelPtr != 0L))
			{
				err = SndDisposeChannel (channelPtr, true);
				channelPtr = 0L;
			}
			if ((asynch == true) && 
				(SndNewChannel 
				(&channelPtr, 0, initMono, 0L) == noErr)) 
				err = SndPlay (channelPtr, (SndListHandle)theSnd, true); 
			else	
				err = SndPlay (0L, (SndListHandle)theSnd, false);
		}
	}
}

//¥ I added this routine to add key hit sound play capability.
void HandleKeyHits (EventRecord *theEvent)
{
	short	chCode;
	long ticks;
	
	chCode = theEvent->message & charCodeMask;

	switch (chCode) 
	{
		case '1':  // User hits the 1 key.
		   	DoTheSound (9001, true);  // '9001' matches res.ID.
		break;

		case '2': 
		   	DoTheSound (9002, true);
		break;

		case '3':
		   	DoTheSound (9003, true);
		break;

		case '4':
		   	DoTheSound (9004, true);
		break;

		case '5':
		   	DoTheSound (9005, true);
		break;

		case '6':
		   	DoTheSound (9006, true);
		break;

		case '7':
		   	DoTheSound (9007, true);
		break;

		case '8':
		   	DoTheSound (9008, true);
		break;

		case '9':
		   	DoTheSound (9009, true);
		break;

		case '0':
		   	DoTheSound (9010, true);
		break;
	}
}

void SetUpMenus(void)
{
	InsertMenu(appleMenu = NewMenu(appleID, "\p\024"), 0);
	InsertMenu(fileMenu = NewMenu(fileID, "\pFile"), 0);
	InsertMenu(editMenu = NewMenu(editID, "\pEdit"), 0);
	InsertMenu(widthMenu = NewMenu(widthID, "\pWidth"), 0);
	DrawMenuBar();
	AppendMenu(appleMenu, "\pWhat about it?");
	AddResMenu(appleMenu, 'DRVR');
	AppendMenu(fileMenu, "\pOpen/O;Close/W;(-;Quit/Q");
	AppendMenu(editMenu, "\pUndo/Z;(-;Cut/X;Copy/C;Paste/V;Clear");
	AppendMenu(widthMenu, "\p1/1;2/2;3/3;4/4;5/5;6/6;7/7;8/8;9/9");
}

void AdjustMenus(void)
{
	register WindowPeek wp = (WindowPeek) FrontWindow();
	short kind = wp ? wp->windowKind : 0;
	Boolean DA = kind < 0;
	
	enable(editMenu, 1, DA);
	enable(editMenu, 3, DA);
	enable(editMenu, 4, DA);
	enable(editMenu, 5, DA);
	enable(editMenu, 6, DA);
	
	enable(fileMenu, openItem, !((WindowPeek) shellWindow)->visible);
	enable(fileMenu, closeItem, DA || ((WindowPeek) shellWindow)->visible);

	CheckItem(widthMenu, theWidth, true);
}

int enable(MenuHandle menu, short item, short ok)
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
		case appleID:
			if (menuItem == 1)
				DoAbout ();
			else
				{
					GetPort(&savePort);
					GetItem(appleMenu, menuItem, name);
					OpenDeskAcc(name);
					SetPort(savePort);
			}
		break;
	
		case fileID:
		switch (menuItem)
		{
			case openItem:
				ShowWindow(shellWindow);
				SelectWindow(shellWindow);
			break;
  								
			case closeItem:
				if ((frontWindow = (WindowPeek) FrontWindow()) == 0L)
					break;
					
				if (frontWindow->windowKind < 0)
					CloseDeskAcc(frontWindow->windowKind);
				else if (frontWindow = (WindowPeek) shellWindow)
					HideWindow(shellWindow);
  			break;
  						
			case quitItem:
				ExitToShell();
			break;
		}
		break;
  				
		case editID:
		if (!SystemEdit(menuItem-1))
			SysBeep(5);
		break;
		
		case widthID:
			CheckItem(widthMenu, theWidth, false);
			theWidth = menuItem;
			InvalRect(&shellWindow->portRect);
		break;
	}
}

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

void HandleMouseDown (EventRecord	*theEvent)
{
	WindowPtr	theWindow;
	int			windowCode = FindWindow (theEvent->where, &theWindow);
	long ticks;
	
	switch (windowCode)
	{
		case inSysWindow: 
			SystemClick (theEvent, theWindow);
		break;
			
		case inMenuBar:
			AdjustMenus();
			HandleMenu(MenuSelect(theEvent->where));
		break;
			
		case inDrag:
			if (theWindow == shellWindow)
				DragWindow(shellWindow, theEvent->where, &dragRect);
		break;
				
		case inContent:
			if (theWindow == shellWindow)
			{
				if (theWindow != FrontWindow())
					SelectWindow(shellWindow);
				else
					InvalRect(&shellWindow->portRect);
			}
			DoTheSound (8999, true);	//¥ <-----Added by K.A.L.
		break;
			
		case inGoAway:
			if (theWindow == shellWindow && 
				TrackGoAway(shellWindow, theEvent->where))
			ExitToShell ();
//			HideWindow(shellWindow);
		break;
	}
}

void HandleEvent(void)
{
	int			ok;
	EventRecord	theEvent;

	HiliteMenu(0);
	SystemTask ();		/* Handle desk accessories */
	
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
				else
					HandleKeyHits (&theEvent);
			break;
				
			case updateEvt:
				BeginUpdate(shellWindow);
				DrawBullseye(((WindowPeek) shellWindow)->hilited);
				EndUpdate(shellWindow);
			break;
					
			case activateEvt:
				InvalRect(&shellWindow->portRect);
			break;
		}
}

void main(void)
{
	long tiks;
	
	InitMacintosh();
	SetUpMenus();
	SetUpWindow();
	DoAbout ();
	Delay (180L, &tiks);
	for (;;)
		HandleEvent();
}
