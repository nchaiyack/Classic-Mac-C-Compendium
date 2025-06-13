// FontShow version 1.1.3, updated 950712 for CodeWarrior 6
//¥-----------------------------------------------------------------------o//
//¥	Note from Kenneth A. Long:
//¥ 	I found this source on the net, but it was only the .c and .h file.
//¥ Then after a time, I found the built application in a different place
//¥ and was able to get the resources out of it.  Then I made the .r file
//¥ from it, for general principles.
//¥ The two includes I excluded caused link errors I couldn't fix, but it
//¥ seems to run okay at first glance.  I've done hardly any testing, and
//¥ I'm sure there could be improvements, refinements and more thorough
//¥ code added.  But HEY!  It's C source!

//¥ 	 6. March	  1994 (v1.1.2) Compiles/runs - Think C v5.0.4/6.0.1
//¥-----------------------------------------------------------------------o//
//o The FontShow

//o (c) 1989 Rainer Fuchs, Am Weingarten 13, 6000 Frankfurt 90, FRG

//o History:
//¥ 	23. November  1994 (v1.1.2) Runs on Code Warrior C.
//¥ 	06. March	  1994 (v1.1.2) Runs on Think C.
//o 	17. Oktober	  1989 (v1.1.1) Compatible with EZ-Menu 
//o 	10. Oktober	  1989 (v1.1)	Font number added; Fontsize added
//o 	 3. Oktober	  1989 (v1.0)	multiple sample strings
//o 	 2. Oktober	  1989 (v1.0b3) changed print format 
//o 	26. September 1989 (v1.0b2) changed print format
//o 	25. September 1989 (v1.0b1) added character table 
//o 	24. September 1989 (v0.0d1) first hack
//o-----------------------------------------------------------------------o//

#include "fontshow.h"

//o-----------------------------------------------------------------------o//
//o Error and information handling ---------------------------------------o//

void ErrorMsg (short index)
{
	Str255 theString;

	InitCursor ();
	GetIndString (theString, 128, index);//o get error msg from rsrc.
	ParamText (theString, "\p", "\p", "\p");
	StopAlert (128, NIL);			//o show error dialog.
}

//o-----------------------------------------------------------------------o//

void ShowInfo ()					//o About FontShowÉ.
{
	DialogPtr 	myDialog;
	EventRecord	myEvent;

	myDialog=GetNewDialog (ABOUT, NIL, (WindowPtr)-1);//o get dialog from rsrc.
	DrawDialog (myDialog);					//o draw it.
	while (!GetNextEvent (keyDownMask|autoKeyMask|mDownMask, &myEvent))
		SystemTask ();				//o wait until event.
	DisposDialog (myDialog);		//o dispose of dialog.
}

//o-----------------------------------------------------------------------o//
//o Sample text output ---------------------------------------------------o//
//o-----------------------------------------------------------------------o//

Boolean BreakCheck ()				//o check for break after each page.
{
	EventRecord theEvent;
	WindowPtr 	whichWindow;

 	for (;;) 
 	{	//o mouse clicks in menu bar ignored for comp. with EZ-Menu 
 		//o *sigh*.
 		while (!GetNextEvent (keyDownMask|autoKeyMask|mDownMask, &theEvent))
			SystemTask ();				//o wait until event.
 		if ((theEvent.what == keyDown) || (theEvent.what == autoKey)) //o keyDown?.
			return ((char)BitAnd (theEvent.message, charCodeMask) == '.' &&
						BitAnd (theEvent.modifiers, cmdKey) != 0);
			//o if Cmd-dot	then TRUE else FALSE.
 		else
 			if (theEvent.what == mouseDown && FindWindow (theEvent.where, &whichWindow)!=inMenuBar)
 					return (FALSE);//o click not in MenuBar.
 	}
}

//o-----------------------------------------------------------------------o//

