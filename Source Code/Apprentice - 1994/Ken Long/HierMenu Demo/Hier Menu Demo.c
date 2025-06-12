//• Updated to Think C 5.0.4 in the summer of 1993 by Kenneth A. Long
//-------------------------------------------------------------------//

/*
*Program name: Hier Menu Demo
*
*Author:  Ted C. Johnson, Sun, Aug 14 1988.
*
*Compilation instructions:  use Lightspeed C, v.2.15.  This program DOES
*							use a resource file.  I developed this program
*							on a Mac SE HD20 running System/Finder 4.1/5.5.
*
*Summary:  This program demonstrates how to use hierarchical menus.  It 
*		   also demonstrates how to handle DA's, how to make your own
*		   "About…" box (with an icon), and how to use dialog boxes.
*
*		   The use of this source code is free, provided it is for 
*		   non-profit purposes.  I hope it can save someone some time!
*
*		   However the name "T Bear Software" and the T Bear icon are 
*		   both CopyRight © 1988 by Ted C. Johnson, and may NOT be used 
*		   without my express written permission.
*/



#include <QuickDraw.h>
#include <Windows.h>
#include <Events.h>
#include <Menus.h>
#include <Dialogs.h>
#include <Fonts.h>
#include <Desk.h>
#include <Types.h>
#include <TextEdit.h>



#define int32		long
#define	int16		int
#define int8		char

#define NIL			0L



/*Define the ID for our DLOG and DITL resource.*/

#define DIALOG_MESSAGE 777


/*
*Define our menu IDs.  My unusual spacing is meant to give you a
*picture of how the sub-menus relate to their parent menu.
*/

#define APPLE_ID		10

#define	FILE_ID			20

#define FONT_N_DA_ID	30
#define		FONT_ID			31 /*Fonts.*/
#define		DA_ID			32 /*DA's.*/

#define	MACINTOSH_ID	40 /*128K, 512K, 512KE, Plus, SE, II*/
#define		SE_ID			41 /*2 diskette, SE HD20*/

#define NICE_CAR_ID		100	/*German, Italian, Japanese*/
#define		GERMAN_ID		110 /*Porsche, Mercedes, BMW*/
#define			PORSCHE_ID		111 /*911, 914, 924, 928, 930, 935, 944, 959*/
#define			MERCEDES_ID		112 /*300, 450*/
#define			BMW_ID			113 /*318, 320, 325, 528, 533, 633, 733*/
#define		ITALIAN_ID		120 /*Lamborghini, Ferrari, Lotus*/
#define		JAPANESE_ID		130 /*Honda Prelude, Acura Legend, Mazda RX-7*/



/*Define our global variables.*/

MenuHandle		Apple_MH;
MenuHandle		File_MH;
MenuHandle 		FontAndDA_MH;
MenuHandle			Font_MH;
MenuHandle			DA_MH;
MenuHandle		Macintosh_MH;
MenuHandle			SE_MH;
MenuHandle		NiceCar_MH;
MenuHandle			German_MH;
MenuHandle				Porsche_MH;
MenuHandle				Mercedes_MH;
MenuHandle				BMW_MH;
MenuHandle			Italian_MH;
MenuHandle			Japanese_MH;


EventRecord	theEvent;
WindowPtr	theWindow, whichWindow;
int16		windowCode;
int8		theCharacter;



main()
{
	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	FlushEvents(everyEvent, 0);
	
	InitializeOurMenus();
	
	
	do {
	
		EventLoop();
		
	} while (TRUE);
}



EventLoop()
{
	/*
	*Give the DA some time.
	*/
	
	SystemTask();
	
	/*
	*The event loop.
	*
	*GetNextEvent() will return FALSE if the event is an event that the
	*Desk Manager wants to handle itself (i.e., Desk Accessory events).
	*Refer to I.M. I-258.
	*/
		
	if (GetNextEvent(everyEvent, &theEvent) == TRUE) {
		switch(theEvent.what) {
				
			case keyDown:                       /*Was a key pressed?*/
				theCharacter = theEvent.message & charCodeMask;
				if (theEvent.modifiers & cmdKey) { /*Check for a menu*/
					doMenu(MenuKey(theCharacter)); /*equivalent.*/
					HiliteMenu(0);
				}
				break;
			
			case mouseDown:
				windowCode = FindWindow(theEvent.where, &whichWindow);
				switch(windowCode) {
					
					case inMenuBar:
					
						/*
						*There was a mouseDown in the menu bar.
						*/
						 
						doMenu(MenuSelect(theEvent.where));
						break;
							
					case inSysWindow:
					
						/*
						*SystemClick() calls the Desk Manager to handle
						*mouse_down events which occur in a desk accessory
						*window (e.g., like in the Control Panel's window).
						*/
						
						SystemClick(&theEvent, whichWindow);
						break;
				}
						
			default:
				break;
			}/*switch(theEvent.what)*/
		}/*if*/
}



