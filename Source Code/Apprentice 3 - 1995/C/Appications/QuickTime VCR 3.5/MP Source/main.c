/*---------------------------------------------------------------------
	File: main.c
	Purpose: 
	
	Created by: Geoffrey Slinker
	Date: 8:26:41 AM  6/8/92
	Modified: 
---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <oops.h>
#include <Movies.h>

#include "Dlog.h"
#include "about_Dlog.h"
#include "Alert_Class.h"
#include "vcr.h"
#include "mycolors.h"
#include "myQuickTime.h"

#define NUKE_ALL_EVENTS	0
#define NULL_POINTER NULL

#define MY_ALERT	601
//#define VCR_DLOG	700
#define VCR_DLOG	702


#define BASE_MENU	600
#define APPLE_ID	600
#define FILE_ID		601
#define VCR_ID		603
#define WINDOW_ID	604

#define ABOUT_ITEM	1


#define OPEN_ITEM	1
#define CLOSE_ITEM	2
#define	QUIT_ITEM	4

#define PLAY_ITEM	1
#define FF_ITEM		3
#define REW_ITEM	4
#define GO_END_ITEM	5
#define GO_START_ITEM	6
#define STOP_ITEM	8
#define V_UP_ITEM	10
#define V_DOWN_ITEM 11


#define SHOW_VCR_ITEM 1
#define SHOW_POSTER_ITEM	2
/*-------------------------
protos
-------------------------*/
void InitMac(void);
void MainLoop(void);
int Eventful(void);
int HandleMouseDown(void);
int HandleMenuChoice (long int choice);
void HandleAppleChoice(int theItem);
int HandleFileChoice(int theItem);
int HandleVCRChoice(int theItem);
int HandleWindowChoice(int theItem);

void MenuBarInit(void);
void InitMyStuff(void);
void CleanUpMyStuff(void);
void ClockTask(void);
void MaintainMenus(void);

/*-------------------------
globals
-------------------------*/
EventRecord gEvent;
MenuHandle gAppleMenu;
about_Dlog *aboutDlog;
Alert_Class *myAlert;
QT *myQT;
Boolean moviesLoaded = false;
VCR *vcr;
Boolean opened = false;

//DateTimeRec oldT;
long oldClockTick; // used for updating vcr clock
extern Boolean blink;
extern long oldTick; // used for updating status

