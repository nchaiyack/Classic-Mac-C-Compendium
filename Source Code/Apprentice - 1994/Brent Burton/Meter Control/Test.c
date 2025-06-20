/**************
** Test.c
**
** Shows how to use the Meter controls.
***************/

#include <MacHeaders>
#include "Meter.h"

static void Init(void);
static void SetupMenus(void);
static void Cleanup(void);
static void DoCommand(long mResult);
static void DoKeyDown(EventRecord *theEvent);
static void DoMouseDown(EventRecord *theEvent);
static void DoControl( EventRecord *theEvent, WindowPtr);
static void DoAbout(void);
static void DoUpdate(EventRecord *theEvent);

MenuHandle gAppleM, gFileM;
int gSleep = 15;
ControlHandle	gDial1, gDial2, gDial3, gDial4;
WindowPtr		gMainWindow;


#define kMenuBar		128
#define kAppleM			128
#define kAppleAboutItem	1
#define kFileM			129
#define kFileQuitItem	1
#define kMainWind		128
#define kAboutDLOG		128
#define kAboutOK		1		/* OK button */

#define kDial1CNTL		128
#define kDial2CNTL		129
#define kDial3CNTL		130
#define kDial4CNTL		131


static void Init(void)
{
	register int i;
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( (ProcPtr)NULL);
	InitCursor();
	for (i=0; i<7; i++) (void)MoreMasters();	
	FlushEvents(everyEvent, 0);
	SetEventMask(everyEvent);
} // Init()



static void SetupMenus(void)
{
	Handle mbar = GetNewMBar( kMenuBar);
	if (mbar != NULL)
		SetMenuBar( mbar);
	else
	{
		SysBeep(1); SysBeep(1);
		ExitToShell();
	}
	gAppleM = GetMenu( kAppleM);
	if (gAppleM == NULL)
	{
		SysBeep(1); SysBeep(1);
		ExitToShell();
	}
	AddResMenu( gAppleM, 'DRVR');
	DrawMenuBar();
} // SetupMenus()


main()
{
	EventRecord theEvent;
	
	Init();
	SetupMenus();
	
	gMainWindow = GetNewCWindow( kMainWind, NULL, (WindowPtr) -1L);
	if (gMainWindow == NULL)
	{
		SysBeep(5); SysBeep(5); SysBeep(5);
		ExitToShell();
	} // if

	gDial1 = GetNewControl( kDial1CNTL, gMainWindow);
	gDial2 = GetNewControl( kDial2CNTL, gMainWindow);
	gDial3 = GetNewControl( kDial3CNTL, gMainWindow);
	gDial4 = GetNewControl( kDial4CNTL, gMainWindow);
	SetPort( gMainWindow);
	ShowWindow(gMainWindow);
	SelectWindow(gMainWindow);
	DrawControls(gMainWindow);
	
	while (1)						// wait for mouse button
		if (WaitNextEvent(everyEvent, &theEvent, gSleep, NULL))
		{
			switch (theEvent.what) {
				case mouseDown:
					DoMouseDown( &theEvent);
					break;
				case keyDown:
				case autoKey:
					DoKeyDown( &theEvent);
					break;
				case updateEvt:
					DoUpdate( &theEvent);
					break;
			} // switch 
		} // if

} // main()



static void Cleanup(void)
{
	KillControls( gMainWindow);
	DisposeWindow( gMainWindow);
	ExitToShell();
} // Cleanup()


static void DoCommand(long mResult)
{
	int theItem = LoWord(mResult);
	int theMenu = HiWord(mResult);
	Str255 name;
	int temp;
	
	switch (theMenu) {
		case kAppleM:
			if (theItem == kAppleAboutItem) {
				DoAbout();
			} else {
				GetItem( gAppleM, theItem, name);
				temp = OpenDeskAcc(name);
			}
			break;
			
		case kFileM:
			switch (theItem) {
				case kFileQuitItem:
					Cleanup();
					break;
			} /* switch theItem */
			break;
			
	} /* switch theMenu */
	HiliteMenu(0);		/* unhilite the selected menu */
} // DoCommand()


