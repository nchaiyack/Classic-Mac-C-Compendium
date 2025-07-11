/* MenuController.c */

#include "MenuController.h"
#include "Memory.h"
#include "CSack.h"
#include "CWindow.h"


#define FontMenuNameID (133L*65536L + 1)

typedef struct
	{
		short				MenuID;
		MenuHandle	TheMenu;
	} MenuListElement;

typedef struct
	{
		long		MenuManagerID;
		short		ItemID;
	} TransTableElement;


static CSack*			TransTable;
static CSack*			ListOfMenus;
static short			AppleMenuID = 0;
static MenuHandle	AppleMenuHandle = NIL;
static short			FontMenuID;
static MyBoolean	FontMenuExists = False;
static short			LastSelectedFontID;

void		AddMenuToList(MenuHandle Menu, short ResID);
short		FindUnusedMenuID(short IDStart);
void		AddItemToList(long MenuManagerID, short ItemID);
short		FindUnusedItemID(void);
short		MMIDtoItemID(long MMID);
long		ItemIDtoMMID(short ItemID);
MenuHandle	MenuHandleFromID(short MenuID);
void		DisableAll(void);
void		RemoveItemFromList(long MenuManagerID, short ItemID);


/* create the internally used data structures */
void		InitMyMenus(void)
	{
		short				ResIndex;
		MenuHandle	Menu;

		/* initializing data structures */
		TransTable = new CSack;
		TransTable->ISack(sizeof(TransTableElement),128);
		ListOfMenus = new CSack;
		ListOfMenus->ISack(sizeof(MenuListElement),128);
		/* building the menus from those stored as resources */
		ResIndex = 1;
		while ((Menu = (MenuHandle)Get1IndResource('MENU',ResIndex)) != NIL)
			{
				Reconstruct(**(short**)Menu);  /* extract menu's ID & properly reconstruct it */
				ResIndex += 1;
			}
	}


/* destroy the internally used data structures */
void		ShutDownMyMenus(void)
	{
		MenuListElement		Thang;
		TransTableElement	TransThang;

		/* getting rid of menu bar (so that none of these menus are in use when they are killed */
		ClearMenuBar();
		DrawMenuBar();
		if (FontMenuExists)
			{
				DisposeMenu(MenuHandleFromID(FontMenuID));
			}
		/* deleting all menus from the list */
		ListOfMenus->ResetScan();
		while (ListOfMenus->GetNext(&Thang))
			{
				DisposeMenu(Thang.TheMenu);
			}
		/* deleting the list itself */
		delete ListOfMenus;
		/* deleting the translation table */
		delete TransTable;
	}


/* make an item not greyed out */
void		MyEnableItem(short ItemID)
	{
		long				MMID;
		MenuHandle	Menu;

		if (ItemID == mFontSelected)
			{
				ERROR(!FontMenuExists,PRERR(ForceAbort,
					"MyEnableItem enabling nonexistent font menu"));
				EnableItem(MenuHandleFromID(FontMenuID),0);
			}
		 else
			{
				MMID = ItemIDtoMMID(ItemID);
				ERROR(MMID==0,PRERR(ForceAbort,"MyEnableItem called on nonexistent menu item."));
				Menu = MenuHandleFromID((MMID & 0xffff0000) >> 16);
				ERROR(Menu==NIL,PRERR(ForceAbort,"MyEnableItem called on item in nonexistent menu."));
				EnableItem(Menu,MMID & 0x0000ffff);
			}
	}


