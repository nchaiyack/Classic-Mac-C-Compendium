#ifndef THINK_C
#include <Controls.h>
#endif

#define kStayPutMDEF	90

#define clearMenuBar	128
#define deleteMenu		129			// param = ID of menu to remove
#define drawMenuBar		130
#define flashMenuBar	131			// param = ID of menu to flash
#define getMenuBar		132			// result = handle to custom data
#define getMHandle		133			// param = menu ID, result = menuHandle
#define hiliteMenu		134			// param = ID of menu to hilite
#define insertMenu		135			// param = menuHandle, varCode = before menu ID
#define menuKey			136			// param = char, result = same as MenuKey
#define menuSelect		137			// param = startPt, result = same as MenuSelect
#define setMenuBar		138			// param = result from getMenuBar

typedef pascal long (*CDEFProc)(short varCode, ControlHandle theControl,
									short msg, long param);

#define CallIt(ctl) ((CDEFProc) *((**ctl).contrlDefProc))

#define ZClearMenuBar(ctl)			CallIt(ctl)(0, ctl, clearMenuBar, 0)
#define ZDeleteMenu(ctl, menuID)	CallIt(ctl)(0, ctl, deleteMenu, menuID)
#define ZDrawMenuBar(ctl)			CallIt(ctl)(0, ctl, drawMenuBar, 0)
#define ZFlashMenuBar(ctl, menuID)	CallIt(ctl)(0, ctl, flashMenuBar, menuID)
#define ZGetMenuBar(ctl)			(Handle) CallIt(ctl)(0, ctl, getMenuBar, 0)
#define ZGetMHandle(ctl, menuID)	(MenuHandle) CallIt(ctl)(0, ctl, getMHandle, menuID)
#define ZHiliteMenu(ctl, menuID)	CallIt(ctl)(0, ctl, hiliteMenu, 0)
#define ZInsertMenu(ctl, mh, id)	CallIt(ctl)(id, ctl, insertMenu, mh)
#define ZMenuKey(ctl, key)			CallIt(ctl)(0, ctl, menuKey, key)
#define ZMenuSelect(ctl, startPt)	CallIt(ctl)(0, ctl, menuSelect, startPt)
#define ZSetMenuBar(ctl, mbar)		CallIt(ctl)(0, ctl, setMenuBar, mbar)
