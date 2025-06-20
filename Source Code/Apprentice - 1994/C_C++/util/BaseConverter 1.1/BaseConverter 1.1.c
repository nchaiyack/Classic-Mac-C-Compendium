/*	BaseConverter90 1.0d14.c */
/*	Roger Brown
  	Courseware Development Group
  	Dartmouth College
	
	Written in THINK C �Symantec Corp.
	
 version 1.1 5/6/94
 Added a font menu for all input fields. It is semi-smart about keyboards.
 It gets a bit confused in non-Roman fonts with numbers not in the US Ascii 
 number range.
 Thanks to David Greenfield @ Dartmouth for the keyboard stuff.
 
 version d14 completed 1/11/89 

 d14: clean up code 
 d13: all fields blank if any one is except binaries 
 d12: fixed TEInit problem
        add MultiFinder awareness
        and scrap publication for DAs

 d11: allow up to 3 error strings 
 d10: try one field ascii again 
 d9: faster paste 
 d8: add paste capability 
 d7: add window location remembering 

 rewrite of old BaseConverter (source number7.c) started 1/2/90 

 To create BaseConverter in Symantec C++ 6.0:
 	Include this source with ANSI Small, MacTraps, and BaseConverter 1.1.rsrc. 
 	Project type is APPL with creator BACO (not registered),
 	MF partition size 100, and MF attributes = 5800 (all 3 settings chosen)
*/

/* Mac header files needed */

#include <QuickDraw.h>
#include <Events.h>
#include <Windows.h>
#include <Menus.h>
#include <Memory.h>
#include <Resources.h>
#include <Controls.h>
#include <Dialogs.h>
#include <Traps.h>
#include <Script.h>


/* C library headers needed */
#include "stdio.h"
#include "ctype.h"
#include "string.h"

/* globals and constants */

/* menus */

#define deskMenuID 1
#define fileMenuID 256
#define editMenuID 257
#define fontMenuID 258
MenuHandle deskMenu,fileMenu,editMenu,fontMenu;


/* dialog items */

#define kDecimal 1
#define kOctal 2
#define kHex 3
#define kBinLoLSB 4
#define kBinLoMSB 5
#define kBinHiLSB 6
#define kBinHiMSB 7
#define kAscii 8
#define kAsciiDump 9
#define kErrorDisplay 10
#define kLabels 11

/* misc other constants */

#define BS 8							/* backspace key */
#define TAB 9							/* tab key */
#define QUITKEY 113						/* Q key */
#define MINUS 45						/* - */
#define MAXLONGINT 2147483647.0
#define MINLONGINT -2147483647.0

/* the dialog window */

DialogPtr BCDialog;						/* pointer to it */
#define BCDialogID 11050				/* resource ID of template */

char asciiText[4][16],asciiNums[4][16];	/* ascii dump strings */

char *conversion[3] = {"%ld","%lo","%lx"};	/* number conversion formats */

/* ascii chart abbreviations */
char *ascii[] = {"NUL","SOH","STX","EXT","EOT","ENQ","ACK","BEL","BS ","HT ","LF ","VT ",
				 "FF ","CR ","SO ","SI ","DLE","DC1","DC2","DC3","DC4","NAK","SYN","ETB",
				 "CAN","EM ","SUB","ESC","FS ","GS ","RS ","US "};

short maxChar[] ={16,16,12,8,8,8,8,4};	/* maximum number of characters allowed 
										   for each dialog item */
										   
QDProcs myProcs;          				/* replacement QuickDraw procs record */

short lastItemHit;						/* remember last hit item here */
long lastNum = -1;				/* remember last number value shown, init to -1 */

/* error handling */

char errorStr[4][64];		  	/* current error strings */
short numErrors;					/* current number of errors */
Boolean outOfRange;				/* input is out of range for a long int */
Boolean invalidChar;			/* input character is invalid for base */
Boolean maxCharsExceeded;		/* too many characters typed */
short pasteError;					/* holds any error from paste operation */
#define kPasteOK 0				/* paste error value: no error */
#define kPasteTooLong 2			/* paste error value: too long to fit in field */
#define kPasteInvalid 4			/* paste error value: invalid characters in stream */

/* event loop globals and constants */

Rect limitRect;					/* drag limit area */

/* MultiFinder stuff */

#define App4Selector(eventPtr) 	(*((unsigned char *) &(eventPtr)->message))
/* top byte of message field is the selector */

#define SUSPEND_RESUME_SELECTOR		0x01
/* selector of this value is suspend/resume */

#define SuspResIsResume(evtMessage)		((evtMessage) & 0x00000001)
/* low bit on, signifies resume */

#define SuspResIsSuspend(evtMessage)	(!SuspResIsResume(evtMessage))
/* low bit off, signifies suspend */

#define ScrapDataHasChanged(evtMessage) 	((evtMessage) & 0x00000002)
/* only valid for suspend/resume messages */

Boolean wneIsImplemented;			/* is WaitNextEvent implmented? */
Boolean inForeground;				/* are we in MF foreground? */
TEHandle theTEH;					/* dialog te handle */

/* some prototypes */

Byte GetByte();
Boolean maxCharacters();
Boolean SelectionEmpty();
pascal void DrawLabels ();
pascal void AsciiDump ();
pascal void ErrorDisplay ();
pascal short MyStdTextMeas();
pascal void MyStdText();
pascal Boolean MyWordBreak();
pascal void OutlineButton ();
void BinaryConvert(char *s,long *n);
void MakeBinary(long n,char *s,short fByte);
void SyncKeyboard2Font(short fontID );



/* ======================== Event Handling ================ */

