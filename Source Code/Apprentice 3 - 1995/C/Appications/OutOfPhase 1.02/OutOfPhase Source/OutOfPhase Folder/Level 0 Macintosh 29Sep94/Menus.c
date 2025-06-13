/* Menus.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Debug.h"
#include "Audit.h"
#include "Definitions.h"

#ifdef THINK_C
	#pragma options(pack_enums)
#endif
#include <Menus.h>
#include <Fonts.h>
#include <Desk.h>
#ifdef THINK_C
	#pragma options(!pack_enums)
#endif

#include "Menus.h"
#include "Memory.h"
#include "Array.h"


/* structure represents a menu */
struct MenuType
	{
		/* menu resource ID number (must be unique) */
		short					MenuID;
		/* handle pointing to system's idea of what the menu is */
		MenuHandle		DaMenuHandle;
	};


/* structure represents a menu item (it remembers the menu and index of item) */
struct MenuItemType
	{
		/* Menu ID + Item Index number used by menu manager to identify item */
		long					MenuManagerID;
		/* pointer to the menu record for the menu that contains the item. */
		MenuType*			WhatMenu;
	};


/* list of menus (for finding unique menu IDs) */
static ArrayRec*					OurMenuList = NIL;

/* list of items in the menus (for searching during menu-choice events) */
static ArrayRec*					OurItemList = NIL;

/* the Apple menu record.  NIL means there is no Apple menu */
static MenuType*					AppleMenu = NIL;

/* number of "real" items on the apple menu.  This is set to 0 whenever the */
/* apple menu is created. */
static long								NumAppleMenuItems;

/* flag indicating that menu bar needs to be redrawn */
static MyBoolean					RedrawMenuBarFlag = False;

/* debugging flag */
EXECUTE(static MyBoolean	Initialized = False;)


/* Initialize the menu subsystem.  This must be called before any menu routines */
/* are used.  It is local to Level 0 and called from module Screen (InitializeScreen) */
/* and should not be called from anywhere else. */
MyBoolean							Eep_InitializeMenus(void)
	{
		ERROR(Initialized,PRERR(ForceAbort,"InitializeMenus called more than once"));
		/* create list that contains menus */
		OurMenuList = NewArray();
		if (OurMenuList == NIL)
			{
			 FailurePoint1:
				return False;
			}
		/* create the list that will hold all of the menu item records (for searching) */
		OurItemList = NewArray();
		if (OurItemList == NIL)
			{
			 FailurePoint2:
				DisposeArray(OurMenuList);
				goto FailurePoint1;
			}
		/* initialize the apple menu variable to indicate that there isn't one */
		AppleMenu = NIL;
		EXECUTE(Initialized = True);
		return True;
	}


/* Destroy any menu stuff that needs to be cleaned up before the program quits. */
/* this should not be called from anywhere else except ShutdownScreen */
void									Eep_ShutdownMenus(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,
			"ShutdownMenus:  Menu manager hasn't been initialized"));
		ERROR(ArrayGetLength(OurMenuList) != 0,PRERR(AllowResume,
			"Eep_ShutdownMenus:  still some menus in existence"));
		ERROR(ArrayGetLength(OurItemList) != 0,PRERR(AllowResume,
			"Eep_ShutdownMenus:  still some menu items in existence"));
		DisposeArray(OurMenuList);
		DisposeArray(OurItemList);
	}


/* create an implementation defined "utility" menu.  On the Macintosh, this is */
/* the standard "Apple Menu". */
MenuType*							MakeAppleMenu(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		/* if an apple menu exists, we return it instead of creating a new one */
		if (AppleMenu == NIL)
			{
				/* create a new menu normally with an apple character */
				AppleMenu = MakeNewMenu("\024");
				if (AppleMenu == NIL)
					{
						return NIL;
					}
				/* initialize item count */
				NumAppleMenuItems = 0;
				/* append the line and desk accessory list */
				AppendMenu(AppleMenu->DaMenuHandle,"\p(-");
				AddResMenu(AppleMenu->DaMenuHandle,'DRVR');
			}
		CheckPtrExistence(AppleMenu);
		return AppleMenu;
	}