/* make an item greyed out */
void		MyDisableItem(short ItemID)
	{
		long				MMID;
		MenuHandle	Menu;

		if (ItemID == mFontSelected)
			{
				short			Scan;
				short			Limit;

				ERROR(!FontMenuExists,PRERR(ForceAbort,
					"MyDisableItem enabling nonexistent font menu"));
				Menu = MenuHandleFromID(FontMenuID);
				Limit = CountMItems(Menu);
				for (Scan = 1; Scan <= Limit; Scan += 1)
					{
						DisableItem(Menu,Scan);
						SetItemMark(Menu,Scan,noMark);
					}
			}
		 else
			{
				MMID = ItemIDtoMMID(ItemID);
				ERROR(MMID==0,PRERR(ForceAbort,"MyDisableItem called on nonexistent menu item."));
				Menu = MenuHandleFromID((MMID & 0xffff0000) >> 16);
				ERROR(Menu==NIL,PRERR(ForceAbort,"MyDisableItem called on item in nonexistent menu."));
				DisableItem(Menu,MMID & 0x0000ffff);
			}
	}


/* adjust the checkmark/hierarchicalmenuthang of a menu */
void		MySetItemMark(short ItemID, char ItemMark)
	{
		long				MMID;
		MenuHandle	Menu;

		MMID = ItemIDtoMMID(ItemID);
		ERROR(MMID==0,PRERR(ForceAbort,"MySetItemMark called on nonexistent menu item."));
		Menu = MenuHandleFromID((MMID & 0xffff0000) >> 16);
		ERROR(Menu==NIL,PRERR(ForceAbort,"MySetItemMark called on item in nonexistent menu."));
		SetItemMark(Menu,MMID & 0x0000ffff,ItemMark);
	}


/* create a new, empty menu & return its unique <resource> ID */
short		CreateMenu(Handle Name)
	{
		short				MenuID;
		MenuHandle	TheMenu;
		PString			NameTemp;

		MenuID = FindUnusedMenuID(256);  /* we reserve < 256 for hierarchical menus */
		Handle2PString(Name,NameTemp);
		TheMenu = NewMenu(MenuID,NameTemp);
		AddMenuToList(TheMenu,MenuID);
		ReleaseHandle(Name);
		return MenuID;
	}


/* create a new, empty hierarchical menu with a unique ID */
short		CreateHierarchicalMenu(Handle Name)
	{
		short				MenuID;
		MenuHandle	TheMenu;
		PString			NameTemp;

		MenuID = FindUnusedMenuID(1);
		if (MenuID >= 256)
			{
				return -1;  /* allocation failed */
			}
		Handle2PString(Name,NameTemp);
		TheMenu = NewMenu(MenuID,NameTemp);
		AddMenuToList(TheMenu,MenuID);
		ReleaseHandle(Name);
		return MenuID;
	}


/* create a new item & add it to the specified menu */
/* specify $1b for ItemShortcut to make a hierarchical menu entry */
short		AddItemToMenu(short MenuID, Handle ItemName, char ItemShortcut)
	{
		short				ItemID;
		MenuHandle	TheMenu;
		PString			NameTemp;

		ItemID = FindUnusedItemID();
		TheMenu = MenuHandleFromID(MenuID);
		ERROR(TheMenu==0,PRERR(ForceAbort,"AddItemToMenu called on nonexistent menu."));
		Handle2PString(ItemName,NameTemp);
		AppendMenu(TheMenu,"\px");
		SetItem(TheMenu,CountMItems(TheMenu),NameTemp);
		SetItemCmd(TheMenu,CountMItems(TheMenu),ItemShortcut);
		AddItemToList( ((((long)MenuID)<<16) & 0xffff0000) |
			(0x0000ffff & CountMItems(TheMenu)), ItemID);
		ReleaseHandle(ItemName);
		return ItemID;
	}


void		DeleteItemFromMenu(short ItemID)
	{
		long				MMID;
		MenuHandle	Menu;

		MMID = ItemIDtoMMID(ItemID);
		Menu = MenuHandleFromID((MMID >> 16) & 0x0000ffff);
		DelMenuItem(Menu,MMID & 0x0000ffff);
		RemoveItemFromList(MMID,ItemID);
	}