static void DoKeyDown(EventRecord *theEvent)
{
	char theChar;
	static int state=0;
	static int values[] = { 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160,
							176, 192, 208, 224, 240, 256};
#define maxStates 17
	
	theChar = (char)theEvent->message & charCodeMask;
	if (theEvent->modifiers & cmdKey)
		DoCommand( MenuKey( theChar));
	else	
	{
		if (theChar == '+')
		{	if (++state == maxStates) state = 0; }
		else
		if (theChar == '-')
			if (--state == -1) state = maxStates-1;

		SetCtlValue( gDial1, values[state]);
		SetCtlValue( gDial2, values[state]);
		SetCtlValue( gDial3, values[state]);
		SetCtlValue( gDial4, values[state]);
	}
} // DoKeyDown()


static void DoMouseDown(EventRecord *theEvent)
{
	WindowPtr whichWindow;
	Rect dragRect;
	int  part;
	
	dragRect = qd.screenBits.bounds;
	InsetRect( &dragRect, -4, -4);
	
	part = FindWindow(theEvent->where, &whichWindow);
	
	switch(part) {
		case inMenuBar:
			DoCommand( MenuSelect(theEvent->where) );
			break;
		case inContent:
			DoControl( theEvent, whichWindow);
			break;
		case inDrag:
			DragWindow( whichWindow, theEvent->where, &dragRect);
			break;
		case inGrow:
			break;
		case inGoAway:
			if (TrackGoAway( whichWindow, theEvent->where) )
				Cleanup();
			break;
	} // switch
} // DoMouseDown()


static void DoControl( EventRecord *theEvent, WindowPtr whichWindow)
{
	ControlHandle theControl;
	int part, newpart;
	int value;
	
	GlobalToLocal( &theEvent->where);
	part = FindControl( theEvent->where, whichWindow, &theControl);
	if ( !part)
		return;
	
/*
**  This code here (checking for the command key) doesn't work because
**  of some quirk I've not figured out with the Control Manager.  The
**  control works perfectly as planned/noted otherwise, but you just
**  can't drag it around.  Bummer, but IM is very vague about some things.
**
	if ( theEvent->modifiers & cmdKey)
	{
		DragControl( theControl, theEvent->where, &whichWindow->portRect,
					&whichWindow->portRect, noConstraint);
		DrawControls( whichWindow);
	}
**
*/

	switch( part) {
	
	case inNeedle:		/* same as inThumb for scroll bars */
		TrackControl( theControl, theEvent->where, NULL);
		break;
		
	case inTurnDown:		/* same as inPageUp for scroll bars */
		HiliteControl( theControl, inTurnDown);
		while (StillDown())
		{
			Point p;
			GetMouse( &p);
			if ( part == (newpart=TestControl( theControl, p)) )
			{
				value = GetCtlValue( theControl);
				SetCtlValue(theControl, value - 4);
			}
		}
		HiliteControl( theControl, 0);
		break;
		
	case inTurnUp:		/* same as inPageDown for scroll bars */
		HiliteControl( theControl, inTurnUp);
		while (StillDown())
		{
			Point p;
			GetMouse( &p);
			if ( part == (newpart=TestControl( theControl, p)) )
			{
				value = GetCtlValue( theControl);
				SetCtlValue(theControl, value + 4);
			}
		}
		HiliteControl( theControl, 0);
		break;
		
	case inUpButton:				/* not implemented for meters */
		HiliteControl( theControl, inUpButton);
		while (StillDown())
		{
			value = GetCtlValue( theControl);
			SetCtlValue(theControl, value - 2);
		}
		HiliteControl( theControl, 0);
		break;
		
	case inDownButton:				/* not implemented for meters */
		HiliteControl( theControl, inDownButton);
		while (StillDown())
		{
			value = GetCtlValue( theControl);
			SetCtlValue(theControl, value + 2);
		}
		HiliteControl( theControl, 0);
		break;
	}
} // DoControl()



static void DoAbout(void)
{
	DialogPtr	about;
	int 		item = 99;
	
	about = GetNewDialog( kAboutDLOG, NULL, (WindowPtr)-1L);
	if (about == NULL)
	{ SysBeep(1); return; }
	
	ShowWindow( about);
	while ( item != kAboutOK)
		ModalDialog( NULL, &item);
	DisposDialog( about);
} /* DoAbout() */


static void DoUpdate(EventRecord *theEvent)
{
	WindowPtr whichWindow = (WindowPtr)theEvent->message;
	BeginUpdate( whichWindow);
	DrawControls( whichWindow);
	EndUpdate( whichWindow);
}