/* create a new menu with the specified name.  The menu will not */
/* be displayed on the menu bar */
MenuType*							MakeNewMenu(char* MenuName)
	{
		unsigned char				NameTemp[256];
		long								Scan;
		long								Limit;
		MenuType*						Menu;

		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		/* allocate the menu record */
		Menu = (MenuType*)AllocPtrCanFail(sizeof(MenuType),"MenuType");
		if (Menu == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		/* convert name to a pascal string */
		Scan = 0;
		while ((Scan < 255) && (MenuName[Scan] != 0))
			{
				NameTemp[Scan + 1] = MenuName[Scan];
				Scan += 1;
			}
		NameTemp[0] = Scan;
		/* find an unused ID number */
		Limit = ArrayGetLength(OurMenuList);
		Menu->MenuID = 256; /* skip 0..255 which are reserved for hierarchical menus */
	 LoopPoint:
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				if (Menu->MenuID == ((MenuType*)ArrayGetElement(OurMenuList,Scan))->MenuID)
					{
						Menu->MenuID += 1;
						goto LoopPoint;
					}
				ERROR(Scan > 16383,PRERR(ForceAbort,"MakeNewMenu:  out of menu IDs"));
			}
		/* allocate the menu itself */
		Menu->DaMenuHandle = NewMenu(Menu->MenuID,NameTemp);
		if (Menu->DaMenuHandle == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Menu);
				goto FailurePoint1;
			}
		/* add menu to list */
		if (!ArrayAppendElement(OurMenuList,Menu))
			{
			 FailurePoint3:
				DisposeMenu(Menu->DaMenuHandle);
				goto FailurePoint2;
			}
		return Menu;
	}


/* hide a menu if it's on the menu bar and delete it and all of the items */
/* it contains. */
void									KillMenuAndDeleteItems(MenuType* TheMenu)
	{
		long								Scan;
		long								Limit;

		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		CheckPtrExistence(TheMenu);
		/* remove menu from our list of menus */
		ArrayDeleteElement(OurMenuList,ArrayFindElement(OurMenuList,TheMenu));
		/* remove menu from the menu bar */
		HideMenu(TheMenu);
		/* dispose the system menu block */
		DisposeMenu(TheMenu->DaMenuHandle);
		/* reset the apple menu value if we're disposing that */
		if (TheMenu == AppleMenu)
			{
				AppleMenu = NIL;
			}
		/* dispose all translation entries for the menu */
		Limit = ArrayGetLength(OurItemList);
		Scan = 0;
		while (Scan < Limit)
			{
				MenuItemType*			MenuItem;

				MenuItem = (MenuItemType*)ArrayGetElement(OurItemList,Scan);
				if ((MenuItem->MenuManagerID >> 16) == TheMenu->MenuID)
					{
						/* dispose the item */
						ArrayDeleteElement(OurItemList,Scan);
						ReleasePtr((char*)MenuItem);
						/* since there's one less item now, decrement limit */
						Limit -= 1;
						/* note that we don't increment scan */
					}
				 else
					{
						/* scan is only incremented if we didn't delete the element */
						Scan += 1;
					}
			}
		/* finally, delete the menu record */
		ReleasePtr((char*)TheMenu);
	}


/* post a menu to the menu bar if it isn't already there */
void									ShowMenu(MenuType* TheMenu)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		CheckPtrExistence(TheMenu);
		/* append menu to menu bar */
		InsertMenu(TheMenu->DaMenuHandle,0);
		/* set flag to indicate that the menu bar needs to be redrawn */
		RedrawMenuBarFlag = True;
	}