void DrawPage (short i)//o draw sample page.
{
	myFontInfo 	*myFont;
	short 			lineNr=0;
	char 			*pos;
	short from, to, nr;
	short c, x, y;
	short			lineH;
	short			wMax;
	Str255		myStr;
	char			buf[4];

	myFont= *fontHdl;				//o Block is already locked !.
	lineH=myFont[i].lineHeight;
	wMax=myFont[i].widMax;

	TextFont (0);					//o draw font name in system font.
	TextSize (12);					//o and 12 pt size.
	MoveTo (TOPH, TOPF);			//o	goto title line.
	DrawString (myFont[i].fontName);//o draw font name.
	DrawString ("\p (ID=");
//	stci_d (buf, myFont[i].fontNum, 3);	//o KAL toox it out - couldn't...
	DrawString (CtoPstr (buf));
	DrawString ("\p) ");
//	stci_d (buf, fontSize, 2);			//o find in headers.
	DrawString (CtoPstr (buf));
	DrawString ("\p point.");

	TextFont (myFont[i].fontNum);//o set font.
	TextSize (fontSize);

	if (RealFont (myFont[i].fontNum, fontSize)) 
	{
		//o sample text.
		if (opt == SAMPLE || prFlag) 
		{
			lineNr++;
			HLock (myText);
			from=nr=to=0;
			pos=*myText;
			
			//o break text into single lines.
			while (to<textLength) 
			{	
				to++;
				
				//o search for CR.
				if ((*pos++ == 0x0D) || (to == textLength)) 
				{
					MoveTo (TOPH, TOPV + lineH*lineNr++ );//o go to next line.
					DrawText (*myText, from, to-from);//o draw line.
					from = to;
				}
			}
			HUnlock (myText);
		}

		//o sample string.
		if (opt == STRING || prFlag ) 
		{
			for (i=1;i;i++) 
			{
				lineNr++;
				GetIndString (myStr, STRINGS, i);
				if (*myStr) 
				{
					MoveTo (TOPH, TOPV + lineH*lineNr++ );
					DrawString (myStr);
				}
				else break;
			}
		}
		//o character map.
		if (opt == MAP || prFlag) 
		{
			lineNr++;
			
			//o only printable characters.
			for (c = 32; c < 256; c++) 
			{	
				//o 32 char per row.
				if (c % 32 == 0) 
				{
					x = TOPH;
					y = TOPV + lineH*lineNr++;	//o new line.
				}
				MoveTo (x, y);
				if (isprint (c))				//o skip 127.
					DrawChar ((char)c);
				else
					DrawChar (32);
				x += wMax;					//o makes a pretty output.
			}
		}
	}
	else 
		{
			TextSize (12);
			TextFont (0);
			DrawString ("\p Font size not available.");
	}
}

//o-----------------------------------------------------------------------o//

void ShowFonts ()					//o Display fonts.
{
	short i;
	DialogPtr	myDialog;
	WindowPtr 	myWindow;

	prFlag = FALSE;

	if (SelectFonts ()) 
	{			//o only when select dialog is ok.
		myWindow = GetNewWindow (FONTWINDOW, NIL, (WindowPtr)-1L); //o open a window.
		myDialog = GetNewDialog (SCREENBREAK, NIL, myWindow);//o show break information.
		DrawDialog (myDialog);
		SetPort (myWindow);
		HLock ((Handle) fontHdl);
		for (i = 0; i < fontCount; i++) 
		{
			//o draw only selected fonts.
			if ((*fontHdl)[i].flag)
			{
				EraseRect (&myWindow->portRect);	//o	clear page.
				DrawPage (i);						//o draw sample page.
				if (BreakCheck ())					//o check for break.
					break;
			}
		}
		HUnlock ((Handle) fontHdl);
		DisposeWindow (myWindow);//o close window and dialog.
		DisposDialog (myDialog);
	}
}

//o-----------------------------------------------------------------------o//

short HowMany ()
{
	return (((**prRecHdl).prJob.bJDocLoop == bDraftLoop) ? 
				(**prRecHdl).prJob.iCopies : 1 );
}

//o-----------------------------------------------------------------------o//

