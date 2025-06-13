// cicn animation demo by Steve Chamberlin.
// ported to CodeWarrior by Ken Long

#include "QDOffscreen.h"

MenuHandle	appleMenu, fileMenu, editMenu, extraMenu;

#define appleID		1
#define fileID		256
#define editID		257
#define extraID		258

#define openItem	1
#define	closeItem	2
#define quitItem	4

#define WIDTH		200
#define HEIGHT		150
#define ITEMWIDTH	32
#define ITEMHEIGHT	32

Rect		bRect, items [20], bbox;
short		x [20], y [20], vx [20], vy [20], color [20], max = 5;
GWorldPtr	offscreen;
CIconHandle			cicn [12];
PixPatHandle		myPat;
Boolean				Is32Bit ();

WindowPtr		myWindow;
CWindowRecord 	wRecord;
Rect			dragRect;

void SetUpWindow (void);
void DrawMy (Boolean hilited);
void SetUpMenus (void);
void AdjustMenus (void);
void MyEnable (MenuHandle menu, short item, Boolean ok);
void HandleMenu (long mSelect);
Boolean Is32Bit (void);
void InitMacintosh (void);
void HandleMouseDown (EventRecord * theEvent);
void HandleEvent (void);
short RandomInt (short range);
void SetUpItems (void);
void LoadSwill (void);
void SetUpOffscreen (void);
void DrawFrame (void);
void Animate (void);
void main (void);

void SetUpWindow ()
{
	dragRect = qd.screenBits.bounds;

	myWindow = GetNewCWindow (256, &wRecord, (WindowPtr) - 1L);
	SetPort (myWindow);
}

void DrawMy (Boolean hilited)
{
	//¥ put redraw window code here, if needed
}

void SetUpMenus ()
{
	appleMenu = GetMenu (appleID);
	fileMenu = GetMenu (fileID);
	editMenu = GetMenu (editID);
	extraMenu = GetMenu (extraID);
	AddResMenu (appleMenu, 'DRVR');
	InsertMenu (appleMenu, 0);
	InsertMenu (fileMenu, 0);
	InsertMenu (editMenu, 0);
	InsertMenu (extraMenu, 0);
	CheckItem (extraMenu, 1, true);
	DrawMenuBar ();
}

void AdjustMenus ()
{
	WindowPeek wp;
	short kind;
	Boolean DA;

	wp = (WindowPeek) FrontWindow ();
	kind = wp ? wp->windowKind : 0;
	DA = kind < 0;

	MyEnable (editMenu, 1, DA);
	MyEnable (editMenu, 3, DA);
	MyEnable (editMenu, 4, DA);
	MyEnable (editMenu, 5, DA);
	MyEnable (editMenu, 6, DA);

	MyEnable (fileMenu, openItem, ! ((WindowPeek) myWindow)->visible);
	MyEnable (fileMenu, closeItem, DA || ((WindowPeek) myWindow)->visible);
}

void MyEnable (MenuHandle menu, short item, Boolean ok)
{
	if (ok)
		EnableItem (menu, item);
	else
		DisableItem (menu, item);
}

void HandleMenu (long mSelect)
{
	short			i, menuID = HiWord (mSelect);
	short			menuItem = LoWord (mSelect);
	Str255		name;
	GrafPtr		savePort;
	WindowPeek	frontWindow;

	switch (menuID)
	{
		case	appleID:
			if (menuItem == 1)
			{
				Alert (9999, 0L);
				break;
			}
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
				case	openItem:
					ShowWindow (myWindow);
					SelectWindow (myWindow);
				break;
	
				case	closeItem:
					if ((frontWindow = (WindowPeek) FrontWindow ()) == 0L)
					break;
					if (frontWindow->windowKind < 0)
						CloseDeskAcc (frontWindow->windowKind);
					else if (frontWindow = (WindowPeek) myWindow)
							HideWindow (myWindow);
 				break;
 	
 				case	quitItem:
					ExitToShell ();
				break;
			}
		break;
	
		case	editID:
			if (! SystemEdit (menuItem - 1))
				SysBeep (5);
		break;

		case	extraID:
			if (menuItem <= 4)
			{
				for (i = 1; i < 5; i++)
					CheckItem (extraMenu, i, false);
		
				max = menuItem * 5;
				CheckItem (extraMenu, menuItem, true);
			}
			if (menuItem == 6)
				SetUpItems ();
				break;
	}
}

Boolean Is32Bit ()
{
	SysEnvRec mySE;

	SysEnvirons (2, &mySE);

	if (! mySE.hasColorQD)
		return (false);

	return (NGetTrapAddress (0xAB03, ToolTrap) !=
			NGetTrapAddress (0xA89F, ToolTrap));
}

void InitMacintosh ()
{
	MaxApplZone ();

	InitGraf (&qd.thePort);
	InitFonts ();
	FlushEvents (everyEvent, 0);
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (0L);
	InitCursor ();

}

