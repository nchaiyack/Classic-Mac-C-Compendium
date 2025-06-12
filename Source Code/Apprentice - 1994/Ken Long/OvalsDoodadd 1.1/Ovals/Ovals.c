//¥-----------------------------------------------------------------------¥//
//¥ Ovals.c																  ¥//
//¥ Ploppyright (P) 1993 Kenneth A. Long.  No rights reserved.			  ¥//
//¥-----------------------------------------------------------------------¥//

//¥-----------------------------------------------------------------------¥//
//¥ I got the oval thing from an old public domain .c file by Greg Corson ¥//
//¥ (which I've included, so you can see all the wrong character case).	  ¥//
//¥ The .c file was called "Display.c" and was merely a program to open a ¥//
//¥ text file and display the contents.  The oval part was kinda cool, so ¥//
//¥ put it in here by itself, stripped all the extra trash, added some	  ¥//
//¥ controls from "EarthPlot 3.0" and modified them for this.  I have	  ¥//
//¥ learned a few things in making this thing work.  Maybe you can, too.  ¥//
//¥-----------------------------------------------------------------------¥//
//¥ Be sure to try changes, as in the "Ovalize" routine suggestions.	  ¥//
//¥-----------------------------------------------------------------------¥//

//¥-----------------------------------------------------------------------¥//
//¥ -------------------------------ENJOY!-------------------------------- ¥//
//¥-----------------------------------------------------------------------¥//

#include <controls.h>	//¥ I called these so I could access them with
#include <quickdraw.h>  //¥ CMaster, without a big SFGetFile path search.
#include <windows.h>	//¥ They were "called" in the original, too.
#include <menus.h>
#include <events.h>
#include <packages.h>	//¥ This is the only one not in MacHeaders.
#include <fonts.h>

#include "OvalGlobals.h"//¥ Raiders of the Lost EarthPlot Source.

enum {
	appleID = 1, 
	fileID, 			//¥ Must be "2" huh?
	editID  			//¥ Must be "3" (duh).
};
	
#define	ours(w) ((theWindow != NULL) && (w == theWindow))	//¥ From MiniEdit.

MenuHandle		appleMenu, fileMenu, editMenu;

Rect			dragRect, ovalRect, totalBarRect, vOffBarRect, 
				hOffBarRect, controlsRect;
				
Boolean			doneFlag, temp, Resized;
EventRecord  	myEvent;
short			code, refNum;
WindowRecord 	wRecord;
WindowPtr		theWindow, whichWindow;
GrafPtr			tempPort;
short			theMenu, theItem,
				fileOpen, width, fd1,
				numH_Ovals, numV_Ovals, hOffset, vOffset,
				j, i;

long			count;
char			tempBuf[32];
Rect			windowBounds = { 0, 0, 384, 512 };	//¥ Our window size.
Point			thePoint;

//¥-----------------------------------------------------------------------¥//

//¥-----------------------------------------------------------------------¥//
//¥ HandleMenu (theMenu, theItem)---this subroutine processes commands	  ¥//
//¥ from the menu bar.													  ¥//
//¥ theMenu is the menu ID, theItem is the item number in the menu		  ¥//
//¥-----------------------------------------------------------------------¥//


DoSplash ()
{
	long ticks;
	
	DrawThemStrangs ();		//¥ The window is already up, so we draw in it.
	Delay (240L, &ticks);	//¥ 60 ticks per second = 4 seconds of fame.
	EraseRect (&ovalRect);	//¥ Dump it and move on.
	AddControls ();			//¥ We didn't want them spoiling our bragging.
	DrawControlLabels ();	//¥ Send the names to Chicago!
	
}

//¥-----------------------------------------------------------------------¥//

HandleMenu (theMenu, theItem)
short theMenu, theItem;
{
	char name [256];
	
	switch (theMenu)
	{
		case appleID:				//¥ Mouse down in apple menu.
			if (theItem == 1)		//¥ The "About" item.
			{
				DrawThemStrangs ();
			}
			else
				{
					GetItem (appleMenu, theItem, name);
					refNum = OpenDeskAcc (name);
					SetPort (theWindow);
			}
		break;
		case fileID:	//¥ Mouse down in file menu.
			switch (theItem)
			{
				case 1:		//¥ Open file?  For what? We're not using it.
				case 2:		//¥ Close file. For what? We're not using it.
				break;
			
				case 3:		//¥ Quit.
					doneFlag = true;	//¥ Yep!  When we quit, we're done.
				break;
			}
		break;
			
		case editID:					//¥ Process system edit events.
			SystemEdit (theItem-1);		
		break;
	}
	HiliteMenu (0);
}
//¥-----------------------------------------------------------------------¥//
//¥ SetUpMenus ()	"Hot wires" them in, rather that a resource used.
//¥ This subroutine sets up menu bar and reads in desk accessory menu
//¥-----------------------------------------------------------------------¥//

