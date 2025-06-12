//¥ This sample program allows a user to open a file and print it's 
//¥ contents to a window on the Mac screen.  
//¥ To give the compiled program an Icon copy the resources from 
//¥ Display.rsrc (use RMover or Resource Editor) and paste them into the 
//¥ compiled file.  Then use SetFile to change the creator
//¥ name of the compiled file to DFIL and set the bundle bit.  When you 
//¥ return to the finder the program icon should have an icon.
//¥ By Greg Corson
//¥ 19141 Summers Drive
//¥ South Bend, IN 46637
//¥ (219) 272-2136
//¥ UUCP: {ihnp4 | ucbvax}!pur-ee!kangaro!milo
//¥ ARPA: pur-ee!kangaro!milo@Purdue.ARPA
//¥ EDU: kangaro!milo@ee.Purdue.EDU
//¥ Or call my BBS at (219) 277-5825


#include <quickdraw.h>  
#include <windows.h>
#include <menus.h>
#include <events.h>
#include <packages.h>
#include <fonts.h>
#include <stdio.h>
#include <string.h>

enum {
	appleID = 1,
	fileID,
	editID,
	stopID
};
	

#define eoferr (-39)

RgnHandle		updateRgn;
MenuHandle		appleMenu, fileMenu, editMenu, stopMenu;
Rect			dragRect, pRect;
Boolean			doneFlag, temp;
EventRecord  	myEvent;
short			code, refNum;
WindowRecord 	wRecord;
WindowPtr		myWindow, whichWindow;
GrafPtr			tempPort;
short			theMenu, theItem;
//short			fileOpen,wide,fd1;
short			fileOpen,wide;
long			count;
char			tempBuf[32];
FILE			*fd1;
//¥-----------------------------------------------------------------------¥//

//¥ ---------------------------------------------------------------------------.
//¥ DoCommand (theMenu, theItem)---this subroutine processes commands from the
//¥ menu bar.  theMenu is the menu ID, theItem is the item number in the menu
//¥-----------------------------------------------------------------------¥//

int DoCommand(long mResult)			//¥ Added by KAL.
//DoCommand (int theMenu, int theItem)	
{
	char name[256];
	Point  openp;
	SFReply rep;
	SFTypeList typeList;
	short i;
	theMenu = HiWord(mResult);		//¥ Added by KAL.
	theItem = LoWord(mResult);		//¥ Added by KAL.

	//¥ Switch to decide what menu the cursor is in.
	
	switch (theMenu)
	{
		case appleID:		//¥ Mouse down in apple menu.
							//¥ Item one is the "about Display a file" box.
			if (theItem == 1)
			{
				TextFont (systemFont);
				TextSize (12);
				EraseRect (&pRect);
				MoveTo (pRect.left, pRect.top + 70);
				Center ("\pDisplay a file program");
				Center ("\pCopyright 1985 by Greg Corson");
				Center ("\pKangaroo Koncepts, Inc.");
				Center ("\p19141 Summers Drive");
				Center ("\pSouth Bend, IN 46637");
				Center ("\p (219) 277-5306");
				TextFont (monaco);
            	TextSize (9); 
				Move (0,-3);
				Center ("\pFeel free to give this program away to all your friends.");
				Center ("\pIt should NOT be sold for profit.  Be sure to try our");
				Center ("\pComputer Based Communications System \"The Connection\"");
				Center ("\pFree demo line (219) 277-5825 available 24 hours at 300 or");
				Center ("\p1200 baud.  Be sure to look at the \"MacTech\" special");
				Center ("\pinterest group for information of interest to Mac");
				Center ("\pprogrammers and the \"macintosh\" SIG for general info.");
				Pretty ();
				MoveTo (pRect.left + 1, pRect.bottom - 2);
			}
         	else	//¥ The rest of the items are desk accessorys.
				{
					GetItem (appleMenu, theItem, name);
            		refNum = OpenDeskAcc (name);
					SetPort (myWindow);
			}
		break;
		
		case fileID:					//¥ Mouse down in file menu.
			switch (theItem)
			{				
				case 1:					//¥ Open file.
					openp.v = 100;
					openp.h = 60;
					strncpy ((char*) typeList,"TEXT",4);
					
					SFGetFile (openp, "\p", 0L, 1, typeList, 0L, &rep);
					if (rep.good)
					{
//						if (fileOpen)
//						{
//							fclose (fd1);
//							fileOpen = false;
//						}
						if (FSOpen(rep.fName, rep.vRefNum, &fd1) == noErr)
						{
							ScrollRect (&pRect, 0, -11, updateRgn);
							fileOpen = true;
						}
					}
				break;
		
				case 2:		//¥ Close file.
					if (fileOpen)
					{
						FSClose((short)fd1);
//						fclose (fd1);
//						fileOpen = false;
					}
				break;
			
				case 4:		//¥ Quit.
					if (fileOpen)
					{
						FSClose((short)fd1);
//						fclose (fd1);
//						fileOpen = false;
					}
					doneFlag = 1;
				break;
			}
			break;
			
		case editID:	//¥ Process system edit events.
			SystemEdit (theItem - 1);
		break;
	}
	HiliteMenu (0);
}
//¥-----------------------------------------------------------------------¥//
//¥ SetUpMenus ()
//¥ This subroutine sets up menu bar and reads in desk accessory menu
//¥-----------------------------------------------------------------------¥//

