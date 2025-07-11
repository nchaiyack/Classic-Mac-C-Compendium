/****************************************************************

	Test program for Mercutio MDEF 1.2
	
	by Ramon M. Felciano, Digital Alchemy
	� 1992-1994, All Rights Reserved
	
	
*****************************************************************/

#include "Mercutio API.h"

MenuHandle	appleMenu, fileMenu, editMenu, modifiersMenu,
			iconsMenu, nonPrintingMenu, callbackMenu, colorMenu;
MenuHandle	ourPopupMenu;

enum	{
	appleMenuID = 300,
	fileMenuID,
	editMenuID,
	modifiersMenuID,
	iconsMenuID,
	nonPrintingMenuID,
	callbackMenuID,
	colorMenuID
	};

enum	{
	ourPopupMenuID = 1
	};

enum	{
	popupDialogItem = 1,
	separatorItem,
	quitItem
	};
	
	
enum	{
	addFolderIconID = 262,
	addFoldersIconID,
	addFileIconID,
	addFilesIconID
	};

pascal	void MyGetItemInfo (short menuID, short previousModifiers, RichItemData *itemData);

/****
 * InitMacintosh()
 *
 * Initialize all the managers & memory
 *
 ****/

void InitMacintosh(void);
void InitMacintosh(void)

{
	MaxApplZone();
	
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();

}
/* end InitMacintosh */


/****
 * SetUpMenus()
 *
 *	Set up the menus. Normally, we�d use a resource file, but
 *	for this example we�ll supply �hardwired� strings.
 *
 ****/

void SetUpMenus(void);
void SetUpMenus(void)

{
	MenuPrefsRec	prefs;
	
	InsertMenu(appleMenu = GetMenu(appleMenuID), 0);
	InsertMenu(fileMenu = GetMenu(fileMenuID), 0);
	InsertMenu(editMenu = GetMenu(editMenuID), 0);
	InsertMenu(modifiersMenu = GetMenu(modifiersMenuID), 0);
	InsertMenu(iconsMenu = GetMenu(iconsMenuID), 0);
	InsertMenu(nonPrintingMenu = GetMenu(nonPrintingMenuID), 0);
	InsertMenu(callbackMenu = GetMenu(callbackMenuID), 0);
	InsertMenu(colorMenu = GetMenu(colorMenuID), 0);
	
	InsertMenu(ourPopupMenu = GetMenu(ourPopupMenuID), -1);
	/*
	**	In addition to the condense & extend bits, lets interpret shadow as the controlKey,
	**	and outline as the callback flag.
	*/

	/*
	
	*** Setup the preferences for our menus *** 
	
	This is where we determine which style bits are mapped to 
	MDEF features. Most of the menus use Mercutio's default 
	settings. These menus are the exceptions. 
	
	Feel free to play with these settings and see how
	the menus are affected.
	
	Note that we could have stored all this information in an
	'Xmnu' resource with the same ID as the menu, and avoided 
	the hassle of setting these preferences programmatically. 
	We do it this way to demonstrate the various features of
	the MDEF.

	*/
	
	/*
	
	Set up the Color menu.
	
	The Color menu uses an 'Xmnu' resource to restore the Condense and Extend bits to their 
	regular functions (as style bits), and sets the DEFAULT modifiers to Option-Command.
	This means that key equivalents in this Menu need the Command and Option keys
	held down, but all the style bits
	 are still free to be used as such. 
	*/
	
	prefs.optionKeyFlag = 0;
	prefs.shiftKeyFlag = 0;
	prefs.controlKeyFlag = 0;
	prefs.cmdKeyFlag = 0;
	prefs.isDynamicFlag = 0;
	prefs.useCallbackFlag = 0;
	prefs.forceNewGroupFlag = outline;
	prefs.requiredModifiers = cmdKey + optionKey;

	MDEF_SetMenuPrefs(colorMenu, &prefs);
	
	
	

	/*
	
	Set up the Modifiers menu.
	
	the Modifiers menu demonstrates all four modifier keys in action. Thus, we need
	to use four style bits, which we select and store below.
	  
	*/
	prefs.optionKeyFlag = underline;
	prefs.shiftKeyFlag = extend;
	prefs.cmdKeyFlag = bold;
	prefs.controlKeyFlag = shadow;
	prefs.isDynamicFlag = 0;
	prefs.forceNewGroupFlag = 0;
	prefs.useCallbackFlag = outline;
	prefs.requiredModifiers = 0;

	MDEF_SetMenuPrefs(modifiersMenu, &prefs);
	



	/*
	
	Set up the Callback menu.
	
	Note that we can make this call regardless of what MDEF we are using, 
	because if an MDEF doesn't recognize a message (in our case, the 
	SetCallback message), it simply ignores it.
	
	*/

	prefs.optionKeyFlag = condense;
	prefs.shiftKeyFlag = extend;
	prefs.cmdKeyFlag = bold;
	prefs.controlKeyFlag = 0;
	prefs.isDynamicFlag = outline;
	prefs.forceNewGroupFlag = italic;
	prefs.useCallbackFlag = underline;
	prefs.requiredModifiers = cmdKey;
	
	MDEF_SetCallbackProc(callbackMenu, (ProcPtr) &MyGetItemInfo);
	MDEF_SetMenuPrefs(callbackMenu, &prefs);


	DrawMenuBar();

}
/* end SetUpMenus */