SetUpMenus ()
{
	InsertMenu (appleMenu = NewMenu (appleID, "\p\024"), 0);
	InsertMenu (fileMenu = NewMenu (fileID, "\pFile"), 0);
	InsertMenu (editMenu = NewMenu (editID, "\pEdit"), 0);
	DrawMenuBar ();
	AppendMenu (appleMenu, "\pAbout Oval DoodaddÉ");
	AddResMenu (appleMenu, 'DRVR');
	AppendMenu (fileMenu, "\pWhy?;What?;Quit/Q");
	AppendMenu (editMenu, "\pUndo/Z; (-;Cut/X;Copy/C;Paste/V;Clear");
}

//¥-----------------------------------------------------------------------¥//

ControlsDataUpdate ()
{
	char s [64];

	MoveTo (72, 350);								//¥ 72 over, 350 down.
	
	//¥ Change the value stored in a short (integer) to a string.
	//¥ That is, turn the value into a "picture" of the value.
	//¥ And get that value from whatever the control is set at.
	NumToString (GetCtlValue (hOvalTotalBar), s);
	
	//¥ Make sure there is nothing in the place we'll draw the value picture.
	EraseRect (&H_OvalTotalDataRect);
	
	//¥ Draw a "picture" of the value gotten from the control.
	DrawString (s);

	//¥ Do it all agoin someplace else.
	MoveTo (182, 350);
	NumToString (GetCtlValue (vOvalTotalBar), s);
	EraseRect (&V_OvalTotalDataRect);
	DrawString (s);
	
	MoveTo (292, 350);
	NumToString (GetCtlValue (verticalBar), s);
	EraseRect (&verticalDataRect);
	DrawString (s);
	
	MoveTo (412, 350);
	NumToString (GetCtlValue (horizontalBar), s);
	EraseRect (&horizontalDataRect);
	DrawString (s);
}

//¥-----------------------------------------------------------------------¥//

InitRectangles ()
{
	SetRect (&ovalRect, 6, 25, 507, 320);	//¥ Shortened for control space.
	
	SetRect (&controlsRect, 0, 320, 512, 384);	//¥ Just for a separate
												//¥ area for the controls.
												//¥ Not really needed/used.

	SetRect (&buttonRect, 	 440, 354, 500, 374);	//¥ Our Draw Button.

	SetRect (&H_OvalTotalBarRect, 	 10, 358, 100, 374);	//¥ Scroll Bars.
	SetRect (&V_OvalTotalBarRect, 	110, 358, 210, 374);
	SetRect (&verticalBarRect,	 	220, 358, 320, 374);
	SetRect (&horizontalBarRect, 	330, 358, 430, 374);


	SetRect (&H_OvalTotalDataRect, 	 72, 330, 102, 350);	//¥ For data.
	SetRect (&V_OvalTotalDataRect, 	182, 330, 212, 350);	//¥ Or "labels."
	SetRect (&verticalDataRect, 	292, 330, 322, 350);
	SetRect (&horizontalDataRect, 	412, 330, 432, 350);
}

//¥-----------------------------------------------------------------------¥//

DrawControlLabels ()
{
	GrafPtr	thePort;
	GetPort (&thePort);

	DrawControls (thePort);			//¥ Freshen up the controls, whether
									//¥ neede or not.
	TextFont (0);					//¥ Make sure we're in Chicago.
	MoveTo (10, 350);				//¥ Go here.
	DrawString ("\pH # Ovals:");	//¥ Draw this profound statement.
	MoveTo (110, 350);				//¥ Move.
	DrawString ("\pV # Ovals:");	//¥ Do.
	MoveTo (220, 350);				//¥ Etcetera.
	DrawString ("\pV Offset:");
	MoveTo (330, 350);
	DrawString ("\pH Offset:");
	
	ControlsDataUpdate ();			//¥ Make the output track, in case it
}									//¥ was messed up.

//¥-----------------------------------------------------------------------¥//

