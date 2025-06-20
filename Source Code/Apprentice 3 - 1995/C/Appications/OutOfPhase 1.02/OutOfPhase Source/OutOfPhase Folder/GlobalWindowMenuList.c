/* GlobalWindowMenuList.c */
/*****************************************************************************/
/*                                                                           */
/*    Out Of Phase:  Digital Music Synthesis on General Purpose Computers    */
/*    Copyright (C) 1994  Thomas R. Lawrence                                 */
/*                                                                           */
/*    This program is free software; you can redistribute it and/or modify   */
/*    it under the terms of the GNU General Public License as published by   */
/*    the Free Software Foundation; either version 2 of the License, or      */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*    GNU General Public License for more details.                           */
/*                                                                           */
/*    You should have received a copy of the GNU General Public License      */
/*    along with this program; if not, write to the Free Software            */
/*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "GlobalWindowMenuList.h"
#include "Array.h"
#include "Memory.h"
#include "Menus.h"


typedef struct
	{
		void*											Refcon;
		void											(*RaiseCallback)(void* Refcon);
		struct MenuItemType*			MenuItem;
	} WindowMenuRec;


static ArrayRec*					MenuItemList = NIL;


/* initialize the internal structures for the list */
MyBoolean					InitializeGlobalWindowMenuList(void)
	{
		ERROR(MenuItemList != NIL,PRERR(ForceAbort,
			"InitializeGlobalWindowMenuList:  already initialized"));
		MenuItemList = NewArray();
		if (MenuItemList == NIL)
			{
			 FailurePoint1:
				return False;
			}
		return True;
	}


/* dispose of the internal structures for the list */
void							ShutdownGlobalWindowMenuList(void)
	{
		ERROR(ArrayGetLength(MenuItemList) != 0,PRERR(ForceAbort,
			"ShutdownGlobalWindowMenuList:  some menu items are still registered"));
		DisposeArray(MenuItemList);
		MenuItemList = NIL;
	}


/* add a new menu item to the window list */
MyBoolean					RegisterWindowMenuItem(struct MenuItemType* Item,
										void (*RaiseFunction)(void* Refcon), void* Refcon)
	{
		WindowMenuRec*	Record;

		CheckPtrExistence(Item);
		Record = (WindowMenuRec*)AllocPtrCanFail(sizeof(WindowMenuRec),"WindowMenuRec");
		if (Record == NIL)
			{
			 FailurePoint1:
				return False;
			}
		Record->Refcon = Refcon;
		Record->RaiseCallback = RaiseFunction;
		Record->MenuItem = Item;
		if (!ArrayAppendElement(MenuItemList,Record))
			{
			 FailurePoint2:
				ReleasePtr((char*)Record);
				goto FailurePoint1;
			}
		return True;
	}


/* remove a menu item from the window list */
void							DeregisterWindowMenuItem(struct MenuItemType* Item)
	{
		long						Limit;
		long						Scan;

		CheckPtrExistence(Item);
		Limit = ArrayGetLength(MenuItemList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				WindowMenuRec*	Record;

				Record = (WindowMenuRec*)ArrayGetElement(MenuItemList,Scan);
				CheckPtrExistence(Record);
				if (Record->MenuItem == Item)
					{
						ArrayDeleteElement(MenuItemList,Scan);
						ReleasePtr((char*)Record);
						return;
					}
			}
		EXECUTE(PRERR(ForceAbort,"DeregisterWindowMenuItem:  item not found"));
	}


/* dispatch a window menu item.  If it wasn't one, then return False, otherwise */
/* raise the appropriate window and return True */
MyBoolean					DispatchWindowMenuItem(struct MenuItemType* Item)
	{
		long						Limit;
		long						Scan;

		CheckPtrExistence(Item);
		Limit = ArrayGetLength(MenuItemList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				WindowMenuRec*	Record;

				Record = (WindowMenuRec*)ArrayGetElement(MenuItemList,Scan);
				CheckPtrExistence(Record);
				if (Record->MenuItem == Item)
					{
						(*Record->RaiseCallback)(Record->Refcon);
						return True;
					}
			}
		return False;
	}


/* enable the items in the window menu */
void							WindowMenuEnableItems(void)
	{
		long						Limit;
		long						Scan;

		Limit = ArrayGetLength(MenuItemList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				WindowMenuRec*	Record;

				Record = (WindowMenuRec*)ArrayGetElement(MenuItemList,Scan);
				EnableMenuItem(Record->MenuItem);
			}
	}