InitializeOurMenus()
{
	Apple_MH = NewMenu(APPLE_ID, "\p\024");/*Desk Accessory menu.*/
	AppendMenu(Apple_MH, "\p^1 Software presents…;(-");
	AddResMenu(Apple_MH, 'DRVR');
	InsertMenu(Apple_MH, 0);

	File_MH = NewMenu(FILE_ID, "\pFile");  /*File menu.*/
	AppendMenu(File_MH, "\pQuit/Q");
	InsertMenu(File_MH, 0);
	
	
	/*
	*Set up our first hierarchical menu:  Fonts and DAs.
	*
	*Note that we can insert menus even before we define their
	*sub-menus!!!  Octal 033 is hex 1B, which is the <ESC>
	*character.  Octal 024 is the Apple symbol. 
	*/
	
	FontAndDA_MH = NewMenu(FONT_N_DA_ID, "\pFonts and DAs");
	AppendMenu(FontAndDA_MH, "\pFonts/\033;\024/\033");
	InsertMenu(FontAndDA_MH, 0);

		Font_MH = NewMenu(FONT_ID, "\pFonts (this title is invisible)");
		AddResMenu(Font_MH, 'FONT');
		InsertMenu(Font_MH, -1);
		SetItemMark(FontAndDA_MH, 1, FONT_ID);/*Insert the Font menu beside
											   *the 1st FontAndDA menu item.
											   */



		DA_MH = NewMenu(DA_ID, "\p\024 (this title is invisible)");/*Desk Accessory menu.*/
		AddResMenu(DA_MH, 'DRVR');
		InsertMenu(DA_MH, -1);
		SetItemMark(FontAndDA_MH, 2, DA_ID);/*Insert the DA menu beside the
											 *the 2nd FontAndDA menu item.
											 */



	/*
	*Set up our second hierarchical menu:  Nice cars.
	*
	*Note that we can insert menus even before we define their
	*sub-menus!!!
	*/
	
	NiceCar_MH = NewMenu(NICE_CAR_ID, "\pNice cars");
	AppendMenu(NiceCar_MH, "\pGerman/\033;Italian/\033;Japanese/\033");
	InsertMenu(NiceCar_MH, 0);

	
		/*Set up and insert the German sub-menu.*/
		
		German_MH = NewMenu(GERMAN_ID, "\pthis title is invisible");
		AppendMenu(German_MH, "\pPorsche/\033;Mercedes/\033;BMW/\033");
		InsertMenu(German_MH, -1);
		SetItemMark(NiceCar_MH, 1, GERMAN_ID);/*Insert the German menu 
											   *beside the 1st NiceCars
											   *menu item.
											   */
			
			
			/*Set up and insert the Porsche sub-sub-menu.*/
			
			Porsche_MH = NewMenu(PORSCHE_ID, "\pthis title is invisible");
			AppendMenu(Porsche_MH, "\p911;914;924;928;930;935;944;959");
			InsertMenu(Porsche_MH, -1);
			SetItemMark(German_MH, 1, PORSCHE_ID);/*Insert the Porsche menu
												   *beside the 1st German
												   *menu item.
												   */
	
	
			/*Set up and insert the Mercedes sub-sub-menu.*/
			
			Mercedes_MH = NewMenu(MERCEDES_ID, "\pthis title is invisible");
			AppendMenu(Mercedes_MH, "\p300;450");
			InsertMenu(Mercedes_MH, -1);			
			SetItemMark(German_MH, 2, MERCEDES_ID);/*Insert the Mercedes
													*menu beside the 2nd
													*German menu item.
													*/
			
			
			/*Set up and insert the BMW sub-sub-menu.*/
			
			BMW_MH = NewMenu(BMW_ID, "\pthis title is invisible");
			AppendMenu(BMW_MH, "\p318;320;325;528;633;733");
			InsertMenu(BMW_MH, -1);			
			SetItemMark(German_MH, 3, BMW_ID);		/*Insert the BMW menu
													 *beside the 3rd German
													 *menu item.
													 */
	

		/*Set up and insert the Italian sub-menu.*/
		
		Italian_MH = NewMenu(ITALIAN_ID, "\pthis title is invisible");
		AppendMenu(Italian_MH, "\pLamborghini;Ferrari;Lotus");
		InsertMenu(Italian_MH, -1);		
		SetItemMark(NiceCar_MH, 2, ITALIAN_ID);/*Insert the Italian menu
												*beside the 2nd NiceCar
												*menu item.
												*/
		
		
		/*Set up and insert the Japanese sub-menu.*/
		
		Japanese_MH = NewMenu(JAPANESE_ID, "\pthis title is invisible");
		AppendMenu(Japanese_MH, "\pHonda Prelude;Acura Legend;Mazda RX-7");
		InsertMenu(Japanese_MH, -1);		
		SetItemMark(NiceCar_MH, 3, JAPANESE_ID);/*Insert the Japanese menu
												 *beside the 3rd NiceCar
												 *menu item.
												 */
		
		
		
	/*
	*Set up our third hierarchical menu:  Macintoshes.
	*
	*Note that we can insert menus even before we define their
	*sub-menus!!!
	*/
	
	Macintosh_MH = NewMenu(MACINTOSH_ID, "\pMacintoshes");
	AppendMenu(Macintosh_MH, "\p128K;512K;512KE;Plus;SE/\033;II");
	InsertMenu(Macintosh_MH, 0);
	
		/*Set up and insert the SE sub-menu.*/
		SE_MH = NewMenu(SE_ID, "\pthis title is invisible");
		AppendMenu(SE_MH, "\pHD 20;2 diskette");
		InsertMenu(SE_MH, -1);	
		SetItemMark(Macintosh_MH, 5, SE_ID);/*Insert the SE menu beside the
											 *5th Macintosh menu item.
											 */


	DrawMenuBar();
}
	