void PrintFonts ()					//o print fonts.
{
	TPPrPort		myPrPort;
	TPrStatus	myPrStatus;
	DialogPtr 	mySpoolDlg, myPrintDlg;
	GrafPtr		savePort;
	short i;
	short copies;
	short 			curs=0;

	prFlag=TRUE;					//o indicate printing.

	GetPort (&savePort);			//o save old grafPort.
	PrOpen ();						//o open PrintMgr.
	
	//o get print job infos.
	if (SelectFonts () && PrJobDialog (prRecHdl)) 
	{ 
		//o just in case last printout was cancelled by user:.
		PrintErr=noErr;
		for (copies = HowMany (); copies > 0 && PrintErr == noErr; copies--) 
		{
			//o show status information:.
			if ((**prRecHdl).prJob.bJDocLoop == bSpoolLoop) 
				DrawDialog (mySpoolDlg=GetNewDialog (SPOOLINFO, NIL, (WindowPtr)-1));
			else
				DrawDialog (myPrintDlg=GetNewDialog (PRINTINFO, NIL, (WindowPtr)-1));

			SetCursor (*myCursor[curs++]);//o we«re busy (rotating watch.
			if (curs == 4) curs = 0;

			myPrPort = PrOpenDoc (prRecHdl, NIL, NIL); //o open printPort.
			SetPort ((GrafPtr) myPrPort);
			TextSize (12);
			HLock ((Handle) fontHdl);
			for (i = 0; i < fontCount; i++) 
			{

				SetCursor (*myCursor[curs++]);//o still busy.
				if (curs == 4) curs = 0;

				if (PrintErr == noErr) 
				{
					//o print only selected fonts.
					if ((*fontHdl)[i].flag)
					{
						PrOpenPage (myPrPort, NIL);
						if (PrintErr == noErr)
							DrawPage (i);			//o print sample page.
						PrClosePage (myPrPort);
					}
				}
			}
			HUnlock ((Handle) fontHdl);
			PrCloseDoc (myPrPort);//o close printPort.

			//o now print spool file.
			if ((**prRecHdl).prJob.bJDocLoop == bSpoolLoop && PrintErr == noErr) 
			{
				DisposDialog (mySpoolDlg);
				DrawDialog (myPrintDlg=GetNewDialog (PRINTINFO, NIL, (WindowPtr)-1));
				PrPicFile (prRecHdl, NIL, NIL, NIL, &myPrStatus); //o print spool file.
			}

			DisposDialog (myPrintDlg);
		}

		InitCursor ();
		if (PrintErr!=noErr)
				ErrorMsg (ERR_PRINT);//o something went wrong.
		else
			SysBeep (10);			//o notify user.
		PrClose ();					//o close printing manager.
		SetPort (savePort);		//o reset old grafPort.
	}
}

//o-----------------------------------------------------------------------o//
//o Font dialog handling -------------------------------------------------o//
//o-----------------------------------------------------------------------o//

pascal void	DrawList(WindowPtr theWindow, short itemNo)
{
	Rect 		rView;
	RgnHandle 	theRgn;
	short 		itemType;
	Handle 		item;
	Rect 		box;
		
	//o get rect of OK button.
	GetDItem (theWindow, OK, &itemType, &item, &box); 
	PenSize (3, 3);
	InsetRect (&box, -4, -4);
	FrameRoundRect (&box, 16, 16);	//o draw bold.
	PenSize (1, 1);

	HLock ((Handle) myList);
	rView= (**myList).rView;
	InsetRect (&rView, -1, -1);
	FrameRect (&rView);				//o frame around list.
	theRgn= (*theWindow).visRgn;
	LUpdate (theRgn, myList);		//o update list.
	HUnlock ((Handle) myList);
}

//o-----------------------------------------------------------------------o//

pascal Boolean	myFilter(DialogPtr theDialog, EventRecord *theEvent, 
								short *itemHit)
{
	char 	charCode;
	GrafPtr savePort;
	Cell 	theCell;
	Boolean flag;

	if ((theEvent->what == keyDown) || (theEvent->what == autoKey)) 
	{ 
		charCode= (char)BitAnd (theEvent->message, charCodeMask);
		
		//o OK button is default.
		if ((charCode == 13) || (charCode == 3)) 
		{ 
			*itemHit=1;
			return (TRUE);
		}
		else if ((charCode == '.') && (BitAnd (theEvent->modifiers, cmdKey) != 0)) 
		{								//o Cmd-dot cancels dialog.
			*itemHit=2;
			return (TRUE);
		}
		else return (FALSE);
	}

	else 
		{
			if (*itemHit == FLIST) 
			{	//o list handling.
					GetPort (&savePort);
					SetPort ((**myList).port);

					//o convert to local coordinates.
					GlobalToLocal (& (theEvent->where));

					//o simulate cmd key to fool default list
					//o definition routine !!!.
					flag = LClick (theEvent->where, 
								   theEvent->modifiers|cmdKey, 
								   myList);
								   
					SetPort (savePort);
			}

			if (flag == TRUE) 
			{		//o double click is equal to ok.
					*itemHit=1;
					return (TRUE);
				}
				else
					return (FALSE);
	}
}

//o-----------------------------------------------------------------------o//
//o Initialize font list. ------------------------------------------------o//
//o-----------------------------------------------------------------------o//