/**********************************************************************/
void main()
{
	InitMac();
	
	MainLoop();
	
	CleanUpMyStuff();
}
/*---------------------------------------------------------------------
	Function: InitMac()
	Purpose: 
	Returns: 
	
	Created by: Geoffrey Slinker
	Date: 8:30:00 AM  6/8/92
	Modified: 
---------------------------------------------------------------------*/
void InitMac()
{
	SysEnvRec thisMac;
	
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent,NUKE_ALL_EVENTS);
	InitWindows();  // This calls InitPalettes
	InitMenus();
	TEInit();
	InitDialogs(NULL_POINTER);
	InitCursor();
	MaxApplZone();
	
	MenuBarInit();
	
	// Is Color QuickDraw available?
	SysEnvirons(1,&thisMac);
	if ( !thisMac.hasColorQD ) {
		
		ParamText("\pColor QuickDraw is needed","\pPlease install Color QuickDraw",
		"\p","\p");	
		StopAlert(602,(ProcPtr)NULL);
		ExitToShell();

	}
	
	InitMyStuff();

	/*-------------------------
	Other things related to the MAC
	or the MAC interface that needs
	to be initiated
	-------------------------*/
	
	(*myQT).MovieCheck();  // Is QuickTime available?
	moviesLoaded = true;
	
	
}
/*---------------------------------------------------------------------
	Function: void MainLoop()
	Purpose: repeatedly call function with getnext event and very
	large switch statement.
	Returns: 
	
	Created by: Geoffrey Slinker
	Date: 8:33:44 AM  6/8/92
	Modified: 
	
	
	
	theErr := FSMakeFSSpec(theSFReply.vRefNum, 0, theSFReply.fName, mySpec);
	
---------------------------------------------------------------------*/
void MainLoop()
{
	AppFile theFile;
	int message, count;
	int response = 1;
	FSSpec mySpec;
	/*-------------------------
	Did they drag and drop?
	-------------------------*/
	
	CountAppFiles(&message,&count);
	if ( (message == 0) && (count >= 1)  ) {
		char stuff[80];
		
		GetAppFiles(1,&theFile);
		//(*myAlert).AlertNote("\pDrag and Drop ocurred",theFile.fName,"\p","\p");
		if ( FSMakeFSSpec(theFile.vRefNum, 0, theFile.fName, &mySpec) != noErr ) {
			(*myAlert).AlertCaution("\pUnable to open from drag and drop",
				"\p","\p","\p");
		} else {
			opened = (*myQT).OpenMovieFSSpec(mySpec);
		}
		ClrAppFiles(1);
	} /* end if */
	

	while (response == 1){
		response = Eventful();
	}
	
}
/*---------------------------------------------------------------------
	Function: Eventful()
	Purpose: Contains GetNextEvent and the switch statement.
	Returns: 
	
	Created by: Geoffrey Slinker
	Date: 8:35:17 AM  6/8/92
	Modified: 
	
---------------------------------------------------------------------*/
int Eventful()
{
	int response = 1;
	char theChar;
	GrafPtr aPort;
	WindowPtr whichWindow;
	Movie myMovie;
	CWindowPtr movieWindow,moviePosterWindow;
	DialogPtr myDlog = (*vcr).myDlog;
	Boolean isDaWindow;
					
	
	
	myMovie = (*myQT).myMovie;
	movieWindow = (*myQT).movieWindow;
	moviePosterWindow = (*myQT).moviePosterWindow;
	
	SystemTask();
	ClockTask();
	(*myQT).MyMoviesTask();
	MaintainMenus();
	
	
	GetNextEvent(everyEvent,&gEvent);
	
	switch ( gEvent.what  ) {
		case mouseDown:
			response = HandleMouseDown();
			break;
		
		case keyDown:
		case autoKey:
			theChar = gEvent.message & charCodeMask;
			if ( (gEvent.modifiers & cmdKey) != 0  ) {
				response = HandleMenuChoice( MenuKey(theChar));
			}
			else {
				whichWindow = FrontWindow();
			}
			break;
		
		case activateEvt:
			SetPort( (WindowPtr) gEvent.message );
			InvalRect(&((WindowPtr)gEvent.message)->portRect);
			break;
			
		case updateEvt:			
			whichWindow = (WindowPtr)gEvent.message;
			if ( whichWindow == NULL_POINTER  ) {
				isDaWindow = false;
				break;
			}
			else {
				isDaWindow = (( (WindowPeek)whichWindow)->windowKind < 0);
			} /* end if then else */
			
			if ( !isDaWindow ) 
			{
				BeginUpdate((WindowPtr)gEvent.message);

				GetPort(&aPort);
				SetPort((WindowPtr)gEvent.message);

					if ( (WindowPtr)gEvent.message == myDlog) {
						(*vcr).RedrawDialog();
					}
					
					if ((CWindowPtr)gEvent.message == movieWindow) {
						if ( (opened) && (myMovie != NULL)){
							UpdateMovie( myMovie );
						}
					}
					
					if ((CWindowPtr)gEvent.message == moviePosterWindow) {
						if ( (opened) && (myMovie != NULL)){
							(*myQT).RedrawPoster();
						}
					}
					
				EndUpdate((WindowPtr)gEvent.message);

				SetPort(aPort);
			} /* end if */

			break;
			
		case nullEvent:
			break;
				
		default: 
			response = 1;
			break;
			
	} /* end switch */


	return(response);
}
/*---------------------------------------------------------------------
	Function: HandleMouseDown
	Purpose: 
	Returns: 
	
	Created by: Geoffrey Slinker
	Date: 8:38:43 AM  6/8/92
	Modified: 
---------------------------------------------------------------------*/
int HandleMouseDown()
{	
	WindowPtr whichWindow,frontWindow;
	short int thePart;
	long int menuChoice, windSize;
	Rect bRect;
	int response = 1;
	Point thePoint;
	
	thePart = FindWindow(gEvent.where,&whichWindow);
	
	switch ( thePart  ) {
		case inMenuBar:
			menuChoice = MenuSelect(gEvent.where);
			response = HandleMenuChoice(menuChoice);
			break;
		
		case inSysWindow:
			SystemClick(&gEvent,whichWindow);
			break;
		
		case inDrag:
			SelectWindow(whichWindow);
			bRect = screenBits.bounds;
			DragWindow(whichWindow,gEvent.where, &bRect);	
			break;
			
		case inContent:
			frontWindow = FrontWindow();
			SelectWindow(whichWindow);
			SetPort(whichWindow);
			if (whichWindow == (*vcr).myDlog) {
				(*vcr).HandleDialog(&gEvent);
			}
			break;
			
		case inGoAway:
			HideWindow(whichWindow);
			break;	
		default: response = 1; break;
	} /* end switch */
	
	return(response);
}
/*---------------------------------------------------------------------
	Function: int HandleMenuChoice(long int choice)
	Purpose: Distribute the menu choice to the proper handler.
	Returns: 
	
	Created by: Geoffrey Slinker
	Date: 8:41:11 AM  6/8/92
	Modified: 
---------------------------------------------------------------------*/
int HandleMenuChoice (long int choice)
{
	int theMenu;
	int	theItem;
	int response;
	char stuff[80];
	Str255 message;
	
	response = 1;
	
	if (choice != 0) {
		theMenu = HiWord(choice);
		theItem = LoWord(choice);
		
		switch ( theMenu  ) {
			case APPLE_ID:
				HandleAppleChoice(theItem);
				break;
			
			case FILE_ID: 
				response = HandleFileChoice(theItem);
				break;
			case VCR_ID:
				HandleVCRChoice(theItem);
				break;
			case WINDOW_ID:
				response = HandleWindowChoice(theItem);
				break;
					
			default: 
				sprintf((char *)message,"Menu Bar Item = %d",theItem);
				(*myAlert).AlertCaution("\pUnimplemented Menu Bar Item",
										CtoPstr(message),"\pSorry!",NULL);
				break;
		} /* end switch */
	} /* end if */
	
	HiliteMenu(0);
	return(response);
}
/*---------------------------------------------------------------------
	Function: HandleAppleChoice
	Purpose: 
	Returns: 
	
	Created by: Geoffrey Slinker
	Date: 8:42:38 AM  6/8/92
	Modified: 
---------------------------------------------------------------------*/
void HandleAppleChoice(int theItem)
{
	Str255	accName;
	int		accNumber;
	
	switch ( theItem  ) {
		case ABOUT_ITEM:
			(*aboutDlog).HandleDialog(&gEvent);
			break;
		default:
			GetItem(gAppleMenu,theItem,accName);
			accNumber = OpenDeskAcc(accName);
			break;
	} /* end switch */
}
/*---------------------------------------------------------------------
	Function: HandleFileChoice
	Purpose: 
	Returns: 
	
	Created by: Geoffrey Slinker
	Date: 8:44:06 AM  6/8/92
	Modified: 
---------------------------------------------------------------------*/
int HandleFileChoice(int theItem)
{
	int response;
	WindowPtr whichWindow;
	Str255 message;
	
	response = 1;
	
	switch ( theItem  ) {
		case OPEN_ITEM: 
			opened = (*myQT).OpenMovie();
			break;
			
		case CLOSE_ITEM:
			(*myQT).CloseMovie();
			opened = false;
			break;
			
		case QUIT_ITEM:	response = 0; break;
		default: 
			sprintf((char *)message,"Menu Bar Item = %d",theItem);
				(*myAlert).AlertCaution("\pUnimplemented Menu Bar Item",
										CtoPstr(message),"\pSorry!",NULL);
			break;
	} /* end switch */
	
	return(response);
}
/*---------------------------------------------------------------------
#define PLAY_ITEM	1
#define FF_ITEM		3
#define REW_ITEM	4
#define GO_END_ITEM	5
#define GO_START_ITEM	6
#define STOP_ITEM	8
	Function: HandleVCRChoice
	Purpose: 
	Returns: 
	
	Created by: Geoffrey Slinker
	Date: 4:29:34 PM  8/1/92
	Modified: 
---------------------------------------------------------------------*/
int HandleVCRChoice(int theItem)
{
	int response;
	WindowPtr whichWindow;
	Str255 message;
	Buttons myButtons;
	DialogPtr myDlog = (*vcr).myDlog;
	GrafPtr oldPort;
	
	GetPort(&oldPort);
	SetPort(myDlog);
	response = 1;
	myButtons = (*vcr).myButtons;
	
	switch ( theItem  ) {
		case PLAY_ITEM: 
			if (myButtons.button[PLAY_BUTTON] == false) {
				(*vcr).ButtonsAndValues(PLAY_BUTTON);
				(*vcr).vcr_mode = playing;
				(*myQT).PlayMovie();
			}
			break;
			
		case FF_ITEM:
			if (myButtons.button[FF_BUTTON] == false) {
				(*vcr).vcr_mode = fastforward;
				(*vcr).ButtonsAndValues(FF_BUTTON);
				(*myQT).FFMyMovie();
			}
			break;
			
		case REW_ITEM:
			if (myButtons.button[RW_BUTTON] == false) {
				(*vcr).vcr_mode = re_wind;
				(*vcr).ButtonsAndValues(RW_BUTTON); 
				(*myQT).RWMyMovie();
			}
			break;
			
		case GO_END_ITEM: 
			if (myButtons.button[GO_END_BUTTON] == false) {
				(*vcr).vcr_mode = gotoend;
				(*vcr).ButtonsAndValues(GO_END_BUTTON);
				myButtons = (*vcr).myButtons;
				myButtons.button[GO_END_BUTTON] = false;
				(*vcr).myButtons = myButtons;
				(*vcr).RedrawButtons();
				(*myQT).GoEndMyMovie();
			}
			break;
			
		case GO_START_ITEM: 
			if (myButtons.button[GO_START_BUTTON] == false) {
				(*vcr).vcr_mode = gotostart;
				(*vcr).ButtonsAndValues(GO_START_BUTTON);
				myButtons = (*vcr).myButtons;
				myButtons.button[GO_START_BUTTON] = false;
				(*vcr).myButtons = myButtons;
				(*vcr).RedrawButtons();
				(*myQT).GoStartMyMovie();
			}
			break;
			
		case STOP_ITEM: 
			if (myButtons.button[STOP_BUTTON] == false) {
				(*vcr).vcr_mode = stopped;
				(*vcr).ButtonsAndValues(STOP_BUTTON);
				myButtons = (*vcr).myButtons;
				myButtons.button[STOP_BUTTON] = false;
				(*vcr).myButtons = myButtons;
				(*vcr).RedrawButtons();
				(*vcr).DrawAllProgress(sl_darkGray);
				(*myQT).StopMyMovie();
			}
			break;
		
		case V_UP_ITEM:
			(*myQT).HandleVolume(UP);
			(*vcr).FlashButton(VOLUME_UP);
			break;
			
		case V_DOWN_ITEM:
			(*myQT).HandleVolume(DOWN);
			(*vcr).FlashButton(VOLUME_DOWN);
			break;
						
		default: 
			sprintf((char *)message,"Menu Bar Item = %d",theItem);
				(*myAlert).AlertCaution("\pUnimplemented Menu Bar Item",
										CtoPstr(message),"\pSorry!",NULL);
			break;
	} /* end switch */
	
	SetPort(oldPort);
	return(response);
}
/*---------------------------------------------------------------------
	Function: HandleWindowChoice
	Purpose: 
	Returns: 
	
	Created by: Geoffrey Slinker
	Date: 4:29:34 PM  8/1/92
	Modified: 
---------------------------------------------------------------------*/
int HandleWindowChoice(int theItem)
{
	int response;
	Str255 message;
	DialogPtr myDlog = (*vcr).myDlog;
	CWindowPtr moviePosterWindow = (*myQT).moviePosterWindow;

	response = 1;
	
	switch ( theItem  ) {
		
		
		case SHOW_VCR_ITEM:
			if ( ((WindowPeek)myDlog)->visible ) {
				HideWindow(myDlog);
			}
			else {
				ShowWindow(myDlog);
				SelectWindow( (WindowPtr)myDlog);
			}
		
	
	
			break;
			
		case SHOW_POSTER_ITEM:
			if ( ((CWindowPeek)moviePosterWindow)->visible ) {
				HideWindow((WindowPtr)moviePosterWindow);
			}
			else {
				ShowWindow((WindowPtr)moviePosterWindow);
				SelectWindow((WindowPtr)moviePosterWindow);
			}
			break;
											
		default: 
			sprintf((char *)message,"Menu Bar Item = %d",theItem);
				(*myAlert).AlertCaution("\pUnimplemented Menu Bar Item",
										CtoPstr(message),"\pSorry!","\pHandleWindowChoice");
			break;
	} /* end switch */
	
	return(response);
}

