/* GlobalWindowMenuList.h */

#ifndef Included_GlobalWindowMenuList_h
#define Included_GlobalWindowMenuList_h

/* GlobalWindowMenuList module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Array */
/* Memory */
/* Menus */

/* forwards */
struct MenuItemType;

/* initialize the internal structures for the list */
MyBoolean					InitializeGlobalWindowMenuList(void);

/* dispose of the internal structures for the list */
void							ShutdownGlobalWindowMenuList(void);

/* add a new menu item to the window list */
MyBoolean					RegisterWindowMenuItem(struct MenuItemType* Item,
										void (*RaiseFunction)(void* Refcon), void* Refcon);

/* remove a menu item from the window list */
void							DeregisterWindowMenuItem(struct MenuItemType* Item);

/* dispatch a window menu item.  If it wasn't one, then return False, otherwise */
/* raise the appropriate window and return True */
MyBoolean					DispatchWindowMenuItem(struct MenuItemType* Item);

/* enable the items in the window menu */
void							WindowMenuEnableItems(void);

#endif
