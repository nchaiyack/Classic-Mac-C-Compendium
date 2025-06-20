
/*
	To do:
	
	� Support hierarchicals
	� Support color
*/

#include "MetaGlobal.h"
#include "MenubarCDEF.h"

#ifdef THINK_C
	#include <SetUpA4.h>
	Ptr GetA0(void) = { 0x2008 };
#else
	#include <Types.h>
	#include <Memory.h>
	#include <Menus.h>
	#include <OSEvents.h>			// For EvQPtr
	#include <Resources.h>			// For GetResource
	#include <Script.h>				// For GetMBarHeight
	#include <ToolUtils.h>			// For HiWord, LoWord
	#include <Windows.h>			// we return inMenubar as our part
	#include "SAGlobals.h"
	
	#include "gcc overrides.h"
	
	#define MenuHook (*(ProcPtr*) 0xA30)
	#define MenuList (*(Handle*) 0xA1C)
#endif

#define NIL				NULL
#define kTopMargin		1
#define kBottomMargin	1
#define kTextMargin		8


typedef struct {
	MenuHandle		menu;
	short			menuLeft;
} MenuRec, *MenuRecPtr;


typedef struct {
	MenuHandle		menu;
	short			reserved;
} HMenuRec, *HMenuRecPtr;


typedef struct {
	short			lastMenu;
	short			lastRight;
	short			mbResID;
	MenuRec			menus[1];
	//	short			lastHMenu;
	//	PixMapHandle	menuTitleSave;
	//	MenuRec			hmenus[1];
} MenuBar, *MenuBarPtr, **MenuBarHdl;


ProcPtr			gOldMenuHook;
ControlHandle	gControl;
MenuBarHdl		gMenuList;
short			gTheMenu;			// _Index_ of currently hilighted menu
short			gNormalHeight;
FontInfo		gFontInfo;
short			gBaseline;


pascal long		main(short varCode, ControlHandle theControl, short msg, long param);

void			DrawMyControl(short part);
short			TestMyControl(Point location);
void			InitMyControl(void);
void			DisposeMyControl(void);

void			DoClearMenuBar(void);
void			DoDeleteMenu(short menuID);
void			DoDrawMenuBar(void);
void			DoFlashMenuBar(short menuID);
MenuBarHdl		DoGetMenuBar(void);
MenuHandle		DoGetMHandle(short menuID);
void			DoHiliteMenu(short menuID);
void			DoInsertMenu(MenuHandle menu, short before);
long			DoMenuKey(char theKey);
long			DoMenuSelect(Point startPt);
void			DoSetMenuBar(MenuBarHdl menuBar);

void			DrawMenuTitle(short menuIndex);
short			FindHitMenu(Point location);		// returns menuIndex (or -1)
short			GetFreeHMenuID(void);
Rect			GetHitRect(short menuIndex);
Rect			GetInvertRect(short menuIndex);
short			GetMenuCount(void);
MenuHandle		GetNthMenu(short index);
Point			GetTitleLocation(short menuIndex);
short			IDToIndex(short);
short			IndexToID(short);
void			MyMenuHook(void);
void			SwapMenuBars(void);

//--------------------------------------------------------------------------------

pascal long	main(short varCode, ControlHandle theControl, short msg, long param)
{
	long		result;
	Ptr			oldA5;
	char		oldState;
	Handle		ourHandle;
	Ptr			ourPtr;

	ourPtr = GetA0();

#ifdef THINK_C
	RememberA0();
	SetUpA4();
#else
	oldA5 = UseGlobals();
#endif

	ourHandle = RecoverHandle(ourPtr);
	oldState = HGetState(ourHandle);
	HLock(ourHandle);
	
	result = 0;
	gControl = theControl;
	GetFontInfo(&gFontInfo);
	gBaseline = kTopMargin + gFontInfo.leading + gFontInfo.ascent;

	if (msg < 128) {
		switch (msg) {
			case drawCntl:
				DrawMyControl((short) param);
				break;
			case testCntl:
				result = TestMyControl(*(Point*) &param);
				break;
			case initCntl:
				InitMyControl();
				break;
			case dispCntl:
				DisposeMyControl();
				break;
			case calcCntlRgn:
			case calcThumbRgn:
				RectRgn((RgnHandle) param, &(**gControl).contrlRect);
				result = 1;
				break;
		}
	} else {
		switch (msg) {
			case clearMenuBar:
				DoClearMenuBar();
				break;
			case deleteMenu:
				DoDeleteMenu((short) param);
				break;
			case drawMenuBar:
				DoDrawMenuBar();
				break;
			case flashMenuBar:
				DoFlashMenuBar((short) param);
				break;
			case getMenuBar:
				result = (long) DoGetMenuBar();
				break;
			case getMHandle:
				result = (long) DoGetMHandle((short) param);
				break;
			case hiliteMenu:
				DoHiliteMenu((short) param);
				break;
			case insertMenu:
				DoInsertMenu((MenuHandle) param, varCode);
				break;
			case menuKey:
				result = DoMenuKey((char) param);
				break;
			case menuSelect:
				result = DoMenuSelect(*(Point*) &param);
				break;
			case setMenuBar:
				DoSetMenuBar((MenuBarHdl) param);
				break;
		}
	}

#ifdef THINK_C
	RestoreA4();
#else
	DoneWithGlobals(oldA5);
#endif
	
	HSetState(ourHandle, oldState);

	return result;
}


