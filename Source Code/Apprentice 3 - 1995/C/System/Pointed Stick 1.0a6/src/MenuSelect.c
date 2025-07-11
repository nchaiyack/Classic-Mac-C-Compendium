#include "Global.h"
#include "MenuSelect.h"
#include "MenuSetup.h"			// CreateFileMenu
#include "Pack3.h"				// gFileToReturn
#include "PLStringFuncs.h"		// PLstrcompare
#include "Utilities.h"			// MyGetResource

typedef pascal long	(*MenuSelectProc)(Point);


typedef struct {
	MenuHandle		menu;
	short			menuLeft;
} MenuRec, *MenuRecPtr;


typedef struct {
	short		lastMenu;
	short		lastRight;
	short		mbResID;
	MenuRec		menus[1];
} MenuListRec, *MenuListPtr, **MenuListHdl;


UniversalProcPtr	gOldMenuSelect;

Handle				pOldProc;

StringHandle		pAppMSTRHandle;
Handle				pAppMSTListHandle;
StringHandle		pSysMSTRHandle;
Handle				pSysMSTListHandle;

long				MenuSelectMyWay(MenuHandle openMenu, short openMenuItem,
								MenuHandle fileMenu, Point startPt);
Handle				CreateNewProc(void);
pascal void			WrapperProc(short msg, MenuHandle theMenu, Rect *menuRect, 
						Point hitPt, short *itemID);
void				GetOpenMenuAndItem(MenuHandle* menu, short* menuItem);
MenuHandle			FindFileMenu(void);
short				FindOpenItem(MenuHandle);
void				GetMultiFinderStrings(short resID);
Boolean				InSearchOf(Str255 string);
Boolean				CheckAgainstMSTR(StringHandle theString, Str255 string);
Boolean				CheckAgainstMSTList(Handle theStrings, Str255 string);


//--------------------------------------------------------------------------------
/*
	Our patch on MenuSelect that adds the hierarchical menu to the Open menu
	item in the File menu.

	We first try to get the MenuHandle to the File menu and the item ID of the
	Open menu item. If we find both of them, we check the enableFlags to see
	if the File menu and Open item are both enabled. If so, we try to open our
	resource fork so we can create our menus, if that works, we try to create
	menu holding the list of recently accessed files. If that works, we call
	MenuSelectMyWay to install the menu and call MenuSelect.

	The above description involves a lot of "ifs". If any of those conditions
	is false (i.e., something failed), we blow out and merely call MenuSelect
	as if nothing happened.
*/

pascal long	MyMenuSelect(Point startPt)
{
	Boolean			didIt;
	MenuHandle		openMenu;
	short			openMenuItem;
	long			enableFlags;
	Boolean			closeIt;
	MenuHandle		fileMenu;
	long			result;

	#ifdef BREAK_ON_ENTRY
	DebugStr("\pBreaking on MyMenuSelect");
	#endif

	SetUpA4();

	didIt = FALSE;
	GetOpenMenuAndItem(&openMenu, &openMenuItem);
	if ((openMenu != NIL) && (openMenuItem > 0)) {

		enableFlags = (**openMenu).enableFlags;
		if ( ((enableFlags & 1) != 0) &&
		  ((openMenuItem > 31) || ((enableFlags & (1 << openMenuItem)) != 0))) {

			fileMenu = CreateFileMenu();
			if (fileMenu != NIL) {
				FilterFilesByFREF();
				AddFiles(fileMenu);
				if (CountMItems(fileMenu) > 0) {
					result = MenuSelectMyWay(openMenu, openMenuItem, fileMenu, startPt);
					didIt = TRUE;
				}
				DisposeMenu(fileMenu);
			} else {
				DebugStr("\pCouldn't create the file menu.");
			}
		}
	}

	if (!didIt)
		result = ((MenuSelectProc) gOldMenuSelect)(startPt);

	RestoreA4();

	return result;
}


//--------------------------------------------------------------------------------
/*
	When we get here, we�ve successfully found the Open menu item, and were
	able to create the menu to attach to it.

	This function attaches the hierarchical menu and calls MenuSelect. When
	MenuSelect returns, we see if an item from the hierarchical was selected.
	If so, we simulate a hit on Open and prepare ourselves for some pupppet-
	stringing via our Pack3 patch.

	If MenuSelect tells us that nothing at all was selected, we have to check
	to see if that was really true. Normally, MenuSelect will return zero if
	the parent of a hierarchical was selected. However, this precludes the
	user from selecting �Open��. Therefore, we call MenuChoice, which will
	tell us if Open was selected. If so, we return the Open item as the
	result.
*/