//#define	gestaltSystemVersion	'sysv'
//#define svAllSmallData	0x0000FF00
//#define	svAllLargeData	0x000000FF

// appends one Pascal string to another
void AppendPStr(StringPtr destStr, StringPtr sourceStr);
void AppendPStr(StringPtr destStr, StringPtr sourceStr)
{
	short	len = sourceStr[0];
	short	curLen = destStr[0];

	BlockMove(&sourceStr[1],&destStr[curLen+1],len);
	destStr[0] = curLen+len;
}

// copies one Pascal string onto another
void CopyPStr(StringPtr destStr, StringPtr sourceStr);
void CopyPStr(StringPtr destStr, StringPtr sourceStr)
{
	short	len = sourceStr[0];

	BlockMove(&sourceStr[1],&destStr[1],len);
}


pascal	void MyGetItemInfo (short menuID, short previousModifiers, RichItemData *itemData)
	/*
	This routine is used by the Callback menu to demonstrate the
	Mercutio callback mechanism. This routine is called for every
	item in the menu flagged as a "callback item" (in our case,
	with the Outline style bit).
	
	In this example, we check the Shift and Option keys to
	determine what the text and icon of the menu item should
	be.
	
	Note the "Dirty" parameter; if we don't change anything
	in the menuItem, this parameter should be false to
	avoid unnecessary redrawing (and flicker).
	*/
{

 	OSErr	theErr = noErr;
	short	modifiers = 0;
	long	currentTicks;
	Str255	tickStr;
	EventRecord	theEvent;
	Boolean	temp;
	
	temp = EventAvail(everyEvent, &theEvent);
	modifiers = theEvent.modifiers;


	if (itemData->itemID == 37) {
		switch (itemData->cbMsg)
		  {
		  case cbBasicDataOnlyMsg:		  	
			if (modifiers & shiftKey) {
				itemData->flags = (itemData->flags | kShiftKey);
				if (modifiers, optionKey) {
					itemData->flags = itemData->flags | kOptionKey;
					CopyPStr(itemData->itemStr, "\pAdd Folders�");
				} else {
					CopyPStr(itemData->itemStr, "\pAdd Folder�");
				}
			} else {
				if (modifiers & optionKey) {
					itemData->flags = itemData->flags | kOptionKey;
					CopyPStr(itemData->itemStr, "\pAdd Files�");
				} else {
					CopyPStr(itemData->itemStr, "\pAdd File�");
				}
			
			}
		  	itemData->flags = itemData->flags | (kChangedByCallback & (modifiers != previousModifiers));
		  	itemData->flags = itemData->flags | kHasIcon;
		  	break;

		  case	cbIconOnlyMsg:
			if (modifiers & shiftKey) {
				if (modifiers, optionKey) {
					itemData->hIcon = (Handle) GetCIcon(addFoldersIconID);
				} else {
					itemData->hIcon = (Handle) GetCIcon(addFolderIconID);
				}
			} else {
				if (modifiers, optionKey) {
					itemData->hIcon = (Handle) GetCIcon(addFilesIconID);
				} else {
					itemData->hIcon = (Handle) GetCIcon(addFileIconID);
				}
			
			}

		  	itemData->iconType = 'cicn';
		  	itemData->flags = itemData->flags | kHasIcon;
		  	
			//	The item has changed if the user is holding down a new set of modifiers
		  	itemData->flags = itemData->flags | (kChangedByCallback & (modifiers != previousModifiers));
		  	
			break;
		
		  case	cbGetLongestItemMsg:
			CopyPStr(itemData->itemStr, "\pAdd Folders�");
		  	itemData->flags = itemData->flags | kShiftKey;
		  	itemData->flags = itemData->flags | kOptionKey;
			break;
		
		  }
	}
	if (itemData->itemID == 38) {
		switch (itemData->cbMsg)
		  {
		  case cbBasicDataOnlyMsg:
		  	currentTicks = TickCount();
		  	NumToString(currentTicks, tickStr);
		  	CopyPStr(itemData->itemStr, "\pTicks: ");
			AppendPStr(itemData->itemStr, tickStr);
		  	itemData->flags = itemData->flags | kChangedByCallback;
		  	break;

		  case	cbIconOnlyMsg:
			break;
		
		  case	cbGetLongestItemMsg:
		  	CopyPStr(itemData->itemStr, "\pTicks: 9999999999");
			break;
		  }
	}


	
}