/* remove a menu from the menu bar if it is there */
void									HideMenu(MenuType* TheMenu)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		CheckPtrExistence(TheMenu);
		/* remove menu from menu bar */
		DeleteMenu(TheMenu->MenuID);
		/* set flag to indicate that the menu bar needs to be redrawn */
		RedrawMenuBarFlag = True;
	}


/* append a new item to an existing menu.  The Shortcut specifies a key that */
/* can be used instead of pulling down the menu.  How this is done and which */
/* keys are allowed are implementation defined.  On the Macintosh, the Command */
/* key is used; keys should be numbers or upper case letters.  If two menu items */
/* are specified with the same shortcut, the result is undefined. */
/* by default, the item is greyed out (disabled). */
MenuItemType*					MakeNewMenuItem(MenuType* TheMenu, char* MenuItemName,
												char Shortcut)
	{
		unsigned char				NameTemp[256];
		long								Scan;
		MenuItemType*				MenuItem;
		short								Index;

		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		CheckPtrExistence(TheMenu);
		/* allocate record */
		MenuItem = (MenuItemType*)AllocPtrCanFail(sizeof(MenuItemType),"MenuItemType");
		if (MenuItem == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		/* add item to the item list */
		if (!ArrayAppendElement(OurItemList,MenuItem))
			{
			 FailurePoint2:
				ReleasePtr((char*)MenuItem);
				goto FailurePoint1;
			}
		/* find out where on menu the item should be placed */
		if (TheMenu == AppleMenu)
			{
				Index = NumAppleMenuItems + 1;
				NumAppleMenuItems += 1;
			}
		 else
			{
				Index = CountMItems(TheMenu->DaMenuHandle) + 1;
			}
		/* insert a dummy item (name will be changed) */
		InsMenuItem(TheMenu->DaMenuHandle,"\px",Index - 1);
		/* convert name */
		Scan = 0;
		while ((Scan < 255) && (MenuItemName[Scan] != 0))
			{
				NameTemp[Scan + 1] = MenuItemName[Scan];
				Scan += 1;
			}
		NameTemp[0] = Scan;
		/* set the name properly */
		SetItem(TheMenu->DaMenuHandle,Index,NameTemp);
		SetItemCmd(TheMenu->DaMenuHandle,Index,Shortcut);
		/* set item's ID mapping */
		MenuItem->MenuManagerID = ((long)(TheMenu->MenuID) << 16) | Index;
		MenuItem->WhatMenu = TheMenu;
		return MenuItem;
	}


/* delete the specified item from the menu. */
void									KillMenuItem(MenuItemType* TheItem)
	{
		long							Scan;
		long							Limit;

		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		CheckPtrExistence(TheItem);
		/* remove item from item list */
		ArrayDeleteElement(OurItemList,ArrayFindElement(OurItemList,TheItem));
		/* delete item from actual menu */
		DelMenuItem(TheItem->WhatMenu->DaMenuHandle,TheItem->MenuManagerID & 0xffff);
		/* fix up item IDs for items after this one on the same menu */
		Limit = ArrayGetLength(OurItemList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				MenuItemType*			OtherItem;

				OtherItem = (MenuItemType*)ArrayGetElement(OurItemList,Scan);
				if (((TheItem->MenuManagerID >> 16) == (OtherItem->MenuManagerID >> 16))
					&& ((TheItem->MenuManagerID & 0xffff) < (OtherItem->MenuManagerID & 0xffff)))
					{
						/* all items after this one move up.  We don't have to do masking and */
						/* all that because item IDs (low 16 bits) are positive, so a borrow */
						/* will never occur */
						OtherItem->MenuManagerID -= 1;
					}
			}
		/* fix up apple menu length */
		if (TheItem->WhatMenu == AppleMenu)
			{
				NumAppleMenuItems -= 1;
			}
		/* release the memory */
		ReleasePtr((char*)TheItem);
	}


/* enable a menu item. Items may only be selected if enabled. */
void									EnableMenuItem(MenuItemType* TheItem)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		CheckPtrExistence(TheItem);
		EnableItem(TheItem->WhatMenu->DaMenuHandle,TheItem->MenuManagerID & 0xffff);
	}