long	MenuSelectMyWay(MenuHandle openMenu, short openMenuItem, MenuHandle fileMenu,
				Point startPt)
{
	short			fileMenuID;
	short			oldItemCmd;
	long			result;

	fileMenuID = (**fileMenu).menuID;

	GetItemCmd(openMenu, openMenuItem, &oldItemCmd);
	SetItemCmd(openMenu, openMenuItem, hMenuCmd);
	SetItemMark(openMenu, openMenuItem, fileMenuID);

	pOldProc = (**openMenu).menuProc;
	(**openMenu).menuProc = CreateNewProc();
	if ((**openMenu).menuProc == NIL) {
		(**openMenu).menuProc = pOldProc;
	}

	InsertMenu(fileMenu, hierMenu);
	result = ((MenuSelectProc) gOldMenuSelect)(startPt);
	DeleteMenu(fileMenuID);
	
	if ((**openMenu).menuProc != pOldProc) {
		DisposeHandle((**openMenu).menuProc);
		(**openMenu).menuProc = pOldProc;
	}

	SetItemCmd(openMenu, openMenuItem, oldItemCmd);
	SetItemMark(openMenu, openMenuItem, noMark);

	if (HiWrd(result) == fileMenuID) {
		gFileToReturn = GetNthOpenableFileObject(gFileList, LoWrd(result)-1);
		if (gFileToReturn != NIL) {
			result = MergeShorts((**openMenu).menuID, openMenuItem);
			if (FileHandleExists(gFileToReturn)) {
				gStandardFileMode = kFaking;
			} else {
				DeleteFileObject(gFileList, gFileToReturn);
				gFileToReturn = NIL;
			}
		} else {
			result = 0;
		}
	} else if (result == 0) {
		result = MergeShorts((**openMenu).menuID, openMenuItem);
		if (result != MenuChoice())
			result = 0;
	}

	return result;
}


//--------------------------------------------------------------------------------

Handle	CreateNewProc()
{
	typedef struct JumpBlock {
		short	jmp;
		void*	routine;
	} JumpBlock;
	
	Handle	newProc = NewHandle(sizeof(JumpBlock));
	
	if (newProc) {
		(**(JumpBlock**) newProc).jmp = 0x4EF9;
		(**(JumpBlock**) newProc).routine = (void*) &WrapperProc;
		
		FlushInstructionCache();
	}
	
	return newProc;
}


//--------------------------------------------------------------------------------

pascal void WrapperProc(short msg, MenuHandle theMenu, Rect* menuRect, Point hitPt,
					short* itemID)
{
	typedef pascal void (*MDEFProc)(short msg, MenuHandle theMenu, Rect* menuRect,
								Point hitPt, short* itemID);

	char			oldState;
	Handle			origMDEF;

	SetUpA4();
	
	if (pOldProc != NIL) {
		if (*pOldProc == NIL)  {
			LoadResource(pOldProc);
		}
		if (*pOldProc != NIL) {
			origMDEF = (**theMenu).menuProc;
			(**theMenu).menuProc = pOldProc;
			oldState = HGetState(pOldProc);
			HLock(pOldProc);
			((MDEFProc) *pOldProc)(msg, theMenu, menuRect, hitPt, itemID);
			HSetState(pOldProc, oldState);
			(**theMenu).menuProc = origMDEF;
			switch (msg) {
				case mChooseMsg:
					LMSetMenuDisable(((long) (**theMenu).menuID << 16) + *itemID);
					break;
			}
		}
	}

	RestoreA4();
}

//--------------------------------------------------------------------------------
/*
	Return the MenuHandle to the File menu, and the item number of the Open
	menu item. If the File menu can�t be found, we return NIL for it. If the
	File menu can be found, but we can�t find the Open menu item within it, we
	return -1 for the menu item number.
*/

void	GetOpenMenuAndItem(MenuHandle* menu, short* menuItem)
{
	*menu = FindFileMenu();
	if (*menu != NIL)
		*menuItem = FindOpenItem(*menu);
	else
		*menuItem = -1;
}


//--------------------------------------------------------------------------------
/*
	Try to find the File menu. To do this, we work from the 'mstr' and 'mst#'
	resources in the System file, as well as any that happen to be in the open
	application.

	First, we load up the appropriate resources (could be 4 in all). Next, we
	walk the menus. For each one, we pass its title to InSearchOf, which will
	compare the string against every string in the 'mst�' resources. If we
	find a match, we return the MenuHandle. If we don�t find a match, we
	return NIL.
*/

