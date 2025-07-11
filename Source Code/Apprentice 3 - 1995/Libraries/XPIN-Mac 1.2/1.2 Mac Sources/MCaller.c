/*
  @Project: The cross-platform plug-in toolkit (XPIN).
  @Project-Version: 1.2
  @Project-Date: December 1992.
  @Filename: MCaller.C - The Macintosh Sample Caller program

  @Author: Ramin Firoozye - rp&A Inc.
  @Copyright: (c) 1992, rp&A Inc. - San Francisco, CA.
  @License.
  The portable plug-in toolkit source and library code is
  presented without any warranty or assumption of responsibility
  from rp&A Inc. and the author. You can not sell this toolkit in
  source or library form as a standalone product without express
  permission of rp&A Inc. You are, however, encouraged to use this
  code in your personal or commercial application without any
  royalties. You can modify the sources as long as the license
  and credits are included with the source.

  For bugs, updates, thanks, or suggestions you can contact the
  author at:
	CompuServe: 70751,252
	Internet: rpa@netcom.com

  @End-License.

  @Description.
  This is a sample caller program for the Macintosh version of the XPIN
  toolkit. It's doesn't do anything other than demonstrate that the
  toolkit works. If no plug-in's are located on the home directory, nothing
  happens. But if one or more valid ones are found, it adds a new "Plug-in"
  menu to the menubar and adds the label of each plug-in to the menu.
  When the item in the menu is invoked, the toolkit is used to call the 
  plug-in. The XBlock can be used to pass parameters back and forth.
  
  NOTE: If you include XCONFIG.H as the first include file, you get a whole
  bunch of #define's that can be used with #ifdef's to do compile-time
  checking on OS, COMPILER, and LANGUAGE settings... Makes cross-platform
  compiles so much easier. Read the text for more detail.
  
  NOTE: The current version of the file was compiled with no errors under 
  both MPW C 3.2.4 and Think C 5.0.4.

  @End-Description.
*/

/* Your basic Macintosh application include files */
#include "XCONFIG.h"
#include <QuickDraw.h>
#include <Traps.h>
#include <ToolUtils.h>
#include <Desk.h>
#include <Windows.h>
#include <Types.h>
#include <Memory.h>
#include <Events.h>
#include <OSEvents.h>
#include <Fonts.h>
#include <Menus.h>
#include <Dialogs.h>
#include <TextEdit.h>
#include <Folders.h>
#include <Packages.h>
#ifdef COMPILER_MPW
#include <Strings.h>
#endif

/* Every plug-in aware application  needs to add this header */
#include "XPINCALL.h"

/* Shared settings between MCaller.c and MCaller.r */
#include "MCaller.h"

#define	kMainWindow		128
#define	kAbout			128
#define	kSleep					20L
#define	kMoveToFront		(WindowPtr) -1L
#define	okButton				1
Boolean		gDone;

/* These settings are global. gHaveXPIN is a nicety so we don't have to
   keep calling XPINCount everytime to see if any plug-ins were loaded.
   The pluginMenu menu is created at runtime only if plug-ins are found.
   Your application may just add items to an existing menu. The menu handle
   has to be deallocated before leaving.
   xpin is the most important item. You need one of these per type of plug-in.
   The pointer to the structure is kept global. The actual structure is
   allocated on the heap at runtime. It should be deallocated before leaving.
*/
Boolean		gHaveXPIN;
MenuHandle	pluginMenu;
XPIN		*xpin;

/* Function prototypes */
void		Initialize();
void		SetupWindow();
void		SetupMenubar();
void		DoEventLoop();
void		DoOneEvent(EventRecord *event);
void		DoMouseDown(EventRecord *event);
void		DoMenuChoice(long menuItem);
void		DoAppleMenu(short item);
void		DoFileMenu(short item);
void		DoUpdate(EventRecord *event);
void		DoAbout(void);

/* Slight discrepancy between Think-C and MPW-C on the syntax for QuickDraw
   globals.
 */
#ifdef	COMPILER_THINK
#define	QD(i)	(i)
#endif

#ifdef	COMPILER_MPW
#define	QD(i)	(qd.i)
#endif