/*---------------------------------------------------------------------
	Function: void MenuBarInit()
	Purpose: Load the menu resources
	Returns: 
	
	Created by: Geoffrey Slinker
	Date: 9:06:34 AM  6/8/92
	Modified: 
---------------------------------------------------------------------*/
void MenuBarInit()
{
	Handle myBar;	

	if ( (myBar = GetNewMBar(BASE_MENU)) == NULL_POINTER  ) {
		ExitToShell();
	} /* end if */
	
	SetMenuBar(myBar);
	
	if ( (gAppleMenu = GetMHandle( APPLE_ID)) == NULL_POINTER  ) {
		ExitToShell();
	} /* end if */
	
	AddResMenu(gAppleMenu,'DRVR');
	

	DrawMenuBar();
}
/*---------------------------------------------------------------------
	Function: InitMyStuff();
	Purpose: 
	Returns: 
	
	Created by: Geoffrey Slinker
	Date: 9:16:28 AM  6/8/92
	Modified: 
---------------------------------------------------------------------*/
void InitMyStuff()
{
	DialogPtr myDlog;
	
	oldClockTick = oldTick = TickCount();
	aboutDlog = new about_Dlog;
	(*aboutDlog).loadResource(ABOUT_DLOG);
	
	myAlert = new Alert_Class;
	(*myAlert).ChooseResource(MY_ALERT);
	
	vcr = new VCR;
	(*vcr).loadResource(VCR_DLOG);
	(*vcr).PutUpDialog();
	
	myQT = new QT;
	(*myQT).LoadResources(); 
}
/*---------------------------------------------------------------------
	Function: CleanUpMyStuff()
	Purpose: 
	Returns: 
	
	Created by: Geoffrey Slinker
	Date: 9:16:28 AM  6/8/92
	Modified: 
---------------------------------------------------------------------*/
void CleanUpMyStuff()
{
	delete aboutDlog;
	delete myAlert;
	delete vcr;
	delete myQT;
	
	if (moviesLoaded) {
		ExitMovies();
	}
	
}
/*---------------------------------------------------------------------
	Function: ClockTask();
	Purpose: 
	Returns: 
	
	Created by: Geoffrey Slinker
	Date: 4:22:18 PM  6/12/92
	Modified: 
---------------------------------------------------------------------*/
void ClockTask()
{
	long newClockTick;
	Boolean blinky;
	DialogPtr myDlog = (*vcr).myDlog;
	
	// IF the vcr is not visible then we don't need
	// to draw anything!
	if ( ((WindowPeek)myDlog)->visible != 1) {
		return;
	}
	
	blinky = blink;
	newClockTick = TickCount();

	
	if ( (newClockTick - oldClockTick) >= 60) {
	
		blink = !blink;
		oldClockTick = newClockTick;
	}
	if (blink != blinky) (*vcr).RedrawTime();
	(*vcr).UpdateProgress();
}
/*---------------------------------------------------------------------
	Function: void MaintainMenus()
	Purpose: 
	Returns: 
	
	Created by: Geoffrey Slinker
	Date: 10:49:56 AM  6/13/92
	Modified: 
---------------------------------------------------------------------*/
void MaintainMenus()
{
	WindowPtr frontWindow;
	MenuHandle theMenu,vcrMenu;
	DialogPtr myDlog;
	CWindowPtr moviePosterWindow;
	

	moviePosterWindow = (*myQT).moviePosterWindow;
	myDlog = (*vcr).myDlog;
	
	
	theMenu = GetMHandle(FILE_ID);
	vcrMenu = GetMHandle(VCR_ID);
	
	if (opened) {
		DisableItem(theMenu,OPEN_ITEM);
		EnableItem(theMenu,CLOSE_ITEM);
		EnableItem(vcrMenu,PLAY_ITEM);
		EnableItem(vcrMenu,STOP_ITEM);
		EnableItem(vcrMenu,FF_ITEM);
		EnableItem(vcrMenu,REW_ITEM);
		EnableItem(vcrMenu,GO_START_ITEM);
		EnableItem(vcrMenu,GO_END_ITEM);
		EnableItem(vcrMenu,V_UP_ITEM);
		EnableItem(vcrMenu,V_DOWN_ITEM);

	}
	else {
		DisableItem(theMenu,CLOSE_ITEM);
		EnableItem(theMenu,OPEN_ITEM);
		
		DisableItem(theMenu,CLOSE_ITEM);
		DisableItem(vcrMenu,PLAY_ITEM);
		DisableItem(vcrMenu,STOP_ITEM);
		DisableItem(vcrMenu,FF_ITEM);
		DisableItem(vcrMenu,REW_ITEM);
		DisableItem(vcrMenu,GO_START_ITEM);
		DisableItem(vcrMenu,GO_END_ITEM);
		DisableItem(vcrMenu,V_UP_ITEM);
		DisableItem(vcrMenu,V_DOWN_ITEM);
	}
	
	theMenu = GetMHandle(WINDOW_ID);
	if ( ((WindowPeek)myDlog)->visible ) {
		SetItem(theMenu,1,"\pHide VCR");
	}
	else {
		SetItem(theMenu,1,"\pShow VCR");
	}
	
	if ( ((CWindowPeek)moviePosterWindow)->visible ) {
		SetItem(theMenu,2,"\pHide Poster");
	}
	else {
		SetItem(theMenu,2,"\pShow Poster");
	}
	
}
