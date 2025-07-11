/* macif.c - macintosh interface routines - rhn@netcom.com  */

void (*OnQuit)(), (*OnMouse)();
extern void do_pass(), on_undo();
extern int on_black(), on_white(), on_bigger(), on_smaller(), on_more(), on_less();

char *AboutMenu = "About ...;-";
char *AboutText = "93Oct01";

int sound_ok = 1;
int randomize_flag = 0;
void macinit(char *d1,int x,int y);	/* initialization	*/
void DoEvent();

#include <Types.h>
#include <QuickDraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <StandardFile.h>
#include <Memory.h>
#include <Files.h>
#include <GestaltEqu.h>

#define MenuBarHeight	20
#define ScreenMargin	4

#define appleID		1
#define fileID		256
#define editID		257
#define controlID	258

static void SetupMenus();
static void DoMouseDown();
static void DoMenuClick();
static void DoDrag();
static void DoGoAway();
static void DoContent();
static void DoKeyPress();
static void DoActivate();
static void DoUpdate();
static void DoMenuCommand();
static void DoAppleMenu();
static void DoFileMenu();
static void DoEditMenu();
static void DoControlMenu();

extern char *PtoCstr();
static void none() { return; } 	/* null function */ 

int screenWidth;
int screenHeight;
int nHorizontal,nVertical,nWidth,nHeight;
static WindowRecord myWrecord;
WindowPtr myWindow;
static CursHandle myCursor;

static MenuHandle appleMenu;
static MenuHandle fileMenu;
static MenuHandle editMenu;
static MenuHandle controlMenu;

static long	gestaltflag;

void macinit(WinTitle ,winx, winy)
char *WinTitle;
int winx,winy;
{
    Rect 	myBoundsRect;
	
    MaxApplZone();
    InitGraf(&thePort);
    InitFonts();
    FlushEvents (everyEvent,0);  /* ignore left over events */
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs(0L);
    InitCursor();

    SetupMenus();

    screenWidth  = screenBits.bounds.right  - screenBits.bounds.left;
    screenHeight = screenBits.bounds.bottom - screenBits.bounds.top;

    SetRect(&myBoundsRect,16, 42, 48, 74);
    myWindow = NewWindow(&myWrecord,&myBoundsRect,CtoPstr(WinTitle),
                        FALSE,documentProc,(WindowPtr)-1,TRUE,0);
    SetPort(myWindow);

    TextFont(4); TextSize(9); TextMode(srcCopy);
    SetFontLock(-1);
    InitWin(winx,winy);

    if ( (Gestalt(gestaltSystemVersion, &gestaltflag) != noErr) ||
		((gestaltflag & 0xffff) < 0x0605) ) {
	gestaltflag = 0;
    }
    
    OnQuit=none; OnMouse=none;

    FlushEvents (everyEvent,0);
}

static void SetupMenus()
{
    appleMenu = NewMenu(appleID,CtoPstr("\24"));
    AppendMenu(appleMenu, CtoPstr(AboutMenu));
    AddResMenu(appleMenu,'DRVR');
    InsertMenu(appleMenu,0);
    
    fileMenu = NewMenu(fileID,CtoPstr("File")); /* the file menu */
    AppendMenu(fileMenu,
        CtoPstr("New Game/N;-;Save;Save as...;-;Print;-;Quit/Q"));
    InsertMenu(fileMenu,0);
    DisableItem(fileMenu,3);
    DisableItem(fileMenu,4);
    DisableItem(fileMenu,6);
    
    editMenu = NewMenu(editID,CtoPstr("Edit"));	/* the edit menu */
    AppendMenu(editMenu, CtoPstr("Undo/Z;-;Cut/X;Copy/C;Paste/V;Clear"));
    InsertMenu(editMenu,0);

    controlMenu = NewMenu(controlID,CtoPstr("Control"));
    AppendMenu(controlMenu,
      CtoPstr(
        "pass/P;Sound;-;play black;play white;play both;bigger game/B;smaller game/S;more handicap/M;less handicap/L;randomize"));
    InsertMenu(controlMenu,0);
    CheckItem(controlMenu,2,sound_ok);
    CheckItem(controlMenu,5,1);

    DrawMenuBar();
}

void Idle(t)
int t;
{
    EventRecord	myEvent;
    long int t0, t1;
    t0 = TickCount();
    t1 = t0 + t;
    while (TickCount() < t1) {
      SystemTask();
      if (gestaltflag == 0) GetNextEvent(0,&myEvent);
      else 		  WaitNextEvent(0,&myEvent,0L,0L);
    }
}