main()
{   
	
	short windowCode;							/* window event code */
	Boolean mayBe;							/* utility boolean */
	EventRecord myEvent;					/* the event record */
	WindowPtr whichWindow;					/* utility window ptr */
	GrafPtr gp;								/* utility graf ptr */
	OSErr err;								/* utility OS Err */
	
	
	StdInits();								/* start the application */
	MyInits();
	
	/* run main event loop */
	
	while (1) {	
							
		SetPort(BCDialog);				
		CheckMenus();
		
		/* get the event */
		
		if (wneIsImplemented) {			/* get an event */
			mayBe = WaitNextEvent(everyEvent,&myEvent,10L,NULL);
		}
		else {
			SystemTask();
			mayBe = GetNextEvent(everyEvent,&myEvent);
		}
		
		/* process the event */
		
		if (IsDialogEvent(&myEvent)) 
			DoDialogEvent(&myEvent);
		else switch(myEvent.what) {			/* other events */
			case mouseDown:
				windowCode = FindWindow(myEvent.where,&whichWindow);
				if ((whichWindow!=FrontWindow()) 
					&&(whichWindow!=NULL)) SelectWindow(whichWindow);
				switch(windowCode) {
					case inContent:
						break;
					case inMenuBar:
						DoMenu(MenuSelect(myEvent.where));
						break;
				    case inSysWindow:
				    	SystemClick(&myEvent,whichWindow);
				    	break;
					case inGoAway:
					    mayBe = TrackGoAway(whichWindow,myEvent.where);
						if (mayBe==TRUE) {
							DoExit();
						}
						break;
					case inDrag:
						DragWindow(whichWindow,myEvent.where,&limitRect);
						DrawGrowIcon(whichWindow);
						break;
					case inGrow:
						break;
				}
			break;
			case updateEvt:
				whichWindow = (WindowPtr)myEvent.message;
				GetPort(&gp);
				SetPort(whichWindow);
				BeginUpdate(whichWindow);
					DrawControls(whichWindow);
					DrawGrowIcon(whichWindow);
					EndUpdate(whichWindow);
				SetPort(gp);
			break;
			case keyDown:
			break;
			case activateEvt:
				if ((WindowPtr)myEvent.message==BCDialog) {
					if ( myEvent.modifiers & activeFlag ) 
						err = TEFromScrap();
					else {
						err = ZeroScrap();
						if (err==noErr)
							TEToScrap();
					}
				}
			break;
			case app4Evt:
				HandleApp4Evt(&myEvent);
			break;
		}
	}
}

/* process a dialog event */

DoDialogEvent(myEvent)
EventRecord *myEvent;
{			
	short code,								/* character code */
		itemHit;							/* dialog item chosen */
	DialogPtr whichDialog;					/* utility dialog ptr */
	OSErr err;								/* utility os error */
	short ok;									/* dialog event status */
	/* check for MF suspend/resume events */
	
	if (myEvent->what == app4Evt)  {
		if (HandleApp4Evt(myEvent))
			return;
	}
	
	/* check for window activate events */
	
	else if (myEvent->what == activateEvt) {
		if ((WindowPtr)myEvent->message==BCDialog) {
			if ( myEvent->modifiers & activeFlag ) 
				err = TEFromScrap();				/* get global scrap */
			else {
				err = ZeroScrap();					/* publish our scrap */
				if (err==noErr)
					TEToScrap();
			}
		}
	}

	/* Handle a key or mouse event */
	
	if ((myEvent->what==keyDown) || (myEvent->what==autoKey))  {
		// Do this in case the user changed the keyboard
		SyncKeyboard2Font((**theTEH).txFont);
		code = BitAnd(myEvent->message,charCodeMask);  /* what keys was pressed? */
			
		/* check command keys */
		if ((myEvent->modifiers & cmdKey) != 0) {
			DoMenu(MenuKey(code));
			ShowErrors();
			return;
		}
		else {
			ok = 1;									/* assume character is valid */
			
			/* filter invalid characters */
			if ((code!=BS)&&(code!=TAB)) {
				if (lastItemHit<kAscii) {
					invalidChar = FALSE;
					ok = ValidChar(code,lastItemHit);
				}
			    if (ok==1) {
			    	maxCharsExceeded = FALSE;
			    	ok = maxCharacters(lastItemHit);
			    }
			    if (!ok) {
			    	Str255 n;
			    	NumToString((long)code,n);
			    	SetWTitle(BCDialog,n);
			    	ShowErrors();
			    	return;
			    }
			}
		}
	}
	
	/* if character is valid or we got a mouseDown, process the event */
	invalidChar = FALSE;
	maxCharsExceeded = FALSE;
	if (DialogSelect(myEvent,&whichDialog,&itemHit)) {
		
		// Some fonts make the text jump, so we do this to clean it up
		Rect r = (**theTEH).viewRect;
		InvalRect(&r);
		if (whichDialog!=BCDialog) return;			/* what else can it be? */
		pasteError = kPasteOK;
		DoDialog(itemHit);
		ShowErrors();
	}
}


/* Handle the app4 event if and only if its a suspend/resume event */
/* The only action we take is to convert the scrap to and from the
   public scrap */

HandleApp4Evt(theEvent)
EventRecord *theEvent;
{
	OSErr	err;
	
	if (App4Selector(theEvent) == SUSPEND_RESUME_SELECTOR) {
		if SuspResIsSuspend(theEvent->message) {
			inForeground = FALSE;
			TEDeactivate(theTEH);
			if (ScrapDataHasChanged(theEvent->message)) {
				err = ZeroScrap();
				err = TEToScrap();					/* convert the scrap */
			}
		}
		else {
			inForeground = TRUE;
			TEActivate(theTEH);
			if (ScrapDataHasChanged(theEvent->message))
				err = TEFromScrap();				/* convert the scrap */
		}
		return 1;
	}
	return 0;
}

/* ================== Initialization ===================== */

/* From Symantec C++ OSChecks.cp */

/******************************************************************************
 TrapAvailable
 
 	Check whether a certain trap exists on this machine. This function uses
 	the new method as per IM VI, p. 3-8.
 	
 ******************************************************************************/

Boolean TrapAvailable( short theTrap)
{
	TrapType tType;
	short    numToolBoxTraps;
	
				// first determine the trap type
				
	tType = (theTrap & 0x800) > 0 ? ToolTrap : OSTrap;
	
				// next find out how many traps there are
				
	if (NGetTrapAddress( _InitGraf, ToolTrap) == NGetTrapAddress( 0xAA6E, ToolTrap))
		numToolBoxTraps = 0x200;
	else
		numToolBoxTraps = 0x400;
	
				// check if the trap number is too big for the
				// current trap table
				
	if (tType == ToolTrap)
	{
		theTrap &= 0x7FF;
		if (theTrap >= numToolBoxTraps)
			theTrap = _Unimplemented;
	}
	
				// the trap is implemented if its address is
				// different from the unimplemented trap
				
	return (NGetTrapAddress( theTrap, tType) != 
			NGetTrapAddress(_Unimplemented, ToolTrap));
}