void		ChangeName(short ItemID, Handle NewName)
	{
		MenuHandle	TheMenu;
		long				MMID;
		PString			TempStr;

		MMID = ItemIDtoMMID(ItemID);
		ERROR(MMID==0,PRERR(ForceAbort,"ChangeName called on nonexistent item."));
		TheMenu = MenuHandleFromID((MMID >> 16) & 0x0000ffff);
		ERROR(TheMenu==NIL,PRERR(ForceAbort,"ChangeName found an item for a nonexistent menu."));
		Handle2PString(NewName,TempStr);
		ReleaseHandle(NewName);
		SetItem(TheMenu,(MMID & 0x0000ffff),TempStr);
	}


/* add a menu to the menu bar.  If it's ID indicates it's hierarchical then add it to that part */
void		PostMenuToBar(short MenuID)
	{
		MenuHandle	TheMenu;

		TheMenu = MenuHandleFromID(MenuID);
		ERROR(TheMenu==NIL,PRERR(ForceAbort,"PostMenuToBar called on nonexistent menu."));
		if (MenuID < 256)
			{
				InsertMenu(TheMenu,-1);  /* hierarchical menus have IDs less than 256 */
			}
		 else
			{
				InsertMenu(TheMenu,0);  /* normals get APPENDED to the menu */
			}
		EnableItem(TheMenu,0); /* make sure menu title is enabled */
		DrawMenuBar();
	}


/* remove any kind of menu from the menu bar list */
void		RemoveMenuFromBar(short MenuID)
	{
		DeleteMenu(MenuID);
		DrawMenuBar();
	}


/* mouse down occurred--track menu bar & return ID of item */
short		MenuMouseDown(EventRecord* TheEvent)
	{
		long			MenuManagerID;
		MyBoolean	Status;

		DisableAll(); /* disable all menu items */
		if (ActiveWindow != NIL)
			{
				ActiveWindow->EnableMenuItems(); /* enable menu items that can be used now */
			}
		MenuManagerID = MenuSelect(TheEvent->where);

		/* checking for user changing his mind */
		if ((MenuManagerID & 0xffff0000) == 0)
			{
				return mNoItem;
			}

		/* some obsolete system 6 nonsense */
#if 0
		if (SystemEdit((MenuManagerID & 0x0000ffff) - 1))
			{
				return mNoItem;
			}
#endif

		/* checking for Font menu */
		if (((MenuManagerID & 0xffff0000) >> 16) == FontMenuID)
			{
				PString		FontName;

				GetItem(MenuHandleFromID((MenuManagerID & 0xffff0000) >> 16),
					MenuManagerID & 0x0000ffff,FontName);
				LastSelectedFontID = GetFontID(FontName);
				return mFontSelected;
			}

		/* checking for Desk accessory open */
		if ((((MenuManagerID & 0xffff0000) >> 16) == AppleMenuID)
			&& ((MenuManagerID & 0x0000ffff) != 1))
			{
				PString		DeskAccName;

				ERROR(AppleMenuHandle==NIL,PRERR(ForceAbort,"AppleMenuHandle == NIL."));
				GetItem(AppleMenuHandle,MenuManagerID & 0x0000ffff,DeskAccName);
				OpenDeskAcc(DeskAccName);
				return mNoItem;
			}

		/* finally, we can return the item!!! */
		return MMIDtoItemID(MenuManagerID);
	}


/* attempt to decode a keyboard shortcut */
short		MenuKeyDown(EventRecord* TheEvent)
	{
		long			MenuManagerID;
		MyBoolean	Status;

		DisableAll(); /* disable all menu items */
		if (ActiveWindow != NIL)
			{
				ActiveWindow->EnableMenuItems(); /* enable menu items that can be used now */
			}
		MenuManagerID = MenuKey(TheEvent->message & 0x000000ff);
		if ((MenuManagerID & 0xffff0000) == 0)
			{
				return mNoItem;
			}
		 else
			{
				if (/*SystemEdit((MenuManagerID & 0x0000ffff) - 1)*/False)
					{
						return mNoItem;
					}
				 else
					{
						return MMIDtoItemID(MenuManagerID);
					}
			}
	}