void DoEvent()
{
    EventRecord	myEvent;
    Point	aPoint;
    
    SystemTask();

    while (  gestaltflag == 0 ?
               GetNextEvent(everyEvent,&myEvent) :
               WaitNextEvent(everyEvent,&myEvent,0L,0L)  ) {
	switch (myEvent.what) {
	    case mouseDown:
		DoMouseDown(&myEvent);
		break;
	    case keyDown:
	    case autoKey:
		DoKeyPress(&myEvent);
		break;
	    case activateEvt:
		DoActivate(&myEvent);
		break;
	    case updateEvt:
		DoUpdate(&myEvent);
		break;
	    case diskEvt:
		if ( HiWord(myEvent.message) != noErr ) {
		  SetPt(&aPoint, 0x070, 0x050);
		  DIBadMount(aPoint, myEvent.message);
		}
		break;
	    }
	}
}

static void DoMouseDown(myEvent)
  EventRecord *myEvent;
{
    WindowPtr whichWindow;

    switch (FindWindow(myEvent->where,&whichWindow)) {
    case inMenuBar:
	DoMenuClick(myEvent);
	break;
    case inSysWindow:
	SystemClick(myEvent,whichWindow);
	break;
    case inDrag:
	DoDrag(myEvent,whichWindow);
	break;
    case inGoAway:
	DoGoAway(myEvent,whichWindow);
	break;
    case inGrow:
	break;
    case inContent:
	DoContent(myEvent,whichWindow);
	break;
    }
}

static void DoMenuClick(myEvent)
  EventRecord *myEvent;
{
    long choice;
    if (choice = MenuSelect(myEvent->where))
	DoMenuCommand(choice);
}

static void DoDrag(myEvent,whichWindow)
  EventRecord *myEvent;
  WindowPtr whichWindow;
{
    Rect dragRect;
    SetRect(&dragRect,0,MenuBarHeight,screenWidth,screenHeight);
    InsetRect(&dragRect,ScreenMargin,ScreenMargin);
    DragWindow(whichWindow,myEvent->where,&dragRect);
}

static void DoGoAway(myEvent,whichWindow)
  EventRecord *myEvent;
  WindowPtr whichWindow;
{
    if (TrackGoAway(whichWindow,myEvent->where)) {
	if (OnQuit != none) (*OnQuit)();
	else {
          TEToScrap();
	  ExitToShell();
	}
    }
}

static void DoContent(myEvent,whichWindow)
  EventRecord *myEvent;
  WindowPtr whichWindow;
{
   Point clickLoc;
	
    if (whichWindow != FrontWindow())
	SelectWindow(whichWindow);
    else {
      if (whichWindow == myWindow)
        if (OnMouse != none) (*OnMouse)(clickLoc.h, clickLoc.v, myEvent->modifiers);
        else {
          myCursor = GetCursor (watchCursor);  SetCursor (*myCursor);
   	  clickLoc = myEvent->where;
   	  GlobalToLocal(&clickLoc);
          clicktask(clickLoc.h, clickLoc.v, myEvent->modifiers);
          InitCursor();
        } 
      }
}

static void DoKeyPress(myEvent)
  EventRecord *myEvent;
{
    long choice;
    
    if (FrontWindow() == myWindow) {
	if (myEvent->modifiers & 0x100) {
	    if (choice = MenuKey((char)myEvent->message))
		DoMenuCommand(choice);
	    }
	else if ((choice = (char)myEvent->message) == 32 ||
	          choice == 'p' || choice == 'P' || choice == 13) {
	  myCursor = GetCursor (watchCursor);  SetCursor (*myCursor);
          do_pass();
          InitCursor();
        }
        else MyBeep(2);
    	}
}

static void DoActivate(myEvent)
  EventRecord *myEvent;
{
    WindowPtr whichWindow;
    whichWindow = (WindowPtr)myEvent->message;
    SetPort(whichWindow);
    if (whichWindow == myWindow)
	DrawGrowIcon(whichWindow);
}

static void DoUpdate(myEvent)
  EventRecord *myEvent;
{
    WindowPtr whichWindow;
    GrafPtr savePort;
    GetPort(&savePort);
    whichWindow = (WindowPtr)myEvent->message;
    SetPort(whichWindow);
    BeginUpdate(whichWindow);
    EraseRect(&whichWindow->portRect);
    if (whichWindow == myWindow) {
	DrawGrowIcon(whichWindow);
	RedrawScreen();
    }
    EndUpdate(whichWindow);
    SetPort(savePort);
}

