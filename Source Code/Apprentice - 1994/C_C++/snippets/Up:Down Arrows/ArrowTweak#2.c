//� UpDownArrowTester.c

//� By Eddy J. Gurney

//� Please send any improvements or bugs in the "heart" of this code
//� (the part that actually does the up/down arrow thing) to
//� <egurney@vcd.hp.com>.  I don't care about bugs in the "shell" 
//� portion; it's just a quick hack used to test the up/down arrow thing!

//� If you find this code useful, a "thanks" in the About box of your
//� app or an e-mail message would always be appreciated. :-)

#include <Palettes.h>

#define NIL					0L

typedef enum DialogMode 
{
	Default, 		//� "Control" keys only.
	AllowAlpha,
	AllowPrint,
	AllowDigit,
	AllowHexDigit,
	AllowAlphaNum	
} DialogMode;

//� ASCII codes */
#define enterKey			0x03
#define returnKey			0x0d
#define escapeKey			0x1b

DialogMode	gDialogMode;
Boolean		gOkActive;

void ToolBoxInit()
{
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NIL);
	MaxApplZone();
	InitCursor();
	
	SetDAFont(applFont);
}

void FrameDialogItem(DialogPtr theDialog, short theItem, Pattern thePattern)
{
	GrafPtr			savePort;
	short			iType;
	Handle			iHandle;
	Rect			iRect;
	PenState		savePen;
	RGBColor		color;
	PixPatHandle	pp;

	GetPort(&savePort);
	SetPort(theDialog);
	GetPenState(&savePen);

	if (thePattern == gray) 
	{
		color.red = 0x8000;
		color.green = 0x8000;
		color.blue = 0x8000;
	} else if (thePattern == black) 
	{
		color.red = 0x0;
		color.green = 0x0;
		color.blue = 0x0;
	}	
	pp = NewPixPat();
	MakeRGBPat(pp, &color);
	PenPixPat(pp);
	
	GetDItem(theDialog, theItem, &iType, &iHandle, &iRect);
	InsetRect(&iRect, -4, -4);
	PenSize(3, 3);
//�	PenPat(thePattern);
	FrameRoundRect(&iRect, 16, 16);

	DisposPixPat(pp);
	SetPenState(&savePen);
	SetPort(savePort);
}

void DoDialogUpdate(DialogPtr theDialog)
{
	GrafPtr	savePort;
	
	GetPort(&savePort);
	SetPort(theDialog);
	
	BeginUpdate(theDialog);
	UpdtDialog(theDialog, theDialog->visRgn);
	FrameDialogItem(theDialog, ok, (gOkActive ? black : gray));
	EndUpdate(theDialog);
	SetPort(savePort);
}

void DoDialogActivate(DialogPtr theDialog, Boolean active)
{
	SetPort(theDialog);
}