MenuHandle	FindFileMenu(void)
{
	MenuListHdl		menuList;
	MenuRecPtr		currentMenu;
	MenuRecPtr		lastMenu;
	MenuHandle		theMenu;

	GetMultiFinderStrings(102);
	menuList = (MenuListHdl) LMGetMenuList();
	lastMenu = (MenuRecPtr) (((char*) *menuList) + (**menuList).lastMenu);
	for (currentMenu = (**menuList).menus; currentMenu <= lastMenu; currentMenu++) {
		theMenu = currentMenu->menu;
		if (InSearchOf((**theMenu).menuData))
			return theMenu;
	}
	return NIL;
}


//--------------------------------------------------------------------------------
/*
	Try to find the Open menu item. To do this, we work from the 'mstr' and
	'mst#' resources in the System file, as well as any that happen to be in
	the open application.

	First, we load up the appropriate resources (could be 4 in all). Next, we
	walk the menu items in the given menu. For each one, we pass its title to
	InSearchOf, which will compare the string against every string in the 'mst
	�' resources. If we find a match, we return the item number. If we don�t
	find a match, we return zero.
*/

short	FindOpenItem(MenuHandle fileMenu)
{
	short	index;
	short	numberOfMenuItems;
	Str255	itemString;

	GetMultiFinderStrings(103);
	numberOfMenuItems = CountMItems(fileMenu);
	for (index = 1; index <= numberOfMenuItems; index++) {
		GetItem(fileMenu, index, itemString);
		if (InSearchOf(itemString))
			return index;
	}
	return 0;
}


//--------------------------------------------------------------------------------
/*
	Preload the resources used by MultiFinder for puppet-stringing. There are
	four resources in all: 'mst#' and 'mstr' in the system resource file, and
	the same in the application file.

	Resources with ID 102 holds names for the File menu, while those with ID
	103 hold the names of the Open menu item. Whichever ID is appropriate is
	passed in the �resID� parameter.
*/

void	GetMultiFinderStrings(short resID)
{
	short		oldResFile;
	THz			currentZone;

	currentZone = GetZone();
	SetZone(SystemZone());

	pAppMSTRHandle = (StringHandle) MyGetResource('mstr', resID);
	pAppMSTListHandle = MyGetResource('mst#', resID);

	oldResFile = CurResFile();
	UseResFile(0);

	pSysMSTRHandle = (StringHandle) GetResource('mstr', resID);
	pSysMSTListHandle = GetResource('mst#', resID);

	UseResFile(oldResFile);

	SetZone(currentZone);
}


//--------------------------------------------------------------------------------
/*
	See if the given string is in any of the four resources loaded in
	LoadMultiFinderStrings. If so, return TRUE, else return FALSE.
*/

Boolean	InSearchOf(Str255 string)
{
	Boolean		result;
	short		oldResFile;

	result = CheckAgainstMSTR(pAppMSTRHandle, string);
	if (!result)
		result = CheckAgainstMSTList(pAppMSTListHandle, string);
	if (!result)
		result = CheckAgainstMSTR(pSysMSTRHandle, string);
	if (!result)
		result = CheckAgainstMSTList(pSysMSTListHandle, string);
	return result;
}

//--------------------------------------------------------------------------------
/*
	See if the given string is in the given 'mstr' resource. This is pretty
	simple: we just call PLstrcompare to perform a non-case sensitive
	comparison.
*/

Boolean	CheckAgainstMSTR(StringHandle theString, Str255 string)
{
	Boolean		result;

	if (theString != NIL) {
		HLock((Handle) theString);
		result = PLstrcompare(*theString, string) == 0;
		HUnlock((Handle) theString);
	} else
		result = FALSE;

	return result;
}


//--------------------------------------------------------------------------------
/*
	See if the given string is in the given 'mst#' resource. We walk the list
	of strings in the resource, calling PLstrcompare to perform a non-case
	sensitive comparison.
*/

Boolean	CheckAgainstMSTList(Handle theStrings, Str255 string)
{
	Boolean		result;
	Str255*		currentString;
	short		numberOfStrings;

	if (theStrings != NIL) {
		HLock(theStrings);
		numberOfStrings = *(short*) *theStrings;
		currentString = (Str255*) (*theStrings + 2);
		while (numberOfStrings > 0) {
			result = PLstrcompare(*currentString, string) == 0;
			if (result)
				break;
			numberOfStrings--;
			currentString = (Str255*) ((char*)currentString + StrLength(currentString) + 1);
		}
		HUnlock(theStrings);
	} else
		result = FALSE;

	return result;
}