void InitList (WindowPtr theWindow)
{
	short 			itemType;
	Handle 		item;
	Point 		csize, theCell;
	Rect 			rView, dataBounds;
	short i;
	myFontInfo 	*myFont;

	GetDItem (theWindow, FLIST, &itemType, &item, &rView);
	SetRect (&dataBounds, 0, 0, 1, fontCount);//o set borders of list.
	SetPt (&csize, 0, 0);
	rView.right-=15;				//o leave room for scroll bar.
	myList=LNew (&rView, &dataBounds, csize, 0, theWindow, TRUE, FALSE, FALSE, TRUE);

	HLock ((Handle) fontHdl);
	myFont= *fontHdl;

	//o fill list with entries from myFontInfo array.
	for (i=0; i<fontCount;i++) 
	{
		SetPt (&theCell, 0, i);
		LSetCell (myFont[i].fontName+1, *myFont[i].fontName, theCell, myList);
		LSetSelect (myFont[i].flag, theCell, myList);//o preselect entries.
	}
	HUnlock ((Handle) fontHdl);
}

//o-----------------------------------------------------------------------o//

Boolean SelectFonts ()			//o handle font select dialog.
{ 
	short 		i;
	short		button;
	short 		itemType;
	Handle 		item;
	Rect 		box;
	Cell 		theCell;
	Boolean 	stop=FALSE;
	myFontInfo 	*myFont;
	DialogPtr 	myDialog = GetNewDialog (FONTLIST, NIL, (WindowPtr)-1);
											//o Auswahl-Dialog.
	
	GetDialogItem(myDialog, FLIST, &itemType, &item, &box);//o get rect of list button.
	SetDialogItem(myDialog, FLIST, userItem, (Handle)DrawList, &box); //o install user item.
	InitList (myDialog); 			//o initialize list.
 
	ShowWindow (myDialog);		//o make dialog visible.
	while (!stop) 
	{
		SystemTask ();
		ModalDialog (myFilter, &button);//o Dialog anzeigen.

		switch (button) 
		{
			case EXALL:				//o deselect all fonts.
				for (i=0; i<fontCount;i++) 
				{
						SetPt (&theCell, 0, i);
						LSetSelect (FALSE, theCell, myList);
				}
			break;

			case INALL:				//o select all fonts.
				for (i=0; i<fontCount;i++) 
				{
						SetPt (&theCell, 0, i);
						LSetSelect (TRUE, theCell, myList);
				}
			break;

			case REVERT:			//o revert to old. 
				HLock ((Handle) fontHdl);
				myFont= *fontHdl;
				for (i=0; i<fontCount;i++) 
				{
						SetPt (&theCell, 0, i);
						LSetSelect (myFont[i].flag, theCell, myList);
				}
				HUnlock ((Handle) fontHdl);
			break;

			case Cancel:			//o dialog cancelled.
				stop=TRUE;
			break;

			case OK:
 				HLock ((Handle) fontHdl);
				myFont= *fontHdl;
				for (i=0; i<fontCount;i++) 
				{//o update myFontInfo array.
						SetPt (&theCell, 0, i);
						myFont[i].flag=LGetSelect (FALSE, &theCell, myList);
				}
				HUnlock ((Handle) fontHdl);
				stop=TRUE;
				break;
		} //o end case.
	}

	LDispose (myList);				//o dispose of list.
	DisposDialog (myDialog);					//o dispose of dialog.
	return ((button == OK)?TRUE:FALSE);
}

//o-----------------------------------------------------------------------o//
//o Event handling -------------------------------------------------------o//
//o-----------------------------------------------------------------------o//