/*
 ********************************
  Entry point
 ********************************
 */
int main(void)
{
	gHaveXPIN = false;	// assume we don't have any plug-ins
	
	Initialize();		// Initialize toolboxes and the XPIN
	SetupWindow();		// Set up the main window
	SetupMenubar();		// Set up the menu bar
	DoEventLoop();		// go into endless loop until we quit

/* 
   After the event loop returns (only when we've officially quit)
   we check to see if any was created. If so, clean up before leaving
 */
	if (gHaveXPIN) {
		XPINDone(xpin);
		DisposPtr((Ptr) xpin);
		DisposeMenu(pluginMenu);
	}
}


/*
 ********************************************************************
  Toolbox Initializations
 ********************************************************************
*/
void		Initialize()
{
XPINErr	status;		// Return status for XPIN

	InitGraf(&QD(thePort));
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	MaxApplZone();

/* 
   Allocate the structure as a zeroed out pointer on the heap.
   This should be done early in the program to avoid heap fragmentation.
   There should be one structure per plug-in type supported.
   NOTE: The filetype setting to XPINInit is passed as a null-terminated
   string. Ordinarily, this is a four-byte character field. But we want
   to be cross-platform compatible so we use pointers.
   $HOME translates to the application's home directory.
   $SYSTEM translates to the System Folder.
 */
	xpin = (XPIN *) NewPtrClear(sizeof(XPIN));
	status = XPINInit(xpin, "$HOME", "xPIN");
	if (status != XPINOK) // Do error checking here...
		return;
	if (XPINCount(xpin) > 0)	// Did we get any?
		gHaveXPIN = true;
}

/*
 ********************************************************************
  No-nonesense Window setup
 ********************************************************************
*/
void		SetupWindow()
{
WindowPtr	window;

	window = GetNewWindow(kMainWindow, nil, kMoveToFront);
	if (window == nil) {		// If we fail, there's no use continuing
		SysBeep(10);
		ExitToShell();
	}
	
	SetPort(window);
	ShowWindow(window);
}

/*
 ********************************************************************
  Setup the menubar + XPIN
 ********************************************************************
*/

void		SetupMenubar(void)
{
Handle		menuBar;
MenuHandle	menu;
long		xCount, i;
char		*label;
Str255		xName;

	menuBar = GetNewMBar(128);
	SetMenuBar(menuBar);
	menu = GetMHandle(mApple);
	AddResMenu(menu, 'DRVR');

/*
 * Get the number of plug-ins found, if zero, don't add anything to
 * the menu bar. NOTE: XPINLabel returns a pointer to an internal
 * storage area, so you need to make sure you make a copy before
 * messing around with it.
 */
	xCount = XPINCount(xpin); 
	if (xCount > 0) {
		pluginMenu = NewMenu(mPlugin, "\pPlug-in");
		for (i = 0; i < xCount; i++) {		
			label = (char *) XPINLabel(xpin, i);
			strcpy((char *) xName, label);
			C2P(xName);
			InsMenuItem(pluginMenu, xName, i+1);
		}
		InsertMenu(pluginMenu, 0) ;
	}
	DrawMenuBar();
}

/*
 ********************************************************************
  The giant event loop
 ********************************************************************
*/

void		DoEventLoop()
{
EventRecord	event;

	gDone = false;
	while (gDone == false) {
		WaitNextEvent(everyEvent, &event, kSleep, nil);
		DoOneEvent(&event);
	}
}

/*
 ********************************************************************
  Process on event at a time
 ********************************************************************
*/

void		DoOneEvent(EventRecord *event)
{
char	theChar;

	switch(event->what) {
		case mouseDown:
			DoMouseDown(event);
			break;
		case keyDown:
		case autoKey:
			theChar = event->message & charCodeMask;
			if ((event->modifiers & cmdKey) != 0)
				DoMenuChoice(MenuKey(theChar));
			break;
		case updateEvt:
			DoUpdate(event);
			break;
	}
}

/*
 ********************************************************************
  Process all mousedown events
 ********************************************************************
*/