Track (ControlHandle theControl, short partCode)
{

	short		i, step;
	long	wait;
	long	waited;
	char	s [128];

	wait = 5; //¥ ticks.

	if (partCode == 0) 				//¥ If we ain't clickin' then don't
		return;						//¥ hang around here!
	
	switch (partCode) 				//¥ If we be clickin' then get to
	{								//¥ switchin'!
		case inUpButton:
			step = -1;				//¥ Pick it down by 1.
		break;
		
		case inDownButton:
			step = 1;				//¥ Bump it up 1.
		break;
		
		case inPageUp:
			step = -10;				//¥ Subtracts 10 increments.
		break;
		
		case inPageDown:
			step = 10;				//¥ Adds up 10 increments.
		break;
	
		case inButton:
			EraseRect (&ovalRect);	//¥ In case you hit the button, it draws.
			Ovalize ();				//¥ That's handy!
		break;
	}
	//¥ Whichever one of the cases switched to, make it so, Number One, ...
	i = GetCtlValue (theControl) + step;	//¥ by assigning the value of
											//¥ the control's setting to "i"
											//¥ AND the "step" amount +/-.
	//¥ Handle the individual controls.
	if (*theControl == *hOvalTotalBar)
	{
		if (i > 148) 		//¥ Whatever "i" is, it's 0 to 148.
			i = 148;
		if (i < 0) 
			i = 0;

		SetCtlValue (theControl, i);	//¥ Set that control to what it IS.

		//¥ Wibe off the drawing surface.
		EraseRect (&H_OvalTotalDataRect);
		
		MoveTo (72, 350);				//¥ Go here.
		
		//¥ Same as before.
		NumToString (GetCtlValue (hOvalTotalBar), s);
		DrawString (s);
		Delay (wait, &waited);
	}
	//¥ And so on..................................!
	if (*theControl == *vOvalTotalBar) 
	{
		if (i > 251) 
			i = 251;
		if (i <  0) 
			i = 0;

		SetCtlValue (theControl, i);

		EraseRect (&V_OvalTotalDataRect);
		MoveTo (182, 350);
		NumToString (GetCtlValue (vOvalTotalBar), s);
		DrawString (s);
		Delay (wait, &waited);
	}

	if (*theControl == *verticalBar) 
	{
		if (i > 50) 
			i = 50;
		if (i <  0) 
			i = 0;

		SetCtlValue (theControl, i);

		EraseRect (&verticalDataRect);
		MoveTo (292, 350);
		NumToString (GetCtlValue (verticalBar), s);
		DrawString (s);
		Delay (wait, &waited);
	}
	
	if (*theControl == *horizontalBar) 
	{
		if (i > 49) 	//¥ If the counter's more than 49,
			i = 49;		//¥ then the count is 49.
		if (i <  0) 	//¥ Same for less than 49.
			i = 0;		//¥ But the it's zero.

		SetCtlValue (theControl, i);

		EraseRect (&horizontalDataRect);
		MoveTo (412, 350);
		NumToString (GetCtlValue (horizontalBar), s);
		DrawString (s);
		Delay (wait, &waited);
	}
	
	if (*theControl == goButton)		//¥ If we click in the "Draw"
	{									//¥ button, we DRAW!!!!!!!!
		EraseRect (&ovalRect);			//¥ (duh)
		Ovalize ();
	}
}

//¥-----------------------------------------------------------------------¥//
//¥ It's the content of the controls we're interested in.				  ¥//
//¥-----------------------------------------------------------------------¥//

DoContent(WindowPtr theWindow, EventRecord *theEvent)
{	
	ControlHandle	theControl;
	short				partCode;
	char			s [100];
	WindowPtr myWindow;

	partCode = FindControl (theEvent->where, theWindow, &theControl);

	if (partCode) 
	{
		switch (partCode) 
		{		
			case inUpButton:
			case inDownButton:
			case inPageUp:
			case inPageDown:
				partCode = TrackControl (theControl, theEvent->where, 0L);
				Track (theControl, partCode);
				ControlsDataUpdate ();
			break;
			
			case inThumb:
				partCode = TrackControl (theControl, theEvent->where, 0L);
				ControlsDataUpdate ();
			break;
			
			case inButton:		//¥ Not this again!
				partCode = TrackControl (theControl, theEvent->where, 0L);
				EraseRect (&ovalRect);
				Ovalize ();
			break;
		}
	}
}

//¥-----------------------------------------------------------------------¥//