// int enable (MenuHandle menu, short item, short ok);


/*****
 * ToggleItem()
 *
 *	Turn the checkmark of a given item on or off.
 *
 *****/


void ToggleItem (MenuHandle menu, short item);
void ToggleItem (MenuHandle menu, short item)
{
	short	curMark;
	
	GetItemMark(menu, item, &curMark);
	if (curMark == noMark)
		CheckItem(menu, item, TRUE);
	else
		CheckItem(menu, item, FALSE);
}


/*****
 * HandleMenu(mSelect)
 *
 *	Handle the menu selection. mSelect is what MenuSelect() and
 *	MenuKey() return: the high word is the menu ID, the low word
 *	is the menu item
 *
 *****/

void HandleMenu (long mSelect);
void HandleMenu (long mSelect)

{
	int			menuID = HiWord(mSelect);
	int			menuItem = LoWord(mSelect);
	short		dummy;
	Str255		name;
	GrafPtr		savePort;
	DialogPtr	AboutDLOG;
	DialogRecord AboutRecord;
	
	switch (menuID)
	  {
	  case	appleMenuID:
		switch (menuItem)
		  {
			case 1 :
				AboutDLOG = GetNewDialog(3000, &AboutRecord, (WindowPtr) (-1));
				ModalDialog(0L, &dummy);
				CloseDialog(AboutDLOG);
				break;
			
			default :
				GetPort(&savePort);
				GetItem(appleMenu, menuItem, name);
				OpenDeskAcc(name);
				SetPort(savePort);
				break;
			}
		break;
		
	  case	fileMenuID:
		switch (menuItem)
		  {
		  case popupDialogItem:
		  //	doPopupDialog(ourPopupMenu);
		  	break;

		  case	quitItem:
			ExitToShell();
			break;
		
		  }
		break;
  				
	  case	editMenuID:
		if (!SystemEdit(menuItem-1))
		  SysBeep(5);
		break;
		
	  case	modifiersMenuID:
		ToggleItem (modifiersMenu, menuItem);
		break;
		
	  case	iconsMenuID:
		ToggleItem (iconsMenu, menuItem);
		break;
		
	  case	nonPrintingMenuID:
		ToggleItem (nonPrintingMenu, menuItem);
		break;
		
	  case	callbackMenuID:
		ToggleItem (callbackMenu, menuItem);
		break;
		
	  case	colorMenuID:
		ToggleItem (colorMenu, menuItem);
		break;
	  }
}
/* end HandleMenu */





/****
 * HandleEvent()
 *
 *		The main event dispatcher. This routine should be called
 *		repeatedly (it  handles only one event).
 *
 *****/

void HandleEvent(void);
void HandleEvent(void)

{
	int	windowCode;
	EventRecord	theEvent;
	WindowPtr	theWindow;

	HiliteMenu(0);
	SystemTask ();		/* Handle desk accessories */
	
	if (GetNextEvent (everyEvent, &theEvent))
	  switch (theEvent.what)
	    {
		case mouseDown:
			windowCode = FindWindow(theEvent.where, &theWindow);
	
		    switch (windowCode)
    		  {
			  case inSysWindow: 
			    SystemClick (&theEvent, theWindow);
	    		break;
	    
			  case inMenuBar:
			  	HandleMenu(MenuSelect(theEvent.where));
		    	break;
	    
	      	}
	      	break;

			
		case keyDown: 
		case autoKey:
			/*
				This codes is commented out in order to get the Modifiers menu
				to work (it doesn't require the commandKey to be held down.
				
				You may want to use a check like this in your code to speed
				up processing.
			*/
			// if ((theEvent.modifiers & cmdKey) != 0)
			//   {
			  HandleMenu(MDEF_MenuKey(theEvent.message,theEvent.modifiers,modifiersMenu));
			//  }
			break;
				
	    }
}
/* end HandleEvent */





/*****
 * main()
 *
 *	This is where everything happens
 *
 *****/

main()

{

	StringHandle	theCopyright;
	long	theVersion;
	
	InitMacintosh();
	SetUpMenus();
	
	theCopyright = MDEF_GetCopyright(modifiersMenu);
	theVersion = MDEF_GetVersion(modifiersMenu);
	DisposeHandle ((Handle)theCopyright);
	
	for (;;)
		HandleEvent();

	// we don't need to dispose of the menuHandles since we're quitting the application
		
}