void	DoMouseDown(EventRecord *event)
{
WindowPtr	theWindow;
short		thePart;
long			menuChoice;

	thePart = FindWindow(event->where, &theWindow);
	switch(thePart) {
		case inMenuBar:
			menuChoice = MenuSelect(event->where);
			DoMenuChoice(menuChoice);
			break;
		case inSysWindow:
			SystemClick(event, theWindow);
			break;
		case inDrag:
			DragWindow(theWindow, event->where, &QD(screenBits.bounds));
			break;
		case inGoAway:
			gDone = true;
			break;
		}
}

/*
 ********************************************************************
  Process all menu selection events
 ********************************************************************
*/

void DoMenuChoice(long menuItem)
{
short	menu;
short	item;

XBlock	xb;			// A single XBlock to pass things back and forth
long	xID;		// The XPIN index. NOTE: It's a longword.
short	result;		// The result from the plug-in itself.
XPINErr	status;		// The result from the XPINCall. Should be XPINOK.

	if (menuItem != 0) {
		menu = HiWord(menuItem);
		item = LoWord(menuItem);
		
		switch(menu) {
			case mApple:
				DoAppleMenu(item);
				break;
			case mFile:
				DoFileMenu(item);
				break;
/*
 * NOTE: We don't distinguish between which plug-in we're going to call
 * so everyone gets the same stuff in the XBlock. A proper plug-in app
 * would set up a protocol for what to load into the XBlock for what
 * type of request... We also attach come up with a simplistic way of
 * translating from the menu item to the XPIN index. Since there's nothing
 * other than plug-in labels on this menu and we're just adding them in
 * order, we get away with just subtracting one from the menu item and
 * getting the index. A real app would be better off coming up with a
 * lookup table of some sort to go from the user-interface element ID
 * to the XPIN index...
 */
			case mPlugin:
				xID = (long) (item - 1);
				XSETw(&xb, 0, 100);				// the xtension can get
				XSETw(&xb, 1, 200);				// this through XGETw(xb, 0)
				status = XPINCall(xpin, xID, &xb);	// call it
				result = XGETSTATw(&xb);		// get the result
				break;
			}
		HiliteMenu(0);
	}
}

/*
 ********************************************************************
  Process Apple Menu requests.
 ********************************************************************
*/

void DoAppleMenu(short item)
{
MenuHandle	appleMenu;
Str255		daName;
short		daNumber;

	switch(item) {
		case iAbout:
			DoAbout();
			break;
		default:
			appleMenu = GetMHandle(mApple);
			GetItem(appleMenu, item, daName);
			daNumber = OpenDeskAcc(daName);
			break;
		}
}

/*
 ********************************************************************
  Handle File menu requests.
 ********************************************************************
*/

void	DoFileMenu(short item)
{
/*
   We just handle Quit. It just sets the Done flag and returns.
   This forces the event loop to break out and exit
 */ 
	switch(item) {
		case iQuit:
			gDone = true;
			break;
		}
}
/*
 ********************************************************************
  Miscellaneous update events inside the window
 ********************************************************************
*/

void		DoUpdate(EventRecord *event)
{
	// Do Window Update here
}

/*
 ********************************************************************
  Do the credits
 ********************************************************************
*/

void	DoAbout()
{
pascal Boolean MyDialogFilter(DialogPtr, EventRecord *, short *);
short	itemHit;
DialogPtr	myDlg;

	myDlg = GetNewDialog(kAbout,0, kMoveToFront);
	do {
		ModalDialog( MyDialogFilter, &itemHit );
		} while(itemHit != okButton);
	DisposDialog( myDlg);
}

/*
 ********************************************************************
  Handle the about dialog
 ********************************************************************
*/

pascal Boolean MyDialogFilter(DialogPtr theDlg,EventRecord *theEvent,
	short *itemHit )
{
	if ( theEvent->what != keyDown )
		return(false);

	switch ( (theEvent->message) & charCodeMask ) {
		case 0x0d:	// Enter or Return 
		case 0x03:
		case 0x1b:
			*itemHit = okButton;
			return( true );	
		default:
			return( false );
	}
}