Ovalize ()			//¥ Get yer MoJo workin'!
{
	Rect tempRect;

	SetPort (theWindow);
	HiliteWindow (theWindow, false);
	
		//¥ The # of ovals is the same as the control value the user set.
		numH_Ovals = GetCtlValue (hOvalTotalBar);	//¥ "=" means "is".
		numV_Ovals = GetCtlValue (vOvalTotalBar);	
		
		//¥ The oval offset is the same as the control value the user set.
		vOffset    = GetCtlValue (verticalBar);
		hOffset    = GetCtlValue (horizontalBar);
	
		BlockMove (&ovalRect, &tempRect, (long)sizeof ovalRect);
		for (j = 0; j < numV_Ovals; j++)	//¥ Change horizontal ovals here.
		{
			FrameOval (&tempRect);		//¥ Comment this line out and then
//¥ 		FrameRect (&tempRect);		//¥ uncomment this line for square.
			InsetRect (&tempRect, vOffset, 0);
		}
		BlockMove (&ovalRect, &tempRect, (long)sizeof ovalRect);
		for (j = 0; j < numH_Ovals; j++)
		{
			FrameOval (&tempRect);		//¥ Comment this line out and then
//¥ 		FrameRect (&tempRect);		//¥ uncomment this line for square.
			InsetRect (&tempRect, 0, hOffset);
		}
}

//¥-----------------------------------------------------------------------¥//

DoInitManagers ()					//¥ Hire these guys to manage stuff.
{	
	InitGraf (&thePort);
	InitFonts ();
	FlushEvents (everyEvent, 0);
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (0L);
	InitCursor ();
}

//¥-----------------------------------------------------------------------¥//
//¥ The window is hot-wired in, too.  Why mess with a resource for this 
//¥ little thing?
//¥-----------------------------------------------------------------------¥//

SetUpWindow ()
{
	//¥ Setup the drag Rectangle so part of the window will always be visible.
	dragRect = screenBits.bounds;
	
	//¥ Create the window and set the current port to the window port.
	theWindow = NewWindow (&wRecord, &windowBounds, "\p", true, 2, 
			 (WindowPtr)-1L, false, 0L);
	SetPort (theWindow);
	
	//¥ get the Rectangle for the current window and put it in ovalRect.
	BlockMove (&thePort->portRect, &ovalRect, (long) sizeof ovalRect);
	width = ovalRect.right  - ovalRect.left;
		
	InitRectangles ();		//¥ Now that we have a window, put things in it.

}
//¥-----------------------------------------------------------------------¥//

AddControls ()
{
	//¥ A new button.
	goButton	= NewControl (theWindow, &buttonRect, 
								"\pDraw", true,  0L, 0L, 0L, 0, 0L);	
	
	//¥ And 4 new scroll bars.
	hOvalTotalBar	= NewControl (theWindow, &H_OvalTotalBarRect, 
										"", true,  1, 1, 148, 16, 0L);
											
	horizontalBar	= NewControl (theWindow, &horizontalBarRect,  
										"", true,  1, 1, 49, 16, 0L);	

	vOvalTotalBar	= NewControl (theWindow, &V_OvalTotalBarRect, 
										"", true,  1, 1, 251, 16, 0L);
											
	verticalBar		= NewControl (theWindow, &verticalBarRect, 	 
										"", true,  1, 1, 50, 16, 0L);	
}

//¥-----------------------------------------------------------------------¥//
//¥ Centers the strings in the window.									  ¥//
//¥-----------------------------------------------------------------------¥//

Center (char *str)
{
	//¥ Move what?  The pen to draw with.  Notice we don't use a pencil?
	//¥ "right" and "left" and "width" imply horizontal.
	//¥ Width is assigned "ovalRect.right" (507) minus "ovalRect.left" (6).
	//¥ That makes "width" 501.
	//¥ 501 minus the width of "str" leaves us with an amount.
	//¥ Then the "0" means we are at the top - or vertical 0.
	//¥ So we are at the top and 501 minus the StringWidth, divided by 2
	//¥ from the left.  If the StringWidth in 101, we're at 200.  Dig?
	Move (((width - StringWidth (str)) / 2), 0);
	
	DrawString (str);			//¥ We draw the string there.
	
	//¥ Am I supposed to explain this one, too?
	Move (-(theWindow->pnLoc.h), (theWindow->txSize) + 2);
}

//¥-----------------------------------------------------------------------¥//

