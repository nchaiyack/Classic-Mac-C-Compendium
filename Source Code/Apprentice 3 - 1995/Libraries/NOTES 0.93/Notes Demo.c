/*****************************************************************
 *
 * Notes Demo.c
 *
 * This file implements an application shell to demonstrate
 * the use of NOTES library functions. See Note.c and Notes.h
 * files for detailed descriptions of the NOTES library.
 * Designed to compile with Think C 7.0.x and Universal Headers.
 *
 *****************************************************************/


#include "NOTES.h"

#define APPLE_MENU		1
#define FILE_MENU		2
#define NEW				1
#define OPEN			2
#define SAVE			3
#define QUIT			5
#define EDIT_MENU		3
#define CUT				1
#define COPY			2
#define PASTE			3
#define CLEAR			4
#define SHOW_NOTES		6
#define EDIT_NOTES		7

#define DEMO_PICT_ID	128

static Rect				gScreenRect;
static PicHandle		gDemoPicture;
static Rect				gPictureRect;
static MenuHandle		gAppleMenu;
static MenuHandle		gFileMenu;
static MenuHandle		gEditMenu;
static WindowPtr		gDemoWindow;
static NoteEnviron		*gNoteEnviron;
static CursHandle		gIBeamCursor;
static Boolean			gIsIBeamCursor;
static Boolean			gQuitFlag;

void InitToolbox(void);
short InitDemo(void);
void DisposeDemo(void);
void HandleOpen(void);
void HandleSave(void);
void HandleMenuSelection(short whichMenu, short whichItem);
void HandleUpdateWindow(void);
void MaintainCursor(void);
void MainEventLoop(void);


/*****************************************************************
 *
 * InitToolbox
 *
 *****************************************************************/

void InitToolbox(void)
{
	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	FlushEvents(everyEvent,0);
	TEInit();
	InitDialogs(0L);
	InitCursor();
} // InitToolbox



/*****************************************************************
 *
 * InitDemo
 *
 *****************************************************************/

short InitDemo(void)
{
	GrafPtr			remPort;
	GrafPort		tempPort;
	Rect			tempRect;
	short			screenWidth;
	short			screenHeight;
	short			windowWidth;
	short			windowHeight;
	short			err;
	
	GetPort(&remPort);					// get screen size
	OpenPort(&tempPort);
	gScreenRect.top = tempPort.portRect.top;
	gScreenRect.bottom = tempPort.portRect.bottom;
	gScreenRect.left = tempPort.portRect.left;
	gScreenRect.right = tempPort.portRect.right;
	ClosePort(&tempPort);
	SetPort(remPort);
	screenWidth = gScreenRect.right - gScreenRect.left;
	screenHeight = gScreenRect.bottom - gScreenRect.top;
	

	ClearMenuBar();						// initialize menu bar
	gAppleMenu = NewMenu(APPLE_MENU, "\p\024");
	AddResMenu(gAppleMenu, 'DRVR');
	InsertMenu(gAppleMenu, 0);
	gFileMenu = NewMenu(FILE_MENU, "\pFile");
	AppendMenu(gFileMenu, "\pNew Notes;Open NotesÉ/O;Save Notes AsÉ/S;(-;Quit/Q");
	InsertMenu(gFileMenu, 0);
	gEditMenu = NewMenu(EDIT_MENU, "\pEdit");
	AppendMenu(gEditMenu, "\pCut/X;Copy/C;Paste/V;Clear;(-;Show Notes;Edit Notes");
	InsertMenu(gEditMenu, 0);
	DrawMenuBar();

	gDemoPicture = GetPicture(DEMO_PICT_ID);	// get picture to go in window
	if (gDemoPicture == NULL)
		return ResError();
	gPictureRect.left = 0;
	gPictureRect.top = 0;
	gPictureRect.right = (**gDemoPicture).picFrame.right - (**gDemoPicture).picFrame.left;
	gPictureRect.bottom = (**gDemoPicture).picFrame.bottom - (**gDemoPicture).picFrame.top;
	
										// initialize window large than picture
	windowWidth = gPictureRect.right + 200;
	windowHeight = gPictureRect.bottom + 200;
	tempRect.left = (screenWidth - windowWidth) / 2;
	tempRect.top = (screenHeight - windowHeight) / 2;
	tempRect.right = tempRect.left + windowWidth;
	tempRect.bottom = tempRect.top + windowHeight;
	gDemoWindow = NewWindow(NULL, &tempRect, "\pNotes Demo", TRUE, noGrowDocProc, (WindowPtr)-1L, FALSE, 0L);
	if (gDemoWindow == NULL)
		return MemError();
	
										// update coordinates to center picture in window
	gPictureRect.left += 100;
	gPictureRect.top += 100;
	gPictureRect.right += 100;
	gPictureRect.bottom += 100;

										// initialize note environment
	err = NOTES_NewEnviron(gDemoWindow, 5, 5, 0, &gNoteEnviron);
	if (err != noErr)
		return err;
	NOTES_Visible(gNoteEnviron, TRUE);
	CheckItem(gEditMenu, SHOW_NOTES, TRUE);
	
										// get i-beam cursor
	gIBeamCursor = GetCursor(iBeamCursor);
	gIsIBeamCursor = FALSE;
	
	gQuitFlag = FALSE;
	
	return noErr;
} // InitDemo