static void DoMenuCommand(choice)
  long choice;
{
    short theMenu,theItem;
    
    theMenu = HiWord(choice);
    theItem = LoWord(choice);
    
    HiliteMenu(theMenu);
    switch (theMenu) {
    case appleID:
	DoAppleMenu(theItem);
	break;
    case fileID:
	DoFileMenu(theItem);
	break;
    case editID:
	DoEditMenu(theItem);
	break;
    case controlID:
	DoControlMenu(theItem);
	break;
    }
    HiliteMenu(0);
}

static void DoAppleMenu(theItem)
  short theItem;
{
    unsigned char name[256];
    GrafPtr gp;

    switch (theItem) {
    case 1:
	doaboutbox(AboutText);
	break;
    default:
	GetItem(appleMenu,theItem, name);
	GetPort(&gp);
	OpenDeskAcc(name);
	SetPort(gp);
	break;
    }
}

static void DoFileMenu(theItem)
  int theItem;
{
    switch (theItem) {
    case 1:	/*  */
	on_new();
	break;
    case 8:	/* Quit */
	if (OnQuit != none) (*OnQuit)();
	else {
	  ExitToShell();
	}
	break;
    default:
	break;
    }
}

static void DoEditMenu(theItem)
  int theItem;
{
  if (SystemEdit(theItem-1)) return;
  else switch (theItem) {
    case 1:	/*  */
	on_undo();
	break;
    default:
	MyBeep(2);
	break;
  }
}

static void DoControlMenu(theItem)
  int theItem;
{
    HiliteMenu(0);
    switch (theItem) {
    case 1:
        myCursor = GetCursor (watchCursor);  SetCursor (*myCursor);
	do_pass();
        InitCursor();
	break;
    case 2:	/* continue */
        sound_ok = !sound_ok;
        CheckItem(controlMenu,2,sound_ok);
	break;
    case 4:	/*  */
        on_black();
        CheckItem(controlMenu,4,1);
        CheckItem(controlMenu,5,0);
        CheckItem(controlMenu,6,0);
	break;
    case 5:	/*  */
	on_white();
        CheckItem(controlMenu,4,0);
        CheckItem(controlMenu,5,1);
        CheckItem(controlMenu,6,0);
      	break;
    case 6:	/*  */
	on_both();
        CheckItem(controlMenu,4,0);
        CheckItem(controlMenu,5,0);
        CheckItem(controlMenu,6,1);
      	break;
    case 7:	/*  */
	on_bigger();
	break;
    case 8:	/*  */
	on_smaller();
	break;
    case 9:	/*  */
	on_more();
	break;
    case 10:	/*  */
	on_less();
	break;
    case 11:	/*  */
        randomize_flag = !randomize_flag;
        CheckItem(controlMenu,11,randomize_flag);
        on_randomize(randomize_flag);
	break;
    }
}

InitWin(xx,yy)
int xx,yy;
{
    Rect rect;

    nHorizontal = 20;
    nVertical = 20 + 2 * MenuBarHeight;
    nWidth = xx;
    nHeight = yy;
    ShowHide(myWindow,0);
    MoveWindow(myWindow,nHorizontal,nVertical,-1);
    SizeWindow(myWindow,nWidth,nHeight,-1);
    InvalRect(&myWindow->portRect);
    ShowHide(myWindow,-1);
    rect = myWindow->portRect;
    EraseRect(&rect);
}

MyBeep(int i) { if (sound_ok) SysBeep(i); }

int mystrlen(s) char *s;{ int i; for (i=0; s[i] != 0; i++); return(i); }

DrawCStr(char *s) { DrawText(s,0,mystrlen(s)); }

DrawNum1(int n) { if (n > 0) { DrawNum1(n/10); DrawChar('0'+(n % 10)); }}
DrawNum(int n) { DrawNum1(n/10); DrawChar('0'+(n % 10)); }

doaboutbox(s)
char *s;
{
  Rect r;
  WindowPtr wp;
  WindowRecord wr;
  SetRect(&r,200, 100, 500, 200);
  wp = NewWindow(&wr,&r,"\pAbout ...",
                        TRUE,dBoxProc,(WindowPtr)-1,FALSE,0);
  if (wp == NULL) ExitToShell();
  SelectWindow(wp);
  SetPort(wp); TextFont(1); TextSize(12);
  MoveTo(20,30); DrawCStr(s);
  MoveTo(20,46); DrawCStr("mac port, rhn@netcom.com");
  MoveTo(20,62); DrawCStr("see file README for credits");
  MoveTo(20,78); DrawCStr("see FSF file COPYING for copyright");
  while (!Button()) SystemTask();
  FlushEvents (everyEvent,0);
  CloseWindow(wp);
  SetPort(myWindow);
}

/* end */