DrawThemStrangs ()
{
	EraseRect (&ovalRect);
	TextFont (courier);		//¥ Could use a procID number for the font.
	TextFace (outline);		//¥ Without this fragment, it would be plain.
	TextSize (24);			//¥ Big ol' typewriter font!
	MoveTo (ovalRect.left, ovalRect.top + 20);	//¥ Start at down 20.
	Center ("\pOval Drawing Doodadd");	//¥ Did I spell "doodadd right?
										//¥ Always take a "\p" first!
	Move (0, -10);
	TextFont (monaco);
	TextSize (9);
	TextFace (bold);
	Center ("\pPublic Domain (P) 1993 by Kenneth A. Long");
	
	Move (0, 10);
	TextFont (courier);	
	TextFace (outline);	
	TextSize (14);					//¥ 5 points bigger in size.
	Center ("\pitty bitty bytesª");
	
	TextFont (monaco);				//¥ Change of font duly noted.
	TextFace (bold);				//¥ The "brazen" style!
	TextSize (9);					//¥ The incredible shrinking font.
	Center ("\pkenlong@netcom.com");	//¥ I know this guy!
	
	TextFace (0);		//¥ Zero = systemFont = Chicago (on my LC).
	Move (0, 20);		//¥ Drop that pen down 20 points.
	
	Center ("\pGleaned from a file display program from 1985 by Greg Corson.");
	Center ("\pYou should NOT try sell this for profit.  Nobody would buy it.");

	TextFont (courier);		//¥ Font stays the same until you say different.
	TextSize (10);			//¥ Courier looks bad in 9.
	Move (0, 10);			//¥ Drop down 10 points.
	Center ("\p\"I Think, therefore I 'C'.\"  Ain't Think Cª v.5.0.4 great!");

	Move (0, 10);			//¥ Drop down 10 points.
	TextFont (monaco);		//¥ Back to Monaco (stay out of the casinos!).
	TextSize (9);			//¥ Shrink back to 9 for this font.
	Center ("\pThis program was made to help beginning programmers learn things.");
	Center ("\pFeel free to give this program away to all your friends.");
	Center ("\pPost it on your local board and otherwise distribute it freely");
	Center ("\pThe oval drawing was the least common program feature of Greg's hack.");

	Move (0, 20);			//¥ Drop the pen down 20 for some "gapposis."
	Center ("\pDraws various texts, centered horizontally in a window.");
	Center ("\pDraws concentric ovals incrementing and decrementing in H and V.");
	Center ("\pTeaches about for loops, controls, drawing two things in one window, ");
	Center ("\pone type of \"splash\" screen, putting quotes in coded-in text, and more.");

	TextFont (systemFont);		//¥ Put it back like we found it.
	TextSize (12);				//¥ Size reverts, too.
	//¥ Note:  Not putting this font back changed my System parameter font!
}

//¥-----------------------------------------------------------------------¥//

MainLoop ()
{
	EventRecord *theEvent;
	
	doneFlag = false;

	do 
	{
	SystemTask ();
	temp = GetNextEvent (everyEvent, &myEvent);
	switch (myEvent.what)
	{
		case mouseDown:  //¥ mouse down, call findwindow to figure out where.
			code = FindWindow (myEvent.where, &whichWindow);
			switch (code)
			{
				case inMenuBar:	//¥ in menu bar, execute the menu command. 
					HandleMenu (MenuSelect (myEvent.where));
				break;
		
				case inSysWindow:	//¥ in desk accessory, call desk manager.
					SystemClick (&myEvent, whichWindow); 
				break;
			
				case inDrag:	//¥ in drag, call dragWindow to move it.
					DragWindow (whichWindow, myEvent.where, &dragRect);
				break;
				
				case inContent:	//¥ in content area, make application window the frontmost.
					if (whichWindow != FrontWindow ())
						SelectWindow (whichWindow);
				else 
					if (ours (whichWindow))
						DoContent(whichWindow, &myEvent);
				break;
			}
		break;
		
		case keyDown:	//¥ If keydown event, check for menu command key.
			if (myEvent.modifiers & cmdKey)
				HandleMenu (MenuKey ((char) (myEvent.message & 0377)));
		break;
		
		case autoKey:
		break;
		
		case activateEvt:	//¥ Application window becomming active, do nothing.
			if ((myEvent.modifiers & 1)&& (((WindowPtr)myEvent.message) == theWindow))
			{
				DisableItem (editMenu, 0);
				EnableItem (fileMenu, 0);
				DrawMenuBar ();
			}
			else
				{
					EnableItem (editMenu, 0);
					DisableItem (fileMenu, 0);
					DrawMenuBar ();
			}
		break;
		
		case updateEvt:	//¥ Update event, update the window frame.
			if (((WindowPtr)myEvent.message) == theWindow)
			{
				BeginUpdate (theWindow);
				DoContent (whichWindow, &myEvent);
				ControlsDataUpdate ();
				EndUpdate (theWindow);
			}
		break;
		}
	} while (doneFlag == 0);
}

//¥-----------------------------------------------------------------------¥//

main ()
{
	DoInitManagers ();	//¥ Hire the managers for our job, here.

	SetUpMenus ();		//¥ Without steak and eggs.
	
	SetUpWindow ();		//¥ So we have a place to do our thing.
	
	DoSplash ();		//¥ Brag for 4 seconds.
	
	MainLoop ();		//¥ Here we go loop-de-loop!
}