/******************************************************************************
 WNEIsImplemented
 
 	Check whether the WaitNextEvent is implemented or not
 ******************************************************************************/

Boolean		WNEIsImplemented()
{
	SysEnvRec	theWorld;				/* System environment				*/
	
	SysEnvirons(1, &theWorld);			/* Check environment				*/
	
	if (theWorld.machineType < 0) 		/* Old ROMs, definitely not present	*/
		return(false);
		
	else								/* Check for WNE trap				*/
		return(TrapAvailable(_WaitNextEvent));
}

/* standard Toolbox inits and start up */

StdInits()
{	
	InitGraf(&thePort);
	InitFonts(); 
	FlushEvents(everyEvent,0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	InitCursor();
	MaxApplZone();
	MoreMasters();
	MoreMasters();
}

/* Inits specific to this app */

MyInits()
{
	Handle iH;
	short iType,i;
	Rect box;
	DialogPeek dp;
	OSErr err;

	
	/* prepare for MultiFinder */
	
	inForeground = TRUE;
	wneIsImplemented = WNEIsImplemented();
	
	/* get our dialog template and create a window */
	
	BCDialog = GetNewDialog(BCDialogID,NULL,(WindowPtr)-1);
	if (BCDialog==NULL) {
		SysBeep(10);
		DoExit();
	}
	

	/* set up the user items */
	
	GetDItem(BCDialog,kLabels,&iType,&iH,&box);
	SetDItem(BCDialog, kLabels, userItem + itemDisable,
	 	(Handle)DrawLabels, &box);
	GetDItem(BCDialog,kAsciiDump,&iType,&iH,&box);
	SetDItem(BCDialog, kAsciiDump, userItem + itemDisable,
	 	(Handle)AsciiDump, &box);
	GetDItem(BCDialog,kErrorDisplay,&iType,&iH,&box);
	SetDItem(BCDialog, kErrorDisplay, userItem + itemDisable,
	 	(Handle)ErrorDisplay, &box);
	
	/* override some graf procs for special measure and drawing */
	
	SetPort(BCDialog);
	SetStdProcs(&myProcs);
	myProcs.textProc	=	(QDPtr)MyStdText;
	myProcs.txMeasProc	=	(QDPtr)MyStdTextMeas;
	BCDialog->grafProcs = &myProcs;
	
	/* set our special word break procedure */
	dp = (DialogPeek)BCDialog;
	theTEH = dp->textH;
	SetWordBreak(MyWordBreak,theTEH);
	
	/* import the public scrap */
	err = TEFromScrap();
	
	/* create my menus */
	setupmenus();

	/* set the window where it should be */
	
	PositionWindow();
	SetFont();
	
	/* initialize some globals */
	
	lastItemHit = kDecimal;
	numErrors = 0;
}

/* position the window where it wants to be unless it is off screen */ 

PositionWindow()
{
	DialogTHndl dt;
	RgnHandle grayRgn;
	Rect myRect,testRect;
	Point tl;
	
	/* get the dialog template  */
	
	dt = (DialogTHndl)GetResource('DLOG',BCDialogID);
	if (dt==NULL) {
		return;
	}
	
	/* get the rectangle where the window was left last time */
	
	myRect = (**dt).boundsRect;
	
	/* if the topLeft of that rectangle is not within the current screen,
	   move the window to a sensible locations.  (This can only occur if the
	   copy of the app moves from a big screen system to a smaller screen system) */
	   
	grayRgn = GetGrayRgn();
	limitRect = (**grayRgn).rgnBBox;
	tl.h = myRect.left;
	tl.v = myRect.top;
	if (PtInRect(tl,&limitRect)==FALSE) {   /* is off screen */
		MoveWindow(BCDialog,54,78,FALSE);
	}
	
	/* and show the window */
	
	ShowWindow(BCDialog);
}

SetFont()
{
	StringHandle sh;
	short f,i;
	Str255 name1,name2;
	
	if (!fontMenu)
		return;
		
	sh = GetString(1000);
	if (sh) {
		GetFNum(*sh,&f);
		(**theTEH).txFont = f;
		TextFont(f);
		BlockMove(*sh,name2,255);
		SyncKeyboard2Font(f);
	}
	else {
		(**theTEH).txFont = systemFont;
		TextFont(systemFont);
		strcpy((char*)name2,(char*)"\pChicago");
		SyncKeyboard2Font(systemFont);
	}
	TECalText(theTEH);
	
	for (i=1;i<CountMItems(fontMenu);i++) {
		GetItem(fontMenu,i,name1);
		if (EqualString(name1,name2,false,false))
			CheckItem(fontMenu,i,true);
	}
	
}

/* ====================== Menu Handling ================ */

/* create our menus */

setupmenus()
{
	char appletitle[2];

	/* desk accessories */
	 
	appletitle[1] = appleMark;appletitle[0]=1;
	deskMenu = NewMenu(deskMenuID,(StringPtr)appletitle);         
	AppendMenu(deskMenu,"\pAbout Base Converter�;(-");
	AddResMenu(deskMenu,'DRVR');
	InsertMenu(deskMenu,0);
	
	/* file menu */
	
	fileMenu = NewMenu(fileMenuID,"\pFile");
	AppendMenu(fileMenu,"\pQuit/Q");
	InsertMenu(fileMenu,0);

	/* edit menu */
	
	editMenu = NewMenu(editMenuID,"\pEdit");
	AppendMenu(editMenu,"\p(Can't Undo;(-;Cut /X;Copy /C;Paste /V;Clear");
	InsertMenu(editMenu,0);
	
	
	/* We only allow font changes if the TERecord of the dialog is not styled,
	   which it is not as of this release. */
	   
	if ((**theTEH).txSize >= 0) {
		/* font menu */
		
		fontMenu = NewMenu(fontMenuID,"\pFont");
		AddResMenu(fontMenu,'FONT');
		InsertMenu(fontMenu,0);
	}
	else
		fontMenu = NULL;
	
	DrawMenuBar();
}

/* process a menu event */

DoMenu(menuresult)
/* handle menu commands */
long menuresult;
{
	short menuid,itemnumber,i,itemHit,t;
	char markchar;
	Str255 name;
	GrafPtr cp;
	Rect r;
			
	menuid = HiWord(menuresult);
	itemnumber = LoWord(menuresult);
	switch (menuid) {
	
		/* DA's */
		
		case deskMenuID:
			if (itemnumber==1) 
				DoAbout();
			else {
				GetItem(deskMenu,itemnumber,name);
				GetPort(&cp);
				i = OpenDeskAcc(name);
				SetPort(cp);
			}
		break;
		
		/* file menu */
		
		case fileMenuID:
			switch (itemnumber) {
				case 1: {
					DoExit();
			   	}              
				break;	
			}
		break;
		
		/* edit menu */
		
		case editMenuID:
			if (!SystemEdit(itemnumber-1))     /* is it a DA? */
			switch (itemnumber) {
				case 1:              
				break;
				case 3:
					DlgCut(BCDialog);
					r = (**theTEH).viewRect;
					EraseRect(&r);
					InvalRect(&r);
					ClearErrors();
				break;
				case 4:
					DlgCopy(BCDialog);
				break;
				case 5:	
					DoPaste();
				break;
				case 6:
					DlgDelete(BCDialog);
					ClearErrors();
				break;
			}
		break;
		case fontMenuID:
		{
			
			for (i=1;i<CountMItems(fontMenu);i++) {
				CheckItem(fontMenu,i,(i == itemnumber));
			}
			GetItem(fontMenu,itemnumber,name);
			GetFNum(name,&i);
			TEDeactivate(theTEH);
			(**theTEH).txFont = i;
			TextFont(i);
			TECalText(theTEH);
			TEActivate(theTEH);
			SyncKeyboard2Font(i);
			ClipRect(&BCDialog->portRect);
			EraseRect(&BCDialog->portRect);
			InvalRect(&BCDialog->portRect);			
		}
	}
	
	HiliteMenu(0);
}

/* quit the program */

DoExit()
{	
	OSErr err;
	
	if (BCDialog!= NULL) {
	
		/* publish the scrap */
		
		err = ZeroScrap();
		err = TEToScrap();
		
		/* save the window position */
		RememberPosition();
		RememberFont();
		
		/* clean up */
		
		DisposDialog(BCDialog);
	}
	ExitToShell();
}

/* remember the current window position */

RememberPosition()
{
	Rect myRect;
	Point corner;
	DialogTHndl dt;
	
	/* get the current location */
	SetPort(BCDialog);
	myRect = BCDialog->portRect;
	
	/* change it to global coordinates */
	corner.h = myRect.left;
	corner.v = myRect.top;
	LocalToGlobal(&corner);
	myRect.left = corner.h;
	myRect.top = corner.v;
	corner.h = myRect.right;
	corner.v = myRect.bottom;
	LocalToGlobal(&corner);
	myRect.right = corner.h;
	myRect.bottom = corner.v;

	/* save location in the dialog template if we can */
	dt = (DialogTHndl)GetResource('DLOG',BCDialogID);
	if (dt==NULL) {
		return;
	}
	(**dt).boundsRect = myRect;
	ChangedResource((Handle)dt);
	if (ResError()!=noErr) {					/* volume is locked, can't do it */
		return;
	}
	WriteResource((Handle)dt);
	if (ResError()!=noErr) {
		return;
	}
}

/* remember the current font */

RememberFont()
{
	StringHandle sh;
	Str255 name;
	
	sh = GetString(1000);
	if (sh != NULL) {
		RmveResource((Handle)sh);
	}
	GetFontName((**theTEH).txFont,name);
	sh = NewString(name);
	AddResource((Handle)sh,'STR ',1000,"\pFont");
	if (ResError()!=noErr) {					/* volume is locked, can't do it */
		return;
	}
	WriteResource((Handle)sh);
}

/* show the about box */

DoAbout()
{
	#define ABOUTDIALOG 11051
	#define kOKBUTTON 1
	#define kOUTLINER 2

	DialogPtr theDialog;
	WindowPtr tempport;
	short	itemhit,itemType;
	Handle	itemHandle;
	Rect	itemBox;
	Boolean done;
		
	/* get the dialog box */
	theDialog = GetNewDialog(ABOUTDIALOG, 0L,(WindowPtr) -1);
	if (theDialog == NULL) {
		SysBeep(1);
		return;
	}
		
	InitCursor();
	
	/* install the default outline */
	GetDItem(theDialog,kOKBUTTON,&itemType,&itemHandle,&itemBox);
	SetDItem(theDialog, kOUTLINER, userItem + itemDisable,
	 	(Handle)OutlineButton, &itemBox);

	/* pose the dialog */
	ShowWindow(theDialog);
	
	/* wait for button hit */
	done = FALSE;
	while (!done) {
		ModalDialog(0L,&itemhit);
		if (itemhit==kOKBUTTON)
			done = TRUE;
	}

	DisposDialog(theDialog);
	return TRUE;
}

/* Handle a paste operation our way: scan all characters on the scrap.
   Each valid one that will fit is moved to a buffer.  Error states
   are set for invalid characters and when the length gets too long.
   The buffer of valid characters is then inserted. */

DoPaste()
{
	Handle scrapH;
	long len,offset;
	short i,pos;
	char c;
	char buff[32];
	TEHandle theTEH;
	DialogPeek dp;
	short	iType;
	Handle	iH;
	Rect	box;
	short tLen,sLen,room;
	Str255 temp;
	
	/* get the TE handle */
	
	dp = (DialogPeek)BCDialog;
	theTEH = dp->textH;

	/* init the error level to ok */
	
	pasteError = kPasteOK;
	
	/* init the buffer */
	buff[0] = pos = 0;
	
	/* limit the input to 32 chars because no field can hold that much anyway */
	
	len = TEGetScrapLen();
	if (len>32) {
		len = 32;
		pasteError = kPasteTooLong;
	}
	
	/* if there is somethig to paste */
 	if (len>0L) {
 		
 		/* get the scrap */
 		
		scrapH = TEScrapHandle();
		HLock(scrapH);
		
		/* process each character in it */
		
		 for (i=0;i<len;i++) {
			c = *(*scrapH+i);
			if (ValidChar(c,lastItemHit)) {
				buff[pos] = c;
				buff[pos+1] = 0;
				pos++;
			}
			else 
				pasteError = kPasteInvalid;
		}
		HUnlock(scrapH);
		
		/* see how much of it will fit */
		
		GetDItem(BCDialog,lastItemHit,&iType,&iH,&box);
		GetIText(iH,temp);
		tLen = temp[0];     								/* have this much now */
		sLen = (**theTEH).selEnd - (**theTEH).selStart;  	/* selection is this long */
		room = maxChar[lastItemHit-1] - tLen + sLen;  		/* can fit this much */
		if (pos > room) {
			pos = room;
			pasteError = kPasteTooLong;
		}
		
		/* delete the selection and insert the processed buffer */
		
		TEDelete(theTEH);
		TEInsert(&buff,(long)pos,theTEH);
		
		/* update all other fields */
		
		DoDialog(lastItemHit);
	}
	
	/* report any errors */
	
	ShowErrors();
}

/* Enable/Disable menus according to conditions. */

CheckMenus()
{
	long len,offset;
	Boolean weAreInFront;
	
	weAreInFront = (FrontWindow()==BCDialog);
		
	/* cut/copy/clear */
	
	if (SelectionEmpty()&&(weAreInFront)) {
		DisableItem(editMenu,3);
		DisableItem(editMenu,4);
		DisableItem(editMenu,6);
	}
	else {
			EnableItem(editMenu,3);
		EnableItem(editMenu,4);
		EnableItem(editMenu,6);
	}
	
	/* paste */
	
	len = TEGetScrapLen();
	if ((len==0L)&&(weAreInFront))
		DisableItem(editMenu,5);
	else 
		EnableItem(editMenu,5); 
	
}


/* ================== Dialog Handling ==================== */

/* handle dialog actions */

DoDialog(itemHit)
short itemHit;
{
	long num,num1,num2,num3,num4;
	Str255 temp,temp2;
	char temp3[32];
	Handle iH;
	short iType,i,j;
	Rect box;
	GrafPtr savePort;
	double fNum;
	Boolean emptyText = FALSE;
	
	outOfRange = FALSE;
	lastItemHit = itemHit;
	
	/* get source of number */
	
	GetDItem(BCDialog,itemHit,&iType,&iH,&box);
	GetIText(iH,temp);
	
	if (temp[0] == 0) {
		if ((itemHit < kBinLoLSB)||(itemHit > kBinHiMSB))
			emptyText = TRUE;
	}
	if (EqualString(temp,"\p-",true,true)) {    /* special case of just a '-' */
		emptyText = TRUE;
		temp[0] = 0;
	}
		
	if (!emptyText) {	
		/* convert the target field first */
		
		switch(itemHit) {
			case kDecimal: {
				PtoCstr(temp);
				sscanf((char *)temp,"%lf",&fNum);
				if ((fNum>MAXLONGINT)||(fNum<MINLONGINT)) {		/* out of range */
					ClearAllFields(kDecimal);
					outOfRange = TRUE;
					lastNum = -1;
					return;
				}
				sscanf((char *)temp,"%ld",&num);
				CtoPstr((char*)temp);
				break;	
			}
		
			case kOctal: {
				PtoCstr(temp);
				sscanf((char *)temp,"%lo",&num);
				sprintf((char *)temp2,"%lo",num);
				strcpy(temp3,(char *)temp);
				StripLeadingZeros(temp3);
				if (strcmp((char *)temp2,"0")==0)
					temp2[0] = 0;
				if (strcmp((char *)temp3,(char *)temp2)!=0) {					/* out of range */
					ClearAllFields(kOctal);
					outOfRange = TRUE;
					lastNum = -1;
					return;
				}
				CtoPstr((char*)temp);
				break;
			}
			
			case kHex: {
				PtoCstr(temp);
				sscanf((char *)temp,"%lx",&num);
				sprintf((char *)temp2,"%lx",num);
				strcpy(temp3,(char *)temp);
				StripLeadingZeros(temp3);
				if (strcmp((char *)temp2,"0")==0)
					temp2[0] = 0;
				ucase(temp3);
				ucase((char*)temp2);
				if (strcmp((char *)temp3,(char *)temp2)!=0) {					/* out of range */
					ClearAllFields(kHex);
					outOfRange = TRUE;
					lastNum = -1;
					return;
				}
				ucase((char*)temp);
				CtoPstr((char*)temp);
				GetDItem(BCDialog,kHex,&iType,&iH,&box); 
				SetIText(iH,temp);
				break;
			}
			
			case kBinHiMSB: {
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num1);                      /* num1 has MSB of hi word */
				GetDItem(BCDialog,kBinHiLSB,&iType,&iH,&box);   /* get LSB of hi word */
				GetIText(iH,temp);
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num2);
				GetDItem(BCDialog,kBinLoMSB,&iType,&iH,&box);   /* get MSB of low word */
				GetIText(iH,temp);
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num3);
				GetDItem(BCDialog,kBinLoLSB,&iType,&iH,&box);   /* get LSB of lo word */
				GetIText(iH,temp);
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num4);
				num = (num1<<24)+(num2<<16)+(num3<<8)+num4;
				break;
			}
			case kBinHiLSB: {
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num2);                      /* num2 has LSB of hi word */
				GetDItem(BCDialog,kBinHiMSB,&iType,&iH,&box);   /* get MSB of hi word */
				GetIText(iH,temp);
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num1);
				GetDItem(BCDialog,kBinLoMSB,&iType,&iH,&box);   /* get MSB of low word */
				GetIText(iH,temp);
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num3);
				GetDItem(BCDialog,kBinLoLSB,&iType,&iH,&box);   /* get LSB of lo word */
				GetIText(iH,temp);
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num4);
				num = (num1<<24)+(num2<<16)+(num3<<8)+num4;
				break;
			}
			case kBinLoMSB: {
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num3);                      /* num3 has MSB of lo word */
				GetDItem(BCDialog,kBinHiMSB,&iType,&iH,&box);   /* get MSB of hi word */
				GetIText(iH,temp);
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num1);
				GetDItem(BCDialog,kBinHiLSB,&iType,&iH,&box);   /* get LSB of hi word */
				GetIText(iH,temp);
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num2);
				GetDItem(BCDialog,kBinLoLSB,&iType,&iH,&box);   /* get LSB of lo word */
				GetIText(iH,temp);
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num4);
				num = (num1<<24)+(num2<<16)+(num3<<8)+num4;
				break;
			}
			case kBinLoLSB: {
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num4);                      /* num4 has LSB of lo word */
				GetDItem(BCDialog,kBinHiMSB,&iType,&iH,&box);   /* get MSB of hi word */
				GetIText(iH,temp);
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num1);
				GetDItem(BCDialog,kBinHiLSB,&iType,&iH,&box);   /* get LSB of hi word */
				GetIText(iH,temp);
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num2);
				GetDItem(BCDialog,kBinLoMSB,&iType,&iH,&box);   /* get MSB of lo word */
				GetIText(iH,temp);
				PtoCstr(temp);
				BinaryConvert((char*)temp,&num3);
				num = (num1<<24)+(num2<<16)+(num3<<8)+num4;
				break;
			}
			
			case kAscii: {
				/* make a number from the bytes of the current string, but
				   shift them all to the right */
				
				num1 = num2 = num3 = num4 = 0;
				
				if (temp[0] > 0 )  
					num1 = temp[temp[0]];
				if (temp[0] > 1  ) 
					num2 = temp[temp[0]-1];
				if (temp[0] > 2  ) 
					num3 = temp[temp[0]-2];
				if (temp[0] > 3  ) 
					num4 = temp[temp[0]-3];
			  
				/* assemble a new number from that */
				num = num1 + (num2<<8) + (num3<<16) + (num4<<24);
				break;
			}
		} /* end of target conversion switch */
	}     
	
	/* now reformat to destinations */	
	
	if (num==lastNum) return;
		
	if (itemHit<kBinLoLSB)  
		sscanf((char *)temp,(char *)conversion[itemHit-1],&num);
	
	for (i=1;i<kBinLoLSB;i++) {
		if (i!=itemHit) {
			if (!emptyText) {
				sprintf((char *)temp,(char *)conversion[i-1],num);
				ucase((char *)temp); // to get hex characters in upper case
				CtoPstr((char *)temp);
			}
			GetDItem(BCDialog,i,&iType,&iH,&box);
			SetIText(iH,temp);
		}
	}
	
	if (itemHit!=kBinHiMSB) {
		GetDItem(BCDialog,kBinHiMSB,&iType,&iH,&box);   /* MSB of Hi Word */
		if (!emptyText) {
			MakeBinary(num,(char*)temp,3);
			CtoPstr((char *)temp);
		}
		SetIText(iH,temp);
	}
	if (itemHit!=kBinHiLSB) {
		if (!emptyText) {
			MakeBinary(num,(char*)temp,2);
			CtoPstr((char *)temp);
		}
		GetDItem(BCDialog,kBinHiLSB,&iType,&iH,&box);   /* LSB of Hi Word */
		SetIText(iH,temp);
	}
	if (itemHit!=kBinLoMSB) {
		if (!emptyText) {
			MakeBinary(num,(char*)temp,1);
			CtoPstr((char *)temp);
		}
		GetDItem(BCDialog,kBinLoMSB,&iType,&iH,&box);  /* MSB of Lo Word */
		SetIText(iH,temp);
	}
	if (itemHit!=kBinLoLSB) {
		if (!emptyText) {
			MakeBinary(num,(char*)temp,0);
			CtoPstr((char *)temp);
		}
		GetDItem(BCDialog,kBinLoLSB,&iType,&iH,&box);  /* LSB of Low Word */
		SetIText(iH,temp);
	}

	if (itemHit!=kAscii) {
		if (!emptyText) {
			for (i=1;i<5;i++) {
				temp[i] = GetByte(num,5-i);
				if (temp[i]==13) temp[i] = 255;   /* to avoid cr wraps */
			}
			temp[0] = 4;
			
			/* update text display in ascii box */
		}
		GetDItem(BCDialog,kAscii,&iType,&iH,&box);
		SetIText(iH,temp);
	}	
	
	/* show ascii dump display */
	if (num!=lastNum) {
		AsciiConvert(num,emptyText);
		/* force redraw of ascii dump area */
		GetPort(&savePort);
		SetPort(BCDialog);
		GetDItem(BCDialog,kAsciiDump,&iType,&iH,&box);
		InvalRect(&box);
		SetPort(savePort);
	}
	
	lastNum = num;
}