void HandleMenu (sel)				//o handle menu bar.
long sel;
{
	short 	theItem=LoWord (sel);
	Str255 name;
	GrafPtr savePort;
	short i;
	myFontInfo *myFont;
	FontInfo	fInfo;

	switch (HiWord (sel)) 
	{
		case APPLE_M:
			GetItem (myMenus[APPLE], theItem, (StringPtr) &name);
			if (theItem == INFO)		//o AboutÉ.
				ShowInfo ();
			else 
				{
					GetPort (&savePort);//o save old grafPort (just in caseÉ).
					OpenDeskAcc ((StringPtr) &name);//o open DA.
					InitCursor ();		//o DA may have changed cursor.
					SetPort (savePort);	//o	DA may have changed grafPort.
			}
		break;

		case FILE_M:
			switch (theItem) 
			{
				case SETUP:
					PrOpen ();		//o open/close PrintMgr acc. to TN.
					PrStlDialog (prRecHdl);//o get page setup.
					PrClose ();
				break;

				case DISPLAY:
					ShowFonts ();
				break;

				case PRINT:
					PrintFonts ();
				break;

				case QUIT:
					quit=TRUE;
				break;
			}
		break;

		case EDIT_M: 
			SystemEdit (theItem-1); //o our application doesn«t handle EDIT menu.
		break;

		case OPTIONS_M:
			CheckItem (myMenus[OPTIONS], opt, FALSE); //o deselect menu item.
			CheckItem (myMenus[OPTIONS], opt=theItem, TRUE); //o select new Item.
		break;

		case SIZE_M:
			if (theItem!=oldItem) 
			{
				SetCursor (*myCursor[0]);
				CheckItem (myMenus[SIZE], oldItem, FALSE);
				CheckItem (myMenus[SIZE], oldItem=theItem, TRUE);
				switch (oldItem) 
				{
					case SIZE9:
						fontSize=9;
					break;

					case SIZE10:
						fontSize=10;
					break;

					case SIZE12:
						fontSize=12;
					break;

					case SIZE14:
						fontSize=14;
					break;

					case SIZE18:
						fontSize=18;
					break;

					case SIZE20:
						fontSize=20;
					break;

					case SIZE24:
						fontSize=24;
					break;
				}
				TextSize (fontSize);
				HLock ((Handle) fontHdl);
				myFont= *fontHdl;
				for (i = 0; i < fontCount; i++) 
				{
				TextFont (myFont[i].fontNum);				//o set font.
				GetFontInfo (&fInfo);							//o and get font info.
				myFont[i].lineHeight = fInfo.ascent+fInfo.descent+fInfo.leading; //o calc. line height.
				myFont[i].widMax=fInfo.widMax;			//o calc. char width.
			}
			HUnlock ((Handle) fontHdl);
			TextSize (12);
			TextFont (0);
			InitCursor ();
		}
		break;
	}
	HiliteMenu (0);
}

//o-----------------------------------------------------------------------o//

void HandleMouseDown (theEvent)//o handle mouseDown events.
EventRecord theEvent;
{
	WindowPtr whichWindow;

	switch (FindWindow (theEvent.where, &whichWindow )) 
	{
		case inDesk:
			SysBeep (10);
		break;

		case inMenuBar:
			HandleMenu (MenuSelect (theEvent.where) ) ;
		break;

		case inSysWindow:
			SystemClick (&theEvent, whichWindow );
		break;
	} //o end switch.
}

//o-----------------------------------------------------------------------o//
//o MainLoop () -----------------------------------------------------------o//
//o-----------------------------------------------------------------------o//

void MainLoop ()
{
	EventRecord theEvent;

	quit = FALSE;
	while (!quit)
	{
		SystemTask ();
		if (GetNextEvent (everyEvent, &theEvent))
			switch (theEvent.what) 
			{
				case mouseDown:	//o mouse click.
					HandleMouseDown (theEvent);
				break;

				case keyDown: 		//o key click.
				case autoKey:
					if ((theEvent.modifiers & cmdKey) != 0) //o we handle only Cmd-key.
						HandleMenu (MenuKey ((char) (theEvent.message & charCodeMask)));
				break;
			} //o end switch (and if).
	}//o end while.
}

//o-----------------------------------------------------------------------o//
//o Initialization and main () --------------------------------------------o//
//o-----------------------------------------------------------------------o//

void InitMenu ()					//o Draw menu bar.
{
	short i;

	myMenus[APPLE]=GetMenu (APPLE_M);
	AddResMenu (myMenus[APPLE], 'DRVR');//o insert DAs in Apple-menu.

	myMenus[FILE]=GetMenu (FILE_M);
	myMenus[EDIT]=GetMenu (EDIT_M);
	myMenus[OPTIONS]=GetMenu (OPTIONS_M);
	myMenus[SIZE]=GetMenu (SIZE_M);

	CheckItem (myMenus[OPTIONS], opt = SAMPLE, TRUE); //o preselect "sample text".
	CheckItem (myMenus[SIZE], oldItem=SIZE12, TRUE); //o preselect font size 12.
	fontSize=12;

	for (i = 0; i < 5; InsertMenu (myMenus[i++], 0))
		;
	DrawMenuBar ();
}

//o-----------------------------------------------------------------------o//