/* take the specified resource id, destroy any existing menus tied to it, load the resource */
/* from the file, strip ItemID numbers embedded in the strings, and create an internal menu */
/* this routine expects the menu NOT to be on the menu bar at the time */
void		Reconstruct(short MenuID)
	{
		Handle						ResMenu;
		uchar*						PMenu;
		long							Scan;
		MenuHandle				OurMenu;

		RemoveMenuFromBar(MenuID); /* remove it from the menu bar */
		MyDeleteMenu(MenuID);  /* call this to get rid of the old one if there is one */
		/* getting the resource menu template */
		ERROR(ResLoad == 0,PRERR(ForceAbort,"Automatic resource loading is disabled."));
		ResMenu = GetResource('MENU',MenuID);
		ERROR(ResErr != noErr,PRERR(ForceAbort,"Resource Error occurred."));
		MoveHHi(ResMenu);
		HLock(ResMenu);
		PMenu = (uchar*)*ResMenu;
		/* now reconstructing the items */
		Scan = 14;  /* starting position of menu name */
		OurMenu = NewMenu(MenuID,&(PMenu[Scan]));  /* create the menu */
		AddMenuToList(OurMenu,MenuID);  /* add it to the list */
		Scan = Scan + 1 + (PMenu[Scan]);  /* add in the length of the string */
		while ((PMenu[Scan]) != 0) /* adding items as they occur */
			{
				short		i;
				short		ItemID;

				i = 0;
				while (i<PMenu[Scan]) /* scan over the string */
					{
						if (PMenu[Scan+i+1] == '\\')
							{
								long		Temp;

								/* strip the command info from the menu definition & add the item */
								ItemID = Hex2Byte(PMenu[Scan+i+2]);
								ItemID = (ItemID*16) + Hex2Byte(PMenu[Scan+i+3]);
								ItemID = (ItemID*16) + Hex2Byte(PMenu[Scan+i+4]);
								ItemID = (ItemID*16) + Hex2Byte(PMenu[Scan+i+5]);
								Temp = Scan;
								Scan = Scan + 1 + (PMenu[Scan]);  /* skip over menu name */
								PMenu[Temp] = i;  /* new string length */
								AppendMenu(OurMenu,"\px");  /* add string to the menu */
								SetItem(OurMenu,CountMItems(OurMenu),&(PMenu[Temp]));
								goto ResumePoint;  /* escape */
							}
						 else
							{
								i += 1;
							}
					}
				ItemID = FindUnusedItemID();  /* create dummy ID if no ID is specified in resource */
				AppendMenu(OurMenu,"\px");  /* add the string to the menu */
				SetItem(OurMenu,CountMItems(OurMenu),&(PMenu[Scan]));
				Scan = Scan + 1 + (PMenu[Scan]);  /* skip over menu name */
			 ResumePoint:
				SetItemCmd(OurMenu,CountMItems(OurMenu),PMenu[Scan+1]); /* set command key equivalent */
				Scan = Scan + 4;  /* 4 other bytes in the menu definition */
				AddItemToList( ((((long)MenuID)<<16) & 0xffff0000) |
					(0x0000ffff & CountMItems(OurMenu)), ItemID);  /* put it in the translation table */
			}
		HUnlock(ResMenu);
		ReleaseResource(ResMenu);
		if (((**OurMenu).menuData[0] == 1) && ((**OurMenu).menuData[1] == 0x14))
			{
				/* then it's the apple menu, so add desk accessories. */
				AddResMenu(OurMenu,'DRVR');
				AppleMenuID = MenuID;
				AppleMenuHandle = OurMenu;
			}
	}


/********************************************************************************/
/* private routines */