/* see if a character is valid for a given type */

ValidChar(c,type)
char c;
short type;
{
	short i;
	char *cp;
	Boolean badChar = FALSE;
	
	c = (char)toupper(c);
	switch (type) {
		case kDecimal:						/* 0-9 */
		   if (c!=MINUS) {
		   		if (c<48) badChar = TRUE;
		   		else if (c>57) badChar = TRUE;
		   	}
		break;
		case kOctal:						/* 0-7 */
			if (c<48) badChar = TRUE;
			else if (c>55) badChar = TRUE;
		break;
		case kHex:						/* 0-9, ABCDEF */
			if (c<48) badChar = TRUE;
			else {
				cp = strchr((char*)"ABCDEF",(short)c);
				if ((c>57)&&(cp==NULL)) badChar = TRUE;
			}
		break;
		case kAscii:
			return 1;     			/* any character is ok */
		break;
		case kBinLoLSB:				/* 0 and 1 only */
		case kBinHiLSB:
		case kBinLoMSB:
		case kBinHiMSB:
			if ((c<48)||(c>49)) badChar = TRUE;
		break;
	}
	if (badChar) {
		invalidChar = TRUE;
		return 0;
	}
	return 1;
}


/* report whether or not a given type already has the maximum number
   of characters. */
   