/*
*This is the subroutine which handles all of the menu commands.
*/

doMenu(menuResult)
int32 menuResult;
{
GrafPtr theCurrentPort;
Str255 theDAsName, menuItemText;
int16 menuID, itemNumber;

	menuID = HiWord(menuResult);
	itemNumber = LoWord(menuResult);
	
	switch(menuID) {
	
		case APPLE_ID:
			if (itemNumber == 1) {
				ShowAboutWindow();
			}
			else {
			
				/*
				*Save the current grafPort, in case the DA doesn't
				*restore it to the previous value.
				*/
				
				GetPort(&theCurrentPort);
				
				GetItem(Apple_MH, itemNumber, &theDAsName);
				OpenDeskAcc(&theDAsName);
				
				SetPort(theCurrentPort);
			}
			break;
			
		case FILE_ID:  /*The "File" menu.*/
			switch(itemNumber) {
				case 1:  
					ExitToShell();
					break;
					
				default:
					break;
			}/*switch(itemNumber)*/
			
			break;
		
		/*Handle the sub-menus for the "Font and DAs" hierarchical menu.*/
		case FONT_ID:
			switch(itemNumber) {
				default:
					GetItem(Font_MH,  itemNumber, menuItemText); 
					message("\pYou selected the ", menuItemText, 
					        "\p font.");
					break;
			}
			break;
			
		case DA_ID:
			switch(itemNumber) {
				default:
					GetItem(DA_MH,  itemNumber, menuItemText);
					message("\pYou selected the \"", menuItemText, 
						    "\p\" DA.");
						
					break;
			}
			break;
			
		/*Handle the sub-menus for the "Nice cars" hierarchical menu.*/
		case PORSCHE_ID:
			switch(itemNumber) {
				default:
					GetItem(Porsche_MH,  itemNumber, menuItemText);
					message("\pYou selected the ",
							 menuItemText, 
							 "\p Porsche.");
					break;
			}
			break;
			
		case MERCEDES_ID:
			switch(itemNumber) {
				default: 
					GetItem(Mercedes_MH,  itemNumber, menuItemText);
					message("\pYou selected the ",
							 menuItemText, 
							 "\p Mercedes.");
					break;
			}
			break;
			
		case BMW_ID:
			switch(itemNumber) {
				default:
					GetItem(BMW_MH,  itemNumber, menuItemText);
					message("\pYou selected the ",
							 menuItemText, 
							 "\p BMW.");
					break;
			}
			break;
			
		case ITALIAN_ID:
			switch(itemNumber) {
				default:
					GetItem(Italian_MH, itemNumber, menuItemText);
					message("\pYou selected the ",
							menuItemText, "\p.");
					break;
			}
			break;
			
		case JAPANESE_ID:
			switch(itemNumber) {
				default:
					GetItem(Japanese_MH, itemNumber, menuItemText);
					message("\pYou selected the ",
							menuItemText, "\p.");
					break;
			}
			break;
			
		/*Handle the Macintosh menu, and sub-menu.*/
		case MACINTOSH_ID:
			switch(itemNumber) {
				default:
					GetItem(Macintosh_MH, itemNumber, menuItemText);
					message("\pYou selected the Mac ",
							menuItemText, "\p.");
					break;
			}
			break;
	
		case SE_ID:
			switch(itemNumber) {
				default:
					GetItem(SE_MH, itemNumber, menuItemText);
					message("\pYou selected the Mac SE ",
							menuItemText, "\p.");
					break;
			}
			break;
		
		default:
			break;
		}/*switch(menuID)*/
	
	HiliteMenu(0);/*Don't forget to un-hilite the menu bar after 
					you're done performing that menu item command.*/
}
		
		
		