/* disable a menu item. */
void									DisableMenuItem(MenuItemType* TheItem)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		CheckPtrExistence(TheItem);
		DisableItem(TheItem->WhatMenu->DaMenuHandle,TheItem->MenuManagerID & 0xffff);
	}


/* Set an implementation defined mark to indicate that the menu item has been */
/* persistently selected.  On the Macintosh, this places a checkmark to the left */
/* of the name of the menu item */
void									SetItemCheckmark(MenuItemType* TheItem)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		CheckPtrExistence(TheItem);
		SetItemMark(TheItem->WhatMenu->DaMenuHandle,TheItem->MenuManagerID & 0xffff,checkMark);
	}


/* remove the implementation defined mark */
void									ClearItemCheckmark(MenuItemType* TheItem)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		CheckPtrExistence(TheItem);
		SetItemMark(TheItem->WhatMenu->DaMenuHandle,TheItem->MenuManagerID & 0xffff,noMark);
	}


/* change the name of a menu item */
void									ChangeItemName(MenuItemType* TheItem, char* NewName)
	{
		unsigned char				NameTemp[256];
		long								Scan;

		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		CheckPtrExistence(TheItem);
		Scan = 0;
		while ((Scan < 255) && (NewName[Scan] != 0))
			{
				NameTemp[Scan + 1] = NewName[Scan];
				Scan += 1;
			}
		NameTemp[0] = Scan;
		SetItem(TheItem->WhatMenu->DaMenuHandle,TheItem->MenuManagerID & 0xffff,NameTemp);
	}


/* Add an implementation defined "separator" to the end of the menu.  On the */
/* Macintosh, this separator is a grey line. */
void									AppendSeparator(MenuType* TheMenu)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		CheckPtrExistence(TheMenu);
		AppendMenu(TheMenu->DaMenuHandle,"\p(-");
	}


/* Disable all menu items, remove any checkmarks */
void									WipeMenusClean(void)
	{
		long								Scan;
		long								Limit;

		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		Limit = ArrayGetLength(OurItemList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				MenuItemType*				Item;

				Item = (MenuItemType*)ArrayGetElement(OurItemList,Scan);
				DisableMenuItem(Item);
				ClearItemCheckmark(Item);
			}
	}


/* internal routine for converting the number returned from the Toolbox into */
/* one of our own ID numbers.  if the MMID number is an apple menu item, then */
/* it is handled here and NIL is returned. */
MenuItemType*					Eep_MMID2ItemID(long MMID)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Menu manager hasn't been initialized"));
		if ((AppleMenu != NIL) && ((MMID >> 16) == AppleMenu->MenuID)
			&& ((MMID & 0xffff) > NumAppleMenuItems))
			{
				Str255							DeskAccName;

				GetItem(AppleMenu->DaMenuHandle,MMID & 0xffff,DeskAccName);
				APRINT(("*OpenDeskAcc %p",DeskAccName));
				OpenDeskAcc(DeskAccName);
				HiliteMenu(0);
			}
		 else
			{
				long								Scan;
				long								Limit;

				Limit = ArrayGetLength(OurItemList);
				for (Scan = 0; Scan < Limit; Scan += 1)
					{
						MenuItemType*				Item;

						Item = (MenuItemType*)ArrayGetElement(OurItemList,Scan);
						if (Item->MenuManagerID == MMID)
							{
								return Item;
							}
					}
			}
		/* fall through == no item. */
		return NIL;
	}


/* this is called from the event loop to redraw the menu bar if it needs it. */
/* redraws are deferred and unified to save time. */
void									Eep_RedrawMenuBar(void)
	{
		if (RedrawMenuBarFlag)
			{
				DrawMenuBar();
				RedrawMenuBarFlag = False;
			}
	}