/*****************************************************************
 *
 * DisposeDemo
 *
 *****************************************************************/

void DisposeDemo(void)
{
	NOTES_DisposeEnviron(gNoteEnviron);	// dispose of note environment
	
	DisposeWindow(gDemoWindow);			// dispose of window
	
	ClearMenuBar();						// dispose of menus
	DeleteMenu(EDIT_MENU);
	DisposeMenu(gEditMenu);
	DeleteMenu(FILE_MENU);
	DisposeMenu(gFileMenu);
	DeleteMenu(APPLE_MENU);
	DisposeMenu(gAppleMenu);
	
	ReleaseResource((Handle)gDemoPicture);	// release picture resource
} // DisposeDemo



/*****************************************************************
 *
 * HandleOpen
 *
 *****************************************************************/

void HandleOpen(void)
{
	Point			where;
	SFTypeList		validFileTypes;
	SFReply			theReply;
	short			fRefNum;
	short			err;
	long			environSize;
	long			inOutCount;
	CompressedList	*buffer;
	
	where.h = 0;							// center dialog
	where.v = 0;
	
	validFileTypes[0] = 'NtDm';
	
											// selcet file
	SFGetFile(where, "\p", NULL, 1, validFileTypes, NULL, &theReply);
	if (!theReply.good)
		return;
		
											// open file
	err = FSOpen(theReply.fName, theReply.vRefNum, &fRefNum);
	if (err != noErr) { SysBeep(1); return; }
	
											// position file at size of environment filed
	err = SetFPos(fRefNum, fsFromStart, 8L);
	if (err != noErr) { FSClose(fRefNum); SysBeep(1); return; }
	
											// read size of environ
	inOutCount = sizeof(long);
	err = FSRead(fRefNum, &inOutCount, &environSize);
	if (err != noErr) { FSClose(fRefNum); SysBeep(1); return; }

	buffer = (CompressedList *)NewPtr(environSize);
	if (buffer == NULL) { FSClose(fRefNum); SysBeep(1); return; }

											// position file at start of file
	err = SetFPos(fRefNum, fsFromStart, 0L);
	if (err != noErr) { FSClose(fRefNum); SysBeep(1); return; }
	
											// read all note environment data
	inOutCount = environSize;
	err = FSRead(fRefNum, &inOutCount, (Ptr)buffer);
	if (err != noErr) { FSClose(fRefNum); SysBeep(1); return; }

	FSClose(fRefNum);
	
											// install data into note environment
	NOTES_SetList(gNoteEnviron, (CompressedList *)buffer);
	
	DisposePtr((Ptr)buffer);
} // HandleOpen



/*****************************************************************
 *
 * HandleSave
 *
 *****************************************************************/