/*
*ShowAboutWindow() puts up a window when the user selects the "About DA…" 
*command from the apple menu.  To exit this window and return to the 
*program, the user has to click the mouse button.		
*/

ShowAboutWindow()
{
EventRecord	theEvent;
GrafPtr 	DA_AboutBox_Port;
WindowPtr	AboutWindow;
Rect		AboutRect, RectForOneLineOfText;
int8		*line1, *line2, *line3, *line4, *line5;

	line1 = "Hier Menu Demo,";
	line2 = "a tutorial program";
	line3 = "by Ted C. Johnson";
	line4 = "President of T Bear Software™";
	line5 = "Copyright © 1988 by Ted C. Johnson";
	
	GetPort(&DA_AboutBox_Port); /*Preserve the DA_About_Box port.*/
	
	SetRect(&AboutRect, 75, 110, 425, 230);/*Window size and position.*/
	
	SetRect(&RectForOneLineOfText, 5, 10, 345, 25);/*Box for first line of text.*/
	
	AboutWindow = NewWindow((WindowPeek)NIL, &AboutRect, "\p", TRUE,
							altDBoxProc, (WindowPtr)-1, TRUE, 0);
	SetPort(AboutWindow);
	
	/*
	*Choose the system font (Chicago), 12 point size.
	*/
	
	TextSize(12);


	TextFont(systemFont);
	
	/*
	*Draw the text.  Offset the RectForOneLineOfText box 20 pixels down and
	*and 0 pixels the right, for each line of text.  Use different styles 
	*of text for each line, just for the heck of it.  See I.M, page I-171.	   		 
	*/
	
	TextFace(shadow);
	TextBox(line1, strlen(line1), &RectForOneLineOfText, teJustCenter);
	
	OffsetRect(&RectForOneLineOfText, 0, 20);
	TextFace(underline);
	TextBox(line2, strlen(line2), &RectForOneLineOfText, teJustCenter);
	
	OffsetRect(&RectForOneLineOfText, 0, 20);
	TextFace(italic);
	TextBox(line3, strlen(line3), &RectForOneLineOfText, teJustCenter);
		
	OffsetRect(&RectForOneLineOfText, 0, 20);
	TextFace(bold);
	TextBox(line4, strlen(line4), &RectForOneLineOfText, teJustCenter);
		
	OffsetRect(&RectForOneLineOfText, 0, 20);
	TextFace(0);
	TextBox(line5, strlen(line5), &RectForOneLineOfText, teJustCenter);
	
	/*
	*Wait for a mouse-down event.  When you get one, erase the About
	*Window, and return to DA_AboutBox Demo window.  
	*/
	
	do {
	
		GetNextEvent(everyEvent, &theEvent); 
		
	} while (theEvent.what != mouseDown);
	
	/*
	*DisposeWindow() calls CloseWindow(), then releases the memory
	*occupied by the window record.
	*/
	
	DisposeWindow(AboutWindow);
	
	SetPort(DA_AboutBox_Port);
}



/*
*showDialog() puts up a modal dialog box and returns the
*number corresponding to the button pushed.		
*/

int16 showDialog(dialogNumber, maxbutton)
int16 dialogNumber;
int16 maxbutton;
{
DialogPtr dialog;			/* pointer to the dialog */
DialogRecord dialogRecord;	/* actual dialog record */
int16 itemHit = maxbutton + 1;

	/* bring up the dialog box */
	dialog = GetNewDialog(dialogNumber, &dialogRecord, (WindowPtr) -1);
	ShowWindow(dialog);

	while (itemHit > maxbutton)
	{
		ModalDialog((ProcPtr) NIL, &itemHit);
	}
	
	CloseDialog(dialog); 
	
	return(itemHit);
}



message(string1, string2, string3)
int8 *string1, *string2, *string3;		/* expect pascal string */
{
	ParamText(string1, string2, string3, "\p");
	showDialog(DIALOG_MESSAGE, 1);
}



messageWithInt(string, theInt)
int8 *string;		/* expect pascal string */
int16 theInt;
{
Str255 tempString;
	
	NumToString(theInt, &tempString);
	ParamText(string, tempString, "\p", "\p");
	showDialog(DIALOG_MESSAGE, 1);
}


strlen(s)
register int8 *s;
{
int8 *s0 = s;

	while(*s++);
	return(s - s0 -1);
}