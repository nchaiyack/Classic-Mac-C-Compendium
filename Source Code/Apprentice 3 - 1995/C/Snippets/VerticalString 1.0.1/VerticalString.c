// VerticalString 1.0.1
// ported to CodeWarrior by Ken Long (kenlong@netcom.com)
// updated for CodeWarrior 6 on 950712

#define over qd.screenBits.bounds.right
#define down qd.screenBits.bounds.bottom

MenuHandle	appleMenu, fileMenu, editMenu;

enum {
	appleID = 1,
	fileID,
	editID
};

enum {
	openItem = 1,
	closeItem,
	quitItem = 4
};


WindowPtr	shell_window, about_window;
Rect		dragRect;
Rect		windowBounds, about_bounds; 
Rect		strRect ={ 5, 20, 364, 40 };
short			width = 1;

// Prototypes.

void SetUpWindow (void);
void DoAbout (void);
void SetUpMenus (void);
void VerticalString (Rect word_rect, Str255 word_string);
void StyleTheText (short whatFont, short whatStyle, short whatSize);
void DrawTheStrings (short active);
void AdjustMenus (void);
static enable (MenuHandle menu, short item, short ok);
void HandleMenu (long mSelect);
void InitMacintosh (void);
void HandleMouseDown (EventRecord *theEvent);
void HandleEvent (void);
void main (void);

void SetUpWindow (void)
{
	dragRect = qd.screenBits.bounds;
	
	SetRect (&windowBounds, 2, 22, over - 4, down - 4);
	
	shell_window = NewWindow (0L, &windowBounds, "\pKen's Shell", true, 
							  plainDBox, (WindowPtr) -1L, true, 0);
	SetPort (shell_window);
}

void SetUpMenus (void)
{
	InsertMenu (appleMenu = NewMenu (appleID, "\p\024"), 0);
	InsertMenu (fileMenu = NewMenu (fileID, "\pFile"), 0);
	InsertMenu (editMenu = NewMenu (editID, "\pEdit"), 0);
	DrawMenuBar ();
	AppendMenu (appleMenu, "\pAbout VerticalString");
	AddResMenu (appleMenu, 'DRVR');
	AppendMenu (fileMenu, "\pOpen/O;Close/W;(-;Quit/Q");
	AppendMenu (editMenu, "\pUndo/Z; (-;Cut/X;Copy/C;Paste/V;Clear");
}

void VerticalString (Rect word_rect, Str255 word_string)
{
	#define center 5	// center of label is 'center' pixels from left of rect.
	
	short numChars, leading, stringCenter, x, curVert;
	FontInfo fInfo;
	
	GetFontInfo (&fInfo);
	leading = fInfo.ascent + fInfo.descent + fInfo.leading;
	stringCenter = word_rect.left - center;

//	ForeColor (redColor);	//¥ Color here and not elsewhere for same.
	curVert = word_rect.top + leading;	// primed for first character.
	numChars = strlen (word_string);
	for (x = 1; x < numChars; x++)
	{
		MoveTo (stringCenter - (CharWidth (word_string[x]) / 2), 
				curVert);
		DrawChar ((char) word_string[x]);
		curVert = curVert + leading;
	}
}

void StyleTheText (short whatFont, short whatStyle, short whatSize)
{
	TextFont (whatFont);
	TextFace (whatStyle);
	TextSize (whatSize);
}

void DrawTheStrings (short active)
{
	short		color = true;
	
	SetPort (shell_window);
	
	ForeColor (blueColor);	// oldStyle color.
	StyleTheText (monaco, 0, 9);
	VerticalString (strRect, "\pMy name is Long but this is not\0");
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\pChinese writing.");
	
	ForeColor (redColor);
	StyleTheText (newYork, outline, 14);
	OffsetRect (&strRect, 40, 0);
	VerticalString (strRect, "\pSuperman\0");
	
	ForeColor (blackColor);
	StyleTheText (monaco, 0, 9);
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\pStrange visitor from another\0");
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\pplanet who came to Earth with\0");
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\ppowers and abilities far beyond\0");
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\pthose of mortal men. Then went\0");
	StyleTheText (0, 0, 12);
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\pto Chicago!\0");
	
	ForeColor (magentaColor);
	StyleTheText (monaco, 0, 9);
	OffsetRect (&strRect, 40, 0);
	VerticalString (strRect, "\pThe VerticalString routine came\0");
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\pfrom SplatMaster Pascal source,\0");
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\pby Jon Benton, and ported to C\0");
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\pby Kenneth A. Long in mid 1992.\0");
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\pIn May, 1994 it was put in this\0");
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\pdemo, for YOUR enjoyment as an.\0");

	StyleTheText (monaco, bold, 9);
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\pitty bitty bytes(tm) product.\0");
	
	StyleTheText (monaco, 0, 9);
	ForeColor (greenColor);
	OffsetRect (&strRect, 40, 0);
	VerticalString (strRect, "\pVertical strings are excellent\0");
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\pfor labeling vertical scrolls.\0");
	
	ForeColor (blackColor);
	OffsetRect (&strRect, 40, 0);
	VerticalString (strRect, "\pBullseye was used for the shell.\0");
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\pJon Benton wrote SplatMaster.\0");
	OffsetRect (&strRect, 20, 0);
	VerticalString (strRect, "\pVertical String Demo is PD.\0");
	StyleTheText (0, 0, 12);
}