Boolean maxCharacters(type)
short type;
{	
	Handle iH;
	short iType,i;
	Rect box;
	Str255 temp;
	
	maxCharsExceeded = FALSE;
	
	/* see how many there are */
	
	GetDItem(BCDialog,type,&iType,&iH,&box);
	GetIText(iH,temp);
	
	/* check against the limit */
	
	if (temp[0]<maxChar[type-1]) 				/* still room for more */
		return 1;
	
	if (!SelectionEmpty())	/* field is full, but there is room in the selection */
		return 1;
		
	/* no more room */
	SysBeep(0);
	maxCharsExceeded = TRUE;
	return 0;	
}

/* convert a string of 0s and 1s into a number equivalent to
   the binary representation of the 01 pattern */
   
void BinaryConvert(char *s,long *n)
{
	short i,j,len;
	long temp,x;
	
	temp = 0;
	len = strlen(s);
	j = 0;
	for (i=0;i<len;i++) {
		x = (long)(len-i-1);
		if (s[i]==49) temp = temp+(1<<x);
	}
	*n = temp;
}

/* convert a long to 4 characters into two representations:
   a string of ascii chart symbols and a string of decimal byte values. */
   
AsciiConvert(num,empty)
long num;
Boolean empty;
{
	short i,len;
	Byte c;
	Str255 numStr;
	
	/* init the strings */
	
	for (i=0;i<4;i++) {
		strcpy(asciiText[i],"");
		strcpy(asciiNums[i],"");
	}
	if (empty) return;			/* no text to represent */
	
	
	/* convert */
	
	for (i=0;i<4;i++) {
		c = GetByte(num,4-i);
		if (c<32) 
			strcat(asciiText[i],ascii[c]);
		else if (c==127)
		   	strcat(asciiText[i],"DEL");
		else {
			len = strlen(asciiText[i]);
			asciiText[i][len] = c;
			asciiText[i][len+1] = 0;
		}
		strcat(asciiText[i]," ");
		NumToString((long)c,numStr);
		PtoCstr(numStr);
		strcat(asciiNums[i],(char *)numStr);
		strcat(asciiNums[i],"  ");
		CtoPstr((char*)asciiText[i]);
		CtoPstr((char*)asciiNums[i]);
	}
}