/* auxiliary routine to add menu to the list */
void		AddMenuToList(MenuHandle Menu, short ResID)
	{
		MenuListElement		Temp;

		Temp.MenuID = ResID;
		Temp.TheMenu = Menu;
		ListOfMenus->PushElement(&Temp);
	}


/* auxiliary routine to find an unused Menu ID number starting somewhere */
short		FindUnusedMenuID(short IDStart)
	{
		MenuListElement		Temp;
		short							MenuIDTemp;

		MenuIDTemp = IDStart;
	 RestartPoint:
		ListOfMenus->ResetScan();
		while (ListOfMenus->GetNext(&Temp))
			{
				if (Temp.MenuID == MenuIDTemp)
					{
						MenuIDTemp += 1;
						goto RestartPoint;
					}
			}
		return MenuIDTemp;
	}


/* auxiliary routine to add item to the list */
void		AddItemToList(long MenuManagerID, short ItemID)
	{
		TransTableElement		Temp;

		Temp.MenuManagerID = MenuManagerID;
		Temp.ItemID = ItemID;
		TransTable->PushElement(&Temp);
	}


/* auxiliary routine to remove item from the item list */
/* we have reassign MenuManagerIDs to all items in the same menu after this */
/* one since they will all be shifted up one. */
void		RemoveItemFromList(long MenuManagerID, short ItemID)
	{
		TransTableElement		Temp;

		Temp.MenuManagerID = MenuManagerID;
		Temp.ItemID = ItemID;
		TransTable->KillElement(&Temp);
		TransTable->ResetScan();
		while (TransTable->GetNext(&Temp))
			{
				if ((Temp.MenuManagerID & 0xffff0000) == (MenuManagerID & 0xffff0000))
					{
						if ((Temp.MenuManagerID & 0x0000ffff) > (MenuManagerID & 0x0000ffff))
							{
								MyBoolean			Success;

								Temp.MenuManagerID = (Temp.MenuManagerID & 0xffff0000)
									| (((Temp.MenuManagerID & 0x0000ffff) - 1) & 0x0000ffff);
								Success = TransTable->RewriteCurrent(&Temp);
								ERROR(!Success,PRERR(AllowResume,
									"RemoveItemFromList RewriteCurrent failed"));
							}
					}
			}
	}


/* auxiliary routine to find an unused Item ID number starting somewhere */
short		FindUnusedItemID(void)
	{
		TransTableElement		Temp;
		ushort							TempID;

		TempID = 0xffff;
	 RestartPoint:
		TransTable->ResetScan();
		while (TransTable->GetNext(&Temp))
			{
				if (Temp.ItemID == TempID)
					{
						TempID -= 1;
						goto RestartPoint;
					}
			}
		return TempID;
	}


/* convert a MenuManagerID to an ItemID */
short		MMIDtoItemID(long MMID)
	{
		TransTableElement		Temp;

		TransTable->ResetScan();
		while (TransTable->GetNext(&Temp))
			{
				if (Temp.MenuManagerID == MMID)
					{
						return Temp.ItemID;
					}
			}
		EXECUTE(PRERR(AllowResume,"Undefined menu item position ID referenced."));
		EXECUTE(return -1);
	}


/* convert an ItemID to a MenuManagerID */
long		ItemIDtoMMID(short ItemID)
	{
		TransTableElement		Temp;

		TransTable->ResetScan();
		while (TransTable->GetNext(&Temp))
			{
				if (Temp.ItemID == ItemID)
					{
						return Temp.MenuManagerID;
					}
			}
		EXECUTE(PRERR(AllowResume,"Undefined menu item local ID referenced."));
		EXECUTE(return 0);
	}


/* take the menu ID and find the handle referring to that menu. */
/* returns NIL if it couldn't find anything */
MenuHandle	MenuHandleFromID(short MenuID)
	{
		MenuListElement		Temp;

		ListOfMenus->ResetScan();
		while (ListOfMenus->GetNext(&Temp))
			{
				if (Temp.MenuID == MenuID)
					{
						return Temp.TheMenu;
					}
			}
		return NIL;
	}


