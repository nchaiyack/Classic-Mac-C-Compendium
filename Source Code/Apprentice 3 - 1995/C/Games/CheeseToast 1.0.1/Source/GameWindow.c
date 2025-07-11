/************************************************************************************
 * GameWindow.c
 *
 * Main Window Routine
 ************************************************************************************/

#include "ObjectWindow.h"
#include "CToast.h"

extern Boolean	gDoneFlag;

#define		MainWIND	128

void MyNewWindow(void);
void MyDraw(WindowPtr theWin);
void MyHandleClick(WindowPtr theWin, Point where);
void MyIdle(WindowPtr theWin, EventRecord *theEvent);
void MyProcessKey(EventRecord *theEvent);


// Create a new main window using a 'WIND' template from the resource fork
//
void MyNewWindow(void)
{
	WindowPtr	theWindow;
	
	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 
	theWindow = InitObjectWindow(MainWIND, NULL,false);
	((ObjectWindowPtr) theWindow)->Draw = MyDraw;
	((ObjectWindowPtr) theWindow)->HandleClick = MyHandleClick;
	((ObjectWindowPtr) theWindow)->Idle = MyIdle;
	((ObjectWindowPtr) theWindow)->ProcessKey = MyProcessKey;
	
	MoveWindow(theWindow,screenBits.bounds.left,screenBits.bounds.top,false);
	SizeWindow(theWindow,screenBits.bounds.right - screenBits.bounds.left,
						 screenBits.bounds.bottom - screenBits.bounds.top,false);

	// Show the window
	ShowWindow(theWindow);

	// Make it the current grafport
	SetPort(theWindow);

	PaintRect(&theWindow->portRect);
	
	InitializeGame(theWindow);
	HideCursor();
}

// Respond to an update event - BeginUpdate has already been called.
//
void MyDraw(WindowPtr theWindow)
{
	extern Ptr	gVideoMem,gScreenMem;
	// Erase the content area
	if (gVideoMem && gScreenMem)
		MyCopyBits();
	else
		PaintRect(&theWindow->portRect);
}


// Respond to a mouse-click - highlight cells until the user releases the button
//
void MyHandleClick(WindowPtr theWin, Point where)
{
	// Mouse is not used in this game
}

// When in "Attract" mode, we use normal event processing
// to get keystrokes.
// While playing game, we use CheckKeys function...
//
void MyProcessKey(EventRecord *theEvent)
{
	short	ch;
	ch = theEvent->message & charCodeMask;
	switch (gGameState) {
	case GS_Attract:
		switch (ch) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			gPrefs.soundLevel = ch - '0';
			if (gPrefs.soundLevel > 7)
				gPrefs.soundLevel = 7;
			SetSoundVol(gPrefs.soundLevel);
			PlaySound(S_Shield, 2);
			SavePreferences();
			break;
		case 'p':
		case 'P':
			BeginGame();
			break;
		case 'k':
		case 'K':
			RemapKeys();
			break;
		case 'q':
		case 'Q':
		case 27:
			gDoneFlag = true;
			break;
		}
	}
}

void MyIdle(WindowPtr theWin, EventRecord *theEvent)
{
	switch (gGameState) {
	case GS_Attract:
		break;
	case GS_Play:
		MainGameLoop(theWin);
		break;
	case GS_GameOver:
		EndGame();
		BeginAttract();
		break;
	}
}

// Draw the startup screen
//
void StartupScreen(void)
{
	CGrafPtr	curPort;
	GDHandle	curDevice;

	GetGWorld(&curPort,&curDevice);

	SetGWorld(gOffScreen,NULL);
	PaintRect(&gOffScreen->portRect);

	DisplayPicture(StartupPICT,-1,-1);

	SetGWorld(curPort,curDevice);
	HideCursor();
	MyCopyBits();
	ShowCursor();
}

// Display picture at provided coordinates
// if coordinates are -1,-1, picture is centered
//
void DisplayPicture(short picNbr, short horiz, short vert)
{
	PicHandle	ph;
	Rect		r;
	ph = (PicHandle) GetResource('PICT', picNbr);
	if (ph) {
		r = (*ph)->picFrame;
		if (horiz == -1)	// Center
			OffsetRect(&r,(gOffScreen->portRect.right - (*ph)->picFrame.right)/2,
						  (gOffScreen->portRect.bottom - (*ph)->picFrame.bottom)/2);
		else
			OffsetRect(&r, horiz, vert);
		DrawPicture(ph,&r);
		// KillPicture(ph);
		ReleaseResource((Handle) ph);
	}
}