/* given a long number and which byte to format, create a string that
   holds a binary representation of the number. */
   
void MakeBinary(long n,char *s,short fByte)
{
	short i,word;
	long test,temp;
	
	if (fByte > 1)           			/* use hi word */
		word = HiWord(n);
	else								/* use lo word */
		word = LoWord(n);
	if ((fByte==1)||(fByte==3)) 
		temp = word>>8;					/* get hi byte of the word */
	else {
		temp = word << 8;				/* get low byte of the word */
		temp = temp >> 8;
	}
	
	/* convert to pattern of 0's and 1's */
	for (i=7;i>-1;i--){
		test = 1<<i;
		if (temp&test) s[7-i] = 49;
		else s[7-i] = 48;
	}
	s[8] = 0;
}

/* is the current selection empty? */

Boolean SelectionEmpty()
{
	TEHandle theTEH;
	DialogPeek dp;
	
	dp = (DialogPeek)BCDialog;
	theTEH = dp->textH;
	return ((**theTEH).selStart==(**theTEH).selEnd);
}

/* erase contents of all fields except one */

ClearAllFields(except)
short except;
{
	short i;
	Handle iH;
	short iType;
	Rect box;
	GrafPtr savePort;
	
	for (i=1;i<kAsciiDump;i++) {
		if (i!=except) {
			GetDItem(BCDialog,i,&iType,&iH,&box);
			SetIText(iH,"\p");
		}
	}
	for (i=0;i<4;i++) {
		asciiText[i][0] = 0;
		asciiNums[i][0] = 0;
	}
	
	/* and force redraw of the ascii dump area */
	
	GetDItem(BCDialog,kAsciiDump,&iType,&iH,&box);
	GetPort(&savePort);
	SetPort(BCDialog);
	InvalRect(&box);
	SetPort(savePort);
}