void InitText ()					//o get sample text from rsrc.
{
	myText = GetResource ('TEXT', 128);
	textLength= (short)SizeResource (myText);
}

//o-----------------------------------------------------------------------o//

void InitPrint ()
{
	PrOpen ();
	prRecHdl= (THPrint)NewHandle (sizeof (TPrint));//o get new print record.
	if (prRecHdl)
		PrintDefault (prRecHdl);	//o validate it.
	else
		ErrorMsg (ERR_NOROOM);
	PrClose ();
}

//o-----------------------------------------------------------------------o//

void BuildList ()					//o build font list.
{
	register 	short i;
	myFontInfo	*myFont;
	FontInfo 	fInfo;
	MenuHandle	tempMenu;

	//o Get a fake menu; use the Menu Manager to fill it with font names - 
	//o the Menu Manager sorts entries alphabetically !.

	tempMenu = NewMenu (TEMP_M, "\px");
	AddResMenu (tempMenu, 'FONT');
	fontCount=CountMItems (tempMenu);

	//o dynamic allocation of heap space for myFontInfo array.
	if (fontHdl= (myFontInfo **)NewHandle (fontCount*sizeof (myFontInfo))) 
	{
		HLock ((Handle) fontHdl);
		TextSize (12);
		myFont= *fontHdl;
		for (i=0;i<fontCount;i++) 
		{
				GetItem (tempMenu, i+1, myFont[i].fontName);//o get font names.
			GetFNum (myFont[i].fontName, &myFont[i].fontNum);//o get font numbers.
			TextFont (myFont[i].fontNum);				//o set font.
			GetFontInfo (&fInfo);							//o and get font info.
			myFont[i].lineHeight = fInfo.ascent+fInfo.descent+fInfo.leading; //o calc. line height.
			myFont[i].widMax=fInfo.widMax;			//o calc. char width.
			myFont[i].flag=TRUE;							//o preselect font.
		}
		HUnlock ((Handle) fontHdl);
	}
	else
		ErrorMsg (ERR_NOROOM);

	DisposeMenu (tempMenu);//o WeÕre done with the menu; dispose of it.
	TextFont (0);	//o and reset system font.
}

//o-----------------------------------------------------------------------o//

void InitThings ()
{
	DialogPtr myDialog;
	short i;

	InitGraf (&qd.thePort); 
	MaxApplZone ();						//o we want it all, and we want it NOW !.
	MoreMasters ();					//o extra pointer blocks at the bottom of the heap.
	MoreMasters ();					//o this is 5 X 64 master pointers.
	MoreMasters ();
	MoreMasters ();
	MoreMasters ();

	InitFonts ();			//o startup the font manager.
	InitWindows ();						//o startup the window manager.
	InitMenus ();					//o startup the menu manager.
	TEInit ();						//o startup the text edit manager.
	InitDialogs (0);				//o startup the dialog manager.

	FlushEvents (everyEvent, 0);

//o get the cursors we use and lock them down - no clutter.

	myCursor[0] = GetCursor (watchCursor);
	myCursor[1] = GetCursor (128);
	myCursor[2] = GetCursor (129);
	myCursor[3] = GetCursor (130);

	for (i = 0; i < 4; i++) 
	{
		MoveHHi ((Handle)myCursor[i]);
		HLock ((Handle)myCursor[i]);
	}

	//o tell user we«re busy.
	myDialog = GetNewDialog (WAIT, NIL, (WindowPtr)-1); 
	DrawDialog (myDialog);
	InitMenu ();						//o build menus.
	InitText ();						//o get sample text.
	InitPrint ();					//o initialize printing.
	BuildList ();					//o build font name list.
	DisposDialog (myDialog);
	InitCursor ();					//o show arrow cursor.
}

//o-----------------------------------------------------------------------o//

void CleanUp ()
{
	short i;

	if (fontHdl)
		DisposHandle ((Handle) fontHdl);
	if (prRecHdl)
		DisposHandle ((Handle) prRecHdl);
	for (i=0;i<4;i++)
		HUnlock ((Handle)myCursor[i]);
}

//o-----------------------------------------------------------------------o//

void main ()
{
	InitThings ();							//o initializations.
	if (fontHdl && prRecHdl)
		MainLoop ();							//o main routine.
	CleanUp ();								//o clean up.
}

//o-----------------------------------------------------------------------o//
//o Show's over, folks!
//o-----------------------------------------------------------------------o//