//--------------------------------------------------------------------------------

void	DrawMyControl(short part)
{
#pragma unused (part)

	if ((**gControl).contrlVis != false) {
		DoDrawMenuBar();
	}
}


//--------------------------------------------------------------------------------

short	TestMyControl(Point location)
{
	short		hitPart;
	
	hitPart = 0;
	if (FindHitMenu(location) >= 0) {
		hitPart = inMenuBar;
		if (Button()) {
			DoMenuSelect(location);
			if ((**gControl).contrlValue == 0) {
				hitPart = 0;
			}
		}
	}

	return hitPart;
}


//--------------------------------------------------------------------------------

void	InitMyControl()
{
	Handle		menuBar;

	gOldMenuHook = NIL;
	gMenuList = NIL;
	gTheMenu = -1;
	gNormalHeight = GetMBarHeight();
	(**gControl).contrlRect.bottom = (**gControl).contrlRect.top + gBaseline
						+ gFontInfo.descent + gFontInfo.leading + kBottomMargin;

	GetMBarHeight() = 0;
	menuBar = GetNewMBar((**gControl).contrlMin);
	if (menuBar != NIL) {
		gMenuList = (MenuBarHdl) menuBar;
	} else {
		menuBar = GetMenuBar();
		ClearMenuBar();
		gMenuList = (MenuBarHdl) GetMenuBar();
		SetMenuBar(menuBar);
		DisposeHandle(menuBar);
	}
	GetMBarHeight() = gNormalHeight;
}


//--------------------------------------------------------------------------------

void	DisposeMyControl()
{
	DisposeHandle((Handle) gMenuList);
}


//--------------------------------------------------------------------------------

void	DoClearMenuBar()
{
	SwapMenuBars();
	ClearMenuBar();
	SwapMenuBars();
}


//--------------------------------------------------------------------------------

void	DoDeleteMenu(short menuID)
{
	if (IndexToID(gTheMenu) == menuID)
		gTheMenu = -1;

	SwapMenuBars();
	DeleteMenu(menuID);
	SwapMenuBars();
}


//--------------------------------------------------------------------------------

void	DoDrawMenuBar()
{
	Rect			frame;
	short			loop;

	PenNormal();
	frame = (**gControl).contrlRect;
	FrameRect(&frame);

	gTheMenu = -1;
	for (loop = GetMenuCount() - 1; loop >= 0; loop--) {
		DrawMenuTitle(loop);
	}
}


//--------------------------------------------------------------------------------

void	DoFlashMenuBar(short menuID)
{
	Rect		bounds;

	if (menuID != 0)
		DoHiliteMenu(menuID);
	else {
		bounds = (**gControl).contrlRect;
		InsetRect(&bounds, 1, 1);
		InvertRect(&bounds);
	}
}


//--------------------------------------------------------------------------------

MenuBarHdl	DoGetMenuBar()
{
	return gMenuList;
}


//--------------------------------------------------------------------------------

MenuHandle	DoGetMHandle(short menuID)
{
	short	menuIndex;
	
	menuIndex = IDToIndex(menuID);
	if (menuIndex >= 0)
		return GetNthMenu(menuIndex);
	else
		return NIL;
}


//--------------------------------------------------------------------------------

void	DoHiliteMenu(short menuID)
{
	short		menuIndex;
	Rect		invertRect;
	
	menuIndex = IDToIndex(menuID);
	if (menuIndex != gTheMenu) {
		if (menuIndex >= 0) {
			invertRect = GetInvertRect(menuIndex);
			InvertRect(&invertRect);
		}
		gTheMenu = menuIndex;
	}
}


//--------------------------------------------------------------------------------

void	DoInsertMenu(MenuHandle menu, short before)
{
	SwapMenuBars();
	InsertMenu(menu, before);
	SwapMenuBars();
}


//--------------------------------------------------------------------------------