/* ================ Error reporting ================= */

/* Build error strings and force a redraw of the errors display area. */

ShowErrors()
{
	GrafPtr savePort;
	short	iType;
	Handle	iH;
	Rect	box;

	numErrors = 0;
	if (outOfRange)
			AddError("Out of range for a long integer.");
	if (maxCharsExceeded)
			AddError("No more characters allowed here.");
	if (pasteError == kPasteInvalid)
			AddError("Some paste characters invalid for this base.");
	else if (invalidChar)
			AddError("Invalid character for this base.");
	if (pasteError == kPasteTooLong)
				AddError("Paste text too long.");
	GetPort(&savePort);
	SetPort(BCDialog);
	GetDItem(BCDialog,kErrorDisplay,&iType,&iH,&box);
	InvalRect(&box);
	SetPort(savePort);
}

/* add an error to the error string array */

AddError(s)
char *s;
{
	numErrors++;
	if (numErrors>3) {
		DebugStr("\ptoo many errors");
		return;
	}
	strcpy(errorStr[numErrors-1],s);
}

/* clear all error flags */

ClearErrors()
{
	outOfRange = FALSE;
	invalidChar = FALSE;
	maxCharsExceeded = FALSE;
	pasteError = kPasteOK;
	ShowErrors();
}

/* ================ utility routines ================= */

/* remove any leading zeros (ascii48) */

StripLeadingZeros(s)
char *s;
{
	char s1[32];
	short i,len = 0;
	Boolean stripOn = TRUE;
	
	for (i=0;i<strlen(s),len<31;i++) {
		if ((s[i] != 48)||(!stripOn)) {
			s1[len] = s[i];
			len++;
			s1[len] = 0;
			stripOn = FALSE;
		}
	}
	strcpy(s,s1);
}

/* change a string to all uppser case */

ucase(s)
char *s;
{
	while (*s!=0) {
		*s = toupper((short)*s);
		s++;
	}
}

/* get a specified byte from a long int */