void HandleMouseDown (EventRecord * theEvent)
{
	WindowPtr	theWindow;
	short			windowCode;

	windowCode = FindWindow (theEvent->where, &theWindow);
	switch (windowCode)
	{
			case inSysWindow:
				SystemClick (theEvent, theWindow);
			break;
		
			case inMenuBar:
				AdjustMenus ();
				HandleMenu (MenuSelect (theEvent->where));
			break;
		
			case inDrag:
				if (theWindow == myWindow)
						DragWindow (myWindow, theEvent->where, &dragRect);
			break; 	
		
			case inContent:
				if (theWindow == myWindow)
				{
					if (theWindow != FrontWindow ())
						SelectWindow (myWindow);
					else
						InvalRect (&myWindow->portRect);
				}
			break;

			case inGoAway:
				if (theWindow == myWindow &&
					TrackGoAway (myWindow, theEvent->where))
				HideWindow (myWindow);
			break;
	}
}

void HandleEvent ()
{
	EventRecord	theEvent;

	HiliteMenu (0);
	WaitNextEvent (everyEvent, &theEvent, 0L, 0L);
	switch (theEvent.what)
	{
		case mouseDown:
			HandleMouseDown (&theEvent);
			break;
		case keyDown:
		case autoKey:
			if ((theEvent.modifiers & cmdKey) != 0)
			{
				AdjustMenus ();
				HandleMenu (MenuKey ((char) (theEvent.message & charCodeMask)));
			}
			break;
		case updateEvt:
			BeginUpdate (myWindow);
			DrawMy (((WindowPeek) myWindow)->hilited);
			EndUpdate (myWindow);
			break;
		case activateEvt:
			InvalRect (&myWindow->portRect);
			break;
		}
}

short RandomInt (short range)
{
	long rawResult;

	rawResult = Random ();

	if (rawResult < 0)
		rawResult *= - 1;
	
	return ((rawResult * range) / 32768);
}

void SetUpItems ()
{
	short i, n;

	for (i = 0; i < 20; i++)
	{
		x [i] = RandomInt (WIDTH - 2 * ITEMWIDTH) + ITEMWIDTH;
		y [i] = RandomInt (HEIGHT - 2 * ITEMHEIGHT) + ITEMHEIGHT;
		while ((n = RandomInt (13) - 6) == 0);
		vx [i] = n;
		while ((n = RandomInt (13) - 6) == 0);
		vy [i] = n;
		color [i] = RandomInt (12);
		SetRect (&items [i], x [i], y [i], x [i] + ITEMWIDTH, y [i] + ITEMHEIGHT);
	}
}

void LoadSwill ()
{
	short i;

	for (i = 0; i < 12; i++)
		cicn [i] = GetCIcon (256 + i);

	myPat = GetPixPat (256);
}

void SetUpOffscreen ()
{
	GDHandle	saveGD;
	GWorldPtr	saveGW;
	QDErr		errcode;

	SetRect (&bRect, 0, 0, WIDTH, HEIGHT);

	GetGWorld (&saveGW, &saveGD);
	errcode = NewGWorld (&offscreen, 8, &bRect, 0L, 0L, 0L);
	if (errcode != noErr)		//¥ could not create new GWorld
	{
		SysBeep (1);
		ExitToShell ();
	}
	LockPixels (offscreen->portPixMap);
	SetGWorld (offscreen, 0L);
	EraseRect (&bRect);
	SetGWorld (saveGW, saveGD);
	UnlockPixels (offscreen->portPixMap);
}

void DrawFrame ()
{
	GDHandle 	saveGD;
	GWorldPtr	saveGW;

	GetGWorld (&saveGW, &saveGD);
	LockPixels (offscreen->portPixMap);
	SetGWorld (offscreen, 0L);
	
	FillCRect (&bRect, myPat);
	Animate ();
	
	SetGWorld (saveGW, saveGD);
	SetPort (myWindow);
	
	CopyBits ((BitMap*)*offscreen->portPixMap, 
			   &myWindow->portBits, 
			   &bRect,
			   &bRect, srcCopy, 0L);
			
	UnlockPixels (offscreen->portPixMap);
}

void Animate ()
{
	short i;

	for (i = 0; i < max; i++)
	{
		PlotCIcon (&items [i], cicn [color [i]]);

		if (x [i] > WIDTH - ITEMWIDTH || x [i] < 0)
			vx [i] *= - 1;
		if (y [i] > HEIGHT - ITEMHEIGHT || y [i] < 0)
			vy [i] *= - 1;
		OffsetRect (&items [i], vx [i], vy [i]);
		x [i] = items [i].left;
		y [i] = items [i].top;
	}
}

void main ()
{
	short i;

	InitMacintosh ();
	if (! Is32Bit ()) 	//¥ must have 32 bit Quickdraw
	{
		StopAlert (400, 0L);
		ExitToShell ();
	}
	SetUpMenus ();
	SetUpWindow ();

	GetDateTime ((unsigned long*) qd.randSeed);

	SetUpOffscreen ();
	SetUpItems ();
	LoadSwill ();

	for (; ; )
	{
		HandleEvent ();
		DrawFrame ();
	}
}