pascal Boolean DialogFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem)
{
	short			iType;
	Handle			iHandle;
	Rect			iRect;
	char			theChar;
	long			endTicks;
	Boolean			result = FALSE;
	WindowPtr		theWindow;

	Point			mousePoint;
	Rect			upArrowRect, downArrowRect, plainArrowRect;
	Boolean			hiLit = FALSE;
	Str255			theString;
	long			theValue, theDelay;
	PicHandle		upArrowPict, downArrowPict, plainArrowPict;

	theWindow = (WindowPtr)(theEvent->message);
	switch (theEvent->what) 
	{
		case updateEvt:					//� In case we're updating
			if (theWindow == theDialog) //� the dialog...
			{
				DoDialogUpdate(theDialog);	//� update it (duh).
				result = TRUE;				//� True!  We're updating.
				*theItem = 0;				//� But not because of an item.
			} 
			else 
				{
				//� Call main DoUpdate(w) routine here to prevent holes 
				//� in  background windows (see TN 304).
			}
		break;
		
		case activateEvt:				//� In case our dialog is being
			if (theWindow == theDialog) //� activated (brought to front)...
			{
				//� activate it (duh).
				DoDialogActivate(theDialog, (theEvent->modifiers & activeFlag) != 0);
				*theItem = 0;	//� But not because of an item.  More
			} 					//� likely, because it was not active and
			else 				//� was clicked in.
				{
				//� Call main DoActivate(w) routine here to deactivate old
				//� frontmost window to unhighlight scroll bars, etc.
			}
		break;
		
		case mouseDown:
			//� Get handles to all possible arrows.
			plainArrowPict = GetPicture(130); 	//� Both arrows, no hit.
			upArrowPict = GetPicture(131);		//� Both, top arrow hit.
			downArrowPict = GetPicture(132);	//� Both, bottom hit.
			
			//� Where the event (mouseDown) was, is the mousePoint (duh).
			mousePoint = theEvent->where;
			//� Find out where that mouseDown was.
			GlobalToLocal(&mousePoint);		
			
			//� Narrow that down to dialog item #5.
			GetDItem(theDialog, 5, &iType, &iHandle, &iRect);
			
			//� The un-hit arrow rectangle is iRect.
			plainArrowRect = iRect;
			upArrowRect = iRect;		//� upArrowRect is iRect, too.
			upArrowRect.bottom -= 15;	//� The bottom is 15 pixels up.
			downArrowRect = iRect;		//� dawnArrow is iRect, too.
			downArrowRect.top += 15;		//� The top is 15 pixels down.
			
			//� Calculate time til first auto-count.
			endTicks = TickCount() + 15;	//� 15 tick delay.
			
			//� Get dialog item 4 - the TextEdit rectangle.
			GetDItem(theDialog, 4, &iType, &iHandle, &iRect);
			
			//� Lock its handle.
			HLock(iHandle);
			
			//� Get the text, via the handle, of the string.
			GetIText(iHandle, theString);
			
			//� Make it into a numerical value.
			StringToNum(theString, &theValue);	//� Convert to number.
			
			//� If the point in the (iRect) rect of the click was in
			//� the up arrow...
			if (PtInRect(mousePoint, &upArrowRect)) 
			{	
				//� Draw the upArrowPict in the plainArrowRect.
				DrawPicture(upArrowPict, &plainArrowRect);
				
				//� Set initial delay for auto-increment.
				theDelay = 15;		
				hiLit = TRUE;	//� Yes, it's hilited.
				theValue++;		//� Increase the value by...
				
				//� converting the value as a string...
				NumToString(theValue, theString);
				
				//� and displaying that string.
				SetIText(iHandle, theString);
				
				//� Keep an eye on the above operations and continue
				//� while the user still holds the mouse button down.
				while (StillDown()) 
				{	
					//� Keep watching where the mouse point is.
					GetMouse(&mousePoint);
					
					//� If it's in the upArrowRect...
					if (PtInRect(mousePoint, &upArrowRect)) 
					{
						if (hiLit == FALSE) //� and it ain't hilited...
						{
							hiLit = TRUE;	//� hilite it, by...
							
							//� drawing the uparrow pict in plainArrowRect.
							DrawPicture(upArrowPict, &plainArrowRect);
						}
					} 
					else 	//� Otherwise (other than mouse being in
						{	//� upArrowRect while still down)...
							if (hiLit == TRUE) 	//� if it's hilited...
							{
								theDelay = 15;	//� wait 15 ticks and...
								hiLit = FALSE;	//� unhilite it, by...
								
								//� drawing the plainArrowPict there.
								DrawPicture(plainArrowPict, &plainArrowRect);
							}
					}
					//� If enough time has passed, auto-increment value.
					//� If we're hilited and the 15 ticks are up...
					if (hiLit == TRUE && TickCount() >= endTicks) 
					{
						//� Increase the value in the TextEdit field.
						theValue++;		
						NumToString(theValue, theString);	//� Convert to string.
						SetIText(iHandle, theString);	//� Display it.
						SelIText(theDialog, 4, 0, 32767);	//� Select the whole thing.
						endTicks = TickCount() + theDelay;	//� Calc new delay time.
						if (theDelay)
							theDelay--;		//� Shorten delay time until it's zero.
					}
				} //� end "while StillDown()".
			} //� end "click in up arrow".
			
			if (PtInRect(mousePoint, &downArrowRect)) 
			{	//� Was click in down arrow?.
				DrawPicture(downArrowPict, &plainArrowRect);
				theDelay = 15;
				hiLit = TRUE;
				theValue--;		//� Decrement it.
				NumToString(theValue, theString);	//� Convert to string.
				SetIText(iHandle, theString);		//� Display it.
				
				while (StillDown()) 
				{	//� Track control if user still holding button.
					GetMouse(&mousePoint);
					if (PtInRect(mousePoint, &downArrowRect)) 
					{		//� In down arrow?.
						if (hiLit == FALSE) 
						{	//� If so and not highlighted, do it.
							hiLit = TRUE;
							DrawPicture(downArrowPict, &plainArrowRect);
						}
					} 
					else 
						{	//� Out of down arrow now, unhighlight and reset delay.
							if (hiLit == TRUE) 
							{
								theDelay = 15;
								hiLit = FALSE;
								DrawPicture(plainArrowPict, &plainArrowRect);
							}
					}
					//� If enough time has passed, auto-decrement value.
					if (hiLit == TRUE && TickCount() >= endTicks) 
					{
						theValue--;		//� Increment value.
						NumToString(theValue, theString);	//� Convert to string.
						SetIText(iHandle, theString);	//� Display it.
						SelIText(theDialog, 4, 0, 32767);	//� Select the whole thing.
						endTicks = TickCount() + theDelay;	//� Calc new delay time.
						if (theDelay)
							theDelay--;		//� Shorten delay time until it's zero.
					}
				} //� end "while StillDown()".
			} //� end "click in down arrow".
			
			HUnlock(iHandle);
			SelIText(theDialog, 4, 0, 32767);  //� Select the whole thing.

			if (hiLit == TRUE) 
			{	//� Unhighlight arrow if necessary.
				DrawPicture(plainArrowPict, &plainArrowRect);
				*theItem = 5;
				result = TRUE;
			}
			
			ReleaseResource((Handle)plainArrowPict);	//� Release arrow PICTs.
			ReleaseResource((Handle)upArrowPict);
			ReleaseResource((Handle)downArrowPict);
		break;
		
		case mouseUp:
		break;
		
		case keyDown:
		case autoKey:
			theChar = theEvent->message & charCodeMask;
			if (theChar == returnKey || theChar == enterKey) 
			{	//� Return/Enter key?.
				if (gOkActive) 
				{  	//� Might be able to use contrlHilite field instead?.
					GetDItem(theDialog, ok, &iType, &iHandle, &iRect);
					HiliteControl((ControlHandle)iHandle, 1);
					Delay(8, &endTicks);
					HiliteControl((ControlHandle)iHandle, 0);
					*theItem = ok;
					return TRUE;
				} 
				else 
					{
						*theItem = 0;
						return TRUE;
				}
			} 
			else 
				if ((theChar == '.' && theEvent->modifiers & cmdKey) 
						|| theChar == escapeKey) 
				{		//� ESC or Cmd-.?
					GetDItem(theDialog, cancel, &iType, &iHandle, &iRect);
					HiliteControl((ControlHandle)iHandle, 1);
					Delay(8, &endTicks);
					HiliteControl((ControlHandle)iHandle, 0);
					*theItem = cancel;
					return TRUE;
			} 
			else 
			{
#ifdef HAS_ISxxxx	//� The "isxxxxx()" functions are easily implemented with a
					   256-byte lookup table... see "ctype.h"/"ctype.c" in Think C.
					   Then you can trap out illegal keys.  Without them, all keys
					   are valid....
				switch (gDialogMode) 
				{
					case AllowAlpha:
						result = isalpha(theChar);
					break;
					
					case AllowPrint:
						result = isprint(theChar);
					break;
					
					case AllowDigit:
						result = isdigit(theChar);
					break;
					
					case AllowHexDigit:
						result = isxdigit(theChar);
					break;
					
					case AllowAlphaNum:
						result = isalnum(theChar);
					break;
					
					default:
						result = FALSE;
				}
				result |= iscntrl(theChar);		//� Always allow spec. keys.
#else
				result = TRUE;
#endif
				if (result)
					return FALSE;
				else 
					{
						SysBeep(1);
						*theItem = 0;
						return TRUE;
				}
			}
			break;
	}
	return result;
}

