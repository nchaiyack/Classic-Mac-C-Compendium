/* Menus.h */

#ifndef Included_Menus_h
#define Included_Menus_h

/* Menus module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Array */

/* structure representing a menu */
struct MenuType;
typedef struct MenuType MenuType;

/* structure representing a menu item */
struct MenuItemType;
typedef struct MenuItemType MenuItemType;

/* Initialize the menu subsystem.  This must be called before any menu routines */
/* are used.  It is local to Level 0 and called from module Screen (InitializeScreen) */
/* and should not be called from anywhere else. */
MyBoolean							Eep_InitializeMenus(void);

/* Destroy any menu stuff that needs to be cleaned up before the program quits. */
/* this should not be called from anywhere else except ShutdownScreen */
void									Eep_ShutdownMenus(void);

/* create an implementation defined "utility" menu.  On the Macintosh, this is */
/* the standard "Apple Menu". */
MenuType*							MakeAppleMenu(void);

/* create a new menu with the specified name.  The menu will not */
/* be displayed on the menu bar */
MenuType*							MakeNewMenu(char* MenuName);

/* hide a menu if it's on the menu bar and delete it and all of the items */
/* it contains. */
void									KillMenuAndDeleteItems(MenuType* TheMenu);

/* post a menu to the menu bar if it isn't already there */
void									ShowMenu(MenuType* TheMenu);

/* remove a menu from the menu bar if it is there */
void									HideMenu(MenuType* TheMenu);

/* append a new item to an existing menu.  The Shortcut specifies a key that */
/* can be used instead of pulling down the menu.  How this is done and which */
/* keys are allowed are implementation defined.  On the Macintosh, the Command */
/* key is used; keys should be numbers or upper case letters.  If two menu items */
/* are specified with the same shortcut, the result is undefined. */
/* by default, the item is greyed out (disabled). */
MenuItemType*					MakeNewMenuItem(MenuType* TheMenu, char* MenuItemName,
												char Shortcut);

/* delete the specified item from the menu. */
void									KillMenuItem(MenuItemType* TheItem);

/* enable a menu item. Items may only be selected if enabled. */
void									EnableMenuItem(MenuItemType* TheItem);

/* disable a menu item. */
void									DisableMenuItem(MenuItemType* TheItem);

/* Set an implementation defined mark to indicate that the menu item has been */
/* persistently selected.  On the Macintosh, this places a checkmark to the left */
/* of the name of the menu item */
void									SetItemCheckmark(MenuItemType* TheItem);

/* remove the implementation defined mark */
void									ClearItemCheckmark(MenuItemType* TheItem);

/* change the name of a menu item */
void									ChangeItemName(MenuItemType* TheItem, char* NewName);

/* Add an implementation defined "separator" to the end of the menu.  On the */
/* Macintosh, this separator is a grey line. */
void									AppendSeparator(MenuType* TheMenu);

/* Disable all menu items, remove any checkmarks */
void									WipeMenusClean(void);


/* private routines; These are not available for general use. */
MenuItemType*					Eep_MMID2ItemID(long MMID);
void									Eep_RedrawMenuBar(void);

#endif