void HandleSave(void)
{
	Point				where;
	SFReply				theReply;
	short				fRefNum;
	short				err;
	long				inOutCount;
	CompressedList		*buffer;
	
	where.h =0;								// center dialog
	where.v = 0;
	
											// selcet file
	SFPutFile(where, "\pSave As:", "\pMy Note Environment", NULL, &theReply);
	if (!theReply.good)
		return;
			
											// get a compressed copy of the current note environment
	buffer = NULL;
	NOTES_GetList(gNoteEnviron, &buffer);
	
											// open file
	err = FSOpen(theReply.fName, theReply.vRefNum, &fRefNum);
	if (err == fnfErr)
	{										// if file does not exist, create it
		err = Create(theReply.fName, theReply.vRefNum, '????', 'NtDm');
		if (err != noErr) { DisposePtr((Ptr)buffer); SysBeep(1); return; }
		err = FSOpen(theReply.fName, theReply.vRefNum, &fRefNum);
	}
	if (err != noErr) { DisposePtr((Ptr)buffer); SysBeep(1); return; }
	
											// position file at start of file
	err = SetFPos(fRefNum, fsFromStart, 0L);
	if (err != noErr) { FSClose(fRefNum); SysBeep(1); return; }
	
											// write all note environment data
	inOutCount = buffer->storage;
	err = FSWrite(fRefNum, &inOutCount, (Ptr)buffer);
	if (err != noErr) { FSClose(fRefNum); SysBeep(1); return; }

	FSClose(fRefNum);
	
	DisposePtr((Ptr)buffer);
} // HandleSave



/*****************************************************************
 *
 * HandleMenuSelection
 *
 *****************************************************************/

void HandleMenuSelection(short whichMenu, short whichItem)
{
	GrafPtr			remPort;
	Str255			itemName;
	short			checkMark;

	switch (whichMenu)
	{
		case APPLE_MENU:
											// open Apple Menu Item
			GetItem(gAppleMenu, whichItem, itemName);
			GetPort(&remPort);
			OpenDeskAcc(itemName);
			SetPort(remPort);
			break;
			
		case FILE_MENU:
			if (FrontWindow() == gDemoWindow)
			{
				switch (whichItem)
				{
					case NEW:
						NOTES_ClearList(gNoteEnviron);	// clear current note environment
						break;
						
					case OPEN:
						HandleOpen();		// open note environment from disk
						break;
						
					case SAVE:
						HandleSave();		// save current note environment to disk
						break;
						
					case QUIT:
						gQuitFlag = TRUE;	// quit this program
						break;
						
					default:
						break;
				}
			}
			break;
		
		case EDIT_MENU:
			if (FrontWindow() == gDemoWindow)
			{
				switch (whichItem)
				{
					case COPY:
						NOTES_DoCopy(gDemoWindow);
						break;
						
					case CUT:
						NOTES_DoCut(gDemoWindow);
						break;
						
					case PASTE:
						NOTES_DoPaste(gDemoWindow);
						break;
						
					case CLEAR:
						NOTES_DoClear(gDemoWindow);
						break;
						
					case EDIT_NOTES:
						GetItemMark(gEditMenu, EDIT_NOTES, &checkMark);
						if (checkMark & 0x00FF)
						{
											// exit note edit mode and uncheck menu item
							NOTES_EditMode(gNoteEnviron, FALSE);
							CheckItem(gEditMenu, EDIT_NOTES, FALSE);
						}
						else
						{
											// enter note edit mode and check menu item
							NOTES_EditMode(gNoteEnviron, TRUE);
							CheckItem(gEditMenu, EDIT_NOTES, TRUE);
						}
						break;
						
					case SHOW_NOTES:
						GetItemMark(gEditMenu, SHOW_NOTES, &checkMark);
						if (checkMark & 0x00FF)
						{
											// hide notes and uncheck menu item
							NOTES_Visible(gNoteEnviron, FALSE);
							CheckItem(gEditMenu, SHOW_NOTES, FALSE);
						}
						else
						{
											// show notes and check menu item
							NOTES_Visible(gNoteEnviron, TRUE);
							CheckItem(gEditMenu, SHOW_NOTES, TRUE);
						}
						break;
					
					default:
						break;
				}
			}
			else
				// handle copy/paste in non-application window
				SystemEdit(whichItem - 1);
				break;
			
		default:
			break;
	}
	HiliteMenu(0);
} // HandleMenuSelection



/*****************************************************************
 *
 * HandleUpdateWindow
 *
 *****************************************************************/