SetUpMenus ()
{
	short i;

	InsertMenu (appleMenu = NewMenu (appleID, "\p\024"), 0);
	InsertMenu (fileMenu = NewMenu (fileID, "\pFile"), 0);
	InsertMenu (editMenu = NewMenu (editID, "\pEdit"), 0);
	InsertMenu (stopMenu = NewMenu (stopID, "\pPause"), 0);	
	DrawMenuBar ();
	AppendMenu (appleMenu, "\pAbout \"Display a File\";(-");
	AddResMenu (appleMenu, 'DRVR');
	AppendMenu (fileMenu, "\pOpen/O;Close/C;(-;Quit/Q");
	AppendMenu (editMenu, "\pUndo/Z;(-;Cut/X;Copy/C;Paste/V;Clear; (-;Show Clipboard");
	AppendMenu (stopMenu, "\pClick here to pause printout;Release mouse button to resume printout");
}

//¥-----------------------------------------------------------------------¥//
//¥ Center a string in the window.
//¥-----------------------------------------------------------------------¥//
Center (char *str)
{
   Move (((wide-StringWidth (str)) / 2), 0);
   DrawString (str);
   Move (- (myWindow->pnLoc.h), (myWindow->txSize) + 2);
}

//¥ draw a "pretty" design.
Pretty ()
{
	short j;
	Rect tmprec;
	
	BlockMove (&pRect, &tmprec, (long) sizeof pRect);
	for (j = 0; j < 12; j++)
	{
		FrameOval (&tmprec);
		InsetRect (&tmprec,6,0);
	}
	BlockMove (&pRect, &tmprec, (long) sizeof pRect);
	for (j = 0; j < 9;j++)
	{
		FrameOval (&tmprec);
		InsetRect (&tmprec,0,6);
	}
}
	
main ()
{
	short i,j;
	Rect screenRect;
	
//¥ Initialize variables.

	j = 0;
	doneFlag = false;
	fileOpen = false;
	
//¥ Initialize quickdraw, fonts, events, windows, menus, dialogs and cursor.
	
	InitGraf (&thePort);
	InitFonts ();
	FlushEvents (everyEvent, 0);
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (0L);
	InitCursor ();

//¥ Create an empty region pointer for use by ScrollRect later.

	updateRgn = NewRgn ();

//¥ Setup the menu bar.

	SetUpMenus ();
	
//¥ Setup the drag Rectangle so part of the window will always be visible.

	SetRect (&screenRect, 4, 40, 508, 338);
	dragRect = screenBits.bounds;
	
//¥ Create the window and set the current port to the window port.
	
	myWindow = NewWindow (&wRecord, &screenRect, "\pDisplay a file", true, 0,
			 (WindowPtr)-1L, false, 0L);
	SetPort (myWindow);

//¥ get the Rectangle for the current window and put it in pRect.

	BlockMove (&thePort->portRect, &pRect, (long) sizeof pRect);
	wide = pRect.right - pRect.left;
	
//¥ Now that the window and menus are drawn set the window font to monaco 9.
	
	TextFont (monaco);
	TextSize (9);
	MoveTo (pRect.left + 1, pRect.bottom - 2);

//¥ Main loop to process events.
	
	do 
	{

//¥	If a file is open copy a line to the output window.

	if (fileOpen)
	{
		count = 32;
		fread (tempBuf, sizeof (&count), 4, fd1);
		if (count == 0)
		{
			fclose (fd1);
			fileOpen=false;
			MoveTo (10, 20);
			ScrollRect (&pRect,0,-11,updateRgn);
			DrawString ("\p-------End of File-------");
			ScrollRect (&pRect,0,-11,updateRgn);
			MoveTo (10, 40);
		}
		else
			{
				for (i = 0; i < count; i++)
				{
					if (tempBuf[i] > 31)
						DrawChar (tempBuf[i]);
					else
						{
					//¥ Scroll window if we get a carriage return.
							if (tempBuf[i] == '\r')
							{
								j = 0;
								ScrollRect (&pRect,0,-11,updateRgn);
								MoveTo (10, 60);
							}
					//¥ Expand tabs by outputting spaces.
							if (tempBuf[i] == '\011')
							{
								DrawChar (' ');
								j++;
								for (;j & 07;j++)
									DrawChar (' ');
							}
					}
				}
		}
	}

//¥ *//¥ Get the next event.

	SystemTask ();
	temp = GetNextEvent (everyEvent, &myEvent);
	switch (myEvent.what)
	{
		case mouseDown:  //¥ mouse down, call findwindow to figure out where.
			code = FindWindow (myEvent.where, &whichWindow);
			switch (code)
			{
				case inMenuBar:	//¥ in menu bar, execute the menu command. 
					DoCommand(MenuSelect(myEvent.where));
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
				break;
			}
		break;
		
		case keyDown:	//¥ If keydown event, check for menu command key.
			if (myEvent.modifiers & cmdKey)
				DoCommand (MenuKey ((char) (myEvent.message & 0377)));
		break;
		
		case autoKey:
		break;
		
		case activateEvt:	//¥ Application window becomming active, do nothing.
			if ((myEvent.modifiers & 1)&& (((WindowPtr)myEvent.message) == myWindow))
			{
				DisableItem (editMenu,0);
				EnableItem (fileMenu,0);
				DrawMenuBar ();
			}
			else
				{
					EnableItem (editMenu,0);
					DisableItem (fileMenu,0);
					DrawMenuBar ();
			}
		break;
		
		case updateEvt:	//¥ Update event, update the window frame.
			if (((WindowPtr)myEvent.message) == myWindow)
			{
				BeginUpdate (myWindow);
				EndUpdate (myWindow);
			}
		break;
		}
	} while (doneFlag == 0);
}