long	DoMenuKey(char theKey)
{
	char			upperKey[2];
	short			loop;
	MenuHandle		menu;
	short			mItems;
	short			menuItem;
	short			key;

	upperKey[0] = 1;
	upperKey[1] = theKey;
	UprString((StringPtr) &upperKey, true);

	for (loop = GetMenuCount() - 1; loop >= 0; loop--) {
		menu = GetNthMenu(loop);
		mItems = CountMItems(menu);
		for (menuItem = 1; menuItem <= mItems; menuItem++) {
			if (menuItem > 31 || (((**menu).enableFlags & (1 << menuItem)) != 0)) {
				GetItemCmd(menu, menuItem, &key);
				if (key == upperKey[1]) {
					return ((long) (**menu).menuID << 16) + menuItem;
				}
			}
		}
	}

	return 0;
}


//--------------------------------------------------------------------------------

long	DoMenuSelect(Point startPt)
{
#pragma unused (startPt)

	Point			currentLocation;
	short			oldMenuIndex;
	short			newMenuIndex;
	Rect			invertRect;
	Point			where;
	MenuHandle		menu;
	long			menuAndItem;
	short			oldMenuID;
	Handle			myMDEF;

	menuAndItem = 0;
	while (Button()) {
		GetMouse(&currentLocation);
		oldMenuIndex = gTheMenu;
		newMenuIndex = FindHitMenu(currentLocation);
		if (oldMenuIndex != newMenuIndex) {
			gTheMenu = newMenuIndex;
			DrawMenuTitle(oldMenuIndex);
			if (newMenuIndex >= 0) {
				DrawMenuTitle(newMenuIndex);
	
				invertRect = GetInvertRect(newMenuIndex);
				where.v = invertRect.bottom + 1;
				where.h = invertRect.left + 1;
				LocalToGlobal(&where);
	
				gOldMenuHook = MenuHook;
				MenuHook = (ProcPtr) MyMenuHook;
	
				menu = GetNthMenu(newMenuIndex);
	
				oldMenuID = (**menu).menuID;
				(**menu).menuID = GetFreeHMenuID();
	
				myMDEF = GetResource('MDEF', kStayPutMDEF);
				if (myMDEF != NIL) {
					(**(StdHeaderHdl) myMDEF).refCon = (long) (**menu).menuProc;
					(**menu).menuProc = myMDEF;
				}
	
				InsertMenu(menu, hierMenu);
				menuAndItem = PopUpMenuSelect(menu, where.v, where.h, 1);
				DeleteMenu((**menu).menuID);
	
				(**menu).menuID = oldMenuID;
	
				if (myMDEF != NIL) {
					(**menu).menuProc = (Handle) (**(StdHeaderHdl) myMDEF).refCon;
					(**(StdHeaderHdl) myMDEF).refCon = (long) NIL;
				}
	
				MenuHook = gOldMenuHook;
	
				if (HiWord(menuAndItem) != 0)
					menuAndItem = ((long) oldMenuID << 16) + (short) menuAndItem;
	
				(**gControl).contrlValue = HiWord(menuAndItem);
				(**gControl).contrlMax = LoWord(menuAndItem);
			}
		}
	}

	if (gTheMenu != -1) {
		oldMenuIndex = gTheMenu;
		gTheMenu = -1;
		DrawMenuTitle(oldMenuIndex);
	}

	return menuAndItem;
}


//--------------------------------------------------------------------------------

void	DoSetMenuBar(MenuBarHdl menuBar)
{
	DisposeHandle((Handle) gMenuList);
	gMenuList = menuBar;
	DoDrawMenuBar();
}


//--------------------------------------------------------------------------------

void	DrawMenuTitle(short menuIndex)
{
	Rect			invertRect;
	Point			titleLocation;
	MenuHandle		menu;

	if ((menuIndex >= 0) && (menuIndex < GetMenuCount())) {

		invertRect = GetInvertRect(menuIndex);
		EraseRect(&invertRect);

		menu = GetNthMenu(menuIndex);
		if ((((**menu).enableFlags & 1) == 0) || ((**gControl).contrlHilite == 255))
			TextMode(grayishTextOr);
		else
			TextMode(srcOr);
		
		titleLocation = GetTitleLocation(menuIndex);
		MoveTo(titleLocation.h, titleLocation.v);
		HLock((Handle) menu);
		DrawString((**menu).menuData);
		HUnlock((Handle) menu);

		if (gTheMenu == menuIndex)
			InvertRect(&invertRect);
	}
}


//--------------------------------------------------------------------------------

short	FindHitMenu(Point location)
{
	short			loop;
	Rect			hitRect;

	for (loop = GetMenuCount() - 1; loop >= 0; loop--) {
		hitRect = GetHitRect(loop);
		if (PtInRect(location, &hitRect)) {
			break;
		}
	}

	return loop;		// returns -1 if no hit
}


//--------------------------------------------------------------------------------