void HandleUpdateWindow(void)
{
	GrafPtr			remPort;

	BeginUpdate(gDemoWindow);
	GetPort(&remPort);
	SetPort(gDemoWindow);
	
	DrawPicture(gDemoPicture, &gPictureRect);	// draw window first
	NOTES_DrawEnviron(gNoteEnviron);			// draw notes last
	
	SetPort(remPort);
	EndUpdate(gDemoWindow);
} // HandleUpdateWindow



/*****************************************************************
 *
 * MaintainCursor
 *
 *****************************************************************/

void MaintainCursor(void)
{
	GrafPtr			remPort;
	Point			where;
	
	GetPort(&remPort);
	SetPort(gDemoWindow);
	GetMouse(&where);
	if (NOTES_MouseInText(where))				// change the cursor to an I-beam when over note's text
	{
		if (!gIsIBeamCursor)
		{
			SetCursor(*gIBeamCursor);
			gIsIBeamCursor = TRUE;
		}
	}
	else
	{
		if (gIsIBeamCursor)
		{
			InitCursor();
			gIsIBeamCursor = FALSE;
		}
	}
	SetPort(remPort);
} // MaintainCursor



/*****************************************************************
 *
 * MainEventLoop
 *
 *****************************************************************/

void MainEventLoop(void)
{
	EventRecord		theEvent;
	WindowPtr		whichWindow;
	long			menuResult;
	
	while (!gQuitFlag)						// repeat MainEventLoop until user selects Quit
	{
		if (WaitNextEvent(everyEvent, &theEvent, 0L, 0L))
		{
			switch (theEvent.what)
			{
				case mouseDown:
											// handle mouse event
					switch (FindWindow(theEvent.where, &whichWindow))
					{
						case inMenuBar:
							menuResult = MenuSelect(theEvent.where);
							if (menuResult != 0)
							{
								HandleMenuSelection(HiWord(menuResult), LoWord(menuResult));
							}
							break;
							
						case inDrag:
							if (whichWindow == gDemoWindow)
							{
											// drag window
								DragWindow(whichWindow, theEvent.where, &gScreenRect);
							}
							else
							{
											// bring window to front
							}
							break;
							
						case inGoAway:
							if (TrackGoAway(whichWindow, theEvent.where))
							{
											// close window
							}
							break;
							
						case inContent:
							if (whichWindow == gDemoWindow)
							{
											// handle click in window
								NOTES_DoClick(gDemoWindow, &theEvent);
							}
							else
							{
											// bring window to front
							}
							break;
						
						case inSysWindow:
											// handle click in non-application window
							SystemClick(&theEvent, whichWindow);
							break;
					}
					break;
				
				case keyDown:
				case autoKey:
											// handle a key event
					if ((FrontWindow() == gDemoWindow) && (NOTES_DoKey(gDemoWindow, &theEvent)))
					{
											// key was handled by NOTES
					}
					else
					{
						if (theEvent.modifiers & cmdKey)
						{
							menuResult = MenuKey((theEvent.message & charCodeMask));
							if (HiWord(menuResult) != 0)
							{
											// handle a menu key-short-cut
								HandleMenuSelection(HiWord(menuResult), LoWord(menuResult));
							}
						}
					}
					break;

				case updateEvt:
											// handle an update event
					if ((WindowPtr)theEvent.message == gDemoWindow)
						HandleUpdateWindow();
					break;
					
				case activateEvt:
					// handle an activate event
					NOTES_Activate(gNoteEnviron, (theEvent.modifiers & activeFlag) == activeFlag);
					break;
				
				case osEvt:
					if (theEvent.message & 0x01000000)
					{
											// handle suspend & resume events
					}
					if ((theEvent.message & 0xFF000000) == 0xFA000000)
					{
											// handle mouse-moved event
					}
					break;
				
				case diskEvt:
											// handle disk insert event
					break;
					
				default:
					break;
			}
			
		} // WaitNextEvent
		
		NOTES_Idle();						// make cursor blink in edited notes
		
		MaintainCursor();					// change cursor to an I-beam when over note's text
		
	}
} // GD_MainEventLoop



/*****************************************************************
 *
 * main
 *
 *****************************************************************/
 
main()
{
	InitToolbox();
	if (NOTES_Init() == noErr)
	{
		if (InitDemo() == noErr)
		{
			MainEventLoop();
			DisposeDemo();
		}
		NOTES_Dispose();
	}
	return 0;
}