/* utterly delete a menu (assume it has been removed from the bar) */
void		MyDeleteMenu(short MenuID)
	{
		MenuListElement			MTemp;
		TransTableElement		ITemp;
		void*								Temp;
		long								Scan;
		MenuHandle					HandleTemp;

		/* destroying the old menu */
		HandleTemp = MenuHandleFromID(MenuID);
		if (HandleTemp != NIL)
			{
				DisposeMenu(HandleTemp);  /* dispose the menu */
			}
		/* removing menu record from list */
	 LoopPoint1:
		ListOfMenus->ResetScan();
		while (ListOfMenus->GetNext(&MTemp))
			{
				if (MTemp.MenuID == MenuID)
					{
						ListOfMenus->KillElement(&MTemp);
						goto LoopPoint1;
					}
			}
		/* removing translation table entries */
	 LoopPoint2:
		TransTable->ResetScan();
		while (TransTable->GetNext(&ITemp))
			{
				if ( ((ITemp.MenuManagerID & 0xffff0000) >> 16) == MenuID)
					{
						TransTable->KillElement(&ITemp);
						goto LoopPoint2;
					}
			}
	}


/* disable all menu items */
void		DisableAll(void)
	{
		TransTableElement		Temp;
		MenuHandle					Menu;
		long								NumElements;
		long								Scan;

		TransTable->ResetScan();
		while (TransTable->GetNext(&Temp))
			{
				short				TempCmd;

				Menu = MenuHandleFromID((Temp.MenuManagerID & 0xffff0000) >> 16);
				DisableItem(Menu,Temp.MenuManagerID & 0x0000ffff);
				GetItemCmd(Menu,Temp.MenuManagerID & 0x0000ffff,&TempCmd);
				if (TempCmd != 0x001b)
					{
						/* disable item's mark if it is not a hierarchical menu */
						SetItemMark(Menu,Temp.MenuManagerID & 0x0000ffff,noMark);
					}
			}
		if (FontMenuExists)
			{
				short			Scan;
				short			Limit;

				ERROR(!FontMenuExists,PRERR(ForceAbort,
					"MyDisableItem enabling nonexistent font menu"));
				Menu = MenuHandleFromID(FontMenuID);
				Limit = CountMItems(Menu);
				for (Scan = 1; Scan <= Limit; Scan += 1)
					{
						DisableItem(Menu,0);
						SetItemMark(Menu,Scan,noMark);
					}
			}
	}


/* add a font menu */
short		AddFontMenu(void)
	{
		ERROR(FontMenuExists,PRERR(AllowResume,"AddFontMenu called when one already exists"));
		FontMenuExists = True;
		FontMenuID = CreateMenu(GetCString(FontMenuNameID));
		AddResMenu(MenuHandleFromID(FontMenuID),'FONT');
		return FontMenuID;
	}


/* find out which font was selected */
short		GetLastFontID(void)
	{
		return LastSelectedFontID;
	}


/* determine which font will be set */
void		SetFontMark(short FontID)
	{
		PString			FontName;
		MenuHandle	Menu;
		short				Scan;
		short				Limit;
		PString			ItemName;

		ERROR(!FontMenuExists,PRERR(AllowResume,"SetFontMark called when no font menu exists"));
		GetFontName(FontID,FontName);
		Menu = MenuHandleFromID(FontMenuID);
		Limit = CountMItems(Menu);
		for (Scan = 1; Scan <= Limit; Scan += 1)
			{
				GetItem(Menu,Scan,ItemName);
				if (MemEqu((char*)FontName,(char*)ItemName,FontName[0] + 1))
					{
						SetItemMark(Menu,Scan,checkMark);
						return;
					}
			}
		/* font not found, but who cares! */
	}