main()
{
	GrafPtr			savePort;
	DialogPtr		theDialog;
	short			itemHit, iType;
	Handle			iHandle, rsrcHandle;
	Boolean			dialogDone = FALSE;
	Rect			iRect;
	Str255			theString;
	long			theValue;
	
	ToolBoxInit();
	
	if ((theDialog = GetNewDialog(130, NIL, (WindowPtr)-1L)) == NIL) {
		SysBeep(1);
		ExitToShell();
	}
	
	GetPort(&savePort);	
	SelectWindow(theDialog);
  	SetPort(theDialog);

	ShowWindow(theDialog);
	SelIText(theDialog, 4, 0, 32767);
	
	gOkActive = FALSE;
	gDialogMode = AllowDigit;
	while (!dialogDone) 
	{
		//� Get the dialog ok item, the item type, the handle
		//� to the item and its rectangle.  get this stuff.
		GetDItem(theDialog, ok, &iType, &iHandle, &iRect);
		
		//� Hilite that item (not select, but hilite) if it's active.
		HiliteControl((ControlHandle)iHandle, (gOkActive ? 0 : 255));
		
		//� Frame the Okay button. With gray if not active.
		FrameDialogItem(theDialog, ok, (gOkActive ? black : gray));
		
		//� Keep an eye on the dialog and watch for item hits.
		ModalDialog(DialogFilter, &itemHit);
		switch (itemHit) 
		{
			case ok:				//� If the Okay button is hit...
				dialogDone = TRUE;	//� we are done with the program.
			break;
			
			case cancel:			//� Same here.
				dialogDone = TRUE;
			break;
			
			case 4:		//� Edit text item.
			case 5:		//� Pict item of small up/down arrows.
				
				//� If it's the arrows, get which one - up or down.
				GetDItem(theDialog, 4, &iType, &iHandle, &iRect);
				
				//� In either arrow, we'll get the text, too.
				GetIText(iHandle, theString);
				
				//� Change the text to a numerical value (integer).
				StringToNum(theString, &theValue);
				
				//� If the value is greater than 9...
				if (theValue > 9)		
					gOkActive = TRUE;	//� Activate the Okay button.
				else
					gOkActive = FALSE;	//� Otherwise, leave it dim.
			break;
		}
	}
	DisposDialog(theDialog);
  	SetPort(savePort);
}