short	GetFreeHMenuID(void)
{
	short	index;

	for (index = 235-20; index > 0; index--) {
		if (GetMHandle(index) == NIL)
			return index;
	}
	return -1;
}


//--------------------------------------------------------------------------------

Rect	GetHitRect(short menuIndex)
{
	Rect		bounds;
	RectPtr		rectPtr;
	MenuBarPtr	menuBarPtr;
	
	rectPtr = &(**gControl).contrlRect;
	menuBarPtr = *gMenuList;

	bounds.top = rectPtr->top + 1;
	bounds.bottom = rectPtr->bottom - 1;
	bounds.left = rectPtr->left + menuBarPtr->menus[menuIndex].menuLeft;
	menuIndex++;
	bounds.right = rectPtr->left + ((menuIndex < GetMenuCount())
										? menuBarPtr->menus[menuIndex].menuLeft
										: menuBarPtr->lastRight);
	
	return bounds;
}


//--------------------------------------------------------------------------------

Rect	GetInvertRect(short menuIndex)
{
	Rect		bounds;
	
	bounds = GetHitRect(menuIndex);
	bounds.left--;
	bounds.right += 4;
	return bounds;
}


//--------------------------------------------------------------------------------

short	GetMenuCount()
{
	return (**gMenuList).lastMenu / (short) sizeof(MenuRec);
}


//--------------------------------------------------------------------------------

MenuHandle	GetNthMenu(short index)
{
	return (**gMenuList).menus[index].menu;
}


//--------------------------------------------------------------------------------

Point	GetTitleLocation(short menuIndex)
{
	Point	result;
	
	result.h = (**gMenuList).menus[menuIndex].menuLeft + kTextMargin;
	result.v = (**gControl).contrlRect.top + gBaseline;
	
	return result;
}


//--------------------------------------------------------------------------------

short	IDToIndex(short menuID)
{
	short		loop;

	for (loop = GetMenuCount() - 1; loop >= 0; loop--) {
		if ((**GetNthMenu(loop)).menuID == menuID)
			break;
	}
	return loop;
}


//--------------------------------------------------------------------------------

short	IndexToID(short menuIndex)
{
	if ((menuIndex >= 0) && (menuIndex < GetMenuCount()))
		return (**GetNthMenu(menuIndex)).menuID;
	else
		return 0;
}


//--------------------------------------------------------------------------------

void	MyMenuHook()
{
	typedef void (*MenuHookProc)(void);

	GrafPtr		oldPort;
	Point		mouseLocation;
	short		hitMenu;
	Ptr			oldA5;

#ifdef THINK_C
	SetUpA4();
#else
	oldA5 = UseGlobals();
#endif

	if (gOldMenuHook != NIL)
		((MenuHookProc) gOldMenuHook)();

	GetPort(&oldPort);
	SetPort((**gControl).contrlOwner);
	GetMouse(&mouseLocation);
	hitMenu = FindHitMenu(mouseLocation);
	if ((hitMenu >= 0) && (hitMenu != gTheMenu)) {
		PostEvent(mouseUp, 0);
	}
	SetPort(oldPort);

#ifdef THINK_C
	RestoreA4();
#else
	DoneWithGlobals(oldA5);
#endif
}


//--------------------------------------------------------------------------------

void	SwapMenuBars()
{
	MenuBarHdl		oldMenuBar;
	
	oldMenuBar = (MenuBarHdl) MenuList;
	MenuList = (Handle) gMenuList;
	gMenuList = oldMenuBar;
}





#if 0

void			CalcMenuPositions(short index);
long			CallMBDF(short message, short param1, long param2);

//--------------------------------------------------------------------------------

void	CalcMenuPositions(short index)
{
	SwapMenuBars();
	CallMBDF(2, 0, (index+1) * 6);
	SwapMenuBars();
}


//--------------------------------------------------------------------------------

long	CallMBDF(short message, short param1, long param2)
{
	typedef pascal long (*MBDFProc)(short selector, short message, short parameter1,
										long parameter2);
	Handle		mbdfHandle;
	short		mbResID;
	short		resID;
	short		mbVariant;
	char		oldState;
	long		result;
	
	mbResID = (**gMenuList).mbResID;
	resID = mbResID >> 3;
	mbVariant = mbResID & 0x0007;
	mbdfHandle = GetResource('MBDF', resID);
	if (mbdfHandle != NIL) {
		if (*mbdfHandle == NIL)  {
			LoadResource(mbdfHandle);
		}
		if (*mbdfHandle != NIL) {
		
			oldState = HGetState(mbdfHandle);
			HLock(mbdfHandle);
			result = ((MBDFProc) *mbdfHandle)(mbVariant, message, param1, param2);
			HSetState(mbdfHandle, oldState);
			return result;
		}
	}

	SysError(dsMBarNFnd);
}
#endif