Byte GetByte(n,b)
long n;
short b;
{
	Byte it;
	short word;
	
	switch (b) {
		case 1:							/* LSB of lo word */
			word = LoWord(n);
			it = word&255;
		break;
		case 2:							/* MSB of lo word */
			word = LoWord(n);
			it = word>>8;
		break;
		case 3:							/* LSB of hi word */
			word = HiWord(n);
			it = word&255;
		break;
		case 4:							/* MSB of hi word */
			word = HiWord(n);
			it = word>>8;
		break;
	}
	return it;
}
/* ================ call back routines =============== */

/* ---------------- dialog user items ---------------- */

/* Dialog user item that draws the labels in the system font */

pascal void DrawLabels (myWindow,itemNo)
WindowPtr myWindow;
short itemNo;
{
	short iType,i;
	Handle iHandle;
	Rect iBox;
	short saveFont,saveSize;
	
	/* get the rectangle for this item and erase it */
	GetDItem(myWindow, itemNo, &iType, &iHandle, &iBox);
	EraseRect(&iBox);
	
	/* save the font in use */
	saveFont = myWindow->txFont;
	saveSize = myWindow->txSize;
	
	/* set a smaller one */
	TextFont(systemFont);
	TextSize(12);
	
	/* draw the text */
	MoveTo(iBox.left+6,iBox.top + 16);
	DrawString("\pDecimal");
	
	MoveTo(iBox.left+24,iBox.top + 40);
	DrawString("\pOctal");
	
	MoveTo(iBox.left+33,iBox.top + 65);
	DrawString("\pHex");
	
	MoveTo(iBox.left+16,iBox.top + 91);
	DrawString("\pBinary");
	
	MoveTo(iBox.left+27,iBox.top + 141);
	DrawString("\pAscii");
	
	/* reset font */
	TextFont(saveFont);
	TextSize(saveSize);
}

/* Dialog user item that displays the ascii field in alternate forms */

pascal void AsciiDump (myWindow,itemNo)
WindowPtr myWindow;
short itemNo;
{
	short iType,i;
	Handle iHandle;
	Rect iBox;
	short saveFont,saveSize;
	
	/* get the rectangle for this item and erase it */
	GetDItem(myWindow, itemNo, &iType, &iHandle, &iBox);
	EraseRect(&iBox);
	
	/* save the font in use */
	saveFont = myWindow->txFont;
	saveSize = myWindow->txSize;
	
	/* set a smaller one */
	TextFont(geneva);
	TextSize(9);
	
	/* draw the text */
	for (i=0;i<4;i++) {
		MoveTo(iBox.left+5+(i*22),iBox.top + 8);
		DrawString((StringPtr)asciiNums[i]);
		MoveTo(iBox.left+5+(i*22),iBox.top + 20);
		DrawString((StringPtr)asciiText[i]);
	}
	
	/* reset font */
	TextFont(saveFont);
	TextSize(saveSize);
}

/* display any error messages in small type at bottom of window */

pascal void ErrorDisplay (myWindow,itemNo)
WindowPtr myWindow;
short itemNo;
{
	short iType,i;
	Handle iHandle;
	Rect iBox;
	short saveFont,saveSize,width,center;
	
	/* get the rectangle for this item and erase it */
	GetDItem(myWindow, itemNo, &iType, &iHandle, &iBox);
	EraseRect(&iBox);
	
	/* if we have errors, draw them in a small font */
	if (numErrors!=0) {
		saveFont = myWindow->txFont;
		saveSize = myWindow->txSize;
		TextFont(geneva);
		TextSize(9);
		for (i=0;i<numErrors;i++) {
			CtoPstr(errorStr[i]);
			width = StringWidth((StringPtr)errorStr[i]);
			center = iBox.left + (iBox.right-iBox.left)/2;
			MoveTo(center - (width/2),iBox.top + (10*(i+1)));
			DrawString("\p! ");
			DrawString((StringPtr)errorStr[i]);
		}
		TextFont(saveFont);
		TextSize(saveSize);
		pasteError = kPasteOK;   // clear the flag
		invalidChar = false;	 // ditto
		SysBeep(0);
	}
	
	/* clear the error count */
	numErrors = 0;
}

/* User item code for default buttons in dialogs */

pascal void OutlineButton (myWindow,itemNo)
WindowPtr myWindow;
short itemNo;
/* outliner for the ok button */
{
	short iType;
	Handle iHandle;
	Rect iBox;
	
	GetDItem(myWindow, itemNo, &iType, &iHandle, &iBox);
	InsetRect(&iBox, -4, -4);
    PenSize(3, 3);
	FrameRoundRect(&iBox, 16, 16);
	PenNormal();
}

/* ---------------- graf procs ---------------- */

/* replace normal drawing so that I can show a null character and CR 
   as an undefined character. */
   
pascal void MyStdText(byteCount,textBuf,numer,denom)
short	byteCount;
QDPtr	textBuf;
Point	numer,denom;
{
	short i;
	char c;
	
	for (i=0;i<byteCount;i++) {
		c = *(textBuf+i);
		if ((c==0)||(c==13))
			c = 255;
		StdText(1,&c,numer,denom);
	}
}

/* override this to give width to the null character and CR */

pascal short MyStdTextMeas(byteCount,textBuf,numer,denom,info)
short	byteCount;
QDPtr	textBuf;
Point	*numer,*denom;
FontInfo *info;
{
	short i,width;
	char c;
		
	SetPort(BCDialog);
	width = StdTxMeas(byteCount,textBuf,numer,denom,info);
	for (i=0;i<byteCount;i++) {
		c = *(textBuf+i);
		if ((c==0)||(c==13)) {
			c = 255;
			width = width + StdTxMeas(1,&c,numer,denom,info);
		}
	}
	return width;
}

/* ---------------- TE word break ---------------- */

/* don't break anywhere.  We want to 'text edit" even non-printing characters. */

pascal Boolean MyWordBreak(Ptr text,short charPos)
{
	return FALSE;
}


/* ScriptManager code from David Greenfield */

/************************************************************************/
void SyncKeyboard2Font(short fontID )
{
	short	fontScript;
	long	keyboard;
	
	if (!TrapAvailable(_ScriptUtil))
		return;
		
	fontScript = Font2Script( fontID );
	keyboard   = GetEnvirons( smKeyScript );
	
	if( keyboard != (long) fontScript )
		KeyScript( fontScript );
}
