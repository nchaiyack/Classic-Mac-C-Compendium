
//============================================================================
//----------------------------------------------------------------------------
//
//									UNIMAC
//
//----------------------------------------------------------------------------
//============================================================================


//========================================================  Includes

#include <Palettes.h>

//========================================================  Defines

#define kNilPointer			0L
#define ScreenDepth(gdh)	((*((*gdh)->gdPMap))->pixelSize)
#define MaxColors(gdh)	(1<<ScreenDepth(gdh))

//========================================================  Prototypes

void ToolBoxInit (void);
void OpenMainWindow (void);
void RedrawWindow (void);
void main (void);

//========================================================  Global Variables

Rect			windowRect;
WindowPtr		mainWindow;
PaletteHandle	thePalette;
CTabHandle		colorTable;
Boolean			quitting;

//========================================================  Functions
//--------------------------------------------------------  ToolBoxInit

void ToolBoxInit (void)

{
	short		count;
	
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(kNilPointer);
	InitCursor();
	
	MoreMasters();
	MoreMasters();
}

//--------------------------------------------------------  RandomInt

short RandomInt (short range)

{
	register long	rawResult;
	
	rawResult = Random();
	if (rawResult < 0)
		rawResult *= -1;
	return (rawResult * (long)range / 32768);
}

//--------------------------------------------------------  OpenMainWindow

void OpenMainWindow (void)

{
	#define			kCLUTResID		128
	
	colorTable = GetCTable(kCLUTResID);
	
	windowRect = screenBits.bounds;
	windowRect.bottom -= 20;
	mainWindow = kNilPointer;
	mainWindow = NewCWindow(kNilPointer, &windowRect, "\pSample", TRUE, plainDBox, 
			(WindowPtr)-1L, TRUE, 0L);
	SetPort(mainWindow);
	MoveWindow(mainWindow, 0, 20, TRUE);
	
	thePalette = NewPalette(32, colorTable, pmAnimated, 0x1500);
	SetPalette((WindowPtr)mainWindow, thePalette, TRUE);
}

//--------------------------------------------------------  RedrawWindow

void RedrawWindow (void)

{
	Rect		theLight;
	short		i, j, horiLights, vertLights;
	
	SetPort(mainWindow);
	FillRect(&windowRect, black);
	
	horiLights = (screenBits.bounds.right - screenBits.bounds.left) / 32;
	vertLights = (screenBits.bounds.bottom - screenBits.bounds.top) / 32 - 1;
	
	for (i = 0; i < horiLights; i++)
	{
		for (j = 0; j < vertLights; j++)
		{
			SetRect(&theLight, 0, 0, 32, 32);
			OffsetRect(&theLight, i * 32, j * 32);
			InsetRect(&theLight, 1, 1);
			PmForeColor(RandomInt(30) + 1);
			PaintRect(&theLight);
		}
	}
}

//--------------------------------------------------------  BlinkLights

void BlinkLights (void)

{
	RGBColor	takeColor, putColor;
	short		taking, putting;
	
	taking = RandomInt(30) + 1;
	putting = RandomInt(30) + 1;
	
	GetEntryColor (thePalette, taking, &takeColor);
	GetEntryColor (thePalette, putting, &putColor);
	
	AnimateEntry (mainWindow, taking, &putColor);
	AnimateEntry (mainWindow, putting, &takeColor);
	
//	Palette2CTab (thePalette, colorTable);
}

//--------------------------------------------------------  main

void main (void)

{
	Rect			newRectangle;
	EventRecord		theEvent;
	GDHandle		thisGDevice;
	MenuHandle		appleMenu;
	WindowPtr		whichWindow;
	long			dummyLong, sleep = 10;
	char			theKey;
	
	ToolBoxInit();
	
	appleMenu = GetMenu(128);
	AddResMenu(appleMenu,'DRVR');
	InsertMenu(appleMenu,0);
	DrawMenuBar();
	
	thisGDevice = GetGDevice();
	OpenMainWindow();
	
	quitting = FALSE;
	
	while (!quitting)
	{
		if (WaitNextEvent(everyEvent, &theEvent, sleep, kNilPointer))
		{
			switch (theEvent.what)
			{
				case mouseDown:
				{
					if (FindWindow(theEvent.where, &whichWindow) == inMenuBar)
					{
						dummyLong = MenuSelect(theEvent.where);
						HiliteMenu(0);
					}
				}
				break;
				
				case keyDown:
				case autoKey:
				{
					theKey = BitAnd(theEvent.message, charCodeMask);
					if ((BitAnd(theEvent.modifiers, cmdKey) != 0) && (theKey == 'q'))
						quitting = TRUE;
				}
				break;
				
				case updateEvt:
				{
					if ((WindowPtr)theEvent.message == mainWindow)
					{
						SetPort(mainWindow);
						BeginUpdate((GrafPtr)theEvent.message);
						RedrawWindow();
						EndUpdate((GrafPtr)theEvent.message);
					}
				}
				break;
				default:
				{
				}
			}
		}
		
		BlinkLights();
	}
	
	if (mainWindow != kNilPointer)
		DisposeWindow(mainWindow);
	RestoreDeviceClut(thisGDevice);
	FlushEvents(everyEvent, 0);
}