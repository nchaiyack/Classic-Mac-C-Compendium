/***************************
** dostuff.c
**
** Contains all the DoXxxx() routines
** for the application.
***************************/

#define kHighLevelEvent 23    /* from Inside mac, volume 6 */

#include "main.h"


/******************
** DoEvent()
** 
** Determines the type of event and dispatches it
** to the correct event handler.
*******************/
void DoEvent(EventRecord *theEvent)
{
	switch (theEvent->what) {
		case mouseDown:
			DoMouseDown(theEvent);
			break;
		case keyDown:
		case autoKey:
			DoKey(theEvent);
			break;
		case activateEvt:
			break;
		case updateEvt:
			if (gMainWindow != NULL)
			{
				BeginUpdate(gMainWindow);
				EraseRect( &(gMainWindow->portRect));
				/* DrawControls(gMainWindow);  */
				/* DrawGrowIcon( gMainWindow); */
				/** do other window drawing here **/
				EndUpdate(gMainWindow);
			}
			break;
		case osEvt:
			break;
		case kHighLevelEvent:
			break;
	}
} /* DoEvent() */


/********************
** DoMouseDown()
**
** handles mouseDown events.
*********************/

void DoMouseDown(EventRecord *theEvent)
{
	WindowPtr whichWindow;
	
	switch (FindWindow( theEvent->where, &whichWindow)) {
		case inSysWindow:
			SystemClick( theEvent, whichWindow);
			break;
		case inMenuBar:
			DoCommand( MenuSelect(theEvent->where) );
			break;
		case inDrag:
			DragWindow(whichWindow, theEvent->where,
				&qd.screenBits.bounds);
			break;
		case inContent:
			DoContent(theEvent);
			break;
		case inGoAway:
			if (TrackGoAway(whichWindow, theEvent->where))
				DoFileClose();
			break;
	}
} /* DoMouseDown() */


/*****************
** DoCommand()
**
** executes the menu options.
******************/

void DoCommand(long mResult)
{
	int theItem = LoWord(mResult);
	int theMenu = HiWord(mResult);
	Str255 name;
	int temp;
	
	switch (theMenu) {
		case APPLE_MENU:
			if (theItem == AppleAboutItem) {
				DoAbout();
			} else {
				GetItem( gAppleM, theItem, name);
				temp = OpenDeskAcc(name);
			}
			break;
			
		case FILE_MENU:
			switch (theItem) {
				case FileNewItem:
					DoFileNew();
					break;
				case FileOpenItem:
					break;
				case FileCloseItem:
					DoFileClose();
					break;
				case FileSaveItem:
				case FileSaveAsItem:
					break;
				case FileQuitItem:
					Cleanup();
					break;
			} /* switch theItem */
			break;
			
		case EDIT_MENU:
			switch (theItem) {
				case EditUndoItem:
					break;
				case EditCopyItem:
					break;
				case EditCutItem:
					break;
				case EditPasteItem:
					break;
				case EditClearItem:
					break;
			} /* switch */
			break;
			
		case LISTEN_MENU:
			switch (theItem) {
				case ListenBegin:  /* begin listening */
					gListening = 1;
					SndListenBegin();
					DisableItem( gListenM, ListenBegin);
					EnableItem(  gListenM, ListenStop);
					break;
				case ListenStop:   /* stop listening */
					gListening = 0;
					SndListenStop();
					EnableItem(  gListenM, ListenBegin);
					DisableItem( gListenM, ListenStop);
					break;
			} /* switch */
			break;
			
	} /* switch theMenu */
	HiliteMenu(0);		/* unhilite the selected menu */
} /* DoCommand() */


/*****************
** DoAbout()
**
** Displays the about box
******************/

void DoAbout(void)
{
	DialogPtr aboutDlog;
	GrafPtr savedPort;
	int itemHit = -9;
	
	GetPort( &savedPort);
	aboutDlog = GetNewDialog(AboutDlogID, NULL, (void*)(-1) );
	if (aboutDlog == NULL) return;
	SelectWindow((WindowPtr)aboutDlog);
	while (itemHit != AboutOKButt)
		ModalDialog( NULL, &itemHit);
	DisposDialog(aboutDlog);
	aboutDlog = NULL;
	SetPort( savedPort);
} /* DoAbout() */


/*********************
** DoFileNew()
**
** executes the File menu, New option.
**
*****************/

void DoFileNew(void)
{
} /* DoFileNew() */


/***************
** DoFileClose()
**
****************/

void DoFileClose(void)
{
} /* DoFileClose() */


void DoContent(EventRecord *theEvent)
{
	int part;
	int value;
	Point pt;
	ControlHandle control;
	
	pt = theEvent->where;
	GlobalToLocal(&pt);
	
  /* In the content area? */
	if ( 1)
	{
	}
  /* In the scroll bar? */
	else
	{
		part = FindControl(pt, gMainWindow, &control);
		switch ( part ) {
			case 0:						/* do nothing for viewRect case */
				break;
			case inThumb:
				value = GetCtlValue(control);
				part = TrackControl(control, pt, NULL);
				if ( part != 0 ) {
					value -= GetCtlValue(control);
					/* value now has CHANGE in value; if value changed, scroll */
				}
				break;
			default:		/* they clicked in an arrow, so track & scroll */
				break;
		} /* switch */
	} /* if */
} /* DoContent() */


/****************
** DoKey()
**
** This is the keyDown event handler.
** 1) handle command-keys
** 2) handle keypresses for other functions
*****************/

void DoKey(EventRecord *theEvent)
{
	char c = theEvent->message & charCodeMask;
	
	if ((theEvent->modifiers & cmdKey ) == cmdKey)
		DoCommand(MenuKey(c));
	else
	{
		/* pass keypress onto something else */
		switch(c) {
		case '+':
		case '=': gRadius += 5;
			break;
		case '-':
		case '_': gRadius -= 5;
			break;
		case ']': gAngleStep += PI/180.;   /* 1¡ in radians */
			break;
		case '[': gAngleStep -= PI/180.;   /* 1¡ in radians */
			break;
		case '1': HideCursor();
			break;
		case '2': ShowCursor();
			break;
		}
	}
} /* DoKey() */