void AdjustMenus (void)
{
	register WindowPeek wp = (WindowPeek) FrontWindow ();
	short kind = wp ? wp->windowKind : 0;
	Boolean DA = kind < 0;
	
	enable (editMenu, 1, DA);
	enable (editMenu, 3, DA);
	enable (editMenu, 4, DA);
	enable (editMenu, 5, DA);
	enable (editMenu, 6, DA);
	
	enable (fileMenu, openItem, ! ((WindowPeek) shell_window)->visible);
	enable (fileMenu, closeItem, DA || ((WindowPeek) shell_window)->visible);
}

static enable (MenuHandle menu, short item, short ok)
{
	if (ok)
		EnableItem (menu, item);
	else
		DisableItem (menu, item);
}

void HandleMenu (long mSelect)
{
	short		menuID = HiWord (mSelect);
	short		menuItem = LoWord (mSelect);
	Str255		name;
	GrafPtr		savePort;
	WindowPeek	frontWindow;
	
	switch (menuID)
	{
		case	appleID:
			if (menuItem == 1)
				;	//¥ Put your DoAboutBox call here.
			else
				{
					GetPort (&savePort);
					GetItem (appleMenu, menuItem, name);
					OpenDeskAcc (name);
					SetPort (savePort);
			}
		break;
		
		case	fileID:
			switch (menuItem)
			{
				case	openItem:
					ShowWindow (shell_window);
					SelectWindow (shell_window);
				break;
  							
				case	closeItem:
					if ((frontWindow = (WindowPeek) FrontWindow ()) == 0L)
				break;
			
				if (frontWindow->windowKind < 0)
					CloseDeskAcc (frontWindow->windowKind);
				else 
					if (frontWindow = (WindowPeek) shell_window)
						HideWindow (shell_window);
  					break;
  					
				case	quitItem:
					ExitToShell ();
				break;
			}
		break;
  				
		case	editID:
			if (!SystemEdit (menuItem-1))
				SysBeep (5);
		break;
	}
}

void InitMacintosh (void)
{
	MaxApplZone ();
	
	InitGraf (& (qd.thePort));
	InitFonts ();
	FlushEvents (everyEvent, 0);
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (0L);
	InitCursor ();
}

void HandleMouseDown (EventRecord	*theEvent)
{
	WindowPtr	theWindow;
	short			windowCode = FindWindow (theEvent->where, &theWindow);
	
    switch (windowCode)
    {
		case inSysWindow: 
			SystemClick (theEvent, theWindow);
		break;
	
		case inMenuBar:
			AdjustMenus ();
			HandleMenu (MenuSelect (theEvent->where));
		break;
	
		case inDrag:
			if (theWindow == shell_window)
				DragWindow (shell_window, theEvent->where, &dragRect);
		break;
		
		case inContent:
//			if (theWindow == shell_window)
//			{
//				if (theWindow != FrontWindow ())
//					SelectWindow (shell_window);
//				else
//					InvalRect (&shell_window->portRect);
//			}
		break;
		
		case inGoAway:
			if (theWindow == shell_window && 
				TrackGoAway (shell_window, theEvent->where))
				HideWindow (shell_window);
		break;
	}
}

void HandleEvent (void)
{
	short			ok;
	EventRecord	theEvent;

	HiliteMenu (0);
	SystemTask ();		/* Handle desk accessories */
	
	ok = GetNextEvent (everyEvent, &theEvent);
	if (ok)
	switch (theEvent.what)
	{
		case mouseDown:
			HandleMouseDown (&theEvent);
		break;
			
		case keyDown: 
		case autoKey:
		if ((theEvent.modifiers & cmdKey) != 0)
		{
			AdjustMenus ();
			HandleMenu (MenuKey ((char) (theEvent.message & charCodeMask)));
		}
		break;
			
		case updateEvt:
			BeginUpdate (shell_window);
				EraseRect (&windowBounds);
				DrawTheStrings (((WindowPeek) shell_window)->hilited);
			EndUpdate (shell_window);
		break;
		
		case activateEvt:
			InvalRect (&shell_window->portRect);
		break;
	}
}

void main ()
{
	InitMacintosh ();
	SetUpMenus ();
	SetUpWindow ();
	
	for (;;)
		HandleEvent ();
}

